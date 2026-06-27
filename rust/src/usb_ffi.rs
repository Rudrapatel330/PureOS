use core::ffi::c_void;

#[repr(C, packed)]
pub struct UsbSetupPacket {
    pub request_type: u8,
    pub request: u8,
    pub value: u16,
    pub index: u16,
    pub length: u16,
}

#[repr(C, packed)]
pub struct UsbDeviceDesc {
    pub length: u8,
    pub desc_type: u8,
    pub usb_version: u16,
    pub device_class: u8,
    pub device_subclass: u8,
    pub device_protocol: u8,
    pub max_packet_size: u8,
    pub vendor_id: u16,
    pub product_id: u16,
    pub device_version: u16,
    pub manufacturer_str_idx: u8,
    pub product_str_idx: u8,
    pub serial_str_idx: u8,
    pub num_configurations: u8,
}

#[repr(C)]
pub struct UsbDevice {
    pub address: u8,
    pub port: u8,
    pub speed: u32,
    pub hcd: *mut UsbHcd,
    pub descriptor: UsbDeviceDesc,
    pub config: *mut c_void,
    pub driver_data: *mut c_void,
}

#[repr(C)]
pub struct UsbHcd {
    pub name: *const u8,
    pub data: *mut c_void,
    pub control_transfer:
        Option<unsafe extern "C" fn(dev: *mut UsbDevice, setup: *mut UsbSetupPacket, buffer: *mut c_void, length: u16) -> i32>,
    pub start_interrupt_in:
        Option<
            unsafe extern "C" fn(
                dev: *mut UsbDevice,
                endpoint: u8,
                buffer: *mut c_void,
                length: u16,
                callback: Option<unsafe extern "C" fn(*mut u8, i32)>,
            ) -> i32,
        >,
    pub start_isochronous_in:
        Option<
            unsafe extern "C" fn(
                dev: *mut UsbDevice,
                endpoint: u8,
                buffer: *mut c_void,
                length: u16,
                callback: Option<unsafe extern "C" fn(*mut u8, i32)>,
            ) -> i32,
        >,
    pub poll: Option<unsafe extern "C" fn(hcd: *mut UsbHcd)>,
    pub reset_port: Option<unsafe extern "C" fn(hcd: *mut UsbHcd, port: u8)>,
}

extern "C" {
    pub fn usb_register_hcd(hcd: *mut UsbHcd);
    pub fn usb_new_device(hcd: *mut UsbHcd, port: u8, speed: u32);
    pub fn kmalloc_ap(size: usize, phys: *mut u32) -> *mut c_void;
    pub fn print_serial(s: *const u8);
    pub fn pci_config_read_dword(bus: u8, slot: u8, func: u8, offset: u8) -> u32;
    pub fn pci_config_read_byte(bus: u8, slot: u8, func: u8, offset: u8) -> u8;
    pub fn pci_config_write_word(bus: u8, slot: u8, func: u8, offset: u8, data: u16);
    pub fn pci_config_write_dword(bus: u8, slot: u8, func: u8, offset: u8, data: u32);
}
