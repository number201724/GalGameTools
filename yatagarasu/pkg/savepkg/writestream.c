#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "writestream.h"


void WS_Init(write_stream_t* stream)
{
	stream->buf = malloc(0);
	stream->size = 0;
}
void WS_WriteBytes(write_stream_t* stream,unsigned char* data,size_t length)
{
	stream->buf = realloc(stream->buf,stream->size+length);
	memcpy(stream->buf+stream->size,data,length);
	stream->size += length;
}
void WS_WriteByte(write_stream_t* stream,unsigned char data)
{
	stream->buf = realloc(stream->buf,stream->size+1);
	stream->buf[stream->size] = data;
	stream->size += 1;
}
void WS_WriteString(write_stream_t* stream,char* string)
{
	int len;
	
	
	len = strlen(string);
	stream->buf = realloc(stream->buf,stream->size+len+1);
	strcpy(stream->buf+stream->size,string);
	
	stream->size += len+1;
}
void WS_Release(write_stream_t* stream)
{
	free(stream->buf);
	stream->size = 0;
}