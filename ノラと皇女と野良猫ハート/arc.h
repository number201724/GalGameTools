#pragma once
#include <string>
#include <map>

#include "crc64.h"

using namespace std;


#define INDEX_DECODE_KEY 0x26ACA46E

#pragma pack(1)

typedef struct arc_file_node_s
{
	I64Integer hash;
	unsigned char type;
	uint32_t offset;
	uint32_t compr_size;
	uint32_t uncompr_size;
}arc_file_node_t;

#pragma pack()

typedef map<uint64_t, arc_file_node_t> arc_files_t;

class arc
{
public:
	arc();
	~arc();

	bool mount(string archive_name);
	string peek(string filename);
	bool exists(I64Integer hash);
protected:
	arc_files_t files;
};

