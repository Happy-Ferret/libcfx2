
void* malloc_info( unsigned size );
void* realloc_info( void* pointer, unsigned size );
void free_info( void* pointer );

#if 1
#define libcfx2_malloc malloc
#define libcfx2_realloc realloc
#define libcfx2_free free
#else
#define libcfx2_malloc malloc_info
#define libcfx2_realloc realloc_info
#define libcfx2_free free_info
#endif
