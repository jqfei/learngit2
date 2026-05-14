#include <stdbool.h>
#include <stdio.h>

#define RING_BUFFER_CAPACITY 6

typedef struct RingBuffer {
    int data[RING_BUFFER_CAPACITY];
    size_t head;
    size_t tail;
} RingBuffer;

static bool ring_buffer_push(RingBuffer *buffer, int value) {
    size_t next_tail = (buffer->tail + 1) % RING_BUFFER_CAPACITY;

    if (next_tail == buffer->head) {
        return false;
    }

    buffer->data[buffer->tail] = value;
    buffer->tail = next_tail;
    return true;
}

static bool ring_buffer_pop(RingBuffer *buffer, int *value) {
    if (buffer->head == buffer->tail) {
        return false;
    }

    *value = buffer->data[buffer->head];
    buffer->head = (buffer->head + 1) % RING_BUFFER_CAPACITY;
    return true;
}

int main(void) {
    RingBuffer buffer = {0};
    int value = 0;
    size_t index = 0;

    for (int i = 10; i < 15; ++i) {
        if (!ring_buffer_push(&buffer, i)) {
            fprintf(stderr, "push failed for value %d\n", i);
        }
    }

    printf("After filling the ring buffer:\n");
    printf("buffer contents: ");
    index = buffer.head;
    while (index != buffer.tail) {
        printf("%d", buffer.data[index]);
        index = (index + 1) % RING_BUFFER_CAPACITY;
        if (index != buffer.tail) {
            printf(" ");
        }
    }
    printf("\n");

    if (!ring_buffer_push(&buffer, 99)) {
        printf("Ring buffer is full, cannot push 99\n");
    }

    if (ring_buffer_pop(&buffer, &value)) {
        printf("Popped value: %d\n", value);
    }

    if (ring_buffer_pop(&buffer, &value)) {
        printf("Popped value: %d\n", value);
    }

    if (ring_buffer_push(&buffer, 15)) {
        printf("Pushed value: 15\n");
    }

    if (ring_buffer_push(&buffer, 16)) {
        printf("Pushed value: 16\n");
    }

    printf("After wrap-around operations:\n");
    printf("buffer contents: ");
    index = buffer.head;
    while (index != buffer.tail) {
        printf("%d", buffer.data[index]);
        index = (index + 1) % RING_BUFFER_CAPACITY;
        if (index != buffer.tail) {
            printf(" ");
        }
    }
    printf("\n");

    return 0;
}
