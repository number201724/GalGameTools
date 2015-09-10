	typedef struct write_stream_s
	{
		unsigned char* buf;
		size_t size;
	}write_stream_t;

	void WS_Init(write_stream_t* stream);
	void WS_WriteBytes(write_stream_t* stream,unsigned char* data,size_t length);
	void WS_WriteByte(write_stream_t* stream,unsigned char data);
	void WS_WriteString(write_stream_t* stream,char* string);
	void WS_Release(write_stream_t* stream);
