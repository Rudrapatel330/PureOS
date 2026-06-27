use core::hint::spin_loop;
use core::sync::atomic::{AtomicBool, Ordering};

// ── Constants ──────────────────────────────────────────────────────────────

const HEAP_START: usize = 0x4000000;
const HEAP_SIZE: usize = 0x1C000000;
const MIN_BLOCK_SIZE: usize = 16;
const BLOCK_MAGIC_USED: u32 = 0xDEADBEEF;
const BLOCK_MAGIC_FREE: u32 = 0xFEEDFACE;
const BLOCK_CANARY: u32 = 0xC0FFEE42;
const NUM_BUCKETS: usize = 8;

#[inline(always)]
unsafe fn fast_memset(s: *mut u8, c: u8, n: usize) {
    core::arch::asm!(
        "rep stosb",
        inout("rdi") s => _,
        inout("rcx") n => _,
        in("al") c,
        options(nostack, preserves_flags)
    );
}

#[inline(always)]
unsafe fn fast_memcpy(dest: *mut u8, src: *const u8, n: usize) {
    core::arch::asm!(
        "rep movsb",
        inout("rdi") dest => _,
        inout("rsi") src => _,
        inout("rcx") n => _,
        options(nostack, preserves_flags)
    );
}

fn write_serial_byte(c: u8) {
    unsafe {
        // Bochs Debug Port
        core::arch::asm!("out dx, al", in("dx") 0xE9u16, in("al") c, options(nomem, nostack, preserves_flags));
        
        // COM1 Output (Wait for transmit empty)
        let mut lsr: u8;
        loop {
            core::arch::asm!("in al, dx", out("al") lsr, in("dx") 0x3F8u16 + 5, options(nomem, nostack, preserves_flags));
            if (lsr & 0x20) != 0 {
                break;
            }
        }
        core::arch::asm!("out dx, al", in("dx") 0x3F8u16, in("al") c, options(nomem, nostack, preserves_flags));
    }
}

fn serial_print(msg: &str) {
    for &byte in msg.as_bytes() {
        write_serial_byte(byte);
    }
}

fn serial_print_hex(val: u64) {
    let mut buf = [0u8; 18];
    buf[0] = b'0';
    buf[1] = b'x';
    for i in 0..16 {
        let digit = ((val >> (60 - i * 4)) & 0xF) as u8;
        buf[i + 2] = if digit < 10 { b'0' + digit } else { b'a' + digit - 10 };
    }
    for &byte in &buf[..18] {
        if byte == 0 { break; }
        write_serial_byte(byte);
    }
}

fn serial_print_dec(n: u32) {
    let mut buf = [0u8; 12];
    let mut i = 10usize;
    let mut v = n;
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
    for &byte in &buf[i..11] {
        write_serial_byte(byte);
    }
}

// ── IRQ-Safe Spinlock (test-and-test-and-set) ─────────────────────────────

pub struct IrqSpinlock {
    locked: AtomicBool,
}

pub struct IrqGuard<'a> {
    lock: &'a IrqSpinlock,
    rflags: u64,
}

impl IrqSpinlock {
    pub const fn new() -> Self {
        Self { locked: AtomicBool::new(false) }
    }

    pub fn lock(&self) -> IrqGuard<'_> {
        let rflags: u64;
        unsafe {
            core::arch::asm!("pushfq; pop {}", out(reg) rflags);
            core::arch::asm!("cli");
        }
        while self.locked.swap(true, Ordering::Acquire) {
            while self.locked.load(Ordering::Relaxed) {
                spin_loop();
            }
        }
        IrqGuard { lock: self, rflags }
    }
}

impl Drop for IrqGuard<'_> {
    fn drop(&mut self) {
        self.lock.locked.store(false, Ordering::Release);
        unsafe {
            core::arch::asm!("push {}; popfq", in(reg) self.rflags);
        }
    }
}

static HEAP_LOCK: IrqSpinlock = IrqSpinlock::new();

// ── Struct Layouts ────────────────────────────────────────────────────────

#[repr(C)]
struct Block {
    magic: u32,
    free: u32,
    next: *mut Block,
    prev: *mut Block,
    size: usize,
    reserved: [u64; 2],
}

#[repr(C)]
struct FreeNode {
    next: *mut Block,
    prev: *mut Block,
}

// ── Globals ────────────────────────────────────────────────────────────────

static mut BUCKETS: [*mut Block; NUM_BUCKETS] = [core::ptr::null_mut(); NUM_BUCKETS];
static mut PHYSICAL_HEAD: *mut Block = core::ptr::null_mut();
static mut HEAP_USED_BYTES: u32 = 0;

// ── Helpers ────────────────────────────────────────────────────────────────

fn get_bucket(size: usize) -> usize {
    match size {
        0..=32 => 0,
        33..=64 => 1,
        65..=128 => 2,
        129..=256 => 3,
        257..=512 => 4,
        513..=1024 => 5,
        1025..=2048 => 6,
        _ => 7,
    }
}

fn is_heap_ptr(ptr: *const u8) -> bool {
    let p = ptr as usize;
    p >= HEAP_START && p < HEAP_START + HEAP_SIZE
}

unsafe fn block_valid(b: *mut Block) -> bool {
    if (*b).magic != BLOCK_MAGIC_USED && (*b).magic != BLOCK_MAGIC_FREE {
        serial_print("HEAP: Invalid magic!\n");
        return false;
    }
    if (*b).free == 0 {
        let canary_ptr = (b as *mut u8)
            .add(core::mem::size_of::<Block>())
            .add((*b).size)
            .sub(core::mem::size_of::<u32>()) as *mut u32;
        if *canary_ptr != BLOCK_CANARY {
            serial_print("HEAP: Canary corruption detected at block ");
            serial_print_hex(b as u64);
            serial_print("\n");
            return false;
        }
    }
    true
}

unsafe fn add_to_free_list(b: *mut Block) {
    let bucket = get_bucket((*b).size);
    let node = (b as *mut u8).add(core::mem::size_of::<Block>()) as *mut FreeNode;

    (*node).next = BUCKETS[bucket];
    (*node).prev = core::ptr::null_mut();

    if !BUCKETS[bucket].is_null() {
        if !is_heap_ptr(BUCKETS[bucket] as *mut u8) {
            serial_print("HEAP: add_to_free_list clobbered by non-heap bucket head!\n");
            BUCKETS[bucket] = core::ptr::null_mut();
            return;
        }
        let next_node = (BUCKETS[bucket] as *mut u8).add(core::mem::size_of::<Block>()) as *mut FreeNode;
        (*next_node).prev = b;
    }
    BUCKETS[bucket] = b;
}

unsafe fn remove_from_free_list(b: *mut Block) {
    let bucket = get_bucket((*b).size);
    let node = (b as *mut u8).add(core::mem::size_of::<Block>()) as *mut FreeNode;

    if !(*node).prev.is_null() {
        if !is_heap_ptr((*node).prev as *mut u8) {
            serial_print("HEAP: remove_from_free_list: prev is NOT a heap pointer! Ignoring.\n");
            return;
        }
        let prev_node = ((*node).prev as *mut u8).add(core::mem::size_of::<Block>()) as *mut FreeNode;
        (*prev_node).next = (*node).next;
    } else {
        BUCKETS[bucket] = (*node).next;
    }

    if !(*node).next.is_null() {
        if !is_heap_ptr((*node).next as *mut u8) {
            serial_print("HEAP: remove_from_free_list: next is NOT a heap pointer! Ignoring.\n");
            return;
        }
        let next_node = ((*node).next as *mut u8).add(core::mem::size_of::<Block>()) as *mut FreeNode;
        (*next_node).prev = (*node).prev;
    }
}

// ── Public API ─────────────────────────────────────────────────────────────

#[no_mangle]
pub extern "C" fn heap_init() {
    unsafe {
        let heap_start_aligned = (HEAP_START + 15) & !15;
        let block = heap_start_aligned as *mut Block;

        (*block).magic = BLOCK_MAGIC_FREE;
        (*block).free = 1;
        (*block).next = core::ptr::null_mut();
        (*block).prev = core::ptr::null_mut();
        (*block).size = HEAP_SIZE - (heap_start_aligned - HEAP_START) - core::mem::size_of::<Block>();

        PHYSICAL_HEAD = block;
        add_to_free_list(block);

        serial_print("HEAP INITIALIZED (448MB O(1))\n");
    }
}

#[inline(always)]
fn kmalloc_internal(size: usize) -> *mut u8 {
    if size == 0 {
        return core::ptr::null_mut();
    }

    let _guard = HEAP_LOCK.lock();

    let actual_size = core::cmp::max(MIN_BLOCK_SIZE, (size + 4 + 15) & !15);
    let bucket = get_bucket(actual_size);

    unsafe {
        for i in bucket..NUM_BUCKETS {
            let mut current = BUCKETS[i];
            while !current.is_null() {
                let b = current;
                
                if !is_heap_ptr(b as *mut u8) {
                    serial_print("HEAP: kmalloc found bad block!\n");
                    return core::ptr::null_mut();
                }

                if (*b).free == 1 && (*b).size >= actual_size {
                    remove_from_free_list(b);

                    if (*b).size > actual_size + core::mem::size_of::<Block>() + MIN_BLOCK_SIZE {
                        let new_b = (b as *mut u8).add(core::mem::size_of::<Block>() + actual_size) as *mut Block;
                        (*new_b).magic = BLOCK_MAGIC_FREE;
                        (*new_b).free = 1;
                        (*new_b).size = (*b).size - actual_size - core::mem::size_of::<Block>();
                        (*new_b).prev = b;
                        (*new_b).next = (*b).next;

                        if !(*b).next.is_null() {
                            (*(*b).next).prev = new_b;
                        }

                        (*b).next = new_b;
                        (*b).size = actual_size;

                        add_to_free_list(new_b);
                    }

                    (*b).free = 0;
                    (*b).magic = BLOCK_MAGIC_USED;
                    
                    let canary_ptr = (b as *mut u8).add(core::mem::size_of::<Block>() + (*b).size - core::mem::size_of::<u32>()) as *mut u32;
                    *canary_ptr = BLOCK_CANARY;

                    HEAP_USED_BYTES += (*b).size as u32 + core::mem::size_of::<Block>() as u32;

                    let ptr = (b as *mut u8).add(core::mem::size_of::<Block>());
                    return ptr;
                }

                let node = (b as *mut u8).add(core::mem::size_of::<Block>()) as *mut FreeNode;
                current = (*node).next;
            }
        }
    }

    serial_print("HEAP: OOM!\n");
    core::ptr::null_mut()
}

#[no_mangle]
pub extern "C" fn kmalloc(size: usize) -> *mut u8 {
    kmalloc_internal(size)
}

#[inline(always)]
fn kfree_internal(ptr: *mut u8) {
    if ptr.is_null() {
        return;
    }

    unsafe {
        if !is_heap_ptr(ptr) {
            serial_print("HEAP: kfree bad ptr\n");
            return;
        }

        let b = ptr.sub(core::mem::size_of::<Block>()) as *mut Block;
        if (*b).magic != BLOCK_MAGIC_USED && (*b).magic != BLOCK_MAGIC_FREE {
            serial_print("HEAP: kfree invalid magic\n");
            return;
        }

        if (*b).free == 1 {
            serial_print("HEAP: double free\n");
            return;
        }

        let canary_ptr = (b as *mut u8).add(core::mem::size_of::<Block>() + (*b).size - core::mem::size_of::<u32>()) as *mut u32;
        if *canary_ptr != BLOCK_CANARY {
            serial_print("HEAP: kfree canary bad\n");
            return;
        }

        let _guard = HEAP_LOCK.lock();

        HEAP_USED_BYTES -= (*b).size as u32 + core::mem::size_of::<Block>() as u32;
        (*b).magic = BLOCK_MAGIC_FREE;
        (*b).free = 1;

        if (*b).size > core::mem::size_of::<FreeNode>() {
            fast_memset(ptr, 0xDE, (*b).size);
        }

        let mut current = b;

        if !(*current).next.is_null() && (*(*current).next).free == 1 {
            let next_b = (*current).next;
            remove_from_free_list(next_b);
            (*current).size += core::mem::size_of::<Block>() + (*next_b).size;
            (*current).next = (*next_b).next;
            if !(*current).next.is_null() {
                (*(*current).next).prev = current;
            }
        }

        if !(*current).prev.is_null() && (*(*current).prev).free == 1 {
            let prev_b = (*current).prev;
            remove_from_free_list(prev_b);
            (*prev_b).size += core::mem::size_of::<Block>() + (*current).size;
            (*prev_b).next = (*current).next;
            if !(*prev_b).next.is_null() {
                (*(*prev_b).next).prev = prev_b;
            }
            current = prev_b;
        }

        add_to_free_list(current);
    }
}

#[no_mangle]
pub extern "C" fn kfree(ptr: *mut u8) {
    kfree_internal(ptr);
}

#[no_mangle]
pub extern "C" fn kmalloc_ap(size: usize, phys: *mut u32) -> *mut u8 {
    let ptr = kmalloc_internal(size + 4096);
    if ptr.is_null() {
        return core::ptr::null_mut();
    }
    let addr = ptr as u32;
    let aligned = (addr + 0xFFF) & !0xFFF;
    if !phys.is_null() {
        unsafe { *phys = aligned; }
    }
    aligned as *mut u8
}

#[no_mangle]
pub extern "C" fn krealloc(ptr: *mut u8, size: usize) -> *mut u8 {
    if ptr.is_null() {
        return kmalloc_internal(size);
    }

    if size == 0 {
        kfree_internal(ptr);
        return core::ptr::null_mut();
    }

    let old_size: usize;
    {
        let _guard = HEAP_LOCK.lock();
        unsafe {
            let block = (ptr as *mut u8).sub(core::mem::size_of::<Block>()) as *mut Block;
            if !block_valid(block) {
                serial_print("HEAP: krealloc on corrupted block!\n");
                return core::ptr::null_mut();
            }
            old_size = (*block).size;
            let new_total = (size + core::mem::size_of::<u32>() + MIN_BLOCK_SIZE - 1) & !(MIN_BLOCK_SIZE - 1);
            if new_total <= old_size {
                return ptr;
            }
        }
    }

    let new_ptr = kmalloc_internal(size);
    if new_ptr.is_null() {
        return core::ptr::null_mut();
    }

    let min_size = core::cmp::min(old_size, size);
    unsafe {
        fast_memcpy(new_ptr, ptr, min_size);
    }

    kfree_internal(ptr);
    new_ptr
}

// ── C Wrappers ─────────────────────────────────────────────────────────────

#[no_mangle]
pub extern "C" fn malloc(size: usize) -> *mut u8 {
    kmalloc_internal(size)
}

#[no_mangle]
pub extern "C" fn realloc(ptr: *mut u8, new_size: usize) -> *mut u8 {
    krealloc(ptr, new_size)
}

#[no_mangle]
pub extern "C" fn calloc(nmemb: usize, size: usize) -> *mut u8 {
    let total = nmemb * size;
    let ptr = kmalloc_internal(total);
    if !ptr.is_null() {
        unsafe {
            fast_memset(ptr, 0, total);
        }
    }
    ptr
}

#[no_mangle]
pub extern "C" fn free(ptr: *mut u8) {
    kfree_internal(ptr);
}

// ── Debug / Stats ─────────────────────────────────────────────────────────

#[no_mangle]
pub extern "C" fn heap_stats() {
    let _guard = HEAP_LOCK.lock();
    unsafe {
        let mut current = PHYSICAL_HEAD;
        let mut total_blocks: u32 = 0;
        let mut _free_blocks: u32 = 0;
        let mut used_bytes: u32 = 0;
        let mut free_bytes: u32 = 0;
        let mut corrupted: u32 = 0;

        while !current.is_null() {
            if !block_valid(current) {
                corrupted = 1;
                break;
            }
            total_blocks += 1;
            if (*current).free != 0 {
                _free_blocks += 1;
                free_bytes += (*current).size as u32;
            } else {
                used_bytes += (*current).size as u32;
            }
            current = (*current).next;
        }

        serial_print("HEAP: blocks=");
        serial_print_dec(total_blocks);
        serial_print(" used=");
        serial_print_dec(used_bytes / 1024);
        serial_print("KB free=");
        serial_print_dec(free_bytes / 1024);
        serial_print("KB");
        if corrupted != 0 {
            serial_print(" CORRUPTED!");
        }
        serial_print("\n");
    }
}

#[no_mangle]
pub extern "C" fn heap_audit() {
    let _guard = HEAP_LOCK.lock();
    unsafe {
        let mut current = PHYSICAL_HEAD;
        while !current.is_null() {
            if !block_valid(current) {
                serial_print("HEAP_AUDIT: CORRUPTION DETECTED - HALTING\n");
                loop { core::arch::asm!("hlt"); }
            }
            current = (*current).next;
        }
    }
}

#[no_mangle]
pub extern "C" fn heap_get_used_bytes() -> u32 {
    unsafe { HEAP_USED_BYTES }
}

#[no_mangle]
pub extern "C" fn heap_get_total_bytes() -> u32 {
    HEAP_SIZE as u32
}
