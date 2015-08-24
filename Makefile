# architecture detection
OS :=               $(shell uname -s | sed 's/ /_/' | tr A-Z a-z)
CPU :=              $(shell uname -m | sed 's/ /_/' | tr A-Z a-z)
ARCH :=             $(OS)_$(CPU)

# check which compiler to use (default clang). e.g. make prefer_gcc=1
ifeq ($(prefer_gcc),1)
CC :=               $(shell which gcc || which clang || which cc)
CXX :=              $(shell which g++ || which clang++ || which c++)
else
CC :=               $(shell which clang || which gcc || which cc)
CXX :=              $(shell which clang++ || which g++ || which c++)
endif

# linker, archiver and ragel parser generator
LD :=               $(CXX)
AR :=               $(shell which ar)
RAGEL :=            $(shell which ragel)

# compiler function tests
check_opt =         $(shell T=$$(mktemp /tmp/test.XXXX.$(2)); echo 'int main() { return 0; }' > $$T ; $(1) $(3) $$T -o /dev/null >/dev/null 2>&1 ; echo $$?; rm $$T)

# compiler flag test definitions
LIBCPP_FLAGS =      -stdlib=libc++
LTO_FLAGS =         -flto
PIE_FLAGS =         -fpie
STPS_FLAGS =        -fstack-protector-strong
STP_FLAGS =         -fstack-protector
RELRO_FLAGS =       -Wl,-z,relro
RELROF_FLAGS =      -Wl,-z,relro,-z,now
NOEXEC_FLAGS =      -Wl,-z,noexecstack

# default optimizer, debug and warning flags
INCLUDES :=         -I$(shell pwd)/sushi
OPT_FLAGS =         -O3
DEBUG_FLAGS =       -g
WARN_FLAGS =        -Wall -Wpedantic -Wsign-compare
CPPFLAGS =
CXXFLAGS =          -std=c++11 $(OPT_FLAGS) $(DEBUG_FLAGS) $(WARN_FLAGS) $(INCLUDES)
LDFLAGS =           

# check if we can use libc++
ifeq ($(call check_opt,$(CXX),cc,$(LIBCPP_FLAGS)), 0)
CXXFLAGS +=         $(LIBCPP_FLAGS)
endif

# check if hardening is enabled. e.g. make enable_harden=1
ifeq ($(enable_harden),1)
# check if we can use stack protector
ifeq ($(call check_opt,$(CXX),cc,$(STPS_FLAGS)), 0)
CXXFLAGS +=         $(STPS_FLAGS)
else
ifeq ($(call check_opt,$(CXX),cc,$(STP_FLAGS)), 0)
CXXFLAGS +=         $(STP_FLAGS)
endif
endif
# check if we can link with read only relocations
ifeq ($(call check_opt,$(CXX),cc,$(RELROF_FLAGS)), 0)
LDFLAGS +=          $(RELROF_FLAGS)
else
ifeq ($(call check_opt,$(CXX),cc,$(RELRO_FLAGS)), 0)
LDFLAGS +=          $(RELRO_FLAGS)
endif
endif
# check if we can link with non executable stack
ifeq ($(call check_opt,$(CXX),cc,$(NOEXEC_FLAGS)), 0)
LDFLAGS +=          $(NOEXEC_FLAGS)
endif
endif

# check if we can use compile position independent executable
ifeq ($(call check_opt,$(CXX),cc,$(PIE_FLAGS)), 0)
CXXFLAGS +=         $(PIE_FLAGS)
endif

# prefer link time optimization by default with clang
ifeq ($(findstring clang++,$(CXX)),clang++)
enable_lto=1
endif

# check if link time optimization is enabled. e.g. make enable_lto=1
ifeq ($(enable_lto),1)
# check if we can use link time optimization
ifeq ($(call check_opt,$(CXX),cc,$(LTO_FLAGS)), 0)
CXXFLAGS +=         $(LTO_FLAGS)
endif
endif

# check whether to enable sanitizer
ifneq (,$(filter $(sanitize),memory address thread undefined))
CXXFLAGS +=         -fno-omit-frame-pointer -fsanitize=$(sanitize)
ifeq ($(sanitize),memory)
CXXFLAGS +=         -fsanitize-memory-track-origins=2
endif
endif

# architecture specific flags
ifeq ($(OS),linux)
CPPFLAGS +=         -D_FILE_OFFSET_BITS=64
endif

# directories
APP_SRC_DIR =       app
LIB_SRC_DIR =       sushi
BUILD_DIR =         build
BIN_DIR =           $(BUILD_DIR)/$(ARCH)/bin
LIB_DIR =           $(BUILD_DIR)/$(ARCH)/lib
OBJ_DIR =           $(BUILD_DIR)/$(ARCH)/obj
DEP_DIR =           $(BUILD_DIR)/$(ARCH)/dep

# helper functions
src_objs =          $(subst $(APP_SRC_DIR),$(OBJ_DIR),$(subst $(LIB_SRC_DIR),$(OBJ_DIR),$(subst .cc,.o,$(1))))
src_deps =          $(subst $(APP_SRC_DIR),$(DEP_DIR),$(subst $(LIB_SRC_DIR),$(DEP_DIR),$(subst .cc,.cc.P,$(1))))

# target source and objects
SUSHI_SRCS =        $(LIB_SRC_DIR)/log.cc \
                    $(LIB_SRC_DIR)/xcode.cc \
                    $(LIB_SRC_DIR)/project.cc \
                    $(LIB_SRC_DIR)/project_parser.cc \
                    $(LIB_SRC_DIR)/util.cc

SUSHI_OBJS =        $(call src_objs, $(SUSHI_SRCS))
SUSHI_LIB =         $(LIB_DIR)/libsushi.a

PBXCREATE_SRCS =    $(APP_SRC_DIR)/pbx_create.cc
PBXCREATE_OBJS =    $(call src_objs, $(PBXCREATE_SRCS))
PBXCREATE_BIN =     $(BIN_DIR)/pbx_create

PBXREAD_SRCS =      $(APP_SRC_DIR)/pbx_read.cc
PBXREAD_OBJS =      $(call src_objs, $(PBXREAD_SRCS))
PBXREAD_BIN =       $(BIN_DIR)/pbx_read

SUSHICLI_SRCS =     $(APP_SRC_DIR)/maki.cc
SUSHICLI_OBJS =     $(call src_objs, $(SUSHICLI_SRCS))
SUSHICLI_BIN =      $(BIN_DIR)/maki

ALL_SRCS =          $(SUSHI_SRCS) $(PBXCREATE_SRCS) $(PBXREAD_SRCS) $(SUSHICLI_SRCS)
BINARIES =          $(PBXCREATE_BIN) $(PBXREAD_BIN) $(SUSHICLI_BIN)

# don't build library if LTO is enabled
ifeq ($(enable_lto),1)
LIBS =
else
LIBS =              $(SUSHI_LIB)
endif

# build rules
all: dirs $(LIBS) $(BINARIES)
.PHONY: dirs
dirs: ; @mkdir -p $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR) $(DEP_DIR)
clean: ; @echo "CLEAN $(BUILD_DIR)"; rm -rf $(BUILD_DIR)

backup: clean ; dir=$$(basename $$(pwd)) ; cd .. && tar -czf $${dir}-backup-$$(date '+%Y%m%d').tar.gz $${dir}
dist: clean ; dir=$$(basename $$(pwd)) ; cd .. && tar --exclude .git -czf $${dir}-$$(date '+%Y%m%d').tar.gz $${dir}

# build targets
ifeq ($(enable_lto),1)
$(PBXCREATE_BIN): $(PBXCREATE_OBJS) $(SUSHI_OBJS) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(PBXREAD_BIN): $(PBXREAD_OBJS) $(SUSHI_OBJS) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(SUSHICLI_BIN): $(SUSHICLI_OBJS) $(SUSHI_OBJS) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
else
$(SUSHI_LIB): $(SUSHI_OBJS) ; $(call cmd, AR $@, $(AR) cr $@ $^)
$(PBXCREATE_BIN): $(PBXCREATE_OBJS) $(SUSHI_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(PBXREAD_BIN): $(PBXREAD_OBJS) $(SUSHI_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(SUSHICLI_BIN): $(SUSHICLI_OBJS) $(SUSHI_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
endif

# build recipes
ifdef V
cmd = $2
else
cmd = @echo "$1"; $2
endif

$(OBJ_DIR)/%.o : $(APP_SRC_DIR)/%.cc ; $(call cmd, CXX $@, $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
$(OBJ_DIR)/%.o : $(LIB_SRC_DIR)/%.cc ; $(call cmd, CXX $@, $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
$(LIB_SRC_DIR)/%.cc : $(LIB_SRC_DIR)/%.rl ; $(call cmd, RAGEL $@, $(RAGEL) $< -o $@)
$(DEP_DIR)/%.cc.P : $(APP_SRC_DIR)/%.cc ; @mkdir -p $(DEP_DIR) ;
	$(call cmd, MKDEP $@, $(CXX) $(CXXFLAGS) -MM $< | sed "s#\(.*\)\.o#$(OBJ_DIR)/\1.o $(DEP_DIR)/\1.P#"  > $@)
$(DEP_DIR)/%.cc.P : $(LIB_SRC_DIR)/%.cc ; @mkdir -p $(DEP_DIR) ;
	$(call cmd, MKDEP $@, $(CXX) $(CXXFLAGS) -MM $< | sed "s#\(.*\)\.o#$(OBJ_DIR)/\1.o $(DEP_DIR)/\1.P#"  > $@)

# make dependencies
include $(call src_deps,$(ALL_SRCS))
