CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Isrc
TARGET_BYS = bys
TARGET_BYHASH = by\#

ifeq ($(OS),Windows_NT)
    TARGET_BYS = bys.exe
    TARGET_BYHASH = by\#.exe
    LDFLAGS += -lws2_32
    RM = del /Q /F
else
    RM = rm -f
    UNAME_S := $(shell uname -s 2>/dev/null)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS += -lpthread
    endif
    ifneq ($(PREFIX),)
        BINDIR = $(PREFIX)/bin
    else
        BINDIR = /usr/local/bin
    endif
endif

SRCS = src/main.cpp src/Lexer.cpp src/Parser.cpp src/Evaluator.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET_BYS) $(TARGET_BYHASH)

$(TARGET_BYS): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_BYHASH): $(TARGET_BYS)
	cp $(TARGET_BYS) "$@" 2>/dev/null || copy $(TARGET_BYS) "$@" 2>nul || true

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TARGET_BYS) $(DESTDIR)$(BINDIR)/$(TARGET_BYS)
	cp $(TARGET_BYS) $(DESTDIR)$(BINDIR)/by\#
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET_BYS)
	chmod 755 $(DESTDIR)$(BINDIR)/by\#

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/$(TARGET_BYS)
	$(RM) $(DESTDIR)$(BINDIR)/by\#

clean:
	$(RM) $(OBJS) $(TARGET_BYS) $(TARGET_BYHASH)

.PHONY: all install uninstall clean
