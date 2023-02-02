# @authors  pintoved (Pinto Veda)
#           cgreenbe (Classie Greenbeard)

#------------------------------------------------#
#   INGREDIENTS                                  #
#------------------------------------------------#
# SRCS      source files
# DIRS      include directories
# FILES     all .cc .h files
#
# BUILD_DIR objects directory
#
# CC        compiler
# CFLAGS    compiler flags
# CPPFLAGS  preprocessor flags

SRCS        := \
	main/hashtable/hash_table.cc \
	main/rb-tree/self_balancing_binary_search_tree.cc \
	main/bp-tree/b_plus_tree.cc
DIRS        := \
	main/common \
	main/hashtable \
	main/rb-tree \
	main/bp-tree

FILES       := $(shell find -regex '.*\(cc\|h\)')

BUILD_DIR   := build

CC          := g++
CFLAGS      := --std=c++17 -O3
CPPFLAGS    := $(DIRS:%=-I %)
AR          := ar
ARFLAGS     := -r -c -s

#------------------------------------------------#
#   UTENSILS                                     #
#------------------------------------------------#
# RM        recursive force remove
# MAKEFLAGS make flags

RM          := rm -r -f
MAKEFLAGS   += #--no-print-directory

#------------------------------------------------#
#   RECIPES                                      #
#------------------------------------------------#
# all       default goal
# clean     remove binary
# tests     run tests
# *.a       build libraries
# research  run research
# linter    run code style check
# cppcheck  run static code analys

all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRCS) main/program.cc main/main.cc \
	-o $(BUILD_DIR)/program.out
	$(BUILD_DIR)/program.out

clean:
	$(RM) $(BUILD_DIR)

tests:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRCS) tests/main.cc \
	-lgtest -o $(BUILD_DIR)/tests.out
	$(BUILD_DIR)/tests.out
	rm storage_export.txt

hash_table.a:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) \
	-c main/hashtable/hash_table.cc
	$(AR) $(ARFLAGS) $(BUILD_DIR)/hash_table.a hash_table.o
	ranlib $(BUILD_DIR)/hash_table.a
	rm hash_table.o

self_balancing_binary_search_tree.a:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) \
	-c main/rb-tree/self_balancing_binary_search_tree.cc
	$(AR) $(ARFLAGS) $(BUILD_DIR)/self_balancing_binary_search_tree.a \
	self_balancing_binary_search_tree.o
	ranlib $(BUILD_DIR)/self_balancing_binary_search_tree.a
	rm self_balancing_binary_search_tree.o

b_plus_tree.a:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c main/bp-tree/b_plus_tree.cc
	$(AR) $(ARFLAGS) $(BUILD_DIR)/b_plus_tree.a b_plus_tree.o
	ranlib $(BUILD_DIR)/b_plus_tree.a
	rm b_plus_tree.o

research:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRCS) main/research.cc \
	-o $(BUILD_DIR)/research.out
	$(BUILD_DIR)/research.out

linter:
	clang-format -n -style=google $(FILES)

cppcheck:
	cppcheck --language=c++ --std=c++17 \
	--enable=all \
	--suppress=missingInclude \
	--suppress=useStlAlgorithm \
	$(FILES)

#------------------------------------------------#
#   SPEC                                         #
#------------------------------------------------#

.PHONY: tests cppcheck hash_table.a b_plus_tree.a \
	self_balancing_binary_search_tree.a
.SILENT:
