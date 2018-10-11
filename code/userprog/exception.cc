// exception.cc
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}


//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions
//      are in machine.h.
//----------------------------------------------------------------------

#ifdef CHANGED

/** 
 * @return int Nb wrote char.
 */
static int copyStringFromMachine(int from, char *to, unsigned int size) {
	unsigned int i;
	int ch;
	for(i = 0; i < size - 1; i++) {
		machine->ReadMem(from + i, 1, &ch);
		if ((char)ch == '\0') break;
		to[i] = (char)ch;
	}
	to[i] = '\0';
	return i;
}

/**
 * @return int Nb read char.
 */
static int copyStringToMachine(char *from, int to, unsigned int size) {
	unsigned int i;
	for(i = 0; i < size - 1; i++) {
		machine->WriteMem(to + i, 1, (int)from[i]);
		if (from[i] == '\0') break;
	}
	machine->WriteMem(to + i, 1, (int)'\0');
	return i;
}

#endif // CHANGED


void ExceptionHandler (ExceptionType which) {

	int type = machine->ReadRegister(2);

	switch (which) {
		case SyscallException:
		{
			switch (type) {
				case SC_Halt:
				{
					DEBUG('s', "Shutdown, initiated by user program.\n");
					interrupt->Halt();
					break;
				}
				#ifdef CHANGED
				case SC_PutChar:
 				{
					DEBUG('s', "Debug PutChar\n");
					int c = machine->ReadRegister(4);
					synchconsole->SynchPutChar(c);
					break;
				}
				case SC_GetChar:
				{
					DEBUG('s', "Debug GetChar\n");
					int c = synchconsole->SynchGetChar();
					// Put the char in r2 (The register used for the return of a function)
					machine->WriteRegister(2, c);
					break;
				}
				case SC_PutString:
				{
					DEBUG('s', "Debug PutString\n");
					char *buffer = new char[MAX_STRING_SIZE];

					int str = machine->ReadRegister(4);
					int shift = 0;
					int loop = 0;
					int nb_write;
					do {
						nb_write = copyStringFromMachine(str + shift, buffer, MAX_STRING_SIZE);
						synchconsole->SynchPutString((const char *)buffer);
						loop++;
						shift = loop * sizeof(char) * (MAX_STRING_SIZE-1);
					} while (nb_write == (MAX_STRING_SIZE-1));

					delete buffer;
					break;
				}
				case SC_GetString:
				{
					DEBUG('s', "Debug GetString\n");
					char *buffer = new char[MAX_STRING_SIZE];
					int s = machine->ReadRegister(4);
					unsigned int n = machine->ReadRegister(5);

					int shift = 0;
					unsigned int remain_char = n;
					int nb_read;

					for (unsigned int i = 0; i < (n / MAX_STRING_SIZE) + 1; i++) {
						remain_char -= (MAX_STRING_SIZE-1);
						synchconsole->SynchGetString(buffer, MAX_STRING_SIZE);
						nb_read = copyStringToMachine(buffer, s + shift, MAX_STRING_SIZE);
						shift = (i+1) * sizeof(char) * (MAX_STRING_SIZE-1);
						if (remain_char <= 0 || (nb_read != MAX_STRING_SIZE-1)) break;
					}

					delete buffer;
					break;
				}
				case SC_Exit:
				{
					int main_ret = machine->ReadRegister(4);
					DEBUG('s', "Debug main return with int %d\n", main_ret);
					int status = machine->ReadRegister(2);
					DEBUG('s', "Debug Exit with code %d\n", status);
					interrupt->Halt();
					break;
				}
				#endif
				default:
				{
					printf("Unimplemented system call %d\n", type);
					ASSERT(FALSE);
				}
			}
			UpdatePC(); // Do not forget to increment the pc before returning!
			break;
		}

		case PageFaultException:
			if (!type) {
				printf("NULL dereference at PC %x!\n", machine->registers[PCReg]);
				ASSERT (FALSE);
			} else {
				printf ("Page Fault at address %x at PC %x\n", type, machine->registers[PCReg]);
				ASSERT (FALSE);	// For now
			}
		default:
			printf ("Unexpected user mode exception %d %d at PC %x\n", which, type, machine->registers[PCReg]);
			ASSERT (FALSE);
	}
}
