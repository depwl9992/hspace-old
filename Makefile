### Makefile for HSpace

.PHONY: all clean

# Change this to one of the following supported types:
#
# PENNMUSH
# TM3 
# MUX
#
# ... that's it for now. ;)
MUSHTYPE=PENNMUSH

# Debug version
CPPFLAGS=-Wall -Wno-comment -D$(MUSHTYPE) -DHSPACE -g

# Release version.
#CPPFLAGS=-D$(MUSHTYPE) -DHSPACE -O2 -DNDEBUG
#
# Don't change anything below this line.
#
INCLUDES=-I. -I../.. -I../../hdrs -Ihsnetwork
SHELL=/bin/sh
OBJDIR=obj
AR=ar
ARFLAGS=rcs
CC=gcc
CPP=cpp

SOURCES=$(wildcard *.cpp)
OBJFILES=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
DEPENDENCIES=$(SOURCES:%.cpp=$(OBJDIR)/%.d)
OUTPUT=libhspace.a
PROJECTS=projhsnetwork hspace

# Used for protoizing
$(OBJDIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.d : %.cpp
	@echo Generating dependency list for $*.cpp...
	@echo -n $(OBJDIR)/ >> $(OBJDIR)/.depend
	@$(CPP) -MM $(INCLUDES) $(notdir $(@:.d=.cpp)) > $@
	@cat $@ >> $(OBJDIR)/.depend

all: $(PROJECTS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

hspace: $(OBJDIR) $(OUTPUT)
	@echo "Done with HSpace."

$(OUTPUT): $(DEPENDENCIES) $(SOURCES) $(OBJFILES)
	$(AR) $(ARFLAGS) $(OUTPUT) $(OBJFILES)

projhsnetwork:
	(cd hsnetwork;make)

-include $(OBJDIR)/.depend

clean:
	find . -name \*.o -exec rm -f {} \;
	find . -name \*.a -exec rm -f {} \;
	find . -name \*.d -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;

distclean:
	find . -name \*.o -exec rm -f {} \;
	find . -name \*.a -exec rm -f {} \;
	find . -name \*.d -exec rm -f {} \;
	find . -name \*~ -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;

