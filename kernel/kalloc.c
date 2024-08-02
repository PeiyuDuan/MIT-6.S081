// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

// struct {
//   struct spinlock lock;
//   struct run *freelist;
// } kmem;

struct {
  struct spinlock lock;
  struct run* freelist;
  char lockname[8];
} kmems[NCPU];

void
kinit()
{
  // initlock(&kmem.lock, "kmem");
  for (int i = 0; i < NCPU; i++)
  {
    snprintf(kmems[i].lockname, 8, "kmem%d", i);
    initlock(&kmems[i].lock, kmems[i].lockname);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int now_cpu = cpuid();

  acquire(&kmems[now_cpu].lock);
  r->next = kmems[now_cpu].freelist;
  kmems[now_cpu].freelist = r;
  release(&kmems[now_cpu].lock);

  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int now_cpu = cpuid();

  acquire(&kmems[now_cpu].lock);
  r = kmems[now_cpu].freelist;
  if(r)
    kmems[now_cpu].freelist = r->next;
  else
  {
    int free_cpu;
    for (free_cpu = 0; free_cpu < NCPU; free_cpu++)
    {
      if (free_cpu == now_cpu)
        continue;
      acquire(&kmems[free_cpu].lock);
      r = kmems[free_cpu].freelist;
      if (r)
      {
        kmems[free_cpu].freelist = r->next;
        release(&kmems[free_cpu].lock);
        break;
      }

      release(&kmems[free_cpu].lock);
    }
  }
  release(&kmems[now_cpu].lock);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
