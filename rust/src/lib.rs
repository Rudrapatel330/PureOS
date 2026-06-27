#![no_std]
#![no_main]

use core::alloc::{GlobalAlloc, Layout};
use core::panic::PanicInfo;

pub mod heap;
pub mod usb_ffi;
pub mod ehci;

struct KernelAlloc;

unsafe impl GlobalAlloc for KernelAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        if layout.align() > 16 {
            panic!("Unsupported alignment > 16");
        }
        crate::heap::kmalloc(layout.size())
    }
    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        crate::heap::kfree(ptr)
    }
}

#[global_allocator]
static ALLOCATOR: KernelAlloc = KernelAlloc;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    unsafe {
        extern "C" {
            fn print_serial(s: *const u8);
        }
        print_serial(b"RUST PANIC!\n\0".as_ptr());
    }
    loop {
        unsafe { core::arch::asm!("hlt"); }
    }
}

#[no_mangle]
pub extern "C" fn rust_test() -> u32 {
    42
}
