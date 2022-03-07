// TODO: implement our own allocator
use buddy_system_allocator::LockedHeap;

pub const HEAP_SIZE: usize = 4096 * 8;

/// The global allocator, enables us to use extern alloc crate.
#[global_allocator]
static HEAP_ALLOCATOR: LockedHeap<64> = LockedHeap::empty();

/// The empty space to use as heap.
static mut HEAP_SPACE: [u8; HEAP_SIZE] = [0; HEAP_SIZE];

/// Initialized the heap  
/// *Don't call this multiple times!*
pub fn init_heap() {
    unsafe {
        HEAP_ALLOCATOR.lock().init(HEAP_SPACE.as_ptr() as usize, HEAP_SIZE);
    }
}

/// Alloc error handler
/// Panic on allocation error.
#[alloc_error_handler]
pub fn on_alloc_error(layout: core::alloc::Layout) -> ! {
    panic!("Heap allocation error on allocating layout {:?}. OOM?", layout);
}