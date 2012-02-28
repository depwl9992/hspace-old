# Makefile for HSpace

.PHONY: all clean

# Change this to one of the following supported types:
#
# PENNMUSH
# TM3
# MUX 
#
# ... that's it for now. ;)
MUSHTYPE=MUX

# Debug version
CPPFLAGS=-D$(MUSHTYPE) -DHSPACE -g

# Release version.
#CPPFLAGS=-D$(MUSHTYPE) -DHSPACE -O2 -DNDEBUG
#
# Don't change anything below this line.
#
INCLUDES=-I.. -I../hsnetwork
LIBS=-L.. -L../hsnetwork -lhspace -lhsnetwork
SHELL=/bin/sh
OBJDIR=.
AR=ar
ARFLAGS=rcs
CC=g++
CPP=cpp

SOURCES=$(wildcard *.cpp)
OBJFILES=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
DEPENDENCIES=$(SOURCES:%.cpp=$(OBJDIR)/%.d)
OUTPUT=admintest
PROJECTS=admintest

# Used for protoizing
$(OBJDIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS) -w -c $< -o $@

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

admintest:  
	$(CC) $(INCLUDES) -o admintest main.cpp $(LIBS)

-include $(OBJDIR)/.depend

clean:
	find . -name \*.o -exec rm -f {} \;
	find . -name \*.a -exec rm -f {} \;
	find . -name \*.d -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;
