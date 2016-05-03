#pragma once

#include <stdint.h>

typedef union I64Integer {
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	};
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	} u;
	uint64_t QuadPart;
} I64Integer;


I64Integer crc64(const char *data);
