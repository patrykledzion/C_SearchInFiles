#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct file_t {
	char* filename;
	FILE* pointer;
	int curr_line;
	int curr_pos_in_line;
} File;

File* File_open(const char* filename);
char File_getc(File* file);
void File_free(File* f);