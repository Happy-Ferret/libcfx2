/*
    Copyright (c) 2010, 2011 Xeatheran Minexew

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

#ifndef __confix2_h__
#define __confix2_h__

#ifdef __cplusplus
extern "C"
{
#endif

/* original ...     2.1.09 */
/* new ...          25.1.10 */

#include <stdio.h>

#define libcfx2_version             0x0081
#define libcfx2_version_string      "0.8.1"
#define libcfx2_version_full        "libcfx2 version " libcfx2_version_string

#ifndef libcfx2
#define libcfx2
#endif

/* Error Codes */
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
/*#define cfx2_parse_error        10*/
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

/* Commands */
#define cfx2_cmd_nop            0
#define cfx2_cmd_select         1

/* Masks */
#define cfx2_piece_full         0
#define cfx2_piece_left         1
#define cfx2_piece_right        2

/* Conditions */
#define cfx2_cond_equals        0
#define cfx2_cond_not_equals    1
#define cfx2_cond_join_or       10
#define cfx2_cond_join_and      11

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
    void** items;
    size_t capacity, length;
}
cfx2_List;

typedef struct cfx2_Mask
{
    char* name;
    int piece_type;
}
cfx2_Mask;

typedef struct cfx2_Condition
{
    int type;
    char* left, * right;
    struct cfx2_Condition* left_cond, * right_cond;
}
cfx2_Condition;

typedef struct cfx2_Cmd
{
    int op;

    cfx2_List* masks;
}
cfx2_Cmd;

typedef struct cfx2_Attrib
{
    char* name, * value;
}
cfx2_Attrib;

typedef struct cfx2_Node
{
    char* name, * text;
    cfx2_List* attributes, * children;
}
cfx2_Node;

/* Interfaces */
typedef struct cfx2_IInput
{
    void ( *finished )( struct cfx2_IInput* );
    void ( *handle_error )( struct cfx2_IInput*, int error_code, int line, const char* desc );
    int ( *is_EOF )( struct cfx2_IInput* );
    size_t ( *read )( struct cfx2_IInput*, void* output, size_t length );
}
cfx2_IInput;

typedef struct cfx2_IOutput
{
    void ( *finished )( struct cfx2_IOutput* );
    void ( *handle_error )( struct cfx2_IOutput*, int error_code, const char* desc );
    size_t ( *write )( struct cfx2_IOutput*, const void* input, size_t length );
}
cfx2_IOutput;

typedef struct cfx2_ICmdListener
{
    void ( *on_node )( struct cfx2_ICmdListener* listener, cfx2_Node* node );
}
cfx2_ICmdListener;

/* Callback Prototypes */
typedef int ( *cfx2_IterateCallback )( size_t index, cfx2_Node* child, cfx2_Node* parent, void* user );
typedef int ( *cfx2_FindTest )( size_t index, cfx2_Node* child, cfx2_Node* parent, void* user );

/* cfx2 core */
libcfx2 const char* cfx2_get_error_desc( int error_code );

/* node manipulation */
libcfx2 int cfx2_create_node( const char* name, cfx2_Node** node );
libcfx2 cfx2_Node* cfx2_new_node( const char* name );
libcfx2 int cfx2_release_node_2( cfx2_Node** node_ptr );
libcfx2 int cfx2_rename_node( cfx2_Node* node, const char* name );
libcfx2 int cfx2_set_node_text( cfx2_Node* node, const char* text );

libcfx2 cfx2_Node* cfx2_clone_node( cfx2_Node* node );
libcfx2 int cfx2_merge_nodes_2( cfx2_Node* left, cfx2_Node* right, cfx2_Node** output_ptr, int flags );

/* node attributes */
libcfx2 cfx2_Attrib* cfx2_find_attrib( cfx2_Node* node, const char* name );
libcfx2 int cfx2_remove_attrib( cfx2_Node* node, const char* name );

libcfx2 int cfx2_get_node_attrib( cfx2_Node*, const char* name, const char** value );
libcfx2 int cfx2_get_node_attrib_int( cfx2_Node*, const char* name, long* value );
libcfx2 int cfx2_get_node_attrib_float( cfx2_Node*, const char* name, double* value );
libcfx2 int cfx2_set_node_attrib( cfx2_Node*, const char* name, const char* value );
libcfx2 int cfx2_set_node_attrib_int( cfx2_Node*, const char* name, long value );
libcfx2 int cfx2_set_node_attrib_float( cfx2_Node*, const char* name, double value );

/* child nodes */
libcfx2 int cfx2_add_child( cfx2_Node* parent, cfx2_Node* child );
libcfx2 int cfx2_insert_child( cfx2_Node* parent, size_t index, cfx2_Node* child );
libcfx2 cfx2_Node* cfx2_create_child( cfx2_Node* parent, const char* name, const char* text, cfx2_Uniqueness uniqueness );
libcfx2 cfx2_Node* cfx2_find_child( cfx2_Node* parent, const char* name );
libcfx2 cfx2_Node* cfx2_find_child_by_test( cfx2_Node* parent, cfx2_FindTest test, void* user );
libcfx2 int cfx2_iterate_child_nodes( cfx2_Node* parent, cfx2_IterateCallback callback, void* user );
libcfx2 int cfx2_remove_child( cfx2_Node* parent, cfx2_Node* child );

/* cfx2 reader */
libcfx2 int cfx2_read( cfx2_IInput* input, cfx2_Node** doc );
libcfx2 int cfx2_read_file( const char* file_name, cfx2_Node** doc );
libcfx2 int cfx2_read_from_string( const char* string, cfx2_Node** doc );
libcfx2 cfx2_Node* cfx2_load_document( const char* file_name );

/* cfx2 writer */
libcfx2 int cfx2_write( cfx2_Node* document, cfx2_IOutput* output );
libcfx2 int cfx2_write_to_buffer( cfx2_Node* document, char** text, size_t* capacity, size_t* used );
libcfx2 int cfx2_save_document( cfx2_Node* document, const char* file_name );

/* cfx2 basic query language */
libcfx2 cfx2_ResultType cfx2_query( cfx2_Node* base, const char* command, int allow_modifications, void** output );
libcfx2 cfx2_Node* cfx2_query_node( cfx2_Node* base, const char* command, int allow_modifications );
libcfx2 cfx2_Attrib* cfx2_query_attrib( cfx2_Node* base, const char* command, int allow_modifications );
libcfx2 const char* cfx2_query_value( cfx2_Node* base, const char* command );

/* cfx2 document query language */
libcfx2 int cfx2_compile_command( const char* command, cfx2_Cmd** cmd_ptr, int flags );
libcfx2 cfx2_Cmd* cfx2_compile( const char* command );
libcfx2 int cfx2_execute_command( cfx2_Node* node, cfx2_Cmd* cmd, cfx2_ICmdListener* listener );
libcfx2 int cfx2_release_command( cfx2_Cmd* cmd );

libcfx2 cfx2_List* cfx2_get_list( cfx2_Node* node, const char* command );
libcfx2 void cfx2_release_list( cfx2_List* list );

/* deprecated functions */

/* cfx2_release_node => cfx2_release_node_2 */
libcfx2 int cfx2_release_node( cfx2_Node* node );

/* cfx2_join_nodes, cfx2_merge_nodes => cfx2_merge_nodes_2 */
libcfx2 cfx2_Node* cfx2_join_nodes( cfx2_Node* left, cfx2_Node* right );
libcfx2 cfx2_Node* cfx2_merge_nodes( cfx2_Node* left, cfx2_Node* right );

/* possible future functions */
/*
libcfx2 int cfx2_remove_references( cfx2_Node* node, cfx2_Node* tree );
*/

/* utility macros */
#define cfx2_list_length( list_ ) ( ( list_ ) ? ( ( list_ )->length ) : 0 )
#define cfx2_has_children( node_ ) ( ( node_ ) ? cfx2_list_length( ( node_ )->children ) : 0 )
#define cfx2_item( list_, index_, type_ ) ( ( ( list_ ) && ( int )( index_ ) < ( int )( list_ )->length ) ? ( type_ )( ( list_ )->items[index_] ) : ( type_ )0 )

#ifdef _MSC_VER
#define libcfx2_snprintf sprintf_s
#else
#define libcfx2_snprintf snprintf
#endif

#ifdef __cplusplus
}
#endif

#endif
