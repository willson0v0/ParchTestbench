use core::arch::asm;

use alloc::string::String;
use alloc::vec::Vec;

use crate::utils::syscall_num::*;
use crate::utils::error_num::ErrorNum;
use crate::utils::types::*;

pub fn do_syscall(args: [usize; 6], syscall_id: usize) -> Result<usize, ErrorNum> {
    let mut ret: usize = args[0];
    let mut success: usize = args[1];
    unsafe {
        asm!(
            "ecall",
            inout("a0") ret,
            inout("a1") success,
            in("a2") args[2],
            in("a3") args[3],
            in("a4") args[4],
            in("a5") args[5],
            in("a7") syscall_id
        )
    }
    if success == 0 {
        Ok(ret as usize)
    } else {
        Err(ErrorNum::try_from(-(ret as isize) as usize).unwrap())
    }
}

pub fn write_raw(fd: FileDecstiptor, buf: *const u8, length: usize) -> Result<usize, ErrorNum> {
    do_syscall([
        fd.0,
        buf as usize,
        length,
        0,0,0
    ], SYSCALL_WRITE)
}

pub fn write(fd: FileDecstiptor, buf: &[u8]) -> Result<usize, ErrorNum> {
    write_raw(fd, buf.as_ptr(), buf.len())
}

pub fn read_raw(fd: FileDecstiptor, buf: *mut u8, length: usize) -> Result<usize, ErrorNum> {
    do_syscall([
        fd.0,
        buf as usize,
        length,
        0,0,0
    ], SYSCALL_READ)

}

pub fn read(fd: FileDecstiptor, buf: &mut [u8]) -> Result<usize, ErrorNum> {
    read_raw(fd, buf.as_mut_ptr(), buf.len())
}

pub fn read_vec(fd: FileDecstiptor, length: usize) -> Result<Vec<u8>, ErrorNum> {
    let mut buf = [0u8].repeat(length);
    read(fd, buf.as_mut_slice())?;
    Ok(buf)
}

pub fn open(path: &str, open_mode: OpenMode) -> Result<FileDecstiptor, ErrorNum> {
    let mut arr = path.as_bytes().to_vec();
    arr.push(0);
    // let p = path.as_ptr();
    do_syscall([
        arr.as_ptr() as usize,
        open_mode.bits()
        ,0,0,0,0
    ], SYSCALL_OPEN).map(|f| f.into())
}

pub fn openat(fd: FileDecstiptor, path: &str, open_mode: OpenMode) -> Result<usize, ErrorNum> {
    let mut arr = path.as_bytes().to_vec();
    arr.push(0);
    do_syscall([
        fd.0,
        arr.as_ptr() as usize,
        path.len(),
        open_mode.bits(),
        0,0
    ], SYSCALL_OPENAT)
}

pub fn close(fd: FileDecstiptor) -> Result<usize, ErrorNum> {
    do_syscall([
        fd.0,
        0,0,0,0,0
    ], SYSCALL_CLOSE)
}

pub fn dup(fd: FileDecstiptor) -> Result<FileDecstiptor, ErrorNum> {
    do_syscall([
        fd.0,
        0,0,0,0,0
    ], SYSCALL_DUP).map(|f| f.into())
}

pub fn fork() -> Result<usize, ErrorNum> {
    do_syscall([
        0,0,0,0,0,0
    ], SYSCALL_FORK)
}

pub fn exec_raw(path: &str, argv_ptr: &[*const u8]) -> Result<usize, ErrorNum> {
    let mut path_arr = path.as_bytes().to_vec();
    path_arr.push(0);
    let path_ptr = path_arr.as_ptr();

    let argv_arr_ptr = argv_ptr.as_ptr();
    do_syscall([
        path_ptr as usize,
        argv_arr_ptr as usize,
        0,0,0,0
    ], SYSCALL_EXEC)
}

pub fn exec(path: &str, argv: Vec<String>) -> Result<usize, ErrorNum> {
    let argv_u8: Vec<Vec<u8>> = argv.into_iter().map(
        |s| -> Vec<u8> {
            let mut arr = s.as_bytes().to_vec();
            arr.push(0);
            arr
        }
    ).collect();
    let mut argv_ptr: Vec<*const u8> = argv_u8.iter().map(
        |a| a.as_ptr()
    ).collect();
    argv_ptr.push(0 as *const u8);
    exec_raw(path, &argv_ptr)
}

pub fn exit(code: isize) -> ! {
    do_syscall([code as usize, 0,0,0,0,0], SYSCALL_EXIT).unwrap();  // just to keep compiler happy
    unreachable!();
}

pub fn mmap(target: usize, length: usize, prot: usize, flag: usize, fd: FileDecstiptor, offset: usize) -> Result<usize, ErrorNum> {
    do_syscall([
        target, 
        length,
        prot,
        flag,
        fd.0,
        offset
    ], SYSCALL_MMAP)
}

pub fn waitpid(pid: isize, exit_code: &mut isize) -> Result<usize, ErrorNum> {
    do_syscall([
        pid as usize,
        exit_code as *mut isize as usize,
        0,0,0,0
    ], SYSCALL_WAITPID)
}

pub fn signal(target_pid: usize, signum: usize) ->  Result<usize, ErrorNum> {
    do_syscall([
        target_pid,
        signum,
        0,0,0,0
    ], SYSCALL_SIGNAL)
}

pub fn sigaction(signum: usize, handler: fn(isize)) -> Result<usize, ErrorNum> {
    do_syscall([
        signum,
        handler as *const fn(isize) as usize,
        0,0,0,0
    ], SYSCALL_SIGACTION)
}

// no manual sigreturn

pub fn getcwd_raw(buf: &mut [u8]) -> Result<usize, ErrorNum> {
    do_syscall([
        buf.as_mut_ptr() as usize,
        buf.len(),
        0,0,0,0
    ], SYSCALL_GETCWD)
}

pub fn getcwd() -> Result<String, ErrorNum> {
    let mut buffer = [0u8; 1024];
    getcwd_raw(&mut buffer)?;
    String::from_utf8(buffer.to_vec()).map_err(|_| ErrorNum::EBADCODEX)
}

pub fn chdir(path: &str) -> Result<usize, ErrorNum> {
    let mut arr = path.as_bytes().to_vec();
    arr.push(0);
    do_syscall([
        arr.as_ptr() as usize,
        path.len(),
        0,0,0,0
    ], SYSCALL_CHDIR)
}

pub fn sbrk(increment: isize) -> Result<usize, ErrorNum> {
    do_syscall([
        increment as usize,
        0,0,0,0,0
    ], SYSCALL_SBRK)
}

pub fn getdents(fd: FileDecstiptor, buf: &mut [Dirent]) -> Result<usize, ErrorNum> {
    let p = buf.as_mut_ptr();
    do_syscall([
        fd.0,
        p as usize,
        buf.len(),0,0,0
    ], SYSCALL_GETDENTS)
}

pub fn ioctl<T: Sized+Copy, F: Sized+Copy>(fd: FileDecstiptor, op: usize, param: T, res: &mut F) -> Result<usize, ErrorNum> {
    let param = unsafe{core::slice::from_raw_parts(&param as *const T as *const u8, core::mem::size_of::<T>())};
    let p_ptr = param.as_ptr() as usize;
    let p_size = core::mem::size_of::<T>();
    let r_ptr = res as *mut F as usize;
    let r_size = core::mem::size_of::<F>();
    do_syscall([
        fd.0,
        op,
        p_ptr,
        p_size,
        r_ptr,
        r_size,
    ], SYSCALL_IOCTL)
}