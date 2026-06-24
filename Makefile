CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
INCLUDES = -Iinclude
SRCS = src/main.cpp src/DataLoader.cpp src/RoutePlanner.cpp src/RailwaySystem.cpp src/Utils.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = route_planner

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)