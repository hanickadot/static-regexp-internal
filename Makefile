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

#$(patsubst %,$(OUT_FOLDER)/%,$(CMP_SOURCE_FILES_APPLICATIONS))

tests: $(patsubst %,$(OUT_FOLDER)/%.app,$(wildcard src/tests/*.cpp))
	@$(patsubst %,%&&,$(wildcard $(OUT_FOLDER)/src/tests/*.cpp.app)) true

all: tests
