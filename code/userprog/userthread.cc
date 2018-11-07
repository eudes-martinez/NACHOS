#ifdef CHANGED

#include "userthread.h"
#include "thread.h"
#include "addrspace.h"
#include "system.h"
#include "synch.h"

static void StartUserThread(void *argsStruct);
static int cptThread = 0;



/**
 * This function is called when the SC ThreadCreate is called.
 */
void do_ThreadCreate(int f, int arg) {
	cptThread++;
	DEBUG ('x', "[DEBUG] cptThread: %d\n", cptThread);
	
	ThreadArgs *TArgs = (ThreadArgs *) malloc(sizeof(ThreadArgs));
	TArgs->f = f;
	TArgs->arg = arg;

	Thread *newThread = new Thread("newThread");
	newThread->Start(StartUserThread, TArgs);
}



/**
 * This function is called at the start of a user thread.
 * Initialize MIPS registers and launch the interpreter.
 */
static void StartUserThread(void *argsStruct) {
	ThreadArgs* TArgs = (ThreadArgs*) argsStruct;
	int stacktopAdress = currentThread->space->AllocateUserStack(cptThread);

	for (int i = 0; i < NumTotalRegs; i++) {
		machine->WriteRegister(i, 0);
	}

	machine->WriteRegister(4, TArgs->arg); // Argument register
	DEBUG('x', "[DEBUG] Reg4: %d\n", TArgs->arg);
	machine->WriteRegister(PCReg, TArgs->f);
	DEBUG('x', "[DEBUG] PCReg: %d\n", TArgs->f);
	machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
	DEBUG('x', "[DEBUG] NextPCReg: %d\n", machine->ReadRegister(PCReg) + 4);
	machine->WriteRegister(StackReg, stacktopAdress);
	DEBUG('x', "[DEBUG] StackReg: %d\n", stacktopAdress);

	machine->Run();
}



void do_ThreadExit() {
	DEBUG ('x', "[DEBUG] cptThread Exit: %d\n", cptThread);
	if (cptThread > 0) {
		cptThread--;
	} else {
		interrupt->Halt();
	}
	currentThread->Finish();
}



#endif // CHANGED
