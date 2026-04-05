#!/usr/local/bin/tcc -run -nostdlib

// Not working on windows and apple because of different API.

#include <unistd.h>
#include <sys/syscall.h>
#if defined __x86_64__
__asm__ ("syscall:\n\t"
	 "mov %rdi,%rax\n\t"
	 "mov %rsi,%rdi\n\t"
	 "mov %rdx,%rsi\n\t"
	 "mov %rcx,%rdx\n\t"
	 "mov %r8,%r10\n\t"
	 "mov %r9,%r8\n\t"
	 "mov 0x8(%rsp),%r9\n\t"
	 "syscall\n\t"
	 "ret");
__asm__ (".global _start\n\t"
	 "_start:\n\t"
	 "mov 0(%rsp), %rdi\n\t"
	 "lea 8(%rsp), %rsi\n\t"
	 "jmp print");
#elif defined __i386__
__asm__ ("syscall:\n\t"
	 "push %ebp\n\t"
	 "push %edi\n\t"
	 "push %esi\n\t"
	 "push %ebx\n\t"
	 "mov 0x2c(%esp),%ebp\n\t"
	 "mov 0x28(%esp),%edi\n\t"
	 "mov 0x24(%esp),%esi\n\t"
	 "mov 0x20(%esp),%edx\n\t"
	 "mov 0x1c(%esp),%ecx\n\t"
	 "mov 0x18(%esp),%ebx\n\t"
	 "mov 0x14(%esp),%eax\n\t"
	 // "call *%gs:0x10\n\t"
	 ".byte 0x65,0xff,0x15,0x10,0x00,0x00,0x00\n\t"
	 "pop %ebx\n\t"
	 "pop %esi\n\t"
	 "pop %edi\n\t"
	 "pop %ebp\n\t"
	 "ret");
__asm__ (".global _start\n\t"
	 "_start:\n\t"
	 "pop %esi\n\t"
	 "mov %esp, %ecx\n\t"
	 "and $0xfffffff0,%esp\n\t"
	 "push %ecx\n\t"
	 "push %esi\n\t"
	 "call print");
#elif defined __arm__
__asm__ ("syscall:\n\t"
	 "mov r12, sp\n\t"
	 "push {r4, r5, r6, r7}\n\t"
	 "mov r7, r0\n\t"
	 "mov r0, r1\n\t"
	 "mov r1, r2\n\t"
	 "mov r2, r3\n\t"
	 "ldm r12, {r3, r4, r5, r6}\n\t"
	 "svc 0x00000000\n\t"
	 "pop {r4, r5, r6, r7}\n\t"
	 "mov pc, lr");
__asm__ (".global _start\n\t"
	 "_start:\n\t"
	 "pop {r0}\n\t"
	 "mov r1, sp\n\t"
	 "bl print");
#elif defined __aarch64__
__asm__ ("syscall:\n\t"
	 ".int 0x2a0003e8\n\t" // mov w8, w0
	 ".int 0xaa0103e0\n\t" // x0, x1
	 ".int 0xaa0203e1\n\t" // mov x1, x2
	 ".int 0xaa0303e2\n\t" // mov x2, x3
	 ".int 0xaa0403e3\n\t" // mov x3, x4
	 ".int 0xaa0503e4\n\t" // mov x4, x5
	 ".int 0xaa0603e5\n\t" // mov x5, x6
	 ".int 0xaa0703e6\n\t" // mov x6, x7
	 ".int 0xd4000001\n\t" // svc  #0x0
	 ".int 0xd65f03c0"); // ret
__asm__ (".global _start\n\t"
	 "_start:\n\t"
	 ".int 0xf94003e0\n\t" // ldr x0, [sp]
	 ".int 0x910023e1\n\t" // add x1, sp, #08
	 ".reloc .,R_AARCH64_CALL26,print\n\t"
	 ".int 0x94000000"); // bl print
#elif defined __riscv
__asm__ ("syscall:\n\t"
	 "mv t1,a0\n\t"
	 "mv a0,a1\n\t"
	 "mv a1,a2\n\t"
	 "mv a2,a3\n\t"
	 "mv a3,a4\n\t"
	 "mv a4,a5\n\t"
	 "mv a5,a6\n\t"
	 "mv a6,a7\n\t"
	 "mv a7,t1\n\t"
	 "ecall\n\t"
	 "ret");
__asm__ (".global _start\n\t"
	 "_start:\n\t"
	 "ld a0,0(sp)\n\t"
	 "addi a1,sp,8\n\t"
	 "jal print");
#endif
unsigned long strlen(const char *s)
{
    unsigned long len = 0;

    while (*s++)
	len++;
    return len;
}

static void pr_num(int num)
{
    char val[20], *p = &val[20];
    
    *--p = '\0';
    do {
	int a = num, b = 0;

	while (a >= 10) {
	    a -= 10;
	    b++;
	}
	*--p = a + '0';
	num = b;
    } while (num);
    syscall(SYS_write, 1, p, strlen(p));
}

static void pr_str(int n, char *s)
{
    pr_num(n);
    syscall(SYS_write, 1, ": ", 2);
    syscall(SYS_write, 1, s, strlen(s));
    syscall(SYS_write, 1, "\n", 1);
}

void print(int argc, char **argv) {
    int i;
    char **envp = &argv[argc + 1];

    syscall(SYS_write, 1, "argc: ", 6);
    pr_num(argc);
    syscall(SYS_write, 1, "\n", 1);
    syscall(SYS_write, 1, "argv[]\n", 7);
    for (i = 0; i < argc; i++)
	pr_str(i, argv[i]);
    syscall(SYS_write, 1, "envp[]\n", 7);
    i = 0;
    while (*envp)
	pr_str(i++, *envp++);
    syscall(SYS_exit, 0);
}
