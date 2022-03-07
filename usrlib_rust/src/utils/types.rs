
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct FileDecstiptor(pub usize);

impl From<usize> for FileDecstiptor {
    fn from(inner: usize) -> Self {
        Self(inner)
    }
}


#[repr(C)]
#[derive(Clone, Copy)]
pub struct Dirent {
    pub inode: u32,
    pub f_type: u16,
    pub name: [u8; 122]
}

use bitflags::*;

bitflags! {
    /// fs flags
    pub struct OpenMode: usize {
        const READ      = 1 << 0;
        const WRITE     = 1 << 1;
        const CREATE    = 1 << 2;
        const EXEC      = 1 << 3;
        const SYS       = 1 << 4;   // special access: opened by kernel
        const NO_FOLLOW = 1 << 5;   // do not follow symbolic link
    }
}
