// Linux x86_64 syscall dispatcher.
// Translates Linux syscall numbers, argument conventions, and data structures
// to PureOS kernel operations via C FFI.

#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

use core::ffi::c_void;
use core::ptr;

// ── Register state as pushed by the C interrupt / fast-syscall path ──────
#[repr(C, packed)]
pub struct Registers {
    pub r15: u64, pub r14: u64, pub r13: u64, pub r12: u64,
    pub r11: u64, pub r10: u64, pub r9:  u64, pub r8:  u64,
    pub rdi: u64, pub rsi: u64, pub rbp: u64, pub rbx: u64,
    pub rdx: u64, pub rcx: u64, pub rax: u64,
    pub int_no:   u64, pub err_code: u64,
    pub rip: u64, pub cs: u64, pub rflags: u64, pub rsp: u64, pub ss: u64,
}

// ── Linux x86_64 syscall numbers ──────────────────────────────────────────
const SYS_READ:             u64 = 0;
const SYS_WRITE:            u64 = 1;
const SYS_OPEN:             u64 = 2;
const SYS_CLOSE:            u64 = 3;
const SYS_STAT:             u64 = 4;
const SYS_FSTAT:            u64 = 5;
const SYS_LSTAT:            u64 = 6;
const SYS_POLL:             u64 = 7;
const SYS_LSEEK:            u64 = 8;
const SYS_MMAP:             u64 = 9;
const SYS_MPROTECT:         u64 = 10;
const SYS_MUNMAP:           u64 = 11;
const SYS_BRK:              u64 = 12;
const SYS_RT_SIGACTION:     u64 = 13;
const SYS_RT_SIGPROCMASK:   u64 = 14;
const SYS_RT_SIGRETURN:     u64 = 15;
const SYS_IOCTL:            u64 = 16;
const SYS_PREAD64:          u64 = 17;
const SYS_PWRITE64:         u64 = 18;
const SYS_READV:            u64 = 19;
const SYS_WRITEV:           u64 = 20;
const SYS_ACCESS:           u64 = 21;
const SYS_PIPE:             u64 = 22;
const SYS_SELECT:           u64 = 23;
const SYS_SCHED_YIELD:      u64 = 24;
const SYS_MREMAP:           u64 = 25;
const SYS_DUP:              u64 = 32;
const SYS_DUP2:             u64 = 33;
const SYS_NANOSLEEP:        u64 = 35;
const SYS_GETPID:           u64 = 39;
const SYS_SENDTO:           u64 = 44;
const SYS_RECVFROM:         u64 = 45;
const SYS_SENDMSG:          u64 = 46;
const SYS_RECVMSG:          u64 = 47;
const SYS_SHUTDOWN:         u64 = 48;
const SYS_SOCKETPAIR:       u64 = 53;
const SYS_SETSOCKOPT:       u64 = 54;
const SYS_GETSOCKOPT:       u64 = 55;
const SYS_CLONE:            u64 = 56;
const SYS_FORK:             u64 = 57;
const SYS_EXECVE:           u64 = 59;
const SYS_EXIT:             u64 = 60;
const SYS_WAIT4:            u64 = 61;
const SYS_KILL:             u64 = 62;
const SYS_UNAME:            u64 = 63;
const SYS_SEMGET:           u64 = 64;
const SYS_SEMCTL:           u64 = 66;
const SYS_GETCWD:           u64 = 79;
const SYS_CHDIR:            u64 = 80;
const SYS_MKDIR:            u64 = 83;
const SYS_RMDIR:            u64 = 84;
const SYS_LINK:             u64 = 86;
const SYS_UNLINK:           u64 = 87;
const SYS_SYMLINK:          u64 = 88;
const SYS_READLINK:         u64 = 89;
const SYS_CHMOD:            u64 = 90;
const SYS_FCHMOD:           u64 = 91;
const SYS_UMASK:            u64 = 93;
const SYS_GETTID:           u64 = 186;
const SYS_GETDENTS64:       u64 = 217;
const SYS_SET_TID_ADDRESS:  u64 = 218;
const SYS_CLOCK_GETTIME:    u64 = 228;
const SYS_EXIT_GROUP:       u64 = 231;
const SYS_TKILL:            u64 = 200;
const SYS_GETUID:           u64 = 102;
const SYS_GETGID:           u64 = 104;
const SYS_GETEUID:          u64 = 107;
const SYS_GETEGID:          u64 = 108;
const SYS_FACCESSAT:        u64 = 269;
const SYS_FUTEX:            u64 = 202;
const SYS_OPENAT:           u64 = 257;
const SYS_NEWFSTATAT:       u64 = 262;
const SYS_SET_ROBUST_LIST:  u64 = 300;
const SYS_GETRANDOM:        u64 = 318;
const SYS_RSEQ:             u64 = 334;
const SYS_FCNTL:            u64 = 72;
const SYS_GETSOCKNAME:      u64 = 51;
const SYS_GETPEERNAME:      u64 = 52;
const SYS_EVENTFD:          u64 = 284;
const SYS_EVENTFD2:         u64 = 290;
const SYS_PIPE2:            u64 = 293;

// ── Linux errno values ────────────────────────────────────────────────────
const EPERM:   i64 = 1;
const ENOENT:  i64 = 2;
const EIO:     i64 = 5;
const EBADF:   i64 = 9;
const ENOMEM:  i64 = 12;
const EACCES:  i64 = 13;
const EFAULT:  i64 = 14;
const EBUSY:   i64 = 16;
const EINVAL:  i64 = 22;
const ENFILE:  i64 = 23;
const ENODEV:  i64 = 19;
const ENOSYS:  i64 = 38;
const ENOTTY:  i64 = 25;
const ESPIPE:  i64 = 29;
const ENOEXEC: i64 = 8;
const ECHILD:  i64 = 10;
const ENOTDIR: i64 = 20;

// ── Linux mmap constants ──────────────────────────────────────────────────
const PROT_READ:     i32 = 0x1;
const PROT_WRITE:    i32 = 0x2;
const PROT_EXEC:     i32 = 0x4;
const MAP_SHARED:    i32 = 0x01;
const MAP_PRIVATE:   i32 = 0x02;
const MAP_ANONYMOUS: i32 = 0x20;
const MAP_FIXED:     i32 = 0x10;
const MAP_FAILED:    u64 = 0xFFFFFFFFFFFFFFFFu64;

// ── Linux fcntl.h open flags ──────────────────────────────────────────────
const O_RDONLY:  i32 = 0;
const O_WRONLY:  i32 = 1;
const O_RDWR:    i32 = 2;
const O_CREAT:   i32 = 0x40;
const O_TRUNC:   i32 = 0x200;
const O_APPEND:  i32 = 0x400;
const O_CLOEXEC: i32 = 0x80000;

// ── Linux struct stat (x86_64 syscall ABI) ────────────────────────────────
#[repr(C)]
struct LinuxStat {
    st_dev:     u64,
    st_ino:     u64,
    st_nlink:   u64,
    st_mode:    u32,
    st_uid:     u32,
    st_gid:     u32,
    __pad0:     u32,
    st_rdev:    u64,
    st_size:    i64,
    st_blksize: i64,
    st_blocks:  i64,
    st_atime:   u64,
    st_atime_nsec: u64,
    st_mtime:   u64,
    st_mtime_nsec: u64,
    st_ctime:   u64,
    st_ctime_nsec: u64,
    __unused:   [i64; 3],
}

// ── Linux struct utsname ──────────────────────────────────────────────────
#[repr(C)]
struct LinuxUtsname {
    sysname:    [u8; 65],
    nodename:   [u8; 65],
    release:    [u8; 65],
    version:    [u8; 65],
    machine:    [u8; 65],
    domainname: [u8; 65],
}

// ── Linux struct timespec ─────────────────────────────────────────────────
#[repr(C)]
struct Timespec {
    tv_sec:  i64,
    tv_nsec: i64,
}

// ── Linux struct iovec ────────────────────────────────────────────────────
#[repr(C)]
struct Iovec {
    iov_base: *mut u8,
    iov_len:  usize,
}

// ── Linux struct msghdr ───────────────────────────────────────────────────
#[repr(C)]
struct MsgHdr {
    msg_name:       *mut u8,
    msg_namelen:    u32,
    _pad1:          u32,
    msg_iov:        *mut Iovec,
    msg_iovlen:     usize,
    msg_control:    *mut u8,
    msg_controllen: usize,
    msg_flags:      i32,
    _pad2:          u32,
}

// ── Linux struct linux_dirent64 ───────────────────────────────────────────
#[repr(C)]
struct LinuxDirent64 {
    d_ino:     u64,
    d_off:     i64,
    d_reclen:  u16,
    d_type:    u8,
    d_name:    [u8; 1], // variable-length; accessed via pointer math
}

#[repr(C)]
struct VfsDentry {
    name: [u8; 128],
    inode: *mut c_void,
    parent: *mut c_void,
    next: *mut c_void,
    child: *mut c_void,
    refcount: u32,
    flags: u32,
    mount_root: *mut c_void,
}

// ── rtc_time_t (mirrors C struct in drivers/rtc.h) ────────────────────────
#[repr(C)]
struct RtcTime {
    second:     u8,
    minute:     u8,
    hour:       u8,
    day:        u8,
    month:      u8,
    year:       u8,
}

// ── PureOS vfs_stat_t (mirrors C struct in fs/vfs.h) ──────────────────────
#[repr(C)]
struct PureOSStat {
    st_ino:     u32,
    st_mode:    u32,
    st_nlink:   u32,
    st_uid:     u32,
    st_gid:     u32,
    st_size:    u32,
    st_blksize: u32,
    st_blocks:  u32,
    st_atime:   u32,
    st_mtime:   u32,
    st_ctime:   u32,
    st_dev:     u32,
    size:       u32,
    flags:      u32,
}

// ── C function declarations (provided by the PureOS kernel) ──────────────
extern "C" {
    fn vfs_open(path: *const u8, flags: i32) -> i32;
    fn vfs_close(fd: i32);
    fn vfs_read(fd: i32, buf: *mut u8, count: u32) -> i32;
    fn vfs_write(fd: i32, buf: *const u8, count: u32) -> i32;
    fn vfs_lseek(fd: i32, offset: u64, whence: i32) -> u64;
    fn vfs_fstat(fd: i32, buf: *mut u8) -> i32;
    fn vfs_stat(path: *const u8, buf: *mut u8) -> i32;
    fn vfs_dup2(oldfd: i32, newfd: i32) -> i32;
    fn vfs_mkdir(path: *const u8) -> i32;
    fn vfs_unlink(path: *const u8) -> i32;
    fn vfs_chmod(path: *const u8, mode: u32) -> i32;
    fn vfs_readlink(path: *const u8, buf: *mut u8, bufsiz: u32) -> i32;
    fn vfs_symlink(target: *const u8, linkpath: *const u8) -> i32;
    fn vfs_rename(oldpath: *const u8, newpath: *const u8) -> i32;
    fn vfs_readdir(fd: i32, index: u32) -> *const VfsDentry;
    fn kmalloc_ap(size: usize, phys: *mut u32) -> *mut u8;
    fn paging_map_user_page(pml4: *mut u8, vaddr: u64, paddr: u64, flags: i32);
    fn get_current_task() -> *mut u8;
    fn pipe(fds: *mut i32) -> i32;
    fn is_user_range(addr: *const u8, len: usize) -> i32;
    fn rtc_read(tm: *mut u8);
    fn keyboard_getc() -> i32;
    fn print_serial(s: *const u8);

    // Linux-compat helpers in syscall.c
    fn linux_helper_get_pid() -> u64;
    fn linux_helper_get_uid() -> u64;
    fn linux_helper_get_gid() -> u64;
    fn linux_helper_get_brk() -> u64;
    fn linux_helper_set_brk(brk: u64);
    fn linux_helper_exit();
    fn linux_helper_get_pagedir() -> *mut u8;
    fn linux_helper_set_fs_base(base: u64);
    fn linux_helper_get_cwd(buf: *mut u8, max_len: i32);
    fn linux_helper_set_cwd(buf: *const u8);
    fn linux_helper_get_mmap() -> u64;
    fn linux_helper_set_mmap(mmap: u64);
    fn linux_helper_frame_alloc() -> u32;
    fn linux_helper_frame_free(phys: u32);
    fn linux_helper_mprotect(addr: u64, len: u64, prot: i32) -> i32;
    fn linux_helper_munmap(addr: u64, len: u64) -> i32;
    fn linux_helper_eventfd2(initval: u32, flags: i32) -> i32;
    fn linux_helper_is_eventfd(fd: i32) -> i32;
    fn linux_helper_eventfd_poll(fd: i32, events: i32) -> i32;
    fn linux_helper_pipe2(fds: *mut i32, flags: i32) -> i32;
    fn linux_helper_clone_thread(regs: *const c_void, flags: u64, child_stack: u64, parent_tid: u64, child_tid: u64, child_tls: u64) -> i64;
    fn linux_helper_get_tid() -> u64;
    fn linux_helper_get_fs_base() -> u64;
    fn linux_helper_set_clear_tid(addr: u64);
    fn linux_helper_get_errno() -> i32;
    fn linux_helper_yield();
    fn shell_print_output(text: *const u8);
    fn get_timer_ms_hires() -> u64;

    // LwIP socket functions
    fn lwip_socket(domain: i32, type_: i32, protocol: i32) -> i32;
    fn lwip_bind(s: i32, name: *const c_void, namelen: u32) -> i32;
    fn lwip_connect(s: i32, name: *const c_void, namelen: u32) -> i32;
    fn lwip_sendto(s: i32, data: *const c_void, size: usize, flags: i32, to: *const c_void, tolen: u32) -> i32;
    fn lwip_recvfrom(s: i32, mem: *mut c_void, len: usize, flags: i32, from: *mut c_void, fromlen: *mut u32) -> i32;
    fn lwip_getsockopt(s: i32, level: i32, optname: i32, optval: *mut c_void, optlen: *mut u32) -> i32;
    fn lwip_setsockopt(s: i32, level: i32, optname: i32, optval: *const c_void, optlen: u32) -> i32;
    fn lwip_getsockname(s: i32, name: *mut c_void, namelen: *mut u32) -> i32;
    fn lwip_getpeername(s: i32, name: *mut c_void, namelen: *mut u32) -> i32;
    fn lwip_read(s: i32, mem: *mut c_void, len: usize) -> i32;
    fn lwip_write(s: i32, data: *const c_void, size: usize) -> i32;
    fn lwip_close(s: i32) -> i32;
    fn lwip_shutdown(s: i32, how: i32) -> i32;
    fn lwip_fcntl(s: i32, cmd: i32, val: i32) -> i32;
    fn lwip_select(maxfdp1: i32, readset: *mut c_void, writeset: *mut c_void, exceptset: *mut c_void, timeout: *mut c_void) -> i32;
    fn lwip_poll(fds: *mut c_void, nfds: u32, timeout: i32) -> i32;
}

// ── Helper: write a string into a fixed-length byte array ─────────────────
fn str_to_fixed(dest: &mut [u8], src: &[u8]) {
    let copy_len = core::cmp::min(dest.len() - 1, src.len());
    dest[..copy_len].copy_from_slice(&src[..copy_len]);
    dest[copy_len] = 0;
}

// ── Helper: translate PureOS stat to Linux stat ──────────────────────────
fn translate_stat(src: &PureOSStat, dst: &mut LinuxStat) {
    dst.st_dev     = src.st_dev as u64;
    dst.st_ino     = src.st_ino as u64;
    dst.st_nlink   = src.st_nlink as u64;
    dst.st_mode    = src.st_mode;
    dst.st_uid     = src.st_uid;
    dst.st_gid     = src.st_gid;
    dst.st_rdev    = 0;
    dst.st_size    = src.st_size as i64;
    dst.st_blksize = src.st_blksize as i64;
    dst.st_blocks  = src.st_blocks as i64;
    dst.st_atime   = src.st_atime as u64;
    dst.st_mtime   = src.st_mtime as u64;
    dst.st_ctime   = src.st_ctime as u64;
}

// mmap and brk state live in the C task_t accessed via linux_helper_* functions

// ── Pseudo-FD numbers for emulated device files ──────────────────────────
const FD_TTY:         u64 = 1001;
const FD_NULL:        u64 = 1002;
const FD_URANDOM:     u64 = 1003;
const FD_ZERO:        u64 = 1004;
const FD_RESOLV_CONF: u64 = 1005;
const FD_HOSTS:       u64 = 1006;
const FD_CA_CERTS:    u64 = 1007;
const FD_OPENSSL_CNF: u64 = 1008;

static mut PSEUDO_OFF_RESOLV: usize = 0;
static mut PSEUDO_OFF_HOSTS: usize = 0;
static mut PSEUDO_OFF_CA_CERTS: usize = 0;
static mut PSEUDO_OFF_OPENSSL_CNF: usize = 0;

static RESOLV_CONF_DATA: &[u8] = b"nameserver 8.8.8.8\nnameserver 10.0.2.3\n";
static HOSTS_DATA: &[u8] = b"127.0.0.1 localhost\n";
static CA_CERTS_DATA: &[u8] = include_bytes!("../ca-certificates.crt");
static OPENSSL_CNF_DATA: &[u8] = b"openssl_conf = openssl_init\n\n[openssl_init]\nssl_conf = ssl_sect\n\n[ssl_sect]\nsystem_default = system_default_sect\n\n[system_default_sect]\nGroups = X25519:P-256:P-384\n";

static mut RNG_STATE: u64 = 0x853c49e6748fea9b;

#[inline]
unsafe fn get_entropy_u64() -> u64 {
    let eax: u32;
    let edx: u32;
    core::arch::asm!("rdtsc", out("eax") eax, out("edx") edx, options(nomem, nostack));
    let tsc = ((edx as u64) << 32) | (eax as u64);
    
    // Mix with global state using SplitMix64
    RNG_STATE = RNG_STATE.wrapping_add(tsc).wrapping_add(0x9e3779b97f4a7c15);
    let mut z = RNG_STATE;
    z = (z ^ (z >> 30)).wrapping_mul(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94d049bb133111eb);
    z ^ (z >> 31)
}

unsafe fn fill_random(buf: *mut u8, len: usize) {
    let mut i = 0;
    while i + 8 <= len {
        let r = get_entropy_u64();
        ptr::copy_nonoverlapping(&r as *const _ as *const u8, buf.add(i), 8);
        i += 8;
    }
    if i < len {
        let r = get_entropy_u64();
        ptr::copy_nonoverlapping(&r as *const _ as *const u8, buf.add(i), len - i);
    }
}

fn is_pseudo_fd(fd: u64) -> bool {
    fd >= FD_TTY && fd <= FD_OPENSSL_CNF
}

/// Check whether `path` (a C string in user space) names a known
/// pseudo-device and return the corresponding pseudo-FD.
unsafe fn path_is_pseudo(path: *const u8) -> Option<u64> {
    const ENTRIES: &[(&[u8], u64)] = &[
        (b"/dev/tty\0",                           FD_TTY),
        (b"/dev/null\0",                          FD_NULL),
        (b"/dev/urandom\0",                       FD_URANDOM),
        (b"/dev/random\0",                        FD_URANDOM),
        (b"/dev/zero\0",                          FD_ZERO),
        (b"/etc/resolv.conf\0",                   FD_RESOLV_CONF),
        (b"/etc/hosts\0",                         FD_HOSTS),
        (b"/etc/ssl/certs/ca-certificates.crt\0", FD_CA_CERTS),
        (b"/etc/ssl/cert.pem\0",                  FD_CA_CERTS),
        (b"/etc/pki/tls/certs/ca-bundle.crt\0",   FD_CA_CERTS),
        (b"/etc/ssl/openssl.cnf\0",               FD_OPENSSL_CNF),
        (b"/etc/pki/tls/openssl.cnf\0",           FD_OPENSSL_CNF),
    ];
    for &(expected, pseudo_fd) in ENTRIES {
        let mut ok = true;
        for (j, &c) in expected.iter().enumerate() {
            if *path.add(j) != c { ok = false; break; }
        }
        if ok {
            if pseudo_fd == FD_RESOLV_CONF {
                PSEUDO_OFF_RESOLV = 0;
            } else if pseudo_fd == FD_HOSTS {
                PSEUDO_OFF_HOSTS = 0;
            } else if pseudo_fd == FD_CA_CERTS {
                PSEUDO_OFF_CA_CERTS = 0;
            } else if pseudo_fd == FD_OPENSSL_CNF {
                PSEUDO_OFF_OPENSSL_CNF = 0;
            }
            return Some(pseudo_fd);
        }
    }
    None
}

/// Build a Linux `st_dev` / `st_rdev` value from major/minor numbers.
fn makedev(major: u32, minor: u32) -> u64 {
    ((minor & 0xFF) | ((major & 0xFFF) << 8) | ((minor & !0xFF) << 12)) as u64
}

/// Fill a Linux stat structure for a pseudo character device or config file.
fn fill_pseudo_stat(st: &mut LinuxStat, fd: u64, rdev: u64) {
    st.st_dev   = 0;
    st.st_ino   = fd;
    if fd == FD_RESOLV_CONF {
        st.st_mode = 0x8000 | 0o644; // S_IFREG | rw-r--r--
        st.st_size = RESOLV_CONF_DATA.len() as i64;
    } else if fd == FD_HOSTS {
        st.st_mode = 0x8000 | 0o644; // S_IFREG | rw-r--r--
        st.st_size = HOSTS_DATA.len() as i64;
    } else if fd == FD_CA_CERTS {
        st.st_mode = 0x8000 | 0o644; // S_IFREG | rw-r--r--
        st.st_size = CA_CERTS_DATA.len() as i64;
    } else if fd == FD_OPENSSL_CNF {
        st.st_mode = 0x8000 | 0o644; // S_IFREG | rw-r--r--
        st.st_size = OPENSSL_CNF_DATA.len() as i64;
    } else {
        st.st_mode  = 0x2000 | 0o666; // S_IFCHR | rw-rw-rw-
        st.st_size  = 0;
    }
    st.st_nlink = 1;
    st.st_uid   = 0;
    st.st_gid   = 0;
    st.st_rdev  = rdev;
    st.st_blksize = 4096;
    st.st_blocks  = 0;
}

fn pseudo_rdev(fd: u64) -> u64 {
    match fd {
        FD_TTY     => makedev(5, 0),   // /dev/tty
        FD_NULL    => makedev(1, 3),   // /dev/null
        FD_URANDOM => makedev(1, 9),   // /dev/urandom
        FD_ZERO    => makedev(1, 5),   // /dev/zero
        _ => 0,
    }
}

// ── Syscall handlers ──────────────────────────────────────────────────────

unsafe fn sys_read(fd: u64, buf: *mut u8, count: usize) -> i64 {
    if count == 0 || buf.is_null() { return 0; }

    match fd {
        0 | FD_TTY => {
            // stdin / /dev/tty: read from keyboard
            let c = keyboard_getc();
            if c < 0 { return 0; }
            *buf = c as u8;
            1
        }
        FD_URANDOM => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            fill_random(buf, count);
            count as i64
        }
        FD_ZERO => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            ptr::write_bytes(buf, 0, count);
            count as i64
        }
        FD_NULL => {
            // /dev/null: EOF
            0
        }
        FD_RESOLV_CONF => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            let avail = RESOLV_CONF_DATA.len().saturating_sub(PSEUDO_OFF_RESOLV);
            let to_copy = avail.min(count);
            if to_copy > 0 {
                ptr::copy_nonoverlapping(RESOLV_CONF_DATA.as_ptr().add(PSEUDO_OFF_RESOLV), buf, to_copy);
                PSEUDO_OFF_RESOLV += to_copy;
            }
            to_copy as i64
        }
        FD_HOSTS => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            let avail = HOSTS_DATA.len().saturating_sub(PSEUDO_OFF_HOSTS);
            let to_copy = avail.min(count);
            if to_copy > 0 {
                ptr::copy_nonoverlapping(HOSTS_DATA.as_ptr().add(PSEUDO_OFF_HOSTS), buf, to_copy);
                PSEUDO_OFF_HOSTS += to_copy;
            }
            to_copy as i64
        }
        FD_CA_CERTS => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            let avail = CA_CERTS_DATA.len().saturating_sub(PSEUDO_OFF_CA_CERTS);
            let to_copy = avail.min(count);
            if to_copy > 0 {
                ptr::copy_nonoverlapping(CA_CERTS_DATA.as_ptr().add(PSEUDO_OFF_CA_CERTS), buf, to_copy);
                PSEUDO_OFF_CA_CERTS += to_copy;
            }
            to_copy as i64
        }
        FD_OPENSSL_CNF => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            let avail = OPENSSL_CNF_DATA.len().saturating_sub(PSEUDO_OFF_OPENSSL_CNF);
            let to_copy = avail.min(count);
            if to_copy > 0 {
                ptr::copy_nonoverlapping(OPENSSL_CNF_DATA.as_ptr().add(PSEUDO_OFF_OPENSSL_CNF), buf, to_copy);
                PSEUDO_OFF_OPENSSL_CNF += to_copy;
            }
            to_copy as i64
        }
        _ if fd >= 512 => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            let ret = lwip_read(fd as i32, buf as *mut c_void, count) as i64;
            if ret < 0 {
                let err = linux_helper_get_errno();
                if err != 0 { -(err as i64) } else { -1 }
            } else {
                ret
            }
        }
        _ => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            vfs_read(fd as i32, buf, count as u32) as i64
        }
    }
}

unsafe fn sys_write(fd: u64, buf: *const u8, count: usize) -> i64 {
    if buf.is_null() { return 0; }

    match fd {
        FD_NULL | FD_ZERO | FD_URANDOM => {
            // /dev/null, /dev/zero, /dev/urandom: discard writes
            count as i64
        }
        FD_TTY => {
            // /dev/tty: write to stdout (fd 1)
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            vfs_write(1, buf, count as u32) as i64
        }
        _ if fd >= 512 => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            let ret = lwip_write(fd as i32, buf as *const c_void, count) as i64;
            if ret < 0 {
                let err = linux_helper_get_errno();
                if err != 0 { -(err as i64) } else { -1 }
            } else {
                ret
            }
        }
        _ => {
            if is_user_range(buf, count) == 0 { return -EFAULT; }
            vfs_write(fd as i32, buf, count as u32) as i64
        }
    }
}

unsafe fn sys_open(path: *const u8, flags: i32, _mode: u32) -> i64 {
    if path.is_null() || is_user_string(path) == 0 {
        return -EFAULT;
    }
    if let Some(pfd) = path_is_pseudo(path) {
        return pfd as i64;
    }
    let pureos_flags = translate_open_flags(flags);
    let fd = vfs_open(path, pureos_flags);
    if fd < 0 {
        return -ENOENT;
    }
    fd as i64
}

unsafe fn sys_openat(_dirfd: i32, path: *const u8, flags: i32, mode: u32) -> i64 {
    sys_open(path, flags, mode)
}

unsafe fn sys_newfstatat(_dirfd: i32, path: *const u8, stat_buf: *mut u8, _flags: i32) -> i64 {
    sys_stat(path, stat_buf)
}

unsafe fn sys_close(fd: u64) -> i64 {
    if is_pseudo_fd(fd) {
        if fd == FD_RESOLV_CONF {
            PSEUDO_OFF_RESOLV = 0;
        } else if fd == FD_HOSTS {
            PSEUDO_OFF_HOSTS = 0;
        } else if fd == FD_CA_CERTS {
            PSEUDO_OFF_CA_CERTS = 0;
        } else if fd == FD_OPENSSL_CNF {
            PSEUDO_OFF_OPENSSL_CNF = 0;
        }
        return 0;
    }
    if fd >= 512 && fd < 1024 {
        lwip_close(fd as i32);
    } else {
        vfs_close(fd as i32);
    }
    0
}

unsafe fn sys_readlink(path: *const u8, buf: *mut u8, bufsiz: u64) -> i64 {
    if path.is_null() || is_user_string(path) == 0 { return -EFAULT; }
    if buf.is_null() || is_user_range(buf, bufsiz as usize) == 0 { return -EFAULT; }
    vfs_readlink(path, buf, bufsiz as u32) as i64
}

unsafe fn sys_unlink(pathname: *const u8) -> i64 {
    if pathname.is_null() || is_user_string(pathname) == 0 { return -EFAULT; }
    vfs_unlink(pathname) as i64
}

unsafe fn sys_symlink(target: *const u8, linkpath: *const u8) -> i64 {
    if target.is_null() || is_user_string(target) == 0 { return -EFAULT; }
    if linkpath.is_null() || is_user_string(linkpath) == 0 { return -EFAULT; }
    vfs_symlink(target, linkpath) as i64
}

unsafe fn sys_chmod(pathname: *const u8, mode: u32) -> i64 {
    if pathname.is_null() || is_user_string(pathname) == 0 { return -EFAULT; }
    vfs_chmod(pathname, mode) as i64
}

unsafe fn sys_mkdir(pathname: *const u8, _mode: u32) -> i64 {
    if pathname.is_null() || is_user_string(pathname) == 0 { return -EFAULT; }
    vfs_mkdir(pathname) as i64
}

unsafe fn sys_lseek(fd: u64, offset: i64, whence: i32) -> i64 {
    if fd == FD_RESOLV_CONF {
        let new_off = match whence {
            0 => offset,
            1 => PSEUDO_OFF_RESOLV as i64 + offset,
            2 => RESOLV_CONF_DATA.len() as i64 + offset,
            _ => return -EINVAL,
        };
        if new_off < 0 { return -EINVAL; }
        PSEUDO_OFF_RESOLV = new_off as usize;
        return new_off;
    }
    if fd == FD_HOSTS {
        let new_off = match whence {
            0 => offset,
            1 => PSEUDO_OFF_HOSTS as i64 + offset,
            2 => HOSTS_DATA.len() as i64 + offset,
            _ => return -EINVAL,
        };
        if new_off < 0 { return -EINVAL; }
        PSEUDO_OFF_HOSTS = new_off as usize;
        return new_off;
    }
    if fd == FD_CA_CERTS {
        let new_off = match whence {
            0 => offset,
            1 => PSEUDO_OFF_CA_CERTS as i64 + offset,
            2 => CA_CERTS_DATA.len() as i64 + offset,
            _ => return -EINVAL,
        };
        if new_off < 0 { return -EINVAL; }
        PSEUDO_OFF_CA_CERTS = new_off as usize;
        return new_off;
    }
    if fd == FD_OPENSSL_CNF {
        let new_off = match whence {
            0 => offset,
            1 => PSEUDO_OFF_OPENSSL_CNF as i64 + offset,
            2 => OPENSSL_CNF_DATA.len() as i64 + offset,
            _ => return -EINVAL,
        };
        if new_off < 0 { return -EINVAL; }
        PSEUDO_OFF_OPENSSL_CNF = new_off as usize;
        return new_off;
    }
    if is_pseudo_fd(fd) { return -ESPIPE; }
    vfs_lseek(fd as i32, offset as u64, whence) as i64
}

unsafe fn sys_stat(path: *const u8, statbuf: *mut u8) -> i64 {
    if path.is_null() || is_user_string(path) == 0 { return -EFAULT; }
    if statbuf.is_null() || is_user_range(statbuf, core::mem::size_of::<LinuxStat>()) == 0 {
        return -EFAULT;
    }
    if let Some(pfd) = path_is_pseudo(path) {
        let mut linux_st: LinuxStat = core::mem::zeroed();
        fill_pseudo_stat(&mut linux_st, pfd, pseudo_rdev(pfd));
        ptr::copy_nonoverlapping(&linux_st as *const _ as *const u8, statbuf, core::mem::size_of::<LinuxStat>());
        return 0;
    }
    let mut pureos: PureOSStat = core::mem::zeroed();
    let ret = vfs_stat(path, &mut pureos as *mut _ as *mut u8);
    if ret < 0 { return ret as i64; }

    let mut linux_st: LinuxStat = core::mem::zeroed();
    translate_stat(&pureos, &mut linux_st);
    ptr::copy_nonoverlapping(&linux_st as *const _ as *const u8, statbuf, core::mem::size_of::<LinuxStat>());
    0
}

unsafe fn sys_fstat(fd: u64, statbuf: *mut u8) -> i64 {
    if statbuf.is_null() || is_user_range(statbuf, core::mem::size_of::<LinuxStat>()) == 0 {
        return -EFAULT;
    }
    if is_pseudo_fd(fd) {
        let mut linux_st: LinuxStat = core::mem::zeroed();
        fill_pseudo_stat(&mut linux_st, fd, pseudo_rdev(fd));
        ptr::copy_nonoverlapping(&linux_st as *const _ as *const u8, statbuf, core::mem::size_of::<LinuxStat>());
        return 0;
    }
    let mut pureos: PureOSStat = core::mem::zeroed();
    let ret = vfs_fstat(fd as i32, &mut pureos as *mut _ as *mut u8);
    if ret < 0 { return ret as i64; }

    let mut linux_st: LinuxStat = core::mem::zeroed();
    translate_stat(&pureos, &mut linux_st);
    ptr::copy_nonoverlapping(&linux_st as *const _ as *const u8, statbuf, core::mem::size_of::<LinuxStat>());
    0
}

unsafe fn sys_mmap(addr: u64, length: u64, prot: i32, flags: i32, fd: i64, _offset: i64) -> u64 {
    if length == 0 { return -EINVAL as u64; }

    // File-backed mmap not yet supported (Phase 4+)
    if (flags & MAP_ANONYMOUS) == 0 && fd >= 0 {
        return MAP_FAILED;
    }

    let pages = ((length + 4095) / 4096) as usize;

    // Derive x86 page-table flags from Linux prot:
    //  0x2 = writable, 0x4 = executable
    //  (Present and User are always forced by paging_map_user_page)
    let page_flags =
        if (prot & PROT_WRITE) != 0 { 0x2 } else { 0 }
        | if (prot & PROT_EXEC)  != 0 { 0x4 } else { 0 };

    if (flags & MAP_FIXED) != 0 {
        if addr == 0 || (addr & 0xFFF) != 0 { return MAP_FAILED; }
        for i in 0..pages {
            let phys = linux_helper_frame_alloc();
            if phys == 0 { return MAP_FAILED; }
            let ptr = phys as *mut u8;
            ptr::write_bytes(ptr, 0, 4096);
            paging_map_user_page(
                linux_helper_get_pagedir(),
                addr + (i as u64) * 4096,
                phys as u64,
                page_flags,
            );
        }
        return addr;
    }

    // Pick a virtual address
    let cur = linux_helper_get_mmap();
    let actual_addr = if addr != 0 && (addr & 0xFFF) == 0 && addr >= cur {
        let next = addr.wrapping_add((pages as u64) * 4096);
        linux_helper_set_mmap(next);
        addr
    } else {
        let next = cur.wrapping_add((pages as u64) * 4096);
        linux_helper_set_mmap(next);
        cur
    };

    // Allocate and zero physical pages
    for i in 0..pages {
        let phys = linux_helper_frame_alloc();
        if phys == 0 { return MAP_FAILED; }
        let ptr = phys as *mut u8;
        ptr::write_bytes(ptr, 0, 4096);
        paging_map_user_page(
            linux_helper_get_pagedir(),
            actual_addr + (i as u64) * 4096,
            phys as u64,
            page_flags,
        );
    }

    actual_addr
}

unsafe fn sys_munmap(addr: u64, length: u64) -> i64 {
    if length == 0 { return -EINVAL; }
    linux_helper_munmap(addr, length) as i64
}

unsafe fn sys_mprotect(addr: u64, len: u64, prot: u64) -> i64 {
    if len == 0 { return 0; }
    linux_helper_mprotect(addr, len, prot as i32) as i64
}

unsafe fn sys_brk(addr: u64) -> i64 {
    let mut current = linux_helper_get_brk();
    if current == 0 {
        // First call fallback if not set by ELF loader: 32MB
        current = 0x02000000;
        linux_helper_set_brk(current);
    }

    // brk(0) – query current break
    if addr == 0 {
        return current as i64;
    }

    // Shrinking heap or no change
    if addr <= current {
        linux_helper_set_brk(addr);
        return addr as i64;
    }

    // Expand heap: map new pages from PAGE_ALIGN(current) to PAGE_ALIGN(addr)
    let start_page = (current + 4095) & !4095u64;
    let end_page   = (addr    + 4095) & !4095u64;
    if end_page > start_page {
        for va in (start_page..end_page).step_by(4096) {
            let phys = linux_helper_frame_alloc();
            if phys == 0 {
                // On failure, Linux brk returns current unmodified break
                return current as i64;
            }
            let ptr = phys as *mut u8;
            ptr::write_bytes(ptr, 0, 4096);
            paging_map_user_page(linux_helper_get_pagedir(), va, phys as u64, 0x3);
        }
    }

    linux_helper_set_brk(addr);
    addr as i64
}

unsafe fn sys_ioctl(fd: u64, request: u64, arg: u64) -> i64 {
    if fd <= 2 || is_pseudo_fd(fd) {
        match request {
            0x5401 /* TCGETS */ => {
                if arg != 0 && is_user_range(arg as *const u8, 36) != 0 {
                    ptr::write_bytes(arg as *mut u8, 0, 36);
                    return 0;
                }
                return -EINVAL;
            }
            0x5402 /* TCSETS */ => return 0,
            0x5413 /* TIOCGWINSZ */ => {
                if arg != 0 && is_user_range(arg as *const u8, 8) != 0 {
                    ptr::write((arg as *mut u16).offset(0), 24); // row
                    ptr::write((arg as *mut u16).offset(1), 80); // col
                    ptr::write((arg as *mut u16).offset(2), 0);
                    ptr::write((arg as *mut u16).offset(3), 0);
                    return 0;
                }
                return -EINVAL;
            }
            0x541B /* FIONREAD */ => {
                if arg != 0 && is_user_range(arg as *const u8, 4) != 0 {
                    ptr::write(arg as *mut i32, 1); // Assume 1 byte available to unblock
                    return 0;
                }
                return -EINVAL;
            }
            _ => return -ENOTTY,
        }
    }
    -ENOTTY
}

unsafe fn sys_pipe(pipefd: *mut i32) -> i64 {
    pipe(pipefd) as i64
}

unsafe fn sys_dup(oldfd: u64) -> i64 {
    vfs_dup2(oldfd as i32, -1) as i64
}

unsafe fn sys_dup2(oldfd: u64, newfd: u64) -> i64 {
    vfs_dup2(oldfd as i32, newfd as i32) as i64
}

unsafe fn sys_fcntl(fd: i32, cmd: i32, arg: u64) -> i64 {
    // Forward F_GETFL / F_SETFL to lwIP for socket fds
    if fd >= 512 {
        match cmd {
            3 => { // F_GETFL
                let lwip_flags = lwip_fcntl(fd, 3, 0);
                // Translate lwIP O_NONBLOCK (1) to Linux O_NONBLOCK (0x800)
                let mut linux_flags = O_RDWR as i64;
                if lwip_flags & 1 != 0 { linux_flags |= 0x800; } // O_NONBLOCK
                return linux_flags;
            }
            4 => { // F_SETFL
                // Translate Linux O_NONBLOCK (0x800) to lwIP O_NONBLOCK (1)
                let lwip_val = if (arg as i32) & 0x800 != 0 { 1 } else { 0 };
                return lwip_fcntl(fd, 4, lwip_val) as i64;
            }
            1 | 2 => return 0, // F_GETFD / F_SETFD
            _ => {}
        }
    }
    match cmd {
        0 => { // F_DUPFD
            vfs_dup2(fd, arg as i32) as i64
        }
        1 => { // F_GETFD – close-on-exec flag
            0
        }
        2 => { // F_SETFD – set close-on-exec flag
            let _ = arg;
            0
        }
        3 => { // F_GETFL – file access flags
            // Return reasonable defaults: they can read+write
            (O_RDWR | O_APPEND) as i64
        }
        4 => { // F_SETFL – set file access flags
            let _ = arg;
            0
        }
        _ => -EINVAL,
    }
}

unsafe fn sys_getdents64(fd: u64, dirp: *mut u8, count: u64) -> i64 {
    if dirp.is_null() || count == 0 { return -EINVAL; }
    if is_user_range(dirp, count as usize) == 0 { return -EFAULT; }
    if is_pseudo_fd(fd) { return -ENOTDIR; }

    let current_idx = vfs_lseek(fd as i32, 0, 1); // SEEK_CUR
    let dentry_ptr = vfs_readdir(fd as i32, current_idx as u32);
    if dentry_ptr.is_null() {
        return 0; // EOF
    }

    let dentry = &*dentry_ptr;
    // Calculate string length of dentry.name
    let mut name_len = 0;
    while name_len < 128 && dentry.name[name_len] != 0 {
        name_len += 1;
    }

    // reclen = sizeof(LinuxDirent64) minus the 1 byte for d_name, plus name_len, plus null terminator, aligned to 8 bytes
    let base_size = 19; // sizeof(LinuxDirent64) without d_name
    let mut reclen = base_size + name_len + 1;
    reclen = (reclen + 7) & !7;

    if (reclen as u64) > count {
        return -EINVAL; // buffer too small
    }

    // Advance directory offset by 1
    vfs_lseek(fd as i32, current_idx + 1, 0); // SEEK_SET

    // Write to user buffer
    let mut dirent: LinuxDirent64 = core::mem::zeroed();
    dirent.d_ino = 1; // Fake inode if not easily accessible
    dirent.d_off = (current_idx + 1) as i64;
    dirent.d_reclen = reclen as u16;
    dirent.d_type = 4; // DT_DIR (just an assumption, could be 8 for DT_REG)

    ptr::copy_nonoverlapping(&dirent as *const _ as *const u8, dirp, base_size);
    ptr::copy_nonoverlapping(dentry.name.as_ptr(), dirp.add(base_size), name_len);
    ptr::write_bytes(dirp.add(base_size + name_len), 0, reclen - base_size - name_len);

    reclen as i64
}

unsafe fn sys_writev(fd: u64, iov: *const Iovec, iovcnt: i32) -> i64 {
    if iov.is_null() || iovcnt <= 0 || iovcnt > 1024 { return -EINVAL; }
    if is_user_range(iov as *const u8, (iovcnt as usize) * core::mem::size_of::<Iovec>()) == 0 { return -EFAULT; }
    let mut total: i64 = 0;
    for i in 0..iovcnt {
        let entry = &*iov.offset(i as isize);
        if entry.iov_len == 0 { continue; }
        if entry.iov_base.is_null() { continue; }
        if is_user_range(entry.iov_base, entry.iov_len) == 0 { return -EFAULT; }
        let n = sys_write(fd, entry.iov_base, entry.iov_len);
        if n < 0 { return if total > 0 { total } else { n }; }
        total += n;
    }
    total
}

unsafe fn sys_readv(fd: u64, iov: *const Iovec, iovcnt: i32) -> i64 {
    if iov.is_null() || iovcnt <= 0 || iovcnt > 1024 { return -EINVAL; }
    if is_user_range(iov as *const u8, (iovcnt as usize) * core::mem::size_of::<Iovec>()) == 0 { return -EFAULT; }
    let mut total: i64 = 0;
    for i in 0..iovcnt {
        let entry = &*iov.offset(i as isize);
        if entry.iov_len == 0 || entry.iov_base.is_null() { continue; }
        if is_user_range(entry.iov_base, entry.iov_len) == 0 { return -EFAULT; }
        let n = sys_read(fd, entry.iov_base, entry.iov_len);
        if n < 0 { return if total > 0 { total } else { n }; }
        total += n;
    }
    total
}

unsafe fn sys_uname(buf: *mut u8) -> i64 {
    if buf.is_null() || is_user_range(buf, core::mem::size_of::<LinuxUtsname>()) == 0 {
        return -EFAULT;
    }
    let mut uts: LinuxUtsname = core::mem::zeroed();
    str_to_fixed(&mut uts.sysname,    b"Linux");
    str_to_fixed(&mut uts.nodename,   b"pureos");
    str_to_fixed(&mut uts.release,    b"6.1.0-pureos");
    str_to_fixed(&mut uts.version,    b"#1 SMP");
    str_to_fixed(&mut uts.machine,    b"x86_64");
    str_to_fixed(&mut uts.domainname, b"(none)");
    ptr::copy_nonoverlapping(&uts as *const _ as *const u8, buf, core::mem::size_of::<LinuxUtsname>());
    0
}

unsafe fn sys_exit(_status: u64) -> i64 {
    linux_helper_exit();
    0 // unreachable
}

unsafe fn sys_exit_group(_status: u64) -> i64 {
    linux_helper_exit();
    0
}

unsafe fn sys_getpid() -> i64 {
    linux_helper_get_pid() as i64
}

unsafe fn sys_getuid() -> i64 {
    linux_helper_get_uid() as i64
}

unsafe fn sys_getgid() -> i64 {
    linux_helper_get_gid() as i64
}

unsafe fn sys_nanosleep(req: *const Timespec, _rem: *mut Timespec) -> i64 {
    if req.is_null() || is_user_range(req as *const u8, core::mem::size_of::<Timespec>()) == 0 {
        return -EFAULT;
    }
    // Stub: nanosleep is a busy-wait yield.  A real OS would use a timer.
    // For now, we just return success without actually sleeping.
    let _ = req;
    0
}

unsafe fn sys_clock_gettime(clock_id: u64, tp: *mut Timespec) -> i64 {
    if tp.is_null() || is_user_range(tp as *const u8, core::mem::size_of::<Timespec>()) == 0 {
        return -EFAULT;
    }

    let ms = get_timer_ms_hires();

    match clock_id {
        0 /* CLOCK_REALTIME */ | 5 /* CLOCK_REALTIME_COARSE */ | 11 /* CLOCK_TAI */ => {
            let mut rtc: RtcTime = core::mem::zeroed();
            rtc_read(&mut rtc as *mut _ as *mut u8);

            let raw_year = rtc.year as u64;
            let y = if raw_year < 100 { 2000 + raw_year } else { raw_year };
            let days_in_month = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
            
            let mut days = 0;
            for i in 1970..y {
                days += if i % 4 == 0 && (i % 100 != 0 || i % 400 == 0) { 366 } else { 365 };
            }
            let m = (rtc.month as usize).clamp(1, 12);
            for i in 1..m {
                days += days_in_month[i];
                if i == 2 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0) {
                    days += 1;
                }
            }
            days += (rtc.day as u64).saturating_sub(1);
            
            let total_secs = days * 86400 + (rtc.hour as u64) * 3600 + (rtc.minute as u64) * 60 + (rtc.second as u64);
            let sub_ms = ms % 1000;

            let ts = Timespec {
                tv_sec:  total_secs as i64,
                tv_nsec: (sub_ms * 1_000_000) as i64,
            };
            ptr::write(tp, ts);
            0
        }
        1 /* CLOCK_MONOTONIC */ | 4 /* CLOCK_MONOTONIC_RAW */ | 6 /* CLOCK_MONOTONIC_COARSE */ |
        7 /* CLOCK_BOOTTIME */ | 2 /* CLOCK_PROCESS_CPUTIME_ID */ | 3 /* CLOCK_THREAD_CPUTIME_ID */ => {
            let ts = Timespec {
                tv_sec:  (ms / 1000) as i64,
                tv_nsec: ((ms % 1000) * 1_000_000) as i64,
            };
            ptr::write(tp, ts);
            0
        }
        _ => {
            let ts = Timespec {
                tv_sec:  (ms / 1000) as i64,
                tv_nsec: ((ms % 1000) * 1_000_000) as i64,
            };
            ptr::write(tp, ts);
            0
        }
    }
}

unsafe fn sys_access(pathname: *const u8, _mode: u32) -> i64 {
    if pathname.is_null() || is_user_string(pathname) == 0 { return -EFAULT; }
    if path_is_pseudo(pathname).is_some() { return 0; }
    let mut st: PureOSStat = core::mem::zeroed();
    let ret = vfs_stat(pathname, &mut st as *mut _ as *mut u8);
    if ret < 0 { -ENOENT } else { 0 }
}

unsafe fn sys_faccessat(dfd: u64, pathname: *const u8, _mode: u32, _flags: u32) -> i64 {
    let _ = dfd;
    if pathname.is_null() || is_user_string(pathname) == 0 { return -EFAULT; }
    if path_is_pseudo(pathname).is_some() { return 0; }
    let mut st: PureOSStat = core::mem::zeroed();
    let ret = vfs_stat(pathname, &mut st as *mut _ as *mut u8);
    if ret < 0 { -ENOENT } else { 0 }
}

unsafe fn sys_getcwd(buf: *mut u8, size: u64) -> i64 {
    if buf.is_null() || size == 0 { return -EINVAL; }
    if is_user_range(buf, size as usize) == 0 { return -EFAULT; }
    linux_helper_get_cwd(buf, size as i32);
    // Determine string length to return as bytes read
    let mut len = 0;
    while len < size && *buf.offset(len as isize) != 0 {
        len += 1;
    }
    (len + 1) as i64 // Include null terminator
}

unsafe fn sys_chdir(path: *const u8) -> i64 {
    if path.is_null() || is_user_string(path) == 0 { return -EFAULT; }
    // Ideally verify directory exists via VFS here
    linux_helper_set_cwd(path);
    0
}

unsafe fn sys_clone(regs_ptr: *const c_void, flags: u64, child_stack: u64, parent_tid: u64, child_tid: u64, child_tls: u64) -> i64 {
    linux_helper_clone_thread(regs_ptr, flags, child_stack, parent_tid, child_tid, child_tls)
}

unsafe fn sys_fork() -> i64 {
    -ENOSYS
}

unsafe fn sys_execve(_path: *const u8, _argv: *const u8, _envp: *const u8) -> i64 {
    // For now, return -ENOEXEC.  Phase 3+ will add Linux ELF exec.
    -ENOEXEC
}

unsafe fn sys_wait4(_pid: i64, _wstatus: *mut i32, _options: i32, _rusage: *mut u8) -> i64 {
    -ECHILD
}

unsafe fn sys_kill(_pid: i64, _sig: i32) -> i64 {
    0 // stub – return success (ignore signals)
}

unsafe fn sys_tkill(_tid: i64, _sig: i32) -> i64 {
    0
}

unsafe fn sys_getrandom(buf: *mut u8, len: u64, _flags: u32) -> i64 {
    if buf.is_null() || is_user_range(buf, len as usize) == 0 { return -EFAULT; }
    fill_random(buf, len as usize);
    len as i64
}

unsafe fn sys_rseq() -> i64 {
    0 // stub: return success (rseq is optional)
}

unsafe fn sys_set_tid_address(tidptr: *mut i32) -> i64 {
    if !tidptr.is_null() && is_user_range(tidptr as *const u8, 4) != 0 {
        linux_helper_set_clear_tid(tidptr as u64);
    }
    linux_helper_get_tid() as i64
}

unsafe fn sys_sched_yield() -> i64 {
    linux_helper_yield();
    0
}

#[repr(C)]
#[derive(Clone, Copy)]
struct PollFd {
    fd: i32,
    events: i16,
    revents: i16,
}

unsafe fn sys_select(nfds: i32, readfds: *mut u8, writefds: *mut u8, exceptfds: *mut u8, timeout: *const u8) -> i64 {
    if !readfds.is_null() && is_user_range(readfds, 128) == 0 { return -EFAULT; }
    if !writefds.is_null() && is_user_range(writefds, 128) == 0 { return -EFAULT; }
    if !exceptfds.is_null() && is_user_range(exceptfds, 128) == 0 { return -EFAULT; }
    if !timeout.is_null() && is_user_range(timeout, 16) == 0 { return -EFAULT; }

    // Clear any non-LwIP FDs (fd < 512) to prevent lwip_select from returning EBADF
    // and causing an infinite busy-loop.
    let mut max_lwip_fd = -1;
    let clear_non_lwip = |fds: *mut u8| {
        if fds.is_null() { return; }
        let fd_array = fds as *mut u64;
        for i in 0..8 { // 512 bits / 64 = 8 u64s
            ptr::write(fd_array.add(i), 0);
        }
    };
    clear_non_lwip(readfds);
    clear_non_lwip(writefds);
    clear_non_lwip(exceptfds);

    if nfds <= 512 {
        // Only local FDs were polled. Just sleep to prevent busy loop.
        let timeout_ms = if !timeout.is_null() { 
            let sec = ptr::read_unaligned(timeout as *const i64);
            let usec = ptr::read_unaligned(timeout.add(8) as *const i64);
            (sec * 1000 + usec / 1000) as i32
        } else { 10 };
        if timeout_ms > 0 { lwip_poll(core::ptr::null_mut(), 0, timeout_ms); }
        return 0;
    }

    lwip_select(nfds, readfds as *mut c_void, writefds as *mut c_void, exceptfds as *mut c_void, timeout as *mut c_void) as i64
}

// Linux poll constants (from <poll.h>)
const LINUX_POLLIN: i16     = 0x0001;
const LINUX_POLLPRI: i16    = 0x0002;
const LINUX_POLLOUT: i16    = 0x0004;
const LINUX_POLLERR: i16    = 0x0008;
const LINUX_POLLHUP: i16    = 0x0010;
const LINUX_POLLNVAL: i16   = 0x0020;
const LINUX_POLLRDNORM: i16 = 0x0040;
const LINUX_POLLRDBAND: i16 = 0x0080;
const LINUX_POLLWRNORM: i16 = 0x0100;
const LINUX_POLLWRBAND: i16 = 0x0200;

// LwIP poll constants (from lwip/sockets.h)
const LWIP_POLLIN: i16     = 0x0001;
const LWIP_POLLOUT: i16    = 0x0002;
const LWIP_POLLERR: i16    = 0x0004;
const LWIP_POLLNVAL: i16   = 0x0008;
const LWIP_POLLRDNORM: i16 = 0x0010;
const LWIP_POLLRDBAND: i16 = 0x0020;
const LWIP_POLLPRI: i16    = 0x0040;
const LWIP_POLLWRNORM: i16 = 0x0080;
const LWIP_POLLWRBAND: i16 = 0x0100;
const LWIP_POLLHUP: i16    = 0x0200;

fn linux_poll_events_to_lwip(events: i16) -> i16 {
    let mut lwip_events: i16 = 0;
    if (events & (LINUX_POLLIN | LINUX_POLLRDNORM)) != 0 {
        lwip_events |= LWIP_POLLIN | LWIP_POLLRDNORM;
    }
    if (events & LINUX_POLLRDBAND) != 0 {
        lwip_events |= LWIP_POLLRDBAND;
    }
    if (events & LINUX_POLLPRI) != 0 {
        lwip_events |= LWIP_POLLPRI;
    }
    if (events & (LINUX_POLLOUT | LINUX_POLLWRNORM)) != 0 {
        lwip_events |= LWIP_POLLOUT | LWIP_POLLWRNORM;
    }
    if (events & LINUX_POLLWRBAND) != 0 {
        lwip_events |= LWIP_POLLWRBAND;
    }
    if (events & LINUX_POLLERR) != 0 {
        lwip_events |= LWIP_POLLERR;
    }
    if (events & LINUX_POLLHUP) != 0 {
        lwip_events |= LWIP_POLLHUP;
    }
    if (events & LINUX_POLLNVAL) != 0 {
        lwip_events |= LWIP_POLLNVAL;
    }
    lwip_events
}

fn lwip_poll_revents_to_linux(revents: i16) -> i16 {
    let mut linux_revents: i16 = 0;
    if (revents & (LWIP_POLLIN | LWIP_POLLRDNORM)) != 0 {
        linux_revents |= LINUX_POLLIN | LINUX_POLLRDNORM;
    }
    if (revents & LWIP_POLLRDBAND) != 0 {
        linux_revents |= LINUX_POLLRDBAND;
    }
    if (revents & LWIP_POLLPRI) != 0 {
        linux_revents |= LINUX_POLLPRI;
    }
    if (revents & (LWIP_POLLOUT | LWIP_POLLWRNORM)) != 0 {
        linux_revents |= LINUX_POLLOUT | LINUX_POLLWRNORM;
    }
    if (revents & LWIP_POLLWRBAND) != 0 {
        linux_revents |= LINUX_POLLWRBAND;
    }
    if (revents & LWIP_POLLERR) != 0 {
        linux_revents |= LINUX_POLLERR;
    }
    if (revents & LWIP_POLLHUP) != 0 {
        linux_revents |= LINUX_POLLHUP;
    }
    if (revents & LWIP_POLLNVAL) != 0 {
        linux_revents |= LINUX_POLLNVAL;
    }
    linux_revents
}

unsafe fn sys_poll(fds: *mut u8, nfds: u64, timeout: i32) -> i64 {
    if fds.is_null() || nfds == 0 {
        return lwip_poll(core::ptr::null_mut(), 0, timeout) as i64;
    }
    if is_user_range(fds, (nfds * 8) as usize) != 0 {
        let user_pollfds = fds as *mut PollFd;
        let n = core::cmp::min(nfds as usize, 128);
        
        let mut k_pollfds = [PollFd { fd: 0, events: 0, revents: 0 }; 128];
        let mut original_fds = [0i32; 128];
        let mut has_eventfd = false;
        
        for i in 0..n {
            let u_pfd = &*user_pollfds.add(i);
            original_fds[i] = u_pfd.fd;
            if u_pfd.fd >= 512 {
                k_pollfds[i].fd = u_pfd.fd;
                k_pollfds[i].events = linux_poll_events_to_lwip(u_pfd.events);
                k_pollfds[i].revents = 0;
            } else {
                if u_pfd.fd >= 0 && linux_helper_is_eventfd(u_pfd.fd) != 0 {
                    has_eventfd = true;
                }
                k_pollfds[i].fd = -1; // Mask out non-LwIP FDs to prevent POLLNVAL aborts
                k_pollfds[i].events = 0;
                k_pollfds[i].revents = 0;
            }
        }
        
        let effective_timeout = if has_eventfd && (timeout < 0 || timeout > 20) {
            20
        } else {
            timeout
        };
        
        // Pass kernel stack buffer k_pollfds so tcpip_thread can safely dereference it without page faulting on user memory
        let _ = lwip_poll(k_pollfds.as_mut_ptr() as *mut c_void, n as u32, effective_timeout);
        
        let mut ret = 0i64;
        for i in 0..n {
            let u_pfd = &mut *user_pollfds.add(i);
            if original_fds[i] >= 512 {
                let linux_rev = lwip_poll_revents_to_linux(k_pollfds[i].revents);
                u_pfd.revents = linux_rev;
                if linux_rev != 0 {
                    ret += 1;
                    print_serial(b"  [poll] fd=\0".as_ptr());
                    print_hex_field(b"\0".as_ptr(), original_fds[i] as u64);
                    print_serial(b" lwip=0x\0".as_ptr());
                    print_hex_field(b"\0".as_ptr(), k_pollfds[i].revents as u64);
                    print_serial(b" linux=0x\0".as_ptr());
                    print_hex_field(b"\0".as_ptr(), linux_rev as u64);
                }
            } else if original_fds[i] >= 0 {
                if linux_helper_is_eventfd(original_fds[i]) != 0 {
                    let ev_rev = linux_helper_eventfd_poll(original_fds[i], u_pfd.events as i32) as i16;
                    u_pfd.revents = ev_rev;
                    if ev_rev != 0 {
                        ret += 1;
                    }
                } else {
                    u_pfd.revents = 0;
                }
            } else {
                u_pfd.revents = 0;
            }
        }
        return ret;
    }
    -EFAULT
}

unsafe fn sys_socketpair(domain: i32, type_: i32, protocol: i32, sv: *mut i32) -> i64 {
    let _ = (domain, type_, protocol);
    // Fall back to pureos pipe-based socketpair
    if sv.is_null() || is_user_range(sv as *const u8, 8) == 0 { return -EFAULT; }
    pipe(sv) as i64
}

#[repr(C)]
struct LwipSockAddr {
    sa_len: u8,
    sa_family: u8,
    sa_data: [u8; 14],
}

unsafe fn convert_sockaddr(linux_addr: *const u8) -> LwipSockAddr {
    let mut lwip_addr = LwipSockAddr { sa_len: 16, sa_family: 0, sa_data: [0; 14] };
    if !linux_addr.is_null() && is_user_range(linux_addr, 16) != 0 {
        // linux sa_family is u16 at offset 0
        let family = ptr::read_unaligned(linux_addr as *const u16);
        lwip_addr.sa_family = family as u8;
        ptr::copy_nonoverlapping(linux_addr.add(2), lwip_addr.sa_data.as_mut_ptr(), 14);
        
        if lwip_addr.sa_family == 2 /* AF_INET */ {
            let port = (lwip_addr.sa_data[0] as u16) << 8 | (lwip_addr.sa_data[1] as u16);
            let ip = [lwip_addr.sa_data[2], lwip_addr.sa_data[3], lwip_addr.sa_data[4], lwip_addr.sa_data[5]];
            // Musl libc sends DNS requests to 127.0.0.1:53 if /etc/resolv.conf is missing.
            if port == 53 && ip == [127, 0, 0, 1] {
                lwip_addr.sa_data[2] = 8;
                lwip_addr.sa_data[3] = 8;
                lwip_addr.sa_data[4] = 8;
                lwip_addr.sa_data[5] = 8;
            }
        }
    }
    lwip_addr
}

unsafe fn sys_sendto(fd: i64, buf: *const u8, len: usize, flags: u32, dest_addr: *const u8, addrlen: u32) -> i64 {
    if buf.is_null() || is_user_range(buf, len) == 0 { return -EFAULT; }
    if fd >= 512 {
        print_serial(b"  [sendto] fd=\0".as_ptr());
        print_hex_field(b"\0".as_ptr(), fd as u64);
        print_serial(b" len=\0".as_ptr());
        print_hex_field(b"\0".as_ptr(), len as u64);
        if len >= 5 && is_user_range(buf, 5) != 0 {
            let rec_type = *buf;
            if rec_type == 0x16 {
                print_serial(b"  [TLS ClientHello]\n\0".as_ptr());
            }
        }
    }
    let ret = if dest_addr.is_null() {
        lwip_sendto(fd as i32, buf as *const c_void, len, flags as i32, core::ptr::null(), 0) as i64
    } else {
        let lwip_addr = convert_sockaddr(dest_addr);
        lwip_sendto(fd as i32, buf as *const c_void, len, flags as i32, &lwip_addr as *const _ as *const c_void, addrlen) as i64
    };
    if ret < 0 {
        let err = linux_helper_get_errno();
        if err != 0 { -(err as i64) } else { -1 }
    } else {
        ret
    }
}

unsafe fn sys_recvfrom(fd: i64, buf: *mut u8, len: usize, flags: u32, src_addr: *mut u8, addrlen: *mut u32) -> i64 {
    if buf.is_null() || is_user_range(buf, len) == 0 { return -EFAULT; }
    
    let ret = if src_addr.is_null() {
        lwip_recvfrom(fd as i32, buf as *mut c_void, len, flags as i32, core::ptr::null_mut(), core::ptr::null_mut()) as i64
    } else {
        let mut lwip_addr = LwipSockAddr { sa_len: 16, sa_family: 0, sa_data: [0; 14] };
        let mut lwip_addrlen: u32 = 16;
        let r = lwip_recvfrom(fd as i32, buf as *mut c_void, len, flags as i32, &mut lwip_addr as *mut _ as *mut c_void, &mut lwip_addrlen) as i64;
        if r >= 0 {
            if is_user_range(src_addr, 16) != 0 {
                ptr::write_unaligned(src_addr as *mut u16, lwip_addr.sa_family as u16);
                ptr::copy_nonoverlapping(lwip_addr.sa_data.as_ptr(), src_addr.add(2), 14);
                if !addrlen.is_null() && is_user_range(addrlen as *const u8, 4) != 0 {
                    ptr::write(addrlen, 16);
                }
            }
        }
        r
    };
    if ret < 0 {
        let err = linux_helper_get_errno();
        if err != 0 { -(err as i64) } else { -1 }
    } else {
        if fd >= 512 && ret > 0 && is_user_range(buf, 1) != 0 {
            let b0 = *buf;
            if b0 == 0x15 {
                print_serial(b"  [TLS ALERT!]\n\0".as_ptr());
            } else if b0 == 0x16 {
                print_serial(b"  [TLS Handshake Response!]\n\0".as_ptr());
            }
        }
        ret
    }
}

unsafe fn sys_recvmsg(fd: i64, msg_ptr: *mut MsgHdr, flags: u32) -> i64 {
    if msg_ptr.is_null() || is_user_range(msg_ptr as *const u8, core::mem::size_of::<MsgHdr>()) == 0 {
        return -EFAULT;
    }
    let msg = ptr::read_unaligned(msg_ptr);

    if msg.msg_iovlen == 0 || msg.msg_iov.is_null() || msg.msg_iovlen > 1024 {
        return -EINVAL;
    }
    let iov_bytes = msg.msg_iovlen.saturating_mul(core::mem::size_of::<Iovec>());
    if is_user_range(msg.msg_iov as *const u8, iov_bytes) == 0 {
        return -EFAULT;
    }

    let mut lwip_addr = LwipSockAddr { sa_len: 16, sa_family: 0, sa_data: [0; 14] };
    let mut lwip_addrlen: u32 = 16;
    let has_name = !msg.msg_name.is_null() && msg.msg_namelen > 0;

    let total_read: i64;

    if msg.msg_iovlen == 1 {
        let entry = &*msg.msg_iov;
        if entry.iov_len == 0 || entry.iov_base.is_null() {
            return 0;
        }
        if is_user_range(entry.iov_base, entry.iov_len) == 0 {
            return -EFAULT;
        }

        let ret = lwip_recvfrom(
            fd as i32,
            entry.iov_base as *mut c_void,
            entry.iov_len,
            flags as i32,
            if has_name { &mut lwip_addr as *mut _ as *mut c_void } else { core::ptr::null_mut() },
            if has_name { &mut lwip_addrlen as *mut u32 } else { core::ptr::null_mut() },
        ) as i64;

        if ret < 0 {
            let err = linux_helper_get_errno();
            return if err != 0 { -(err as i64) } else { -1 };
        }
        total_read = ret;
    } else {
        // Multi-iovec scatter path using stack buffer
        let mut scratch = [0u8; 2048];
        let mut total_req = 0usize;
        for i in 0..msg.msg_iovlen {
            let entry = &*msg.msg_iov.add(i);
            total_req = total_req.saturating_add(entry.iov_len);
        }
        let cap = core::cmp::min(scratch.len(), total_req);

        let ret = lwip_recvfrom(
            fd as i32,
            scratch.as_mut_ptr() as *mut c_void,
            cap,
            flags as i32,
            if has_name { &mut lwip_addr as *mut _ as *mut c_void } else { core::ptr::null_mut() },
            if has_name { &mut lwip_addrlen as *mut u32 } else { core::ptr::null_mut() },
        ) as i64;

        if ret < 0 {
            let err = linux_helper_get_errno();
            return if err != 0 { -(err as i64) } else { -1 };
        }

        let mut copied = 0usize;
        let bytes_avail = ret as usize;
        for i in 0..msg.msg_iovlen {
            if copied >= bytes_avail { break; }
            let entry = &*msg.msg_iov.add(i);
            if entry.iov_len == 0 || entry.iov_base.is_null() { continue; }
            if is_user_range(entry.iov_base, entry.iov_len) == 0 { return -EFAULT; }
            let to_copy = core::cmp::min(entry.iov_len, bytes_avail - copied);
            ptr::copy_nonoverlapping(scratch.as_ptr().add(copied), entry.iov_base, to_copy);
            copied += to_copy;
        }
        total_read = copied as i64;
    }

    // Write back sender address in Linux sockaddr_in format
    if has_name && is_user_range(msg.msg_name, 16) != 0 {
        ptr::write_unaligned(msg.msg_name as *mut u16, lwip_addr.sa_family as u16);
        ptr::copy_nonoverlapping(lwip_addr.sa_data.as_ptr(), msg.msg_name.add(2), 14);
        ptr::write_unaligned(ptr::addr_of_mut!((*msg_ptr).msg_namelen), 16);
    }
    ptr::write_unaligned(ptr::addr_of_mut!((*msg_ptr).msg_flags), 0);
    ptr::write_unaligned(ptr::addr_of_mut!((*msg_ptr).msg_controllen), 0);

    total_read
}

unsafe fn sys_sendmsg(fd: i64, msg_ptr: *const MsgHdr, flags: u32) -> i64 {
    if msg_ptr.is_null() || is_user_range(msg_ptr as *const u8, core::mem::size_of::<MsgHdr>()) == 0 {
        return -EFAULT;
    }
    let msg = ptr::read_unaligned(msg_ptr);

    if msg.msg_iovlen == 0 || msg.msg_iov.is_null() || msg.msg_iovlen > 1024 {
        return -EINVAL;
    }
    let iov_bytes = msg.msg_iovlen.saturating_mul(core::mem::size_of::<Iovec>());
    if is_user_range(msg.msg_iov as *const u8, iov_bytes) == 0 {
        return -EFAULT;
    }

    let has_dest = !msg.msg_name.is_null() && msg.msg_namelen > 0;
    let lwip_addr = if has_dest {
        convert_sockaddr(msg.msg_name)
    } else {
        LwipSockAddr { sa_len: 0, sa_family: 0, sa_data: [0; 14] }
    };
    let dest_ptr = if has_dest {
        &lwip_addr as *const _ as *const c_void
    } else {
        core::ptr::null()
    };
    let dest_len = if has_dest { msg.msg_namelen } else { 0 };

    if msg.msg_iovlen == 1 {
        let entry = &*msg.msg_iov;
        if entry.iov_len == 0 || entry.iov_base.is_null() {
            return 0;
        }
        if is_user_range(entry.iov_base, entry.iov_len) == 0 {
            return -EFAULT;
        }
        let ret = lwip_sendto(
            fd as i32,
            entry.iov_base as *const c_void,
            entry.iov_len,
            flags as i32,
            dest_ptr,
            dest_len,
        ) as i64;
        if ret < 0 {
            let err = linux_helper_get_errno();
            if err != 0 { -(err as i64) } else { -1 }
        } else {
            ret
        }
    } else {
        let mut scratch = [0u8; 2048];
        let mut gathered = 0usize;
        for i in 0..msg.msg_iovlen {
            let entry = &*msg.msg_iov.add(i);
            if entry.iov_len == 0 || entry.iov_base.is_null() { continue; }
            if is_user_range(entry.iov_base, entry.iov_len) == 0 { return -EFAULT; }
            let to_copy = core::cmp::min(entry.iov_len, scratch.len().saturating_sub(gathered));
            if to_copy > 0 {
                ptr::copy_nonoverlapping(entry.iov_base, scratch.as_mut_ptr().add(gathered), to_copy);
                gathered += to_copy;
            }
        }
        let ret = lwip_sendto(
            fd as i32,
            scratch.as_ptr() as *const c_void,
            gathered,
            flags as i32,
            dest_ptr,
            dest_len,
        ) as i64;
        if ret < 0 {
            let err = linux_helper_get_errno();
            if err != 0 { -(err as i64) } else { -1 }
        } else {
            ret
        }
    }
}

unsafe fn sys_shutdown(fd: i64, how: i32) -> i64 {
    let ret = lwip_shutdown(fd as i32, how) as i64;
    if ret < 0 {
        let err = linux_helper_get_errno();
        if err != 0 { -(err as i64) } else { -1 }
    } else {
        0
    }
}

unsafe fn sys_getsockopt(fd: i64, level: i32, optname: i32, optval: *mut u8, optlen: *mut u32) -> i64 {
    let (lwip_level, lwip_optname) = match (level, optname) {
        (1 /* SOL_SOCKET */, 4 /* SO_ERROR */) => (0xfff, 0x1007),
        _ => (level, optname),
    };
    let ret = lwip_getsockopt(fd as i32, lwip_level, lwip_optname, optval as *mut c_void, optlen) as i64;
    if !optlen.is_null() && is_user_range(optlen as *const u8, 4) != 0 {
        if level == 1 && optname == 4 {
            ptr::write(optlen, 4);
        }
    }
    // Debug: print SO_ERROR value
    if level == 1 && optname == 4 && !optval.is_null() && is_user_range(optval, 4) != 0 {
        let err_val = ptr::read_unaligned(optval as *const i32);
        print_serial(b"  [getsockopt SO_ERROR] val=\0".as_ptr());
        print_hex_field(b"\0".as_ptr(), err_val as u64);
    }
    ret
}

unsafe fn sys_setsockopt(fd: i64, level: i32, optname: i32, optval: *const u8, optlen: u32) -> i64 {
    // Linux and LwIP use different values for SOL_SOCKET, SO_RCVTIMEO, etc.
    // For now, pretend setsockopt succeeds so musl libc doesn't abort.
    let _ = (fd, level, optname, optval, optlen);
    0
}

unsafe fn sys_eventfd(initval: u32) -> i64 {
    linux_helper_eventfd2(initval, 0) as i64
}

unsafe fn sys_eventfd2(initval: u32, flags: i32) -> i64 {
    linux_helper_eventfd2(initval, flags) as i64
}

unsafe fn sys_pipe2(pipefd: *mut i32, flags: i32) -> i64 {
    if pipefd.is_null() || is_user_range(pipefd as *const u8, 8) == 0 { return -EFAULT; }
    linux_helper_pipe2(pipefd, flags) as i64
}

unsafe fn sys_getsockname(fd: i64, addr: *mut u8, addrlen: *mut u32) -> i64 {
    if addr.is_null() || addrlen.is_null() { return -EFAULT; }
    let mut lwip_addr = LwipSockAddr { sa_len: 16, sa_family: 0, sa_data: [0; 14] };
    let mut lwip_len: u32 = 16;
    let ret = lwip_getsockname(fd as i32, &mut lwip_addr as *mut _ as *mut c_void, &mut lwip_len) as i64;
    if ret >= 0 {
        if is_user_range(addr, 16) != 0 {
            ptr::write_unaligned(addr as *mut u16, lwip_addr.sa_family as u16);
            ptr::copy_nonoverlapping(lwip_addr.sa_data.as_ptr(), addr.add(2), 14);
            if is_user_range(addrlen as *const u8, 4) != 0 {
                ptr::write(addrlen, 16);
            }
        }
        // Debug: print returned address
        if fd >= 512 {
            print_serial(b"  [getsockname] fam=\0".as_ptr());
            print_hex_field(b"\0".as_ptr(), lwip_addr.sa_family as u64);
            print_serial(b"  port=\0".as_ptr());
            let port = ((lwip_addr.sa_data[0] as u16) << 8) | (lwip_addr.sa_data[1] as u16);
            print_hex_field(b"\0".as_ptr(), port as u64);
            print_serial(b"  ip=\0".as_ptr());
            print_hex_field(b"\0".as_ptr(),
                ((lwip_addr.sa_data[2] as u64) << 24) |
                ((lwip_addr.sa_data[3] as u64) << 16) |
                ((lwip_addr.sa_data[4] as u64) << 8) |
                (lwip_addr.sa_data[5] as u64));
        }
    }
    ret
}

unsafe fn sys_getpeername(fd: i64, addr: *mut u8, addrlen: *mut u32) -> i64 {
    if addr.is_null() || addrlen.is_null() { return -EFAULT; }
    let mut lwip_addr = LwipSockAddr { sa_len: 16, sa_family: 0, sa_data: [0; 14] };
    let mut lwip_len: u32 = 16;
    let ret = lwip_getpeername(fd as i32, &mut lwip_addr as *mut _ as *mut c_void, &mut lwip_len) as i64;
    if ret >= 0 {
        if is_user_range(addr, 16) != 0 {
            ptr::write_unaligned(addr as *mut u16, lwip_addr.sa_family as u16);
            ptr::copy_nonoverlapping(lwip_addr.sa_data.as_ptr(), addr.add(2), 14);
            if is_user_range(addrlen as *const u8, 4) != 0 {
                ptr::write(addrlen, 16);
            }
        }
    }
    ret
}

unsafe fn sys_arch_prctl(code: i32, addr: u64) -> i64 {
    const ARCH_SET_GS: i32 = 0x1001;
    const ARCH_SET_FS: i32 = 0x1002;
    const ARCH_GET_FS: i32 = 0x1003;
    const ARCH_GET_GS: i32 = 0x1004;

    match code {
        ARCH_SET_FS => {
            linux_helper_set_fs_base(addr);
            0
        }
        ARCH_GET_FS => {
            if is_user_range(addr as *const u8, 8) == 0 {
                return -EFAULT;
            }
            let cur = linux_helper_get_fs_base();
            ptr::write(addr as *mut u64, cur);
            0
        }
        _ => -EINVAL as i64,
    }
}

unsafe fn sys_semget(_key: i32, _nsems: i32, _semflg: i32) -> i64 {
    -ENOSYS
}

unsafe fn sys_semctl(_semid: i32, _semnum: i32, _cmd: i32, _arg: u64) -> i64 {
    -ENOSYS
}

unsafe fn sys_mremap(_old_addr: u64, _old_size: u64, _new_size: u64, _flags: u64) -> u64 {
    MAP_FAILED
}

unsafe fn sys_umask(mask: u32) -> i64 {
    // Stub: return a default umask
    let _ = mask;
    0o022i64
}

unsafe fn sys_chdir2(path: *const u8) -> i64 {
    let _ = path;
    0
}

unsafe fn sys_pread64(fd: u64, buf: *mut u8, count: usize, pos: i64) -> i64 {
    // Save current offset, seek, read, restore
    let saved = vfs_lseek(fd as i32, 0, 1); // SEEK_CUR
    if pos >= 0 {
        vfs_lseek(fd as i32, pos as u64, 0); // SEEK_SET
    }
    let n = sys_read(fd, buf, count);
    vfs_lseek(fd as i32, saved, 0); // SEEK_SET
    n
}

unsafe fn sys_pwrite64(fd: u64, buf: *const u8, count: usize, pos: i64) -> i64 {
    let saved = vfs_lseek(fd as i32, 0, 1);
    if pos >= 0 {
        vfs_lseek(fd as i32, pos as u64, 0);
    }
    let n = sys_write(fd, buf, count);
    vfs_lseek(fd as i32, saved, 0);
    n
}

unsafe fn sys_rename(oldpath: *const u8, newpath: *const u8) -> i64 {
    if oldpath.is_null() || is_user_string(oldpath) == 0 { return -EFAULT; }
    if newpath.is_null() || is_user_string(newpath) == 0 { return -EFAULT; }
    vfs_rename(oldpath, newpath) as i64
}

// ── Helper: validate a null-terminated user string ──────────────────────
unsafe fn is_user_string(s: *const u8) -> i32 {
    if s.is_null() { return 0; }
    let addr = s as u64;
    if addr >= 0xC0000000u64 || addr < 0x1000 { return 0; }
    for i in 0..4096usize {
        let b = *s.offset(i as isize);
        if b == 0 { return 1; }
        if (s.offset(i as isize) as u64) >= 0xC0000000u64 { return 0; }
    }
    0
}

// ── Debug: print a null-terminated label followed by a hex value ─────────
unsafe fn print_hex_field(label: *const u8, val: u64) {
    print_serial(label);
    let mut buf = [0u8; 24];
    let mut idx = 21;
    buf[idx] = 0;
    idx -= 1;
    buf[idx] = b'\n';
    idx -= 1;
    if val == 0 {
        buf[idx] = b'0';
        idx -= 1;
    } else {
        let mut v = val;
        while v > 0 && idx > 0 {
            let d = (v & 0xF) as u8;
            buf[idx] = if d < 10 { b'0' + d } else { b'a' + d - 10 };
            v >>= 4;
            idx -= 1;
        }
    }
    print_serial(buf[idx + 1..].as_ptr());
}

// ── Helper: translate Linux open flags to PureOS open flags ──────────────
fn translate_open_flags(linux_flags: i32) -> i32 {
    let access = linux_flags & 3;
    let mut pureos = access;
    if (linux_flags & O_CREAT) != 0 { pureos |= 0x40; }
    if (linux_flags & O_TRUNC) != 0 { pureos |= 0x200; }
    if (linux_flags & O_APPEND) != 0 { pureos |= 0x400; }
    pureos
}

// ── Main dispatch entry point (called from C syscall_handler) ──────────
#[no_mangle]
pub unsafe extern "C" fn linux_syscall_handler(regs_ptr: *mut Registers) -> u64 {
    let regs = &mut *regs_ptr;
    let nr = regs.rax;

    // TRACE: Print every linux syscall
    print_serial(b"LINUX SYSCALL: \0".as_ptr());
    let mut buf = [0u8; 32];
    let mut nr_val = nr;
    let mut idx = 29;
    buf[30] = b'\n';
    buf[31] = 0;
    if nr_val == 0 { buf[idx] = b'0'; idx -= 1; }
    else {
        while nr_val > 0 && idx > 0 {
            buf[idx] = b'0' + (nr_val % 10) as u8;
            nr_val /= 10;
            idx -= 1;
        }
    }
    print_serial(buf[idx + 1..].as_ptr());

    // DEBUG: dump user state at syscall entry
    print_hex_field(b"    rip=0x\0".as_ptr(), regs.rip);
    print_hex_field(b"    rsp=0x\0".as_ptr(), regs.rsp);
    print_hex_field(b"    rflags=0x\0".as_ptr(), regs.rflags);
    print_hex_field(b"    rax=0x\0".as_ptr(), regs.rax);
    print_hex_field(b"    rdi=0x\0".as_ptr(), regs.rdi);
    print_hex_field(b"    rsi=0x\0".as_ptr(), regs.rsi);

    // Linux x86_64 syscall ABI:
    //   arg1 = rdi,  arg2 = rsi,  arg3 = rdx
    //   arg4 = r10,  arg5 = r8,   arg6 = r9
    let result = match nr {
        SYS_READ        => sys_read(regs.rdi, regs.rsi as *mut u8, regs.rdx as usize),
        SYS_WRITE       => sys_write(regs.rdi, regs.rsi as *const u8, regs.rdx as usize),
        SYS_OPEN        => sys_open(regs.rdi as *const u8, regs.rsi as i32, regs.rdx as u32),
        SYS_CLOSE       => sys_close(regs.rdi),
        SYS_STAT        => sys_stat(regs.rdi as *const u8, regs.rsi as *mut u8),
        SYS_FSTAT       => sys_fstat(regs.rdi, regs.rsi as *mut u8),
        SYS_LSTAT       => sys_stat(regs.rdi as *const u8, regs.rsi as *mut u8), // lstat ≈ stat for now
        SYS_POLL        => sys_poll(regs.rdi as *mut u8, regs.rsi, regs.rdx as i32),
        SYS_LSEEK       => sys_lseek(regs.rdi, regs.rsi as i64, regs.rdx as i32),
        SYS_MMAP        => sys_mmap(regs.rdi, regs.rsi, regs.rdx as i32, regs.r10 as i32, regs.r8 as i64, regs.r9 as i64) as i64,
        SYS_MPROTECT    => sys_mprotect(regs.rdi, regs.rsi, regs.rdx),
        SYS_MUNMAP      => sys_munmap(regs.rdi, regs.rsi),
        SYS_BRK         => sys_brk(regs.rdi),
        SYS_RT_SIGACTION    |
        SYS_RT_SIGPROCMASK  |
        SYS_RT_SIGRETURN    => 0, // ignore signals
        SYS_IOCTL       => sys_ioctl(regs.rdi, regs.rsi, regs.rdx),
        SYS_PREAD64     => sys_pread64(regs.rdi, regs.rsi as *mut u8, regs.rdx as usize, regs.r10 as i64),
        SYS_PWRITE64    => sys_pwrite64(regs.rdi, regs.rsi as *const u8, regs.rdx as usize, regs.r10 as i64),
        SYS_READV       => sys_readv(regs.rdi, regs.rsi as *const Iovec, regs.rdx as i32),
        SYS_WRITEV      => sys_writev(regs.rdi, regs.rsi as *const Iovec, regs.rdx as i32),
        SYS_ACCESS      => sys_access(regs.rdi as *const u8, regs.rsi as u32),
        SYS_PIPE        => sys_pipe(regs.rdi as *mut i32),
        SYS_SELECT      => sys_select(regs.rdi as i32, regs.rsi as *mut u8, regs.rdx as *mut u8, regs.r10 as *mut u8, regs.r8 as *const u8),
        SYS_SCHED_YIELD => sys_sched_yield(),
        SYS_MREMAP      => sys_mremap(regs.rdi, regs.rsi, regs.rdx, regs.r10) as i64,
        41 /* SYS_SOCKET */ => {
            let domain = regs.rdi as i32;
            let sock_type = (regs.rsi as i32) & 0xF;
            if domain == 10 /* AF_INET6 */ {
                -97 /* -EAFNOSUPPORT in Linux x86_64 */
            } else {
                let fd = lwip_socket(domain, sock_type, regs.rdx as i32) as i64;
                if fd >= 0 && ((regs.rsi as i32) & 0x800 != 0) {
                    lwip_fcntl(fd as i32, 4, 1);
                }
                fd
            }
        },
        42 /* SYS_CONNECT */=> {
            let lwip_addr = convert_sockaddr(regs.rsi as *const u8);
            // Debug: print connect destination
            if lwip_addr.sa_family == 2 {
                print_serial(b"  [connect] to \0".as_ptr());
                let mut ip_buf = [0u8; 32];
                let mut idx = 0usize;
                for octet_i in 0..4usize {
                    let mut val = lwip_addr.sa_data[2 + octet_i];
                    if val >= 100 { ip_buf[idx] = b'0' + val / 100; idx += 1; val %= 100; ip_buf[idx] = b'0' + val / 10; idx += 1; val %= 10; }
                    else if val >= 10 { ip_buf[idx] = b'0' + val / 10; idx += 1; val %= 10; }
                    ip_buf[idx] = b'0' + val; idx += 1;
                    if octet_i < 3 { ip_buf[idx] = b'.'; idx += 1; }
                }
                ip_buf[idx] = b':'; idx += 1;
                let port = ((lwip_addr.sa_data[0] as u16) << 8) | (lwip_addr.sa_data[1] as u16);
                let mut pv = port;
                if pv >= 10000 { ip_buf[idx] = b'0' + (pv / 10000) as u8; idx += 1; pv %= 10000; }
                if pv >= 1000 || port >= 10000 { ip_buf[idx] = b'0' + (pv / 1000) as u8; idx += 1; pv %= 1000; }
                if pv >= 100 || port >= 1000 { ip_buf[idx] = b'0' + (pv / 100) as u8; idx += 1; pv %= 100; }
                if pv >= 10 || port >= 100 { ip_buf[idx] = b'0' + (pv / 10) as u8; idx += 1; pv %= 10; }
                ip_buf[idx] = b'0' + pv as u8; idx += 1;
                ip_buf[idx] = b'\n'; idx += 1;
                ip_buf[idx] = 0;
                print_serial(ip_buf.as_ptr());
            }
            let ret = lwip_connect(regs.rdi as i32, &lwip_addr as *const _ as *const c_void, regs.rdx as u32);
            if ret < 0 {
                let err = linux_helper_get_errno();
                print_serial(b"  [connect] FAILED err=\0".as_ptr());
                print_hex_field(b"\0".as_ptr(), err as u64);
                if err != 0 {
                    -(err as i64)
                } else {
                    -115 // -EINPROGRESS
                }
            } else {
                print_serial(b"  [connect] OK\n\0".as_ptr());
                0
            }
        },
        49 /* SYS_BIND */   => {
            let lwip_addr = convert_sockaddr(regs.rsi as *const u8);
            lwip_bind(regs.rdi as i32, &lwip_addr as *const _ as *const c_void, regs.rdx as u32) as i64
        },
        SYS_DUP         => sys_dup(regs.rdi),
        SYS_DUP2        => sys_dup2(regs.rdi, regs.rsi),
        SYS_FUTEX       => 0, // stub futex for musl init
        SYS_SET_ROBUST_LIST => 0, // stub for musl init
        SYS_OPENAT      => sys_openat(regs.rdi as i32, regs.rsi as *const u8, regs.rdx as i32, regs.r10 as u32),
        SYS_NEWFSTATAT  => sys_newfstatat(regs.rdi as i32, regs.rsi as *const u8, regs.rdx as *mut u8, regs.r10 as i32),
        SYS_FCNTL       => sys_fcntl(regs.rdi as i32, regs.rsi as i32, regs.rdx),
        SYS_GETUID      => sys_getuid(),
        SYS_GETGID      => sys_getgid(),
        SYS_GETEUID     => sys_getuid(), // euid = uid for now
        SYS_GETEGID     => sys_getgid(), // egid = gid for now
        SYS_NANOSLEEP   => sys_nanosleep(regs.rdi as *const Timespec, regs.rsi as *mut Timespec),
        SYS_GETPID      => sys_getpid(),
        SYS_SENDTO      => sys_sendto(regs.rdi as i64, regs.rsi as *const u8, regs.rdx as usize, regs.r10 as u32, regs.r8 as *const u8, regs.r9 as u32),
        SYS_RECVFROM    => sys_recvfrom(regs.rdi as i64, regs.rsi as *mut u8, regs.rdx as usize, regs.r10 as u32, regs.r8 as *mut u8, regs.r9 as *mut u32),
        SYS_SENDMSG     => sys_sendmsg(regs.rdi as i64, regs.rsi as *const MsgHdr, regs.rdx as u32),
        SYS_RECVMSG     => sys_recvmsg(regs.rdi as i64, regs.rsi as *mut MsgHdr, regs.rdx as u32),
        SYS_SHUTDOWN    => sys_shutdown(regs.rdi as i64, regs.rsi as i32),
        SYS_SOCKETPAIR  => sys_socketpair(regs.rdi as i32, regs.rsi as i32, regs.rdx as i32, regs.r10 as *mut i32),
        SYS_SETSOCKOPT  => sys_setsockopt(regs.rdi as i64, regs.rsi as i32, regs.rdx as i32, regs.r10 as *const u8, regs.r8 as u32),
        SYS_GETSOCKOPT  => sys_getsockopt(regs.rdi as i64, regs.rsi as i32, regs.rdx as i32, regs.r10 as *mut u8, regs.r8 as *mut u32),
        SYS_GETSOCKNAME => sys_getsockname(regs.rdi as i64, regs.rsi as *mut u8, regs.rdx as *mut u32),
        SYS_GETPEERNAME => sys_getpeername(regs.rdi as i64, regs.rsi as *mut u8, regs.rdx as *mut u32),
        SYS_EVENTFD     => sys_eventfd(regs.rdi as u32),
        SYS_EVENTFD2    => sys_eventfd2(regs.rdi as u32, regs.rsi as i32),
        SYS_PIPE2       => sys_pipe2(regs.rdi as *mut i32, regs.rsi as i32),
        SYS_CLONE       => sys_clone(regs_ptr as *const c_void, regs.rdi, regs.rsi, regs.rdx, regs.r10, regs.r8),
        SYS_FORK        => sys_fork(),
        SYS_EXECVE      => sys_execve(regs.rdi as *const u8, regs.rsi as *const u8, regs.rdx as *const u8),
        SYS_EXIT        => sys_exit(regs.rdi),
        SYS_WAIT4       => sys_wait4(regs.rdi as i64, regs.rsi as *mut i32, regs.rdx as i32, regs.r10 as *mut u8),
        SYS_KILL        => sys_kill(regs.rdi as i64, regs.rsi as i32),
        SYS_UNAME       => sys_uname(regs.rdi as *mut u8),
        SYS_SEMGET      => sys_semget(regs.rdi as i32, regs.rsi as i32, regs.rdx as i32),
        SYS_SEMCTL      => sys_semctl(regs.rdi as i32, regs.rsi as i32, regs.rdx as i32, regs.r10),
        SYS_GETCWD      => sys_getcwd(regs.rdi as *mut u8, regs.rsi),
        SYS_CHDIR       => sys_chdir(regs.rdi as *const u8),
        SYS_MKDIR       => sys_mkdir(regs.rdi as *const u8, regs.rsi as u32),
        SYS_RMDIR       => sys_unlink(regs.rdi as *const u8), // rmdir ≈ unlink for now
        SYS_LINK        => { let _ = regs.rdi; -ENOSYS } // link not supported
        SYS_UNLINK      => sys_unlink(regs.rdi as *const u8),
        SYS_SYMLINK     => sys_symlink(regs.rdi as *const u8, regs.rsi as *const u8),
        SYS_READLINK    => sys_readlink(regs.rdi as *const u8, regs.rsi as *mut u8, regs.rdx),
        SYS_CHMOD       => sys_chmod(regs.rdi as *const u8, regs.rsi as u32),
        SYS_FCHMOD      => { let _ = regs.rdi; 0 } // stub
        SYS_UMASK       => sys_umask(regs.rdi as u32),
        SYS_GETTID      => linux_helper_get_tid() as i64,
        SYS_GETDENTS64  => sys_getdents64(regs.rdi, regs.rsi as *mut u8, regs.rdx),
        SYS_SET_TID_ADDRESS => sys_set_tid_address(regs.rdi as *mut i32),
        SYS_CLOCK_GETTIME   => sys_clock_gettime(regs.rdi, regs.rsi as *mut Timespec),
        SYS_EXIT_GROUP      => sys_exit_group(regs.rdi),
        SYS_TKILL           => sys_tkill(regs.rdi as i64, regs.rsi as i32),
        SYS_FACCESSAT       => sys_faccessat(regs.rdi, regs.rsi as *const u8, regs.rdx as u32, regs.r10 as u32),
        SYS_GETRANDOM       => sys_getrandom(regs.rdi as *mut u8, regs.rsi, regs.rdx as u32),
        SYS_RSEQ            => sys_rseq(),
        158 /* SYS_ARCH_PRCTL */ => sys_arch_prctl(regs.rdi as i32, regs.rsi),
        _ => {
            print_serial(b"Unknown Linux Syscall: \0".as_ptr());
            let mut buf = [0u8; 32];
            // Poor man's integer to string:
            let mut nr_val = nr;
            let mut idx = 29;
            buf[30] = b'\n';
            buf[31] = 0; // NULL terminator for print_serial
            if nr_val == 0 {
                buf[idx] = b'0';
                idx -= 1;
            } else {
                while nr_val > 0 && idx > 0 {
                    buf[idx] = b'0' + (nr_val % 10) as u8;
                    nr_val /= 10;
                    idx -= 1;
                }
            }
            print_serial(buf[idx + 1..].as_ptr());
            -ENOSYS
        }
    };

    // DEBUG: confirm the handler returned (return path will now run)
    print_serial(b"SYSCALL DONE: \0".as_ptr());
    let mut dbuf = [0u8; 32];
    let mut dv = nr;
    let mut didx = 29;
    dbuf[30] = b'\n';
    dbuf[31] = 0;
    if dv == 0 { dbuf[didx] = b'0'; didx -= 1; }
    else {
        while dv > 0 && didx > 0 {
            dbuf[didx] = b'0' + (dv % 10) as u8;
            dv /= 10;
            didx -= 1;
        }
    }
    print_serial(dbuf[didx + 1..].as_ptr());
    print_hex_field(b"    result=0x\0".as_ptr(), result as u64);

    regs.rax = result as u64;
    regs_ptr as u64
}
