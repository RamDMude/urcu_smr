CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LIBS = -lurcu-qsbr -lpthread

CFLAGS += -DUSE_LINKED_LIST

# Targets and objects
TARGET = rcu_app
OBJS = main.o linkedlist.o

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# Compile main
main.o: main.c linkedlist.h
	$(CC) $(CFLAGS) -c -o $@ main.c

# Compile linked list
linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) -c -o $@ linkedlist.c


# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
