#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include <Windows.h>

using namespace std;
#pragma pack(1)

typedef struct neko_pack_pair_s
{
	uint32_t hash;		//hash code
	uint32_t size;
}neko_pack_pair_t;

typedef struct neko_pack_header_s
{
	char magic[8];
	neko_pack_pair_t verify;		//parity name
	neko_pack_pair_t files;			//file table
}neko_pack_header_t;

struct neko_pack_record
{
	uint32_t hash;
	uint32_t length;
	uint32_t offset;
};
struct neko_pack_prepare
{
	std::string file;
	unsigned char *data;
	uint32_t length;

	uint32_t hash;
	uint32_t name_hash;
	unsigned char *encrypt;
};
enum BufferType
{
	BufferTypeUnknow,
	BufferTypeText,
	BufferTypeOgg,
	BufferTypePng,
	BufferTypeMng,
	BufferTypeBmp,
};
#pragma pack()
std::vector<neko_pack_record> file_records;
std::vector<std::string> file_lists;
std::map<uint32_t, std::string> hash_to_name_map;
std::map<std::string, uint32_t> name_to_hash_map;
std::vector<neko_pack_prepare> prepare_lists;

BufferType GetBufferType(unsigned char *data)
{
	if (!strncmp((char *)data, "OggS", 4))
	{
		return BufferTypeOgg;
	}
	else if (!strncmp((char *)data, "\x89PNG", 4))
	{
		return BufferTypePng;
	}
	else if (!strncmp((char *)data, "\x8aMNG", 4))
	{
		return BufferTypeMng;
	}
	else if (data[0] == 0xFF && data[1] == 0xFE) {
		return BufferTypeText;
	}
	else if (data[0] == 0x42 && data[1] == 0x4D) {
		return BufferTypeBmp;
	}

	return BufferTypeUnknow;
}

void decrypt(int count, unsigned char *data, uint16_t key[4])
{
	uint16_t ctx_key[4];

	for (int i = 0; i < 4; i++) {
		ctx_key[i] = key[i];
	}

	for (int i = 0; i < count; i++)
	{
		uint16_t *curr = (uint16_t *)&data[i * sizeof(neko_pack_pair_t)];

		for (int j = 0; j < 4; j++) {
			curr[j] ^= ctx_key[j];
			ctx_key[j] += curr[j];
		}
	}
}

void encrypt(int count, unsigned char *data, uint16_t key[4])
{
	uint16_t ctx_key[4];

	for (int i = 0; i < 4; i++) {
		ctx_key[i] = key[i];
	}

	for (int i = 0; i < count; i++)
	{
		uint16_t *curr = (uint16_t *)&data[i * sizeof(neko_pack_pair_t)];

		for (int j = 0; j < 4; j++) {
			uint16_t temp = curr[j];
			curr[j] ^= ctx_key[j];
			ctx_key[j] += temp;
		}
	}
}

uint32_t get_crc(uint32_t a, uint32_t b)
{
	uint32_t v = (b ^ ((b ^ ((b ^ ((b ^ a) + 0x5D588B65)) - 0x359D3E2A)) - 0x70E44324)) + 0x6C078965;
	int shift = ((b ^ ((b ^ a) + 0x5D588B65)) - 0x359D3E2A) >> 27;
	return _rotl(v, shift);
}

void get_key(uint16_t *out, uint32_t key)
{
	uint32_t v2;
	uint32_t v3;
	uint32_t v4;

	v2 = key ^ (key + 0x5D588B65);
	v3 = v2 ^ (key - 0x359D3E2A);
	v4 = v3 ^ (v2 - 0x70E44324);

	((uint32_t*)out)[0] = v4;
	((uint32_t*)out)[1] = v4 ^ (v3 + 0x6C078965);
}

unsigned char hash_table[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x38,0x2F,0x33,0x3C,0x40,0x3B,0x2A,0x2E,0x31,0x30,0x26,0x44,0x35,0x28,0x3E,0x12,
	0x02,0x22,0x06,0x20,0x1A,0x1C,0x0F,0x11,0x18,0x17,0x42,0x2B,0x3A,0x37,0x34,0x0C,
	0x41,0x08,0x1D,0x07,0x15,0x21,0x05,0x1E,0x0A,0x14,0x0E,0x10,0x09,0x27,0x1F,0x0B,
	0x23,0x16,0x0D,0x01,0x25,0x04,0x1B,0x03,0x13,0x24,0x19,0x2D,0x12,0x29,0x32,0x3F,
	0x3D,0x08,0x1D,0x07,0x15,0x21,0x05,0x1E,0x0A,0x14,0x0E,0x10,0x09,0x27,0x1F,0x0B,
	0x23,0x16,0x0D,0x01,0x25,0x04,0x1B,0x03,0x13,0x24,0x19,0x2C,0x39,0x43,0x36,0x00,
	0x4B,0xA9,0xA7,0xAF,0x50,0x52,0x91,0x9F,0x47,0x6B,0x96,0xAB,0x87,0xB5,0x9B,0xBB,
	0x99,0xA4,0xBF,0x5C,0xC6,0x9C,0xC2,0xC4,0xB6,0x4F,0xB8,0xC1,0x85,0xA8,0x51,0x7E,
	0x5F,0x82,0x73,0xC7,0x90,0x4E,0x45,0xA5,0x7A,0x63,0x70,0xB3,0x79,0x83,0x60,0x55,
	0x5B,0x5E,0x68,0xBA,0x53,0xA1,0x67,0x97,0xAC,0x71,0x81,0x59,0x64,0x7C,0x9D,0xBD,
	0x9D,0xBD,0x95,0xA0,0xB2,0xC0,0x6F,0x6A,0x54,0xB9,0x6D,0x88,0x77,0x48,0x5D,0x72,
	0x49,0x93,0x57,0x65,0xBE,0x4A,0x80,0xA2,0x5A,0x98,0xA6,0x62,0x7F,0x84,0x75,0xBC,
	0xAD,0xB1,0x6E,0x76,0x8B,0x9E,0x8C,0x61,0x69,0x8D,0xB4,0x78,0xAA,0xAE,0x8F,0xC3,
	0x58,0xC5,0x74,0xB7,0x8E,0x7D,0x89,0x8A,0x56,0x4D,0x86,0x94,0x9A,0x4C,0x92,0xB0
};

uint32_t hash_string(uint32_t magic, const char *str)
{
	uint32_t result;
	char c;

	c = *str;
	for (result = magic; ; result = 81 * (hash_table[c] ^ result))
	{
		c = *str;

		if (!c)
			break;

		++str;
	}
	return result;
}

uint32_t align_pair(uint32_t size)
{
	return ((size + 0x3F) & 0xFFFFFFC0);
}

unsigned char *decrypt_file(neko_pack_header_t *hdr,int index, uint32_t& length)
{
	unsigned char *data = (unsigned char *)hdr;
	unsigned char *result = NULL;
	neko_pack_pair_t pair;

	//seek
	data = &data[file_records[index].offset];
	memcpy(&pair, data, sizeof(neko_pack_pair_t));
	data += sizeof(neko_pack_pair_t);

	uint32_t hash = get_crc(hdr->verify.hash, file_records[index].length);
	if (hash != pair.hash) {
		cout << "invalid hash" << endl;
		return result;
	}

	uint16_t key[4];
	get_key(key, pair.hash);

	uint32_t allocate_len = align_pair(file_records[index].length);

	result = new unsigned char[allocate_len];
	memcpy(result, data, file_records[index].length);
	int decrypt_count = allocate_len / sizeof(neko_pack_pair_t);

	decrypt(decrypt_count, result, key);

	length = file_records[index].length;
	return result;
}

void load_file_list(neko_pack_header_t *hdr)
{
	uint32_t story_hash = hash_string(hdr->verify.hash,"story.txt");
	unsigned char *story_data;
	uint32_t story_length;

	for (int i = 0; i < file_records.size(); i++)
	{
		if (file_records[i].hash == story_hash)
		{
			story_data = decrypt_file(hdr, i, story_length);
			break;
		}
	}

	if (story_data)
	{
		char *str_story = new char[story_length + 0x1];
		memcpy(str_story, story_data, story_length);
		str_story[story_length] = 0;
		std::set<std::string> unique_set;

		char* pos = str_story;
		while ((pos = (char*)strstr(pos, ".call ")) != NULL)
		{
			pos += (sizeof(".call ") - 1);
			char* next_pos = strstr(pos, "\r\n");
			if (!next_pos) break;
			*next_pos = 0;

			std::string script_name = pos;
			script_name.append(".txt");

			if (unique_set.find(script_name) == unique_set.end())
			{
				unique_set.insert(script_name);
				file_lists.push_back(script_name);
			}

			pos = next_pos + (sizeof("\r\n") - 1);
		}

		delete[] str_story;
		delete[] story_data;
	}

	file_lists.push_back("story.txt");
	file_lists.push_back("start.txt");
	file_lists.push_back("titlelogo.txt");
	file_lists.push_back("title.txt");
	file_lists.push_back("@test.txt");

	for (int i = 0; i < file_lists.size(); i++)
	{
		uint32_t hash = hash_string(hdr->verify.hash, file_lists[i].c_str());
		hash_to_name_map[hash] = file_lists[i];
		name_to_hash_map[file_lists[i]] = hash;
	}
}

void load_files(neko_pack_header_t *hdr, neko_pack_pair_t *files, int count)
{
	uint32_t offset = 0;
	
	//skip headers
	offset += sizeof(neko_pack_header_t);
	offset += hdr->verify.size;
	offset += hdr->files.size;

	for (int i = 1; i < count; i++)
	{
		neko_pack_record record;
		record.hash = files[i].hash;
		record.length = files[i].size;
		record.offset = offset;

		file_records.push_back(record);

		offset += sizeof(neko_pack_pair_t);
		offset += files[i].size;
	}
}

unsigned char *load_file(std::string file, uint32_t &length)
{
	std::streampos pos;
	fstream script(file, ios::binary | ios::in);
	if (!script.is_open()) {
		return 0;
	}

	script.seekg(0, ios::end);
	pos = script.tellg();
	script.seekg(0, ios::beg);

	unsigned char *data = new unsigned char[pos];
	script.read((char*)data, pos);
	length = pos;

	script.close();
	return data;
}


void load_update_files(std::string dir)
{
	HANDLE hFind;
	std::string sfind = dir + "/*.txt";
	WIN32_FIND_DATAA ffd;

	hFind = FindFirstFileA(sfind.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "invalid find" << endl;
		exit(0);
	}

	do
	{
		struct neko_pack_prepare prepare;
		std::string file = dir + "/" + ffd.cFileName;
		uint32_t length;
		unsigned char *data = load_file(file, length);

		if (prepare.data == NULL)
		{
			printf("prepare failed:%s\n", ffd.cFileName);
			getchar();
			exit(0);
		}

		prepare.encrypt = NULL;
		prepare.data = data;
		prepare.file = ffd.cFileName;
		prepare.length = length;
		prepare_lists.push_back(prepare);

		
		printf("prepare:%s\n",ffd.cFileName);
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
}

void prepare_file(neko_pack_header_t *hdr)
{
	uint16_t key[4];

	for (int i = 0; i < prepare_lists.size(); i++)
	{
		uint32_t align = align_pair(prepare_lists[i].length);
		unsigned char *data = new unsigned char[align];
		memcpy(data, prepare_lists[i].data, prepare_lists[i].length);

		prepare_lists[i].hash = get_crc(hdr->verify.hash, prepare_lists[i].length);
		prepare_lists[i].name_hash = hash_string(hdr->verify.hash, prepare_lists[i].file.c_str());
		get_key(key, prepare_lists[i].hash);

		int encrypt_count = align / sizeof(neko_pack_pair_t);
		encrypt(encrypt_count, data, key);

		prepare_lists[i].encrypt = data;
	}
}

int find_prepare(uint32_t hash)
{
	for (int i = 0; i < prepare_lists.size(); i++)
	{
		if (prepare_lists[i].name_hash == hash) {
			return i;
		}
	}
	return -1;
}

bool get_record(uint32_t hash, neko_pack_record &record)
{
	for (int i = 0; i < file_records.size(); i++)
	{
		if (file_records[i].hash == hash)
		{
			record = file_records[i];
			return true;
		}
	}
	return false;
}

void create_package(neko_pack_header_t *hdr, neko_pack_pair_t *files)
{
	neko_pack_pair_t *my_files;

	uint32_t offset = sizeof(neko_pack_header_t);
	unsigned char *pack_buff = (unsigned char *)malloc(sizeof(neko_pack_header_t) + hdr->files.size);
	memcpy(pack_buff, hdr, offset);
	
	unsigned char *org = (unsigned char *)hdr;
	my_files = (neko_pack_pair_t *)&pack_buff[sizeof(neko_pack_header_t)];
	memcpy(my_files, files, hdr->files.size);
	offset += hdr->files.size;

	int file_count = hdr->files.size / sizeof(neko_pack_pair_t);

	for (int i = 0; i < file_count; i++)
	{
		my_files = (neko_pack_pair_t *)&pack_buff[sizeof(neko_pack_header_t)];
		uint32_t hash = my_files[i].hash;
		int index = find_prepare(hash);
		
		neko_pack_record record;
		if (!get_record(hash, record))
		{
			continue;
		}

		if (index == -1)
		{
			uint32_t new_size = offset + sizeof(neko_pack_pair_t) + record.length;
			pack_buff = (unsigned char *)realloc(pack_buff, new_size);
			my_files = (neko_pack_pair_t *)&pack_buff[sizeof(neko_pack_header_t)];

			memcpy(&pack_buff[offset], &org[record.offset], record.length + sizeof(neko_pack_pair_t));
			offset += record.length + sizeof(neko_pack_pair_t);
		}
		else
		{
			neko_pack_prepare prepare = prepare_lists[index];

			uint32_t new_size = offset + sizeof(neko_pack_pair_t) + prepare.length;
			pack_buff = (unsigned char *)realloc(pack_buff, new_size);
			my_files = (neko_pack_pair_t *)&pack_buff[sizeof(neko_pack_header_t)];

			my_files[i].size = prepare.length;

			printf("%d\n", prepare.length != record.length);
			neko_pack_pair_t pair;
			pair.hash = prepare.hash;
			pair.size = prepare.length;

			memcpy(&pack_buff[offset], &pair, sizeof(pair));
			offset += sizeof(pair);

			memcpy(&pack_buff[offset], prepare.encrypt, prepare.length);
			offset += prepare.length;
		}
	}
	
	for (int i = 0; i < file_count; i++)
	{
		std::string name = hash_to_name_map[my_files[i].hash];
		cout << name << endl;
	}

	uint16_t key[4];
	get_key(key, hdr->files.hash);
	encrypt(file_count, (unsigned char*)my_files, key);

	FILE *gg = fopen("C:/data/games/Endless Dungeon/script.dat", "wb");
	fwrite(pack_buff, 1, offset, gg);
	fclose(gg);

}
int main(int argc,char* argv[])
{
	std::streampos pos;
	std::vector<neko_pack_pair_t> pairs;

	fstream script("script.dat", ios::binary | ios::in);
	if (!script.is_open()) {
		cout << "open script failed" << endl;
		return 0;
	}

	script.seekg(0, ios::end);
	pos = script.tellg();
	script.seekg(0, ios::beg);

	unsigned char *data = new unsigned char[pos];
	script.read((char*)data, pos);

	neko_pack_header_t *hdr = (neko_pack_header_t*)data;
	if (strncmp(hdr->magic, "NEKOPACK", 8))
	{
		cout << "Invalid File" << endl;
		return 0;
	}

	//验证crc是否正确
	uint32_t crc = get_crc(hdr->verify.hash, hdr->files.size);
	if (crc != hdr->files.hash) {
		cout << "invalid files crc" << endl;
		return 0;
	}

	int count = hdr->files.size / sizeof(neko_pack_pair_t);
	cout << "file count:" << count << endl;

	uint16_t key[4];
	get_key(key, hdr->files.hash);

	unsigned char *decrypt_data = &data[sizeof(neko_pack_header_t)];
	decrypt(count, decrypt_data, key);

	neko_pack_pair_t *files = (neko_pack_pair_t *)decrypt_data;

	load_files(hdr, files, count);
	load_file_list(hdr);
	load_update_files("C:/data/games/Endless Dungeon/str_merge/output");
	prepare_file(hdr);

	create_package(hdr,files);

	return 0;
}