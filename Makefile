CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17

SRCDIR = src
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
BINDIR = $(BUILDDIR)

SOURCES = $(wildcard $(SRCDIR)/*.cpp)

OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

EXECUTABLE = $(BINDIR)/CheckerOS

# Windows-specific libraries
WIN_LIBS = -lole32 -loleaut32

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(WIN_LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(BUILDDIR)
