#include "syscall.h"

int main()
{
	int openFileId;
	int fileSize;
	char c; 
	char fileName[32];
	int i;
	PrintString("Input filename: ");
	ReadString(fileName, 31);

	openFileId = Open(fileName, 1); 
	if (openFileId != -1) 
	{
		fileSize = Seek(-1, openFileId);
		i = 0;
		Seek(0, openFileId);
		for (i; i < fileSize; i++) 
		{
			Read(&c, 1, openFileId); 
			PrintChar(c); 
		}
		Close(openFileId);
	}
	else
	{
		PrintString("Unable to open file!\n\n");
	}
	return 0;
}
