#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // uint64 *va, uint64 npages, uint64 * uaddr
  // printf("=== >>>>>>> pgaccess:\n");
  uint64 uva;
  uint64 dstva;
  int npages;
  // pte_t *pte;
  pagetable_t pagetable = myproc()->pagetable;

  // a = PGROUNDDOWN(va);
  argaddr(0, &uva);
  argint(1, &npages);
  argaddr(2, &dstva);

  pte_t *pte;
  // if((va % PGSIZE) != 0)
    // panic("uvmunmap: not aligned");

  // pgaccess(buf, 32, &abits)
  // a = walkaddr(pagetable, a);
  // pte = walk(pagetable, uva, 0);
  // printf("pa %p\n", *pte);
  // printf("pte %p [accessed %s]\n", *pte, *pte & PTE_A);
  int abits = 1;
  // int counter = 60;
  int bitLength = 0;
  // printf(abits << 2)
  // int fakeabits = 0b1000000000000000000000000000110;
  // uva = PGROUNDDOWN(uva);

  for(uint64 i = uva + (npages - 1)*PGSIZE; i >= uva ; i -= PGSIZE){
    bitLength++;
    // pte = walk(pagetable, i, 0);
    if((pte = walk(pagetable, i, 0)) == 0)
      panic("sys_pgaccess: walk");
    // if((*pte & PTE_V) == 0)
    //   panic("uvmunmap: not mapped");
    // if(PTE_FLAGS(*pte) == PTE_V)
    //   panic("uvmunmap: not a leaf");
    // *pte = 0;
    if ((*pte & PTE_A) == 0) {
      // printf("%d\n", (i - uva) / PGSIZE );
      abits = abits << 1;
    } else {
      // printf("%d active\n", (i - uva) / PGSIZE );
      abits = (abits << 1) | 1; 
    }
    // printf("%p\n", abits);
    // printf("pte %p [accessed %d]\n", *pte, *pte & PTE_A);
  };
  // shifit padding bit
  abits = (abits << (32 - bitLength)) >> (32 - bitLength);
  // printf("%p ==> %p\n", abits);
  // printf("%d")
  // print_binary(abits);
  copyout(pagetable, dstva, (char *)&abits, sizeof(abits));
  // if (0b01000000000000000000000000000110 != ((1 << 1) | (1 << 2) | (1 << 30)))
    // printf("incorrect access bits set");
  // printf("%p", abits);
  // vmprint(pagetable);
  // printf("-------- pgaccess end ------ \n\n");
  clearptea(pagetable, 2);
  // lab pgtbl: your code here.
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
