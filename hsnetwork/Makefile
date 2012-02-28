# Makefile for HSNetwork

.PHONY: all clean

# Debug version
CPPFLAGS=-g

# Release version.
#CPPFLAGS=-O2 -DNDEBUG
#
# Don't change anything below this line.
#
INCLUDES=-I.. -I.
SHELL=/bin/sh
OBJDIR=obj
AR=ar
ARFLAGS=rcs
CC=gcc
CPP=cpp

SOURCES=$(wildcard *.cpp)
OBJFILES=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
DEPENDENCIES=$(SOURCES:%.cpp=$(OBJDIR)/%.d)
OUTPUT=libhsnetwork.a

# Used for protoizing
$(OBJDIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS) -Wall -c $< -o $@

$(OBJDIR)/%.d : %.cpp
	@echo Generating dependency list for $*.cpp...
	@echo -n $(OBJDIR)/ >> $(OBJDIR)/.depend
	@$(CPP) -MM $(INCLUDES) $(notdir $(@:.d=.cpp)) > $@
	@cat $@ >> $(OBJDIR)/.depend

all: hsnetwork

$(OBJDIR):
	@mkdir -p $(OBJDIR)

hsnetwork: $(OBJDIR) $(OUTPUT)
	@echo "Done with HSNetwork."

$(OUTPUT): $(DEPENDENCIES) $(SOURCES) $(OBJFILES)
	$(AR) $(ARFLAGS) $(OUTPUT) $(OBJFILES)

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

