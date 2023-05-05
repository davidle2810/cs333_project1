#include "syscall.h"
#include "copyright.h"

void strcopy(const char* src, char* dest)
{
	while (*src)
		*dest++ = *src++;
	*dest='\0';
}


int main()
{
	char s[100]; 
	char fileName[100];
	strcopy("Input filename: ", s);
	Write(s, 32, ConsoleOutput);
	Read(fileName, 32,ConsoleInput);
	Remove(fileName);
	return 0;
}

