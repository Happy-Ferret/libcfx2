
#ifdef libcfx2_make_test

#define cfx2_enable_experimental

#include <confix2.h>
#include <stdio.h>

static void print_node( cfx2_Node* node, unsigned depth )
{
    unsigned i;

    /*if ( node->name )*/
    {
        for ( i = 0; i < depth; i++ )
            printf( "  " );

        printf( "[%s]", node->name );

        if ( node->text )
            printf( ": '%s'", node->text );

        printf( " (" );

        for ( i = 0; i < node->attributes->length; i++ )
        {
            cfx2_Attrib* attrib = ( cfx2_Attrib* )node->attributes->items[i];
            printf( "%s : '%s'", attrib->name, attrib->value );

            if ( i + 1 < node->attributes->length )
                printf( ", " );
        }

        printf( ")\n" );
    }

    if ( cfx2_has_children( node ) )
        for ( i = 0; i < node->children->length; i++ )
            print_node( ( cfx2_Node* )node->children->items[i], depth + 1 );
}

static void on_node( cfx2_ICmdListener* listener, cfx2_Node* node )
{
    printf( "matching node: [%s]\n", node->name );
}

static void test_adml( cfx2_Node* doc )
{
    cfx2_Cmd* cmd;

    cfx2_ICmdListener listener;

    cmd = cfx2_compile( "select User*, Root*" );

    if ( !cmd )
    {
        printf( "compilation failed.\n" );
        return;
    }

    listener.on_node = on_node;
    cfx2_execute_command( doc, cmd, &listener );

    cfx2_release_command( cmd );
}

int main( int argc, char** argv )
{
    cfx2_Node* n;

    int error = cfx2_read_file( "le_doc.cfx2", &n );

    printf( "%s\n", cfx2_get_error_desc( error ) );

    /*static const char* filename = "config.cfx2";

    cfx2_Node* doc;

    printf( libcfx2_version_full "\n" );

    if ( argc > 1 )
        filename = argv[1];

    printf( "libcfx2: loading `%s`...\n", filename );
    doc = cfx2_load_document( filename );

    if ( !doc )
        return 1;

    print_node( doc, 1 );

    test_adml( doc );

    return 0;*/
}
#endif
