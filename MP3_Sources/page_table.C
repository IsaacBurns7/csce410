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

//is this all I have left to do (probably 1 hr or less?) 
	//this is most of MP3 though... ? 
	//obviously + report 
	//obviously + report 
void PageTable::handle_fault(REGS * _r)
{
	if((_r->err_code) & 0x1){
		Console::puts("PROTECTION FAULT at addr=");
		Console::putui(read_cr2());
		Console::puts(" eip=");
		Console::putui(_r->eip);
		Console::puts(" err=");
		Console::putui(_r->err_code);
		Console::puts("\n");
		abort();
	}
	unsigned long addr = read_cr2();
	unsigned long PDE_idx = (addr >> 22); 
	unsigned long PTE_mask = (1 << 10) - 1; 
	unsigned long PTE_idx = (addr >> 12) & PTE_mask;
	unsigned long offset_mask = (1 << 12) - 1; 
	unsigned long offset = addr & offset_mask; 
	unsigned long *pd = current_page_table->page_directory;
	//PDE not present - walk PD
	if(!(pd[PDE_idx] & 0x1)){
		//create PTP at frame (PDE_idx << 22) + (PTE_idx << 12) 
		unsigned long PT_frame = kernel_mem_pool->get_frames(1); 
		unsigned long *new_PT = (unsigned long *) (PT_frame * PAGE_SIZE); //this is direct mapped...
		for(int i = 0;i < ENTRIES_PER_PAGE; i++){
			new_PT[i] = 0x2; //R/W, not present, 0x6 if user
		}
		pd[PDE_idx] = (PT_frame * PAGE_SIZE) | 0x3; //present, R/W, 0x7 if user 
	}
	//PTE not present - walk PT 
	unsigned long PDE_frame_mask = ((1UL << 20) - 1) << 12; 
	unsigned long *pt = (unsigned long *) (pd[PDE_idx] & PDE_frame_mask);
	unsigned long page_frame = process_mem_pool->get_frames(1);
	pt[PTE_idx] = (page_frame * PAGE_SIZE) | 0x3; //present, R/W 
  	Console::puts("handled page fault\n");
}

