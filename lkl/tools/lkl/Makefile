# Do not use make's built-in rules
# (this improves performance and avoids hard-to-debug behaviour);
# also do not print "Entering directory..." messages from make
.SUFFIXES:
MAKEFLAGS += -r --no-print-directory

KCONFIG?=defconfig

ifneq ($(silent),1)
  ifneq ($(V),1)
	QUIET_AUTOCONF       = @echo '  AUTOCONF '$@;
	Q = @
  endif
endif

PREFIX   := /usr

ifeq (,$(srctree))
  srctree := $(patsubst %/,%,$(dir $(shell pwd)))
  srctree := $(patsubst %/,%,$(dir $(srctree)))
endif
export srctree

-include ../scripts/Makefile.include

# OUTPUT fixup should be *after* include ../scripts/Makefile.include
ifneq ($(OUTPUT),)
  OUTPUT := $(OUTPUT)/tools/lkl/
else
  OUTPUT := $(CURDIR)/
endif
export OUTPUT

all:

conf: $(OUTPUT)Makefile.conf

$(OUTPUT)Makefile.conf: Makefile.autoconf
	$(call QUIET_AUTOCONF)$(MAKE) -f Makefile.autoconf -s

-include $(OUTPUT)Makefile.conf

export CFLAGS += -I$(OUTPUT)/include -Iinclude -Wall -g -O2 -Wextra \
	 -Wno-unused-parameter \
	 -Wno-missing-field-initializers -fno-strict-aliasing

-include Targets

TARGETS := $(progs-y:%=$(OUTPUT)%$(EXESUF))
TARGETS += $(OUTPUT)liblkl.a
all: $(TARGETS)

# this workaround is for FreeBSD
bin/stat:
ifeq ($(LKL_HOST_CONFIG_BSD),y)
	$(Q)ln -sf `which gnustat` bin/stat
	$(Q)ln -sf `which gsed` bin/sed
else
	$(Q)touch bin/stat
endif

# rule to build lkl.o
$(srctree)/lkl.o: $(srctree)/.config

$(srctree)/.config: $(srctree)/arch/lkl/configs/defconfig
	$(Q)$(MAKE) -C ../.. ARCH=lkl $(KOPT) $(KCONFIG)

$(srctree)/lkl.o: FORCE
	$(MAKE) -C ../.. ARCH=lkl $(KOPT) install INSTALL_PATH=$(OUTPUT)

# rules to link libs
$(OUTPUT)%$(SOSUF): LDFLAGS += -shared
$(OUTPUT)%$(SOSUF): $(OUTPUT)%-in.o $(OUTPUT)liblkl.a
	$(QUIET_LINK)$(CC) $(LDFLAGS) $(LDFLAGS_$*-y) -o $@ $^ $(LDLIBS) $(LDLIBS_$*-y)

# liblkl is special
$(OUTPUT)liblkl$(SOSUF): $(srctree)/lkl.o $(OUTPUT)%-in.o
$(OUTPUT)liblkl.a: setup_sysheaders $(srctree)/lkl.o $(OUTPUT)lib/liblkl-in.o
	$(QUIET_AR)$(AR) -rc $@ $(filter-out setup_sysheaders, $^)

# rule to link programs
#$(OUTPUT)%$(EXESUF): $(OUTPUT)%-in.o $(OUTPUT)liblkl.a
#	$(QUIET_LINK)$(CC) $(LDFLAGS) $(LDFLAGS_$*-y) -o $@ $^ $(LDLIBS) $(LDLIBS_$*-y)

# rule to build objects
$(OUTPUT)%-in.o: $(srctree)/lkl.o FORCE
	$(Q)$(MAKE) -f $(srctree)/tools/build/Makefile.build dir=$(patsubst %/,%,$(dir $*)) obj=$(notdir $*)


clean:
	$(call QUIET_CLEAN, objects)find $(OUTPUT) -name '*.o' -delete -o -name '\.*.cmd'\
	 -delete -o -name '\.*.d' -delete
	$(call QUIET_CLEAN, headers)$(RM) -r $(OUTPUT)/include/lkl/
	$(call QUIET_CLEAN, liblkl.a)$(RM) $(OUTPUT)/liblkl.a
	$(call QUIET_CLEAN, targets)$(RM) $(TARGETS)
	$(call QUIET_CLEAN, lkl.o)$(RM) $(srctree)/lkl.o

clean-conf: clean
	$(call QUIET_CLEAN, Makefile.conf)$(RM) $(OUTPUT)/Makefile.conf

headers_install: $(TARGETS)
	$(call QUIET_INSTALL, headers) \
	    install -d $(DESTDIR)$(PREFIX)/include ; \
	    install -m 644 include/lkl.h include/lkl_host.h $(OUTPUT)include/lkl_autoconf.h \
	      include/lkl_config.h $(DESTDIR)$(PREFIX)/include ; \
	    cp -r $(OUTPUT)include/lkl $(DESTDIR)$(PREFIX)/include

libraries_install: $(libs-y:%=$(OUTPUT)%$(SOSUF)) $(OUTPUT)liblkl.a
	$(call QUIET_INSTALL, libraries) \
	    install -d $(DESTDIR)$(PREFIX)/lib ; \
	    install -m 644 $^ $(DESTDIR)$(PREFIX)/lib

programs_install: $(progs-y:%=$(OUTPUT)%$(EXESUF))
	$(call QUIET_INSTALL, programs) \
	    install -d $(DESTDIR)$(PREFIX)/bin ; \
	    install -m 755 $^ $(DESTDIR)$(PREFIX)/bin

install: headers_install libraries_install programs_install

setup_sysheaders:
	@if [ ! -d include/asm ]; then ln -sf /usr/include/asm-generic include/asm; fi
	@if [ ! -d include/asm-generic ]; then ln -sf /usr/include/asm-generic include/asm-generic; fi
	@if [ ! -d include/linux ]; then ln -sf /usr/include/linux include/linux; fi

FORCE: ;
.PHONY: all clean FORCE run-tests
.PHONY: headers_install libraries_install programs_install install
.PHONY: setup_sysheaders
.SECONDARY:

