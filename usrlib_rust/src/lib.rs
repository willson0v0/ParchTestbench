#![no_std]
#![feature(linkage)]
#![feature(panic_info_message)]
#![feature(alloc_error_handler)]

// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         let result = 2 + 2;
//         assert_eq!(result, 4);
//     }
// }

pub mod utils;
pub mod syscall;
#[macro_use]
pub mod fmt_io;
pub mod allocator;

extern crate alloc;

unsafe fn strlen(s: *const u8) -> usize {
    let mut p = s;
    while p.read_volatile() != 0 {
        p = p.add(core::mem::size_of::<u8>());
    }
    p as usize - s as usize
}

#[linkage = "weak"]
#[no_mangle]
fn main(_argc: usize, _argv: &[&[u8]]) -> isize{
    panic!("main() not found in user program.");
}

#[no_mangle]
#[link_section = ".text.entry"]
pub extern "C" fn _start(argc: usize, argv_raw: usize) -> ! {
    let argv_ptr = unsafe{core::slice::from_raw_parts(argv_raw as *const *const u8, argc)};
    let mut argv_buf: [&[u8]; 16] = [&[0]; 16];
    for i in 0..argc {
        argv_buf[i] = unsafe {core::slice::from_raw_parts(argv_ptr[i], strlen(argv_ptr[i]))}
    }
    allocator::init_heap();
    let ret = main(argc, &argv_buf);
    syscall::exit(ret);
}