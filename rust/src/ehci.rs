#![allow(static_mut_refs)]

use crate::usb_ffi::*;
use core::ptr::{read_volatile, write_volatile};
use core::sync::atomic::{fence, Ordering};

// ── EHCI Hardware Data Structures (Phase 2) ────────────────────────────────

/// Queue Head: 48 bytes, aligned to 64 bytes per EHCI spec.
#[repr(C, align(64))]
#[derive(Clone, Copy)]
pub struct EhciQh {
    pub horizontal_link: u32,
    pub endpoint_chars: u32,
    pub endpoint_caps: u32,
    pub current_qtd: u32,
    pub next_qtd: u32,
    pub alt_qtd: u32,
    pub token: u32,
    pub buffer: [u32; 5],
}

/// Queue Element Transfer Descriptor: 32 bytes, aligned to 32 bytes.
#[repr(C, align(32))]
#[derive(Clone, Copy)]
pub struct EhciQtd {
    pub next_qtd: u32,
    pub alt_qtd: u32,
    pub token: u32,
    pub buffer: [u32; 5],
}

/// Isochronous Transfer Descriptor: 64 bytes, aligned to 32 bytes.
#[repr(C, align(32))]
#[derive(Clone, Copy)]
pub struct EhciItd {
    pub next_link: u32,
    pub transaction: [u32; 8],
    pub buffer_page: [u32; 7],
}

// ── Capability Register Offsets (relative to BAR0) ────────────────────────

#[allow(dead_code)]
const CAP_CAPLENGTH: usize = 0x00;
#[allow(dead_code)]
const CAP_HCIVERSION: usize = 0x02;
const CAP_HCSPARAMS: usize = 0x04;
const CAP_HCCPARAMS: usize = 0x08;

// ── Operational Register Offsets (relative to BAR0 + CAPLENGTH) ──────────

const OP_USBCMD: usize = 0x00;
const OP_USBSTS: usize = 0x04;
#[allow(dead_code)]
const OP_USBINTR: usize = 0x08;
#[allow(dead_code)]
const OP_FRINDEX: usize = 0x0C;
#[allow(dead_code)]
const OP_CTRLDSSEGMENT: usize = 0x10;
#[allow(dead_code)]
const OP_PERIODICLISTBASE: usize = 0x14;
#[allow(dead_code)]
const OP_ASYNCLISTADDR: usize = 0x18;
#[allow(dead_code)]
const OP_CONFIGFLAG: usize = 0x40;
#[allow(dead_code)]
const OP_PORTSC: usize = 0x44;

// ── USBCMD bit definitions ────────────────────────────────────────────────

#[allow(dead_code)]
const USBCMD_RS: u32 = 1 << 0;
const USBCMD_HCRESET: u32 = 1 << 1;
#[allow(dead_code)]
const USBCMD_FLSIZE: u32 = 3 << 2;
#[allow(dead_code)]
const USBCMD_IAAD: u32 = 1 << 6;
#[allow(dead_code)]
const USBCMD_ASP: u32 = 3 << 8;
#[allow(dead_code)]
const USBCMD_ASPE: u32 = 1 << 11;

// ── USBSTS bit definitions ────────────────────────────────────────────────

#[allow(dead_code)]
const USBSTS_USBINT: u32 = 1 << 0;
#[allow(dead_code)]
const USBSTS_ERR: u32 = 1 << 1;
#[allow(dead_code)]
const USBSTS_PORT_CHANGE: u32 = 1 << 2;
#[allow(dead_code)]
const USBSTS_FRAME_ROLL: u32 = 1 << 3;
const USBSTS_HCHALTED: u32 = 1 << 12;
#[allow(dead_code)]
const USBSTS_RECLAMATION: u32 = 1 << 13;
#[allow(dead_code)]
const USBSTS_ASYNC_ADVANCE: u32 = 1 << 5;

// ── PORTSC bit definitions ────────────────────────────────────────────────

#[allow(dead_code)]
const PORTSC_CCS: u32 = 1 << 0;
#[allow(dead_code)]
const PORTSC_CSC: u32 = 1 << 1;
#[allow(dead_code)]
const PORTSC_PE: u32 = 1 << 2;
#[allow(dead_code)]
const PORTSC_EC: u32 = 1 << 3;
#[allow(dead_code)]
const PORTSC_OCA: u32 = 1 << 4;
#[allow(dead_code)]
const PORTSC_OCC: u32 = 1 << 5;
#[allow(dead_code)]
const PORTSC_RESET: u32 = 1 << 8;
#[allow(dead_code)]
const PORTSC_SUSPEND: u32 = 1 << 7;
#[allow(dead_code)]
const PORTSC_FPR: u32 = 1 << 9;
#[allow(dead_code)]
const PORTSC_PP: u32 = 1 << 12;
#[allow(dead_code)]
const PORTSC_WOC: u32 = 1 << 22;
const PORTSC_PO: u32 = 1 << 13;
#[allow(dead_code)]
const PORTSC_PTC: u32 = 0xF << 14;
#[allow(dead_code)]
const PORTSC_PSPD: u32 = 3 << 26;
#[allow(dead_code)]
const PORTSC_LINE_STATUS: u32 = 3 << 10;

// ── USBCMD additional bits ────────────────────────────────────────────────

#[allow(dead_code)]
const USBCMD_PSE: u32 = 1 << 4;
#[allow(dead_code)]
const USBCMD_ASE: u32 = 1 << 5;

// ── QH endpoint characteristics ───────────────────────────────────────────

const QH_HEAD: u32 = 1 << 15;
const QH_DT_CONTROL: u32 = 1 << 14;
#[allow(dead_code)]
const QH_EPS_HIGH: u32 = 2 << 12;
#[allow(dead_code)]
const QH_EPS_FULL: u32 = 0 << 12;
#[allow(dead_code)]
const QH_EPS_LOW: u32 = 1 << 12;

// ── QH horizontal link type bits ──────────────────────────────────────────

const QH_LINK_TYPE_QH: u32 = 0x02;
#[allow(dead_code)]
const QH_LINK_TYPE_ITD: u32 = 0x00;
#[allow(dead_code)]
const QH_LINK_TERMINATE: u32 = 0x01;

// ── qTD token definitions ─────────────────────────────────────────────────

#[allow(dead_code)]
const QTD_ACTIVE: u32 = 1 << 7;
#[allow(dead_code)]
const QTD_HALTED: u32 = 1 << 6;
#[allow(dead_code)]
const QTD_BABBLE: u32 = 1 << 4;
#[allow(dead_code)]
const QTD_XACTERR: u32 = 1 << 3;
const QTD_IOC: u32 = 1 << 15;
const QTD_PID_SETUP: u32 = 2 << 8;
const QTD_PID_IN: u32 = 1 << 8;
#[allow(dead_code)]
const QTD_PID_OUT: u32 = 0 << 8;
const QTD_CERR: u32 = 3 << 10;
const QTD_TOGGLE_0: u32 = 0 << 31;
const QTD_TOGGLE_1: u32 = 1 << 31;
#[allow(dead_code)]
const QTD_TOGGLE_MASK: u32 = 1 << 31;

// ── iTD transaction definitions ───────────────────────────────────────────

#[allow(dead_code)]
const ITD_IOC: u32 = 1 << 31;
#[allow(dead_code)]
const ITD_ACTIVE: u32 = 1 << 28;
#[allow(dead_code)]
const ITD_STATUS_MASK: u32 = 0xF << 28;

// ── Frame list ────────────────────────────────────────────────────────────

const FRAME_LIST_COUNT: usize = 1024;
#[allow(dead_code)]
const FRAME_LIST_TERMINATE: u32 = 1;

// ── Port state tracking for poll_fn reset state machine ──────────────────

#[allow(dead_code)]
const PORT_RESET_POLLS: u32 = 50;

// ── HCSPARAMS bit extractions ─────────────────────────────────────────────

const fn hcsparams_n_ports(v: u32) -> u8 {
    (v & 0x0F) as u8
}

// ── HCCPARAMS bit definitions ─────────────────────────────────────────────

const fn hccparams_eecp(v: u32) -> u8 {
    ((v >> 8) & 0xFF) as u8
}

// ── Legacy Support Capability IDs (PCI config space) ────────────────────

const LEGACY_CAP_ID: u8 = 0x01;
const LEGSUP_BIOS_OWNED: u32 = 1 << 16;
const LEGSUP_OS_OWNED: u32 = 1 << 24;
const LEGSUP_OS_REQ: u32 = 1 << 24;

// ── EHCI Private Controller State ─────────────────────────────────────────

#[repr(C)]
struct EhciData {
    mmio_base: *mut u8,
    op_regs: *mut u32,
    cap_len: u8,
    num_ports: u8,
    bus: u8,
    slot: u8,
    func: u8,
    hcd: UsbHcd,
    frame_list: *mut u32,
    frame_list_phys: u32,
    async_head: *mut EhciQh,
    async_head_phys: u32,
    // Port reset state machine
    port_reset_timer: [u32; 8],
    // Phase 6 isochronous state
    isoch_itds: [*mut EhciItd; 8],
    isoch_buf: *mut u8,
    isoch_buf_phys: u32,
    isoch_buf_len: u16,
    isoch_endpoint: u8,
    isoch_callback: Option<unsafe extern "C" fn(*mut u8, i32)>,
    isoch_active: bool,
    isoch_frame: u32,
}

static mut EHCI_STATE: Option<EhciData> = None;

// ── MMIO Access Helpers ───────────────────────────────────────────────────

#[inline(always)]
unsafe fn op_read(base: *mut u32, reg: usize) -> u32 {
    read_volatile(base.add(reg / 4))
}

#[inline(always)]
unsafe fn op_write(base: *mut u32, reg: usize, val: u32) {
    write_volatile(base.add(reg / 4), val)
}

#[inline(always)]
unsafe fn cap_read(base: *mut u8, offset: usize) -> u32 {
    read_volatile(base.add(offset) as *const u32)
}

// ── Serial Output Helpers ─────────────────────────────────────────────────

unsafe fn serial_str(s: &[u8]) {
    print_serial(s.as_ptr());
}

unsafe fn serial_hex16(val: u16) {
    let mut buf = [0u8; 8];
    buf[0] = b'0';
    buf[1] = b'x';
    for i in 0..4 {
        let digit = ((val >> (12 - i * 4)) & 0xF) as u8;
        buf[i + 2] = if digit < 10 { b'0' + digit } else { b'a' + digit - 10 };
    }
    buf[6] = b'\n';
    buf[7] = 0;
    print_serial(buf.as_ptr());
}

unsafe fn serial_dec(val: u32) {
    let mut buf = [0u8; 14];
    let mut i = 10usize;
    let mut v = val;
    if v == 0 {
        buf[10] = b'0';
        i = 9;
    } else {
        while v > 0 && i > 0 {
            i -= 1;
            buf[i] = b'0' + (v % 10) as u8;
            v /= 10;
        }
    }
    buf[11] = b'\n';
    buf[12] = 0;
    // i is now the index of the first digit (or 9 if val was 0)
    let start = i.min(10);
    print_serial(buf[start..].as_ptr());
}

unsafe fn serial_hex32(val: u32) {
    let mut buf = [0u8; 12];
    buf[0] = b'0';
    buf[1] = b'x';
    for i in 0..8 {
        let digit = ((val >> (28 - i * 4)) & 0xF) as u8;
        buf[i + 2] = if digit < 10 { b'0' + digit } else { b'a' + digit - 10 };
    }
    buf[10] = b'\n';
    buf[11] = 0;
    print_serial(buf.as_ptr());
}

// ── BIOS Handoff ──────────────────────────────────────────────────────────

unsafe fn ehci_bios_handoff(bus: u8, slot: u8, func: u8, mmio_base: *mut u8) -> bool {
    let hccparams = cap_read(mmio_base, CAP_HCCPARAMS);
    if hccparams_eecp(hccparams) == 0 {
        serial_str(b"EHCI: No EECP, skipping BIOS handoff\n\0");
        return true;
    }

    let eecp = hccparams_eecp(hccparams);
    if eecp == 0 {
        serial_str(b"EHCI: EECP present but pointer is 0\n\0");
        return true;
    }

    serial_str(b"EHCI: EECP=0x\0");
    serial_hex16(eecp as u16);

    let mut cap_ptr = eecp;
    while cap_ptr != 0 {
        let cap_id = pci_config_read_byte(bus, slot, func, cap_ptr);
        if cap_id == LEGACY_CAP_ID {
            // USB Legacy Support found
            let legsup_offs = cap_ptr as u8;
            let legsup = pci_config_read_dword(bus, slot, func, legsup_offs);

            serial_str(b"EHCI: USBLEGSUP=0x\0");
            serial_hex32(legsup);

            if (legsup & LEGSUP_BIOS_OWNED) != 0 {
                serial_str(b"EHCI: BIOS owns controller, requesting ownership...\n\0");

                // Request OS ownership: set bit 24, clear bit 0
                pci_config_write_dword(
                    bus,
                    slot,
                    func,
                    legsup_offs,
                    (legsup & !LEGSUP_BIOS_OWNED) | LEGSUP_OS_REQ,
                );

                let mut timeout = 1000000u32;
                while timeout > 0 {
                    let cur = pci_config_read_dword(bus, slot, func, legsup_offs);
                    if (cur & LEGSUP_OS_OWNED) != 0 {
                        serial_str(b"EHCI: BIOS handoff successful\n\0");
                        break;
                    }
                    timeout -= 1;
                }

                if timeout == 0 {
                    serial_str(b"EHCI: BIOS handoff timeout, forcing ownership\n\0");
                    // Force ownership by writing
                    pci_config_write_dword(
                        bus,
                        slot,
                        func,
                        legsup_offs,
                        LEGSUP_OS_OWNED,
                    );
                }
            } else {
                serial_str(b"EHCI: No BIOS ownership conflict\n\0");
            }
            return true;
        }

        // Read next capability pointer
        let next = pci_config_read_byte(bus, slot, func, cap_ptr + 1);
        if next == cap_ptr {
            break; // avoid infinite loop
        }
        cap_ptr = next;
    }

    serial_str(b"EHCI: Legacy Support capability not found\n\0");
    true
}

// ── Controller Reset ──────────────────────────────────────────────────────

unsafe fn ehci_reset(op_regs: *mut u32) -> bool {
    let usbcmd = op_read(op_regs, OP_USBCMD);
    if (usbcmd & USBCMD_HCRESET) != 0 {
        serial_str(b"EHCI: Controller already in reset\n\0");
    }

    op_write(op_regs, OP_USBCMD, USBCMD_HCRESET);

    let mut timeout = 100000u32;
    while timeout > 0 {
        let val = op_read(op_regs, OP_USBCMD);
        if (val & USBCMD_HCRESET) == 0 {
            serial_str(b"EHCI: Reset complete\n\0");
            return true;
        }
        timeout -= 1;
    }

    serial_str(b"EHCI: Reset timeout!\n\0");
    false
}

// ── Port Count ────────────────────────────────────────────────────────────

unsafe fn ehci_port_count(mmio_base: *mut u8) -> u8 {
    let hcsparams = cap_read(mmio_base, CAP_HCSPARAMS);
    let n = hcsparams_n_ports(hcsparams);
    serial_str(b"EHCI: HCSPARAMS=0x\0");
    serial_hex32(hcsparams);
    serial_str(b"EHCI: N_PORTS=\0");
    serial_dec(n as u32);
    n
}

// ── Public Initialization Entry Point (Phase 3) ───────────────────────────

#[no_mangle]
pub unsafe extern "C" fn ehci_init(bar0: usize, bus: u8, slot: u8, func: u8) {
    serial_str(b"\nEHCI: init(bar0=0x\0");
    serial_hex32(bar0 as u32);
    serial_str(b", bus=\0");
    serial_dec(bus as u32);
    serial_str(b", slot=\0");
    serial_dec(slot as u32);
    serial_str(b", func=\0");
    serial_dec(func as u32);
    serial_str(b")\n\0");

    // 1. Map MMIO base address
    let mmio_base = bar0 as *mut u8;

    // 2. Read CAPLENGTH (byte at offset 0)
    let cap_len = read_volatile(mmio_base);
    serial_str(b"EHCI: CAPLENGTH=\0");
    serial_dec(cap_len as u32);

    // 3. Read HCIVERSION (2 bytes at offset 2)
    let hci_version = read_volatile(mmio_base.add(CAP_HCIVERSION) as *const u16);
    let major = (hci_version >> 8) as u8;
    let minor = (hci_version & 0xFF) as u8;
    serial_str(b"EHCI: HCIVERSION=\0");
    serial_dec(major as u32);
    serial_str(b".\0");
    serial_dec(minor as u32);
    serial_str(b"\n\0");

    // 4. Get operational registers base
    let op_regs = mmio_base.add(cap_len as usize) as *mut u32;

    // 5. Read port count
    let _num_ports = ehci_port_count(mmio_base);

    // 6. BIOS handoff
    ehci_bios_handoff(bus, slot, func, mmio_base);

    // 7. Reset the controller
    ehci_reset(op_regs);

    let usbsts = op_read(op_regs, OP_USBSTS);
    serial_str(b"EHCI: USBSTS after reset=0x\0");
    serial_hex32(usbsts);

    if (usbsts & USBSTS_HCHALTED) != 0 {
        serial_str(b"EHCI: Controller is halted (expected)\n\0");
    }

    // 8. Allocate Periodic Frame List (4KB, 1024 entries * 4 bytes)
    let mut frame_list_phys = 0u32;
    let frame_list = kmalloc_ap(FRAME_LIST_COUNT * 4, &mut frame_list_phys) as *mut u32;
    if frame_list.is_null() {
        serial_str(b"EHCI: FAILED to allocate frame list!\n\0");
        return;
    }
    serial_str(b"EHCI: Frame list allocated, phys=0x\0");
    serial_hex32(frame_list_phys);

    // 9. Allocate Async Schedule Head QH (64-byte aligned via kmalloc_ap)
    let mut async_head_phys = 0u32;
    let async_head = kmalloc_ap(core::mem::size_of::<EhciQh>(), &mut async_head_phys) as *mut EhciQh;
    if async_head.is_null() {
        serial_str(b"EHCI: FAILED to allocate async head QH!\n\0");
        return;
    }
    serial_str(b"EHCI: Async head allocated, phys=0x\0");
    serial_hex32(async_head_phys);

    // 10. Initialize frame list: terminate all entries
    for i in 0..FRAME_LIST_COUNT {
        write_volatile(frame_list.add(i), FRAME_LIST_TERMINATE);
    }
    fence(Ordering::Release);

    // 11. Initialize Async Head QH
    (*async_head).horizontal_link = async_head_phys | QH_LINK_TYPE_QH; // points to itself, type=QH
    (*async_head).endpoint_chars = QH_HEAD | QH_EPS_HIGH | (64u32 << 16); // H=1, HS, MPS=64
    (*async_head).endpoint_caps = 0;
    (*async_head).current_qtd = 0;
    (*async_head).next_qtd = 1; // terminate (T=1)
    (*async_head).alt_qtd = 0;
    (*async_head).token = 0;
    (*async_head).buffer = [0u32; 5];
    fence(Ordering::Release);

    // 12. Set PERIODICLISTBASE
    op_write(op_regs, OP_PERIODICLISTBASE, frame_list_phys);

    // 13. Set ASYNCLISTADDR
    op_write(op_regs, OP_ASYNCLISTADDR, async_head_phys);

    // 14. Set CONFIGFLAG to route all ports to EHCI
    op_write(op_regs, OP_CONFIGFLAG, 1);
    fence(Ordering::Release);

    // 15. Start the controller
    let usbcmd = op_read(op_regs, OP_USBCMD);
    op_write(
        op_regs,
        OP_USBCMD,
        usbcmd | USBCMD_RS | USBCMD_PSE | USBCMD_ASE,
    );
    fence(Ordering::Release);

    serial_str(b"EHCI: Controller started, ConfigFlag=1\n\0");

    // 16. Verify controller is running
    let sts = op_read(op_regs, OP_USBSTS);
    if (sts & USBSTS_HCHALTED) == 0 {
        serial_str(b"EHCI: Controller is RUNNING\n\0");
    }

    // 17. Build EhciData state
    let num_ports = ehci_port_count(mmio_base);
    let name_ptr: *const u8 = b"EHCI\0".as_ptr();
    let new_state = EhciData {
        mmio_base,
        op_regs,
        cap_len,
        num_ports,
        bus,
        slot,
        func,
        hcd: UsbHcd {
            name: name_ptr,
            data: core::ptr::null_mut(),
            control_transfer: Some(ehci_control_transfer),
            start_interrupt_in: None,
            start_isochronous_in: Some(ehci_start_isochronous_in),
            poll: Some(ehci_poll_fn),
            reset_port: None,
        },
        frame_list,
        frame_list_phys,
        async_head,
        async_head_phys,
        port_reset_timer: [0u32; 8],
        isoch_itds: [core::ptr::null_mut(); 8],
        isoch_buf: core::ptr::null_mut(),
        isoch_buf_phys: 0,
        isoch_buf_len: 0,
        isoch_endpoint: 0,
        isoch_callback: None,
        isoch_active: false,
        isoch_frame: 0,
    };

    // 18. Move to static and fix self-pointer
    EHCI_STATE = Some(new_state);
    let state_ptr = EHCI_STATE.as_mut().unwrap() as *mut EhciData;
    (*state_ptr).hcd.data = state_ptr as *mut core::ffi::c_void;

    // 19. Register with USB core
    // Safety: hcd.data points to the static EhciData which lives forever
    usb_register_hcd(&mut (*state_ptr).hcd as *mut UsbHcd);

    serial_str(b"EHCI: Registered with USB core\n\n\0");
}

// ── Phase 4: Port Management & Device Connection ──────────────────────────

unsafe fn ehci_get_state_from_hcd(hcd: *mut UsbHcd) -> &'static mut EhciData {
    let data_ptr = (*hcd).data as *mut EhciData;
    &mut *data_ptr
}

unsafe extern "C" fn ehci_poll_fn(hcd: *mut UsbHcd) {
    let state = ehci_get_state_from_hcd(hcd);
    fence(Ordering::Acquire);

    // 1. Iterate through all PORTSC registers
    let n_ports = state.num_ports.min(8);
    for port in 0..n_ports {
        let portsc_offs = OP_PORTSC + (port as usize) * 4;
        let portsc = op_read(state.op_regs, portsc_offs);

        // 2. Handle port reset state machine
        let timer = state.port_reset_timer[port as usize];
        if timer > 0 {
            state.port_reset_timer[port as usize] = timer - 1;
            if timer == 1 {
                // Reset complete: clear RESET bit
                let cur = op_read(state.op_regs, portsc_offs);
                op_write(state.op_regs, portsc_offs, cur & !PORTSC_RESET);
                fence(Ordering::Release);

                // Read back to check line status and speed
                let after = op_read(state.op_regs, portsc_offs);

                // Check port speed bits (PSPD bits 27:26)
                let speed_bits = (after >> 26) & 3;
                let is_high_speed = speed_bits == 2;

                if is_high_speed && (after & PORTSC_CCS) != 0 {
                    serial_str(b"EHCI: Port \0");
                    serial_dec(port as u32);
                    serial_str(b" High Speed device connected\n\0");
                    usb_new_device(hcd, port, 480);
                } else {
                    serial_str(b"EHCI: Port \0");
                    serial_dec(port as u32);
                    serial_str(b" Full/Low Speed - handoff to companion\n\0");
                    // Set Port Owner bit to hand off to companion controller
                    let cur = op_read(state.op_regs, portsc_offs);
                    op_write(state.op_regs, portsc_offs, cur | PORTSC_PO);
                }
            }
        }

        // 3. Connection Status Change
        if (portsc & PORTSC_CSC) != 0 && timer == 0 {
            serial_str(b"EHCI: Port \0");
            serial_dec(port as u32);
            serial_str(b" CSC set\n\0");

            // Clear CSC by writing 1 to the CSC bit
            op_write(state.op_regs, portsc_offs, portsc | PORTSC_CSC);
            fence(Ordering::Release);

            if (portsc & PORTSC_CCS) != 0 {
                // Device connected: start port reset
                serial_str(b"EHCI: Port \0");
                serial_dec(port as u32);
                serial_str(b" resetting...\n\0");

                let cur = op_read(state.op_regs, portsc_offs);
                op_write(state.op_regs, portsc_offs, cur | PORTSC_RESET);
                fence(Ordering::Release);
                state.port_reset_timer[port as usize] = PORT_RESET_POLLS;
            } else {
                serial_str(b"EHCI: Port \0");
                serial_dec(port as u32);
                serial_str(b" disconnected\n\0");
            }
        }

        // 4. Over-current change
        if (portsc & PORTSC_OCC) != 0 {
            op_write(state.op_regs, portsc_offs, portsc | PORTSC_OCC);
        }
    }

    // 5. Poll active iTDs for completed isochronous frames
    if state.isoch_active {
        for itd_idx in 0..8usize {
            let itd = state.isoch_itds[itd_idx];
            if itd.is_null() {
                continue;
            }
            // Check all 8 transactions for completion
            let mut all_done = true;
            for txn in 0..8usize {
                let tr = read_volatile(&(*itd).transaction[txn]);
                // Transaction length (bits 27:16) will be non-zero if configured
                let txn_len = (tr >> 16) & 0xFFF;
                if txn_len == 0 {
                    continue; // unconfigured transaction slot
                }
                // Check Active bit (bit 28 of iTD transaction record)
                if (tr & ITD_ACTIVE) != 0 {
                    all_done = false;
                    break;
                }
            }
            if all_done {
                // All transactions in this iTD completed
                if let Some(cb) = state.isoch_callback {
                    let dma_buf = state.isoch_buf;
                    if !dma_buf.is_null() && state.isoch_buf_len > 0 {
                        cb(dma_buf, state.isoch_buf_len as i32);
                    }
                }
            }
        }
    }
}

// ── Phase 5: Control Transfers ────────────────────────────────────────────

unsafe extern "C" fn ehci_control_transfer(
    dev: *mut UsbDevice,
    setup: *mut UsbSetupPacket,
    buffer: *mut core::ffi::c_void,
    length: u16,
) -> i32 {
    let state = EHCI_STATE.as_mut().unwrap();
    let dev_addr = (*dev).address;
    let dir_in = ((*setup).request_type & 0x80) != 0;

    serial_str(b"EHCI: control_transfer dev=\0");
    serial_dec(dev_addr as u32);
    serial_str(b" len=\0");
    serial_dec(length as u32);
    serial_str(b"\n\0");

    // 1. Allocate DMA buffer for setup packet (and data if needed)
    let dma_size = 8 + length as usize;
    let mut dma_phys = 0u32;
    let dma_buf = kmalloc_ap(dma_size, &mut dma_phys) as *mut u8;
    if dma_buf.is_null() {
        return -1;
    }
    // Copy setup packet into DMA buffer
    core::ptr::copy_nonoverlapping(
        setup as *const u8,
        dma_buf,
        8,
    );

    // 2. Allocate 3 qTDs (Setup, Data, Status) with 32-byte alignment
    let mut setup_qtd_phys = 0u32;
    let setup_qtd = kmalloc_ap(core::mem::size_of::<EhciQtd>(), &mut setup_qtd_phys) as *mut EhciQtd;

    let mut data_qtd_phys = 0u32;
    let data_qtd = kmalloc_ap(core::mem::size_of::<EhciQtd>(), &mut data_qtd_phys) as *mut EhciQtd;

    let mut status_qtd_phys = 0u32;
    let status_qtd = kmalloc_ap(core::mem::size_of::<EhciQtd>(), &mut status_qtd_phys) as *mut EhciQtd;

    if setup_qtd.is_null() || data_qtd.is_null() || status_qtd.is_null() {
        serial_str(b"EHCI: ctrl: failed to allocate qTDs\n\0");
        return -1;
    }

    // 3. Allocate a device QH for endpoint 0 (64-byte aligned)
    let mut qh_phys = 0u32;
    let qh = kmalloc_ap(core::mem::size_of::<EhciQh>(), &mut qh_phys) as *mut EhciQh;
    if qh.is_null() {
        serial_str(b"EHCI: ctrl: failed to allocate QH\n\0");
        return -1;
    }

    // 4. Configure Setup qTD (8 bytes, PID=SETUP, toggle=0)
    (*setup_qtd).next_qtd = data_qtd_phys;
    (*setup_qtd).alt_qtd = 0;
    (*setup_qtd).token = QTD_ACTIVE | QTD_PID_SETUP | QTD_CERR | QTD_TOGGLE_0 | (8 << 16);
    (*setup_qtd).buffer = [0u32; 5];
    (*setup_qtd).buffer[0] = dma_phys;

    // 5. Configure Data qTD
    if length > 0 {
        let pid = if dir_in { QTD_PID_IN } else { QTD_PID_OUT };
        let toggle = QTD_TOGGLE_1;
        // Copy data to/from DMA buffer (OUT: copy to DMA, IN: we'll copy back after)
        if !dir_in {
            // OUT: copy data into DMA buffer after setup packet
            core::ptr::copy_nonoverlapping(
                buffer as *const u8,
                dma_buf.add(8),
                length as usize,
            );
        }
        (*data_qtd).next_qtd = status_qtd_phys;
        (*data_qtd).alt_qtd = 0;
        (*data_qtd).token = QTD_ACTIVE | pid | QTD_CERR | toggle | ((length as u32) << 16);
        (*data_qtd).buffer = [0u32; 5];
        (*data_qtd).buffer[0] = dma_phys.wrapping_add(8);
    } else {
        // No data: status qTD follows setup directly
        (*setup_qtd).next_qtd = status_qtd_phys;
    }

    // 6. Configure Status qTD (0 bytes, direction opposite of data phase)
    let status_pid = if length == 0 || dir_in {
        QTD_PID_OUT
    } else {
        QTD_PID_IN
    };
    // If there was a data phase, toggle is 1 for status (data toggle sequence)
    // If no data, toggle is 1 (control transfer: setup=0, status=1)
    // For control transfers: Setup toggle=0, Data toggle=1, Status toggle=1
    // Actually: Setup=0, Data=1, Status=1. The DT bit in QH handles auto-toggle.
    // With DT=1 in QH, the controller auto-toggles, so we just set toggle=0 in qTD
    // and let the QH handle it.
    (*status_qtd).next_qtd = 1; // terminate
    (*status_qtd).alt_qtd = 0;
    (*status_qtd).token = QTD_ACTIVE | status_pid | QTD_CERR | QTD_TOGGLE_1 | (0 << 16) | QTD_IOC;
    (*status_qtd).buffer = [0u32; 5];

    fence(Ordering::Release);

    // 7. Set up QH for endpoint 0
    let eps = QH_EPS_HIGH;
    let endpoint_chars = (dev_addr as u32 & 0x7F)
        | ((0u32) << 8)     // endpoint 0
        | QH_DT_CONTROL     // auto data toggle
        | eps
        | (64u32 << 16);    // max packet size = 64 for HS EP0
    (*qh).horizontal_link = state.async_head_phys | QH_LINK_TYPE_QH; // point back to head QH, type=QH
    (*qh).endpoint_chars = endpoint_chars;
    (*qh).endpoint_caps = 0;
    (*qh).current_qtd = 0;
    (*qh).next_qtd = setup_qtd_phys; // first qTD
    (*qh).alt_qtd = 0;
    (*qh).token = 0;
    (*qh).buffer = [0u32; 5];
    fence(Ordering::Release);

    // 8. Insert QH into async schedule after head
    // Save head's current link, then chain: head -> new_qh -> (head's old link)
    let head_old_link = (*state.async_head).horizontal_link;
    (*state.async_head).horizontal_link = qh_phys | QH_LINK_TYPE_QH;
    (*qh).horizontal_link = head_old_link; // already has type bits from head's original link
    fence(Ordering::Release);

    // 9. Poll for completion (poll Active bit in status qTD)
    let mut timeout = 50000u32;
    while timeout > 0 {
        fence(Ordering::Acquire);
        let token = read_volatile(&(*status_qtd).token);
        if (token & QTD_ACTIVE) == 0 {
            // Check for errors
            if (token & (QTD_HALTED | QTD_BABBLE | QTD_XACTERR)) != 0 {
                serial_str(b"EHCI: ctrl: qTD error token=0x\0");
                serial_hex32(token);
                // Remove QH from schedule and ring doorbell
                (*state.async_head).horizontal_link = head_old_link;
                fence(Ordering::Release);
                // Ring async advance doorbell to safely reclaim QH
                let cmd = op_read(state.op_regs, OP_USBCMD);
                op_write(state.op_regs, OP_USBCMD, cmd | USBCMD_IAAD);
                return -1;
            }
            // Success
            if dir_in && length > 0 {
                // Copy data from DMA buffer back to caller
                core::ptr::copy_nonoverlapping(
                    dma_buf.add(8),
                    buffer as *mut u8,
                    length as usize,
                );
            }
            serial_str(b"EHCI: ctrl: success\n\0");
            // Remove QH from async schedule and ring doorbell
            (*state.async_head).horizontal_link = head_old_link;
            fence(Ordering::Release);
            // Ring async advance doorbell to safely reclaim QH
            let cmd = op_read(state.op_regs, OP_USBCMD);
            op_write(state.op_regs, OP_USBCMD, cmd | USBCMD_IAAD);
            return 0;
        }
        timeout -= 1;
    }

    serial_str(b"EHCI: ctrl: timeout\n\0");
    // Remove QH on timeout and ring doorbell
    (*state.async_head).horizontal_link = head_old_link;
    fence(Ordering::Release);
    let cmd = op_read(state.op_regs, OP_USBCMD);
    op_write(state.op_regs, OP_USBCMD, cmd | USBCMD_IAAD);
    -1
}

// ── Phase 6: Isochronous Transfers ────────────────────────────────────────

unsafe extern "C" fn ehci_start_isochronous_in(
    dev: *mut UsbDevice,
    endpoint: u8,
    _buffer: *mut core::ffi::c_void,
    length: u16,
    callback: Option<unsafe extern "C" fn(*mut u8, i32)>,
) -> i32 {
    let state = EHCI_STATE.as_mut().unwrap();
    if state.isoch_active {
        serial_str(b"EHCI: isoch already active\n\0");
        return -1;
    }

    serial_str(b"EHCI: isoch_in dev=\0");
    serial_dec((*dev).address as u32);
    serial_str(b" ep=\0");
    serial_dec(endpoint as u32);
    serial_str(b" len=\0");
    serial_dec(length as u32);
    serial_str(b"\n\0");

    // 1. Determine number of iTDs needed
    // Each iTD covers 8 micro-frames (= 1 frame)
    // Each transaction can hold up to (typically 3072 bytes but depends on endpoint)
    // For webcam isochronous: typical max packet = 3072 bytes, so 1 iTD per frame
    // But we need to cover multiple frames. Use up to 8 iTDs for a pool.
    let num_itds = 8usize;

    // 2. Allocate periodic DMA buffer (multiple of page size for alignment)
    // We need enough space for all iTD transactions to point into
    let mut buf_phys = 0u32;
    let buf_size = (length as usize + 0xFFF) & !0xFFF;
    let dma_buf = kmalloc_ap(buf_size, &mut buf_phys) as *mut u8;
    if dma_buf.is_null() {
        serial_str(b"EHCI: isoch: failed to alloc DMA buffer\n\0");
        return -1;
    }

    // 3. Allocate iTDs
    let mut itds = [core::ptr::null_mut::<EhciItd>(); 8];
    let mut itd_phys_addrs = [0u32; 8];
    for i in 0..num_itds {
        let mut phys = 0u32;
        let itd = kmalloc_ap(core::mem::size_of::<EhciItd>(), &mut phys) as *mut EhciItd;
        if itd.is_null() {
            serial_str(b"EHCI: isoch: failed to alloc iTD\n\0");
            return -1;
        }
        itds[i] = itd;
        itd_phys_addrs[i] = phys;
    }

    // 4. Configure each iTD
    // Per EHCI spec: iTD transaction record layout:
    //   Bits 31    = IOC (Interrupt on Complete)
    //   Bits 30:28 = Status (bit 28 = Active)
    //   Bits 27:16 = Transaction Length (12 bits)
    //   Bits 15:12 = Page Select (PG)
    //   Bits 11:0  = Transaction Offset within page
    let dev_addr = (*dev).address;
    let bytes_per_transaction = length as u32 / 8;
    // Clamp to max 3072 per HS isochronous transaction
    let bytes_per_txn = if bytes_per_transaction > 3072 { 3072 } else { bytes_per_transaction };

    for i in 0..num_itds {
        let itd = itds[i];
        // Next link: point to next iTD, or terminate if last
        if i + 1 < num_itds {
            (*itd).next_link = itd_phys_addrs[i + 1]; // type=iTD (bits 1:0 = 00)
        } else {
            (*itd).next_link = FRAME_LIST_TERMINATE; // terminate
        }

        // Set up 8 transactions (one per micro-frame)
        for txn in 0..8usize {
            let txn_offset = (i * 8 + txn) as u32 * bytes_per_txn;
            if txn_offset + bytes_per_txn > length as u32 {
                // Past end: mark as inactive (no Active bit)
                (*itd).transaction[txn] = 0;
                continue;
            }
            let page = (txn_offset >> 12) as u32;
            let offs = txn_offset & 0xFFF;

            // Build transaction record per EHCI spec
            let mut tr: u32 = 0;
            // Active = bit 28 of the iTD transaction record (status field bits 30:28)
            tr |= ITD_ACTIVE;                              // Mark Active
            tr |= (bytes_per_txn & 0xFFF) << 16;           // Transaction Length (bits 27:16)
            tr |= (page & 7) << 12;                        // Page Select (bits 15:12)
            tr |= offs & 0xFFF;                            // Offset (bits 11:0)

            // Set IOC on last transaction of last iTD
            if i == num_itds - 1 && txn == 7 {
                tr |= ITD_IOC;
            }
            (*itd).transaction[txn] = tr;
        }

        // Set buffer page pointers with required endpoint/device info
        // buffer_page[0]: bits 6:0 = Device Address, bits 11:8 = Endpoint Number
        // buffer_page[1]: bits 10:0 = Max Packet Size, bit 11 = Direction (1=IN)
        // buffer_page[2]: bits 1:0 = Multi (transactions per micro-frame, typically 1)
        // All pages: bits 31:12 = Page physical address
        let page_base = buf_phys & !0xFFF;
        for pg in 0..7usize {
            let page_addr = page_base.wrapping_add((pg as u32) * 0x1000) & !0xFFF;
            let extra = match pg {
                0 => (dev_addr as u32 & 0x7F) | ((endpoint as u32 & 0xF) << 8),
                1 => (bytes_per_txn & 0x7FF) | (1u32 << 11), // MPS + Direction=IN
                2 => 1u32, // Multi=1 (one transaction per micro-frame)
                _ => 0,
            };
            (*itd).buffer_page[pg] = page_addr | extra;
        }
    }
    fence(Ordering::Release);

    // 5. Insert iTDs into the periodic frame list, spread across frames
    // Each iTD gets its own frame slot for better bandwidth distribution
    for i in 0..num_itds {
        let frame_idx = i % FRAME_LIST_COUNT;
        write_volatile(
            state.frame_list.add(frame_idx),
            itd_phys_addrs[i], // T=0, type=iTD (bits 1:0 = 00)
        );
    }
    // Last iTD terminates
    (*itds[num_itds - 1]).next_link = FRAME_LIST_TERMINATE;
    fence(Ordering::Release);

    // 6. Save state for poll_fn to use
    state.isoch_active = true;
    state.isoch_callback = callback;
    state.isoch_buf = dma_buf;
    state.isoch_buf_phys = buf_phys;
    state.isoch_buf_len = length;
    state.isoch_endpoint = endpoint;
    state.isoch_frame = 0; // Starting frame index
    for i in 0..num_itds {
        state.isoch_itds[i] = itds[i];
    }

    serial_str(b"EHCI: isochronous IN started\n\0");
    0
}
