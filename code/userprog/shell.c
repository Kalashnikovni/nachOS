//
//

#include "syscall.c"
#include <stdbool.h>

#define MAX_LINE_SIZE 60
#define MAX_ARG_COUNT 32

#define ARG_SEPARATOR ' '
#define ARG_AGGREGATOR_START '('
#define ARG_AGGREGATOR_END   ')'
#define BACKGROUND_RUN '&'

#define NULL ((void *) 0)

static inline unsigned
strlen(const char* s)
{
    unsigned i = 0;
    if(s != NULL){
        for(i = 0; s[i] != '\0'; i++);
    }
    return i;
}

static inline void
WritePrompt(OpenFileId output)
{
    static const char PROMPT[] = "nachos$ ";
    Write(PROMPT, sizeof(PROMPT) -1, output);
}

static inline void
WriteError(const char *description, OpenFileId output)
{
    static const char PREFIX[] = "(nachOS)Error: ";
    static const char SUFFIX[] = '\n';

    if(description != NULL){
        Write(PREFIX, sizeof(PREFIX) -1, output);
        Write(description, strlen(description), output);
        Write(SUFFIX, sizeof(SUFFIX) -1, output);
    }
}

static unsigned
ReadLine(char *buffer, unsigned size, OpenFileId input)
{
    unsigned i;

    if(buffer != NULL){
        for(i = 0; i < size-1; i++){
            Read(&buffer[i], 1, input);
            if((buffer[i] == '\n') || (buffer[i] == '\0')){
                i--;
                break;
            }
        }
        buffer[i+1] = '\0'
    }
    return i;
}

int
PrepareArguments(char *line, char **argv, unsigned argvSize, bool *background)
{
    /*TODO*/
} 

int
main(void)
{
    const OpenFileId INPUT = ConsoleInput; /*def in syscall.h*/
    const OpenFileId OUTPUT = ConsoleOutput; /* = */
    char line[MAX_LINE_SIZE];
    char *argv[MAX_ARG_COUNT];

    for(;;){
        WritePrompt(OUTPUT);
        const unsigned lineSize = ReadLine(line, MAX_LINE_SIZE, INPUT);
        if(lineSize == 0){
            continue;
        }

        bool *bg;
        if(PrepareArguments(line, argv, MAX_ARG_COUNT, bg) < 1) {
            WriteError("Too many arguments.", OUTPUT);
            continue;
        }

        const SpaceId newProc = Exec(line, argv); /*TODO?: Podriamos no darle los argumentos x separado*/
        if(SpaceId < 0){
            WriteError("Error executing the process.", OUTPUT);
            continue;
        }

        if(!bg){
            Join(newProc);
            /*TODO: check for join errors*/
        }

    }

    return 0; //Never reached
}

