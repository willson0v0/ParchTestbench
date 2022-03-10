#![no_std]
#![no_main]
#![feature(panic_info_message)]

use alloc::{string::{String, self, ToString}, format, vec::Vec};
use usrlib_rust::{syscall::{open, read}, utils::{types::OpenMode, error_num::ErrorNum}};

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

fn rust_main(argc: usize, argv: &[&[u8]]) -> Result<(), String> {
    if argc != 2 {
        return Err(format!("Cat only accept 1 argument, but {} was received.", argc - 1));
    }
    let path_bytes = argv[1];
    let path = String::from_utf8(path_bytes.to_vec()).map_err(|_| "Illegal utf-8 byte sequence".to_string())?;
    let fd = open(&path, OpenMode::READ).map_err(|code| format!("Failed to open file, syscall return {:?}", code))?;

    let mut res: Vec<u8> = Vec::new();
    loop {
        let mut buffer = [0u8; 128];
        let length = read(fd, &mut buffer).map_err(|code| format!("Read file failed with code {:?}", code))?;
        if length == 0 {
            break;
        }
        res.extend(buffer[0..length].iter());
    }

    let file_content = String::from_utf8_lossy(&res);
    println!("{}\n========== EOF ==========", file_content);

    Ok(())
}

#[no_mangle]
fn main(argc: usize, argv: &[&[u8]]) -> isize {
    let res = rust_main(argc, argv);
    if let Err(msg) = res {
        println!("Cat failed, {}", msg);
        -1
    } else {
        0
    }
}