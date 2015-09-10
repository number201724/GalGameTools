#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <windows.h>
#include <vector>
using namespace std;

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

enum g00_type_info
{
	type_24bit,
	type_8bit,
	type_dir
};
#pragma pack(1)
typedef struct g00_header_s
{
	uint8_t type;
	uint16_t width;
	uint16_t height;
}g00_header_t;
typedef struct g00_24bit_header_s
{
	uint32_t compress_length;
	uint32_t decompress_length;
}g00_24bit_header_t;
typedef struct g02_info_s{
	uint32_t orig_x;			// 原点在屏幕中的位置
	uint32_t orig_y;
	uint32_t end_x;			// 终点在屏幕中的位置
	uint32_t end_y;
	uint32_t unknown[2];		// 0
}g02_info_t;
#pragma pack()


uint32_t get_g02_index_entries(g00_header_t* pheader)
{
	uint8_t * pbuf = (uint8_t*)pheader;

	pbuf += sizeof(g00_header_t); 
	return *(uint32_t*)pbuf;
}
////24bit lzss
//void lzss_decompress_type0(unsigned char* src,unsigned char* dst,unsigned char* dst_end)
//{
//	__asm
//	{
//		mov esi,src
//		mov edi,dst
//		xor edx,edx
//		cld
//Loop1:
//		mov dl,byte ptr [esi]
//		inc esi
//		mov dh,0x8
//Loop2:
//		cmp edi,dst_end
//		je End
//		test dl,1
//		je DecompTag
//		movsw
//		movsb
//		mov byte ptr[edi],0xFF
//		inc edi
//		jmp DecompTag2
//DecompTag:
//		xor eax,eax
//		lods word ptr [esi]
//		mov ecx,eax
//		shr eax,4
//		shl eax,2
//
//		and ecx,0xF
//		add ecx,0x1
//		mov ebx,esi
//		mov esi,edi
//		sub esi,eax
//		rep movsb
//		mov esi,ebx
//DecompTag2:
//		shr dl,1
//		dec dh
//		jnz Loop2
//		jmp Loop1
//End:
//	}
//}

void lzss_decompress(unsigned char* src,unsigned char* dst,unsigned char* dst_end)
{
	__asm
	{
		mov esi,src
		mov edi,dst
		xor edx,edx
		cld
Loop1:
		mov dl,byte ptr [esi]
		inc esi
		mov dh,0x8
Loop2:
		cmp edi,dst_end
		je End
		test dl,1
		je DecompTag
		movsb
		jmp DecompTag2
DecompTag:
		xor eax,eax
		lods word ptr [esi]
		mov ecx,eax
		shr eax,4
		and ecx,0xF
		add ecx,0x2
		mov ebx,esi
		mov esi,edi
		sub esi,eax
		rep movsb
		mov esi,ebx
DecompTag2:
		shr dl,1
		dec dh
		jnz Loop2
		jmp Loop1
End:
	}
}
//D0
typedef struct g02_part_info_s
{
	uint16_t type;					// type 0 1 2					//0
	uint16_t block_count;			// g02_block_info_t count     //2
	uint32_t hs_orig_x;				//hot part x        //4
	uint32_t hs_orig_y;				//hot part y			//8
	uint32_t width;					//pic width						//C
	uint32_t height;				//pic height					//10
	uint32_t screen_show_x;											//14
	uint32_t screen_show_y;											//18
	uint32_t full_part_width;		//0x101							//1C
	uint32_t full_part_height;		//0x30							//20
	uint32_t reserved[20];			//unknown						..
}g02_part_info_t;

typedef struct g02_block_info_s
{
	uint16_t orig_x; //block start x
	uint16_t orig_y;  //block start y
	uint16_t info;		//0 = center 
	uint16_t width;
	uint16_t height;
	uint16_t reserved[41];//zero?
}g02_block_info_t;

typedef struct g02_pair_s
{
	uint32_t offset;
	uint32_t length;
}g02_pair_t;

typedef struct lzss_compress_head_s
{
	uint32_t compress_length;
	uint32_t decompress_length;
}lzss_compress_head_t;




BOOL ScreenSaveFile(const char *file, int width, int height, void *data)
{
	int nAlignWidth = (width*32+31)/32;
	FILE *pfile;
	

	BITMAPFILEHEADER Header;
	BITMAPINFOHEADER HeaderInfo;
	Header.bfType = 0x4D42;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) ;
	Header.bfSize =(DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nAlignWidth* height * 4);
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
		return FALSE;
	}
	

	fwrite(&Header, 1, sizeof(BITMAPFILEHEADER), pfile);
	fwrite(&HeaderInfo, 1, sizeof(BITMAPINFOHEADER), pfile);
	fwrite(data, 1, HeaderInfo.biSizeImage, pfile);
	fclose(pfile);

	return TRUE;
}

void g00_decode_buffer()
{

}
typedef struct g00_extract_imginfo_s
{
	uint32_t type;
	uint32_t full_part_width;
	uint32_t full_part_height;
	uint32_t screen_show_x;
	uint32_t screen_show_y;
	uint32_t hs_orig_x;
	uint32_t hs_orig_y;
	uint32_t part_width;
	uint32_t part_height;
}g00_extract_imginfo_t;
typedef struct g00_extract_info_s
{
	uint32_t type;
	uint32_t orig_x;
	uint32_t orig_y;
	uint32_t width;
	uint32_t height;
	uint32_t info;
}g00_extract_info_t;
//void part_block_to_extract_info(g02_block_info_t* block,g00_extract_info_t* extract)
//{
//	g00_extract_info_t->
//}

void part_extract_buf(g00_extract_info_t* info,uint32_t width_length,uint8_t* src,uint8_t* dst)
{
	switch(info->type)
	{
	case 2:
		{
			//32位的位图,所以*4
			uint32_t hotpart_width_length = (info->width * 4); 
			uint32_t line_length = width_length - hotpart_width_length;
			
			for(uint32_t n=0;n<info->height;n++)
			{
				memcpy(dst,src,hotpart_width_length);
				src += hotpart_width_length;
				dst += width_length;
			}
			break;
		}
	}
}
void extract_g02_part(g02_part_info_t* part_info,uint8_t** out_buf,uint32_t* out_len)
{
	uint8_t* buf = (uint8_t*)part_info;
	uint8_t* dib_buf;
	uint32_t dib_length;
	buf += sizeof(g02_part_info_t);

	//extract info
	g00_extract_imginfo_t img_info;
	g00_extract_info_t info;

	
	switch(part_info->type)
	{
	case 0:
		{

			break;
		}
	case 1:
		{
			
			img_info.type = 2;
			img_info.full_part_width = part_info->full_part_width;
			img_info.full_part_height = part_info->full_part_height;
			img_info.screen_show_x = part_info->screen_show_x;
			img_info.screen_show_y = part_info->screen_show_y;
			img_info.hs_orig_x = part_info->hs_orig_x;
			img_info.hs_orig_y = part_info->hs_orig_y;
			img_info.part_width = part_info->width + part_info->hs_orig_x;
			img_info.part_height = part_info->height + part_info->hs_orig_y;

			dib_length = part_info->width * part_info->height * 4;
			dib_buf = (uint8_t*)malloc(dib_length);
			memset(dib_buf,0,dib_length);

			//hot part
			for(uint32_t n=0;n<part_info->block_count;n++)
			{
				g02_block_info_t* block = (g02_block_info_t*)buf;
				info.type = img_info.type;
				info.orig_x = block->orig_x;
				info.orig_y = block->orig_y;
				info.width = block->width;
				info.height = block->height;
				buf += sizeof(g02_block_info_t);
				uint32_t width_length = (part_info->width * 4);
				uint8_t* dst = (dib_buf + ((info.orig_y - img_info.hs_orig_y) * width_length)) + ((info.orig_x - img_info.hs_orig_x) * 4);
				
				part_extract_buf(&info,width_length,buf,dst);
				//热点宽*高*(32/8)
				buf += block->width * block->height * 4; // 32 bit map
				
			}
			*out_buf = dib_buf;
			*out_len = dib_length;
			break;
		}
	}
}
/*
201724  22:56:29
image_info:
ebx = type
ebx+4 = full_part_width
ebx+8 = full_part_height
ebx+c = screen_show_x
ebx+10 = screen_show_y
ebx+14 = hs_orig_x
ebx+18 = hs_orig_y
ebx+1C = width + hs_orig_x
ebx+20 = height + hs_orig_y





part_info:
[eax]=2
eax+4 = orig_x
eax+8 = orig_y
eax+c = width
eax+10 = height
eax+14 = (info == 1)



//(orig_y - hs_orig_y) * 
//orig_x - hs_orig_x


local.41 = hs_orig_x
local.40 = hs_orig_y

orig_y - hs_orig_y
*/
void extract_g00_type2_pic(g00_header_t *pheader)
{
	uint8_t* buf = ((uint8_t*)pheader) + sizeof(g00_header_t);
	uint8_t* debuf;

	
	vector <g02_info_t*> g02_info_list;
	lzss_compress_head_t* compress_info;


	uint32_t index_entries = *(uint32_t*)buf;

	uint32_t debuf_entries;

	buf += sizeof(uint32_t);

	for(uint32_t i=0;i<index_entries;i++)
	{
		g02_info_t* m_info = new g02_info_t;
		memcpy(m_info,buf,sizeof(g02_info_t));
		buf += sizeof(g02_info_t);
		g02_info_list.push_back(m_info);
	}


	compress_info = (lzss_compress_head_t*)buf;

	debuf = new uint8_t[compress_info->decompress_length];

	buf += sizeof(lzss_compress_head_t);
	
	//decompress
	lzss_decompress(buf,debuf,debuf+compress_info->decompress_length);

	debuf_entries = *(uint32_t*)debuf;
	g02_pair_t* entries = (g02_pair_t*)(debuf + 4);

	for(uint32_t i=0;i<debuf_entries;i++)
	{
		uint8_t* img_buf;
		uint32_t img_len;
		g02_part_info_t * g02_part = (g02_part_info_t*)&debuf[entries[i].offset];
		extract_g02_part(g02_part,&img_buf,&img_len);

		ScreenSaveFile("c:\\1.bmp",g02_part->width,g02_part->height,img_buf);

		printf("index:%d width:%d height:%d\n",i,g02_part->width,g02_part->height);
		free(img_buf);
	}
}


int main(int argc, char* argv[])
{
	FILE* f;

	uint8_t* buf;
	uint32_t length;
	g00_header_t* pheader;
	
	f = fopen("H:\\Iris\\ないしょのないしょ！\\_MENU_BTN_SUB.g00","rb");
	fseek(f,0,SEEK_END);
	length = ftell(f);
	fseek(f,0,SEEK_SET);
	buf = (uint8_t*)malloc(length);
	fread(buf,length,1,f);
	fclose(f);

	pheader = (g00_header_t*)buf;
	
	switch(pheader->type)
	{
	case type_24bit:

		break;
	case type_8bit:
		break;
	case type_dir:
		extract_g00_type2_pic(pheader);
		break;
	default:
		break;
	}


	return 0;
}



/*
005ABBE0 <Fixed2_m.extrace_g00>               /$  55             push    ebp                                                                            ;  sub_5ABBE0
005ABBE1                                      |.  8BEC           mov     ebp, esp
005ABBE3                                      |.  0FB601         movzx   eax, byte ptr ds:[ecx]
005ABBE6                                      |.  83EC 08        sub     esp, 0x8
005ABBE9                                      |.  53             push    ebx
005ABBEA                                      |.  8B5D 08        mov     ebx, [arg.1]
005ABBED                                      |.  56             push    esi
005ABBEE                                      |.  57             push    edi
005ABBEF                                      |.  8903           mov     dword ptr ds:[ebx], eax
005ABBF1                                      |.  8D71 01        lea     esi, dword ptr ds:[ecx+0x1]
005ABBF4                                      |.  85C0           test    eax, eax                                                                       ;  Switch (cases 0..2)
005ABBF6                                      |.  75 25          jnz     short <loc_5ABC1D>
005ABBF8                                      |.  8D7B 04        lea     edi, dword ptr ds:[ebx+0x4]                                                    ;  Case 0 of switch 005ABBF4
005ABBFB                                      |.  B8 01000000    mov     eax, 0x1
005ABC00                                      |.  E8 2B050000    call    <sub_5AC130>
005ABC05                                      |.  8B03           mov     eax, dword ptr ds:[ebx]
005ABC07                                      |.  8B0F           mov     ecx, dword ptr ds:[edi]
005ABC09                                      |.  50             push    eax
005ABC0A                                      |.  51             push    ecx
005ABC0B                                      |.  8BC6           mov     eax, esi
005ABC0D                                      |.  E8 EE000000    call    <sub_5ABD00>
005ABC12                                      |.  5F             pop     edi
005ABC13                                      |.  5E             pop     esi
005ABC14                                      |.  B0 01          mov     al, 0x1
005ABC16                                      |.  5B             pop     ebx
005ABC17                                      |.  8BE5           mov     esp, ebp
005ABC19                                      |.  5D             pop     ebp
005ABC1A                                      |.  C2 0400        retn    0x4
005ABC1D <Fixed2_m.loc_5ABC1D>                |>  83F8 01        cmp     eax, 0x1                                                                       ;  loc_5ABC1D
005ABC20                                      |.  75 20          jnz     short <loc_5ABC42>
005ABC22                                      |.  8D7B 04        lea     edi, dword ptr ds:[ebx+0x4]                                                    ;  Case 1 of switch 005ABBF4
005ABC25                                      |.  E8 06050000    call    <sub_5AC130>
005ABC2A                                      |.  8B13           mov     edx, dword ptr ds:[ebx]
005ABC2C                                      |.  8B07           mov     eax, dword ptr ds:[edi]
005ABC2E                                      |.  52             push    edx
005ABC2F                                      |.  50             push    eax
005ABC30                                      |.  8BC6           mov     eax, esi
005ABC32                                      |.  E8 C9000000    call    <sub_5ABD00>
005ABC37                                      |.  5F             pop     edi
005ABC38                                      |.  5E             pop     esi
005ABC39                                      |.  B0 01          mov     al, 0x1
005ABC3B                                      |.  5B             pop     ebx
005ABC3C                                      |.  8BE5           mov     esp, ebp
005ABC3E                                      |.  5D             pop     ebp
005ABC3F                                      |.  C2 0400        retn    0x4
005ABC42 <Fixed2_m.loc_5ABC42>                |>  83F8 02        cmp     eax, 0x2                                                                       ;  loc_5ABC42
005ABC45                                      |.  0F85 A2000000  jnz     <loc_5ABCED>
005ABC4B                                      |.  8B7E 04        mov     edi, dword ptr ds:[esi+0x4]                                                    ;  读取文件count; Case 2 of switch 005ABBF4
005ABC4E                                      |.  8D0C7F         lea     ecx, dword ptr ds:[edi+edi*2]
005ABC51                                      |.  8D5CCE 08      lea     ebx, dword ptr ds:[esi+ecx*8+0x8]
005ABC55                                      |.  33D2           xor     edx, edx
005ABC57                                      |.  8BC3           mov     eax, ebx
005ABC59                                      |.  897D FC        mov     [local.1], edi
005ABC5C                                      |.  E8 2F0C0000    call    <sub_5AC890>
005ABC61                                      |.  8B15 C43CB104  mov     edx, dword ptr ds:[<dword_4B13CC4>]
005ABC67                                      |.  2B15 C03CB104  sub     edx, dword ptr ds:[<dword_4B13CC0>]
005ABC6D                                      |.  83C0 40        add     eax, 0x40
005ABC70                                      |.  3BD0           cmp     edx, eax
005ABC72                                      |.  7D 0A          jge     short <loc_5ABC7E>
005ABC74                                      |.  BE C03CB104    mov     esi, offset <dword_4B13CC0>
005ABC79                                      |.  E8 9248E6FF    call    <sub_410510>
005ABC7E <Fixed2_m.loc_5ABC7E>                |>  8B15 C03CB104  mov     edx, dword ptr ds:[<dword_4B13CC0>]                                            ;  loc_5ABC7E
005ABC84                                      |.  8BC3           mov     eax, ebx
005ABC86                                      |.  E8 050C0000    call    <sub_5AC890>
005ABC8B                                      |.  8B4D 08        mov     ecx, [arg.1]
005ABC8E                                      |.  8B1D C03CB104  mov     ebx, dword ptr ds:[<dword_4B13CC0>]
005ABC94                                      |.  83C1 04        add     ecx, 0x4
005ABC97                                      |.  8BC7           mov     eax, edi
005ABC99                                      |.  8BF9           mov     edi, ecx
005ABC9B                                      |.  894D F8        mov     [local.2], ecx
005ABC9E                                      |.  E8 8D040000    call    <sub_5AC130>
005ABCA3                                      |.  33F6           xor     esi, esi
005ABCA5                                      |.  3975 FC        cmp     [local.1], esi
005ABCA8                                      |.  7E 43          jle     short <loc_5ABCED>
005ABCAA                                      |.  33FF           xor     edi, edi
005ABCAC                                      |.  8D6424 00      lea     esp, dword ptr ss:[esp]
005ABCB0 <Fixed2_m.loc_5ABCB0>                |>  85DB           /test    ebx, ebx                                                                      ;  loc_5ABCB0
005ABCB2                                      |.  74 30          |je      short <loc_5ABCE4>
005ABCB4                                      |.  8B03           |mov     eax, dword ptr ds:[ebx]
005ABCB6                                      |.  85F6           |test    esi, esi
005ABCB8                                      |.  78 2A          |js      short <loc_5ABCE4>
005ABCBA                                      |.  3BF0           |cmp     esi, eax
005ABCBC                                      |.  7D 26          |jge     short <loc_5ABCE4>
005ABCBE                                      |.  8B44F3 04      |mov     eax, dword ptr ds:[ebx+esi*8+0x4]
005ABCC2                                      |.  85C0           |test    eax, eax
005ABCC4                                      |.  74 1E          |je      short <loc_5ABCE4>
005ABCC6                                      |.  837CF3 08 00   |cmp     dword ptr ds:[ebx+esi*8+0x8], 0x0
005ABCCB                                      |.  74 17          |je      short <loc_5ABCE4>
005ABCCD                                      |.  03C3           |add     eax, ebx
005ABCCF                                      |.  74 13          |je      short <loc_5ABCE4>
005ABCD1                                      |.  8B4D 08        |mov     ecx, [arg.1]
005ABCD4                                      |.  8B11           |mov     edx, dword ptr ds:[ecx]
005ABCD6                                      |.  8B4D F8        |mov     ecx, [local.2]
005ABCD9                                      |.  52             |push    edx
005ABCDA                                      |.  8B11           |mov     edx, dword ptr ds:[ecx]
005ABCDC                                      |.  03D7           |add     edx, edi
005ABCDE                                      |.  52             |push    edx
005ABCDF                                      |.  E8 1C000000    |call    <sub_5ABD00>
005ABCE4 <Fixed2_m.loc_5ABCE4>                |>  46             |inc     esi                                                                           ;  loc_5ABCE4
005ABCE5                                      |.  83C7 34        |add     edi, 0x34
005ABCE8                                      |.  3B75 FC        |cmp     esi, [local.1]
005ABCEB                                      |.^ 7C C3          \jl      short <loc_5ABCB0>
005ABCED <Fixed2_m.loc_5ABCED>                |>  5F             pop     edi                                                                            ;  loc_5ABCED; Default case of switch 005ABBF4
005ABCEE                                      |.  5E             pop     esi
005ABCEF                                      |.  B0 01          mov     al, 0x1
005ABCF1                                      |.  5B             pop     ebx
005ABCF2                                      |.  8BE5           mov     esp, ebp
005ABCF4                                      |.  5D             pop     ebp
005ABCF5                                      \.  C2 0400        retn    0x4


*/