CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2
INCLUDES = -Iinclude
SRCS = src/main.cpp src/DataLoader.cpp src/RoutePlanner.cpp src/RailwaySystem.cpp src/Utils.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = route_planner

.PHONY: all clean run help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

help:
	@echo "Available targets:"
	@echo "  make            Build the project"
	@echo "  make run        Build and run the program"
	@echo "  make clean      Remove object files and binary"
	@echo "  make help       Show this help message"