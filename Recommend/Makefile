CC = g++
CFLAGS = -Wall -std=c++14 -Os -pedantic-errors
LDFLAGS = -L ../Shared/ -lclustering
INCLUDE = -I ./src/ -I ../Shared/src/
PROFILE = #-pg --coverage

TARGET = run

OBJECTS = $(patsubst %.cpp, %.o, $(shell find ./src/ -type f -name '*.cpp'))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -lm -o $@ $(PROFILE) $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(INCLUDE) -c $(PROFILE) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGET) $(OBJECTS)
