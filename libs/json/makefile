# This is a general makefile template.

                               BIN_DIR = ./bin
                                TARGET = $(BIN_DIR)/libjson.0.5.0.so
                              C_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                MACROS = -D_REENTRANT -D__USE_POSIX
                                    CC = gcc
                            CPP_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -I.
                                  LIBS =
                            OPTIM_FLAG = -O2
                                   CPP = g++
                                LFLAGS = -Wl -fPIC -shared
                              LIB_DIRS =
                                 VPATH = ./src
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
																				 json_reader.cpp  \
																				 json_value.cpp  \
																				 json_writer.cpp

                                CFILES =  

# To use 'make debug=0' build release edition.
ifdef debug
	ifeq ("$(origin debug)", "command line")
		IS_DEBUG = $(debug)
	endif
else
	IS_DEBUG = 1
endif
ifndef IS_DEBUG
	IS_DEBUG = 1
endif
ifeq ($(IS_DEBUG), 1)
	OPTIM_FLAG += -g3
else
	MACROS += -DNDEBUG
endif

# To use 'make quiet=1' all the build command will be hidden.
# To use 'make quiet=0' all the build command will be displayed.
ifdef quiet
	ifeq ("$(origin quiet)", "command line")
		QUIET = $(quiet)
	endif
endif
ifeq ($(QUIET), 1)
	Q = @
else
	Q =
endif

OBJECTS := $(addprefix $(OBJECT_DIR), $(notdir $(CPPFILES:%.cpp=%.o)))
OBJECTS += $(addprefix $(OBJECT_DIR), $(notdir $(CFILES:%.c=%.o)))

CALL_CFLAGS := $(C_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
CPPALL_CFLAGS := $(CPP_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
LFLAGS += $(OPTIM_FLAG) $(LIB_DIRS) $(LIBS)

all: checkdir $(TARGET)

$(TARGET): $(OBJECTS)
	$(Q)$(LINKER) -o $@ $(OBJECTS) $(strip $(LFLAGS))
	@ln -sf $(notdir $(TARGET)) $(BIN_DIR)/libjson.so

$(OBJECT_DIR)%.o:%.cpp
	$(Q)$(CPP) $(strip $(CPPALL_CFLAGS)) -c $< -o $@

$(OBJECT_DIR)%.o:%.c
	$(Q)$(CC) $(strip $(CALL_CFLAGS)) -c $< -o $@

checkdir:
	@if ! [ -d "$(BIN_DIR)" ]; then \
		mkdir $(BIN_DIR) ; \
		fi
	@if ! [ -d "$(OBJECT_DIR)" ]; then \
		mkdir $(OBJECT_DIR); \
		fi
clean:
	$(Q)rm -f $(OBJECTS)
cleanall: clean
	$(Q)rm -f $(TARGET)

.PHONY: all clean cleanall checkdir
