#ifndef _COMPRESS_H_
#define _COMPRESS_H_

extern "C"
{
	unsigned char* WINAPI decompress(unsigned char* Data,size_t Length,size_t* out_length);
	unsigned char* WINAPI compress(unsigned char* Data,size_t Length,size_t* out_length);
	void WINAPI free_buffer(unsigned char* Buffer);
	size_t WINAPI crc(unsigned char* data,size_t length);

	size_t WINAPI length_of_file(char* file_name);
	unsigned char* WINAPI get_file_data(char* file_name,size_t* length);
}

#endif