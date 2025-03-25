CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = space_station_game
SRCS = StationCLIgame.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) 