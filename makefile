#Input
MKDIR_P         ?=mkdir -p
COMPILER        ?=g++
TOP             ?=$(shell pwd)
SCALAR          ?= FLOAT
HAVE_TERM       := $(shell echo $$TERM)
VERION_COMPILER := `$(COMPILER) --version`
USE_BLAS        := false
USE_OPENBLAS    := false
DISABLE_BACKPROPAGATION := false
#program name
EXE_NAME ?= DENN

###################### SOURCE FILES #####################
S_DIR  = $(TOP)/source/
S_INC  = $(TOP)/include/


# C++ files
SOURCE_FILES = $(wildcard $(S_DIR)/*.cpp) $(wildcard $(S_DIR)/*/*.cpp)  $(wildcard $(S_DIR)/*/*/*.cpp)
SOURCE_DEBUG_OBJS = $(addprefix $(O_DEBUG_DIR)/,$(notdir $(SOURCE_FILES:.cpp=.o)))
SOURCE_RELEASE_OBJS = $(addprefix $(O_RELEASE_DIR)/,$(notdir $(SOURCE_FILES:.cpp=.o)))

#################### THIRD PARTY #########################
DIPS_INCLUDE = $(TOP)/third_party/Eigen/include/
DIPS_LIBS = -lz

######################## OUTPUTs ########################
ifeq ($(SCALAR),FLOAT)
O_DEBUG_DIR    = $(TOP)/Debug/obj-float
O_RELEASE_DIR  = $(TOP)/Release/obj-float
O_DEBUG_PROG   = $(TOP)/Debug/$(EXE_NAME)-float
O_RELEASE_PROG = $(TOP)/Release/$(EXE_NAME)-float
C_FLAGS		  += -DUSE_FLOAT
else ifeq ($(SCALAR),DOUBLE)
O_DEBUG_DIR    = $(TOP)/Debug/obj-double
O_RELEASE_DIR  = $(TOP)/Release/obj-double
O_DEBUG_PROG   = $(TOP)/Debug/$(EXE_NAME)-double
O_RELEASE_PROG = $(TOP)/Release/$(EXE_NAME)-double
C_FLAGS		  += -DUSE_DOUBLE
else ifeq ($(SCALAR),LONG_DOUBLE)
O_DEBUG_DIR    = $(TOP)/Debug/obj-long-double
O_RELEASE_DIR  = $(TOP)/Release/obj-long-double
O_DEBUG_PROG   = $(TOP)/Debug/$(EXE_NAME)-long-double
O_RELEASE_PROG = $(TOP)/Release/$(EXE_NAME)-long-double
C_FLAGS		  += -DUSE_LONG_DOUBLE
else 
	$(error Set SCALAR=FLOAT|DOUBLE|LONG_DOUBLE)
endif


######################## FLAGS ########################
# C FLAGS
C_FLAGS = -fPIC -D_FORCE_INLINES
# CPP FLAGS
CC_FLAGS = -std=c++14 -I $(DIPS_INCLUDE) -I $(S_INC)
# RELEASE_FLAGS
RELEASE_FLAGS = -O3 -DNDEBUG -fomit-frame-pointer -Wno-unused-result
# DEBUG_FLAGS
DEBUG_FLAGS = -g -D_DEBUG -Wall -Wno-misleading-indentation
# Linker
LDFLAGS += -lm -lutil $(DIPS_LIBS)

##############
# LINUX FLAGS
ifeq ($(shell uname -s),Linux)
# too slow -fopenmp 
C_FLAGS += -pthread 
#clang
ifneq ($(findstring clang,$(VERION_COMPILER)), clang) 
# remove gcc warning (eigen)
C_FLAGS += -Wno-int-in-bool-context
# only linux + gcc
C_FLAGS += -lpthread
endif
endif
##############
# macOS FLAGS
ifeq ($(shell uname -s),Darwin)
#No OpenMP 
RELEASE_FLAGS += -march=native
endif

#################### BACKPROPAGATION ##################
ifeq ($(DISABLE_BACKPROPAGATION),true)
C_FLAGS += -DDISABLE_BACKPROPAGATION 
endif

######################### BLAS ########################
ifeq ($(USE_BLAS),true) # BLAS
RELEASE_FLAGS += -DEIGEN_USE_BLAS #enable blas (eigen)
ifeq ($(shell uname -s),Darwin) #macOS or Linux
LDFLAGS += -framework Accelerate 
else
LDFLAGS += -lblas
endif #END macOS or Linux
endif #END BLAS

####################### OpenBLAS ######################
ifeq ($(USE_OPENBLAS),true) # OpenBLAS
RELEASE_FLAGS += -DEIGEN_USE_BLAS #Enable OpenBLAS (eigen)
ifeq ($(shell uname -s),Darwin) #macOS or Linux
C_FLAGS += -I $(shell brew --prefix openblas)/include
LDFLAGS += -L $(shell brew --prefix openblas)/lib
LDFLAGS += -lblas
else
LDFLAGS += -lopenblas
endif #END macOS or Linux
endif #END OpenBLAS

##################### COLOR OUTPUT ####################
#undef to none (linux)
ifndef HAVE_TERM
	HAVE_TERM = none
endif
#dump to none (macOS)
ifeq ($(HAVE_TERM),dumb)
	HAVE_TERM = none
endif

##
# Support function for colored output
# Args:
#     - $(1) = Color Type
#     - $(2) = String to print
ifneq ($(HAVE_TERM),none)
define colorecho
	@tput setaf $(1)
	@echo $(2)
	@tput sgr0
endef
else
define colorecho
	@echo $(2)
endef
endif

# Color Types
COLOR_BLACK = 0
COLOR_RED = 1
COLOR_GREEN = 2
COLOR_YELLOW = 3
COLOR_BLUE = 4
COLOR_MAGENTA = 5
COLOR_CYAN = 6
COLOR_WHITE = 7

################### COMPILER SECTION ##################
all: directories debug release

directories: ${O_DEBUG_DIR} ${O_RELEASE_DIR}

rebuild: clean directories debug release

rebuild_debug: clean_debug debug

rebuild_release: clean_release release

debug: directories show_debug_flags $(SOURCE_DEBUG_OBJS)
	$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(SOURCE_DEBUG_OBJS) $(LDFLAGS) -o $(O_DEBUG_PROG)

release: directories show_release_flags $(SOURCE_RELEASE_OBJS)
	$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(SOURCE_RELEASE_OBJS) $(LDFLAGS) -o $(O_RELEASE_PROG)

# makedir
${O_DEBUG_DIR}:
	$(call colorecho,$(COLOR_CYAN),"[ Create $(O_DEBUG_DIR) directory ]")
	@${MKDIR_P} ${O_DEBUG_DIR}

# makedir
${O_RELEASE_DIR}:
	$(call colorecho,$(COLOR_CYAN),"[ Create $(O_RELEASE_DIR) directory ]")
	@${MKDIR_P} ${O_RELEASE_DIR}

show_debug_flags:
	$(call colorecho,$(COLOR_YELLOW),"[ Debug flags: $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) ]")

show_release_flags:
	$(call colorecho,$(COLOR_YELLOW),"[ Release flags: $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) ]")

$(O_DEBUG_DIR)/%.o: $(S_DIR)/%.cpp
	$(call colorecho,$(COLOR_GREEN),"[ Make debug object: $(subst $(O_DEBUG_DIR)/,,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@))  ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(O_RELEASE_DIR)/%.o: $(S_DIR)/%.cpp
	$(call colorecho,$(COLOR_GREEN),"[ Make release object: $(subst $(O_RELEASE_DIR)/,,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@))  ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(O_DEBUG_DIR)/%.o: $(S_DIR)/*/%.cpp
	$(call colorecho,$(COLOR_GREEN),"[ Make debug object: $(subst $(O_DEBUG_DIR)/,,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@))  ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(O_RELEASE_DIR)/%.o: $(S_DIR)/*/%.cpp
	$(call colorecho,$(COLOR_GREEN),"[ Make release object: $(subst $(O_RELEASE_DIR)/,,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@))  ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(O_DEBUG_DIR)/%.o: $(S_DIR)/*/*/%.cpp
	$(call colorecho,$(COLOR_GREEN),"[ Make debug object: $(subst $(O_DEBUG_DIR)/,,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@))  ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(O_RELEASE_DIR)/%.o: $(S_DIR)/*/*/%.cpp
	$(call colorecho,$(COLOR_GREEN),"[ Make release object: $(subst $(O_RELEASE_DIR)/,,$(@:.o=.cpp)) => $(subst $(TOP)/,,$(@))  ]")
	@$(COMPILER) $(C_FLAGS) $(CC_FLAGS) $(RELEASE_FLAGS) -c $< -o $@
# Clean
clean: clean_debug clean_release

clean_debug:
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete debug obj files ]")
	@rm -f -R $(O_DEBUG_DIR)
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete debug executable files ]")
	@rm -f $(O_DEBUG_PROG)
	
clean_release:
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete release obj files ]")
	@rm -f -R $(O_RELEASE_DIR)
	$(call colorecho,$(COLOR_MAGENTA),"[ Delete release executable files ]")
	@rm -f $(O_RELEASE_PROG)