// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "syscall.h"
#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
//#include "start.o"

//----------------------------------------------------------------------
// StartUserProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartUserProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    ProcessAddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new ProcessAddrSpace(executable);    
    currentThread->space = space;

    delete executable;			// close file

    space->InitUserCPURegisters();		// set the initial register values
    space->RestoreStateOnSwitch();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

static void
algIntHandler(int dummy)
{
    TimeSortedWaitQueue *ptr;
    if (interrupt->getStatus() != IdleMode) {
        // Check the head of the sleep queue
        while ((sleepQueueHead != NULL) && (sleepQueueHead->GetWhen() <= (unsigned)stats->totalTicks)) {
           sleepQueueHead->GetThread()->Schedule();
           ptr = sleepQueueHead;
           sleepQueueHead = sleepQueueHead->GetNext();
           delete ptr;
        }
        //printf("[%d] Timer interrupt.\n", stats->totalTicks);
	//printf("Timer handler yielding!, schedAlg= %d\n", schedAlg);
	if ((schedAlg >= 7 && schedAlg <= 10) && ((stats->totalTicks - currentThread->runningStart) >= timeQuantum) && currentThread->currentlyRunning == true) {
	  //printf("Condition Fulfilled!, %d\n", stats->totalTicks - currentThread->runningStart);
	  NachOSThread *thread = currentThread;
	  if (thread->currentlyRunning == true && thread->runningStart != stats->totalTicks) {
	    thread->runningEnd = stats->totalTicks;
	    thread->previousBurst = thread->runningEnd - thread->runningStart;
	    totalBusyTime+= (thread->previousBurst);
	    
	    if (thread->previousBurst > maxBurst) {
	      maxBurst = thread->previousBurst;
	    }
	    if (thread->previousBurst < minBurst) {
	      minBurst = thread->previousBurst;
	    }
	    numBurst += 1;
	    
	    thread->predBurst = (thread->predBurst + thread->previousBurst) / 2;
	  }
	  
	  thread->currentlyRunning = false;
	  interrupt->YieldOnReturn();
	}
	if (schedAlg > 2)
	  interrupt->YieldOnReturn();
    }
}


void
ForkStartFunctionAgain (int dummy)
{
   currentThread->Startup();
   machine->Run();
}


NachOSThread*
makeThread(char *filename, unsigned thread_index)
{
  OpenFile *executable = fileSystem->Open(filename);
  ProcessAddrSpace *space;
  
  if (executable == NULL) {
    printf("Unable to open file %s\n", filename);
    return NULL;
  }
  space = new ProcessAddrSpace(executable);
  char threadName[12] = {'n', 'e', 'w', 't', 'h', 'r', 'e', 'a', 'd', ' ', thread_index + '0', '\0'}; 
  NachOSThread* newThread = new NachOSThread(" ");
  newThread->space = space;
  delete executable;			// close file
  
  space->InitUserCPURegisters();		// set the initial register values
  space->RestoreStateOnSwitch();		// load page table register
  newThread->SaveUserState();
  newThread->ResetReturnValue();
  newThread->AllocateThreadStack(ForkStartFunctionAgain, 0);
  return newThread;
}

void 
EnqueueExecutables(char *filename)
{
  OpenFile *listFile = fileSystem->Open(filename);
  int avgBurstTestLoop = 120, quantum;
  char line[80], exec[80];
  int res = 80, priority = -1, lineCursor=0, execCursor=0, p=0;
  
  res = listFile->Read(line, 2);

  schedAlg = line[lineCursor] - '0';
  if (schedAlg == 1 && (line[lineCursor+1] - '0' == 0)) {
    printf("Entered here\n");
    res = listFile->Read(line, 1);
    schedAlg = 10;
  } 
  //printf("This byte is: %c", line[lineCursor]);

  while(res!= 0) {
    lineCursor = 0;
    res = listFile->Read(line, 80);
    //printf("The line read is: %s", line);
    printf("Number of bytes read: %d\n", res);

    while(lineCursor != res) {
      if(line[lineCursor] == '\n') {
	if (p == 0 || priority == -1 || priority == 0)
	  priority = 100;
	
	NachOSThread* thread = makeThread(exec, thread_index);
	currentThread->AddPriority(thread, priority);

	thread->Schedule();
	
	printf("executable: %s, execCursor: %d, priority: %d\n", exec, execCursor, priority);
	for (int i=0; i<execCursor; i++)
	  {
	    //printf("%c" , exec[i]);
	    exec[i] = '\0';
	  }
	execCursor = 0;
	priority = -1;
	p = 0;
	
      }
      else if (line[lineCursor] == ' ') {
	p = 1;
	priority = 0;
	exec[execCursor] = '\0';
      }
      else if (p == 0) {
	exec[execCursor] = line[lineCursor];
	execCursor += 1;
      }
      else if (p == 1) {
	priority = priority*10 + (int) line[lineCursor] - '0';
      }
      lineCursor = lineCursor + 1;
    }
    printf("here I am \n");
  }
  //NachOSThread *nextThread = scheduler->FindNextThreadToRun();
  //if (nextThread != NULL) printf("we have a next thread. \n");
  //scheduler->Schedule(nextThread);
  //printf("cp1");
  int i, exitcode = 0;
  exitThreadArray[0] = true;
  //printf("cp2");
  // Find out if all threads have called exit
  for (i=0; i<thread_index; i++) {
    if (!exitThreadArray[i]) break;
  }
  beginExecTime = stats->totalTicks;
  switch(schedAlg){
  case 1:
    quantum=100;
    break;
  case 2:
    quantum=100;
    break;
  case 3:
    quantum=avgBurstTestLoop/4;
    break;
  case 4:
    quantum=avgBurstTestLoop/2;
    break;
  case 5:
    quantum=3*avgBurstTestLoop/4;
    break;
  case 6:
    quantum=30; //just a guess for now
    break;
  case 7:
    quantum=avgBurstTestLoop/4;
    break;
  case 8:
    quantum=avgBurstTestLoop/2;
    break;
  case 9:
    quantum=3*avgBurstTestLoop/4;
    break;
  case 10:
    quantum=30; //just a guess for now
    break;
  }
  timer = new Timer(algIntHandler, 0, false, quantum);
  currentThread->Exit(i==thread_index, exitcode);
}
// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
