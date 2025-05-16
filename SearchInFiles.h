#pragma once

#include <windows.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "file.h"

#define SIF_F_FILE			0b00000001
#define SIF_F_DIRECTORY		0b00000010
#define SIF_F_CASE_IGNORE	0b00000100

#define SIF_SL_END			-1

#define RETURN_ERR			-1
#define RETURN_OK			 1

typedef struct found_phrase_t {
	char* filepath;
	int line;
	int pos_in_line;
	struct found_phrase_t* next; 
} FoundPhrase;

typedef struct search_in_files_t {
	char* filepath;
	char* dirpath;
	char* run_directory;
	char* phrase_to_search;
	int search_level;
	int flags;
	struct found_phrase_t* found_phrases;
} SearchInFiles;

SearchInFiles* SIF_create(char* path, char* phrase, int flags, int level);
void SIF_search(SearchInFiles* sif, int level);
void __SIF_search_in_file(SearchInFiles* sif, char* filename);
int __SIF_add_found(SearchInFiles* sif, File* file, int line, int pos_in_line);
void SIF_print(SearchInFiles* sif);
void SIF_free(SearchInFiles* sif);

