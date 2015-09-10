#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

struct match_block_s
{
	wchar_t* string;
	unsigned long string_length;
	unsigned int array_count;
	unsigned int* array_buffer;
};
std::vector <match_block_s> match_array;

FILE* fp;

unsigned int read_int(unsigned char*& position,unsigned int& offset)
{
	unsigned int value;
	value = *(unsigned int *)position;

	offset += sizeof(unsigned int);
	position += sizeof(unsigned int );

	return value;
}


void read_wchar(unsigned char*& position,unsigned int& offset,unsigned int size,wchar_t*& outbuffer)
{
	outbuffer = new wchar_t[size + 1];

	memset(outbuffer,0,sizeof(wchar_t) * (size + 1));

	memcpy(outbuffer,position,size * sizeof(wchar_t));

	offset += size * sizeof(wchar_t);
	position += size * sizeof(wchar_t);
}

void read_int_array(unsigned char*& position,unsigned int& offset,unsigned int count,unsigned int*& outarray)
{
	outarray = new unsigned int [count];

	memcpy(outarray,position,count * sizeof(unsigned int));

	position += count * sizeof(unsigned int);
	offset += count * sizeof(unsigned int);
}


void put_text(unsigned char* top_buffer,size_t top_size,unsigned int offset)
{
	static FILE* txt;
	char section_name[128];
	static int index = 0;

	if(!txt)
	{
		sprintf(section_name,"K:\\Galette\\onikin\\Data\\data3\\csx_match\\output_text.txt");
		txt = fopen(section_name,"wb");
		fwrite("\xFF\xFE",2,1,txt);
	}
	
	fwprintf(txt,L"%08X¡Ñ",offset);
	for(int i=0;i<top_size;i++)
	{
		fprintf(txt, "%c", top_buffer[i]);
	}

	fwprintf(txt, L"\r\n");
	index++;
}


void search_text()
{
	unsigned char find_text_top[] = {02,03,04,02,00,00,00};
	unsigned char find_bytes[] = {0x02,0x00,0x06,0x00,0x00,0x00,0x80};
	FILE* fp_image;
	long f_size;
	unsigned char* f_buf;

	fp_image = fopen("K:\\Galette\\onikin\\Data\\data3\\csx_match\\image","rb");

	if(fp_image)
	{
		fseek(fp,0,SEEK_END);
		f_size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		f_buf = new unsigned char[f_size];

		fread(f_buf,f_size,1,fp);

		fclose(fp);
		
		for(long i=0;i<f_size - sizeof(find_bytes);i++)
		{
			//if(memcmp(&f_buf[i],find_text_top,sizeof(find_text_top))==0)
			//{
			unsigned char* first = &f_buf[i];

			if(memcmp(first,find_bytes,sizeof(find_bytes))==0)
			{
				unsigned int index_of_name = *(unsigned int*)(first + sizeof(find_bytes));
				if(index_of_name < match_array.size() && index_of_name >= 19024)
				{
					int len = wcslen(match_array[index_of_name].string);
					if(match_array[index_of_name].string_length >= 2)
					{
						
						if(!wcsstr(match_array[index_of_name].string,L"\r\n") && 
							match_array[index_of_name].string[0] != L'ÖÐ' &&
							match_array[index_of_name].string[0] != L'´ó')
						{
							put_text((unsigned char*)match_array[index_of_name].string,match_array[index_of_name].string_length * sizeof(wchar_t),i);
						}
					}
						
				}

				/*unsigned int index_of_name = *(unsigned int*)(first + sizeof(find_bytes));
				unsigned char* next = (first + sizeof(find_bytes) + sizeof(unsigned int));
				if(memcmp(next,find_bytes,sizeof(find_bytes))==0)
				{
					unsigned int index_of_text = *(unsigned int*)(next + sizeof(find_bytes));

					if(index_of_name < match_array.size())
					{
						if(match_array[index_of_name].string_length >= 2)
							put_text((unsigned char*)match_array[index_of_name].string,match_array[index_of_name].string_length * sizeof(wchar_t));
					}
					if(index_of_text < match_array.size())
					{
						if(match_array[index_of_name].string_length >= 2)
							put_text((unsigned char*)match_array[index_of_text].string,match_array[index_of_text].string_length * sizeof(wchar_t));
					}
				}*/
			}
			//}
		}
	}
}
//K:\Galette\onikin\Data\data3\csx_match\conststr_match
int main(int argc,char** argv)
{
	long f_size;
	unsigned char* f_buf;

	fp = fopen("K:\\Galette\\onikin\\Data\\data3\\csx_match\\conststr","rb");

	if(!fp)
	{
		printf("can't not open file\n");
		exit(0);
	}
	fseek(fp,0,SEEK_END);
	f_size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	f_buf = new unsigned char[f_size];

	fread(f_buf,f_size,1,fp);

	fclose(fp);

	unsigned int offset = 0;
	unsigned char* position = f_buf;
	unsigned int total_count = read_int(position,offset);

	for(int i=0;i<total_count;i++)
	{
		wchar_t * text_buffer;
		unsigned int text_len = read_int(position,offset);

		read_wchar(position,offset,text_len,text_buffer);

		unsigned int array_count = read_int(position,offset);
		unsigned int* outarray;

		read_int_array(position,offset,array_count,outarray);

		//put_text((unsigned char*)text_buffer,text_len * sizeof(wchar_t));


		match_block_s block;
		block.string = text_buffer;
		block.string_length = text_len;
		block.array_buffer = outarray;
		block.array_count = array_count;

		match_array.push_back(block);
	}

	search_text();

	//printf("%08x %08x",f_size,offset);

	return 0;
}