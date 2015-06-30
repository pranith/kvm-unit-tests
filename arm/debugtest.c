/* 
 * Test guest access to debug registers.
 *
 * Copyright (C) 2015, Linaro Ltd, Alex Bennée <alex.bennee@linaro.org>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */

#include <libcflat.h>
#include <asm/psci.h>
#include <asm/smp.h>
#include <asm/cpumask.h>
#include <asm/barrier.h>

/* From Linux cputype.h */
#define read_cpuid(reg) ({						\
	u64 __val;							\
	asm("mrs	%0, " #reg : "=r" (__val));			\
	__val;								\
})


static cpumask_t smp_test_complete;

static void test_debug_regs(void)
{
	int errors = 0;
	int cpu = smp_processor_id();
	unsigned int id = read_cpuid(ID_AA64DFR0_EL1);;
	int nbp, nwp;

	printf("CPU%d online\n", cpu);

	/* Check the number of break/watch points */
	nbp = ((id >> 12) & 0xf) + 1;
	nwp = ((id >> 20) & 0xf) + 1;

	printf("  %d BPS, %d WPS\n", nbp, nwp);

	report("CPU%d: Done - Errors: %d\n", errors == 0, cpu, errors);

	cpumask_set_cpu(cpu, &smp_test_complete);
	if (cpu != 0)
		halt();
}

/* int main(int argc, char **argv) */
int main(void)
{
	int cpu;

	for_each_present_cpu(cpu) {
		if (cpu == 0)
			continue;
		smp_boot_secondary(cpu, test_debug_regs);
	}

	test_debug_regs();

	while (!cpumask_full(&smp_test_complete))
		cpu_relax();

	return report_summary();
}
