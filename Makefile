CXX := g++
CXXFLAGS := -std=c++17 -Wall -g -pthread

TARGET := main
OBJS := main.o test.o util.o clasestemplate.o pointers.o linkedlist.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
