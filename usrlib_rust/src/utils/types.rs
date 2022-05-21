
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

bitflags! {
    pub struct Permission: u16 {
        const OWNER_R = 0o400;
        const OWNER_W = 0o200;
        const OWNER_X = 0o100;
        const GROUP_R = 0o040;
        const GROUP_W = 0o020;
        const GROUP_X = 0o010;
        const OTHER_R = 0o004;
        const OTHER_W = 0o002;
        const OTHER_X = 0o001;
    }
}

impl Permission {
    pub fn default() -> Self {
        Self::OWNER_R | Self::OWNER_W | Self::GROUP_R | Self::OTHER_R
    }

    pub fn ro() -> Self {
        Self::OWNER_R | Self::GROUP_R | Self::OTHER_R
    }
}