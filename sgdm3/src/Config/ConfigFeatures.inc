## Config/ConfigFeatures.inc -- GNU Makefile snippet
##################################################################
## Take the values in $(ENABLED_FEATURES) and $(DISABLED_FEATURES)
## and write $(OBJDIR)/GEN_ConfigFeatures.h so that we get the
## various #define symbols without using -D compiler flags (which
## don't work on Windows when using a .sln).

$(OBJDIR)/GEN_ConfigFeatures.h:	    ../Config/ConfigFeatures.template
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@rm -f $(OBJDIR)/tmp_CF.h 
	@rm -f $(OBJDIR)/tmp_CF2.h
	@cp ../Config/ConfigFeatures.template $(OBJDIR)/tmp_CF.h
	@chmod 644 $(OBJDIR)/tmp_CF.h
	@for f in $(ENABLED_FEATURES); do \
		echo Enabling feature $$f;	\
		sed "s|//define $$f|#define $$f|" <$(OBJDIR)/tmp_CF.h  >$(OBJDIR)/tmp_CF2.h; \
		cp $(OBJDIR)/tmp_CF2.h $(OBJDIR)/tmp_CF.h; \
	done
	@for f in $(DISABLED_FEATURES); do \
		echo Disabling feature $$f;	\
		sed "s|#define $$f|//define $$f|" <$(OBJDIR)/tmp_CF.h  >$(OBJDIR)/tmp_CF2.h; \
		cp $(OBJDIR)/tmp_CF2.h $(OBJDIR)/tmp_CF.h; \
	done
	@cp $(OBJDIR)/tmp_CF.h $@
	@rm -f $(OBJDIR)/tmp_CF.h
	@rm -f $(OBJDIR)/tmp_CF2.h
##	@echo Final feature config is:
##	@cat $@
	@echo Created $@
