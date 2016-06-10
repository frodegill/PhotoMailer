#
# Based on Makefile from <URL: http://hak5.org/forums/index.php?showtopic=2077&p=27959 >

PROGRAM = PhotoMailer

all:    $(PROGRAM)
.PHONY: all

DEBUG_INFO = YES
SOURCES = $(shell find . -name '*.cpp')
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = $(OBJECTS:.o=.dep)

######## compiler- and linker settings #########
WX_CONFIG := wx-config
CXX = $(shell $(WX_CONFIG) --cxx)
CXXFLAGS = $(shell $(WX_CONFIG) --cxxflags) $(shell pkg-config libexif --cflags) -W -Wall -Werror -pipe -std=c++11
LIBSFLAGS = $(shell $(WX_CONFIG) --libs std) $(shell pkg-config libexif --libs) -lmimetic
ifdef DEBUG_INFO
 CXXFLAGS += -g
else
 CXXFLAGS += -O
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.dep: %.cpp
	$(CXX) $(CXXFLAGS) -MM $< -MT $(<:.cpp=.o) > $@

############# Main application #################
$(PROGRAM):	$(OBJECTS) $(DEPS)
	$(CXX) -o $@ $(OBJECTS) $(LIBSFLAGS)

################ Dependencies ##################
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS)
endif

################### Clean ######################
clean:
	find . -name '*~' -delete
	-rm -f $(PROGRAM) $(OBJECTS) $(DEPS)
