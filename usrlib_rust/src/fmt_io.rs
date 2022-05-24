use core::fmt::{self, Write};
use alloc::string::String;

use crate::{syscall::write, utils::{types::FileDecstiptor, error_num::ErrorNum}};

struct Stdout;

pub const STDIN : FileDecstiptor = FileDecstiptor(0);
pub const STDOUT: FileDecstiptor = FileDecstiptor(1);

impl Write for Stdout {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        write(STDOUT, s.as_bytes()).map_err(|_| fmt::Error)?;
        Ok(())
    }
}

pub fn print(args: fmt::Arguments) {
    Stdout.write_fmt(args).unwrap();
}

#[macro_export]
macro_rules! print {
    ($fmt: literal $(, $($arg: tt)+)?) => {
        $crate::fmt_io::print(format_args!($fmt $(, $($arg)+)?));
    }
}

#[macro_export]
macro_rules! println {
    () =>  {
        print!("\r\n");
    };
    ($fmt: literal $(, $($arg: tt)+)?) => {
        $crate::fmt_io::print(format_args!(concat!($fmt, "\r\n") $(, $($arg)+)?));
    }
}

pub fn getbyte_noecho() -> Result<u8, ErrorNum> {
    let mut buf = [0u8];
    let count = crate::syscall::read(STDIN, &mut buf)?;
    if count == 0 {
        Err(ErrorNum::EEOF)
    } else {
        Ok(buf[0])
    }
}

pub fn getchar_noecho() -> Result<char, ErrorNum> {
    let init : u8 = getbyte_noecho()?;
    let mut buf : u32;

    let length : u8;
    if init < 0b10000000 {
        return Ok(init as char);
    }
    else if init < 0b11100000 {length = 2;}
    else if init < 0b11110000 {length = 3;}
    else if init < 0b11111000 {length = 4;}
    else if init < 0b11111100 {length = 5;}
    else if init < 0b11111110 {length = 6;}
    else { return Ok('�'); }     // illegal utf-8 sequence
    buf = (init & (0b01111111 >> length)) as u32;

    for _i in 1..length {
        let b : u8 = getbyte_noecho()?;

        if b & 0b11000000 != 0b10000000 { return Ok('�'); }
        assert_eq!(b & 0b11000000, 0b10000000); // check utf-8 sequence
        buf <<= 6;
        buf += (b & 0b00111111) as u32;
    }
    
    match char::from_u32(buf) {
        None => Ok('�'),    // unknown sequence
        Some(res) => Ok(res)
    }
}

pub fn getchar() -> Result<char, ErrorNum> {
    let res = getchar_noecho()?;
    if res == '\r' {print!("\n");}  // qemu uart is strange.
    print!("{}", res);
    Ok(res)
}

pub fn get_line() -> Result<String, ErrorNum> {
    let mut res = String::new();
    loop {
        let b = getchar()?;
        if b == '\r' || b == '\n' {
            break;
        }
        // del
        if b == '\x7f' {
            if let Some(_) = res.pop() {
                print!("\x08 \x08");
            }
            continue;
        }
        res.push(b);
    }
    Ok(res)
}