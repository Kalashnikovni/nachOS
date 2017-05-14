// Routines to read strings from user and write to buffer


#ifndef __IOBUFFER_H__
#define __IOBUFFER_H__

void 
ReadStringFromUser(int userAddress, char *outString, unsigned maxByteCount);

void 
ReadBufferFromUser(int userAddress, char *outBuffer, unsigned byteCount);

void 
WriteStringToUser(const char *string, int userAddress);

void 
WriteBufferToUser(const char* buffer, int userAddress, unsigned byteCount);

#endif //__IOBUFFER_H_
