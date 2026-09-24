#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = nullptr;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = nullptr;
ContFramePool * PageTable::process_mem_pool = nullptr;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size) 
{
	PageTable::kernel_mem_pool = _kernel_mem_pool;
	PageTable::process_mem_pool = _process_mem_pool;
	PageTable::shared_size = _shared_size;
}

PageTable::PageTable()
{
	//2-level paging... 
	unsigned long n_shared_pages = shared_size / PAGE_SIZE; 
	unsigned long PTs_needed = n_shared_pages / ENTRIES_PER_PAGE;
	//allocate fd + PTs 1...PTs_needed
	unsigned long fd_frame = kernel_mem_pool->get_frames(1 + PTs_needed);
	assert(fd_frame != 0); 
	//initialize page directory entries 0..PTs_needed
	unsigned long *pd = (unsigned long*)(PAGE_SIZE * fd_frame); //unsigned long b/c PT_SIZE is 32... 
	for(unsigned long i = 0;i < PTs_needed;i++){ 
		unsigned long pt_frame = (fd_frame + i + 1); 
		unsigned long *pt = (unsigned long*)(pt_frame * PAGE_SIZE); 
		//within current page table page(*pt), identity-map all pages to frames, and set present + write_enabled
		for(unsigned long j = 0;j < ENTRIES_PER_PAGE; j++){
			unsigned long phys = (i * ENTRIES_PER_PAGE + j) * PAGE_SIZE; //bits 31...12 are frame_idx 
			pt[j] = phys | 0x3; //present(0x1) + writes(0x2)
		}
		pd[i] = (unsigned long)pt | 0x3; //PDE -> Page table page 
	}
	for(unsigned long i = PTs_needed;i < ENTRIES_PER_PAGE; i++)
		pd[i] = 0x2; //writes enabled 
	page_directory = pd;
}


void PageTable::load()
{
    current_page_table = this;
    write_cr3((unsigned long)page_directory);
}

void PageTable::enable_paging()
{
    write_cr0(read_cr0() | 0x80000000);   // set bit 31 (PG)
	paging_enabled = 1;
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}

