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
#include "synch.h"

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
CallingJoin(Thread * thread)
{
	thread->Join();
}

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}


void
Joiner(Thread *joinee)
{
  currentThread->Yeild();
  currentThread->Yeild();

  printf("Waiting for the Joinee to finish executing.\n");

  currentThread->Yield();
  currentThread->Yield();

  // Note that, in this program, the "joinee" has not finished
  // when the "joiner" calls Join().  You will also need to handle
  // and test for the case when the "joinee" _has_ finished when
  // the "joiner" calls Join().

  joinee->Join();

  currentThread->Yield();
  currentThread->Yield();

  printf("Joinee has finished executing, we can continue.\n");

  currentThread->Yield();
  currentThread->Yield();
}

void
Joinee()
{
  int i;

  for (i = 0; i < 5; i++) {
    printf("Smell the roses.\n");
    currentThread->Yield();
  }

  currentThread->Yield();
  printf("Done smelling the roses!\n");
  currentThread->Yield();
}

void
ForkerThread()
{
  Thread *joiner = new Thread("joiner");  // will not be joined
  Thread *joinee = new Thread("joinee", 1);  // WILL be joined

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
  joinee->Fork((VoidFunctionPtr) Joinee, 0);

  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
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

//----------------------------------------------------------------------
// LockTest1
//----------------------------------------------------------------------

Lock *locktest1 = NULL;

void
LockThread1(int param)
{
    printf("L1:0\n");
    locktest1->Acquire();
    printf("L1:1\n");
    currentThread->Yield();
    printf("L1:2\n");
    locktest1->Release();
    printf("L1:3\n");
}

void
LockThread2(int param)
{
    printf("L2:0\n");
    locktest1->Acquire();
    printf("L2:1\n");
    currentThread->Yield();
    printf("L2:2\n");
    locktest1->Release();
    printf("L2:3\n");
}

void
LockTest1()
{
    DEBUG('t', "Entering LockTest1");

    locktest1 = new Lock("LockTest1");

    Thread *t = new Thread("one");
    t->Fork(LockThread1, 0);
    t = new Thread("two");
    t->Fork(LockThread2, 0);
}

/* (1) a thread that will be joined only is destroyed once Join has been called on it */
void
ThreadTest101()
{
	printf("%s", "--------------------------------\n");
	Thread *child = new Thread("child", 1);

	printf("%s", "1) Forked thread\n");
	child->Fork(DoNothing, 0);

	printf("%s", "2) Child Thread Finished - Waiting to be Destroyed\n");

	printf("%s", "3) Killing Time\n");
	Fibbonacci(39);

	printf("%s", "4) Time Killed, Calling Join on completed child\n");
	child->Join();

	printf("%s", "--------------------------------\n");
}

/* (2) if a parent calls Join on a child and the child is still executing, the parent waits */
void
ThreadTest102()
{
	printf("%s", "--------------------------------\n");

	printf("%s", "1) Forked thread\n");
	Thread *child = new Thread("child", 1);
	child->Fork(Fibbonacci, 39);

	printf("%s", "2) Child called Join \n");
	child->Join();

	printf("%s", "3) Child Finished -Back on Main thread\n");
	printf("%s", "--------------------------------\n");

}

/* (3) if a parent calls Join on a child and the child has finished executing, the parent does not block */
void
ThreadTest103()
{
	printf("%s", "--------------------------------\n");

	printf("%s", "1) Forked thread\n");
	Thread *child = new Thread("child", 1);
	child->Fork(Fibbonacci, 39);

	printf("%s", "2) Child called Join \n");
	child->Join();

	printf("%s", "3) Child Finished -Back on Main thread\n");
	printf("%s", "4) Back on Main thread - still printing...\n");
	printf("%s", "--------------------------------\n");
}

/* (4) a thread does not call Join on itself */
void
ThreadTest104()
{
	printf("%s", "--------------------------------\n");

	printf("%s", "1) Forked thread - Calling Join on itself - Failure Expected\n");
	Thread *child = new Thread("child", 1);
	child->Fork( (VoidFunctionPtr) CallingJoin, (int) child);

	printf("%s", "--------------------------------\n");
	
}

/* (5) Join is only invoked on threads created to be joined */
void
ThreadTest105()
{
	printf("%s", "--------------------------------\n");

	printf("%s", "1) Created Thread that is not meant to be joined\n");
	printf("%s", "2) Calling Join - Failure Expected");

	Thread *child = new Thread("child", 0);
	child->Fork( Fibbonacci, 39);
	child->Join();

	printf("%s", "--------------------------------\n");
}

/* (6) Join is only called on a thread that has forked */
void
ThreadTest106()
{
	printf("%s", "--------------------------------\n");

	printf("%s", "1) Created thread - Calling Join - Failure Expected\n");
	Thread *child = new Thread("child", 1);
	child->Join();
	printf("%s", "--------------------------------\n");

}

/* (7) Join is not called more than once on a thread (if it is, then this could easily lead to a segmentation fault because the child is likely deleted). */
void
ThreadTest107()
{
	printf("%s", "--------------------------------\n");

	printf("%s", "1) Forked thread\n");
	Thread *child = new Thread("child", 1);
	child->Fork(Fibbonacci, 39);

	printf("%s", "2) Child called Join \n");
	child->Join();

	printf("%s", "3) Calling Join a second time - Failure Expected\n");
	child->Join();

	printf("%s", "--------------------------------\n");

}

void
ThreadTest10()
{

}


void ThreadTest108()
{
}

// Child finishes before parent calls join
void ThreadTest109()
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
void ThreadTest110()
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
    case 2:
	LockTest1();
    case 101:
    	ThreadTest101();
	break;
    case 102:
    	ThreadTest102();
	break;
    case 103:
    	ThreadTest103();
	break;
    case 104:
    	ThreadTest104();
	break;
    case 105:
    	ThreadTest105();
	break;
    case 106:
    	ThreadTest106();
	break;
    case 107:
    	ThreadTest107();
	break;
    case 108:
    	ThreadTest108();
	break;
    case 109:
    	ThreadTest109();
	break;
    case 110:
    	ThreadTest110();
	break;
    case 111:
    	ForkerThread();
	break;
    default:
        printf("No test specified.\n");
        break;
    }
}

