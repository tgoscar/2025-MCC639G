CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -pthread
LDFLAGS = -pthread

TARGET = main
SRCS = main.cpp \
       test.cpp util.cpp \
       clasestemplate.cpp \
       pointers.cpp \
       ContainersDemo.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
