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
#define read_sysreg(reg) ({						\
	u64 __val;							\
	asm("mrs	%0, " #reg : "=r" (__val));			\
	__val;								\
})

#define write_sysreg(reg, val) ({		\
      asm("msr	" #reg ", %0" :: "r" (val));    \
})

typedef struct {
  uint32_t dbgbcr;
  uint64_t dbgbvr;
  uint32_t dbgwcr;
  uint64_t dbgwvr;
} dbgregs_t;

static cpumask_t smp_test_complete;

static void read_dbgb(int n, dbgregs_t *array)
{
  switch (n-1) {
  case 15:
    array[15].dbgbcr = read_sysreg(dbgbcr15_el1);
    array[15].dbgbvr = read_sysreg(dbgbcr15_el1);
  case 14:
    array[14].dbgbcr = read_sysreg(dbgbcr14_el1);
    array[14].dbgbvr = read_sysreg(dbgbcr14_el1);
  case 13:
    array[13].dbgbcr = read_sysreg(dbgbcr13_el1);
    array[13].dbgbvr = read_sysreg(dbgbcr13_el1);
  case 12:
    array[12].dbgbcr = read_sysreg(dbgbcr12_el1);
    array[12].dbgbvr = read_sysreg(dbgbcr12_el1);
  case 11:
    array[11].dbgbcr = read_sysreg(dbgbcr11_el1);
    array[11].dbgbvr = read_sysreg(dbgbcr11_el1);
  case 10:
    array[10].dbgbcr = read_sysreg(dbgbcr10_el1);
    array[10].dbgbvr = read_sysreg(dbgbcr10_el1);
  case 9:
    array[9].dbgbcr = read_sysreg(dbgbcr9_el1);
    array[9].dbgbvr = read_sysreg(dbgbcr9_el1);
  case 8:
    array[8].dbgbcr = read_sysreg(dbgbcr8_el1);
    array[8].dbgbvr = read_sysreg(dbgbcr8_el1);
  case 7:
    array[7].dbgbcr = read_sysreg(dbgbcr7_el1);
    array[7].dbgbvr = read_sysreg(dbgbcr7_el1);
  case 6:
    array[6].dbgbcr = read_sysreg(dbgbcr6_el1);
    array[6].dbgbvr = read_sysreg(dbgbcr6_el1);
  case 5:
    array[5].dbgbcr = read_sysreg(dbgbcr5_el1);
    array[5].dbgbvr = read_sysreg(dbgbcr5_el1);
  case 4:
    array[4].dbgbcr = read_sysreg(dbgbcr4_el1);
    array[4].dbgbvr = read_sysreg(dbgbvr4_el1);
  case 3:
    array[3].dbgbcr = read_sysreg(dbgbcr3_el1);
    array[3].dbgbvr = read_sysreg(dbgbvr3_el1);
  case 2:
    array[2].dbgbcr = read_sysreg(dbgbcr2_el1);
    array[2].dbgbvr = read_sysreg(dbgbvr2_el1);
  case 1:
    array[1].dbgbcr = read_sysreg(dbgbcr1_el1);
    array[1].dbgbvr = read_sysreg(dbgbvr1_el1);
  case 0:
    array[0].dbgbcr = read_sysreg(dbgbcr0_el1);
    array[0].dbgbvr = read_sysreg(dbgbvr0_el1);
    break;
   default:
     break;
  }
}

static void test_debug_regs(void)
{
	int errors = 0;
	int cpu = smp_processor_id();
	unsigned int id = read_sysreg(ID_AA64DFR0_EL1);;
	int nbp, nwp, i;
        dbgregs_t initial[16], current[16];

	printf("CPU%d online\n", cpu);

	/* Check the number of break/watch points */
	nbp = ((id >> 12) & 0xf) + 1;
	nwp = ((id >> 20) & 0xf) + 1;

	printf("CPU%d: %d BPS, %d WPS\n", cpu, nbp, nwp);

        read_dbgb(nbp, &initial[0]);
        /* read_dbgw(cpu, nwp, &initial); */
        for (i=0; i<nbp; i++) {
          printf("CPU%d: B%d 0x%08x:0x%016llx\n", cpu, i, initial[i].dbgbcr, initial[i].dbgbvr);
        }
        
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
