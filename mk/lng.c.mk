C_SUFFIX := .c
LIB_SUFFIX := .so

override LOCALC_SECONDARY_O_FILES_C += $(addsuffix .obj,$(filter %$(C_SUFFIX),$(O_FILES_SECONDARY_TMP)))
override LOCALC_O_FILES += $(addsuffix .obj,$(filter %$(C_SUFFIX),$(O_FILES_TMP)))
override LOCALC_D_FILES += $(addsuffix .d,$(filter %$(C_SUFFIX),$(D_FILES_TMP)))
override LOCALC_DEP_FILES += $(addsuffix .dep,$(filter %$(C_SUFFIX),$(D_FILES_TMP)))
override LOCALC_APPS += $(addsuffix .app,$(filter %$(C_SUFFIX),$(CMP_SOURCE_FILES_APPLICATIONS_TMP)))
override LOCALC_LIBS += $(addsuffix .lib,$(filter %$(C_SUFFIX),$(CMP_SOURCE_FILES_LIBRARIES_TMP)))

override SECONDARY_O_FILES += $(LOCALC_SECONDARY_O_FILES_C)
override O_FILES += $(LOCALC_O_FILES)
override D_FILES += $(LOCALC_D_FILES)
override XAPPS += $(LOCALC_APPS)
override XLIBS += $(LOCALC_LIBS)

override CXX_O_FILES_PATTERN := $(addprefix $(OBJ_FOLDER)/,$(addsuffix .obj,%$(C_SUFFIX)))

.PRECIOUS: $(LOCALC_O_FILES) $(LOCALC_DEP_FILES) $(LOCALC_APPS) $(LOCALC_LIBS)

$(DEP_FOLDER)/%$(C_SUFFIX).dep: %$(C_SUFFIX) | $(DEP_FOLDER)/.stamp
	@echo "  DEP "$(patsubst $(DEP_FOLDER)/%.dep,%,$@) $(ECHO)
	$(VIS)$(DEPC) -pipe $(DEPCFLAGS) $(XDEPFLAGS) $(patsubst $(DEP_FOLDER)/%.dep,$(OBJ_FOLDER)/%.obj,$@) -c $(patsubst $(DEP_FOLDER)/%.dep,%,$@) -o $@
	$(VIS)cp $@ $(patsubst %.dep,%.d,$@)
	@touch $@

$(OBJ_FOLDER)/%$(C_SUFFIX).obj: $(DEP_FOLDER)/%$(C_SUFFIX).dep | $(OBJ_FOLDER)/.stamp
	@echo "    C "$(patsubst $(OBJ_FOLDER)/%.obj,%,$@) $(ECHO)
	$(VIS)$(DEPC)  -pipe $(CFLAGS) -c $(patsubst $(OBJ_FOLDER)/%.obj,%,$@) -o $@
	@touch $@

$(OUT_FOLDER)/%$(C_SUFFIX).app: $(OBJ_FOLDER)/%$(C_SUFFIX).obj | $(OUT_FOLDER)/.stamp
	@echo " LINK "$(notdir $(basename $(patsubst %.app,%,$@))) $(ECHO)
	$(VIS)$(CXX)  -pipe $(LDFLAGS) $(LD_$(patsubst $(OUT_FOLDER)/%.app,%,$@)) $(SECONDARY_O_FILES) $(patsubst $(OUT_FOLDER)/%.app,$(OBJ_FOLDER)/%.obj,$@) $(LIBS) -o $@ 
	$(VIS)cp $@ $(BLD_FOLDER)/$(notdir $(basename $(patsubst $(OUT_FOLDER)/%.app,%,$@)))$(SUFFIX)
ifeq ($(debug), yes)
ifeq ($(SYSTEM), darwin)
	@echo " DSYM "$(notdir $(basename $(patsubst %.app,%,$@))) $(ECHO)
	$(VIS)dsymutil $@ 
	$(VIS)cp -r $@.dSYM $(BLD_FOLDER)/$(notdir $(basename $(patsubst $(OUT_FOLDER)/%.app,%,$@))).dSYM
endif
endif

$(OUT_FOLDER)/%$(CXX_SUFFIX).lib: $(OBJ_FOLDER)/%$(C_SUFFIX).obj | $(OUT_FOLDER)/.stamp
	@echo " LINK "lib$(notdir $(basename $(patsubst %.lib,%,$@)))$(LIB_SUFFIX) $(ECHO)
	$(VIS)$(CXX)  -pipe $(LDFLAGS) $(LD_$(patsubst $(OUT_FOLDER)/%.lib,%,$@)) $(SECONDARY_O_FILES) $(SLIBFLAGS) $(SONAME)lib$(notdir $(basename $(patsubst %.lib,%,$@)))$(LIB_SUFFIX) $(patsubst $(OUT_FOLDER)/%.lib,$(OBJ_FOLDER)/%.obj,$@) $(LIBS) -o $@ 
	$(VIS)cp $@ $(BLD_FOLDER)/lib$(notdir $(basename $(patsubst $(OUT_FOLDER)/%.lib,%,$@)))$(LIB_SUFFIX)
