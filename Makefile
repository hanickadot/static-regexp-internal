.PHONY:

PROJECT_NAME := static-regexp

SRC_PREFIX := src

TYPE := deploy

include mk/config.mk

APPLICATION_TARGETS := $(wildcard src/mains/*.cpp) $(wildcard src/tests/*.cpp)

SOURCE_FILES := $(shell find . -name *.cpp | cut -c 3-) 
IGNORE := 
INCLUDE_HEADS := -I.
#LIBS += 

include mk/compile.mk

tests: $(OUT_FOLDER)/src/tests/check.cpp.app
	@$(OUT_FOLDER)/src/tests/check.cpp.app

all: tests
