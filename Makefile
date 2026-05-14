CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pthread
TARGETS = reverse_list ring_buffer producer_consumer

all: $(TARGETS)

reverse_list: reverse_list.c
	$(CC) $(CFLAGS) -o reverse_list reverse_list.c

ring_buffer: ring_buffer.c
	$(CC) $(CFLAGS) -o ring_buffer ring_buffer.c

producer_consumer: producer_consumer.cpp
	$(CXX) $(CXXFLAGS) -o producer_consumer producer_consumer.cpp

clean:
	rm -f $(TARGETS)

.PHONY: all clean
