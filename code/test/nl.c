// Programa de testeo nl
 
#include "syscall.h"
#define MAX_NLINES 5

void 
itoc(int i)
{
    int j, k = 0; 
    char buffer[MAX_NLINES], space = ' ';

    for(j = 1; j <= i; j = j * 10)
        k++;

    k--;

    for(j = k; j < MAX_NLINES; j++)
        buffer[j] = ' ';

    if(i < 10)
        buffer[0] = i + 48;

    else {
        for(; i >= 10; i = i / 10){
            buffer[k] = i % 10 + 48;
            k--;
        }

        buffer[0] = i + 48;
    }

    Write(&space, 1, ConsoleOutput);
    Write(buffer, MAX_NLINES, ConsoleOutput);
    Write(&space, 1, ConsoleOutput);
}


int
main(int argc, char **argv)
{
    int i, line;
    char *error = "Error al intentar abrir archivo, reintente\n", eol = '\n', n;

    for(i = 1; i < argc; i++){
        OpenFileId fid = Open(argv[i]);
        if(fid != -1){
            char c;
            itoc(line);
            line++;
            while(Read(&c, 1, fid) != 0){
                Write(&c, 1, ConsoleOutput);
                if(c == '\n'){
                    itoc(line);
                    line++;
                }
            }
            Write(&eol, 1, ConsoleOutput);
            Close(fid);
        }
        else
            Write(error, 50, ConsoleOutput);
    }

    Exit(0);
}
