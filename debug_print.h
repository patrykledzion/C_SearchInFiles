#pragma once
#include <stdio.h>

#define DEBUG_PRINT(x)	\
	printf("%s: %s/%d\n", x, __func__,  __LINE__);


#define ASSERT_ALLOC(pointer)		\
	if(pointer==NULL)			\
	{					\
		fprintf(stderr, "Error: Couldn't allocate memory");	\
		exit(0);			\
	}

#define ASSERT_FILE(fpointer)	\
	if(fpointer==NULL)			\
	{					\
		printf("");	\
		return NULL;			\
	}