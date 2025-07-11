#include <stdio.h>
#include <stdint.h>

struct cpuid_out {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
};

void
lcpuid(const unsigned leaf, const unsigned subleaf, struct cpuid_out *out)
{
        if (out == NULL)
                return;

#ifdef __x86_64__
        asm volatile("mov %4, %%eax\n\t"
                     "mov %5, %%ecx\n\t"
                     "cpuid\n\t"
                     "mov %%eax, %0\n\t"
                     "mov %%ebx, %1\n\t"
                     "mov %%ecx, %2\n\t"
                     "mov %%edx, %3\n\t"
                     : "=g"(out->eax), "=g"(out->ebx), "=g"(out->ecx),
                       "=g"(out->edx)
                     : "g"(leaf), "g"(subleaf)
                     : "%eax", "%ebx", "%ecx", "%edx");
#else
        asm volatile("push %%ebx\n\t"
                     "mov %4, %%eax\n\t"
                     "mov %5, %%ecx\n\t"
                     "cpuid\n\t"
                     "mov %%eax, %0\n\t"
                     "mov %%ebx, %1\n\t"
                     "mov %%ecx, %2\n\t"
                     "mov %%edx, %3\n\t"
                     "pop %%ebx\n\t"
                     : "=g"(out->eax), "=g"(out->ebx), "=g"(out->ecx),
                       "=g"(out->edx)
                     : "g"(leaf), "g"(subleaf)
                     : "%eax", "%ecx", "%edx");
#endif
}

int main(int argc, char *argv[])
{
	struct cpuid_out res;
	lcpuid(0x7, 0x0, &res);
	if (!(res.ebx & (1 << 12))) {
		printf("CPUID.0x7.0: Monitoring capability not supported!\n");
		return 1;
    }
    return 0;
}
