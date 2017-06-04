CC = g++
CFLAGS = -Wall -std=c++14 -Os -pedantic-errors
LDFLAGS =
INCLUDE = -I ./src/
PROFILE = #-pg --coverage

TARGET = libclustering.a

OBJECTS = $(patsubst %.cpp, %.o, $(shell find ./src/ -type f -name '*.cpp'))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $@ $^

%.o: %.cpp
	$(CC) $(INCLUDE) -c $(PROFILE) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGET) $(OBJECTS)
