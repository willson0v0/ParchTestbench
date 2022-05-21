#![no_std]
#![no_main]
#![feature(panic_info_message)]

use usrlib_rust::{syscall::{exit, getcwd, mkdir}, utils::types::Permission};

#[macro_use]
extern crate usrlib_rust;
extern crate alloc;

#[panic_handler]
fn panic_handler(panic_info: &core::panic::PanicInfo) -> ! {
    let err = panic_info.message().unwrap();
    if let Some(location) = panic_info.location() {
        println!("Mkdir panicked at {}:{}, {}", location.file(), location.line(), err);
    } else {
        println!("Mkdir panicked: {}", err);
    }
    exit(-1);
}

#[no_mangle]
fn main(argc: usize, argv: &[&[u8]]) -> isize {
    if argc != 2 {
        panic!("Mkdir requires exactly 1 arguments.")
    }
    let path = core::str::from_utf8(argv[1]).unwrap();
    mkdir(path, Permission::default()).unwrap();
    0
}