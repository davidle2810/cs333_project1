#include "syscall.h"
//#include "copyright.h"
int getLength(int id)
{
	int length=Seek(-1,id);
	Seek(0,id);
	return length;
}

void strcopy(char* src, char* dest)
{
	while (*src)
		*dest++ = *src++;
	*dest='\0';
}

int main()
{
	int openFileId;
	int openFileLength;
	int fileSize;
	char s[100]; 
	char* fileName;
	char* content;
	strcopy("Input filename: ", s);
	Write(s, 32, 1);
	Read(fileName, 32,0);

	openFileId = Open(fileName, 1); 
	openFileLength = getLength(openFileId);
	
	Read(content, openFileLength, openFileId);
	Write(content, openFileLength, 1);
	Halt();
}
