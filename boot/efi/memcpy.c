#ifndef _My_OS_memcpy_
#define _My_OS_memcpy_
void *memcpy(void *dst,void *src,unsigned long long int size);
asm(".text");
asm("memcpy:");
asm("_memcpy_X2:");
asm("sub $64,%rdx");
asm("jb _memcpy_X1");
asm("movups (%rsi),%xmm0");
asm("movups 16(%rsi),%xmm1");
asm("movups 32(%rsi),%xmm2");
asm("movups 48(%rsi),%xmm3");
asm("movups %xmm0,(%rdi)");
asm("movups %xmm1,16(%rdi)");
asm("movups %xmm2,32(%rdi)");
asm("movups %xmm3,48(%rdi)");
asm("lea 64(%rdi),%rdi");
asm("lea 64(%rsi),%rsi");
asm("jmp _memcpy_X2");
asm("_memcpy_X1:");
asm("test $32,%dl");
asm("je _memcpy_X3");
asm("movups (%rsi),%xmm0");
asm("movups 16(%rsi),%xmm1");
asm("movups %xmm0,(%rdi)");
asm("movups %xmm1,16(%rdi)");
asm("lea 32(%rdi),%rdi");
asm("lea 32(%rsi),%rsi");
asm("_memcpy_X3:");
asm("test $16,%dl");
asm("je _memcpy_X4");
asm("movups (%rsi),%xmm0");
asm("movups %xmm0,(%rdi)");
asm("lea 16(%rdi),%rdi");
asm("lea 16(%rsi),%rsi");
asm("_memcpy_X4:");
asm("test $8,%dl");
asm("je _memcpy_X5");
asm("mov (%rsi),%rax");
asm("mov %rax,(%rdi)");
asm("lea 8(%rdi),%rdi");
asm("lea 8(%rsi),%rsi");
asm("_memcpy_X5:");
asm("test $4,%dl");
asm("je _memcpy_X6");
asm("mov (%rsi),%eax");
asm("mov %eax,(%rdi)");
asm("lea 4(%rdi),%rdi");
asm("lea 4(%rsi),%rsi");
asm("_memcpy_X6:");
asm("test $2,%dl");
asm("je _memcpy_X7");
asm("mov (%rsi),%ax");
asm("mov %ax,(%rdi)");
asm("lea 2(%rdi),%rdi");
asm("lea 2(%rsi),%rsi");
asm("_memcpy_X7:");
asm("test $1,%dl");
asm("je _memcpy_X8");
asm("mov (%rsi),%al");
asm("mov %al,(%rdi)");
asm("_memcpy_X8:");
asm("ret");
#endif