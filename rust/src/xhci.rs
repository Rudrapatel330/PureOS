use xhci::accessor::Mapper;
use core::num::NonZeroUsize;

#[derive(Clone)]
struct MemoryMapper;

impl Mapper for MemoryMapper {
    unsafe fn map(&mut self, phys_base: usize, _bytes: usize) -> NonZeroUsize {
        // Our kernel identity-maps MMIO addresses (0xF0000000+).
        NonZeroUsize::new(phys_base).unwrap()
    }

    fn unmap(&mut self, _virt_base: usize, _bytes: usize) {
        // No-op for our kernel
    }
}

extern "C" {
    fn print_serial(s: *const u8);
    fn set_xhci_debug_msg(s: *const u8);
    fn get_timer_ticks() -> u32;
    fn kernel_heartbeat();
    fn kernel_poll_events();
}

// ---- PORTSC bits (spec 5.4.8) used by the round-2 port scan ----
const PORTSC_CCS: u32 = 1 << 0;
const PORTSC_PR: u32 = 1 << 4;
const PORTSC_PP: u32 = 1 << 9;
const PORTSC_PRC: u32 = 1 << 21;
const PORTSC_PLC: u32 = 1 << 22;
const PORTSC_CSC: u32 = 1 << 23;
const PORTSC_W1C_CHANGES: u32 = PORTSC_PRC | PORTSC_PLC | PORTSC_CSC;

static mut XHCI_PORTS_BASE: usize = 0;
static mut XHCI_MAX_PORTS: u32 = 0;

unsafe fn portsc_addr(port: u32) -> *mut u32 {
    (XHCI_PORTS_BASE + ((port as usize - 1) * 0x10)) as *mut u32
}

// Read TSC for wall-clock fallback when PIT ticks may not advance
#[inline(always)]
fn rdtsc() -> u64 {
    let lo: u32;
    let hi: u32;
    unsafe {
        core::arch::asm!("rdtsc", out("eax") lo, out("edx") hi, options(nomem, nostack));
    }
    ((hi as u64) << 32) | (lo as u64)
}

// Conservative TSC cycles per millisecond estimate for timeout fallback.
// Most x86-64 CPUs run >1 GHz, so 2M cycles/ms (~2 GHz) is a safe lower
// bound. The TSC path only fires when PIT ticks are frozen, so being
// conservative (shorter real-time) is fine — it just means timeouts fire
// slightly earlier than intended, which is better than hanging.
const TSC_CYCLES_PER_MS: u64 = 2_000_000;

// Wait ~n timer ticks (250 Hz, so n ticks = n*4 ms). Bounded by BOTH:
// 1. PIT tick count (primary, when interrupts work)
// 2. TSC wall-clock (fallback, when PIT is frozen)
// 3. Iteration spin guard (last resort)
fn wait_ticks(n: u32) {
    let ms_budget = (n as u64) * 4; // 250 Hz → 4ms per tick
    let tsc_budget = ms_budget * TSC_CYCLES_PER_MS;
    let tsc_start = rdtsc();
    unsafe {
        let start = get_timer_ticks();
        let mut guard = 0u32;
        while guard < 25_000_000 {
            guard += 1;
            let now = get_timer_ticks();
            if now.wrapping_sub(start) >= n {
                return;
            }
            // TSC fallback: if enough real wall-clock time has passed, bail out
            if rdtsc().wrapping_sub(tsc_start) >= tsc_budget {
                return;
            }
        }
    }
}

fn ticks_elapsed_since(start: u32) -> u32 {
    unsafe { get_timer_ticks().wrapping_sub(start) }
}

fn now_ticks() -> u32 {
    unsafe { get_timer_ticks() }
}

// TSC-based elapsed milliseconds for timeout checks that don't depend on PIT
fn tsc_ms_elapsed(tsc_start: u64) -> u64 {
    rdtsc().wrapping_sub(tsc_start) / TSC_CYCLES_PER_MS
}

// Wall-clock wait budgets (250 Hz tick). Real-HW MMIO reads run ~50x slower
// than QEMU, so iteration-count budgets (10M/100M spins) silently became
// minutes of frozen boot on the LOQ. Every wait below is capped in TIME.
const TICKS_1S: u32 = 250;
const TICKS_500MS: u32 = 125;

extern "C" {
    pub fn fast_fill(dest: *mut u32, color: u32, count: u32);
    pub static mut framebuffer: *mut u32;
    pub static screen_width: u16;
}

pub unsafe fn visual_debug(color: u32, row: u32) {
    let fb = core::ptr::read_volatile(&framebuffer);
    if !fb.is_null() {
        let sw = core::ptr::read_volatile(&screen_width) as u32;
        let offset = sw * (row * 6); // 6 rows spacing down from the top
        fast_fill(fb.offset(offset as isize), color, sw * 5); // 5 pixels thick
    }
}

#[no_mangle]
pub extern "C" fn rust_xhci_init(bar_ptr: usize) -> u32 {
    unsafe {
        visual_debug(0xFFFF0000, 1); // Red: Start of init
        print_serial(b"RUST xHCI: Initializing controller...\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Found controller, starting init...\0".as_ptr());
    }

    let mapper = MemoryMapper;

    // Registers::new does NOT return Result — it returns Self directly
    let mut registers = unsafe { xhci::Registers::new(bar_ptr, mapper) };

    let caplength = registers.capability.caplength.read_volatile().get();
    unsafe { visual_debug(0xFF00FF00, 2); } // Green: Capability read success
    if caplength == 0 || caplength == 0xFF {
        unsafe { print_serial(b"RUST xHCI: ABORT - Invalid caplength (MMIO read failed)!\n\0".as_ptr()); }
        return 1;
    }

    // BIOS Handoff (USB Legacy Support)
    unsafe {
        let hccparams1 = core::ptr::read_volatile((bar_ptr + 0x10) as *const u32);
        let mut xecp = (hccparams1 >> 16) & 0xFFFF;
        if xecp != 0 && xecp != 0xFFFF {
            let mut ptr = bar_ptr + (xecp as usize * 4);
            let mut safe_guard = 0;
            while xecp != 0 && safe_guard < 32 {
                safe_guard += 1;
                let cap = core::ptr::read_volatile(ptr as *const u32);
                let cap_id = cap & 0xFF;
                let next_ptr = (cap >> 8) & 0xFF;

                if cap == 0xFFFFFFFF {
                    print_serial(b"RUST xHCI: xecp read 0xFFFFFFFF - Aborting handoff!\n\0".as_ptr());
                    break;
                }

                if cap_id == 1 { // USB Legacy Support
                    print_serial(b"RUST xHCI: Found USB Legacy Support. Requesting OS ownership...\n\0".as_ptr());
                    set_xhci_debug_msg(b"xHCI: BIOS Handoff - Found Legacy Support...\0".as_ptr());
                    print_serial(b"RUST xHCI: Stealth Takeover: Clearing BIOS Owned Semaphore directly...\n\0".as_ptr());
                    set_xhci_debug_msg(b"xHCI: Stealth Takeover initialized...\0".as_ptr());
                    
                    // 1. Force clear BIOS Owned Semaphore (Bit 16), but DO NOT set OS Owned Semaphore (Bit 24)
                    core::ptr::write_volatile(ptr as *mut u32, cap & !(1 << 16));
                    
                    // 2. Wait ~50ms to let the SMM realize it has been disconnected gracefully
                    wait_ticks(13);
                    
                    // 3. Carefully disable all SMI enables in USBLEGCTLSTS (Offset +4)
                    let ctl_sts_ptr = ptr + 4;
                    core::ptr::write_volatile(ctl_sts_ptr as *mut u32, 0);
                    break;
                }

                if next_ptr == 0 { break; }
                ptr += next_ptr as usize * 4;
            }
        }
    }

    // Reset the controller
    unsafe {
        visual_debug(0xFF0000FF, 3); // Blue: After BIOS handoff, before reset
        print_serial(b"RUST xHCI: Waiting for controller to halt...\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Waiting for controller to halt...\0".as_ptr());
    }
    
    // Stop the controller first
    registers.operational.usbcmd.update_volatile(|w| {
        w.clear_run_stop();
    });

    // Wall-clock bounded USBSTS/USBCMD waits: a wedged controller can never
    // freeze boot before the lockscreen renders.
    let t_halt = now_ticks();
    let tsc_halt = rdtsc();
    while !registers.operational.usbsts.read_volatile().hc_halted() {
        if ticks_elapsed_since(t_halt) >= TICKS_500MS || tsc_ms_elapsed(tsc_halt) >= 800 {
            unsafe {
                print_serial(b"RUST xHCI: Controller did not halt (continuing)!\n\0".as_ptr());
                set_xhci_debug_msg(b"xHCI: Halt TIMEOUT (continuing)!\0".as_ptr());
            }
            break;
        }
    }

    unsafe {
        print_serial(b"RUST xHCI: Resetting controller...\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Resetting controller...\0".as_ptr());
    }
    registers.operational.usbcmd.update_volatile(|w| {
        w.set_host_controller_reset();
    });
    
    // Wait for reset to complete (wall-clock bounded)
    let t_rst = now_ticks();
    let tsc_rst = rdtsc();
    while registers.operational.usbcmd.read_volatile().host_controller_reset() {
        if ticks_elapsed_since(t_rst) >= TICKS_1S || tsc_ms_elapsed(tsc_rst) >= 1500 {
            unsafe {
                print_serial(b"RUST xHCI: Controller reset TIMEOUT!\n\0".as_ptr());
                set_xhci_debug_msg(b"xHCI: Reset TIMEOUT (continuing)!\0".as_ptr());
            }
            break;
        }
    }
    let t_cnr = now_ticks();
    let tsc_cnr = rdtsc();
    while registers.operational.usbsts.read_volatile().controller_not_ready() {
        if ticks_elapsed_since(t_cnr) >= TICKS_1S || tsc_ms_elapsed(tsc_cnr) >= 1500 {
            unsafe {
                print_serial(b"RUST xHCI: CNR never cleared (continuing)!\n\0".as_ptr());
                set_xhci_debug_msg(b"xHCI: CNR TIMEOUT (continuing)!\0".as_ptr());
            }
            break;
        }
    }

    unsafe {
        visual_debug(0xFFFFFF00, 4); // Yellow: Reset complete
        print_serial(b"RUST xHCI: Controller reset complete.\n\0".as_ptr());
    }

    // Set Max Device Slots Enabled
    let _caplength = registers.capability.caplength.read_volatile();
    let hcsparams1 = registers.capability.hcsparams1.read_volatile();
    let max_slots = hcsparams1.number_of_device_slots();
    let _max_ports = hcsparams1.number_of_ports();

    registers.operational.config.update_volatile(|w| {
        w.set_max_device_slots_enabled(max_slots);
    });

    // Allocate DCBAAP (Device Context Base Address Array Pointer)
    // Size = 256 * 8 = 2048 bytes (must be 64-byte aligned, kmalloc_ap gives 4096-byte alignment)
    let mut dcbaap_phys = 0u32;
    let dcbaap_ptr = crate::heap::kmalloc_ap(2048, &mut dcbaap_phys);
    if dcbaap_ptr.is_null() {
        unsafe { print_serial(b"RUST xHCI: Failed to allocate DCBAAP\n\0".as_ptr()); }
        return 1;
    }
    // Clear DCBAA
    unsafe { core::ptr::write_bytes(dcbaap_ptr, 0, 2048); }
    registers.operational.dcbaap.update_volatile(|w| {
        w.set(dcbaap_phys as u64);
    });

    // Allocate Command Ring (1 page = 4096 bytes)
    let mut cmd_ring_phys = 0u32;
    let cmd_ring_ptr = crate::heap::kmalloc_ap(4096, &mut cmd_ring_phys);
    if cmd_ring_ptr.is_null() {
        unsafe { print_serial(b"RUST xHCI: Failed to allocate Command Ring\n\0".as_ptr()); }
        return 1;
    }
    unsafe { core::ptr::write_bytes(cmd_ring_ptr, 0, 4096); }
    
    // Set CRCR (Command Ring Control Register)
    // Bit 0 is Ring Cycle State (RCS), set to 1 for initial consumer cycle state
    registers.operational.crcr.update_volatile(|w| {
        w.set_ring_cycle_state();
        w.set_command_ring_pointer(cmd_ring_phys as u64);
    });

    // Allocate Event Ring Segment Table (ERST) (1 entry = 16 bytes)
    let mut erst_phys = 0u32;
    let erst_ptr = crate::heap::kmalloc_ap(16, &mut erst_phys);
    let mut erst_seg_phys = 0u32;
    let erst_seg_ptr = crate::heap::kmalloc_ap(4096, &mut erst_seg_phys);
    if erst_ptr.is_null() || erst_seg_ptr.is_null() {
        unsafe { print_serial(b"RUST xHCI: Failed to allocate Event Ring\n\0".as_ptr()); }
        return 1;
    }
    unsafe { 
        core::ptr::write_bytes(erst_ptr, 0, 16); 
        core::ptr::write_bytes(erst_seg_ptr, 0, 4096); 
    }

    // Set ERST entry 0
    unsafe {
        let erst = erst_ptr as *mut u64;
        erst.write_volatile(erst_seg_phys as u64); // Ring Segment Base Address
        erst.add(1).write_volatile(256); // Ring Segment Size (number of TRBs: 4096/16 = 256)
    }

    // Configure Interrupter 0
    let mut interrupter = registers.interrupter_register_set.interrupter_mut(0);
    interrupter.erstsz.update_volatile(|w| {
        w.set(1);
    });
    interrupter.erstba.update_volatile(|w| {
        w.set(erst_phys as u64);
    });
    interrupter.erdp.update_volatile(|w| {
        w.set_event_ring_dequeue_pointer(erst_seg_phys as u64);
    });
    interrupter.iman.update_volatile(|w| {
        w.set_interrupt_enable();
    });

    // Store MMIO base and event ring segment pointer for the IRQ handler
    unsafe {
        XHCI_MMIO_BASE = bar_ptr;
        XHCI_EVENT_RING_BASE = erst_seg_ptr as u64;
        XHCI_EVENT_RING_PHYS = erst_seg_phys as u64;
        XHCI_ERDP_INDEX = 0;
        XHCI_EVENT_CYCLE = 1;
        XHCI_CAP_LENGTH = registers.capability.caplength.read_volatile().get() as usize;
        
        XHCI_CMD_RING_BASE = cmd_ring_ptr as u64;
        XHCI_CMD_RING_PHYS = cmd_ring_phys as u64;
        XHCI_CMD_INDEX = 0;
        XHCI_CMD_CYCLE = 1;
        XHCI_LAST_SLOT_ID = 0;
        XHCI_DCBAAP = dcbaap_ptr as u64;
    }

    // Start controller
    unsafe { 
        print_serial(b"RUST xHCI: Starting controller...\n\0".as_ptr()); 
        set_xhci_debug_msg(b"xHCI: Starting controller...\0".as_ptr());
    }
    registers.operational.usbcmd.update_volatile(|w| {
        w.set_interrupter_enable();
        w.set_run_stop();
    });
    
    let t_run = now_ticks();
    let tsc_run = rdtsc();
    while registers.operational.usbsts.read_volatile().hc_halted() {
        if ticks_elapsed_since(t_run) >= TICKS_500MS || tsc_ms_elapsed(tsc_run) >= 800 {
            unsafe {
                print_serial(b"RUST xHCI: Controller did not start (continuing)!\n\0".as_ptr());
                set_xhci_debug_msg(b"xHCI: Start TIMEOUT (continuing)!\0".as_ptr());
            }
            break;
        }
    }

    unsafe {
        print_serial(b"RUST xHCI: Controller running successfully!\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Controller running successfully!\0".as_ptr());
        XHCI_STAGE = 1;
    }

    // Read capability parameters for logging
    let caplength = registers.capability.caplength.read_volatile();
    let hcsparams1 = registers.capability.hcsparams1.read_volatile();
    let max_slots = hcsparams1.number_of_device_slots();
    let max_ports = hcsparams1.number_of_ports();

    unsafe {
        print_serial(b"RUST xHCI: Cap Length: \0".as_ptr());
        let mut buf = [0u8; 16];
        let n = caplength.get() as usize;
        let s = format_num(n, &mut buf);
        print_serial(s.as_ptr());
        print_serial(b"\n\0".as_ptr());

        print_serial(b"RUST xHCI: Max Device Slots: \0".as_ptr());
        let s = format_num(max_slots as usize, &mut buf);
        print_serial(s.as_ptr());
        print_serial(b"\n\0".as_ptr());

        print_serial(b"RUST xHCI: Max Ports: \0".as_ptr());
        let s = format_num(max_ports as usize, &mut buf);
        print_serial(s.as_ptr());
        print_serial(b"\n\0".as_ptr());

        print_serial(b"RUST xHCI: Controller detected successfully!\n\0".as_ptr());
    }

    // Phase 3: Port Initialization & Device Enumeration.
    // Round 4 (reduced): bounded short rescan. PP is forced once, then up to
    // 4 passes x ~128ms (~0.5s worst case). A TSC absolute timeout of 3s
    // backstops the entire scan even when PIT ticks are frozen.
    unsafe {
        XHCI_PORTS_BASE = bar_ptr + caplength.get() as usize + 0x400;
        XHCI_MAX_PORTS = max_ports as u32;

        power_ports_once();

        let mut connected_mask = 0u32;
        let mut pass = 0u32;
        let mut buf = [0u8; 16];
        let tsc_scan = rdtsc();
        while pass < 4 {
            connected_mask = scan_ports_connected();
            if connected_mask != 0 {
                break;
            }
            pass += 1;
            // Absolute TSC timeout for the entire port scan phase
            if tsc_ms_elapsed(tsc_scan) >= 3000 {
                print_serial(b"RUST xHCI: Port scan absolute timeout (3s)\n\0".as_ptr());
                break;
            }
            if pass % 2 == 0 {
                set_xhci_debug_msg(b"xHCI: Waiting for USB device...\0".as_ptr());
            }
            print_serial(b"RUST xHCI: No device yet (pass \0".as_ptr());
            print_serial(format_num(pass as usize, &mut buf).as_ptr());
            print_serial(b")\n\0".as_ptr());
            wait_ticks(32);
        }

        if connected_mask == 0 {
            set_xhci_debug_msg(b"xHCI: No USB device found on any port!\0".as_ptr());
            print_serial(b"RUST xHCI: No device found after rescan!\n\0".as_ptr());
        } else {
            set_xhci_debug_msg(b"xHCI: Device detected on root port!\0".as_ptr());
        }

        let mut p = 1u32;
        while p <= max_ports as u32 && p <= 32 {
            if connected_mask & (1 << (p - 1)) != 0 {
                try_enumerate_port(p);
            }
            p += 1;
        }
    }

    // Phase 4: Identify devices and configure HID keyboard
    unsafe {
        for i in 0..XHCI_NUM_DEVICES as usize {
            let dev = &mut XHCI_DEVICES[i];
            let slot_id = dev.slot_id;

            let mut buf = [0u8; 16];
            // Allocate a buffer for the device descriptor (18 bytes, but align to 64)
            let mut desc_buf_phys = 0u32;
            let desc_buf = crate::heap::kmalloc_ap(64, &mut desc_buf_phys) as *mut u8;
            core::ptr::write_bytes(desc_buf, 0, 64);

            // Send GET_DESCRIPTOR (Device) via control transfer on EP0
            // Setup Stage TRB: bmRequestType=0x80 (IN), bRequest=6 (GET_DESCRIPTOR),
            // wValue=0x0100 (Device Desc), wIndex=0, wLength=18
            let setup_lo: u32 = 0x80 | (6 << 8) | (0x0100 << 16); // bmRequestType | bRequest | wValue
            let setup_hi: u32 = 0 | (18 << 16); // wIndex | wLength

            // Push Setup Stage TRB (Type 2) on EP0 ring
            // Dword 2: TRB Transfer Length = 8, Dword 3: Type=2, TRT=3 (IN Data Stage), IDT=1, IOC=0
            push_ep0_trb(dev, setup_lo, setup_hi, 8, (2 << 10) | (3 << 16) | (1 << 6));

            // Push Data Stage TRB (Type 3) on EP0 ring
            // Points to desc_buf, Transfer Length = 18, Direction = IN (bit 16 of dword3 = 1)
            push_ep0_trb(dev, desc_buf_phys as u32, (desc_buf_phys as u64 >> 32) as u32,
                         18, (3 << 10) | (1 << 16) | (1 << 5)); // IOC=1 (bit 5)

            // Push Status Stage TRB (Type 4) on EP0 ring
            // Direction = OUT (bit 16 = 0) for IN transfers, IOC=1
            push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 5));

            core::ptr::write_volatile(&mut XHCI_TRANSFER_DONE, 0);
            
            // Ring doorbell for this slot (doorbell index = slot_id, target = 1 for EP0)
            ring_doorbell(slot_id, 1);

            // Wait for Transfer Event completion (wall-clock bounded; events
            // are serviced by the 250 Hz timer-tick poll within ~4 ms)
            let t_xfer = now_ticks();
            let tsc_xfer = rdtsc();
            while core::ptr::read_volatile(&XHCI_TRANSFER_DONE) == 0 {
                if ticks_elapsed_since(t_xfer) >= TICKS_1S || tsc_ms_elapsed(tsc_xfer) >= 1500 {
                    print_serial(b"RUST xHCI: Timeout waiting for GET_DESCRIPTOR\n\0".as_ptr());
                    break;
                }
                rust_xhci_handle_irq(core::ptr::null());
                wait_ticks(1);
            }

            if core::ptr::read_volatile(&XHCI_TRANSFER_DONE) != 0 {
                // Parse device descriptor
                let dev_class = core::ptr::read_volatile(desc_buf.add(4));
                let dev_subclass = core::ptr::read_volatile(desc_buf.add(5));
                let dev_protocol = core::ptr::read_volatile(desc_buf.add(6));

                print_serial(b"RUST xHCI: Device Class: \0".as_ptr());
                print_serial(format_num(dev_class as usize, &mut buf).as_ptr());
                print_serial(b" SubClass: \0".as_ptr());
                print_serial(format_num(dev_subclass as usize, &mut buf).as_ptr());
                print_serial(b" Protocol: \0".as_ptr());
                print_serial(format_num(dev_protocol as usize, &mut buf).as_ptr());
                print_serial(b"\n\0".as_ptr());
            }

            // Now get Configuration Descriptor to find the HID interface and interrupt endpoint
            let mut cfg_buf_phys = 0u32;
            let cfg_buf = crate::heap::kmalloc_ap(256, &mut cfg_buf_phys) as *mut u8;
            core::ptr::write_bytes(cfg_buf, 0, 256);

            // GET_DESCRIPTOR (Configuration), wValue=0x0200, wLength=64 first pass
            let setup_lo2: u32 = 0x80 | (6 << 8) | (0x0200 << 16);
            let setup_hi2: u32 = 0 | (64 << 16);

            push_ep0_trb(dev, setup_lo2, setup_hi2, 8, (2 << 10) | (3 << 16) | (1 << 6));
            push_ep0_trb(dev, cfg_buf_phys as u32, (cfg_buf_phys as u64 >> 32) as u32,
                         64, (3 << 10) | (1 << 16) | (1 << 5));
            push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 5));
            
            core::ptr::write_volatile(&mut XHCI_TRANSFER_DONE, 0);
            ring_doorbell(slot_id, 1);

            let t_xfer2 = now_ticks();
            let tsc_xfer2 = rdtsc();
            while core::ptr::read_volatile(&XHCI_TRANSFER_DONE) == 0 {
                if ticks_elapsed_since(t_xfer2) >= TICKS_1S || tsc_ms_elapsed(tsc_xfer2) >= 1500 { break; }
                rust_xhci_handle_irq(core::ptr::null());
                wait_ticks(1);
            }

            // Parse configuration descriptor to find HID keyboard interface.
            // Composite devices (kbd+mouse+consumer) ship configs longer than the
            // initial 64-byte read, so refetch at full length when needed.
            if core::ptr::read_volatile(&XHCI_TRANSFER_DONE) != 0 {
                let total_len = ((core::ptr::read_volatile(cfg_buf.add(3)) as u16) << 8
                              | core::ptr::read_volatile(cfg_buf.add(2)) as u16) as usize;
                let mut parse_buf = cfg_buf;
                let mut parse_len = if total_len < 64 { total_len } else { 64 };

                if total_len > 64 && total_len <= 4096 {
                    let mut full_buf_phys = 0u32;
                    let full_buf = crate::heap::kmalloc_ap(4096, &mut full_buf_phys) as *mut u8;
                    if !full_buf.is_null() {
                        core::ptr::write_bytes(full_buf, 0, 4096);

                        // GET_DESCRIPTOR (Configuration) again, wLength = total length
                        let setup_lo_full: u32 = 0x80 | (6 << 8) | (0x0200 << 16);
                        let setup_hi_full: u32 = (total_len as u32) << 16;

                        push_ep0_trb(dev, setup_lo_full, setup_hi_full, 8, (2 << 10) | (3 << 16) | (1 << 6));
                        push_ep0_trb(dev, full_buf_phys as u32, (full_buf_phys as u64 >> 32) as u32,
                                     total_len as u32, (3 << 10) | (1 << 16) | (1 << 5));
                        push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 5));

                        core::ptr::write_volatile(&mut XHCI_TRANSFER_DONE, 0);
                        ring_doorbell(slot_id, 1);

                        let t_xfer3 = now_ticks();
                        let tsc_xfer3 = rdtsc();
                        while core::ptr::read_volatile(&XHCI_TRANSFER_DONE) == 0 {
                            if ticks_elapsed_since(t_xfer3) >= TICKS_1S || tsc_ms_elapsed(tsc_xfer3) >= 1500 { break; }
                            rust_xhci_handle_irq(core::ptr::null());
                            wait_ticks(1);
                        }

                        if core::ptr::read_volatile(&XHCI_TRANSFER_DONE) != 0 {
                            parse_buf = full_buf;
                            parse_len = total_len;
                            print_serial(b"RUST xHCI: Full config descriptor fetched\n\0".as_ptr());
                        }
                    }
                }

                let mut offset = 0usize;
                let mut is_keyboard = false;
                let mut found_ep = false;
                let mut ep_addr: u8 = 0;
                let mut ep_interval: u8 = 0;
                let mut ep_max_packet: u16 = 0;
                let mut kb_iface_num: u8 = 0;

                let mut is_mtp = false;
                let mut found_mtp_in = false;
                let mut found_mtp_out = false;
                let mut mtp_in_addr: u8 = 0;
                let mut mtp_out_addr: u8 = 0;
                let mut mtp_in_max_pkt: u16 = 512;
                let mut mtp_out_max_pkt: u16 = 512;

                let mut is_bt = false;
                let mut found_bt_int = false;
                let mut bt_int_addr: u8 = 0;
                let mut bt_int_max_pkt: u16 = 64;
                let mut bt_int_interval: u8 = 1;
                let mut found_bt_bulk_out = false;
                let mut bt_bulk_out_addr: u8 = 0;
                let mut bt_bulk_out_max_pkt: u16 = 64;

                while offset + 1 < parse_len {
                    let desc_len = core::ptr::read_volatile(parse_buf.add(offset)) as usize;
                    let desc_type = core::ptr::read_volatile(parse_buf.add(offset + 1));

                    if desc_len == 0 { break; }

                    // Interface Descriptor (type 4)
                    if desc_type == 4 && desc_len >= 9 {
                        let iface_class = core::ptr::read_volatile(parse_buf.add(offset + 5));
                        let iface_subclass = core::ptr::read_volatile(parse_buf.add(offset + 6));
                        let iface_protocol = core::ptr::read_volatile(parse_buf.add(offset + 7));

                        print_serial(b"RUST xHCI: Interface Class: \0".as_ptr());
                        print_serial(format_num(iface_class as usize, &mut buf).as_ptr());
                        print_serial(b" Sub: \0".as_ptr());
                        print_serial(format_num(iface_subclass as usize, &mut buf).as_ptr());
                        print_serial(b" Proto: \0".as_ptr());
                        print_serial(format_num(iface_protocol as usize, &mut buf).as_ptr());
                        print_serial(b"\n\0".as_ptr());

                        // New interface boundary: endpoint descriptors below belong
                        // to this interface only (composite devices).
                        is_keyboard = iface_class == 3 && iface_subclass == 1 && iface_protocol == 1;
                        if is_keyboard {
                            kb_iface_num = core::ptr::read_volatile(parse_buf.add(offset + 2));
                            XHCI_STAGE = 4;
                            print_serial(b"RUST xHCI: *** KEYBOARD FOUND! ***\n\0".as_ptr());
                            set_xhci_debug_msg(b"xHCI: Keyboard interface found!\0".as_ptr());
                        }

                        // MTP / Mobile Device Interface detection:
                        // Only match Still Image/MTP (Class 6, Sub 1, Proto 1) or PTP.
                        // Reset is_mtp on every interface boundary.
                        is_mtp = (iface_class == 6 && iface_subclass == 1 && iface_protocol == 1)
                              || (iface_class == 6 && iface_subclass == 1 && iface_protocol == 0);
                        if is_mtp {
                            print_serial(b"RUST xHCI: *** MOBILE PHONE / MTP INTERFACE FOUND! ***\n\0".as_ptr());
                            set_xhci_debug_msg(b"xHCI: Mobile MTP interface found!\0".as_ptr());
                        }

                        // Bluetooth Controller Interface detection (Class 0xE0, Sub 1, Proto 1)
                        is_bt = (iface_class == 0xE0 && iface_subclass == 1 && iface_protocol == 1)
                             || (iface_class == 0xE0 && iface_subclass == 1)
                             || (iface_class == 0xE0);
                        if is_bt {
                            print_serial(b"RUST xHCI: *** BLUETOOTH CONTROLLER FOUND! ***\n\0".as_ptr());
                            set_xhci_debug_msg(b"xHCI: Bluetooth Wireless controller found!\0".as_ptr());
                        }
                    }

                    // Endpoint Descriptor (type 5)
                    if desc_type == 5 && desc_len >= 7 {
                        let cur_ep_addr = core::ptr::read_volatile(parse_buf.add(offset + 2));
                        let cur_ep_attribs = core::ptr::read_volatile(parse_buf.add(offset + 3));
                        let cur_ep_max_pkt = (core::ptr::read_volatile(parse_buf.add(offset + 5)) as u16) << 8
                                           | (core::ptr::read_volatile(parse_buf.add(offset + 4)) as u16);
                        let cur_ep_interval = core::ptr::read_volatile(parse_buf.add(offset + 6));

                        print_serial(b"RUST xHCI:   EP addr=0x\0".as_ptr());
                        print_serial(format_hex(cur_ep_addr as u32, &mut buf).as_ptr());
                        print_serial(b" attr=0x\0".as_ptr());
                        print_serial(format_hex(cur_ep_attribs as u32, &mut buf).as_ptr());
                        print_serial(b" maxpkt=\0".as_ptr());
                        print_serial(format_num(cur_ep_max_pkt as usize, &mut buf).as_ptr());
                        print_serial(b"\n\0".as_ptr());

                        // Keyboard Interrupt IN endpoint
                        if is_keyboard && !found_ep {
                            if (cur_ep_attribs & 0x03) == 0x03 && (cur_ep_addr & 0x80) != 0 {
                                ep_addr = cur_ep_addr;
                                ep_max_packet = cur_ep_max_pkt;
                                ep_interval = cur_ep_interval;
                                found_ep = true;
                                print_serial(b"RUST xHCI: Found Keyboard Interrupt Endpoint\n\0".as_ptr());
                            }
                        }

                        // Bluetooth Interrupt IN endpoint (HCI Events) & Bulk OUT endpoint (HCI Commands)
                        if is_bt {
                            if (cur_ep_attribs & 0x03) == 0x03 && (cur_ep_addr & 0x80) != 0 && !found_bt_int {
                                bt_int_addr = cur_ep_addr;
                                bt_int_max_pkt = cur_ep_max_pkt;
                                bt_int_interval = cur_ep_interval;
                                found_bt_int = true;
                                print_serial(b"RUST xHCI: Found Bluetooth Interrupt IN Endpoint\n\0".as_ptr());
                            } else if (cur_ep_attribs & 0x03) == 0x02 && (cur_ep_addr & 0x80) == 0 && !found_bt_bulk_out {
                                bt_bulk_out_addr = cur_ep_addr;
                                bt_bulk_out_max_pkt = cur_ep_max_pkt;
                                found_bt_bulk_out = true;
                                print_serial(b"RUST xHCI: Found Bluetooth Bulk OUT Endpoint\n\0".as_ptr());
                            }
                        }

                        // Mobile MTP Bulk endpoints (lock once found so later interfaces can't overwrite)
                        if is_mtp {
                            if (cur_ep_attribs & 0x03) == 0x02 { // Bulk
                                if (cur_ep_addr & 0x80) != 0 {
                                    if !found_mtp_in {
                                        mtp_in_addr = cur_ep_addr;
                                        mtp_in_max_pkt = cur_ep_max_pkt;
                                        found_mtp_in = true;
                                        print_serial(b"RUST xHCI: Found MTP Bulk IN Endpoint\n\0".as_ptr());
                                    }
                                } else {
                                    if !found_mtp_out {
                                        mtp_out_addr = cur_ep_addr;
                                        mtp_out_max_pkt = cur_ep_max_pkt;
                                        found_mtp_out = true;
                                        print_serial(b"RUST xHCI: Found MTP Bulk OUT Endpoint\n\0".as_ptr());
                                    }
                                }
                            }
                        }
                    }

                    offset += desc_len;
                }

                if is_keyboard && found_ep {
                    // Step 1: SET_CONFIGURATION (bRequest=9, wValue=1)
                    let setup_lo3: u32 = 0x00 | (9 << 8) | (1 << 16); // Host-to-Device, SET_CONFIGURATION, config=1
                    let setup_hi3: u32 = 0; // wIndex=0, wLength=0

                    push_ep0_trb(dev, setup_lo3, setup_hi3, 8, (2 << 10) | (0 << 16) | (1 << 6)); // TRT=0 (No Data)
                    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5)); // Status IN, IOC
                    
                    core::ptr::write_volatile(&mut XHCI_TRANSFER_DONE, 0);
                    ring_doorbell(slot_id, 1);

                    let t_setcfg = now_ticks();
                    while core::ptr::read_volatile(&XHCI_TRANSFER_DONE) == 0 {
                        if ticks_elapsed_since(t_setcfg) >= TICKS_1S { break; }
                        rust_xhci_handle_irq(core::ptr::null());
                        wait_ticks(1);
                    }
                    print_serial(b"RUST xHCI: SET_CONFIGURATION done\n\0".as_ptr());

                    // Step 2: SET_PROTOCOL to Boot Protocol (bRequest=0x0B, wValue=0)
                    // bmRequestType = 0x21 (class, interface, host-to-device)
                    let setup_lo4: u32 = 0x21 | (0x0B << 8) | (0 << 16);
                    let setup_hi4: u32 = kb_iface_num as u32; // wIndex = keyboard interface

                    push_ep0_trb(dev, setup_lo4, setup_hi4, 8, (2 << 10) | (0 << 16) | (1 << 6));
                    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5));
                    
                    core::ptr::write_volatile(&mut XHCI_TRANSFER_DONE, 0);
                    ring_doorbell(slot_id, 1);

                    let t_setproto = now_ticks();
                    while core::ptr::read_volatile(&XHCI_TRANSFER_DONE) == 0 {
                        if ticks_elapsed_since(t_setproto) >= TICKS_1S { break; }
                        rust_xhci_handle_irq(core::ptr::null());
                        wait_ticks(1);
                    }
                    print_serial(b"RUST xHCI: SET_PROTOCOL (Boot) done\n\0".as_ptr());

                    // Step 2b: SET_IDLE(0): report only on change (bRequest=0x0A)
                    let setup_lo5: u32 = 0x21 | (0x0A << 8) | (0 << 16);
                    let setup_hi5: u32 = kb_iface_num as u32;

                    push_ep0_trb(dev, setup_lo5, setup_hi5, 8, (2 << 10) | (0 << 16) | (1 << 6));
                    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5));

                    core::ptr::write_volatile(&mut XHCI_TRANSFER_DONE, 0);
                    ring_doorbell(slot_id, 1);

                    let t_setidle = now_ticks();
                    while core::ptr::read_volatile(&XHCI_TRANSFER_DONE) == 0 {
                        if ticks_elapsed_since(t_setidle) >= TICKS_1S { break; }
                        rust_xhci_handle_irq(core::ptr::null());
                        wait_ticks(1);
                    }
                    print_serial(b"RUST xHCI: SET_IDLE(0) done\n\0".as_ptr());

                    // Step 3: Configure Endpoint - tell xHCI about the interrupt IN endpoint
                    let ep_num = ep_addr & 0x0F; // Endpoint number (1-15)
                    let ep_dir = (ep_addr >> 7) & 1; // 1 = IN
                    // DCI (Device Context Index) = ep_num * 2 + ep_dir
                    let dci = (ep_num as u32) * 2 + (ep_dir as u32);

                    // Allocate interrupt endpoint Transfer Ring
                    let mut int_ring_phys = 0u32;
                    let int_ring = crate::heap::kmalloc_ap(4096, &mut int_ring_phys) as *mut u32;
                    core::ptr::write_bytes(int_ring, 0, 1024);

                    // Re-use input context, clear it
                    let input_ctx = dev.input_ctx as *mut u32;
                    core::ptr::write_bytes(input_ctx, 0, 1024);

                    // Input Control Context: Add flag for Slot (A0) and the endpoint (A[dci])
                    core::ptr::write_volatile(input_ctx.add(1), (1 << 0) | (1 << dci));

                    // Slot Context: Update Context Entries to include the new endpoint
                    // (spec 6.2.2: Speed DW0[23:20], Root Hub Port Number DW1[23:16])
                    let slot_ctx = input_ctx.add(8);
                    core::ptr::write_volatile(slot_ctx.add(0), (dci << 27) | ((dev.speed & 0xF) << 20)); // Context Entries = dci, Speed
                    core::ptr::write_volatile(slot_ctx.add(1), ((dev.port) & 0xFF) << 16); // Root Hub Port Number

                    // Endpoint Context at offset 0x20 + dci * 0x20 (each context = 32 bytes = 8 u32s)
                    let ep_ctx = input_ctx.add(8 + (dci as usize) * 8);

                    // EP Type for Interrupt IN = 7, CErr = 3, MaxPacketSize, Interval
                    // FS/LS descriptor bInterval is milliseconds; the EP context
                    // field is an exponent of 125us units. Convert: smallest i with
                    // 2^i >= ms, then +3 (8x125us per ms). HS/SS pass through as-is.
                    let interval_val: u32 = if dev.speed >= 3 {
                        ep_interval as u32
                    } else {
                        let mut e: u32 = 0;
                        while e < 10 && (1u32 << e) < ep_interval as u32 {
                            e += 1;
                        }
                        e + 3
                    };
                    core::ptr::write_volatile(ep_ctx.add(0), (interval_val as u32) << 16); // Interval
                    core::ptr::write_volatile(ep_ctx.add(1),
                        (3 << 1) |                          // CErr = 3
                        (7 << 3) |                          // EP Type = Interrupt IN
                        ((ep_max_packet as u32 & 0x7FF) << 16) // Max Packet Size
                    );

                    let int_ring_ptr = int_ring_phys as u64;
                    core::ptr::write_volatile(ep_ctx.add(2), (int_ring_ptr as u32) | 1); // DCS = 1
                    core::ptr::write_volatile(ep_ctx.add(3), (int_ring_ptr >> 32) as u32);
                    core::ptr::write_volatile(ep_ctx.add(4), 8); // Average TRB Length

                    // Send Configure Endpoint Command (Type 12)
                    print_serial(b"RUST xHCI: Sending Configure Endpoint Command...\n\0".as_ptr());
                    core::ptr::write_volatile(&mut XHCI_LAST_SLOT_ID, 0);
                    push_command_trb(dev.input_ctx_phys as u32, (dev.input_ctx_phys as u64 >> 32) as u32, 0,
                                     (12 << 10) | (slot_id << 24));

                    let t_cfgep = now_ticks();
                    while core::ptr::read_volatile(&XHCI_LAST_SLOT_ID) == 0 {
                        if ticks_elapsed_since(t_cfgep) >= TICKS_1S {
                            print_serial(b"RUST xHCI: Timeout Configure Endpoint\n\0".as_ptr());
                            break;
                        }
                        rust_xhci_handle_irq(core::ptr::null());
                        wait_ticks(1);
                    }

                    if core::ptr::read_volatile(&XHCI_LAST_SLOT_ID) != 255 {
                        print_serial(b"RUST xHCI: Configure Endpoint SUCCESS!\n\0".as_ptr());

                        // Save keyboard state
                        XHCI_KB_SLOT = slot_id;
                        XHCI_KB_DCI = dci;
                        XHCI_KB_INT_RING = int_ring as u64;
                        XHCI_KB_INT_PHYS = int_ring_phys as u64;
                        XHCI_KB_INT_ENQ = 0;
                        XHCI_KB_INT_CYCLE = 1;

                        // Allocate HID report buffer (8 bytes for boot keyboard)
                        let mut hid_buf_phys = 0u32;
                        let hid_buf = crate::heap::kmalloc_ap(64, &mut hid_buf_phys) as *mut u8;
                        core::ptr::write_bytes(hid_buf, 0, 64);
                        XHCI_KB_HID_BUF = hid_buf as u64;
                        XHCI_KB_HID_PHYS = hid_buf_phys as u64;

                        // Queue first interrupt IN transfer
                        queue_keyboard_transfer();

                        XHCI_STAGE = 5;
                        dev.configured = true;
                        print_serial(b"RUST xHCI: Keyboard HID polling ACTIVE!\n\0".as_ptr());
                        set_xhci_debug_msg(b"xHCI: Keyboard polling ACTIVE!\0".as_ptr());
                    } else {
                        print_serial(b"RUST xHCI: Configure Endpoint FAILED\n\0".as_ptr());
                        set_xhci_debug_msg(b"xHCI: Configure Endpoint FAILED!\0".as_ptr());
                    }
                }

                if found_mtp_in && found_mtp_out {
                    print_serial(b"RUST xHCI: Configuring Mobile / MTP endpoints...\n\0".as_ptr());

                    // If SET_CONFIGURATION wasn't done yet, do it now
                    if !is_keyboard || !found_ep {
                        let setup_lo: u32 = 0x00 | (9 << 8) | (1 << 16);
                        let setup_hi: u32 = 0;
                        push_ep0_trb(dev, setup_lo, setup_hi, 8, (2 << 10) | (0 << 16) | (1 << 6));
                        push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5));
                        core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
                        ring_doorbell(slot_id, 1);
                        let t_setcfg = now_ticks();
                        while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
                            if ticks_elapsed_since(t_setcfg) >= TICKS_1S { break; }
                            rust_xhci_handle_irq(core::ptr::null());
                            wait_ticks(1);
                        }
                    }

                    let ep_in_num = (mtp_in_addr & 0x0F) as u32;
                    let dci_in = ep_in_num * 2 + 1;

                    let ep_out_num = (mtp_out_addr & 0x0F) as u32;
                    let dci_out = ep_out_num * 2 + 0;

                    let max_dci = if dci_in > dci_out { dci_in } else { dci_out };

                    if mtp_in_max_pkt == 0 { mtp_in_max_pkt = 512; }
                    if mtp_out_max_pkt == 0 { mtp_out_max_pkt = 512; }

                    print_serial(b"RUST xHCI: MTP Endpoints IN=0x\0".as_ptr());
                    print_serial(format_hex(mtp_in_addr as u32, &mut buf).as_ptr());
                    print_serial(b" (dci=\0".as_ptr());
                    print_serial(format_num(dci_in as usize, &mut buf).as_ptr());
                    print_serial(b", pkt=\0".as_ptr());
                    print_serial(format_num(mtp_in_max_pkt as usize, &mut buf).as_ptr());
                    print_serial(b") OUT=0x\0".as_ptr());
                    print_serial(format_hex(mtp_out_addr as u32, &mut buf).as_ptr());
                    print_serial(b" (dci=\0".as_ptr());
                    print_serial(format_num(dci_out as usize, &mut buf).as_ptr());
                    print_serial(b", pkt=\0".as_ptr());
                    print_serial(format_num(mtp_out_max_pkt as usize, &mut buf).as_ptr());
                    print_serial(b")\n\0".as_ptr());

                    let mut in_ring_phys = 0u32;
                    let in_ring = crate::heap::kmalloc_ap(4096, &mut in_ring_phys) as *mut u32;
                    core::ptr::write_bytes(in_ring, 0, 1024);

                    let mut out_ring_phys = 0u32;
                    let out_ring = crate::heap::kmalloc_ap(4096, &mut out_ring_phys) as *mut u32;
                    core::ptr::write_bytes(out_ring, 0, 1024);

                    let input_ctx = dev.input_ctx as *mut u32;
                    core::ptr::write_bytes(input_ctx, 0, 1024);

                    // Input Control Context: Add Slot + Bulk IN + Bulk OUT
                    core::ptr::write_volatile(input_ctx.add(1), (1 << 0) | (1 << dci_in) | (1 << dci_out));

                    // Slot Context:
                    let slot_ctx = input_ctx.add(8);
                    core::ptr::write_volatile(slot_ctx.add(0), (max_dci << 27) | ((dev.speed & 0xF) << 20));
                    core::ptr::write_volatile(slot_ctx.add(1), ((dev.port) & 0xFF) << 16);

                    // Bulk IN Context:
                    let ep_in_ctx = input_ctx.add(8 + (dci_in as usize) * 8);
                    core::ptr::write_volatile(ep_in_ctx.add(0), 0);
                    core::ptr::write_volatile(ep_in_ctx.add(1), (3 << 1) | (6 << 3) | ((mtp_in_max_pkt as u32 & 0x7FF) << 16));
                    let in_ptr = in_ring_phys as u64;
                    core::ptr::write_volatile(ep_in_ctx.add(2), (in_ptr as u32) | 1);
                    core::ptr::write_volatile(ep_in_ctx.add(3), (in_ptr >> 32) as u32);
                    core::ptr::write_volatile(ep_in_ctx.add(4), mtp_in_max_pkt as u32);

                    // Bulk OUT Context:
                    let ep_out_ctx = input_ctx.add(8 + (dci_out as usize) * 8);
                    core::ptr::write_volatile(ep_out_ctx.add(0), 0);
                    core::ptr::write_volatile(ep_out_ctx.add(1), (3 << 1) | (2 << 3) | ((mtp_out_max_pkt as u32 & 0x7FF) << 16));
                    let out_ptr = out_ring_phys as u64;
                    core::ptr::write_volatile(ep_out_ctx.add(2), (out_ptr as u32) | 1);
                    core::ptr::write_volatile(ep_out_ctx.add(3), (out_ptr >> 32) as u32);
                    core::ptr::write_volatile(ep_out_ctx.add(4), mtp_out_max_pkt as u32);

                    // Send Configure Endpoint Command
                    core::ptr::write_volatile(&raw mut XHCI_LAST_SLOT_ID, 0);
                    push_command_trb(dev.input_ctx_phys as u32, (dev.input_ctx_phys as u64 >> 32) as u32, 0,
                                     (12 << 10) | (slot_id << 24));

                    let t_cfgep = now_ticks();
                    while core::ptr::read_volatile(&raw const XHCI_LAST_SLOT_ID) == 0 {
                        if ticks_elapsed_since(t_cfgep) >= TICKS_1S {
                            print_serial(b"RUST xHCI: Timeout Configure Endpoint for MTP\n\0".as_ptr());
                            break;
                        }
                        rust_xhci_handle_irq(core::ptr::null());
                        wait_ticks(1);
                    }

                    if core::ptr::read_volatile(&raw const XHCI_LAST_SLOT_ID) != 255 {
                        print_serial(b"RUST xHCI: MTP Configure Endpoint SUCCESS!\n\0".as_ptr());
                        core::ptr::write_volatile(&raw mut XHCI_MTP_SLOT, slot_id);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DCI, dci_in);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DCI, dci_out);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_RING, in_ring as u64);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_PHYS, in_ring_phys as u64);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_ENQ, 0);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_CYCLE, 1);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_RING, out_ring as u64);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_PHYS, out_ring_phys as u64);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_ENQ, 0);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_CYCLE, 1);

                        if core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DMA_BUF) == 0 {
                            let mut dma_phys = 0u32;
                            let dma_buf = crate::heap::kmalloc_ap(16384, &mut dma_phys) as *mut u8;
                            core::ptr::write_bytes(dma_buf, 0, 16384);
                            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DMA_BUF, dma_buf as u64);
                            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DMA_PHYS, dma_phys as u64);
                        }
                        if core::ptr::read_volatile(&raw const XHCI_MTP_IN_DMA_BUF) == 0 {
                            let mut dma_phys = 0u32;
                            let dma_buf = crate::heap::kmalloc_ap(16384, &mut dma_phys) as *mut u8;
                            core::ptr::write_bytes(dma_buf, 0, 16384);
                            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DMA_BUF, dma_buf as u64);
                            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DMA_PHYS, dma_phys as u64);
                        }
                        dev.configured = true;
                        print_serial(b"RUST xHCI: Mobile Data Transfer Ready!\n\0".as_ptr());
                        set_xhci_debug_msg(b"xHCI: Mobile MTP ready!\0".as_ptr());
                    } else {
                        print_serial(b"RUST xHCI: MTP Configure Endpoint FAILED\n\0".as_ptr());
                    }
                }

                if !is_keyboard && !is_mtp && found_bt_int {
                    print_serial(b"RUST xHCI: Configuring Bluetooth endpoints...\n\0".as_ptr());
                    configure_bt_device(i, bt_int_addr, bt_int_max_pkt, bt_int_interval, bt_bulk_out_addr, bt_bulk_out_max_pkt);
                }
            }
        }
    }

    0
}

// Round-3: force Port Power ON exactly once per port. Repeated PP writes
// every pass churned real-silicon link states and generated endless port
// change events; QEMU ignores PP entirely so only HW noticed.
unsafe fn power_ports_once() {
    let max_ports = XHCI_MAX_PORTS;
    let mut port = 1u32;
    while port <= max_ports {
        let paddr = portsc_addr(port);
        let val = core::ptr::read_volatile(paddr);
        core::ptr::write_volatile(paddr, (val | PORTSC_W1C_CHANGES | PORTSC_PP) & !PORTSC_PR);
        port += 1;
    }
}

// One scan pass across all ports: clears stale W1C change bits, then
// reports which ports currently show CCS.
unsafe fn scan_ports_connected() -> u32 {
    let max_ports = XHCI_MAX_PORTS;
    let mut mask = 0u32;
    let mut port = 1u32;
    while port <= max_ports {
        let paddr = portsc_addr(port);
        let val = core::ptr::read_volatile(paddr);
        let write_val = (val | PORTSC_W1C_CHANGES) & !PORTSC_PR;
        core::ptr::write_volatile(paddr, write_val);

        let status = core::ptr::read_volatile(paddr);
        XHCI_LAST_SCAN_PORT = port;
        XHCI_LAST_SCAN_PORTSC = status;

        if status & PORTSC_CCS != 0 && port <= 32 {
            mask |= 1 << (port - 1);
        }
        port += 1;
    }
    mask
}

// Enumerate one port end-to-end: reset, Enable Slot, Address Device.
unsafe fn try_enumerate_port(port: u32) -> bool {
    let mut buf = [0u8; 16];
    let paddr = portsc_addr(port);

    print_serial(b"RUST xHCI: Device connected on Port \0".as_ptr());
    print_serial(format_num(port as usize, &mut buf).as_ptr());
    print_serial(b"\n\0".as_ptr());
    set_xhci_debug_msg(b"xHCI: Device connected! Resetting port...\0".as_ptr());

    // Reset the port (Set PR)
    let w1c_mask = 0x00FE0000u32;
    let portsc_val = core::ptr::read_volatile(paddr);
    let mut reset_val = portsc_val & !w1c_mask;
    reset_val |= PORTSC_PR;

    print_serial(b"RUST xHCI: Resetting port...\n\0".as_ptr());
    core::ptr::write_volatile(paddr, reset_val);

    // Wait for Port Reset Change (PRC) - tick-bounded (~200 ms) so a wedged
    // port can never stall boot. Round-2's 200M uncached-MMIO spin budget
    // could burn tens of seconds PER PORT on real hardware.
    let mut prc_ticks = 0u32;
    loop {
        let status = core::ptr::read_volatile(paddr);
        if status & PORTSC_PRC != 0 || prc_ticks >= 50 {
            break;
        }
        wait_ticks(1);
        prc_ticks += 1;
    }

    // Wait for Port Enabled (PED) - tick-bounded (~200 ms).
    let mut ped_ticks = 0u32;
    loop {
        let status = core::ptr::read_volatile(paddr);
        if status & (1 << 1) != 0 || ped_ticks >= 50 {
            break;
        }
        wait_ticks(1);
        ped_ticks += 1;
    }
    if core::ptr::read_volatile(paddr) & (1 << 1) == 0 {
        print_serial(b"RUST xHCI: Port did not reach enabled state!\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Port enable FAILED!\0".as_ptr());
        return false;
    }

    print_serial(b"RUST xHCI: Port reset complete. Enabled!\n\0".as_ptr());
    set_xhci_debug_msg(b"xHCI: Port reset complete. Enabling slot...\0".as_ptr());

    // Hardware delay for physical ports to stabilize
    let mut delay = 0;
    while delay < 20000000 {
        delay += 1;
        core::arch::asm!("nop");
    }

    // Read Port Speed (Bits 13:10)
    let status = core::ptr::read_volatile(paddr);
    let speed = (status >> 10) & 0xF;
    print_serial(b"RUST xHCI: Port Speed: \0".as_ptr());
    print_serial(format_num(speed as usize, &mut buf).as_ptr());
    print_serial(b"\n\0".as_ptr());

    // Clear PRC (write 1 to clear)
    let clear_val = (status & !w1c_mask) | PORTSC_PRC;
    core::ptr::write_volatile(paddr, clear_val);

    // Send Enable Slot Command (Type 9)
    print_serial(b"RUST xHCI: Sending Enable Slot Command...\n\0".as_ptr());
    XHCI_LAST_SLOT_ID = 0;
    push_command_trb(0, 0, 0, 9 << 10);

    let t_enab = now_ticks();
    let tsc_enab = rdtsc();
    while core::ptr::read_volatile(&XHCI_LAST_SLOT_ID) == 0 {
        rust_xhci_handle_irq(core::ptr::null());
        if ticks_elapsed_since(t_enab) >= TICKS_1S || tsc_ms_elapsed(tsc_enab) >= 1000 {
            print_serial(b"RUST xHCI: Timeout waiting for Slot ID\n\0".as_ptr());
            set_xhci_debug_msg(b"xHCI: Enable Slot TIMEOUT!\0".as_ptr());
            break;
        }
    }

    let slot_id = core::ptr::read_volatile(&XHCI_LAST_SLOT_ID);
    if !(slot_id > 0 && slot_id != 255) {
        set_xhci_debug_msg(b"xHCI: Enable Slot FAILED!\0".as_ptr());
        return false;
    }

    XHCI_STAGE = 2;
    print_serial(b"RUST xHCI: Received Slot ID: \0".as_ptr());
    print_serial(format_num(slot_id as usize, &mut buf).as_ptr());
    print_serial(b"\n\0".as_ptr());

    // Allocate Input Context, Output Context, and EP0 Transfer Ring
    let mut input_ctx_phys = 0u32;
    let input_ctx = crate::heap::kmalloc_ap(4096, &mut input_ctx_phys) as *mut u32;
    let mut output_ctx_phys = 0u32;
    let output_ctx = crate::heap::kmalloc_ap(4096, &mut output_ctx_phys) as *mut u32;
    let mut ep0_ring_phys = 0u32;
    let ep0_ring = crate::heap::kmalloc_ap(4096, &mut ep0_ring_phys) as *mut u32;

    core::ptr::write_bytes(input_ctx, 0, 1024);
    core::ptr::write_bytes(output_ctx, 0, 1024);
    core::ptr::write_bytes(ep0_ring, 0, 1024);

    // Set DCBAA entry for this slot
    let dcbaa = XHCI_DCBAAP as *mut u64;
    core::ptr::write_volatile(dcbaa.add(slot_id as usize), output_ctx_phys as u64);

    // Initialize Input Control Context (Offset 0x00)
    // Dword 1: Add Context Flags (A0 and A1)
    core::ptr::write_volatile(input_ctx.add(1), (1 << 0) | (1 << 1));

    // Initialize Slot Context (Offset 0x20 = 8 u32s)
    // Per xHCI spec section 6.2.2:
    //   DW0: Route String [19:0] | Speed [23:20] | MTT [24] | Hub [25] | Context Entries [31:27]
    //   DW1: Max Exit Latency [15:0] | Root Hub Port Number [23:16] | Number of Ports [31:24]
    let slot_ctx = input_ctx.add(8);
    core::ptr::write_volatile(slot_ctx.add(0), (1 << 27) | ((speed & 0xF) << 20)); // Context Entries = 1, Speed, Route String = 0 (root hub port)
    core::ptr::write_volatile(slot_ctx.add(1), (port & 0xFF) << 16); // Root Hub Port Number
    core::ptr::write_volatile(slot_ctx.add(2), 0); // Interrupter Target = 0

    // Initialize EP0 Context (Offset 0x40 = 16 u32s)
    let ep0_ctx = input_ctx.add(16);

    // xHCI port speeds: 1=Full(12Mb) 2=Low(1.5Mb) 3=High(480Mb) 4=Super
    // EP0 MaxPacketSize: Low=8, Full=64, High=64, Super=512
    let max_packet_size: u32 = match speed {
        2 => 8,
        4 => 512,
        _ => 64,
    };

    core::ptr::write_volatile(ep0_ctx.add(1), (3 << 1) | (4 << 3) | (max_packet_size << 16)); // Error count = 3, EP Type = Control (4), Max Packet Size

    // Set EP0 TR Dequeue Pointer (Dwords 2 and 3)
    // Note: bit 0 of Dword 2 is DCS (Dequeue Cycle State), set it to 1
    let tr_ptr = ep0_ring_phys as u64;
    core::ptr::write_volatile(ep0_ctx.add(2), (tr_ptr as u32) | 1);
    core::ptr::write_volatile(ep0_ctx.add(3), (tr_ptr >> 32) as u32);

    core::ptr::write_volatile(ep0_ctx.add(4), 8); // Average TRB Length

    // Send Address Device Command (Type 11)
    print_serial(b"RUST xHCI: Sending Address Device Command...\n\0".as_ptr());
    core::ptr::write_volatile(&mut XHCI_LAST_SLOT_ID, 0);
    push_command_trb(input_ctx_phys as u32, (input_ctx_phys as u64 >> 32) as u32, 0, (11 << 10) | (slot_id << 24));

    let t_addr = now_ticks();
    while core::ptr::read_volatile(&XHCI_LAST_SLOT_ID) == 0 {
        if ticks_elapsed_since(t_addr) >= TICKS_1S {
            print_serial(b"RUST xHCI: Timeout waiting for Address Device completion\n\0".as_ptr());
            set_xhci_debug_msg(b"xHCI: Address Device TIMEOUT!\0".as_ptr());
            break;
        }
        rust_xhci_handle_irq(core::ptr::null());
        wait_ticks(1);
    }

    if core::ptr::read_volatile(&XHCI_LAST_SLOT_ID) == 0
        || core::ptr::read_volatile(&XHCI_LAST_SLOT_ID) == 255
    {
        print_serial(b"RUST xHCI: Address Device FAILED\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Address Device FAILED!\0".as_ptr());
        // Disable slot to prevent slot exhaustion
        push_command_trb(0, 0, 0, (10 << 10) | (slot_id << 24));
        let dcbaa = XHCI_DCBAAP as *mut u64;
        if dcbaa as usize != 0 {
            core::ptr::write_volatile(dcbaa.add(slot_id as usize), 0);
        }
        return false;
    }

    set_xhci_debug_msg(b"xHCI: Address Device SUCCESS! Polling...\0".as_ptr());
    print_serial(b"RUST xHCI: Address Device SUCCESS!\n\0".as_ptr());
    XHCI_STAGE = 3;

    // Save per-device state for Phase 4
    let mut dev_idx = 4usize;
    for i in 0..4 {
        if XHCI_DEVICES[i].slot_id == 0 {
            dev_idx = i;
            break;
        }
    }
    if dev_idx >= 4 {
        push_command_trb(0, 0, 0, (10 << 10) | (slot_id << 24));
        let dcbaa = XHCI_DCBAAP as *mut u64;
        if dcbaa as usize != 0 {
            core::ptr::write_volatile(dcbaa.add(slot_id as usize), 0);
        }
        return false;
    }
    XHCI_DEVICES[dev_idx].slot_id = slot_id;
    XHCI_DEVICES[dev_idx].port = port;
    XHCI_DEVICES[dev_idx].speed = speed;
    XHCI_DEVICES[dev_idx].ep0_ring = ep0_ring as u64;
    XHCI_DEVICES[dev_idx].ep0_ring_phys = ep0_ring_phys as u64;
    XHCI_DEVICES[dev_idx].ep0_enq = 0;
    XHCI_DEVICES[dev_idx].ep0_cycle = 1;
    XHCI_DEVICES[dev_idx].input_ctx = input_ctx as u64;
    XHCI_DEVICES[dev_idx].input_ctx_phys = input_ctx_phys as u64;
    XHCI_DEVICES[dev_idx].configured = false;
    if (dev_idx as u32) >= XHCI_NUM_DEVICES {
        XHCI_NUM_DEVICES = (dev_idx as u32) + 1;
    }

    true
}

fn format_num(mut n: usize, buf: &mut [u8; 16]) -> &[u8] {
    if n == 0 {
        buf[0] = b'0';
        buf[1] = 0;
        return &buf[..2];
    }
    let mut i = 14;
    buf[15] = 0; // null terminator
    while n > 0 && i > 0 {
        buf[i] = b'0' + (n % 10) as u8;
        n /= 10;
        i -= 1;
    }
    &buf[i + 1..]
}

fn format_hex(mut n: u32, buf: &mut [u8; 16]) -> &[u8] {
    if n == 0 {
        buf[0] = b'0';
        buf[1] = 0;
        return &buf[..2];
    }
    let hex_chars = b"0123456789abcdef";
    let mut i = 14;
    buf[15] = 0;
    while n > 0 && i > 0 {
        buf[i] = hex_chars[(n & 0x0F) as usize];
        n >>= 4;
        i -= 1;
    }
    &buf[i + 1..]
}

// ---- Per-device state ----
#[derive(Copy, Clone)]
struct XhciDevice {
    slot_id: u32,
    port: u32,
    speed: u32,
    ep0_ring: u64,
    ep0_ring_phys: u64,
    ep0_enq: u32,
    ep0_cycle: u32,
    input_ctx: u64,
    input_ctx_phys: u64,
    configured: bool,
}

impl XhciDevice {
    const fn new() -> Self {
        XhciDevice { slot_id: 0, port: 0, speed: 0, ep0_ring: 0, ep0_ring_phys: 0, ep0_enq: 0, ep0_cycle: 1, input_ctx: 0, input_ctx_phys: 0, configured: false }
    }
}

static mut XHCI_DEVICES: [XhciDevice; 4] = [XhciDevice::new(); 4];
static mut XHCI_NUM_DEVICES: u32 = 0;

// ---- Global state for the IRQ handler ----
static mut XHCI_MMIO_BASE: usize = 0;
static mut XHCI_EVENT_RING_BASE: u64 = 0;
static mut XHCI_EVENT_RING_PHYS: u64 = 0;
static mut XHCI_EVENT_RING_SIZE: u32 = 256;
static mut XHCI_ERDP_INDEX: u32 = 0;
static mut XHCI_EVENT_CYCLE: u32 = 1;
static mut XHCI_CAP_LENGTH: usize = 0;

static mut XHCI_CMD_RING_BASE: u64 = 0;
static mut XHCI_CMD_RING_PHYS: u64 = 0;
static mut XHCI_CMD_INDEX: u32 = 0;
static mut XHCI_CMD_CYCLE: u32 = 1;
static mut XHCI_LAST_SLOT_ID: u32 = 0;
static mut XHCI_DCBAAP: u64 = 0;

// Transfer completion flag
static mut XHCI_TRANSFER_DONE: u32 = 0;

// Keyboard-specific state
static mut XHCI_KB_SLOT: u32 = 0;
static mut XHCI_KB_DCI: u32 = 0;
static mut XHCI_KB_INT_RING: u64 = 0;
static mut XHCI_KB_INT_PHYS: u64 = 0;
static mut XHCI_KB_INT_ENQ: u32 = 0;
static mut XHCI_KB_INT_CYCLE: u32 = 1;
static mut XHCI_KB_HID_BUF: u64 = 0;
static mut XHCI_KB_HID_PHYS: u64 = 0;
static mut XHCI_KB_PREV_KEYS: [u8; 6] = [0; 6]; // Previous key state for detecting press/release

// Mobile MTP device state
static mut XHCI_MTP_SLOT: u32 = 0;
static mut XHCI_MTP_IN_DCI: u32 = 0;
static mut XHCI_MTP_OUT_DCI: u32 = 0;
static mut XHCI_MTP_IN_RING: u64 = 0;
static mut XHCI_MTP_IN_PHYS: u64 = 0;
static mut XHCI_MTP_IN_ENQ: u32 = 0;
static mut XHCI_MTP_IN_CYCLE: u32 = 1;
static mut XHCI_MTP_OUT_RING: u64 = 0;
static mut XHCI_MTP_OUT_PHYS: u64 = 0;
static mut XHCI_MTP_OUT_ENQ: u32 = 0;
static mut XHCI_MTP_OUT_CYCLE: u32 = 1;

static mut XHCI_MTP_OUT_DMA_BUF: u64 = 0;
static mut XHCI_MTP_OUT_DMA_PHYS: u64 = 0;
static mut XHCI_MTP_IN_DMA_BUF: u64 = 0;
static mut XHCI_MTP_IN_DMA_PHYS: u64 = 0;
static mut XHCI_MTP_IN_DONE: u32 = 0;
static mut XHCI_MTP_IN_REQ_LEN: u32 = 0;
static mut XHCI_MTP_IN_TRANSFERRED: u32 = 0;
static mut XHCI_MTP_OUT_DONE: u32 = 0;

// Bluetooth HCI device state
static mut XHCI_BT_SLOT: u32 = 0;
static mut XHCI_BT_INT_IN_DCI: u32 = 0;
static mut XHCI_BT_INT_IN_RING: u64 = 0;
static mut XHCI_BT_INT_IN_PHYS: u64 = 0;
static mut XHCI_BT_INT_IN_ENQ: u32 = 0;
static mut XHCI_BT_INT_IN_CYCLE: u32 = 1;
static mut XHCI_BT_INT_MAX_PKT: u16 = 64;

static mut XHCI_BT_EVT_DMA_BUF: u64 = 0;
static mut XHCI_BT_EVT_DMA_PHYS: u64 = 0;

static mut XHCI_BT_CMD_DMA_BUF: u64 = 0;
static mut XHCI_BT_CMD_DMA_PHYS: u64 = 0;

// Bluetooth Bulk OUT endpoint (HCI Command transport)
static mut XHCI_BT_BULK_OUT_DCI: u32 = 0;
static mut XHCI_BT_BULK_OUT_RING: u64 = 0;
static mut XHCI_BT_BULK_OUT_PHYS: u64 = 0;
static mut XHCI_BT_BULK_OUT_ENQ: u32 = 0;
static mut XHCI_BT_BULK_OUT_CYCLE: u32 = 1;
static mut XHCI_BT_BULK_OUT_DONE: u32 = 0;
static mut XHCI_BT_INT_REQ_LEN: u32 = 512;

const BT_FIFO_CAPACITY: usize = 2048;
static mut XHCI_BT_FIFO: [u8; BT_FIFO_CAPACITY] = [0u8; BT_FIFO_CAPACITY];
static mut XHCI_BT_FIFO_HEAD: usize = 0;
static mut XHCI_BT_FIFO_TAIL: usize = 0;

unsafe fn configure_bt_device(dev_idx: usize, bt_int_addr: u8, bt_int_max_pkt: u16, bt_int_interval: u8, bt_bulk_out_addr: u8, bt_bulk_out_max_pkt: u16) -> bool {
    let dev = &mut XHCI_DEVICES[dev_idx];
    let slot_id = dev.slot_id;
    if slot_id == 0 || dev.configured {
        return false;
    }

    // Step 1: SET_CONFIGURATION (1)
    let setup_lo: u32 = 0x00 | (9 << 8) | (1 << 16);
    let setup_hi: u32 = 0;
    push_ep0_trb(dev, setup_lo, setup_hi, 8, (2 << 10) | (0 << 16) | (1 << 6));
    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5));
    core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
    ring_doorbell(slot_id, 1);

    let t0 = now_ticks();
    while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
        if ticks_elapsed_since(t0) >= TICKS_1S { break; }
        rust_xhci_handle_irq(core::ptr::null());
        wait_ticks(1);
    }
    print_serial(b"RUST xHCI: Bluetooth SET_CONFIGURATION done\n\0".as_ptr());

    // Step 2: Compute DCIs for both endpoints
    let int_ep_num = (bt_int_addr & 0x0F) as u32;
    let int_ep_dir = ((bt_int_addr >> 7) & 1) as u32;
    let int_dci = int_ep_num * 2 + int_ep_dir;

    let bulk_out_ep_num = (bt_bulk_out_addr & 0x0F) as u32;
    let bulk_out_dci = bulk_out_ep_num * 2; // OUT direction, DCI = ep_num * 2

    // Determine the highest DCI for the Slot Context
    let max_dci = if int_dci > bulk_out_dci { int_dci } else { bulk_out_dci };

    // Step 3: Allocate transfer rings for both endpoints
    let mut int_ring_phys = 0u32;
    let int_ring = crate::heap::kmalloc_ap(4096, &mut int_ring_phys) as *mut u32;
    core::ptr::write_bytes(int_ring, 0, 1024);

    let mut bulk_out_ring_phys = 0u32;
    let bulk_out_ring = crate::heap::kmalloc_ap(4096, &mut bulk_out_ring_phys) as *mut u32;
    core::ptr::write_bytes(bulk_out_ring, 0, 1024);

    // Step 4: Build Input Context with both endpoints
    let input_ctx = dev.input_ctx as *mut u32;
    core::ptr::write_bytes(input_ctx, 0, 1024);
    // Add Context flags: Slot (bit 0) + both endpoint DCIs
    core::ptr::write_volatile(input_ctx.add(1), (1 << 0) | (1 << int_dci) | (1 << bulk_out_dci));

    // Slot Context: set Context Entries to max_dci
    let slot_ctx = input_ctx.add(8);
    core::ptr::write_volatile(slot_ctx.add(0), (max_dci << 27) | ((dev.speed & 0xF) << 20));
    core::ptr::write_volatile(slot_ctx.add(1), ((dev.port) & 0xFF) << 16);

    // -- Interrupt IN endpoint context --
    let int_ep_ctx = input_ctx.add(8 + (int_dci as usize) * 8);
    let interval_val: u32 = if dev.speed >= 3 {
        bt_int_interval as u32
    } else {
        let mut e: u32 = 0;
        while e < 10 && (1u32 << e) < bt_int_interval as u32 {
            e += 1;
        }
        e + 3
    };
    core::ptr::write_volatile(int_ep_ctx.add(0), (interval_val as u32) << 16);
    core::ptr::write_volatile(int_ep_ctx.add(1),
        (3 << 1) | // CErr = 3
        (7 << 3) | // EP Type = Interrupt IN
        ((bt_int_max_pkt as u32 & 0x7FF) << 16)
    );
    let int_ring_ptr = int_ring_phys as u64;
    core::ptr::write_volatile(int_ep_ctx.add(2), (int_ring_ptr as u32) | 1); // DCS = 1
    core::ptr::write_volatile(int_ep_ctx.add(3), (int_ring_ptr >> 32) as u32);
    core::ptr::write_volatile(int_ep_ctx.add(4), 8); // Average TRB Length

    // -- Bulk OUT endpoint context --
    let bulk_out_ep_ctx = input_ctx.add(8 + (bulk_out_dci as usize) * 8);
    core::ptr::write_volatile(bulk_out_ep_ctx.add(0), 0); // No interval for Bulk
    core::ptr::write_volatile(bulk_out_ep_ctx.add(1),
        (3 << 1) | // CErr = 3
        (2 << 3) | // EP Type = Bulk OUT
        ((bt_bulk_out_max_pkt as u32 & 0x7FF) << 16)
    );
    let bulk_out_ring_ptr = bulk_out_ring_phys as u64;
    core::ptr::write_volatile(bulk_out_ep_ctx.add(2), (bulk_out_ring_ptr as u32) | 1); // DCS = 1
    core::ptr::write_volatile(bulk_out_ep_ctx.add(3), (bulk_out_ring_ptr >> 32) as u32);
    core::ptr::write_volatile(bulk_out_ep_ctx.add(4), bt_bulk_out_max_pkt as u32); // Average TRB Length

    // Step 5: Send Configure Endpoint Command
    core::ptr::write_volatile(&raw mut XHCI_LAST_SLOT_ID, 0);
    push_command_trb(dev.input_ctx_phys as u32, (dev.input_ctx_phys as u64 >> 32) as u32, 0,
                     (12 << 10) | (slot_id << 24));

    let t_cfgep = now_ticks();
    while core::ptr::read_volatile(&raw const XHCI_LAST_SLOT_ID) == 0 {
        if ticks_elapsed_since(t_cfgep) >= TICKS_1S { break; }
        rust_xhci_handle_irq(core::ptr::null());
        wait_ticks(1);
    }

    if core::ptr::read_volatile(&raw const XHCI_LAST_SLOT_ID) != 255 {
        print_serial(b"RUST xHCI: Bluetooth Configure Endpoint SUCCESS!\n\0".as_ptr());
        core::ptr::write_volatile(&raw mut XHCI_BT_SLOT, slot_id);

        // Store Interrupt IN state
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_DCI, int_dci);
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_MAX_PKT, bt_int_max_pkt);
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_RING, int_ring as u64);
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_PHYS, int_ring_phys as u64);
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_ENQ, 0);
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_CYCLE, 1);

        // Store Bulk OUT state
        core::ptr::write_volatile(&raw mut XHCI_BT_BULK_OUT_DCI, bulk_out_dci);
        core::ptr::write_volatile(&raw mut XHCI_BT_BULK_OUT_RING, bulk_out_ring as u64);
        core::ptr::write_volatile(&raw mut XHCI_BT_BULK_OUT_PHYS, bulk_out_ring_phys as u64);
        core::ptr::write_volatile(&raw mut XHCI_BT_BULK_OUT_ENQ, 0);
        core::ptr::write_volatile(&raw mut XHCI_BT_BULK_OUT_CYCLE, 1);

        if core::ptr::read_volatile(&raw const XHCI_BT_EVT_DMA_BUF) == 0 {
            let mut dma_phys = 0u32;
            let dma_buf = crate::heap::kmalloc_ap(512, &mut dma_phys) as *mut u8;
            core::ptr::write_bytes(dma_buf, 0, 512);
            core::ptr::write_volatile(&raw mut XHCI_BT_EVT_DMA_BUF, dma_buf as u64);
            core::ptr::write_volatile(&raw mut XHCI_BT_EVT_DMA_PHYS, dma_phys as u64);
        }
        if core::ptr::read_volatile(&raw const XHCI_BT_CMD_DMA_BUF) == 0 {
            let mut dma_phys = 0u32;
            let dma_buf = crate::heap::kmalloc_ap(512, &mut dma_phys) as *mut u8;
            core::ptr::write_bytes(dma_buf, 0, 512);
            core::ptr::write_volatile(&raw mut XHCI_BT_CMD_DMA_BUF, dma_buf as u64);
            core::ptr::write_volatile(&raw mut XHCI_BT_CMD_DMA_PHYS, dma_phys as u64);
        }

        queue_bt_event_transfer();
        dev.configured = true;
        print_serial(b"RUST xHCI: Bluetooth HCI Driver ONLINE & LISTENING!\n\0".as_ptr());
        set_xhci_debug_msg(b"xHCI: Bluetooth HCI active!\0".as_ptr());
        return true;
    } else {
        print_serial(b"RUST xHCI: Bluetooth Configure Endpoint FAILED\n\0".as_ptr());
    }
    false
}

unsafe fn queue_bt_event_transfer() {
    let ring = core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_RING);
    let buf = core::ptr::read_volatile(&raw const XHCI_BT_EVT_DMA_PHYS);
    let slot_id = core::ptr::read_volatile(&raw const XHCI_BT_SLOT);
    let dci = core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_DCI);
    if ring == 0 || buf == 0 || slot_id == 0 || dci == 0 { return; }

    let base = ring as *mut u32;
    let idx = core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_ENQ);
    let cycle = core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_CYCLE);
    let trb = base.add((idx as usize) * 4);

    let req_len = 512u32;
    core::ptr::write_volatile(&raw mut XHCI_BT_INT_REQ_LEN, req_len);

    core::ptr::write_volatile(trb.add(0), buf as u32);
    core::ptr::write_volatile(trb.add(1), (buf >> 32) as u32);
    core::ptr::write_volatile(trb.add(2), req_len);
    let dword3 = (1 << 10) | (1 << 5) | (1 << 2) | cycle; // Normal TRB, IOC, ISP, Cycle
    core::ptr::write_volatile(trb.add(3), dword3);

    let mut next = idx + 1;
    if next == 255 {
        let link = base.add(255 * 4);
        let phys = core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_PHYS);
        core::ptr::write_volatile(link.add(0), phys as u32);
        core::ptr::write_volatile(link.add(1), (phys >> 32) as u32);
        core::ptr::write_volatile(link.add(2), 0);
        core::ptr::write_volatile(link.add(3), (6 << 10) | (1 << 1) | cycle);
        next = 0;
        core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_CYCLE, cycle ^ 1);
    }
    core::ptr::write_volatile(&raw mut XHCI_BT_INT_IN_ENQ, next);

    ring_doorbell(slot_id, dci);
}

unsafe fn xhci_bt_enqueue_event(len: u32) {
    let src = core::ptr::read_volatile(&raw const XHCI_BT_EVT_DMA_BUF) as *const u8;
    if src.is_null() { return; }
    for i in 0..len as usize {
        let next_head = (XHCI_BT_FIFO_HEAD + 1) % BT_FIFO_CAPACITY;
        if next_head != XHCI_BT_FIFO_TAIL {
            XHCI_BT_FIFO[XHCI_BT_FIFO_HEAD] = core::ptr::read_volatile(src.add(i));
            XHCI_BT_FIFO_HEAD = next_head;
        }
    }
}

pub unsafe fn xhci_bt_poll_event(buf: *mut u8, max_len: u32) -> i32 {
    let available = if XHCI_BT_FIFO_HEAD >= XHCI_BT_FIFO_TAIL {
        XHCI_BT_FIFO_HEAD - XHCI_BT_FIFO_TAIL
    } else {
        BT_FIFO_CAPACITY - XHCI_BT_FIFO_TAIL + XHCI_BT_FIFO_HEAD
    };
    if available < 2 {
        return 0;
    }

    // Raw HCI Event framing on USB Interrupt IN (Bluetooth Core Spec Vol 4 Part B):
    // FIFO[TAIL]     = event_code (e.g. 0x0E, 0x0F, 0x02, 0x22, 0x2F, 0x3E)
    // FIFO[TAIL + 1] = param_len
    // FIFO[TAIL + 2..] = params (param_len bytes)
    let param_len = XHCI_BT_FIFO[(XHCI_BT_FIFO_TAIL + 1) % BT_FIFO_CAPACITY] as usize;
    let total_pkt_len = 2 + param_len;

    if available < total_pkt_len {
        return 0; // Wait for full event packet to arrive in FIFO
    }

    let copy_len = if (total_pkt_len as u32) > max_len { max_len as usize } else { total_pkt_len };
    for i in 0..copy_len {
        let byte = XHCI_BT_FIFO[XHCI_BT_FIFO_TAIL];
        XHCI_BT_FIFO_TAIL = (XHCI_BT_FIFO_TAIL + 1) % BT_FIFO_CAPACITY;
        core::ptr::write_volatile(buf.add(i), byte);
    }
    if total_pkt_len > copy_len {
        for _ in 0..(total_pkt_len - copy_len) {
            XHCI_BT_FIFO_TAIL = (XHCI_BT_FIFO_TAIL + 1) % BT_FIFO_CAPACITY;
        }
    }

    copy_len as i32
}

pub unsafe fn xhci_bt_send_cmd(opcode: u16, params: *const u8, param_len: u8) -> i32 {
    let slot_id = core::ptr::read_volatile(&raw const XHCI_BT_SLOT);
    let cmd_buf_ptr = core::ptr::read_volatile(&raw const XHCI_BT_CMD_DMA_BUF);
    let cmd_phys = core::ptr::read_volatile(&raw const XHCI_BT_CMD_DMA_PHYS);
    if slot_id == 0 || cmd_buf_ptr == 0 || cmd_phys == 0 {
        return -1;
    }

    let mut dev_idx = 4usize;
    for i in 0..4 {
        if XHCI_DEVICES[i].slot_id == slot_id {
            dev_idx = i;
            break;
        }
    }
    if dev_idx >= 4 {
        return -1;
    }
    let dev = &mut XHCI_DEVICES[dev_idx];

    let cmd_len = 3 + (param_len as u32);
    let cmd_buf = cmd_buf_ptr as *mut u8;
    core::ptr::write_volatile(cmd_buf.add(0), (opcode & 0xFF) as u8);
    core::ptr::write_volatile(cmd_buf.add(1), ((opcode >> 8) & 0xFF) as u8);
    core::ptr::write_volatile(cmd_buf.add(2), param_len);
    if param_len > 0 && !params.is_null() {
        for i in 0..param_len as usize {
            core::ptr::write_volatile(cmd_buf.add(3 + i), core::ptr::read_volatile(params.add(i)));
        }
    }

    // Bluetooth USB Transport Spec (Vol 4 Part B Section 2.2):
    // All HCI commands are transmitted via Class-specific Control Request on Endpoint 0:
    // bmRequestType = 0x20 (Host-to-Device, Class, Device)
    // bRequest = 0x00
    // wValue = 0x0000
    // wIndex = 0x0000
    // wLength = length of HCI command packet (cmd_len)
    let setup_lo: u32 = 0x20 | (0x00 << 8) | (0 << 16);
    let setup_hi: u32 = cmd_len << 16;

    // Setup Stage TRB: Type=2 (Setup), TRT=2 (OUT Data Stage), IDT=1 (Immediate Data)
    push_ep0_trb(dev, setup_lo, setup_hi, 8, (2 << 10) | (2 << 16) | (1 << 6));
    // Data Stage TRB: Type=3 (Data Stage), DIR=0 (OUT Data Stage), Length=cmd_len
    push_ep0_trb(dev, cmd_phys as u32, (cmd_phys >> 32) as u32,
                 cmd_len, (3 << 10) | (0 << 16));
    // Status Stage TRB: Type=4 (Status Stage), DIR=1 (IN handshake), IOC=1
    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5));

    core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
    ring_doorbell(slot_id, 1);

    let t0 = now_ticks();
    let tsc0 = rdtsc();
    while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
        if ticks_elapsed_since(t0) >= TICKS_500MS || tsc_ms_elapsed(tsc0) >= 500 { break; }
        rust_xhci_handle_irq(core::ptr::null());
        wait_ticks(1);
    }

    if core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 1 {
        0
    } else {
        -2
    }
}

pub unsafe fn xhci_bt_is_present() -> i32 {
    if core::ptr::read_volatile(&raw const XHCI_BT_SLOT) != 0 &&
       core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_RING) != 0 {
        1
    } else {
        0
    }
}

/// Hot-plug rescan: re-scan all xHCI ports for newly attached devices.
/// Called from C when the user runs `phone status` and no MTP device is known yet.
#[no_mangle]
pub unsafe extern "C" fn rust_xhci_rescan() -> u32 {
    if XHCI_PORTS_BASE == 0 || XHCI_MAX_PORTS == 0 {
        print_serial(b"RUST xHCI rescan: Controller not initialized\n\0".as_ptr());
        return 0;
    }

    // Already have an MTP device?
    if core::ptr::read_volatile(&raw const XHCI_MTP_SLOT) != 0 {
        print_serial(b"RUST xHCI rescan: MTP already connected\n\0".as_ptr());
        return 1;
    }

    print_serial(b"RUST xHCI rescan: Scanning for hot-plugged devices...\n\0".as_ptr());

    // Power ports again in case new device needs it
    power_ports_once();

    // Wait a bit for port to settle
    wait_ticks(50);

    // Scan ports
    let mut connected_mask = 0u32;
    let mut pass = 0u32;
    while pass < 6 {
        connected_mask = scan_ports_connected();
        if connected_mask != 0 {
            break;
        }
        pass += 1;
        wait_ticks(50);
    }

    let mut buf = [0u8; 16];
    if connected_mask == 0 {
        print_serial(b"RUST xHCI rescan: No devices found on any port\n\0".as_ptr());
        return 0;
    }

    print_serial(b"RUST xHCI rescan: Device mask = \0".as_ptr());
    print_serial(format_num(connected_mask as usize, &mut buf).as_ptr());
    print_serial(b"\n\0".as_ptr());

    // Enumerate new ports (skip ports that already have devices)
    let mut found_new = 0u32;
    let mut p = 1u32;
    while p <= XHCI_MAX_PORTS && p <= 32 {
        if connected_mask & (1 << (p - 1)) != 0 {
            // Check if this port already has an active, configured device
            let mut already_known = false;
            for i in 0..XHCI_NUM_DEVICES as usize {
                if XHCI_DEVICES[i].port == p && XHCI_DEVICES[i].slot_id != 0 {
                    already_known = true;
                    break;
                }
            }
            if !already_known {
                print_serial(b"RUST xHCI rescan: New device on port \0".as_ptr());
                print_serial(format_num(p as usize, &mut buf).as_ptr());
                print_serial(b"\n\0".as_ptr());
                if try_enumerate_port(p) {
                    found_new += 1;
                }
            }
        }
        p += 1;
    }

    if found_new == 0 {
        print_serial(b"RUST xHCI rescan: No new devices enumerated\n\0".as_ptr());
        return 0;
    }

    // Phase 4 for new devices: identify and configure MTP
    for i in 0..XHCI_NUM_DEVICES as usize {
        if configure_mtp_device(i) {
            return 1;
        }
    }
    0
}

/// Helper to identify and configure an MTP mobile phone on an enumerated device
unsafe fn configure_mtp_device(dev_idx: usize) -> bool {
    let mut buf = [0u8; 16];
    let dev = &mut XHCI_DEVICES[dev_idx];
    let slot_id = dev.slot_id;
    if slot_id == 0 || dev.configured {
        return false;
    }
    if core::ptr::read_volatile(&raw const XHCI_MTP_SLOT) != 0 {
        return false;
    }

    // Get Device Descriptor (18 bytes)
    let mut desc_phys = 0u32;
    let desc_buf = crate::heap::kmalloc_ap(64, &mut desc_phys) as *mut u8;
    core::ptr::write_bytes(desc_buf, 0, 64);

    let setup_lo: u32 = 0x80 | (6 << 8) | (0x0100 << 16);
    let setup_hi: u32 = 0 | (18 << 16);
    push_ep0_trb(dev, setup_lo, setup_hi, 8, (2 << 10) | (3 << 16) | (1 << 6));
    push_ep0_trb(dev, desc_phys, (desc_phys as u64 >> 32) as u32, 18, (3 << 10) | (1 << 16) | (1 << 5));
    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 5));

    core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
    ring_doorbell(slot_id, 1);
    let t0 = now_ticks();
    while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
        if ticks_elapsed_since(t0) >= TICKS_1S { break; }
        rust_xhci_handle_irq(core::ptr::null());
        wait_ticks(1);
    }

    if core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
        print_serial(b"RUST xHCI: GET_DESCRIPTOR (device) timed out\n\0".as_ptr());
        return false;
    }

    let vid = (core::ptr::read_volatile(desc_buf.add(9)) as u16) << 8 |
               core::ptr::read_volatile(desc_buf.add(8)) as u16;
    let pid = (core::ptr::read_volatile(desc_buf.add(11)) as u16) << 8 |
               core::ptr::read_volatile(desc_buf.add(10)) as u16;

    print_serial(b"RUST xHCI: Dev VID=\0".as_ptr());
    print_serial(format_hex(vid as u32, &mut buf).as_ptr());
    print_serial(b" PID=\0".as_ptr());
    print_serial(format_hex(pid as u32, &mut buf).as_ptr());
    print_serial(b"\n\0".as_ptr());

    // Get Configuration Descriptor (first pass: 64 bytes)
    let mut cfg_buf_phys = 0u32;
    let cfg_buf = crate::heap::kmalloc_ap(512, &mut cfg_buf_phys) as *mut u8;
    core::ptr::write_bytes(cfg_buf, 0, 512);

    let setup_lo2: u32 = 0x80 | (6 << 8) | (0x0200 << 16);
    let setup_hi2: u32 = 0 | (64 << 16);
    push_ep0_trb(dev, setup_lo2, setup_hi2, 8, (2 << 10) | (3 << 16) | (1 << 6));
    push_ep0_trb(dev, cfg_buf_phys, (cfg_buf_phys as u64 >> 32) as u32, 64, (3 << 10) | (1 << 16) | (1 << 5));
    push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 5));

    core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
    ring_doorbell(slot_id, 1);
    let t1 = now_ticks();
    while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
        if ticks_elapsed_since(t1) >= TICKS_1S { break; }
        rust_xhci_handle_irq(core::ptr::null());
        wait_ticks(1);
    }

    if core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
        print_serial(b"RUST xHCI: GET_DESCRIPTOR (cfg) timed out\n\0".as_ptr());
        return false;
    }

    let total_len = ((core::ptr::read_volatile(cfg_buf.add(3)) as u16) << 8
                  | core::ptr::read_volatile(cfg_buf.add(2)) as u16) as usize;
    let mut parse_buf = cfg_buf;
    let mut parse_len = if total_len < 64 { total_len } else { 64 };

    if total_len > 64 && total_len <= 512 {
        let setup_lo_full: u32 = 0x80 | (6 << 8) | (0x0200 << 16);
        let setup_hi_full: u32 = (total_len as u32) << 16;
        push_ep0_trb(dev, setup_lo_full, setup_hi_full, 8, (2 << 10) | (3 << 16) | (1 << 6));
        push_ep0_trb(dev, cfg_buf_phys, (cfg_buf_phys as u64 >> 32) as u32, total_len as u32, (3 << 10) | (1 << 16) | (1 << 5));
        push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 5));

        core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
        ring_doorbell(slot_id, 1);
        let t_full = now_ticks();
        while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
            if ticks_elapsed_since(t_full) >= TICKS_1S { break; }
            rust_xhci_handle_irq(core::ptr::null());
            wait_ticks(1);
        }
        if core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) != 0 {
            parse_len = total_len;
        }
    }

    // Parse config descriptor for MTP interface
    let mut offset = 0usize;
    let mut is_mtp = false;
    let mut found_mtp_in = false;
    let mut found_mtp_out = false;
    let mut mtp_in_addr: u8 = 0;
    let mut mtp_out_addr: u8 = 0;
    let mut mtp_in_max_pkt: u16 = 512;
    let mut mtp_out_max_pkt: u16 = 512;

    while offset + 1 < parse_len {
        let desc_len = core::ptr::read_volatile(parse_buf.add(offset)) as usize;
        let desc_type = core::ptr::read_volatile(parse_buf.add(offset + 1));
        if desc_len == 0 { break; }

        if desc_type == 4 && desc_len >= 9 {
            let iface_class = core::ptr::read_volatile(parse_buf.add(offset + 5));
            let iface_subclass = core::ptr::read_volatile(parse_buf.add(offset + 6));
            let iface_protocol = core::ptr::read_volatile(parse_buf.add(offset + 7));

            print_serial(b"RUST xHCI: Iface Class=\0".as_ptr());
            print_serial(format_num(iface_class as usize, &mut buf).as_ptr());
            print_serial(b" Sub=\0".as_ptr());
            print_serial(format_num(iface_subclass as usize, &mut buf).as_ptr());
            print_serial(b" Proto=\0".as_ptr());
            print_serial(format_num(iface_protocol as usize, &mut buf).as_ptr());
            print_serial(b"\n\0".as_ptr());

            is_mtp = (iface_class == 6 && iface_subclass == 1 && iface_protocol == 1)
                  || (iface_class == 6 && iface_subclass == 1 && iface_protocol == 0)
                  || (iface_class == 0xFF && iface_subclass == 0xFF)
                  || (iface_class == 0xEF && iface_subclass == 0x02 && iface_protocol == 0x01);
            if is_mtp {
                print_serial(b"RUST xHCI: *** MTP INTERFACE FOUND! ***\n\0".as_ptr());
            }
        }

        if desc_type == 5 && desc_len >= 7 && is_mtp {
            let cur_ep_addr = core::ptr::read_volatile(parse_buf.add(offset + 2));
            let cur_ep_attribs = core::ptr::read_volatile(parse_buf.add(offset + 3));
            let cur_ep_max_pkt = (core::ptr::read_volatile(parse_buf.add(offset + 5)) as u16) << 8
                               | (core::ptr::read_volatile(parse_buf.add(offset + 4)) as u16);

            if (cur_ep_attribs & 0x03) == 0x02 {
                if (cur_ep_addr & 0x80) != 0 {
                    mtp_in_addr = cur_ep_addr;
                    mtp_in_max_pkt = cur_ep_max_pkt;
                    found_mtp_in = true;
                    print_serial(b"RUST xHCI: Found MTP Bulk IN\n\0".as_ptr());
                } else {
                    mtp_out_addr = cur_ep_addr;
                    mtp_out_max_pkt = cur_ep_max_pkt;
                    found_mtp_out = true;
                    print_serial(b"RUST xHCI: Found MTP Bulk OUT\n\0".as_ptr());
                }
            }
        }

        offset += desc_len;
    }

    if found_mtp_in && found_mtp_out {
        print_serial(b"RUST xHCI: Configuring MTP endpoints...\n\0".as_ptr());

        // SET_CONFIGURATION
        let setup_cfg: u32 = 0x00 | (9 << 8) | (1 << 16);
        push_ep0_trb(dev, setup_cfg, 0, 8, (2 << 10) | (0 << 16) | (1 << 6));
        push_ep0_trb(dev, 0, 0, 0, (4 << 10) | (1 << 16) | (1 << 5));
        core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 0);
        ring_doorbell(slot_id, 1);
        let tc = now_ticks();
        while core::ptr::read_volatile(&raw const XHCI_TRANSFER_DONE) == 0 {
            if ticks_elapsed_since(tc) >= TICKS_1S { break; }
            rust_xhci_handle_irq(core::ptr::null());
            wait_ticks(1);
        }

        let ep_in_num = (mtp_in_addr & 0x0F) as u32;
        let dci_in = ep_in_num * 2 + 1;
        let ep_out_num = (mtp_out_addr & 0x0F) as u32;
        let dci_out = ep_out_num * 2;
        let max_dci = if dci_in > dci_out { dci_in } else { dci_out };

        let mut in_ring_phys = 0u32;
        let in_ring = crate::heap::kmalloc_ap(4096, &mut in_ring_phys) as *mut u32;
        core::ptr::write_bytes(in_ring, 0, 1024);
        let mut out_ring_phys = 0u32;
        let out_ring = crate::heap::kmalloc_ap(4096, &mut out_ring_phys) as *mut u32;
        core::ptr::write_bytes(out_ring, 0, 1024);

        let input_ctx = dev.input_ctx as *mut u32;
        core::ptr::write_bytes(input_ctx, 0, 1024);
        core::ptr::write_volatile(input_ctx.add(1), (1 << 0) | (1 << dci_in) | (1 << dci_out));

        let slot_ctx = input_ctx.add(8);
        core::ptr::write_volatile(slot_ctx.add(0), (max_dci << 27) | ((dev.speed & 0xF) << 20));
        core::ptr::write_volatile(slot_ctx.add(1), ((dev.port) & 0xFF) << 16);

        let ep_in_ctx = input_ctx.add(8 + (dci_in as usize) * 8);
        core::ptr::write_volatile(ep_in_ctx.add(0), 0);
        core::ptr::write_volatile(ep_in_ctx.add(1), (3 << 1) | (6 << 3) | ((mtp_in_max_pkt as u32 & 0x7FF) << 16));
        let in_ptr = in_ring_phys as u64;
        core::ptr::write_volatile(ep_in_ctx.add(2), (in_ptr as u32) | 1);
        core::ptr::write_volatile(ep_in_ctx.add(3), (in_ptr >> 32) as u32);
        core::ptr::write_volatile(ep_in_ctx.add(4), mtp_in_max_pkt as u32);

        let ep_out_ctx = input_ctx.add(8 + (dci_out as usize) * 8);
        core::ptr::write_volatile(ep_out_ctx.add(0), 0);
        core::ptr::write_volatile(ep_out_ctx.add(1), (3 << 1) | (2 << 3) | ((mtp_out_max_pkt as u32 & 0x7FF) << 16));
        let out_ptr = out_ring_phys as u64;
        core::ptr::write_volatile(ep_out_ctx.add(2), (out_ptr as u32) | 1);
        core::ptr::write_volatile(ep_out_ctx.add(3), (out_ptr >> 32) as u32);
        core::ptr::write_volatile(ep_out_ctx.add(4), mtp_out_max_pkt as u32);

        core::ptr::write_volatile(&raw mut XHCI_LAST_SLOT_ID, 0);
        push_command_trb(dev.input_ctx_phys as u32, (dev.input_ctx_phys as u64 >> 32) as u32, 0,
                         (12 << 10) | (slot_id << 24));

        let te = now_ticks();
        while core::ptr::read_volatile(&raw const XHCI_LAST_SLOT_ID) == 0 {
            if ticks_elapsed_since(te) >= TICKS_1S { break; }
            rust_xhci_handle_irq(core::ptr::null());
            wait_ticks(1);
        }

        if core::ptr::read_volatile(&raw const XHCI_LAST_SLOT_ID) != 255 {
            print_serial(b"RUST xHCI: MTP Configure Endpoint SUCCESS!\n\0".as_ptr());
            core::ptr::write_volatile(&raw mut XHCI_MTP_SLOT, slot_id);
            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DCI, dci_in);
            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DCI, dci_out);
            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_RING, in_ring as u64);
            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_PHYS, in_ring_phys as u64);
            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_ENQ, 0);
            core::ptr::write_volatile(&raw mut XHCI_MTP_IN_CYCLE, 1);
            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_RING, out_ring as u64);
            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_PHYS, out_ring_phys as u64);
            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_ENQ, 0);
            core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_CYCLE, 1);

            if core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DMA_BUF) == 0 {
                let mut dma_phys = 0u32;
                let dma_buf = crate::heap::kmalloc_ap(16384, &mut dma_phys) as *mut u8;
                core::ptr::write_bytes(dma_buf, 0, 16384);
                core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DMA_BUF, dma_buf as u64);
                core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DMA_PHYS, dma_phys as u64);
            }
            if core::ptr::read_volatile(&raw const XHCI_MTP_IN_DMA_BUF) == 0 {
                let mut dma_phys = 0u32;
                let dma_buf = crate::heap::kmalloc_ap(16384, &mut dma_phys) as *mut u8;
                core::ptr::write_bytes(dma_buf, 0, 16384);
                core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DMA_BUF, dma_buf as u64);
                core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DMA_PHYS, dma_phys as u64);
            }
            print_serial(b"RUST xHCI: Mobile Data Transfer Ready!\n\0".as_ptr());
            dev.configured = true;
            return true;
        } else {
            print_serial(b"RUST xHCI: MTP Configure Endpoint FAILED\n\0".as_ptr());
        }
    }
    false
}

/// Instantaneous non-blocking hotplug polling.
/// Returns:
///   1 = MTP Phone connected
///  -1 = MTP Phone disconnected
///   0 = No change
#[no_mangle]
pub unsafe extern "C" fn rust_xhci_poll_hotplug() -> i32 {
    if XHCI_PORTS_BASE == 0 || XHCI_MAX_PORTS == 0 {
        return 0;
    }

    let mtp_slot = core::ptr::read_volatile(&raw const XHCI_MTP_SLOT);

    // 1. Check if any previously registered device disconnected
    for i in 0..4 {
        let slot = XHCI_DEVICES[i].slot_id;
        let port = XHCI_DEVICES[i].port;
        if slot != 0 && port >= 1 && port <= XHCI_MAX_PORTS {
            let paddr = portsc_addr(port);
            let status = core::ptr::read_volatile(paddr);
            if status & PORTSC_CCS == 0 {
                let was_mtp = (slot == mtp_slot);
                let mut buf = [0u8; 16];
                print_serial(b"RUST xHCI: Device disconnected on port \0".as_ptr());
                print_serial(format_num(port as usize, &mut buf).as_ptr());
                print_serial(b"\n\0".as_ptr());

                // Disable slot command (Type 10)
                push_command_trb(0, 0, 0, (10 << 10) | (slot << 24));
                let dcbaa = XHCI_DCBAAP as *mut u64;
                if dcbaa as usize != 0 {
                    core::ptr::write_volatile(dcbaa.add(slot as usize), 0);
                }

                // Clear device record
                XHCI_DEVICES[i].slot_id = 0;
                XHCI_DEVICES[i].port = 0;
                XHCI_DEVICES[i].configured = false;

                if was_mtp {
                    core::ptr::write_volatile(&raw mut XHCI_MTP_SLOT, 0);
                    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DCI, 0);
                    core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DCI, 0);
                    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_RING, 0);
                    core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_RING, 0);
                    return -1; // Phone disconnected!
                }
            }
        }
    }

    // 2. Scan ports for any newly connected device
    let max_ports = if XHCI_MAX_PORTS > 32 { 32 } else { XHCI_MAX_PORTS };
    let mut p = 1u32;
    while p <= max_ports {
        let paddr = portsc_addr(p);
        let status = core::ptr::read_volatile(paddr);
        if status & PORTSC_CCS != 0 {
            // Acknowledge Connect Status Change (CSC) if set
            let w1c_mask = 0x00FE0000u32;
            if status & PORTSC_CSC != 0 {
                core::ptr::write_volatile(paddr, (status & !w1c_mask) | PORTSC_CSC);
            }

            // Check if this port already has an active device (slot allocated)
            let mut known = false;
            for i in 0..4 {
                if XHCI_DEVICES[i].port == p && XHCI_DEVICES[i].slot_id != 0 {
                    known = true;
                    break;
                }
            }
            if !known {
                let mut buf = [0u8; 16];
                print_serial(b"RUST xHCI hotplug: New device on port \0".as_ptr());
                print_serial(format_num(p as usize, &mut buf).as_ptr());
                print_serial(b"\n\0".as_ptr());

                if try_enumerate_port(p) {
                    let mut configured_ok = false;
                    for i in 0..4 {
                        if XHCI_DEVICES[i].port == p && XHCI_DEVICES[i].slot_id != 0 && !XHCI_DEVICES[i].configured {
                            if configure_mtp_device(i) {
                                configured_ok = true;
                                return 1; // Phone connected!
                            }
                        }
                    }
                    if !configured_ok {
                        // Slot failed configuration - disable slot to prevent slot exhaustion
                        for i in 0..4 {
                            if XHCI_DEVICES[i].port == p && XHCI_DEVICES[i].slot_id != 0 && !XHCI_DEVICES[i].configured {
                                let slot = XHCI_DEVICES[i].slot_id;
                                push_command_trb(0, 0, 0, (10 << 10) | (slot << 24));
                                let dcbaa = XHCI_DCBAAP as *mut u64;
                                if dcbaa as usize != 0 {
                                    core::ptr::write_volatile(dcbaa.add(slot as usize), 0);
                                }
                                XHCI_DEVICES[i].slot_id = 0;
                                XHCI_DEVICES[i].port = 0;
                            }
                        }
                    }
                }
            }
        }
        p += 1;
    }

    0
}

pub unsafe fn xhci_mtp_is_connected() -> i32 {
    if core::ptr::read_volatile(&raw const XHCI_MTP_SLOT) != 0 &&
       core::ptr::read_volatile(&raw const XHCI_MTP_IN_RING) != 0 &&
       core::ptr::read_volatile(&raw const XHCI_MTP_OUT_RING) != 0 {
        1
    } else {
        0
    }
}

pub unsafe fn xhci_mtp_write(data: *const u8, len: u32) -> i32 {
    if core::ptr::read_volatile(&raw const XHCI_MTP_SLOT) == 0 ||
       core::ptr::read_volatile(&raw const XHCI_MTP_OUT_RING) == 0 || len == 0 {
        return -1;
    }
    let copy_len = if len > 16384 { 16384 } else { len };
    core::ptr::copy_nonoverlapping(data, core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DMA_BUF) as *mut u8, copy_len as usize);

    let base = core::ptr::read_volatile(&raw const XHCI_MTP_OUT_RING) as *mut u32;
    let idx = core::ptr::read_volatile(&raw const XHCI_MTP_OUT_ENQ);
    let cycle = core::ptr::read_volatile(&raw const XHCI_MTP_OUT_CYCLE);
    let trb = base.add((idx as usize) * 4);

    let buf_phys = core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DMA_PHYS);
    core::ptr::write_volatile(trb.add(0), buf_phys as u32);
    core::ptr::write_volatile(trb.add(1), (buf_phys >> 32) as u32);
    core::ptr::write_volatile(trb.add(2), copy_len);
    // Type=1 (Normal), IOC = (1 << 5), Cycle
    let dword3 = (1 << 10) | (1 << 5) | cycle;
    core::ptr::write_volatile(trb.add(3), dword3);

    let mut next = idx + 1;
    if next == 255 {
        let link = base.add(255 * 4);
        let out_phys = core::ptr::read_volatile(&raw const XHCI_MTP_OUT_PHYS);
        core::ptr::write_volatile(link.add(0), out_phys as u32);
        core::ptr::write_volatile(link.add(1), (out_phys >> 32) as u32);
        core::ptr::write_volatile(link.add(2), 0);
        core::ptr::write_volatile(link.add(3), (6 << 10) | (1 << 1) | cycle);
        next = 0;
        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_CYCLE, cycle ^ 1);
    }
    core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_ENQ, next);

    let slot = core::ptr::read_volatile(&raw const XHCI_MTP_SLOT);
    let out_dci = core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DCI);
    // Commented out per-command serial spew to prevent serial port stalling during bulk MTP transfers
    let mut dbuf = [0u8; 16];
    print_serial(b"RUST xHCI: mtp_write slot=\0".as_ptr());
    print_serial(format_num(slot as usize, &mut dbuf).as_ptr());
    print_serial(b" dci=\0".as_ptr());
    print_serial(format_num(out_dci as usize, &mut dbuf).as_ptr());
    print_serial(b" len=\0".as_ptr());
    print_serial(format_num(copy_len as usize, &mut dbuf).as_ptr());
    print_serial(b"\n\0".as_ptr());

    core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DONE, 0);
    ring_doorbell(slot, out_dci);

    let t_start = now_ticks();
    while core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DONE) == 0 {
        kernel_heartbeat();
        if ticks_elapsed_since(t_start) >= 5 {
            // Bulk OUT TRB is queued and doorbell rung.
            // On Windows WinUSB, Bulk OUT completion is delivered when
            // the subsequent Bulk IN transfer is submitted.
            break;
        }
        rust_xhci_handle_irq(core::ptr::null());
        if core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DONE) != 0 {
            break;
        }
        wait_ticks(1);
    }
    copy_len as i32
}

pub unsafe fn xhci_mtp_read_timeout(buffer: *mut u8, max_len: u32, timeout_ms: u32) -> i32 {
    if core::ptr::read_volatile(&raw const XHCI_MTP_SLOT) == 0 ||
       core::ptr::read_volatile(&raw const XHCI_MTP_IN_RING) == 0 || max_len == 0 {
        return -1;
    }

    // Check if an IN transfer already completed into our DMA buffer
    if core::ptr::read_volatile(&raw const XHCI_MTP_IN_DONE) != 0 {
        let actual = core::ptr::read_volatile(&raw const XHCI_MTP_IN_TRANSFERRED);
        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DONE, 0);
        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_TRANSFERRED, 0);
        if actual > 0 {
            let copy = actual.min(max_len);
            core::ptr::copy_nonoverlapping(core::ptr::read_volatile(&raw const XHCI_MTP_IN_DMA_BUF) as *const u8, buffer, copy as usize);
            return copy as i32;
        }
        return 0;
    }

    let req_len = if max_len > 16384 { 16384 } else { max_len };
    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_REQ_LEN, req_len);

    let base = core::ptr::read_volatile(&raw const XHCI_MTP_IN_RING) as *mut u32;
    let idx = core::ptr::read_volatile(&raw const XHCI_MTP_IN_ENQ);
    let cycle = core::ptr::read_volatile(&raw const XHCI_MTP_IN_CYCLE);
    let trb = base.add((idx as usize) * 4);

    let buf_phys = core::ptr::read_volatile(&raw const XHCI_MTP_IN_DMA_PHYS);
    core::ptr::write_volatile(trb.add(0), buf_phys as u32);
    core::ptr::write_volatile(trb.add(1), (buf_phys >> 32) as u32);
    core::ptr::write_volatile(trb.add(2), req_len);
    // Type=1 (Normal), IOC = (1 << 5), ISP = (1 << 2), Cycle
    let dword3 = (1 << 10) | (1 << 5) | (1 << 2) | cycle;
    core::ptr::write_volatile(trb.add(3), dword3);

    let mut next = idx + 1;
    if next == 255 {
        let link = base.add(255 * 4);
        let in_phys = core::ptr::read_volatile(&raw const XHCI_MTP_IN_PHYS);
        core::ptr::write_volatile(link.add(0), in_phys as u32);
        core::ptr::write_volatile(link.add(1), (in_phys >> 32) as u32);
        core::ptr::write_volatile(link.add(2), 0);
        core::ptr::write_volatile(link.add(3), (6 << 10) | (1 << 1) | cycle);
        next = 0;
        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_CYCLE, cycle ^ 1);
    }
    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_ENQ, next);

    let slot = core::ptr::read_volatile(&raw const XHCI_MTP_SLOT);
    let in_dci = core::ptr::read_volatile(&raw const XHCI_MTP_IN_DCI);
    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DONE, 0);
    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_TRANSFERRED, 0);
    ring_doorbell(slot, in_dci);

    let max_ticks = (timeout_ms / 4).max(1);
    let t_start = now_ticks();
    let mut poll_counter: u32 = 0;
    while core::ptr::read_volatile(&raw const XHCI_MTP_IN_DONE) == 0 {
        kernel_heartbeat();
        rust_xhci_handle_irq(core::ptr::null());
        if core::ptr::read_volatile(&raw const XHCI_MTP_IN_DONE) != 0 {
            break;
        }
        if ticks_elapsed_since(t_start) >= max_ticks {
            return -2;
        }
        // Process UI events every 4th iteration to keep the interface responsive
        poll_counter += 1;
        if poll_counter & 3 == 0 {
            kernel_poll_events();
        }
        wait_ticks(1);
    }

    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DONE, 0);
    let actual = core::ptr::read_volatile(&raw const XHCI_MTP_IN_TRANSFERRED);
    core::ptr::write_volatile(&raw mut XHCI_MTP_IN_TRANSFERRED, 0);
    if actual > 0 {
        let copy = actual.min(max_len);
        core::ptr::copy_nonoverlapping(core::ptr::read_volatile(&raw const XHCI_MTP_IN_DMA_BUF) as *const u8, buffer, copy as usize);
        return copy as i32;
    }
    0
}

pub unsafe fn xhci_mtp_read(buffer: *mut u8, max_len: u32) -> i32 {
    xhci_mtp_read_timeout(buffer, max_len, 5000)
}

// Diagnostics shown on the lockscreen panel via rust_xhci_diag():
// low 16 bits = init stage (1=running, 2=slot, 3=addressed, 4=kbd found,
// 5=interrupt EP active), high 16 bits = HID reports consumed.
static mut XHCI_STAGE: u32 = 0;
static mut XHCI_KB_REPORTS: u32 = 0;

// Round-2 hardware diagnostics (visible on the lockscreen panel without a
// serial console): last scanned port + its raw PORTSC, last command
// completion code + slot.
static mut XHCI_LAST_SCAN_PORT: u32 = 0;
static mut XHCI_LAST_SCAN_PORTSC: u32 = 0;
static mut XHCI_LAST_CMD_CODE: u32 = 0;
static mut XHCI_LAST_CMD_SLOT: u32 = 0;
static mut XHCI_CMD_MSG: [u8; 64] = [0u8; 64];

#[no_mangle]
pub extern "C" fn rust_xhci_diag() -> u32 {
    unsafe { (XHCI_STAGE & 0xFFFF) | ((XHCI_KB_REPORTS & 0xFFFF) << 16) }
}

// Bits 31:24 = last scanned port, bits 23:0 = that port's raw PORTSC.
#[no_mangle]
pub extern "C" fn rust_xhci_diag2() -> u32 {
    unsafe {
        ((XHCI_LAST_SCAN_PORT & 0xFF) << 24) | (XHCI_LAST_SCAN_PORTSC & 0x00FFFFFF)
    }
}

// Bits 15:8 = last command completion code, bits 7:0 = slot id.
#[no_mangle]
pub extern "C" fn rust_xhci_diag3() -> u32 {
    unsafe { ((XHCI_LAST_CMD_CODE & 0xFF) << 8) | (XHCI_LAST_CMD_SLOT & 0xFF) }
}

unsafe fn format_cmd_fail_msg(comp_code: u32) {
    let prefix = b"xHCI: Cmd FAILED code ";
    let mut m = 0usize;
    let mut i = 0;
    while i < prefix.len() && m < 63 {
        XHCI_CMD_MSG[m] = prefix[i];
        m += 1;
        i += 1;
    }
    let mut nbuf = [0u8; 16];
    let num = format_num(comp_code as usize, &mut nbuf);
    let mut j = 0;
    while j < num.len() && m < 63 {
        XHCI_CMD_MSG[m] = num[j];
        m += 1;
        j += 1;
    }
    if m > 0 && XHCI_CMD_MSG[m - 1] != 0 {
        XHCI_CMD_MSG[m] = 0;
    }
}

// USB HID keycode to PS/2 Set 1 scancode translation table
// Index = USB HID usage ID, Value = PS/2 make code
static USB_TO_PS2: [u8; 104] = [
    0x00, 0x00, 0x00, 0x00, 0x1E, 0x30, 0x2E, 0x20, // 0x00-0x07: None, errors, a, b, c, d
    0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, // 0x08-0x0F: e, f, g, h, i, j, k, l
    0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, // 0x10-0x17: m, n, o, p, q, r, s, t
    0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x02, 0x03, // 0x18-0x1F: u, v, w, x, y, z, 1, 2
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, // 0x20-0x27: 3, 4, 5, 6, 7, 8, 9, 0
    0x1C, 0x01, 0x0E, 0x0F, 0x39, 0x0C, 0x0D, 0x1A, // 0x28-0x2F: Enter, Esc, Bksp, Tab, Space, -, =, [
    0x1B, 0x2B, 0x2B, 0x27, 0x28, 0x29, 0x33, 0x34, // 0x30-0x37: ], \, #, ;, ', `, ,, .
    0x35, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, // 0x38-0x3F: /, CapsLk, F1, F2, F3, F4, F5, F6
    0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x37, 0x46, // 0x40-0x47: F7, F8, F9, F10, F11, F12, PrtSc, ScrLk
    0x00, 0x52, 0x47, 0x49, 0x53, 0x4F, 0x51, 0x4D, // 0x48-0x4F: Pause, Ins, Home, PgUp, Del, End, PgDn, Right
    0x4B, 0x50, 0x48, 0x45, 0x35, 0x37, 0x4A, 0x4E, // 0x50-0x57: Left, Down, Up, NumLk, KP/, KP*, KP-, KP+
    0x1C, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, // 0x58-0x5F: KPEnt, KP1-3, KP4, KP5, KP6, KP7
    0x48, 0x49, 0x52, 0x53, 0x56, 0x00, 0x00, 0x00, // 0x60-0x67: KP8, KP9, KP0, KP., \|, App, Pwr, KP=
];

// Modifier key USB HID bit positions -> PS/2 make codes
static MOD_TO_PS2: [u8; 8] = [
    0x1D, // Left Ctrl
    0x2A, // Left Shift
    0x38, // Left Alt
    0x00, // Left GUI (no PS/2 equivalent in set 1 basic)
    0x1D, // Right Ctrl  (same scancode, extended in real PS/2)
    0x36, // Right Shift
    0x38, // Right Alt
    0x00, // Right GUI
];

extern "C" {
    fn receive_keyboard_byte(scancode: u8);
}

// ---- Helper functions ----

unsafe fn push_ep0_trb(dev: &mut XhciDevice, dword0: u32, dword1: u32, dword2: u32, mut dword3: u32) {
    let base = dev.ep0_ring as *mut u32;
    let idx = dev.ep0_enq;
    let cycle = dev.ep0_cycle;

    dword3 &= !1;
    dword3 |= cycle;

    let trb = base.add((idx as usize) * 4);
    core::ptr::write_volatile(trb.add(0), dword0);
    core::ptr::write_volatile(trb.add(1), dword1);
    core::ptr::write_volatile(trb.add(2), dword2);
    core::ptr::write_volatile(trb.add(3), dword3);

    let mut next = idx + 1;
    if next == 255 {
        // Write Link TRB
        let link = base.add(255 * 4);
        core::ptr::write_volatile(link.add(0), dev.ep0_ring_phys as u32);
        core::ptr::write_volatile(link.add(1), (dev.ep0_ring_phys >> 32) as u32);
        core::ptr::write_volatile(link.add(2), 0);
        core::ptr::write_volatile(link.add(3), (6 << 10) | (1 << 1) | cycle);
        next = 0;
        dev.ep0_cycle ^= 1;
    }
    dev.ep0_enq = next;
}

unsafe fn ring_doorbell(slot_id: u32, target: u32) {
    let db_off = core::ptr::read_volatile((XHCI_MMIO_BASE + 0x14) as *const u32) & !0x3;
    let db_addr = (XHCI_MMIO_BASE + db_off as usize + (slot_id as usize) * 4) as *mut u32;
    core::ptr::write_volatile(db_addr, target);
}

unsafe fn queue_keyboard_transfer() {
    if XHCI_KB_INT_RING == 0 || XHCI_KB_HID_BUF == 0 { return; }

    let base = XHCI_KB_INT_RING as *mut u32;
    let idx = XHCI_KB_INT_ENQ;
    let cycle = XHCI_KB_INT_CYCLE;

    let buf = XHCI_KB_HID_PHYS;
    let trb = base.add((idx as usize) * 4);

    // Normal TRB (Type 1): pointer to HID buffer, length = 8, IOC = 1, ISP = 1
    core::ptr::write_volatile(trb.add(0), buf as u32);
    core::ptr::write_volatile(trb.add(1), (buf >> 32) as u32);
    core::ptr::write_volatile(trb.add(2), 8); // Transfer Length = 8 bytes (boot keyboard report)
    let dword3 = (1 << 10) | (1 << 5) | (1 << 2) | cycle; // Type=1 (Normal), IOC, ISP, Cycle
    core::ptr::write_volatile(trb.add(3), dword3);

    let mut next = idx + 1;
    if next == 255 {
        let link = base.add(255 * 4);
        core::ptr::write_volatile(link.add(0), XHCI_KB_INT_PHYS as u32);
        core::ptr::write_volatile(link.add(1), (XHCI_KB_INT_PHYS >> 32) as u32);
        core::ptr::write_volatile(link.add(2), 0);
        core::ptr::write_volatile(link.add(3), (6 << 10) | (1 << 1) | cycle);
        next = 0;
        XHCI_KB_INT_CYCLE ^= 1;
    }
    XHCI_KB_INT_ENQ = next;

    // Ring doorbell for keyboard slot, target = DCI of interrupt endpoint
    ring_doorbell(XHCI_KB_SLOT, XHCI_KB_DCI);
}

unsafe fn push_command_trb(dword0: u32, dword1: u32, dword2: u32, mut dword3: u32) {
    let base = XHCI_CMD_RING_BASE as *mut u32;
    let mut idx = XHCI_CMD_INDEX;
    let cycle = XHCI_CMD_CYCLE;

    dword3 &= !1;
    dword3 |= cycle;

    let trb = base.add((idx as usize) * 4);
    core::ptr::write_volatile(trb.add(0), dword0);
    core::ptr::write_volatile(trb.add(1), dword1);
    core::ptr::write_volatile(trb.add(2), dword2);
    core::ptr::write_volatile(trb.add(3), dword3);

    idx += 1;
    if idx == 255 {
        let link_trb = base.add(255 * 4);
        core::ptr::write_volatile(link_trb.add(0), XHCI_CMD_RING_PHYS as u32);
        core::ptr::write_volatile(link_trb.add(1), (XHCI_CMD_RING_PHYS >> 32) as u32);
        core::ptr::write_volatile(link_trb.add(2), 0);
        core::ptr::write_volatile(link_trb.add(3), (6 << 10) | (1 << 1) | cycle);
        idx = 0;
        XHCI_CMD_CYCLE ^= 1;
    }

    XHCI_CMD_INDEX = idx;

    let db_off = core::ptr::read_volatile((XHCI_MMIO_BASE + 0x14) as *const u32) & !0x3;
    let db_addr = (XHCI_MMIO_BASE + db_off as usize) as *mut u32;
    core::ptr::write_volatile(db_addr, 0);
}

// xHCI register offsets
const USBSTS_OFFSET: usize = 0x04;
const USBSTS_EINT: u32 = 1 << 3;
const RTSOFF_CAP_OFFSET: usize = 0x18;
const INTERRUPTER0_OFFSET: usize = 0x20;
const IMAN_OFFSET: usize = 0x00;
const ERDP_OFFSET: usize = 0x18;
const IMAN_IP: u32 = 1 << 0;
const IMAN_IE: u32 = 1 << 1;

#[no_mangle]
pub extern "C" fn rust_xhci_handle_irq(_regs: *const core::ffi::c_void) -> u64 {
    unsafe {
        let base = XHCI_MMIO_BASE;
        if base == 0 || XHCI_EVENT_RING_BASE == 0 {
            return _regs as u64;
        }
        let cap_len = XHCI_CAP_LENGTH;

        // 1. Clear USBSTS.EINT
        let usbsts_addr = (base + cap_len + USBSTS_OFFSET) as *mut u32;
        core::ptr::write_volatile(usbsts_addr, USBSTS_EINT);

        // 2. Get runtime/interrupter base
        let rtsoff = core::ptr::read_volatile((base + RTSOFF_CAP_OFFSET) as *const u32) & !0x1F;
        let interrupter_base = base + rtsoff as usize + INTERRUPTER0_OFFSET;

        // 3. Clear IMAN.IP
        let iman_addr = (interrupter_base + IMAN_OFFSET) as *mut u32;
        core::ptr::write_volatile(iman_addr, IMAN_IP | IMAN_IE);

        // 4. Consume all pending Event Ring TRBs
        let ring_base = XHCI_EVENT_RING_BASE as *const u32;
        let ring_size = XHCI_EVENT_RING_SIZE;
        let mut idx = XHCI_ERDP_INDEX;
        let mut cycle = XHCI_EVENT_CYCLE;
        let mut consumed = 0u32;

        loop {
            let trb_base = ring_base.add((idx as usize) * 4);
            let dword3 = core::ptr::read_volatile(trb_base.add(3));
            let trb_cycle = dword3 & 1;

            if trb_cycle != cycle {
                break;
            }

            let trb_type = (dword3 >> 10) & 0x3F;

            // Command Completion Event (Type 33)
            if trb_type == 33 {
                let dword2 = core::ptr::read_volatile(trb_base.add(2));
                let slot_id = (dword3 >> 24) & 0xFF;
                let comp_code = (dword2 >> 24) & 0xFF;

                core::ptr::write_volatile(&mut XHCI_LAST_CMD_CODE, comp_code);
                core::ptr::write_volatile(&mut XHCI_LAST_CMD_SLOT, slot_id);

                print_serial(b"RUST xHCI ISR: Cmd Complete. Code: \0".as_ptr());
                let mut buf = [0u8; 16];
                print_serial(format_num(comp_code as usize, &mut buf).as_ptr());
                print_serial(b" Slot: \0".as_ptr());
                print_serial(format_num(slot_id as usize, &mut buf).as_ptr());
                print_serial(b"\n\0".as_ptr());

                if comp_code == 1 {
                    core::ptr::write_volatile(&mut XHCI_LAST_SLOT_ID, slot_id);
                } else {
                    core::ptr::write_volatile(&mut XHCI_LAST_SLOT_ID, 255);
                    format_cmd_fail_msg(comp_code);
                    set_xhci_debug_msg(XHCI_CMD_MSG.as_ptr());
                }
            }
            // Transfer Event (Type 32)
            else if trb_type == 32 {
                let dword2 = core::ptr::read_volatile(trb_base.add(2));
                let comp_code = (dword2 >> 24) & 0xFF;
                let slot_id = (dword3 >> 24) & 0xFF;
                let ep_id = (dword3 >> 16) & 0x1F;

                print_serial(b"RUST xHCI ISR: Transfer Event. Code: \0".as_ptr());
                let mut buf = [0u8; 16];
                print_serial(format_num(comp_code as usize, &mut buf).as_ptr());
                print_serial(b" Slot: \0".as_ptr());
                print_serial(format_num(slot_id as usize, &mut buf).as_ptr());
                print_serial(b" EP: \0".as_ptr());
                print_serial(format_num(ep_id as usize, &mut buf).as_ptr());
                print_serial(b"\n\0".as_ptr());

                let remainder = dword2 & 0xFFFFFF;

                // Signal transfer completion for control transfers (EP0 has DCI 1)
                if ep_id == 1 {
                    if comp_code == 1 || comp_code == 13 { // Success or Short Packet
                        core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, 1);
                    } else {
                        core::ptr::write_volatile(&raw mut XHCI_TRANSFER_DONE, comp_code);
                    }
                }

                // Check if this is from the keyboard interrupt endpoint
                if slot_id == core::ptr::read_volatile(&raw const XHCI_KB_SLOT) && ep_id == core::ptr::read_volatile(&raw const XHCI_KB_DCI) && core::ptr::read_volatile(&raw const XHCI_KB_HID_BUF) != 0 {
                    if comp_code == 1 || comp_code == 13 {
                        process_hid_keyboard_report();
                    }
                    // Re-queue the next transfer
                    queue_keyboard_transfer();
                }

                // Check if this is from MTP Bulk endpoints
                let mtp_slot = core::ptr::read_volatile(&raw const XHCI_MTP_SLOT);
                if slot_id == mtp_slot && mtp_slot != 0 {
                    if ep_id == core::ptr::read_volatile(&raw const XHCI_MTP_OUT_DCI) {
                        print_serial(b"RUST xHCI: MTP Bulk OUT completed!\n\0".as_ptr());
                        core::ptr::write_volatile(&raw mut XHCI_MTP_OUT_DONE, if comp_code == 1 || comp_code == 13 { 1 } else { comp_code });
                    } else if ep_id == core::ptr::read_volatile(&raw const XHCI_MTP_IN_DCI) {
                        let req_len = core::ptr::read_volatile(&raw const XHCI_MTP_IN_REQ_LEN);
                        let transferred = req_len.saturating_sub(remainder);
                        print_serial(b"RUST xHCI: MTP Bulk IN completed, bytes=\0".as_ptr());
                        print_serial(format_num(transferred as usize, &mut buf).as_ptr());
                        print_serial(b"\n\0".as_ptr());
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_TRANSFERRED, transferred);
                        core::ptr::write_volatile(&raw mut XHCI_MTP_IN_DONE, if comp_code == 1 || comp_code == 13 { 1 } else { comp_code });
                    }
                }

                // Check if this is from Bluetooth endpoints
                let bt_slot = core::ptr::read_volatile(&raw const XHCI_BT_SLOT);
                if slot_id == bt_slot && bt_slot != 0 {
                    let bt_bulk_out_dci = core::ptr::read_volatile(&raw const XHCI_BT_BULK_OUT_DCI);
                    if ep_id == bt_bulk_out_dci && bt_bulk_out_dci != 0 {
                        core::ptr::write_volatile(&raw mut XHCI_BT_BULK_OUT_DONE, if comp_code == 1 || comp_code == 13 { 1 } else { comp_code });
                    } else {
                        let bt_int_in_dci = core::ptr::read_volatile(&raw const XHCI_BT_INT_IN_DCI);
                        if ep_id == bt_int_in_dci && bt_int_in_dci != 0 {
                            if comp_code == 1 || comp_code == 13 {
                                let req_len = core::ptr::read_volatile(&raw const XHCI_BT_INT_REQ_LEN);
                                let transferred = req_len.saturating_sub(remainder);
                                if transferred >= 2 {
                                    xhci_bt_enqueue_event(transferred);
                                }
                            }
                            queue_bt_event_transfer();
                        }
                    }
                }
            }
            // Port Status Change Event (Type 34)
            else if trb_type == 34 {
                print_serial(b"RUST xHCI ISR: Port Status Change Event\n\0".as_ptr());
            }

            consumed += 1;
            idx += 1;
            if idx >= ring_size {
                idx = 0;
                cycle ^= 1;
            }
            if consumed > ring_size {
                break;
            }
        }

        XHCI_ERDP_INDEX = idx;
        XHCI_EVENT_CYCLE = cycle;

        let new_erdp = XHCI_EVENT_RING_PHYS + (idx as u64) * 16;
        let erdp_addr = (interrupter_base + ERDP_OFFSET) as *mut u64;
        core::ptr::write_volatile(erdp_addr, new_erdp | (1 << 3));
    }

    _regs as u64
}

unsafe fn process_hid_keyboard_report() {
    XHCI_KB_REPORTS = XHCI_KB_REPORTS.wrapping_add(1);
    let buf = XHCI_KB_HID_BUF as *const u8;

    // Boot keyboard report format (8 bytes):
    // Byte 0: Modifier keys (bitfield)
    // Byte 1: Reserved
    // Bytes 2-7: Key codes (up to 6 simultaneous keys)

    let modifiers = core::ptr::read_volatile(buf);
    let prev_modifiers = XHCI_KB_PREV_MODS;

    // Process modifier changes
    for bit in 0..8u8 {
        let mask = 1 << bit;
        let ps2_code = MOD_TO_PS2[bit as usize];
        if ps2_code == 0 { continue; }

        if (modifiers & mask) != 0 && (prev_modifiers & mask) == 0 {
            // Modifier pressed
            receive_keyboard_byte(ps2_code);
        } else if (modifiers & mask) == 0 && (prev_modifiers & mask) != 0 {
            // Modifier released
            receive_keyboard_byte(ps2_code | 0x80);
        }
    }
    XHCI_KB_PREV_MODS = modifiers;

    // Detect newly pressed keys
    let mut current_keys = [0u8; 6];
    for k in 0..6 {
        current_keys[k] = core::ptr::read_volatile(buf.add(2 + k));
    }

    // Keys released: were in prev but not in current
    for p in 0..6 {
        let prev_key = XHCI_KB_PREV_KEYS[p];
        if prev_key == 0 { continue; }
        let mut still_pressed = false;
        for c in 0..6 {
            if current_keys[c] == prev_key {
                still_pressed = true;
                break;
            }
        }
        if !still_pressed && (prev_key as usize) < USB_TO_PS2.len() {
            let ps2 = USB_TO_PS2[prev_key as usize];
            if ps2 != 0 {
                receive_keyboard_byte(ps2 | 0x80); // Break code
            }
        }
    }

    // Keys pressed: in current but not in prev
    for c in 0..6 {
        let cur_key = current_keys[c];
        if cur_key == 0 { continue; }
        let mut was_pressed = false;
        for p in 0..6 {
            if XHCI_KB_PREV_KEYS[p] == cur_key {
                was_pressed = true;
                break;
            }
        }
        if !was_pressed && (cur_key as usize) < USB_TO_PS2.len() {
            let ps2 = USB_TO_PS2[cur_key as usize];
            if ps2 != 0 {
                receive_keyboard_byte(ps2); // Make code
            }
        }
    }

    XHCI_KB_PREV_KEYS = current_keys;
}

static mut XHCI_KB_PREV_MODS: u8 = 0;

