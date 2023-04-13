#ifndef STABLE_H
#define STABLE_H
#include "synch.h"
#include "bitmap.h"

#define MAX_SEMAPHORE 10
#endif
class Sem
{
	private:
		char name[50]; // The semaphore name
		Semaphore* sem; // Create semaphore for management
	public:
		// Initial the Sem object, the started value is null
		// Remember to initial the Sem to use
		Sem(char* na, int i)
		{
			strcpy(this->name, na);
			sem = new Semaphore(this->name, i);
		}
		~Sem() // Destruct the Sem object
		{
			if(sem)
			delete sem;
		}
		void wait()
		{
			sem->P(); // Conduct the waiting function
		}
		void signal()
		{
			sem->V(); // Release semaphore
		}
		char* GetName() // Return the semaphore name
		{
			return this->name;
		}
}
class STable
{
	private:
		BitMap* bm; // Manage the free slot
		Sem* semTab[MAX_SEMAPHORE];
	public:
	// Initial the Sem object, the started value is null
	// Remember to initial the bm object to use
		STable();
		~STable();
	// Check the semaphore name, create a new one if not already
		int Create(char *name, int init);
	// If the semaphore name already exists, call this->P() to execute it. If not, report an error in Wait, Signal function
		int Wait(char *name);
		int FindFreeSlot(int id); // Find an empty slot
};
