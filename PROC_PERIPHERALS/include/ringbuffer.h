

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdlib.h>

typedef struct RingBuffer_T{
    void * data;
    char * rcursor; //读指针
    char * wcursor; //写指针
    char * rb_buffer;

    size_t rb_capacity;      //缓冲大小
}RingBuffer;

//struct RingBuffer* rb_new(size_t capacity);
RingBuffer *CreateRingBuffer(int size);
void rb_free( RingBuffer *rb);
void rb_reset( RingBuffer *rb);

size_t rb_capacity( RingBuffer *rb);
size_t rb_can_read( RingBuffer *rb);
size_t rb_can_write( RingBuffer *rb);

size_t rb_read( RingBuffer *rb, void *data, size_t count);
size_t rb_write( RingBuffer *rb, const void *data, size_t count);

#endif
