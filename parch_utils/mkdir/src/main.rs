#![no_std]
#![no_main]
#![feature(panic_info_message)]

#[macro_use]
extern crate usrlib_rust;
extern crate alloc;

#[panic_handler]
fn panic_handler(panic_info: &core::panic::PanicInfo) -> ! {
    let err = panic_info.message().unwrap();
    if let Some(location) = panic_info.location() {
        println!("Shell panicked at {}:{}, {}", location.file(), location.line(), err);
    } else {
        println!("Shell panicked: {}", err);
    }
    loop {}
}

#[no_mangle]
fn main(argc: usize, argv: &[&[u8]]) -> isize {
    println!("Hello world.");
    0
}