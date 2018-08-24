

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"



#define min(a, b) (a)<(b)?(a):(b)

/*
struct RingBuffer{
    size_t rb_capacity;
    char  *rcursor;
    char  *wcursor;
    char  *rb_buff;
};
*/




RingBuffer *CreateRingBuffer(int size){
    RingBuffer *rb;
    rb=(RingBuffer *)malloc(sizeof(RingBuffer)+size);
    if (rb == NULL) return NULL;
    rb->rb_capacity=size;
    //指向数据部分
    rb->rb_buffer=(char *)rb+sizeof(RingBuffer);
    rb->rcursor=rb->rb_buffer;
    rb->wcursor=rb->rb_buffer;
    return rb;
}

/*
struct RingBuffer* rb_new(size_t capacity)
{
    struct RingBuffer *rb = (struct RingBuffer *) mymalloc(sizeof(struct RingBuffer) + capacity);
    if (rb == NULL) return NULL;
    
    rb->rb_capacity = capacity;
    rb->rb_buff     = (char*)rb + sizeof(struct RingBuffer);
    rb->rcursor     = rb->rb_buff;
    rb->wcursor     = rb->rb_buff;

	return rb;
};
*/
void rb_reset( RingBuffer *rb)
{
	rb->rb_buffer     = (char*)rb + sizeof(RingBuffer);
	rb->rcursor     = rb->rb_buffer;
	rb->wcursor     = rb->rb_buffer;
}

void  rb_free( RingBuffer *rb)
{
    free((char*)rb);
}

size_t     rb_capacity( RingBuffer *rb)
{
    assert(rb != NULL);
    return rb->rb_capacity;
}
size_t     rb_can_read( RingBuffer *rb)
{
    assert(rb != NULL);
    if (rb->rcursor == rb->wcursor) return 0;
    if (rb->rcursor < rb->wcursor) return rb->wcursor - rb->rcursor;
    return rb_capacity(rb) - (rb->rcursor - rb->wcursor);
}
size_t     rb_can_write( RingBuffer *rb)
{
    assert(rb != NULL);
	return rb_capacity(rb) - rb_can_read(rb) - 1;
}

size_t     rb_read( RingBuffer *rb, void *data, size_t count)
{
    assert(rb != NULL);
    assert(data != NULL);
    if (rb->rcursor < rb->wcursor)
    {
        int copy_sz = min(count, rb_can_read(rb));
        memcpy(data, rb->rcursor, copy_sz);
        rb->rcursor += copy_sz;
        return copy_sz;
    }
    else
    {
        if (count < rb_capacity(rb)-(rb->rcursor - rb->rb_buffer))
        {
            int copy_sz = count;
            memcpy(data, rb->rcursor, copy_sz);
            rb->rcursor += copy_sz;
            return copy_sz;
        }
        else
        {
            int copy_sz = rb_capacity(rb) - (rb->rcursor - rb->rb_buffer);
            memcpy(data, rb->rcursor, copy_sz);
            rb->rcursor = rb->rb_buffer;
            copy_sz += rb_read(rb, (char*)data+copy_sz, count-copy_sz);
            return copy_sz;
        }
    }
}

size_t     rb_write( RingBuffer *rb, const void *data, size_t count)
{
    assert(rb != NULL);
    assert(data != NULL);
    
	if (count > rb_can_write(rb)) return -1;
    
    if (rb->rcursor <= rb->wcursor)
    {
        int tail_avail_sz = (int)rb_capacity(rb) - (rb->wcursor - rb->rb_buffer);
        if ((int)count <= tail_avail_sz)
        {
            memcpy(rb->wcursor, data, count);
            rb->wcursor += count;
            if (rb->wcursor == rb->rb_buffer+rb_capacity(rb))
                rb->wcursor = rb->rb_buffer;
            return count;
        }
        else
        {
            memcpy(rb->wcursor, data, tail_avail_sz);
            rb->wcursor = rb->rb_buffer;
            
            return tail_avail_sz + rb_write(rb, (char*)data+tail_avail_sz, count-tail_avail_sz);
        }
    }
    else
    {
        memcpy(rb->wcursor, data, count);
        rb->wcursor += count;
        return count;
    }
}

