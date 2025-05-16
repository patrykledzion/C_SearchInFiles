#include "SearchInFiles.h"
#include "debug_print.h"

static size_t searched_files = 0;

SearchInFiles* SIF_create(char* path, char* phrase, int flags, int level)
{
	if (path == NULL || phrase == NULL)
	{
		return NULL;
	}

	SearchInFiles* sif_ret = malloc(sizeof(SearchInFiles));

	ASSERT_ALLOC(sif_ret);

	sif_ret->flags = flags;
	sif_ret->dirpath = NULL;
	sif_ret->filepath = NULL;
	sif_ret->search_level = level;

	size_t path_len = strlen(path);

	if ((flags & SIF_F_FILE) && (flags & SIF_F_DIRECTORY))
	{
		return NULL;
	}

	if (flags & SIF_F_FILE)
	{
		sif_ret->filepath = malloc(sizeof(char) * path_len + 1);
		ASSERT_ALLOC(sif_ret->filepath);
		strncpy_s(sif_ret->filepath, path_len + 1, path, path_len + 1);
		sif_ret->filepath[path_len] = '\x0';
	}
	else if (flags & SIF_F_DIRECTORY)
	{
		if (path[path_len - 1] != '/' && path[path_len - 1] != '\\')path_len += 1;
		sif_ret->dirpath = malloc(sizeof(char) * path_len + 1);
		ASSERT_ALLOC(sif_ret->dirpath);
		strncpy_s(sif_ret->dirpath, path_len + 1, path, path_len + 1);
		if (path[path_len - 1] != '/' && path[path_len - 1] != '\\')sif_ret->dirpath[path_len - 1] = '/';
		sif_ret->dirpath[path_len] = '\x0';
	}

	char curr_path[2048];
	GetCurrentDirectoryA(2048, curr_path);
	size_t curr_path_len = strlen(curr_path);
	sif_ret->run_directory = malloc(sizeof(char) * curr_path_len + 1);
	ASSERT_ALLOC(sif_ret->run_directory);

	strncpy_s(sif_ret->run_directory, curr_path_len + 1, curr_path, curr_path_len + 1);
	sif_ret->run_directory[curr_path_len] = '\x0';

	
	sif_ret->found_phrases = NULL;

	size_t phrase_len = strlen(phrase);

	sif_ret->phrase_to_search = malloc(sizeof(char) * phrase_len + 1);
	ASSERT_ALLOC(sif_ret->phrase_to_search);

	strncpy_s(sif_ret->phrase_to_search, phrase_len + 1, phrase, phrase_len + 1);
	sif_ret->phrase_to_search[phrase_len] = '\x0';
	
	return sif_ret;

}

void SIF_search(SearchInFiles* sif, int level)
{
	if (sif == NULL)return;
	if (sif->filepath != NULL)
	{
		char new_path[2048];
		if (sif->run_directory[strlen(sif->run_directory) - 1] == '\\' || sif->run_directory[strlen(sif->run_directory) - 1] == '/')
		{
			sprintf_s(new_path, 2048, "%s%s", sif->run_directory, sif->filepath);
		}
		else {
			sprintf_s(new_path, 2048, "%s/%s", sif->run_directory, sif->filepath);
		}
		__SIF_search_in_file(sif, sif->filepath);
	}
	else if(sif->dirpath!=NULL){
		char new_path[2048];
		if (sif->dirpath[strlen(sif->dirpath) - 1] == '\\' || sif->dirpath[strlen(sif->dirpath) - 1] == '/')
		{ 
			sprintf_s(new_path, 2048, "%s*", sif->dirpath);
		}
		else {
			sprintf_s(new_path, 2048, "%s/*", sif->dirpath);
		}

		WIN32_FIND_DATAA found;
		HANDLE hFind = FindFirstFileA(new_path, &found);
		
		do {
			if (strlen(found.cFileName) >= 260)continue;
			if (strncmp(found.cFileName, ".", 1) == 0 || strncmp(found.cFileName, "..", 2) == 0)continue;
			char filepath[510];
			sprintf_s(filepath, 510, "%s%s", sif->dirpath, found.cFileName);
			int type = (found.dwFileAttributes >> 4) & 0xf;
			// 0x00000010 for directory
			if (type == 1)
			{
				if (level >= sif->search_level && sif->search_level != -1)continue;
				//SIF_search(sif, level + 1);
				char* prev = malloc(sizeof(char) * strlen(sif->dirpath) + 1);
				ASSERT_ALLOC(prev);
				size_t new_len = strlen(sif->dirpath) + strlen(found.cFileName) + 1; 
				strncpy_s(prev, strlen(sif->dirpath) + 1, sif->dirpath, strlen(sif->dirpath) + 1);
				char* temp_path = realloc(sif->dirpath, sizeof(char) * new_len + 1);
				ASSERT_ALLOC(temp_path);
				sif->dirpath = temp_path;
				strncpy_s(sif->dirpath + strlen(prev), strlen(found.cFileName) + 1, found.cFileName, strlen(found.cFileName) + 1);
				sif->dirpath[new_len - 1] = '/';
				sif->dirpath[new_len] = '\x0';
				SIF_search(sif, level + 1);
				temp_path = realloc(sif->dirpath, sizeof(char) * (strlen(prev) + 1));
				ASSERT_ALLOC(temp_path);
				sif->dirpath = temp_path;
				strncpy_s(sif->dirpath, strlen(prev) + 1, prev, strlen(prev) + 1);
				sif->dirpath[strlen(prev)] = '\x0';
				free(prev);
			}	
			// 0x00000020 for file
			else if(type == 2)
				__SIF_search_in_file(sif, filepath);

		} while (FindNextFileA(hFind, &found));
	}
}

void __SIF_search_in_file(SearchInFiles* sif, char* filename)
{
	if (sif==NULL || filename == NULL)return;
	if (sif->phrase_to_search == NULL)return;
	
	char* phrase = sif->phrase_to_search;

	size_t phrase_len = strlen(phrase);
	size_t matched_count = 0;

	File* file = File_open(filename);
	if (file == NULL)return;

	searched_files++;
	if (searched_files % 10000 == 0)printf("Searched in %llu files...\n", searched_files);

	char c = File_getc(file);
	while (c != EOF)
	{
		char _c = sif->flags & SIF_F_CASE_IGNORE ? tolower(c) : c;
		char _phrase_c = sif->flags & SIF_F_CASE_IGNORE ? tolower(phrase[matched_count]) : phrase[matched_count];
		if (_phrase_c == _c)
		{
			matched_count++;
		}
		else {
			matched_count = 0;
		}

		if (matched_count >= phrase_len)
		{
			__SIF_add_found(sif, file, file->curr_line, file->curr_pos_in_line - matched_count);
			matched_count = 0;
		}

		c = File_getc(file);
	}

	File_free(file);
}

int __SIF_add_found(SearchInFiles* sif, File* file, int line, int pos_in_line)
{
	if (sif == NULL || file == NULL)return;
	if (file->filename == NULL)return;
	
	FoundPhrase* phrase;

	phrase = malloc(sizeof(FoundPhrase));
	ASSERT_ALLOC(phrase);

	size_t filename_len = strlen(file->filename);
	phrase->filepath = malloc(sizeof(char) * filename_len + 1);
	ASSERT_ALLOC(phrase->filepath);
	strncpy_s(phrase->filepath, filename_len + 1, file->filename, filename_len + 1);
	phrase->filepath[filename_len] = '\x0';
	phrase->line = line;
	phrase->pos_in_line = pos_in_line;
	phrase->next = NULL;

	FoundPhrase* curr = sif->found_phrases;
	if (curr == NULL)
	{
		sif->found_phrases = phrase;
	}
	else {
		while (curr->next != NULL)
		{
			curr = curr->next;
		}

		curr->next = phrase;
	} 

	return RETURN_OK;
}

void SIF_print(SearchInFiles* sif)
{
	if (sif == NULL)return;
	int count = 0;
	printf("Occurrences of %s:\n", sif->phrase_to_search);
	FoundPhrase* curr = sif->found_phrases;
	while (curr != NULL)
	{
		printf(" %s:%d:%d\n", curr->filepath, curr->line, curr->pos_in_line);
		curr = curr->next;
		count++;
	}

	printf("\nSearched files: %llu\n", searched_files);
	printf("Found %d occurences\n", count);
}

void SIF_free(SearchInFiles* sif)
{
	if (sif == NULL)return;
	if (sif->dirpath != NULL)
		free(sif->dirpath);
	if (sif->filepath != NULL)free(sif->filepath);
	if (sif->found_phrases != NULL)
	{
		FoundPhrase* curr = sif->found_phrases;
		while (curr != NULL)
		{
			FoundPhrase* next = curr->next;
			free(curr);
			curr = next;
			int x = 3;
		} 
	}
	if (sif->phrase_to_search != NULL)free(sif->phrase_to_search);
}
