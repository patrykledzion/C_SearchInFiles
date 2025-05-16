#include "file.h"
#include "debug_print.h"

File* File_open(const char* filename)
{
	if (filename == NULL)return NULL;

	FILE* fp = fopen(filename, "rb");
	ASSERT_FILE(fp);

	File* f_ret = malloc(sizeof(File)); 
	ASSERT_ALLOC(f_ret);

	f_ret->pointer = fp;
	size_t name_size = strlen(filename);
	f_ret->filename = malloc(sizeof(char) * name_size + 1); 
	ASSERT_ALLOC(f_ret->filename);

	strncpy_s(f_ret->filename, name_size + 1, filename, name_size + 1);
	f_ret->filename[name_size] = '\x0';
	f_ret->curr_line = 1;
	f_ret->curr_pos_in_line = 1;
	return f_ret;
}

char File_getc(File* file)
{
	if (file == NULL)return 0;
	if (file->pointer == NULL)return 0;

	char ret = fgetc(file->pointer);

	file->curr_pos_in_line++;
	if (ret == '\n')
	{
		file->curr_line++;
		file->curr_pos_in_line = 1;
	}

	return ret;
}

void File_free(File* f)
{
	fclose(f->pointer);
	free(f->filename);
	free(f);
}