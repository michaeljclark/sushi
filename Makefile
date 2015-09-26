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
INCLUDES :=         -I$(shell pwd)/sushi -I$(shell pwd)/tinyxml2
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
TEST_SRC_DIR =      test
MAKI_SRC_DIR =      maki
SUSHI_SRC_DIR =     sushi
TINYXML2_SRC_DIR =  tinyxml2
BUILD_DIR =         build
BIN_DIR =           $(BUILD_DIR)/$(ARCH)/bin
LIB_DIR =           $(BUILD_DIR)/$(ARCH)/lib
OBJ_DIR =           $(BUILD_DIR)/$(ARCH)/obj
DEP_DIR =           $(BUILD_DIR)/$(ARCH)/dep

# target source and objects
SUSHI_SRCS =        $(SUSHI_SRC_DIR)/project.cc \
                    $(SUSHI_SRC_DIR)/project_parser.cc \
                    $(SUSHI_SRC_DIR)/util.cc \
                    $(SUSHI_SRC_DIR)/visual_studio.cc \
                    $(SUSHI_SRC_DIR)/visual_studio_parser.cc \
                    $(SUSHI_SRC_DIR)/xcode.cc
SUSHI_OBJS =        $(addprefix $(OBJ_DIR)/,$(subst .cc,.o,$(SUSHI_SRCS)))
SUSHI_LIB =         $(LIB_DIR)/libsushi.a

TINYXML2_SRCS =		$(TINYXML2_SRC_DIR)/tinyxml2.cpp
TINYXML2_OBJS =     $(addprefix $(OBJ_DIR)/,$(subst .cpp,.o,$(TINYXML2_SRCS)))
TINYXML2_LIB =      $(LIB_DIR)/libtinyxml2.a

GLOBRE_SRCS =       $(TEST_SRC_DIR)/globre.cc
GLOBRE_OBJS =       $(addprefix $(OBJ_DIR)/,$(subst .cc,.o,$(GLOBRE_SRCS)))
GLOBRE_BIN =        $(BIN_DIR)/globre

PBXREAD_SRCS =      $(TEST_SRC_DIR)/pbx_read.cc
PBXREAD_OBJS =      $(addprefix $(OBJ_DIR)/,$(subst .cc,.o,$(PBXREAD_SRCS)))
PBXREAD_BIN =       $(BIN_DIR)/pbx_read

VSREAD_SRCS =       $(TEST_SRC_DIR)/vs_read.cc
VSREAD_OBJS =       $(addprefix $(OBJ_DIR)/,$(subst .cc,.o,$(VSREAD_SRCS)))
VSREAD_BIN =        $(BIN_DIR)/vs_read

MAKI_SRCS =         $(MAKI_SRC_DIR)/maki.cc
MAKI_OBJS =         $(addprefix $(OBJ_DIR)/,$(subst .cc,.o,$(MAKI_SRCS)))
MAKI_BIN =          $(BIN_DIR)/maki

APP_SRCS =          $(PBXREAD_SRCS) $(VSREAD_SRCS) $(MAKI_SRCS)
BINARIES =          $(MAKI_BIN)
TESTS =             $(GLOBRE_BIN) $(PBXREAD_BIN) $(VSREAD_BIN)


# build rules
all: dirs $(LIBS) $(BINARIES) $(TESTS)

.PHONY: dirs
dirs: ; @mkdir -p $(OBJ_DIR)/test $(OBJ_DIR)/maki $(OBJ_DIR)/sushi $(OBJ_DIR)/tinyxml2 $(LIB_DIR) $(BIN_DIR)
clean: ; @echo "CLEAN $(BUILD_DIR)"; rm -rf $(BUILD_DIR)

backup: clean ; dir=$$(basename $$(pwd)) ; cd .. && tar -czf $${dir}-backup-$$(date '+%Y%m%d').tar.gz $${dir}
dist: clean ; dir=$$(basename $$(pwd)) ; cd .. && tar --exclude .git -czf $${dir}-$$(date '+%Y%m%d').tar.gz $${dir}

# build targets
$(SUSHI_LIB): $(SUSHI_OBJS) ; $(call cmd, AR $@, $(AR) cr $@ $^)
$(TINYXML2_LIB): $(TINYXML2_OBJS) ; $(call cmd, AR $@, $(AR) cr $@ $^)
$(PBXCREATE_BIN): $(PBXCREATE_OBJS) $(SUSHI_LIB) $(TINYXML2_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(GLOBRE_BIN): $(GLOBRE_OBJS) $(SUSHI_LIB) $(TINYXML2_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(PBXREAD_BIN): $(PBXREAD_OBJS) $(SUSHI_LIB) $(TINYXML2_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(VSREAD_BIN): $(VSREAD_OBJS) $(SUSHI_LIB) $(TINYXML2_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(UUID_BIN): $(UUID_OBJS) $(SUSHI_LIB) $(TINYXML2_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)
$(MAKI_BIN): $(MAKI_OBJS) $(SUSHI_LIB) $(TINYXML2_LIB) ; $(call cmd, LD $@, $(LD) $(CXXFLAGS) $(LDFLAGS) $^ -o $@)

# build recipes
ifdef V
cmd = $2
else
cmd = @echo "$1"; $2
endif

$(OBJ_DIR)/test/%.o : $(TEST_SRC_DIR)/%.cc ; $(call cmd, CXX $@, $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
$(OBJ_DIR)/maki/%.o : $(MAKI_SRC_DIR)/%.cc ; $(call cmd, CXX $@, $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
$(OBJ_DIR)/sushi/%.o : $(SUSHI_SRC_DIR)/%.cc ; $(call cmd, CXX $@, $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
$(OBJ_DIR)/tinyxml2/%.o : $(TINYXML2_SRC_DIR)/%.cpp ; $(call cmd, CXX $@, $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@)
$(SUSHI_SRC_DIR)/%.cc : $(SUSHI_SRC_DIR)/%.rl ; $(call cmd, RAGEL $@, $(RAGEL) $< -o $@)

$(DEP_DIR)/$(TEST_SRC_DIR)/%.cc.P : $(TEST_SRC_DIR)/%.cc ; @mkdir -p $(DEP_DIR)/$(TEST_SRC_DIR) ;
	$(call cmd, MKDEP $@, $(CXX) $(CXXFLAGS) -MM $< | sed "s#\(.*\)\.o#$(OBJ_DIR)/$(TEST_SRC_DIR)/\1.o $(DEP_DIR)/$(TEST_SRC_DIR)/\1.cc.P#"  > $@)
$(DEP_DIR)/$(MAKI_SRC_DIR)/%.cc.P : $(MAKI_SRC_DIR)/%.cc ; @mkdir -p $(DEP_DIR)/$(MAKI_SRC_DIR) ;
	$(call cmd, MKDEP $@, $(CXX) $(CXXFLAGS) -MM $< | sed "s#\(.*\)\.o#$(OBJ_DIR)/$(MAKI_SRC_DIR)/\1.o $(DEP_DIR)/$(MAKI_SRC_DIR)/\1.cc.P#"  > $@)
$(DEP_DIR)/$(SUSHI_SRC_DIR)/%.cc.P : $(SUSHI_SRC_DIR)/%.cc ; @mkdir -p $(DEP_DIR)/$(SUSHI_SRC_DIR) ;
	$(call cmd, MKDEP $@, $(CXX) $(CXXFLAGS) -MM $< | sed "s#\(.*\)\.o#$(OBJ_DIR)/$(SUSHI_SRC_DIR)/\1.o $(DEP_DIR)/$(SUSHI_SRC_DIR)/\1.cc.P#"  > $@)
$(DEP_DIR)/$(TINYXML2_SRC_DIR)/%.cpp.P : $(TINYXML2_SRC_DIR)/%.cpp ; @mkdir -p $(DEP_DIR)/$(TINYXML2_SRC_DIR) ;
	$(call cmd, MKDEP $@, $(CXX) $(CXXFLAGS) -MM $< | sed "s#\(.*\)\.o#$(OBJ_DIR)/$(TINYXML2_SRC_DIR)/\1.o $(DEP_DIR)/$(TINYXML2_SRC_DIR)/\1.cpp.P#"  > $@)

# make dependencies
include $(addprefix $(DEP_DIR)/,$(subst .cc,.cc.P,$(TEST_SRCS)))
include $(addprefix $(DEP_DIR)/,$(subst .cc,.cc.P,$(MAKI_SRCS)))
include $(addprefix $(DEP_DIR)/,$(subst .cc,.cc.P,$(SUSHI_SRCS)))
include $(addprefix $(DEP_DIR)/,$(subst .cpp,.cpp.P,$(TINYXML2_SRCS)))
