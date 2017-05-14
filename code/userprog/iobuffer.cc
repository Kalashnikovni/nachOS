// Routines to read and write to buffer easily
// relaying mainly on the functions ReadMem and
// WriteMem defined in machine/translate

#include "machine/translate.cc"

#define READMEM(addr,size,val) ASSERT(machine->ReadMem(addr,size,val))
#define WRITEMEM(addr,size,val) (machine->Writemem(addr,size,val))


// Read a null terminated string from user mem
// (The address is assumed to be 4 byte aligned)
void 
ReadStringFromUser(int addr, char *strbuf, unsigned maxcount)
{
    int i=0;
    char c; 
    do {
        READMEM(addr+i, 1, strbuf+i);
        i++;
        READMEM(addr+i, 1, &c);
    } while (c != '\0');
    strbuf[i]='\0'; //add EOF
}


// Read from user mem to a buffer
// Short version, many disk read/writes
// (Address assumed to be 4 byte aligned)
void
ReadBufferFromUser(int addr, char *outbuf, unsigned count)
{
    int i;
    for (i=0; i < count; i++){
        READMEM(addr+i, 1, outbuf+i);
    }
}

// Read from user mem to a buffer
// Long version, less disk read/writes
// (Address assumed to be 4 byte aligned)
void
SpareReadBufferFromUser(int addr, char *outbuf, unsigned count)
{
    int i = 0;
    while (count-i > 3){ //Read 4 bytes at a time while possible
        READMEM(addr+i, 4, outbuf+i)
        i+=4;
    }
    switch(i){
        case 3:  //Read 3 bytes more
            READMEM(addr+i, 2, outbuf+i);
            READMEM(addr+i+2, 1, outbuf+i+2);
            break;
        case 2:  //Read 2 bytes more
            READMEM(addr+i, 2, outbuf+i);
            break;
        case 1:  //Read 1 byte more
            READMEM(addr+i, 1, outbuf+i);
            break;
        default: //End
            break;
    }
}


// Write a null terminated string to user mem
void
WriteStringToUser(const char *str, int addr)
{
    int i;
    for(i=0; str[i] != '\0'; i++){
        WRITEMEM(addr+i, 1, str[i]);
    }
    WRITEMEM(addr+i+1, 1, '\0'); //TODO: Esta bien asi?
}


// Write a buffer to a user memory space
// Short version, many disk read/writes
void
WriteBufferToUser(const char *buf, int addr, unsigned count)
{
    int i;
    for (i=0; i < count; i++){
        WRITEMEM(addr+i, 1, buf[i]);
    }
}

// Write a buffer to a user memory space
// Long version, less disk read/writes
void
SpareWriteBufferToUser(const char *buf, int addr, unsigned count)
{
    int i = 0;
    while (count-i > 3){ //Write 4 bytes at a time while possible
        WRITEMEM(addr+i, 4, buf[i])
        i+=4;
    }
    switch(i){
        case 3:  //Read 3 bytes more
            WRITEMEM(addr+i, 2, buf[i]);
            WRITEMEM(addr+i+2, 1, outbuf[i+2]);
            break;
        case 2:  //Read 2 bytes more
            WRITEMEM(addr+i, 2, buf[i]);
            break;
        case 1:  //Read 1 byte more
            WRITEMEM(addr+i, 1, outbuf[i]);
            break;
        default: //End
            break;
    }
}
