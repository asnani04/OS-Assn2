// stats.h 
//	Routines for managing statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "system.h"
#include "stats.h"

//----------------------------------------------------------------------
// Statistics::Statistics
// 	Initialize performance metrics to zero, at system startup.
//----------------------------------------------------------------------

Statistics::Statistics()
{
    totalTicks = idleTicks = systemTicks = userTicks = 0;
    numDiskReads = numDiskWrites = 0;
    numConsoleCharsRead = numConsoleCharsWritten = 0;
    numPageFaults = numPacketsSent = numPacketsRecvd = 0;
}

//----------------------------------------------------------------------
// Statistics::Print
// 	Print performance metrics, when we've finished everything
//	at system shutdown.
//----------------------------------------------------------------------

void
Statistics::Print()
{
  printf("Total CPU Busy Time: %d\n", totalBusyTime);
  printf("Total CPU Execution Time: %d\n", totalExecTime);
  CPUutil = (double) totalBusyTime * 100.0 / totalExecTime ;
  printf("CPU utilization: %lf\n", CPUutil);
  printf("Total Number of CPU Bursts: %d\n", numBurst);
  avgBurst = (double) totalBusyTime / numBurst;
  printf("Max, min and avg Bursts: %d, %d, %lf\n", maxBurst, minBurst, avgBurst);
  printf("Total Wait Time: %d, Number of Waits: %d\n" , totalWait, numWaits);
  avgWait = (double) totalWait / numWaits ;
  printf("Avg Wait Time: %lf\n", avgWait);
  
  printf("Total Thread Completion Time: %d\n", totThreadCom);
  printf("total threads completed: %d\n", threadsTot);
  avgThreadCom = (double) totThreadCom / threadsTot;
  printf("Avg Thread Completion Time: %lf\n", avgThreadCom);
  printf("Max, min Thread Completion Times: %d, %d\n", maxThreadCom, minThreadCom);
  varThreadCom = (double) sumSquares / threadsTot;
  printf("Variance of thread completion times: %lf\n", varThreadCom);

  printf("Ticks: total %d, idle %d, system %d, user %d\n", totalTicks, 
	idleTicks, systemTicks, userTicks);
    printf("Disk I/O: reads %d, writes %d\n", numDiskReads, numDiskWrites);
    printf("Console I/O: reads %d, writes %d\n", numConsoleCharsRead, 
	numConsoleCharsWritten);
    printf("Paging: faults %d\n", numPageFaults);
    printf("Network I/O: packets received %d, sent %d\n", numPacketsRecvd, 
	numPacketsSent);
}
