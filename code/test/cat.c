// Programa de testeo cat

#include "syscall.h"

int
main(int argc, char **argv)
{
    int i;
    for(i = 1; i < argc; i++){
        OpenFileId fid = Open(argv[i]);
        if(fid != -1){
            char c;
            while(Read(&c, 1, fid) != 0){
                Write(&c, 1, ConsoleOutput);
            }
        }
        Close(argv[i]);
    }

    Exit(0);
}
