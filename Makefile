CXX ?= c++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -I. -I./third_party
TARGET = story_orchestrator
SRCS = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRCS) include/types.hpp include/embedding.hpp include/planner.hpp include/narrative_engine.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) rabbit_tortoise_story.json --vectors --compatibility --compose

clean:
	rm -f $(TARGET) story.txt visualizer_manifest.json

.PHONY: all run clean
