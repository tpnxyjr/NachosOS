// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

void DoNothing(int n)
{
	printf("%s%s", "X) Nothing Thread Complete", "\n");

}

int FibbonacciHelper(int n)
{
	if(n <= 1)
		return n;
	else
		return FibbonacciHelper(n-1) + FibbonacciHelper(n-2);

}

void Fibbonacci(int n)
{
	FibbonacciHelper(n);
//	printf("%s%s", "X) Child Complete ", "\n");
}

void CreateAThread(int n)
{
	if(n == 0)
	{
		Thread *child = new Thread("child", 1);
		printf("%s", "2) Forked thread\n");
		child->Fork(CreateAThread, 1);
		child->Join();
		printf("%s", "5) Join Complete\n");

	}
	else if (n==1)
	{
		printf("%s", "3) Computing Fib\n");
		Fibbonacci(39);
		printf("%s", "4) Fib Complete\n");
	}
	
}


//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}


void ThreadTest100()
{
	printf("%s", "--------------------------------\n");

	Thread *child = new Thread("child", 1);
	child->Fork(Fibbonacci, 39);
	printf("%s", "1) Forked thread\n");
	child->Join();
	printf("%s", "3) Back on Main thread\n");
	printf("%s", "--------------------------------\n");
}

void ThreadTest101()
{
	printf("%s", "--------------------------------\n");
	Thread *child = new Thread("child", 1);

	printf("%s", "1) Forked thread\n");
	child->Fork(DoNothing, 0);

	printf("%s", "2) Killing Time\n");
	Fibbonacci(39);

	printf("%s", "3) Time Killed, Calling Join on completed child\n");
	child->Join();

	printf("%s", "--------------------------------\n");

}

// Testing nested joins
void ThreadTest102()
{
	printf("%s", "--------------------------------\n");
	Thread *child = new Thread("child", 1);
		
	printf("%s", "1) Forked thread\n");
	child->Fork(CreateAThread, 0);
	child->Join();
	printf("%s", "6) Join Complete\n");



	printf("%s", "--------------------------------\n");
}





//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
        ThreadTest1();
        break;
    case 100:
    	ThreadTest100();
	break;
    case 101:
    	ThreadTest101();
	break;
    case 102:
    	ThreadTest102();
	break;
    default:
        printf("No test specified.\n");
        break;
    }
}

