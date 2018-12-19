#ifndef BWW_STR_LIB_PREFS
#define BWW_STR_LIB_PREFS

#define s_malloc malloc
#define s_free free
#define s_realloc realloc

// If true It'll memset all memory to 0 whenever freeing/doing extra mallocs
// could be useful to see if you have done something wrong (but only ocassionally segfault)
#define SANTIZE_MEMORY 0
// abort on OOM, may/may not be what you want
#define CRASH_ON_OOM 0

#define EXTEND_FACTOR 2

#endif
