#include "defines.h"
#include "fileutils.h"
#include "matcher.h"
#include "verilog.h"

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        printf( "Wrong number of command-line arguments.\n" );
        return FAIL;
    }

    if (argc == 2)
    {
        char *in_file1 = argv[1];
        char *in_file2 = argv[2];

        Circuit *cir1 = parse_verilog(FileUtils::load_file(in_file1));
        Circuit *cir2 = parse_verilog(FileUtils::load_file(in_file2));

        return OK;
    }

    return OK;
}




