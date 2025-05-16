#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <timeapi.h>
#include "SearchInFiles.h"
#include "debug_print.h"

#define TYPE_FILE 0
#define TYPE_DIRECTORY 1

#define WRONG_ARGS -1
#define MEM_ALLOC_ERR -2
#define OTHER_ERR -3

int read_cli_args(int argc, char** argv, int* level, char** src, int* case_ignore, char** phrase, int* type)
{
	if (argc < 3)return WRONG_ARGS;

	if (type == NULL || src == NULL || phrase == NULL)return OTHER_ERR;
	if (case_ignore != NULL)*case_ignore = 0;
	if (level != NULL)*level = -1;
	*type = -1;
	*src = NULL;
	*phrase = NULL; 
	for (int i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], "-f", 2) == 0 || strncmp(argv[i], "-d", 2) == 0)
		{
			if (i+1 >= argc || argv[i+1][0] == '-')return WRONG_ARGS; 
			*src = argv[i + 1];
			*type = strncmp(argv[i], "-f", 2) == 0 ? SIF_F_FILE : SIF_F_DIRECTORY;
			i++; 
		}

		else if (strncmp(argv[i], "-phrase", 7) == 0)
		{ 
			if (i + 1 >= argc)return WRONG_ARGS;
			*phrase = argv[i + 1];
			i++; 
		}

		else if (strncmp(argv[i], "-case-ignore", 12) == 0)
		{ 
			if (case_ignore == NULL)return OTHER_ERR;
			*case_ignore = SIF_F_CASE_IGNORE;
			i++; 
		}

		else if (strncmp(argv[i], "-level", 6) == 0)
		{ 
			if (i + 1 >= argc)return WRONG_ARGS;
			if (level == NULL)return OTHER_ERR;
			*level = atoi(argv[i + 1]);
			i++;  
		}
	}

	if (*type == -1 || *src == NULL || *phrase == NULL)return WRONG_ARGS;

	return 0;
}

int main(int argc, char** argv)
{
	DWORD time_start_ms = timeGetTime();
	if (argc == 2 && strncmp(argv[1], "help", 4) == 0)
	{
		printf("sif:\n");
		printf(" -d / -f {src}\t\tdirectory / file\n");
		printf(" -case-ignore\t\tignore case\n");
		printf(" -level {level}\t\tsearch level\n");
		printf(" -phrase {phrase}\tphrase to search\n");
		return 0;
	}

	char* src;
	char* phrase;
	int type;
	int case_ignore;
	int level;
	int ret = read_cli_args(argc, argv, &level, &src, &case_ignore, &phrase, &type);
	if (ret == WRONG_ARGS)
	{
		printf("Try: sif (-f/-d {path}) (-phrase {phrase}) [-case-ignore] [-level {level}]\n");
		return 1;
	}

	SearchInFiles* sif = SIF_create(src, phrase, type | case_ignore, level);
	if (sif == NULL)
	{
		printf("Couldn't create SIF object");
		return 1;
	}

	SIF_search(sif, 0);
	DWORD time_end_ms = timeGetTime();
	SIF_print(sif); 
	printf("time: %.3lf seconds\n", (double)(time_end_ms - time_start_ms)/1000);
	SIF_free(sif);
	return 0;
}