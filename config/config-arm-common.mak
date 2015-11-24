#
# arm common makefile
#
# Authors: Andrew Jones <drjones@redhat.com>
#

ifeq ($(LOADADDR),)
	# qemu mach-virt default load address
	LOADADDR = 0x40000000
endif

tests-common = $(TEST_DIR)/selftest.flat
tests-common += $(TEST_DIR)/spinlock-test.flat
tests-common += $(TEST_DIR)/tlbflush-test.flat
tests-common += $(TEST_DIR)/locking-test.flat
tests-common += $(TEST_DIR)/barrier-litmus-test.flat

all: test_cases

##################################################################
phys_base = $(LOADADDR)

CFLAGS += -std=gnu99
CFLAGS += -ffreestanding
CFLAGS += -Wextra
CFLAGS += -O2
CFLAGS += -I lib -I lib/libfdt

asm-offsets = lib/$(ARCH)/asm-offsets.h
include config/asm-offsets.mak

cflatobjs += lib/alloc.o
cflatobjs += lib/devicetree.o
cflatobjs += lib/virtio.o
cflatobjs += lib/virtio-mmio.o
cflatobjs += lib/chr-testdev.o
cflatobjs += lib/prng.o
cflatobjs += lib/arm/io.o
cflatobjs += lib/arm/setup.o
cflatobjs += lib/arm/mmu.o
cflatobjs += lib/arm/bitops.o
cflatobjs += lib/arm/psci.o
cflatobjs += lib/arm/smp.o

libeabi = lib/arm/libeabi.a
eabiobjs = lib/arm/eabi_compat.o

libgcc := $(shell $(CC) $(machine) --print-libgcc-file-name)
start_addr := $(shell printf "%x\n" $$(( $(phys_base) + $(kernel_offset) )))

FLATLIBS = $(libcflat) $(LIBFDT_archive) $(libgcc) $(libeabi)
%.elf: LDFLAGS = $(CFLAGS) -nostdlib
%.elf: %.o $(FLATLIBS) arm/flat.lds
	$(CC) $(LDFLAGS) -o $@ \
		-Wl,-T,arm/flat.lds,--build-id=none,-Ttext=$(start_addr) \
		$(filter %.o, $^) $(FLATLIBS)

%.flat: %.elf
	$(OBJCOPY) -O binary $^ $@

$(libeabi): $(eabiobjs)
	$(AR) rcs $@ $^

arm_clean: libfdt_clean asm_offsets_clean
	$(RM) $(TEST_DIR)/*.{o,flat,elf} $(libeabi) $(eabiobjs) \
	      $(TEST_DIR)/.*.d lib/arm/.*.d

##################################################################

generated_files = $(asm-offsets)

test_cases: $(generated_files) $(tests-common) $(tests)

$(TEST_DIR)/selftest.elf: $(cstart.o) $(TEST_DIR)/selftest.o
$(TEST_DIR)/spinlock-test.elf: $(cstart.o) $(TEST_DIR)/spinlock-test.o
$(TEST_DIR)/tlbflush-test.elf: $(cstart.o) $(TEST_DIR)/tlbflush-test.o
$(TEST_DIR)/locking-test.elf: $(cstart.o) $(TEST_DIR)/locking-test.o
$(TEST_DIR)/barrier-litmus-test.elf: $(cstart.o) $(TEST_DIR)/barrier-litmus-test.o
