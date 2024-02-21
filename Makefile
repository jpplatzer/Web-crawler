#
# 'make depend' uses makedepend to automatically generate dependencies
# (dependencies are added to end of Makefile)
# 'make' build executable files
# 'make clean' removes all .o and executable files
#

# Default C compiler
C = gcc

# Default C++ compiler
CXX = g++

# C++ ARM compiler
# CXX = arm-linux-gnueabihf-g++

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
# CFLAGS  = -Wall -Os
CFLAGS  = -O2 -Wall
CPPFLAGS  = -O2 -Wall -std=c++20

# Remove
RM = rm

# define any directories containing header files other than /usr/include
#
# INCLUDES = -I/home/example/include  -I../include
INCLUDES = -I. -I./include
SYSINCLUDES = -I/usr/include/x86_64-linux-gnu/

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L/usr/lib/x86_64-linux-gnu/

# https://www.gnu.org/software/make/manual/html_node/File-Name-Functions.html

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
# LIBS = -lcurl -lssl -lcrypto 
# UTEST_LIBS = -lgtest -lpthread -lcurl -lssl -lcrypto 
LIBS = -lcurl

# build binaries in a BIN directory
BINDIR=bin/

MAIN_SRC = main.cpp web_crawler.cpp web_page_reader.cpp url_mgr.cpp

# define the CPP object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
# https://stackoverflow.com/questions/5173611/prepending-a-path-on-make
# MAIN_OBJS = $(MAIN_SRC:%.c=$(BINDIR)%.o) $(SRC_CMN:%.c=$(BINDIR)%.o)
# MAIN_OBJS = $(MAIN_SRC:%.c=$(BINDIR)%.o)
MAIN_OBJS = $(MAIN_SRC:%.cpp=$(BINDIR)%.o)

# define the executable file
MAIN = web-crawler
BIN_MAIN=$(addprefix $(BINDIR),$(MAIN))

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all: pre-build $(MAIN)
	@echo  The test has been compiled

pre-build: 
	mkdir -p $(BINDIR)

$(MAIN): $(BIN_MAIN)
	@echo  Built the test

$(BIN_MAIN): $(MAIN_OBJS)
	$(CXX) $(CPPFLAGS) -o $(BIN_MAIN) $(MAIN_OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
$(BINDIR)%.o: %.c
	$(C) $(CFLAGS) $(INCLUDES) $(SYSINCLUDES) -c $<  -o $@

$(BINDIR)%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(SYSINCLUDES) -c $<  -o $@

clean:
	$(RM) $(BINDIR)*.o $(BIN_MAIN)

depend: $(MAIN_SRC)
	makedepend $(INCLUDES) $^


# DO NOT DELETE THIS LINE -- make depend needs it
main.o: ./url_mgr.h ./web_common.h ./web_crawler.h ./include/thread_pool.h
web_crawler.o: ./web_crawler.h ./include/thread_pool.h ./web_common.h
web_crawler.o: ./url_mgr.h ./web_page_reader.h
web_page_reader.o: ./web_page_reader.h ./web_common.h
web_page_reader.o: ./include/common_macros.h
url_mgr.o: ./url_mgr.h ./web_common.h
