
DTC ?= dtc
CPP ?= cpp
DESTDIR ?=

DTCVERSION ?= $(shell $(DTC) --version | grep ^Version | sed 's/^.* //g')

MAKEFLAGS += -rR --no-print-directory

ALL_PLATFORMES := $(patsubst overlays/%,%,$(wildcard overlays/*))

PHONY += all
all: $(foreach i,$(ALL_PLATFORMES),all_$(i))

PHONY += clean
clean: $(foreach i,$(ALL_PLATFORMES),clean_$(i))

PHONY += install
install: $(foreach i,$(ALL_PLATFORMES),install_$(i))

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS += -rR --no-print-directory

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

DTC_FLAGS += -Wno-unit_address_vs_reg
#http://snapshot.debian.org/binary/device-tree-compiler/
#http://snapshot.debian.org/package/device-tree-compiler/1.4.4-1/#device-tree-compiler_1.4.4-1
#http://snapshot.debian.org/archive/debian/20170925T220404Z/pool/main/d/device-tree-compiler/device-tree-compiler_1.4.4-1_amd64.deb

ifeq "$(DTCVERSION)" "1.4.5"
	#GIT BROKEN!!!! Ubuntu Bionic has patches..
	DTC_FLAGS += -Wno-dmas_property
	DTC_FLAGS += -Wno-gpios_property
	DTC_FLAGS += -Wno-pwms_property
	DTC_FLAGS += -Wno-interrupts_property
endif

ifeq "$(DTCVERSION)" "1.4.6"
	#http://snapshot.debian.org/package/device-tree-compiler/1.4.6-1/#device-tree-compiler_1.4.6-1
	#http://snapshot.debian.org/archive/debian/20180426T224735Z/pool/main/d/device-tree-compiler/device-tree-compiler_1.4.6-1_amd64.deb
	#Debian: 1.4.6
	DTC_FLAGS += -Wno-chosen_node_is_root
	DTC_FLAGS += -Wno-alias_paths
	DTC_FLAGS += -Wno-avoid_unnecessary_addr_size
endif

ifeq "$(DTCVERSION)" "1.4.7"
	#http://snapshot.debian.org/package/device-tree-compiler/1.4.7-3/#device-tree-compiler_1.4.7-3
	#http://snapshot.debian.org/archive/debian/20180911T215003Z/pool/main/d/device-tree-compiler/device-tree-compiler_1.4.7-3_amd64.deb
	#Debian: 1.4.6
	DTC_FLAGS += -Wno-chosen_node_is_root
	DTC_FLAGS += -Wno-alias_paths
	DTC_FLAGS += -Wno-avoid_unnecessary_addr_size
endif

ifeq "$(DTCVERSION)" "1.5.0"
	#http://snapshot.debian.org/package/device-tree-compiler/1.5.0-1/#device-tree-compiler_1.5.0-1
	#http://snapshot.debian.org/archive/debian/20190313T032949Z/pool/main/d/device-tree-compiler/device-tree-compiler_1.5.0-1_amd64.deb
	#Debian: 1.4.6
	DTC_FLAGS += -Wno-chosen_node_is_root
	DTC_FLAGS += -Wno-alias_paths
	DTC_FLAGS += -Wno-avoid_unnecessary_addr_size
endif

ifeq "$(DTCVERSION)" "2.0.0"
	#BUILDBOT...http://gfnd.rcn-ee.org:8080/job/beagleboard_overlays/job/master/
	DTC_FLAGS += -Wno-chosen_node_is_root
	DTC_FLAGS += -Wno-alias_paths
endif

# Beautify output
# ---------------------------------------------------------------------------
#
# Normally, we echo the whole command before executing it. By making
# that echo $($(quiet)$(cmd)), we now have the possibility to set
# $(quiet) to choose other forms of output instead, e.g.
#
#         quiet_cmd_cc_o_c = Compiling $(RELDIR)/$@
#         cmd_cc_o_c       = $(CC) $(c_flags) -c -o $@ $<
#
# If $(quiet) is empty, the whole command will be printed.
# If it is set to "quiet_", only the short version will be printed. 
# If it is set to "silent_", nothing will be printed at all, since
# the variable $(silent_cmd_cc_o_c) doesn't exist.
#
# A simple variant is to prefix commands with $(Q) - that's useful
# for commands that shall be hidden in non-verbose mode.
#
#       $(Q)ln $@ :<
#
# If KBUILD_VERBOSE equals 0 then the above command will be hidden.
# If KBUILD_VERBOSE equals 1 then the above command is displayed.

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(filter 4.%,$(MAKE_VERSION)),)	# make-4
ifneq ($(filter %s ,$(firstword x$(MAKEFLAGS))),)
  quiet=silent_
endif
else					# make-3.8x
ifneq ($(filter s% -s%,$(MAKEFLAGS)),)
  quiet=silent_
endif
endif

export quiet Q KBUILD_VERBOSE

all_%:
	$(Q)$(MAKE) PLATFORM=$* all_arch
	gcc -o config-pin ./tools/pmunts_muntsos/config-pin.c

clean_%:
	$(Q)$(MAKE) PLATFORM=$* clean_arch
	rm config-pin || true
	rm -rf output

install_%:
	$(Q)$(MAKE) PLATFORM=$* install_arch

ifeq ($(PLATFORM),)

ALL_DTS		:= $(shell find overlays/* -name \*.dts)

ALL_DTB		:= $(patsubst %.dts,%.dtbo,$(ALL_DTS))

$(ALL_DTB): PLATFORM=$(word 2,$(subst /, ,$@))
$(ALL_DTB): FORCE
	$(Q)$(MAKE) PLATFORM=$(PLATFORM) $@

else

PLATFORM_DTS	:= $(shell find overlays/$(PLATFORM) -name \*.dts)

PLATFORM_DTB	:= $(patsubst %.dts,%.dtbo,$(PLATFORM_DTS))

src	:= overlays/$(PLATFORM)
obj	:= overlays/$(PLATFORM)

include scripts/Kbuild.include

cmd_files := $(wildcard $(foreach f,$(PLATFORM_DTB),$(dir $(f)).$(notdir $(f)).cmd))

ifneq ($(cmd_files),)
  include $(cmd_files)
endif

quiet_cmd_clean    = CLEAN   $(obj)
      cmd_clean    = rm -f $(__clean-files)

dtc-tmp = $(subst $(comma),_,$(dot-target).dts.tmp)

dtc_cpp_flags  = -Wp,-MD,$(depfile).pre.tmp -nostdinc		\
                 -Iinclude -I$(src) -Ioverlays -Itestcase-data	\
                 -undef -D__DTS__

quiet_cmd_dtc = DTC     $@
cmd_dtc = $(CPP) $(dtc_cpp_flags) -x assembler-with-cpp -o $(dtc-tmp) $< ; \
        $(DTC) -O dtb -o $@ -b 0 -@ \
                -i $(src) $(DTC_FLAGS) \
                -d $(depfile).dtc.tmp $(dtc-tmp) ; \
        cat $(depfile).pre.tmp $(depfile).dtc.tmp > $(depfile)

$(obj)/%.dtbo: $(src)/%.dts FORCE | create_output
	$(call if_changed_dep,dtc)
	@mv $@ output

create_output: 
	@mkdir -p output

PHONY += all_arch
all_arch: $(PLATFORM_DTB)
	@:

PHONY += install_arch
install_arch: $(PLATFORM_DTBO)
	mkdir -p $(DESTDIR)/lib/firmware/
	cp -v $(obj)/*.dtbo $(DESTDIR)/lib/firmware/
	mkdir -p $(DESTDIR)/usr/bin/
	cp -v config-pin $(DESTDIR)/usr/bin/

RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o -name CVS \
                   -o -name .pc -o -name .hg -o -name .git \) -prune -o

PHONY += clean_arch
clean_arch: __clean-files = $(PLATFORM_DTB)
clean_arch: FORCE
	$(call cmd,clean)
	@find . $(RCS_FIND_IGNORE) \
		\( -name '.*.cmd' \
		-o -name '.*.d' \
		-o -name '.*.tmp' \
		\) -type f -print | xargs rm -f

endif

help:
	@echo "Targets:"
	@echo "  all:                   Build all device tree binaries for all architectures"
	@echo "  clean:                 Clean all generated files"
	@echo "  install:               Install all generated files (sudo)"
	@echo ""
	@echo "  all_<PLATFORM>:            Build all device tree binaries for <PLATFORM>"
	@echo "  clean_<PLATFORM>:          Clean all generated files for <PLATFORM>"
	@echo "  install_<PLATFORM>:        Install all generated files for <PLATFORM> (sudo)"
	@echo ""
	@echo "  overlays/<PLATFORM>/<DTS>.dtbo   Build a single device tree binary"
	@echo ""
	@echo "Architectures: $(ALL_PLATFORMES)"

PHONY += FORCE
FORCE:

.PHONY: $(PHONY)


builddeb:
	# build the source package in the parent directory
	# then rename it to project_version.orig.tar.gz
	#$(PYTHON) setup.py sdist $(COMPILE) --dist-dir=../
	#rename -f 's/$(PROJECT)-(.*)\.tar\.gz/$(PROJECT)_$$1\.orig\.tar\.gz/' ../*
	# build the package
	./install.sh
	dpkg-buildpackage -i -I -rfakeroot

