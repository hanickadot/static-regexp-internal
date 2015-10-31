.PHONY: visible all
	
all:

VIS := @
ECHO := 

CC := clang
CXX := clang

libcxx := no
LIBCXX_INCLUDE := 
LIBCXX_LIBPATH := 

override MAXOPTIMIZE := -O3
OPTIMIZE := $(MAXOPTIMIZE)
LDOPTIMIZE := 

override UNAME_SYSTEM := $(shell uname)

# detection FreeBSD
ifeq ($(UNAME_SYSTEM),FreeBSD)
 CXX := clang 
 CC := clang
 override SYSTEM := freebsd
 override BUILD := FREEBSD
 override LDFLAGS := -lpthread -lc++
endif

# detection OSX
ifeq ($(UNAME_SYSTEM),Darwin)
 override SYSTEM := darwin
 override BUILD := DARWIN
endif

# detection Linuxu
ifeq ($(UNAME_SYSTEM),Linux)
 override SYSTEM := linux
 override BUILD := LINUX
 override HAVE_GOLD_TMP := $(shell $(LD) -v 2>&1 | grep "gold")
ifeq (,$(findstring gold,$(HAVE_GOLD_TMP)))
 override lto := no
endif
endif

override CXX_TYPE := unknown
override CXX_VERSION := $(shell $(CXX) -v 2>&1 | grep " version " )
override EMCC_VERSION := $(shell $(CXX) -v 2>&1 | grep emcc )

# detection gcc-clang (on osx)
ifneq (,$(findstring Apple LLVM,$(CXX_VERSION)))
 override CXX_TYPE_CLANG := clang
 override CXX_TYPE := clang
 override CXX_VERSION := ""
endif

# detection clang
ifneq (,$(findstring clang,$(CXX_VERSION)))
 override CXX_TYPE_CLANG := clang
 override CXX_TYPE := clang
 override CXX_CLANG_MAJOR := $(shell $(CXX) -v 2>&1 | grep " version " | cut -d' ' -f3  | cut -d'.' -f1)
 override CXX_CLANG_MINOR := $(shell $(CXX) -v 2>&1 | grep " version " | cut -d' ' -f3  | cut -d'.' -f2)
endif

# detection gcc
ifneq (,$(findstring gcc,$(CXX_VERSION)))
 override CXX_TYPE_GCC := gcc
 override CXX_TYPE := gcc
 override CXX_GCC_MAJOR := $(shell $(CXX) -v 2>&1 | grep " version " | cut -d' ' -f3  | cut -d'.' -f1)
 override CXX_GCC_MINOR := $(shell $(CXX) -v 2>&1 | grep " version " | cut -d' ' -f3  | cut -d'.' -f2)
endif

ifneq (,$(findstring icc,$(CXX_VERSION)))
 override CXX_TYPE_ICC := icc
 override CXX_TYPE := icc
endif

ifneq (,$(findstring emcc,$(EMCC_VERSION)))
 override CXX_TYPE_EMCC := emcc
 override CXX_TYPE := emcc
endif



##### KOMBINACE SYSTEMU / KOMPILATORU

override SLIBFLAGS += -shared

lto := yes

ifeq ($(debug), yes)
override lto := no
endif

ifeq ($(lto), yes)
override FLTO := -flto
override LDOPTIMIZE := $(OPTIMIZE)
else
override FLTO := 
endif

ifeq ($(lto), yes)
override BUILDNOTE := -lto
else
override BUILDNOTE := 
endif

ifneq ($(libcxx), no)
 LIBCXX_INCLUDE := $(libcxx)/include/c++/v1
 LIBCXX_LIBPATH := $(libcxx)/lib
endif

ifeq ($(SYSTEM), linux)
 override SYSTEMMAJ := LINUX
 override SONAME := -Wl,-soname,
else ifeq ($(SYSTEM), darwin)
 override SYSTEMMAJ := DARWIN
 override SONAME := -Wl,-install_name,
endif

ifeq ($(CXX_TYPE), gcc)
 override DEPFLAGS += -std=c++11
 override CXXFLAGS += -std=c++11
 override LDFLAGS += -lc++
endif

ifeq ($(CXX_TYPE), clang)
 override CFLAGS += -fPIC -std=gnu99
 override DEPCFLAGS += -std=gnu99
 override CXXFLAGS +=  -fPIC -std=c++11 -stdlib=libc++
 override MMFLAGS +=  -fPIC -std=c++11 -stdlib=libc++
 override DEPFLAGS += -std=c++11 -stdlib=libc++ 
 ifeq ($(SYSTEM), linux)
  override OPTIMIZE := $(MAXOPTIMIZE) $(FLTO)
  override LDOPTIMIZE := $(MAXOPTIMIZE) $(FLTO)
  override LDFLAGS += -lc++ -lc++abi -ldl -lpthread 
 else ifeq ($(SYSTEM), darwin)
  override OPTIMIZE := $(MAXOPTIMIZE) $(FLTO)
  override LDOPTIMIZE := $(MAXOPTIMIZE) $(FLTO)
  ifneq ($(LIBCXX_INCLUDE),)
    override DEPFLAGS += -I$(LIBCXX_INCLUDE)
    override CXXFLAGS += -I$(LIBCXX_INCLUDE)
    override MMFLAGS += -I$(LIBCXX_INCLUDE)
  endif
  ifneq ($(LIBCXX_LIBPATH),)
    override LDFLAGS += -L$(LIBCXX_LIBPATH) -lc++
  endif
  override LDFLAGS +=  -lc++
  override SLIBFLAGS += -Wl,-undefined -Wl,dynamic_lookup
 endif
endif

ifeq ($(debug), yes)
 TYPE := debug$(BUILDNOTE)
 override CFLAGS += -ggdb
 override CXXFLAGS += -ggdb
 override MMFLAGS += -ggdb
 override OPTIMIZE := -O0
 override LDOPTIMIZE := 
else
 TYPE := deploy$(BUILDNOTE)
endif

override CFLAGS += $(OPTIMIZE) $(INCLUDE_HEADS) -DCXX_TYPE_$(CXX_TYPE) -DBUILD_$(BUILD)
override DEPCFLAGS += $(OPTIMIZE) $(INCLUDE_HEADS) -DCXX_TYPE_$(CXX_TYPE) -DBUILD_$(BUILD)
override CXXFLAGS += $(OPTIMIZE) $(INCLUDE_HEADS) -DCXX_TYPE_$(CXX_TYPE) -DBUILD_$(BUILD)
override MMFLAGS += $(OPTIMIZE) $(INCLUDE_HEADS) -DCXX_TYPE_$(CXX_TYPE) -DBUILD_$(BUILD)
override DEPFLAGS += $(INCLUDE_HEADS) -DCXX_TYPE_$(CXX_TYPE)
override LDFLAGS += $(LDOPTIMIZE)

verbose := no

ifeq ($(verbose), yes)
override VIS := 
override ECHO := > /dev/null
else
override VIS := @
override ECHO := 
endif
#verbose := no
#override VIS_no := @
#override VIS_yes :=
#override VIS := $(VIS_$(verbose))
#override ECHO_no := 
#override ECHO_yes := > /dev/null
#override ECHO := $(ECHO_$(verbose))

config:
	@echo "------"
	$(VIS)echo "LTO: $(lto)"
	$(VIS)echo "CXX: $(CXX)"
	$(VIS)echo "CXX_VERSION: $(CXX_VERSION)"
	$(VIS)echo "cxxtype: $(CXX_TYPE)"
	$(VIS)echo "tmp: $(TMP)"
	$(VIS)echo "CXXFLAGS: $(CXXFLAGS)"
	$(VIS)echo "LDFLAGS: $(LDFLAGS)"
	$(VIS)echo "$(TYPE)"
	$(VIS)echo "system: $(SYSTEM)"
	$(VIS)echo "version: $(CXX_VERSION)"
	$(VIS)echo "$(CXX_TYPE_CLANG)"
	$(VIS)echo "$(CXX_CLANG_MAJOR)"
	$(VIS)echo "$(CXX_CLANG_MINOR)"
	$(VIS)echo "$(CXX_TYPE_LLVM_GCC)"
	$(VIS)echo "type: $(CXX_TYPE)"
	$(VIS)echo "$(CXX_GCC_MAJOR)"
	$(VIS)echo "$(CXX_GCC_MINOR)"
	$(VIS)echo "$(findstring icc,$(CXX_VERSION))"
	$(VIS)echo $(findstring v,$(MAKEFLAGS))