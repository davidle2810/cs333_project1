#include "ptable.h"
#include "synch.h"

PTable::PTable(int size)
{

    if (size < 0)
        return;

    psize = size;
    bm = new BitMap(size);
    bmsem = new Semaphore("bmsem",1);

    for (int i = 0; i < MAX_PROCESS; ++i){
		pcb[i] = 0;
    }

	bm->Mark(0);

	pcb[0] = new PCB(0);
	pcb[0]->SetFileName("./test/scheduler");
	pcb[0]->parentID = -1;
}

PTable::~PTable()
{
    if( bm != 0 )
	delete bm;
    
   for (int i = 0; i < psize; ++i){
		if(pcb[i] != 0)
			delete pcb[i];
    }
		
	if( bmsem != 0)
		delete bmsem;
}

int PTable::ExecUpdate(char* name)
{
	bmsem->P();
	
	if(name == NULL)
	{
		printf("\nPTable::Exec : Can't not execute name is NULL.\n");
		bmsem->V();
		return -1;
	}

	if( strcmp(name,"./test/scheduler") == 0 || strcmp(name,currentThread->getName()) == 0 )
	{
		printf("\nPTable::Exec : Can't not execute itself.\n");		
		bmsem->V();
		return -1;
	}

	int index = this->GetFreeSlot();

    // Check if have free slot
	if(index < 0)
	{
		printf("\nPTable::Exec :There is no free slot.\n");
		bmsem->V();
		return -1;
	}

	pcb[index] = new PCB(index);
	pcb[index]->SetFileName(name);

    	pcb[index]->parentID = currentThread->processID;

	int pid = pcb[index]->Exec(name,index);

	bmsem->V();

	return pid;
}

int PTable::JoinUpdate(int id)
{
	
	if (id < 0)
	{
		printf("\nPTable::JoinUpdate : id = %d", id);
		return -1;
	}
	// Check if process running is parent process of process which joins
	if (currentThread->processID != pcb[id]->parentID)
	{
		printf("\nPTable::JoinUpdate Can't join in process which is not it's parent process.\n");
		return -1;
	}

 
	pcb[pcb[id]->parentID]->IncNumWait();
	

	//pcb[id]->boolBG = 1;
	
	pcb[id]->JoinWait();

	
	int exitcode = pcb[id]->GetExitCode();
	pcb[id]->ExitRelease();

	return exitcode;
}
int PTable::ExitUpdate(int exitcode)
{              

	int id = currentThread->processID;
	if(id == 0)
	{
		
		currentThread->FreeSpace();		
		interrupt->Halt();
		return 0;
	}
    
        if(IsExist(id) == false)
	{
		printf("\nPTable::ExitUpdate: This %d is not exist. Try again?", id);
		return -1;
	}

	
	pcb[id]->SetExitCode(exitcode);
	pcb[pcb[id]->parentID]->DecNumWait();
    
   
	pcb[id]->JoinRelease();
	pcb[id]->ExitWait();
	
	Remove(id);
	return exitcode;
}

// Find free slot in order to save the new process infom
int PTable::GetFreeSlot()
{
	return bm->Find();
}

// Check if Process ID is Exist
bool PTable::IsExist(int pid)
{
	return bm->Test(pid);
}

// Remove proccess ID out of table
// When it ends
void PTable::Remove(int pid)
{
	bm->Clear(pid);
	if(pcb[pid] != 0)
		delete pcb[pid];
}

char* PTable::GetFileName(int id)
{
	return (pcb[id]->GetFileName());
}
