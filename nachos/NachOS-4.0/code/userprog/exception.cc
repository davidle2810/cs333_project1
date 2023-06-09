// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"

#define MaxFileLength 32
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------
void PCIncrement()
{
	int counter = kernel->machine->ReadRegister(PCReg);
	kernel->machine->WriteRegister(PrevPCReg, counter);
	counter = kernel->machine->ReadRegister(NextPCReg);
	kernel->machine->WriteRegister(PCReg, counter);
	kernel->machine->WriteRegister(NextPCReg, counter+4);
}

char* User2System(int virtAddr, int limit)
{
	int i;
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit+1];
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit+1);

	for (i = 0; i<limit; ++i)
	{
		kernel->machine->ReadMem(virtAddr+i,1,&oneChar);
		kernelBuf[i]=(char)oneChar;
		if (oneChar==0)
			break;
	}
	return kernelBuf;
}
int System2User(int virtAddr,int len,char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0 ;
	do
	{
		oneChar= (int) buffer[i];
		kernel->machine->WriteMem(virtAddr+i,1,oneChar);
 		i ++;
	}
	while(i < len && oneChar != 0);
	return i;
} 
void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);
	int size;
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) 
	{
	case NoException:
		return;

	case PageFaultException:
		DEBUG('a', "\n No valid translation found");
		printf("\n\n No valid translation found");
		SysHalt();
		break;

	case ReadOnlyException:
		DEBUG('a', "\n Write attempted to page marked read-only");
		printf("\n\n Write attempted to page marked read-only");
		SysHalt();
		break;

	case BusErrorException:
		DEBUG('a', "\n Translation resulted invalid physical address");
		printf("\n\n Translation resulted invalid physical address");
		SysHalt();
		break;

	case AddressErrorException:
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space");
		SysHalt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		SysHalt();
		break;

    case SyscallException:
		switch(type) 
		{
    	case SC_Halt:
				DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
				printf("\nShutdown, initiated by user program. ");
				SysHalt();

				ASSERTNOTREACHED();
				break;

			case SC_Create:
			{
				int virtAddr;
				char* filename;

				virtAddr = kernel->machine->ReadRegister(4); 
				filename = User2System(virtAddr, 32);

				if (strlen(filename) == 0)
				{
					printf("\n File name is not valid");
					kernel->machine->WriteRegister(2, -1); 
					delete[] filename;
					break;
				}
			
				if (filename == NULL)  
				{
					printf("\n Not enough memory in system");
					kernel->machine->WriteRegister(2, -1);
					delete[] filename;
					break; 
				}
			
				if (!kernel->fileSystem->Create(filename)) 
				{
					printf("\n Error create file '%s'", filename);
					kernel->machine->WriteRegister(2, -1);
					delete filename;
					break;
				}
				printf("\n Create file '%s' success", filename);
				kernel->machine->WriteRegister(2, 0);
				delete[] filename;
				//break;
				PCIncrement(); return;
			}
			case SC_Open:
			{
				int virtAddr = kernel->machine->ReadRegister(4); 
				int type = kernel->machine->ReadRegister(5); 
				char* filename = User2System(virtAddr, 32); 
				int freeSlot = kernel->fileSystem->findFreeSlot();
				if (freeSlot != -1) 
				{
					if (type == 0 || type == 1) 
					{
						if ((kernel->fileSystem->openfile[freeSlot] = kernel->fileSystem->Open(filename, type)) != NULL) 
							kernel->machine->WriteRegister(2, freeSlot); 
					}
					else if (type == 2) 
						kernel->machine->WriteRegister(2, 0); 
					else 
						kernel->machine->WriteRegister(2, 1); 
					delete[] filename;
					PCIncrement();
					return;
				}
				kernel->machine->WriteRegister(2, -1); 
				delete[] filename;
				//break;
				PCIncrement();
				return;
				}

			case SC_Close:
			{
				int id = kernel->machine->ReadRegister(4); 
				if (id >= 2 && id <= 20) 
					if (kernel->fileSystem->openfile[id])
					{
						delete kernel->fileSystem->openfile[id]; 
						kernel->fileSystem->openfile[id] = NULL; 
						kernel->machine->WriteRegister(2, 0);
						PCIncrement();
						return;
						//break;
					}
				kernel->machine->WriteRegister(2, -1);
				//break;
				PCIncrement();
				return;
			}

			case SC_Read:
			{
				int virtAddr = kernel->machine->ReadRegister(4); 
				int charcount = kernel->machine->ReadRegister(5); 
				int id = kernel->machine->ReadRegister(6); 
				int OldPos;
				int NewPos;
				char *buf;
				if (id < 0 || id > 19)
				{
					printf("\n Read fail, ID out of range");
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				if (kernel->fileSystem->openfile[id] == NULL)
				{
					printf("\n Read fail, null");
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				if (kernel->fileSystem->openfile[id]->fileOpen->type == 3) 
				{
					printf("\n Read fail, stdout");
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				OldPos = kernel->fileSystem->openfile[id]->fileOpen->GetCurrentPos(); 
				buf = User2System(virtAddr, charcount); 
				if (kernel->fileSystem->openfile[id]->fileOpen->type == 2)
				{
					int size = 0;
					char chr, *t = new char[charcount+1];
 					while (size < charcount)
					{
						chr = kernel->synchConsoleIn->GetChar();
						
						if (chr=='\n'||chr=='\0'||chr==0||chr==EOF)
							break;
						t[size]=chr;
						size++;
					}
					t[size]='\0';
					buf=t;
					System2User(virtAddr, size, buf);
					printf("\n Read successfully");
					kernel->machine->WriteRegister(2, size); 
					delete[] buf, t, chr;
					PCIncrement();
					return;
				}
			
				if ((kernel->fileSystem->openfile[id]->fileOpen->Read(buf, charcount)) > 0)
				{
					NewPos = kernel->fileSystem->openfile[id]->fileOpen->GetCurrentPos();
					System2User(virtAddr, NewPos - OldPos, buf); 
					printf("\n Read successfully");
					kernel->machine->WriteRegister(2, NewPos - OldPos);
				}
				else
				{
					kernel->machine->WriteRegister(2, -1);
				}
				delete buf;
				PCIncrement();
				return;
			}

			case SC_Write:
			{
				int virtAddr = kernel->machine->ReadRegister(4); 
				int charcount = kernel->machine->ReadRegister(5); 
				int id = kernel->machine->ReadRegister(6); 
				int OldPos;
				int NewPos;
				char *buf;
				if (id < 0 || id > 19)
				{
					printf("\n Write fail, out of range");
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				if (kernel->fileSystem->openfile[id] == NULL)
				{
					printf("\n Write fail,null");
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				if (kernel->fileSystem->openfile[id]->fileOpen->type == 1 || kernel->fileSystem->openfile[id]->type == 2)
				{
					printf("\n Write fail, stdin or read only");
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				OldPos = kernel->fileSystem->openfile[id]->GetCurrentPos();
				buf = User2System(virtAddr, charcount);  
				if (kernel->fileSystem->openfile[id]->fileOpen->type == 0)
				{
					if ((kernel->fileSystem->openfile[id]->fileOpen->Write(buf, charcount)) > 0)
					{
						NewPos = kernel->fileSystem->openfile[id]->fileOpen->GetCurrentPos();
					printf("\n Write successfully");
						kernel->machine->WriteRegister(2, NewPos - OldPos);
						delete buf;
						PCIncrement();
						return;
					}
				}
				if (kernel->fileSystem->openfile[id]->fileOpen->type == 3) 
				{
					int i = 0;
					while (buf[i] != 0 && buf[i] != '\n') 
					{
						kernel->synchConsoleOut->PutChar(buf[i]); 
						i++;
					}
					buf[i] = '\n';
					kernel->synchConsoleOut->PutChar(buf[i]);
					kernel->machine->WriteRegister(2, i - 1); 
					printf("\n Write successfully");
					delete buf;
					PCIncrement();
					return;
				}
			}

			case SC_Seek:
			{
				int pos = kernel->machine->ReadRegister(4); 
				int id = kernel->machine->ReadRegister(5); 
				if (id < 0 || id > 19)
				{
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				if (kernel->fileSystem->openfile[id] == NULL)
				{
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				if (id == 0 || id == 1)
				{
					kernel->machine->WriteRegister(2, -1);
					PCIncrement();
					return;
				}
				pos = (pos == -1) ? kernel->fileSystem->openfile[id]->fileOpen->Length() : pos;
				if (pos > kernel->fileSystem->openfile[id]->fileOpen->Length() || pos < 0)
					kernel->machine->WriteRegister(2, -1);
				else
				{
					kernel->fileSystem->openfile[id]->fileOpen->Seek(pos);
					kernel->machine->WriteRegister(2, pos);
				}
				PCIncrement();
				return;
			}
			
			case SC_Remove:
			{
				int virtAddr=kernel->machine->ReadRegister(4);
				char* filename;
				filename=User2System(virtAddr, 32);
				if (strlen(filename) == 0)
				{
					kernel->machine->WriteRegister(2, -1); 
					delete[] filename;
					break;
				}

				if(!kernel->fileSystem->Remove(filename))
					kernel->machine->WriteRegister(2,-1);
				else 
					kernel->machine->WriteRegister(2,0);
				delete filename;
				break;
				
			}

			case SC_Exec:
			{
				int virtAddr;
				virtAddr = kernel->machine->ReadRegister(4);	
				char* name;
				name = User2System(virtAddr, MaxFileLength + 1); 	
				if(name == NULL)
				{
					DEBUG('a', "\n Not enough memory in System");
					printf("\n Not enough memory in System");
					kernel->machine->WriteRegister(2, -1);
					return;
				}
				OpenFile *openFile = kernel->fileSystem->Open(name);
				if (openFile == NULL)
				{
					printf("\nExec:: Can't open this file.");
					kernel->machine->WriteRegister(2,-1);
					PCIncrement();
					return;
				}

				delete openFile;

				// Return child process id
				int id = pTab->ExecUpdate(name); 
				kernel->machine->WriteRegister(2,id);

				delete[] name;	
				PCIncrement();
				return;
		}

		case SC_Join:
		{ 
			int id = machine->ReadRegister(4);
			int result = pTab->JoinUpdate(id);			
			kernel->machine->WriteRegister(2, result);
			PCIncrement();
			return;
		}
		case SC_Exit:
		{
			int exitStatus = kernel->machine->ReadRegister(4);
			if(exitStatus != 0)
			{
				PCIncrement();
				return;
			}			
			
			int result = pTab->ExitUpdate(exitStatus);
			currentThread->FreeSpace();
			currentThread->Finish();
			PCIncrement();
			return; 
				
		}
		case SC_CreateSemaphore:
		{
			int virtAddr = kernel->machine->ReadRegister(4);
			int semval = kernel->machine->ReadRegister(5);
			char *name = User2System(virtAddr, MaxFileLength + 1);
			if(name == NULL)
			{
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				PCIncrement();
				return;
			}
			
			int result = semTab->Create(name, semval);

			if (result == -1)
			{
				printf("\n Can not create semaphore");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				PCIncrement();
				return;				
			}
			
			delete[] name;
			kernel->machine->WriteRegister(2, result);
			PCIncrement();
			return;
		}

		case SC_Wait:			
		{
			int virtAddr = kernel->machine->ReadRegister(4);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if(name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				PCIncrement();
				return;
			}
			
			int result = semTab->Wait(name);

			if(result == -1)
			{
				printf("\n Semaphore does not exist");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				PCIncrement();
				return;				
			}
			
			delete[] name;
			kernel->machine->WriteRegister(2, result);
			PCIncrement();
			return;
		}
		
		case SC_Signal:		
		{
			int virtAddr = kernel->machine->ReadRegister(4);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if(name == NULL)
			{
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				PCIncrement();
				return;
			}
			
			int result = semTab->Signal(name);

			if (result == -1)
			{
				printf("\n Semaphore does not exist");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				PCIncrement();
				return;				
			}
			
			delete[] name;
			kernel->machine->WriteRegister(2, result);
			PCIncrement();
			return;
		}
	
			case SC_Add:
			{
				DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
				/* Process SysAdd Systemcall*/
				int result;
				result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),/* int op2 */(int)kernel->machine->ReadRegister(5));

				DEBUG(dbgSys, "Add returning with " << result << "\n");
				/* Prepare Result */
				kernel->machine->WriteRegister(2, (int)result);
	
				/* Modify return point */
				{
	  			/* set previous programm counter (debugging only)*/
	  			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  			/* set next programm counter for brach execution */
	  			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
				}

				return;
				ASSERTNOTREACHED();
				break;
			}
		   
			default:
				cerr << "Unexpected system call " << type << "\n";
				break;
      	}
		PCIncrement();
    	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
    }
    ASSERTNOTREACHED();
	return;
}
