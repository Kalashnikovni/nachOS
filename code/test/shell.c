#include "syscall.h"

#define MAX_LINE_SIZE  60
#define MAX_ARG_COUNT  32

#define ARG_SEPARATOR        ' '
#define ARG_AGGREGATOR_START '('
#define ARG_AGGREGATOR_END   ')'
#define BACKGROUND_RUN       '&'

#define NULL  ((void *) 0)

static inline unsigned
strlen(const char *s)
{
    unsigned i = 0;
    if(s != NULL){
        for (i = 0; s[i] != '\0'; i++);
    }
    return i;
}

static inline void
WritePrompt(OpenFileId output)
{
    static const char PROMPT[] = "nachos$ ";
    Write(PROMPT, sizeof(PROMPT) - 1, output);
}

static inline void
WriteError(const char *description, OpenFileId output)
{
    static const char PREFIX[] = "(nachos)Error: ";
    static const char SUFFIX[] = "\n";

    if(description != NULL){
        Write(PREFIX, sizeof PREFIX - 1, output);
        Write(description, strlen(description), output);
        Write(SUFFIX, sizeof SUFFIX - 1, output);
    }
}

//Returns -1 if only \n or EOF, or size of the reading if not
static int
ReadLine(char *buffer, unsigned size, OpenFileId input)
{
    int i;

    if(buffer != NULL){
        for (i = 0; i < size-1; i++) {
            Read(&buffer[i], 1, input);
            if((buffer[i] == '\n') || (buffer[i] == '\0')){
                i--;
                break;
            }
        }
        buffer[i+1] = '\0';
    }
    return i;
}

static int
PrepareArguments(char *line, char **argv, unsigned argvSize, _Bool *bg)
{
    // TODO: how to make sure that `line` and `argv` are not `NULL`?, and
    //        for `argvSize`, what precondition should be fulfilled?
    //
    // PENDIENTE: use `bool` instead of `int` as return type; for doing this,
    //            given that we are in C and not C++, it is convenient to
    //            include `stdbool.h`.

    unsigned argCount;

    argv[0] = line;
    argCount = 1;

    // Traverse the whole line and replace spaces between arguments by null
    // characters, so as to be able to treat each argument as a standalone
    // string.
    //
    // TODO: what if the user wants to include a space as part of an
    //        argument?
    unsigned i = 0;
//    if(line[i] == BACKGROUND_RUN){
//        *bg = 1;
//        i=2;
//    }
//    else{
//        *bg = 0;
//    }

    for (; line[i] != '\0'; i++)
        if (line[i] == ARG_SEPARATOR) {
            if (argCount == argvSize - 1)
                // The maximum of allowed arguments is exceeded, and
                // therefore the size of `argv` is too.  Note that 1 is
                // decreased in order to leave space for the NULL at the end.
                return 0;
            line[i] = '\0';
            argv[argCount] = &line[i + 1];
            argCount++;
            // Solves case with two or more consecutive spaces
            while(line[i+1] == ARG_SEPARATOR){
                i++;
            }
        }

    argv[argCount] = NULL;
    return 1;
}

int
main(void)
{
    const OpenFileId INPUT  = ConsoleInput;
    const OpenFileId OUTPUT = ConsoleOutput;
    char line[MAX_LINE_SIZE];
    char *argv[MAX_ARG_COUNT];
    char line2[MAX_LINE_SIZE - 2];


    for (;;) {
        WritePrompt(OUTPUT);
        const unsigned lineSize = ReadLine(line, MAX_LINE_SIZE, INPUT);
        if (lineSize <= 0){
            continue;
        }
        
        char line_;
        int bg;
        if (line[0]=='&') {
            bg = 1;
            line_ = line + 2*sizeof(char);
        }
        else {
        bg = 0;
        line_ = line;
        }
        if (PrepareArguments(line_, argv, MAX_ARG_COUNT, &bg) == 0) {
            WriteError("too many arguments.", OUTPUT);
            continue;
        }

        WriteError(line, OUTPUT);
        const SpaceId newProc = Exec(line_, argv);
        if(newProc < 0){ //FIXME if needed (after implementing pids)
            WriteError("error executing the process.", OUTPUT);
        }

        if(!bg){
            if(Join(newProc) < 0){
                WriteError("join failed or process exited with error.", OUTPUT);
            }
        }
    }

    return 0;  // Never reached.
}
