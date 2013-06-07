/*
    Copyright (c) 2010, 2011, 2013 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#ifndef confix2_h
#define confix2_h

#ifdef __cplusplus
extern "C"
{
#endif

/* original ...     2.1.09 */
/* new ...          25.1.10 */
/* 0.9.x ...        30.4.13 */

#include <stddef.h>

#define libcfx2_version             0x0091
#define libcfx2_version_string      "0.9.1"
#define libcfx2_version_full        "libcfx2 version " libcfx2_version_string

#ifndef libcfx2
#define libcfx2
#endif

/* Emulate stdint.h */
typedef unsigned char           cfx2_uint8_t;
typedef unsigned short          cfx2_uint16_t;

typedef signed char             cfx2_int8_t;
typedef signed short            cfx2_int16_t;

/* Result Codes */
#define cfx2_ok                 0
#define cfx2_EOF                1
#define cfx2_syntax_error       2
#define cfx2_interrupted        3
#define cfx2_param_invalid      4
#define cfx2_cant_open_file     5
#define cfx2_alloc_error        6
#define cfx2_attrib_not_found   7
#define cfx2_missing_node_name  8
#define cfx2_node_not_found     9
#define cfx2_max_err            10

/* Callback Reactions */
typedef int cfx2_Action;
#define cfx2_continue           0
#define cfx2_stop               1

/* Uniqueness Values */
typedef int cfx2_Uniqueness;
#define cfx2_multiple           0
#define cfx2_unique             1

/* Query Results */
typedef int cfx2_ResultType;
#define cfx2_fail               ( -1 )
#define cfx2_void               0
#define cfx2_node               1
#define cfx2_attrib             2

/* Clone Flags */
#define cfx2_clone_recursive    1

/* Merge Flags */
#define cfx2_release_left       1
#define cfx2_release_right      2
#define cfx2_name_from_left     4
#define cfx2_text_from_left     8
#define cfx2_name_from_right    16
#define cfx2_text_from_right    32
#define cfx2_prefer_attribs_from_left   64
#define cfx2_prefer_attribs_from_right  128
#define cfx2_left_children_first    256
#define cfx2_right_children_first   512

/* Structures */
typedef struct cfx2_List
{
    cfx2_uint8_t*   items;
    size_t          length;
}
cfx2_List;

typedef struct cfx2_Attrib
{
    char*   name;
    char*   value;
}
cfx2_Attrib;

typedef struct cfx2_Node
{
    char*       name;
    char*       text;
    cfx2_List   attributes;
    cfx2_List   children;
    char*       shared;
}
cfx2_Node;

/* Option Structures */
typedef struct cfx2_RdOpt cfx2_RdOpt;
typedef struct cfx2_WrOpt cfx2_WrOpt;

struct cfx2_RdOpt
{
    void* stream_priv;
    size_t ( *stream_read )( cfx2_RdOpt* rd_opt, char* buffer, size_t length );
    void ( * stream_close )( cfx2_RdOpt* rd_opt );
    
    void* client_priv;
    int ( *on_error)( cfx2_RdOpt* rd_opt, int rc, int line, const char* desc );
    
    int flags;
};

struct cfx2_WrOpt
{
    void* stream_priv;
    size_t ( *stream_write )( cfx2_WrOpt* rd_opt, const char* buffer, size_t length );
    void ( * stream_close )( cfx2_WrOpt* rd_opt );
    
    void* client_priv;
    int ( *on_error)( cfx2_WrOpt* rd_opt, int rc, int line, const char* desc );
    
    int flags;
};

/* Callback Prototypes */
typedef int ( *cfx2_IterateCallback )( size_t index, cfx2_Node* child, cfx2_Node* parent, void* user );
typedef int ( *cfx2_FindTest )( size_t index, cfx2_Node* child, cfx2_Node* parent, void* user );

/* cfx2 core */
libcfx2 const char* cfx2_get_error_desc( int error_code );

/* node manipulation */
libcfx2 int         cfx2_create_node( cfx2_Node** node );
libcfx2 cfx2_Node*  cfx2_new_node( const char* name );
libcfx2 void        cfx2_release_node( cfx2_Node** node_ptr );

libcfx2 int         cfx2_preallocate_shared_buffer( cfx2_Node* node, size_t size, int flags );
libcfx2 int         cfx2_rename_node( cfx2_Node* node, const char* name );
libcfx2 int         cfx2_set_node_text( cfx2_Node* node, const char* text );

libcfx2 cfx2_Node*  cfx2_clone_node( cfx2_Node* node, int flags );
/*libcfx2 int         cfx2_merge_nodes( cfx2_Node* left, cfx2_Node* right, cfx2_Node** output_ptr, int flags );*/

/* node attributes */
libcfx2 cfx2_Attrib* cfx2_find_attrib( cfx2_Node* node, const char* name );
libcfx2 int         cfx2_remove_attrib( cfx2_Node* node, const char* name );

libcfx2 int         cfx2_get_node_attrib( cfx2_Node*, const char* name, const char** value );
libcfx2 int         cfx2_get_node_attrib_int( cfx2_Node*, const char* name, long* value );
libcfx2 int         cfx2_get_node_attrib_float( cfx2_Node*, const char* name, double* value );
libcfx2 int         cfx2_set_node_attrib( cfx2_Node*, const char* name, const char* value );
libcfx2 int         cfx2_set_node_attrib_int( cfx2_Node*, const char* name, long value );
libcfx2 int         cfx2_set_node_attrib_float( cfx2_Node*, const char* name, double value );

/* child nodes */
libcfx2 int         cfx2_add_child( cfx2_Node* parent, cfx2_Node* child );
libcfx2 int         cfx2_insert_child( cfx2_Node* parent, size_t index, cfx2_Node* child );
libcfx2 cfx2_Node*  cfx2_create_child( cfx2_Node* parent, const char* name, const char* text, cfx2_Uniqueness uniqueness );
libcfx2 cfx2_Node*  cfx2_find_child( cfx2_Node* parent, const char* name );
libcfx2 cfx2_Node*  cfx2_find_child_by_test( cfx2_Node* parent, cfx2_FindTest test, void* user );
libcfx2 int         cfx2_iterate_child_nodes( cfx2_Node* parent, cfx2_IterateCallback callback, void* user );
libcfx2 int         cfx2_remove_child( cfx2_Node* parent, cfx2_Node* child );

/* cfx2 reader */
libcfx2 int         cfx2_read( cfx2_Node** doc_ptr, cfx2_RdOpt* rd_opt );
libcfx2 int         cfx2_read_file( cfx2_Node** doc_ptr, const char* filename, const cfx2_RdOpt* rd_opt_in );
libcfx2 int         cfx2_read_from_string( cfx2_Node** doc_ptr, const char* document, const cfx2_RdOpt* rd_opt_in );
libcfx2 cfx2_Node*  cfx2_load_document( const char* filename );

/* cfx2 writer */
libcfx2 int         cfx2_write( cfx2_Node* doc, cfx2_WrOpt* wr_opt );
libcfx2 int         cfx2_write_to_buffer( cfx2_Node* doc, char** text, size_t* capacity, size_t* used );
libcfx2 int         cfx2_save_document( cfx2_Node* doc, const char* file_name );

/* cfx2 basic query language */
libcfx2 cfx2_ResultType cfx2_query( cfx2_Node* base, const char* command, int allow_modifications, void** output );
libcfx2 cfx2_Node*  cfx2_query_node( cfx2_Node* base, const char* command, int allow_modifications );
libcfx2 cfx2_Attrib* cfx2_query_attrib( cfx2_Node* base, const char* command, int allow_modifications );
libcfx2 const char* cfx2_query_value( cfx2_Node* base, const char* command );

/* utility macros */
#define cfx2_list_length( list_ ) ( (list_).length )
#define cfx2_has_children( node_ ) ( (node_) ? cfx2_list_length( (node_)->children ) : 0 )
#define cfx2_item( list_, index_, type_ ) ( (type_*)(list_).items )[index_]

#ifdef _MSC_VER
#define libcfx2_snprintf sprintf_s
#else
#define libcfx2_snprintf snprintf
#endif

#ifdef __cplusplus
}
#endif

#endif
