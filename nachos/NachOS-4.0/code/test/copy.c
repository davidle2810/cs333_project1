#include "syscall.h"

int main()
{
	int srcId;
	int destId;
	int fileSize; 
	char c; 
	char source[32];
	char dest[32];

	PrintString("Source file: ");
	ReadString(source, 32); 
	PrintString("Destination file: ");
	ReadString(dest, 32); 
	srcId = Open(source, 1); 
	
	if (srcId != -1) 
	{
		destId = Create(dest);
		Close(destId);
		
		destId = Open(dest, 0); 
		if (destId != -1) 
		{
			int i;
			fileSize = Seek(-1, srcId);
			Seek(0, srcId); 
			Seek(0, destId); 			
			for (i; i < fileSize; i++) 
			{
				Read(&c, 1, srcId);  
				Write(&c, 1, destId); 
			}
			
			PrintString("Successed.\n\n");
			Close(destId); 
		}
		else
		{
			PrintString("Failed.\n\n");
		}
		
		Close(srcId); 
	}
	else
	{
		PrintString("Error");
	}
	
	return 0;
}
