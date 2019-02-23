CXX = g++
CXXFLAGS = -pthread -std=c++11
LDFLAGS = -pthread
SOURCE = $(wildcard *.cpp)
OBJECTS = $(SOURCE:.cpp=.o)
TARGET = parcount

default: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

parcount: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET)

load: parcount
	@./$(TARGET)