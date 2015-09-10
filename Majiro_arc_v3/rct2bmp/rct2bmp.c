#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
//#include <Windows.h>

#pragma pack(1)

struct RCTHDR
{
	unsigned int signature;
	char magic[4];		//TC00 TC01
	int width;
	int height;
	int length;	//数据长度
};

typedef struct tagBITMAPFILEHEADER {
        unsigned short    bfType;
        unsigned long   bfSize;
        unsigned short    bfReserved1;
        unsigned short    bfReserved2;
        unsigned long   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
        unsigned long      biSize;
        unsigned long       biWidth;
        unsigned long       biHeight;
        unsigned short       biPlanes;
        unsigned short       biBitCount;
        unsigned long      biCompression;
        unsigned long      biSizeImage;
        unsigned long       biXPelsPerMeter;
        unsigned long       biYPelsPerMeter;
        unsigned long      biClrUsed;
        unsigned long      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()

unsigned int get_file_size(FILE* fd)
{
	unsigned int now_offset;
	unsigned int file_size;
	now_offset = ftell(fd);


	fseek(fd,0,SEEK_END);

	file_size = ftell(fd);

	fseek(fd,now_offset,SEEK_SET);

	return file_size;
}

void reverse_dib(unsigned char* src,unsigned char* dst,int height,int width)
{
	unsigned int k =0;
	unsigned char* copy_src;
	unsigned char* copy_to;
	int pic_width_line_size = width * 4;
	//widthlen * nHeight
	for(k = 0; k < height ; k++)
	{
		copy_to = &dst[(height - k - 1) * pic_width_line_size];
		copy_src = &src[pic_width_line_size * k];
		memcpy(copy_to,copy_src,pic_width_line_size);
	}
}

int save_rgba_to_bmp(const char *file, int width, int height, void *data)
{
	FILE *pfile;

	int nAlignWidth = (width*32+31)/32;
	
	unsigned char* tmp_data = malloc(4 * nAlignWidth * height);

	BITMAPFILEHEADER Header;
	BITMAPINFOHEADER HeaderInfo;
	Header.bfType = 0x4D42;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) ;
	Header.bfSize = (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nAlignWidth* height * 4);
	HeaderInfo.biSize = sizeof(BITMAPINFOHEADER);
	HeaderInfo.biWidth = width;
	HeaderInfo.biHeight = height;
	HeaderInfo.biPlanes = 1;
	HeaderInfo.biBitCount = 32;
	HeaderInfo.biCompression = 0;
	HeaderInfo.biSizeImage = 4 * nAlignWidth * height;
	HeaderInfo.biXPelsPerMeter = 0;
	HeaderInfo.biYPelsPerMeter = 0;
	HeaderInfo.biClrUsed = 0;
	HeaderInfo.biClrImportant = 0; 
	
	
	if(!(pfile = fopen(file, "wb+")))
	{
		return 0;
	}
	

	reverse_dib((unsigned char*)data,tmp_data,height,width);
	

	fwrite(&Header, 1, sizeof(BITMAPFILEHEADER), pfile);
	fwrite(&HeaderInfo, 1, sizeof(BITMAPINFOHEADER), pfile);
	fwrite(tmp_data, 1, HeaderInfo.biSizeImage, pfile);
	fclose(pfile);
	
	free(tmp_data);

	return 1;
}

unsigned int get_bmp_size(int height,int width)
{
	unsigned int bmp_buf_length = 0;
	int align_width = (width*32+31)/32;
	bmp_buf_length = 4 * align_width * height;
	return bmp_buf_length;
}

int fexists(char* path)
{
	FILE* fd;
	int result = -1;
	
	fd = fopen(path,"rb");

	if(fd)
	{
		result = 0;
		fclose(fd);
	}
	return result;
}

//rct lzss uncompress
unsigned int rc8_decompress(unsigned char *uncompr, unsigned int uncomprLen, unsigned char *compr, unsigned int comprLen, unsigned int width)
{
	unsigned int i = 0;
	unsigned int act_uncomprLen = 0;
	unsigned int curByte = 0;
	unsigned int pos[16];

	pos[0] = -1;
	pos[1] = -2;
	pos[2] = -3;
	pos[3] = -4;
	pos[4] = 3 - width;
	pos[5] = 2 - width;
	pos[6] = 1 - width;
	pos[7] = 0 - width;
	pos[8] = -1 - width;
	pos[9] = -2 - width;
	pos[10] = -3 - width;
	pos[11] = 2 - (width * 2);
	pos[12] = 1 - (width * 2);
	pos[13] = (0 - width) << 1;
	pos[14] = -1 - (width * 2);
	pos[15] = (-1 - width) * 2;

	uncompr[act_uncomprLen++] = compr[curByte++];

	while (1) {
		unsigned char flag;
		unsigned int copy_bytes, copy_pos;

		if (curByte >= comprLen)
			break;

		flag = compr[curByte++];

		if (!(flag & 0x80))
		{
			if (flag != 0x7f)
				copy_bytes = flag + 1;
			else
			{
				if (curByte + 1 >= comprLen)
					break;

				copy_bytes = compr[curByte++];
				copy_bytes |= compr[curByte++] << 8;
				copy_bytes += 0x80;
			}

			if (curByte + copy_bytes - 1 >= comprLen)
				break;
			if (act_uncomprLen + copy_bytes - 1 >= uncomprLen)
				break;

			memcpy(&uncompr[act_uncomprLen], &compr[curByte], copy_bytes);
			act_uncomprLen += copy_bytes;
			curByte += copy_bytes;
		}
		else
		{
			copy_bytes = flag & 7;
			copy_pos = (flag >> 3) & 0xf;

			if (copy_bytes != 7)
				copy_bytes += 3;
			else {
				if (curByte + 1 >= comprLen)
					break;

				copy_bytes = compr[curByte++];
				copy_bytes |= compr[curByte++] << 8;
				copy_bytes += 0xa;
			}

			for (i = 0; i < copy_bytes; i++) {
				if (act_uncomprLen >= uncomprLen)
					break;
				uncompr[act_uncomprLen] = uncompr[act_uncomprLen + pos[copy_pos]];
				act_uncomprLen++;
			}
		}
	}

	//	if (curByte != comprLen)
	//		fprintf(stderr, "compr miss-match %d VS %d\n", curByte, comprLen);

	return act_uncomprLen;
}

unsigned int rct_decompress(unsigned char *uncompr, unsigned int uncomprLen, unsigned char *compr, unsigned int comprLen, unsigned int width)
{
	unsigned int i = 0;
	unsigned int act_uncomprLen = 0;
	unsigned int curByte = 0;
	unsigned int pos[32];

	pos[0] = -3;
	pos[1] = -6;
	pos[2] = -9;
	pos[3] = -12;
	pos[4] = -15;
	pos[5] = -18;
	pos[6] = (3 - width) * 3;
	pos[7] = (2 - width) * 3;
	pos[8] = (1 - width) * 3;
	pos[9] = (0 - width) * 3;
	pos[10] = (-1 - width) * 3;
	pos[11] = (-2 - width) * 3;
	pos[12] = (-3 - width) * 3;
	pos[13] = 9 - ((width * 3) << 1);
	pos[14] = 6 - ((width * 3) << 1);
	pos[15] = 3 - ((width * 3) << 1);
	pos[16] = 0 - ((width * 3) << 1);
	pos[17] = -3 - ((width * 3) << 1);
	pos[18] = -6 - ((width * 3) << 1);
	pos[19] = -9 - ((width * 3) << 1);
	pos[20] = 9 - width * 9;
	pos[21] = 6 - width * 9;
	pos[22] = 3 - width * 9;
	pos[23] = 0 - width * 9;
	pos[24] = -3 - width * 9;
	pos[25] = -6 - width * 9;
	pos[26] = -9 - width * 9;
	pos[27] = 6 - ((width * 3) << 2);
	pos[28] = 3 - ((width * 3) << 2);
	pos[29] = 0 - ((width * 3) << 2);
	pos[30] = -3 - ((width * 3) << 2);
	pos[31] = -6 - ((width * 3) << 2);

	uncompr[act_uncomprLen++] = compr[curByte++];
	uncompr[act_uncomprLen++] = compr[curByte++];
	uncompr[act_uncomprLen++] = compr[curByte++];

	while (1) {
		unsigned char flag;
		unsigned int copy_bytes, copy_pos;

		if (curByte >= comprLen)
			break;

		flag = compr[curByte++];

		if (!(flag & 0x80)) {
			if (flag != 0x7f)
				copy_bytes = flag * 3 + 3;
			else {
				if (curByte + 1 >= comprLen)
					break;

				copy_bytes = compr[curByte++];
				copy_bytes |= compr[curByte++] << 8;
				copy_bytes += 0x80;
				copy_bytes *= 3;
			}

			if (curByte + copy_bytes - 1 >= comprLen)
				break;
			if (act_uncomprLen + copy_bytes - 1 >= uncomprLen)
				break;

			memcpy(&uncompr[act_uncomprLen], &compr[curByte], copy_bytes);
			act_uncomprLen += copy_bytes;
			curByte += copy_bytes;
		}
		else {
			copy_bytes = flag & 3;
			copy_pos = (flag >> 2) & 0x1f;

			if (copy_bytes != 3) {
				copy_bytes = copy_bytes * 3 + 3;
			}
			else {
				if (curByte + 1 >= comprLen)
					break;

				copy_bytes = compr[curByte++];
				copy_bytes |= compr[curByte++] << 8;
				copy_bytes += 4;
				copy_bytes *= 3;
			}

			for (i = 0; i < copy_bytes; i++) {
				if (act_uncomprLen >= uncomprLen)
					goto out;
				uncompr[act_uncomprLen] = uncompr[act_uncomprLen + pos[copy_pos]];
				act_uncomprLen++;
			}
		}
	}
out:
	//	if (curByte != comprLen)
	//		fprintf(stderr, "compr miss-match %d VS %d\n", curByte, comprLen);

	return act_uncomprLen;
}



void convert_to_32bit_dib(int width,int height,unsigned char* in_buf,unsigned int in_length,unsigned char** out_buf,unsigned int* out_length)
{
	unsigned int new_length = width * height * 4;
	unsigned char* new_buf;
	unsigned int i = 0;
	unsigned int k = 0;

	*out_length = new_length;

	new_buf = (unsigned char*)malloc(new_length);

	memset(new_buf,0,new_length);

	for(i = 0;i<height;i++)
	{
		for(k = 0; k < width; k++)
		{
			new_buf[i * width * 4 + k * 4]     = in_buf[i * width * 3 + k * 3 + 2];
			new_buf[i * width * 4 + k * 4 + 1] = in_buf[i * width * 3 + k * 3 + 1];
			new_buf[i * width * 4 + k * 4 + 2] = in_buf[i * width * 3 + k * 3];
			new_buf[i * width * 4 + k * 4 + 3] = 0;			//alpha 填充纯色
		}
	}

	save_rgba_to_bmp("c:\\1.bmp",width,height,new_buf);
}

void uncompress_tc0_image(struct RCTHDR * hdr,unsigned char* buf,unsigned int length)
{
	unsigned char* compress_data;
	unsigned char* uncompress_picture;
	unsigned int uncompress_length;
	unsigned char* picture_data;
	unsigned int picture_length;
	unsigned int uncompress_size;


	compress_data = (unsigned char*)malloc(hdr->length);
	memcpy(compress_data,&buf[sizeof(struct RCTHDR)],hdr->length);

	uncompress_length = hdr->width * hdr->height * 3;
	uncompress_picture = (unsigned char*)malloc(uncompress_length);

	uncompress_size = rct_decompress(uncompress_picture,uncompress_length,compress_data,hdr->length,hdr->width);

	if(uncompress_size != uncompress_length)
	{
		printf("error\n");
	}

	convert_to_32bit_dib(hdr->width,hdr->height,uncompress_picture,uncompress_length,&picture_data,&picture_length);
}

int main(int argc, char* argv[])
{
	FILE* fd;
	struct RCTHDR * hdr;
	unsigned char* buf;
	unsigned int file_size;


	fd = fopen("K:\\しゃんぐりら\\[対異形生贄神官ティアナ][Setup]\\bg01.rct","rb");

	if(!fd)
	{
		printf("file open failed\n");
		exit(0);
	}

	file_size = get_file_size(fd);
	buf = (unsigned char*)malloc(file_size);	
	fread(buf,file_size,1,fd);
	fclose(fd);


	hdr = (struct RCTHDR *)buf;


	if(hdr->signature != 0x9A925A98)
	{
		printf("invalid rct signature\n");
		exit(0);
	}


	if(strnicmp(hdr->magic,"TC00",4)==0)
	{
		uncompress_tc0_image(hdr,buf,file_size);
	}

	if(strnicmp(hdr->magic,"TS00",4)==0)
	{
		
	}

	return 0;
}

