#include "arc.h"
#include <fstream>



arc::arc()
{
}


arc::~arc()
{

}
bool arc::exists(I64Integer hash)
{
	return files.find(hash.QuadPart) != files.end();
}
string arc::peek(string filename)
{
	string s;
	I64Integer hash = crc64(filename.c_str());

	if (exists(hash)) {
		return filename;
	}

	s = filename + ".txt";
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}

	s = filename + ".tlg";
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}

	s = "z/" + filename + ".tlg";
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}

	s = "z/" + filename;

	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}

	s = "bg/" + filename;
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}
	s = "bg/" + filename + ".tlg";
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}

	s = "ev/" + filename;
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}
	s = "ev/" + filename + ".tlg";
	hash = crc64(s.c_str());
	if (exists(hash)) {
		return s;
	}

	return "";
}
bool arc::mount(string archive_name)
{
	uint32_t count;
	
	arc_file_node_t node;

	fstream input(archive_name, ios::in | ios::binary);
	if (!input.is_open()) {
		return false;
	}

	input.read((char*)&count, sizeof(count));

	count ^= INDEX_DECODE_KEY;

	for (uint32_t i = 0; i < count; i++)
	{
		input.read((char*)&node, sizeof(node));
		node.offset ^= node.hash.LowPart;
		node.compr_size ^= node.hash.LowPart;
		node.uncompr_size ^= node.hash.LowPart;

		files[node.hash.QuadPart] = node;
	}

	input.close();

	return true;
}