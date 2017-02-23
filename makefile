CC = g++
CFLAGS = -c -Wall -DDEBUG -g
LDFLAGS =
COMMON_SOURCES = helper.cpp
TARGET_SOURCES = server.cpp
TEST_SOURCES = client.cpp
COMMON_OBJECTS = $(COMMON_SOURCES:.cpp=.o)
TARGET_OBJECTS = $(TARGET_SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
EXECUTABLE = server
TEST_EXECUTABLE = client

.PHONY: all server client

all: server client

server: $(EXECUTABLE)

client: $(TEST_EXECUTABLE)

$(EXECUTABLE): $(COMMON_OBJECTS) $(TARGET_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(TEST_EXECUTABLE): $(COMMON_OBJECTS) $(TEST_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
