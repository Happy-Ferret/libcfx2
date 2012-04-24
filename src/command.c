
#define cfx2_enable_experimental

#include "config.h"
#include "lexer.h"
#include "list.h"

#include <confix2.h>
#include <stdlib.h>
#include <string.h>

/*
    TODO: back-releasing mask list on stx error
*/

static int is_keyword( Token* tok )
{
    if ( tok->type != T_text )
        return 0;

    return strcmp( tok->text, "select" ) == 0 || strcmp( tok->text, "where" ) == 0;
}

static cfx2_Mask* new_mask( const char* name, int piece_type )
{
    cfx2_Mask* mask;

    mask = ( cfx2_Mask* )libcfx2_malloc( sizeof( cfx2_Mask ) );

    if ( !mask )
        return 0;

    mask->name = ( char* )libcfx2_malloc( strlen( name ) + 1 );

    if ( !mask->name )
        return 0;

    strcpy( mask->name, name );

    mask->piece_type = piece_type;
    return mask;
}

static int str_mask_cmp( const char* str, cfx2_Mask* mask )
{
    if ( !mask )
        return 0;
    else if ( !str )
        return 1;

    switch ( mask->piece_type )
    {
        case cfx2_piece_full: return strcmp( str, mask->name );
        case cfx2_piece_left: return strncmp( str, mask->name, strlen( mask->name ) );
        case cfx2_piece_right: return strcmp( str + strlen( str ) - strlen( mask->name ), mask->name );
    }

    printf( "error445 " );
    return 0;
}

static int str_masks_cmp( const char* str, cfx2_List* masks )
{
    size_t i;

    if ( !masks )
        return 0;

    for ( i = 0; i < masks->length; i++ )
        if ( str_mask_cmp( str, cfx2_item( masks, i, cfx2_Mask* ) ) == 0 )
            return 0;

    return 1;
}

static void release_mask( cfx2_Mask* mask )
{
    free( mask->name );
    free( mask );
}

static void release_mask_list( cfx2_List* masks )
{
    size_t i;

    if ( !masks )
        return;

    for ( i = 0; i < masks->length; i++ )
        release_mask( cfx2_item( masks, i, cfx2_Mask* ) );

    cfx2_release_list( masks );
}

static int parse_mask( Lexer* lexer, cfx2_Mask** mask_ptr )
{
    cfx2_Mask* mask;
    Token tok;
    int error;

    error = lexer_read( lexer, &tok );

    if ( error )
        return error;

    /*
        If the mask begins with an '*', it is either followed by a string
        (a right-piece mask) or is actually no a mask at all
    */
    if ( tok.type == T_asterisk )
    {
        /* drop the '*' token and peek at the next one */
        error = lexer_get_current( lexer, &tok );

        /* it is a pure text token - we've got a right-piece mask */
        if ( !error && tok.type == T_text && !is_keyword( &tok ) )
        {
            error = lexer_read( lexer, &tok );

            if ( error )
                return error;

            mask = new_mask( tok.text, cfx2_piece_right );
            lexer_delete_token( &tok );

            *mask_ptr = mask;
            return mask ? cfx2_ok : cfx2_alloc_error;
        }
        else if ( error && error != cfx2_EOF )
            return error;

        *mask_ptr = 0;
        return cfx2_ok;
    }
    /*
        If the mask begins with a string, it is either followed by an '*'
        (a left-piece mask) or is a full-name mask
    */
    else if ( tok.type == T_text )
    {
        /* can not drop the old token yet, gonna need it */
        Token next;

        error = lexer_get_current( lexer, &next );

        /* an '*' - we've got a left-piece mask */
        if ( !error && next.type == T_asterisk )
        {
            error = lexer_read( lexer, &next );

            if ( error )
            {
                lexer_delete_token( &tok );
                return error;
            }

            mask = new_mask( tok.text, cfx2_piece_left );
            lexer_delete_token( &tok );

            *mask_ptr = mask;
            return mask ? cfx2_ok : cfx2_alloc_error;
        }
        else if ( error && error != cfx2_EOF )
        {
            lexer_delete_token( &tok );
            return error;
        }

        /* a full-name mask */
        mask = new_mask( tok.text, cfx2_piece_full );
        lexer_delete_token( &tok );

        *mask_ptr = mask;
        return mask ? cfx2_ok : cfx2_alloc_error;
    }

    error = cfx2_syntax_error;
    lexer->input->handle_error( lexer->input, error, lexer->line, "Expected mask near TODO" );
    return error;
}

static int parse_masks( Lexer* lexer, cfx2_List** list_ptr )
{
    cfx2_List* list;
    cfx2_Mask* mask;
    int error;

    list = 0;

    for ( ; ; )
    {
        Token next;

        error = parse_mask( lexer, &mask );

        if ( error )
            return error;

        if ( mask )
        {
            if ( !list )
            {
                list = new_list();

                if ( !list )
                {
                    release_mask( mask );
                    return cfx2_alloc_error;
                }
            }

            list_add( list, mask );
        }
        else
        {
            if ( list )
            {
                release_mask_list( list );

                error = cfx2_syntax_error;
                lexer->input->handle_error( lexer->input, error, lexer->line, "Unexpected '*' near TODO" );
                return error;
            }

            *list_ptr = 0;
            return cfx2_ok;
        }

        /* if a ',' follows, continue parsing the masks */
        error = lexer_get_current( lexer, &next );

        if ( error == cfx2_EOF || ( !error && next.type != T_comma ) )
            break;
        else if ( error )
            return error;

        error = lexer_read( lexer, &next );

        if ( error )
            return error;
    }

    *list_ptr = list;
    return cfx2_ok;
}

static int parse_command( Lexer* lexer, cfx2_Cmd** cmd_ptr )
{
    Token op_name, tok;
    cfx2_Cmd* cmd;
    int op, error;

    error = lexer_read( lexer, &op_name );

    if ( error )
        return error;

    if ( op_name.type != T_text )
    {
        lexer_delete_token( &op_name );

        error = cfx2_syntax_error;

        lexer->input->handle_error( lexer->input, error, lexer->line, "Expected command near TODO" );
        return error;
    }

    if ( strcmp( op_name.text, "select" ) == 0 )
        op = cfx2_cmd_select;
    else
    {
        lexer_delete_token( &op_name );

        error = cfx2_syntax_error;

        lexer->input->handle_error( lexer->input, error, lexer->line, "Unknown command TODO" );
        return error;
    }

    lexer_delete_token( &op_name );

    if ( op == cfx2_cmd_select )
    {
        cfx2_List* masks;

        error = parse_masks( lexer, &masks );

        if ( error )
            return error;

        error = lexer_get_current( lexer, &tok );

        /* it is a pure text token - we've got a right-piece mask */
        if ( !error && tok.type == T_text && strcmp( op_name.text, "where" ) == 0 )
        {
            error = lexer_read( lexer, &tok );

            if ( error )
            {
                release_mask_list( masks );
                return error;
            }

            lexer_delete_token( &tok );

            /* TODO: parse condition */
        }
        else if ( error && error != cfx2_EOF )
        {
            release_mask_list( masks );
            return error;
        }

        cmd = ( cfx2_Cmd* )malloc( sizeof( cfx2_Cmd ) );

        if ( !cmd )
        {
            release_mask_list( masks );
            return cfx2_alloc_error;
        }

        cmd->op = cfx2_cmd_select;
        cmd->masks = masks;

        *cmd_ptr = cmd;
        return cfx2_ok;
    }

    printf( "ERROR: don't get\n" );
    return cfx2_alloc_error;
}

libcfx2 int cfx2_compile_command( const char* command, cfx2_Cmd** cmd_ptr, int flags )
{
    cfx2_IInput* input;
    Lexer* lexer;
    int error;

    /* First of all, test the arguments */
    /* This is a bit more complicated than we would like it to be,
        because of the need for releasing /input/ */

    if ( !command || !cmd_ptr )
        return cfx2_param_invalid;

    /* construct an input stream and a lexer */

    error = new_buffer_input_from_string( command, &input );

    if ( error )
        return error;

    error = new_lexer( input, &lexer );

    if ( error )
    {
        input->finished( input );
        return error;
    }

    /* And launch the parsing! */
    error = parse_command( lexer, cmd_ptr );

    /* We don't need the input any more, so let's free it. */
    delete_lexer( lexer );

    return error;
}

libcfx2 cfx2_Cmd* cfx2_compile( const char* command )
{
    cfx2_Cmd* cmd;

    if ( cfx2_compile_command( command, &cmd, 0 ) == cfx2_ok )
        return cmd;
    else
        return 0;
}

typedef struct ExecutionState
{
    cfx2_Cmd* cmd;
    cfx2_ICmdListener* listener;
}
ExecutionState;

static int cmd_select_callback( size_t index, cfx2_Node* child, cfx2_Node* parent, void* user )
{
    ExecutionState* state;

    state = ( ExecutionState* ) user;

    if ( state->listener && state->listener->on_node && str_masks_cmp( child->name, state->cmd->masks ) == 0 )
        state->listener->on_node( state->listener, child );

    return cfx2_continue;
}

libcfx2 int cfx2_execute_command( cfx2_Node* node, cfx2_Cmd* cmd, cfx2_ICmdListener* listener )
{
    ExecutionState state;

    if ( !node || !cmd )
        return cfx2_param_invalid;

    state.cmd = cmd;
    state.listener = listener;

    switch ( cmd->op )
    {
        case cfx2_cmd_select:
            cfx2_iterate_child_nodes( node, cmd_select_callback, &state );
            break;
    }

    return cfx2_ok;
}

libcfx2 int cfx2_release_command( cfx2_Cmd* cmd )
{
    if ( !cmd )
        return cfx2_param_invalid;

    release_mask_list( cmd->masks );
    free( cmd );

    return cfx2_ok;
}

typedef struct
{
    cfx2_ICmdListener listener;
    cfx2_List* list;
}
GetSetState;

static void get_set_on_node( cfx2_ICmdListener* listener, cfx2_Node* node )
{
    GetSetState* state;

    state = ( GetSetState* ) listener;

    if ( !state->list )
        state->list = new_list();

    list_add( state->list, node );
}

libcfx2 cfx2_List* cfx2_get_list( cfx2_Node* node, const char* command )
{
    cfx2_Cmd* cmd;
    GetSetState state;

    cmd = cfx2_compile( command );

    if ( !cmd )
        return 0;

    state.listener.on_node = get_set_on_node;
    state.list = 0;

    cfx2_execute_command( node, cmd, &state.listener );
    cfx2_release_command( cmd );

    return state.list;
}
