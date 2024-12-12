CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LIBS = -lurcu-qsbr -lpthread

# Select data structure
ifeq ($(DATASTRUCTURE), STACK)
    CFLAGS += -DUSE_STACK
else ifeq ($(DATASTRUCTURE), QUEUE)
    CFLAGS += -DUSE_QUEUE
else
    CFLAGS += -DUSE_LINKED_LIST
endif

# Targets and objects
TARGET = rcu_app
OBJS = main.o linkedlist.o stack.o queue.o

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# Compile main
main.o: main.c linkedlist.h stack.h queue.h
	$(CC) $(CFLAGS) -c -o $@ main.c

# Compile linked list
linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) -c -o $@ linkedlist.c

# Compile stack
stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c -o $@ stack.c

# Compile queue
queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c -o $@ queue.c

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
