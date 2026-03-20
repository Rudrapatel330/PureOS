#include "ahci.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "ports.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);
extern void k_itoa_hex(uint32_t n, char *s);

static hba_mem_t *ahci_hba = 0;
static int ahci_sata_port = -1; // First SATA port found

// --- Port type detection ---
static int check_type(hba_port_t *port) {
  uint32_t ssts = port->ssts;
  uint8_t ipm = (ssts >> 8) & 0x0F;
  uint8_t det = ssts & 0x0F;

  if (det != HBA_PORT_DET_PRESENT)
    return AHCI_DEV_NULL;
  if (ipm != HBA_PORT_IPM_ACTIVE)
    return AHCI_DEV_NULL;

  switch (port->sig) {
  case SATA_SIG_ATAPI:
    return AHCI_DEV_SATAPI;
  case SATA_SIG_SEMB:
    return AHCI_DEV_SEMB;
  case SATA_SIG_PM:
    return AHCI_DEV_PM;
  default:
    return AHCI_DEV_SATA;
  }
}

// --- Stop command engine on a port ---
static void ahci_stop_cmd(hba_port_t *port) {
  port->cmd &= ~HBA_PxCMD_ST;
  port->cmd &= ~HBA_PxCMD_FRE;

  // Wait until FR and CR are cleared
  int timeout = 500000;
  while (timeout-- > 0) {
    if (!(port->cmd & HBA_PxCMD_FR) && !(port->cmd & HBA_PxCMD_CR))
      break;
  }
}

// --- Start command engine on a port ---
static void ahci_start_cmd(hba_port_t *port) {
  // Wait until CR is cleared
  int timeout = 500000;
  while ((port->cmd & HBA_PxCMD_CR) && timeout-- > 0)
    ;

  port->cmd |= HBA_PxCMD_FRE;
  port->cmd |= HBA_PxCMD_ST;
}

// --- Initialize a port (allocate command list + FIS receive) ---
static void ahci_port_init(hba_port_t *port, int portno) {
  ahci_stop_cmd(port);

  // Allocate command list (1KB aligned)
  uint32_t clb_phys;
  void *clb = kmalloc_ap(1024, &clb_phys);
  memset(clb, 0, 1024);
  port->clb = clb_phys;
  port->clbu = 0;

  // Allocate FIS receive area (256 bytes aligned)
  uint32_t fb_phys;
  void *fb = kmalloc_ap(256, &fb_phys);
  memset(fb, 0, 256);
  port->fb = fb_phys;
  port->fbu = 0;

  // Allocate command tables (one per command header, 32 total)
  hba_cmd_header_t *cmd_hdr = (hba_cmd_header_t *)(uintptr_t)clb_phys;
  for (int i = 0; i < 32; i++) {
    cmd_hdr[i].prdtl = 8; // 8 PRDT entries per command
    uint32_t ctba_phys;
    void *ct = kmalloc_ap(256, &ctba_phys);
    memset(ct, 0, 256);
    cmd_hdr[i].ctba = ctba_phys;
    cmd_hdr[i].ctbau = 0;
  }

  // Clear pending interrupts
  port->serr = 0xFFFFFFFF;
  port->is = 0xFFFFFFFF;

  ahci_start_cmd(port);

  char buf[16];
  print_serial("AHCI: Port ");
  k_itoa(portno, buf);
  print_serial(buf);
  print_serial(" initialized\n");
}

// --- Find a free command slot ---
static int ahci_find_cmdslot(hba_port_t *port) {
  uint32_t slots = (port->sact | port->ci);
  for (int i = 0; i < 32; i++) {
    if (!(slots & (1 << i)))
      return i;
  }
  return -1;
}

// --- Read sectors via DMA ---
int ahci_read_sectors(int portno, uint64_t lba, uint32_t count, void *buffer) {
  if (!ahci_hba || portno < 0 || portno >= 32)
    return -1;

  hba_port_t *port = &ahci_hba->ports[portno];
  port->is = 0xFFFFFFFF; // Clear interrupts

  int slot = ahci_find_cmdslot(port);
  if (slot < 0) {
    print_serial("AHCI: No free command slot\n");
    return -1;
  }

  hba_cmd_header_t *cmd_hdr = (hba_cmd_header_t *)(uintptr_t)(port->clb);
  cmd_hdr += slot;
  cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
  cmd_hdr->w = 0; // Read
  cmd_hdr->prdtl = (uint16_t)((count - 1) / 16 + 1);

  hba_cmd_table_t *cmd_tbl = (hba_cmd_table_t *)(uintptr_t)(cmd_hdr->ctba);
  memset(cmd_tbl, 0,
         sizeof(hba_cmd_table_t) +
             (cmd_hdr->prdtl - 1) * sizeof(hba_prdt_entry_t));

  // Setup PRDT entries
  uint32_t buf_phys = (uint32_t)(uintptr_t)buffer;
  int i;
  for (i = 0; i < cmd_hdr->prdtl - 1; i++) {
    cmd_tbl->prdt_entry[i].dba = buf_phys;
    cmd_tbl->prdt_entry[i].dbau = 0;
    cmd_tbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8KB per PRDT
    cmd_tbl->prdt_entry[i].i = 1;
    buf_phys += 8 * 1024;
    count -= 16; // 16 sectors = 8KB
  }
  // Last entry
  cmd_tbl->prdt_entry[i].dba = buf_phys;
  cmd_tbl->prdt_entry[i].dbau = 0;
  cmd_tbl->prdt_entry[i].dbc = (count * 512) - 1;
  cmd_tbl->prdt_entry[i].i = 1;

  // Setup command FIS (H2D Register FIS)
  fis_reg_h2d_t *fis = (fis_reg_h2d_t *)(&cmd_tbl->cfis);
  memset(fis, 0, sizeof(fis_reg_h2d_t));

  fis->fis_type = FIS_TYPE_REG_H2D;
  fis->c = 1; // Command
  fis->command = ATA_CMD_READ_DMA_EX;

  fis->lba0 = (uint8_t)(lba & 0xFF);
  fis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
  fis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
  fis->device = 1 << 6; // LBA mode

  fis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
  fis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
  fis->lba5 = (uint8_t)((lba >> 40) & 0xFF);

  fis->countl = count & 0xFF;
  fis->counth = (count >> 8) & 0xFF;

  // Wait for port to be ready
  int timeout = 1000000;
  while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && timeout-- > 0)
    ;
  if (timeout <= 0) {
    print_serial("AHCI: Port busy timeout\n");
    return -1;
  }

  // Issue command
  port->ci = 1 << slot;

  // Wait for completion
  while (1) {
    if (!(port->ci & (1 << slot)))
      break;
    if (port->is & HBA_PxIS_TFES) {
      print_serial("AHCI: Read disk error\n");
      return -1;
    }
  }

  if (port->is & HBA_PxIS_TFES) {
    print_serial("AHCI: Read disk error (post)\n");
    return -1;
  }

  return 0;
}

// --- Write sectors via DMA ---
int ahci_write_sectors(int portno, uint64_t lba, uint32_t count,
                       const void *buffer) {
  if (!ahci_hba || portno < 0 || portno >= 32)
    return -1;

  hba_port_t *port = &ahci_hba->ports[portno];
  port->is = 0xFFFFFFFF;

  int slot = ahci_find_cmdslot(port);
  if (slot < 0)
    return -1;

  hba_cmd_header_t *cmd_hdr = (hba_cmd_header_t *)(uintptr_t)(port->clb);
  cmd_hdr += slot;
  cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
  cmd_hdr->w = 1; // Write
  cmd_hdr->prdtl = (uint16_t)((count - 1) / 16 + 1);

  hba_cmd_table_t *cmd_tbl = (hba_cmd_table_t *)(uintptr_t)(cmd_hdr->ctba);
  memset(cmd_tbl, 0,
         sizeof(hba_cmd_table_t) +
             (cmd_hdr->prdtl - 1) * sizeof(hba_prdt_entry_t));

  uint32_t buf_phys = (uint32_t)(uintptr_t)buffer;
  int i;
  for (i = 0; i < cmd_hdr->prdtl - 1; i++) {
    cmd_tbl->prdt_entry[i].dba = buf_phys;
    cmd_tbl->prdt_entry[i].dbau = 0;
    cmd_tbl->prdt_entry[i].dbc = 8 * 1024 - 1;
    cmd_tbl->prdt_entry[i].i = 1;
    buf_phys += 8 * 1024;
    count -= 16;
  }
  cmd_tbl->prdt_entry[i].dba = buf_phys;
  cmd_tbl->prdt_entry[i].dbau = 0;
  cmd_tbl->prdt_entry[i].dbc = (count * 512) - 1;
  cmd_tbl->prdt_entry[i].i = 1;

  fis_reg_h2d_t *fis = (fis_reg_h2d_t *)(&cmd_tbl->cfis);
  memset(fis, 0, sizeof(fis_reg_h2d_t));

  fis->fis_type = FIS_TYPE_REG_H2D;
  fis->c = 1;
  fis->command = ATA_CMD_WRITE_DMA_EX;

  fis->lba0 = (uint8_t)(lba & 0xFF);
  fis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
  fis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
  fis->device = 1 << 6;

  fis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
  fis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
  fis->lba5 = (uint8_t)((lba >> 40) & 0xFF);

  fis->countl = count & 0xFF;
  fis->counth = (count >> 8) & 0xFF;

  int timeout = 1000000;
  while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && timeout-- > 0)
    ;
  if (timeout <= 0)
    return -1;

  port->ci = 1 << slot;

  while (1) {
    if (!(port->ci & (1 << slot)))
      break;
    if (port->is & HBA_PxIS_TFES) {
      print_serial("AHCI: Write disk error\n");
      return -1;
    }
  }

  return 0;
}

// --- Probe ports and init ---
static void probe_port(hba_mem_t *hba) {
  uint32_t pi = hba->pi;
  for (int i = 0; i < 32; i++) {
    if (pi & 1) {
      int dt = check_type(&hba->ports[i]);
      if (dt == AHCI_DEV_SATA) {
        char buf[16];
        print_serial("AHCI: SATA drive found on port ");
        k_itoa(i, buf);
        print_serial(buf);
        print_serial("\n");
        ahci_port_init(&hba->ports[i], i);
        if (ahci_sata_port < 0)
          ahci_sata_port = i;
      } else if (dt == AHCI_DEV_SATAPI) {
        char buf[16];
        print_serial("AHCI: SATAPI drive found on port ");
        k_itoa(i, buf);
        print_serial(buf);
        print_serial("\n");
      }
    }
    pi >>= 1;
  }
}

// --- Public init ---
void ahci_init(uint32_t bar5) {
  print_serial("AHCI: Initializing controller...\n");
  ahci_hba = (hba_mem_t *)(uintptr_t)bar5;

  // Enable AHCI mode
  ahci_hba->ghc |= (1 << 31); // AHCI Enable

  probe_port(ahci_hba);

  if (ahci_sata_port >= 0) {
    print_serial("AHCI: Ready (read/write available)\n");
  } else {
    print_serial("AHCI: No SATA drives found\n");
  }
}

int ahci_get_sata_port(void) { return ahci_sata_port; }
