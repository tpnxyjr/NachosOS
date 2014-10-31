// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    name = debugName;
    queue = new List;
lock = false;	
}
Lock::~Lock() {
	delete queue;
}
bool Lock::test_and_set (bool *flag) {
	bool old = *flag;
	*flag = true;
	return old;
}
bool Lock::isHeldByCurrentThread(){
	return lock;
}
void Lock::Acquire(){

    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts


    while (test_and_set(&lock)) { 
        queue->Append((void *)currentThread);   // so go to sleep
        currentThread->Sleep();
    }
	lock = true;
    (void) interrupt->SetLevel(oldLevel);       // re-enable interrupts

}

void Lock::Release() {
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    lock = false;

    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    (void) interrupt->SetLevel(oldLevel);

}

Condition::Condition(char* debugName) {
	name = debugName;
	value = 0;
	amount = 0;
	queue = new List;
 }
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock) {
   conditionLock->Acquire(); 
	value = 0;
	amount++;
	while(value != 1){
		queue->Append((void *)currentThread);
		currentThread->Sleep();
	}
   conditionLock->Release();
}
void Condition::Signal(Lock* conditionLock) { 
    conditionLock->Acquire();
    amount--;
    Thread *thread;
    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    conditionLock->Release();
}
void Condition::Broadcast(Lock* conditionLock) {
    conditionLock->Acquire();
    value = 1;
    int i = 0;
    Thread *thread;
    for(i = 0; i < amount; i++){
        thread = (Thread *)queue->Remove();
        if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    }
    amount = 0;
    conditionLock->Release();

}

Mailbox::Mailbox(char* debugName){
        name = debugName;
        sent = false;
        received = false;
	cond = new Condition("a");
	lock = new Lock("a");
}
Mailbox::~Mailbox(){}
void Mailbox::Send(int message){
        sent = true;
        while(!received){
        	cond->Wait(lock); 
        }
        buffer = message;
	cond->Signal(lock);
}
void Mailbox::Receive(int * message){
        received = true;
        while(!sent){
		cond->Wait(lock);
        }
	cond->Signal(lock);
}

Whale::Whale(char* debugName){
	name = debugName;
	males = 0;
	females = 0;
	matchmakers = 0;
	maleQueue = new List;
	femaleQueue = new List;
	matchQueue = new List;
}
Whale::~Whale(){
	delete maleQueue;
	delete femaleQueue;
	delete matchQueue;
}
void Whale::Male(){
	males++;
	while(femaleQueue == 0 || matchQueue == 0){
                maleQueue->Append((void *)currentThread);
                currentThread->Sleep();

	}
	Thread *thread;
	thread = (Thread *) maleQueue->Remove();
	if(thread != NULL) scheduler->ReadyToRun(thread);
	males--;
}
void Whale::Female(){
	females++;
        while(maleQueue == 0 || matchQueue == 0){
                femaleQueue->Append((void *)currentThread);
                currentThread->Sleep();

        }
        Thread *thread;
        thread = (Thread *) femaleQueue->Remove();
        if(thread != NULL) scheduler->ReadyToRun(thread);
        females--;

}
void Whale::Matchmaker(){
        matchmakers++;
        while(femaleQueue == 0 || maleQueue == 0){
                matchQueue->Append((void *)currentThread);
                currentThread->Sleep();

        }
        Thread *thread;
        thread = (Thread *) matchQueue->Remove();
        if(thread != NULL) scheduler->ReadyToRun(thread);
        matchmakers--;

}
