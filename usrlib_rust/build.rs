use std::io::{Result, Write};
use std::fs::{OpenOptions};
use csv::Reader;

fn update_syscall_number() -> Result<()> {
    let fi = OpenOptions::new()
        .read(true)
        .open("../../syscall_num.csv")?;
    let mut fo = OpenOptions::new()
        .write(true)
        .truncate(true)
        .create(true)
        .open("src/utils/syscall_num.rs")?;
    let mut rdr = Reader::from_reader(fi);
    for result in rdr.records() {
        let record = result?;
        println!("{} => {}", record.get(0).unwrap(), record.get(1).unwrap());
        writeln!(fo, "pub const SYSCALL_{:<10}: usize = {:>3};", record.get(0).unwrap().to_ascii_uppercase(), record.get(1).unwrap())?;
    }
    Ok(())
}

fn main() {
    println!("cargo:rerun-if-changed=./src/");
    println!("cargo:rerun-if-changed=../../syscall_num.csv");
    update_syscall_number().unwrap();
}