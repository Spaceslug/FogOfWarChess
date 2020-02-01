LDFLAGS = -W4 -ldl 

CXX = g++
CXXFLAGS += -std=c++1z -Wall


SRC  = src
SOURCES := $(wildcard $(SRC)*.cpp)
OBJECTS := $(patsubst $(SRC)%.cpp,%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRC)%.cpp,%.d,$(SOURCES))

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: slugchess 

slugchess: field.o gamerules.o slugchess.o 
	$(CXX) $^ $(LDFLAGS) -o $@

slugchesstest: field.o gamerules.o slugchess.o slugchesstest.o
	$(CXX) $^ $(LDFLAGS) -o $@
	
runtest: slugchesstest
	./slugchesstest



-include $(DEPENDS)


clean:
	rm -f *.o  $(DEPENDS) slugchess slugchesstest
	
%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@
