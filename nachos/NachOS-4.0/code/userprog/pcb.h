#ifndef PCB_H
#define PCB_H

#include "thread.h"
#include "synch.h"
#include "utility.h"
#include "addrspace.h"
class PCB
{
	private:
		Semaphore* joinsem; // semaphore for join process
		Semaphore* exitsem; // semaphore for exit process
		Semaphore* multex; // exclusive access semaphore
		int exitcode;
		int numwait; // the number of join process
	public:
		int parentID; // The parent process’s ID
		PCB(int id); // Constructor
		~PCB(); // Destructor
		// Load the program has the name is “filename” and the process id is pid
		int Exec(char* name,int pid); //Create a thread with the name is filename and the process id is pid
		int GetID(); // Return the PID of the current process
		int GetNumWait(); // Return the number of the waiting process
		void JoinWait(); // The parent process wait for the child process finishes
		void ExitWait(); // The child process finishes
		void JoinRelease(); // The child process notice the parent process
		void ExitRelease(); // The parent process accept to exit the child process
		void IncNumWait(); // Increase the number of the waiting process
		void DecNumWait(); // Decrease the number of the waiting process
		void SetExitCode(int); // Set the exit code for the process
		int GetExitCode(); // Return the exitcode
		void SetFileName(char*); // Set the process name
		char* GetFileName(); // Return the process name
};

#endif
