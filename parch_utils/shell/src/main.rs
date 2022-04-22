#![no_std]
#![no_main]
#![feature(panic_info_message)]

use core::ops::{Deref, DerefMut};
use core::fmt::Debug;
use core::str;
use alloc::{string::{String, ToString}, boxed::Box, vec::Vec, collections::VecDeque};
use usrlib_rust::fmt_io::get_line;
use usrlib_rust::syscall::{exit, getcwd, read};
use usrlib_rust::{println, fmt_io::{getchar, getchar_noecho, STDIN, STDOUT}, utils::{error_num::ErrorNum, types::{FileDecstiptor, OpenMode}}, syscall::{chdir, fork, waitpid, close, open, exec}};

#[macro_use]
extern crate usrlib_rust;
extern crate alloc;
extern crate time;

#[panic_handler]
fn panic_handler(panic_info: &core::panic::PanicInfo) -> ! {
    let err = panic_info.message().unwrap();
    if let Some(location) = panic_info.location() {
        println!("Shell panicked at {}:{}, {}", location.file(), location.line(), err);
    } else {
        println!("Shell panicked: {}", err);
    }
    exit(-1);
}

fn serve_cd(cmd: String) -> Result<(), ErrorNum> {
    chdir(cmd.strip_prefix("cd ").unwrap().trim())?;
    Ok(())
}

fn serve_exit(_cmd: String) -> ! {
    println!("Bye.");
    exit(0);
}

fn serve_help(_cmd: String) -> Result<(), ErrorNum> {
    println!("Built-in commands:");
    println!("\tcd [dir]\t: change current working directory.");
    println!("\thelp\t: display help message.");
    println!("\texit\t: exit shell.");
    println!("\nPlease enter built-in commands, or path to elf file to execute.");
    Ok(())
}

fn serve_exec(cmd: String) -> Result<(), ErrorNum> {
    let cmd = Command::parse(cmd).map_err(|_| ErrorNum::ENOEXEC)?;
    let ret = cmd.execute()?;
    println!("Program exited with code {}.", ret);
    Ok(())
}

#[derive(PartialEq, Eq)]
enum IOType {
    Default,
    File(String),
    Cmd(Command)
}

impl Debug for IOType {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            Self::Default => write!(f, "Default"),
            Self::File(arg0) => f.debug_tuple("File").field(arg0).finish(),
            Self::Cmd(arg0) => f.debug_tuple("Cmd").field(&arg0.exec_path).finish(),
        }
    }
}

struct CommandInner {
    pub exec_path: String,
    pub args: Vec<String>,
    pub input: IOType,
    pub output: IOType
}

struct Command(Box<CommandInner>);

impl Deref for Command {
    type Target = CommandInner;

    fn deref(&self) -> &Self::Target {
        self.0.deref()
    }
}

impl DerefMut for Command {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.0.deref_mut()
    }
}

impl PartialEq for Command {
    fn eq(&self, other: &Self) -> bool {
        self as *const Command == other as *const Command
    }
}

impl Eq for Command {}

struct ParseErr;

impl Debug for Command {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        writeln!(f, "Command {}", self.exec_path)?;
        writeln!(f, "arguments:")?;
        for a in self.args.iter() {
            writeln!(f, "\t\"{}\"", a)?;
        }
        writeln!(f, "Input : {:?}", self.input)?;
        writeln!(f, "Output: {:?}", self.output)
    }
}

impl Command {
    // cmd [args] [ | nxt_cmd or > dst_file] [ < src_file ]
    pub fn parse(cmd: String) -> Result<Self, ParseErr> {
        // first priority: |
        if let Some(split_pos) = cmd.find('|') {
            let (l, r) = cmd.split_at(split_pos);
            let mut l = Self::parse(l.trim().to_string())?;
            let r = Self::parse(r.trim().to_string())?;

            if l.output != IOType::Default || r.input != IOType::Default{
                return Err(ParseErr)
            }
            l.output = IOType::Cmd(r);
            // r's default will be modified by l
            Ok(l)
        } else {
            let input_pos = cmd.find('<');
            let output_pos = cmd.find('>');

            let (exec, input, output) = if input_pos.is_some() && output_pos.is_some() {
                let input_pos = input_pos.unwrap();
                let output_pos = output_pos.unwrap();

                if input_pos < output_pos {
                    let (e, i_o) = cmd.split_at(input_pos);
                    let i_o = i_o.strip_prefix('<').unwrap();
                    let (i, o) = i_o.split_at(output_pos - input_pos);
                    let o = o.strip_prefix('>').unwrap();
                    (e.trim().to_string(), Some(i.trim().to_string()), Some(o.trim().to_string()))
                } else {
                    let (e, o_i) = cmd.split_at(output_pos);
                    let o_i = o_i.strip_prefix('>').unwrap();
                    let (o, i) = o_i.split_at(input_pos - output_pos);
                    let i = i.strip_prefix('<').unwrap();
                    (e.trim().to_string(), Some(i.trim().to_string()), Some(o.trim().to_string()))
                }
            } else if let Some(input_pos) = input_pos {
                let (e, i) = cmd.split_at(input_pos);
                let i = i.strip_prefix('<').unwrap();
                (e.trim().to_string(), Some(i.trim().to_string()), None)
            } else if let Some(output_pos) = output_pos {
                let (e, o) = cmd.split_at(output_pos);
                let o = o.strip_prefix('>').unwrap();
                (e.trim().to_string(), None, Some(o.trim().to_string()))
            } else {
                (cmd.trim().to_string(), None, None)
            };

            // parse exec
            let mut exec_and_args: VecDeque<String>= exec.split(' ').filter_map(
                |s| -> Option<String> {
                    if s.is_empty() {
                        None
                    } else {
                        Some(s.to_string())
                    }
                }
            ).collect();

            let exec_path = exec_and_args.pop_front().ok_or(ParseErr)?;

            let input = if let Some(path) = input {
                IOType::File(path)
            } else {
                IOType::Default
            };

            let output = if let Some(path) = output {
                IOType::File(path)
            } else {
                IOType::Default
            };

            Ok(Self(Box::new(CommandInner{
                exec_path,
                args: exec_and_args.into(),
                input,
                output,
            })))
        }
    }

    pub fn execute(&self) -> Result<isize, ErrorNum>{
        let fork_res = fork()?;
        if fork_res == 0 {
            if let IOType::File(path) = &self.input {
                // close stdin, open file as read
                close(STDIN)?;
                let fd = open(path.as_str(), OpenMode::READ)?;
                if fd != STDIN {
                    panic!("Failed to re-open fd 0 to redirect input, re-opened fd is {:?}.", fd);
                }
            } else if let IOType::Cmd(command) = &self.input {
                // TODO: finish this after pipe is done
                todo!()
            }

            if let IOType::File(path) = &self.output {
                // close stdin, open file as read
                close(STDOUT)?;
                let fd = open(path.as_str(), OpenMode::WRITE | OpenMode::CREATE)?;
                if fd != STDOUT {
                    panic!("Failed to re-open fd 1 to redirect input, re-opened fd is {:?}.", fd);
                }
            } else if let IOType::Cmd(command) = &self.input {
                // TODO: finish this after pipe is done
                todo!()
            }

            exec(self.exec_path.as_str(), self.args.clone())?;
        } else {
            let mut ret_code: isize = 0;
            waitpid(fork_res as isize, &mut ret_code)?;
            return Ok(ret_code);
        }

        Err(ErrorNum::ENOSYS)
    }
}

fn serve_command(cmd: String) -> Result<(), ErrorNum> {
    if cmd.starts_with("cd ") {
        serve_cd(cmd)
    } else if cmd.starts_with("help") {
        serve_help(cmd)
    } else if cmd.starts_with("exit") {
        serve_exit(cmd)
    } else {
        serve_exec(cmd)
    }
}

fn print_header() {
    let fd = open("/dev/rtc0", OpenMode::READ).unwrap();
    let mut buf: [u8; core::mem::size_of::<usize>()] = [0u8; core::mem::size_of::<usize>()];
    assert!(read(fd, &mut buf).unwrap() == core::mem::size_of::<usize>());
    let epoch = usize::from_le_bytes(buf);
    let epoch = time::OffsetDateTime::from_unix_timestamp_nanos(epoch as i128).to_offset(time::UtcOffset::east_hours(8));
    close(fd).unwrap();
    print!("[ {} ] {} $ ", getcwd().unwrap(), epoch.format("%F %r %z"));
}

#[no_mangle]
fn main(argc: usize, argv: &[&[u8]]) -> isize {
    println!("[shell] hello world!");
    // println!("argc: {}", argc);
    // for i in 0..argc {
    //     println!("argv [{}]: {:?}", i, core::str::from_utf8(argv[i]));
    // }
    // 
    // println!("\n\n\n\n\n\n");
    // loop {
    //     println!("{:?}", getchar_noecho());
    // }

    if argc > 1 {
        // remap STDIN
        close(STDIN).unwrap();
        let fd = open(str::from_utf8(argv[1]).unwrap(), OpenMode::READ).unwrap();
        if fd != STDIN {
            panic!("failed to redirect stdin. new fd = {:?}", fd);
        }
    }
    loop {
        print_header();
        if let Ok(line) = get_line() {
            if let Err(e) = serve_command(line) {
                println!("Command failed with {:?}.", e);
            }
        } else {
            break;
        }
    }
    println!("Cannot read further input. Exiting.");
    0
}