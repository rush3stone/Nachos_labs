// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
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
#include "machine.h" //Lab3 Exercise 2
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    //--------------- Lab3: PageFault ---------------------
    if(which == PageFaultException) {
        if(machine->tlb == NULL){ //pyq:TLB is empty means this exception was caused by PageFault
        //由于Nachos默认会把程序的所有代码和数据都加入内存，所以不会出现查询页表发生PageFault的情况；
            DEBUG('m', "=> Linear page table page fault.\n");
            ASSERT(FALSE); //报错
        }else{//
            DEBUG('T', "=> TLB Miss, Page Fault.\n");
            int BadVAddr = machine->ReadRegister(BadVAddrReg); //pyq: It has been saved in machine.cc/RaiseException()
            TLBHandler(BadVAddr);   
        }
        return; //处理完，返回
    }

    //--------------- SysCalls --------------------------------
    int type = machine->ReadRegister(2); //pyq: Syscall type is saved in number-2 register
    if (which == SyscallException) {
        if(type == SC_Halt){
            PrintTLBStatus(); // TLB Hit Rate
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
        }else if(type == SC_Exit) {
            ControlAddrSpaceWithExit(type); //pyq: handle space problem in a function
        }
        //其他syscall后补


        IncrementPCRegs(); //避免进入无限循环
                    //(系统调用是一种异常，但是发生异常时Nachos的PC不自增，这样就会一直执行此syscall)
        return;
    }//if-which

    printf("Unexpected user mode exception %d %d\n", which, type);
    ASSERT(FALSE);

}

//-------------------------------Exit syscall----------------------------------
//Exit syscall
void ControlAddrSpaceWithExit(int type) {
    if(type == SC_Exit) {
        
        PrintTLBStatus(); //Print out TLB current status

        //Hints from comments line36：4号寄存器保存传入的第一个参数，对于Exit来说，正常退出，会传回状态０
        int status = machine->ReadRegister(4);
        // currentThread->setExitStatus(status);   //Now we don't have this function
        if(status == 0) {
            DEBUG('S', "User Program Exit Correctly(status 0)\n");
        } else {
            DEBUG('S', "User Program Exit With Status %d\n", status);
        }

#ifdef USER_PROGRAM
        if (currentThread->space != NULL) {
#if USE_BITMAP 
            // Lab3 Ex4: BitMap Clear
            // If there are more than one thread, you should only clear your bits, not all!
            // Add a function:clearBitMap() to addrspace for clear
            currentThread->space->ClearBitMap();
#endif
            delete currentThread->space; //释放内存空间
            currentThread->space = NULL; 
        }
#endif // USER_PROGRAM
        currentThread->Finish();
    }//if

}//ControlAddrSpaceWithExit


//----------------------------- Lab3 Exercise2 & 3 --------------------------

// choose one from below methods
// #define TLB_FIFO TRUE
#define TLB_CLOCK TRUE  

//---------- print out TLB Miss Rate ---------------------------------
void
PrintTLBStatus(){
#ifdef USE_TLB
    DEBUG('T', "TLBSize=%d, TLB Miss: %d, TLB Hit: %d, Total Translation: %d, TLB Miss Rate: %.2lf%%\n",
            TLBSize, TLBMissCount, translateCount-TLBMissCount, translateCount, (double)(TLBMissCount*100)/(translateCount));
#endif
}


int TLBreplaceIdx = 0; //TLB pointer, for TLB update 


//------------ TLB Hanler ---------------------------------------------
void
TLBHandler(int VirtAddr) {
    unsigned int vpn;
    vpn = (unsigned)VirtAddr / PageSize; //获取页号
    //获取页面内容，TLB更新在下面用#ifdef操作
    TranslationEntry phyPage = machine->pageTable[vpn];
    if(!phyPage.valid){  //Lab3 Ex6 缺页中断
        DEBUG('m', "====> Truely Page Fault\n"); //标记为'm'
#ifdef USE_BITMAP        
        phyPage = PageFaultHandler(vpn); 
#endif        
    }

#if TLB_FIFO
    TLBasFIFO(phyPage);
#elif TLB_CLOCK
    TLBasClock(phyPage);
#else //test Lab3 Ex2　
    macine->tlb[TLBreplaceIdx] = phyPage;
    TLBreplaceIdx = (++TLBreplaceIdx) % 4; //TLBSize is 4
#endif

}//TLBHandler


//---------------Lab3-ex3 FIFO for TLB update ---------------------------
#ifdef TLB_FIFO
void
TLBasFIFO(TranslationEntry page) {
    int TLBreplaceIdx = -1;
    for(int i = 0; i < TLBSize; i++) { //查找空闲页
        if(machine->tlb[i].valid == FALSE) {
            TLBreplaceIdx = i;
            break;
        }
    }
    if(TLBreplaceIdx == -1) { //所有向前移动一位，即０处保存的页被覆盖
        TLBreplaceIdx = TLBSize - 1;
        for(int i = 0; i <= TLBSize-2; i++) {
            machine->tlb[i] = machine->tlb[i+1];
        }
    }
    machine->tlb[TLBreplaceIdx] = page; //更新
}    
#endif

//--------------Lab3-ex3 CLOCK for TLB update ---------------------------
#ifdef TLB_CLOCK
void
TLBasClock(TranslationEntry page) {
    while(1){
        TLBreplaceIdx = TLBreplaceIdx % TLBSize;
        if(machine->tlb[TLBreplaceIdx].valid == FALSE) { //空，直接加入
            break;
        } else{
             if(machine->tlb[TLBreplaceIdx].use) { //已使用,更改标记，扫描下一页
                machine->tlb[TLBreplaceIdx].use = FALSE;
                TLBreplaceIdx++;
            }else{ //找到，替换
                break;
            }
        }
    }//while
    machine->tlb[TLBreplaceIdx] = page;
    machine->tlb[TLBreplaceIdx].use = TRUE; //标记已使用
}
#endif


//--------------------------------------------------------------------------
// Lab3 Ex-7 按需调页
//　先找未修改的页面，若找到则直接替换
// 若未找到，则选择第一个已修改页面，置换，并写回磁盘；
//-------------------------------------------------------------------------
int 
DemandPageReplacement(int vpn){
    int pageFrame = -1;
    for(int selectVPN = 0; selectVPN < machine->pageTableSize; selectVPN++){
        if(machine->pageTable[selectVPN].valid) { 
            if(!(machine->pageTable[selectVPN].use)) { //未修改
                pageFrame = machine->pageTable[selectVPN].physicalPage; //替换
                break;
            }
        }
    }//for
    if(pageFrame == -1){
        for (int replaced_vpn = 0; replaced_vpn < machine->pageTableSize, replaced_vpn != vpn; replaced_vpn++) {
            if (machine->pageTable[replaced_vpn].valid) {
                machine->pageTable[replaced_vpn].valid = FALSE;
                pageFrame = machine->pageTable[replaced_vpn].physicalPage;

                // 写回磁盘
                OpenFile *vm = fileSystem->Open("VirtualMemory");
                vm->WriteAt(&(machine->mainMemory[pageFrame*PageSize]), PageSize, replaced_vpn*PageSize);
                delete vm; // close file
                break;
            }
        }
    }//if
    return pageFrame;
}//DemandPageReplacement


//------------------------------------------------------------------------------------
// Lab3 Exercise-6 缺页中断
//----------------------------------------------------------------------------------
#ifdef USE_BITMAP
TranslationEntry
PageFaultHandler(int vpn) {
    //查看内存有空页
    int emptyPageFrame = machine->bitmap1->Find();
    //调页
    if(emptyPageFrame == -1){
        emptyPageFrame = DemandPageReplacement(vpn);
    }
    machine->pageTable[vpn].physicalPage = emptyPageFrame;

    // 从虚拟内存加载页面
    DEBUG('a', "Demand paging: loading page from virtual memory!\n");
    OpenFile *vm = fileSystem->Open("VirtualMemory"); //VirtualMemory(文件)定义在addrspace.cc中
    vm->ReadAt(&(machine->mainMemory[emptyPageFrame*PageSize]), PageSize, vpn*PageSize);
    delete vm; // close the file

    // 更新页面的标记
    machine->pageTable[vpn].valid = TRUE;
    machine->pageTable[vpn].use = FALSE;
    machine->pageTable[vpn].dirty = FALSE;
    machine->pageTable[vpn].readOnly = FALSE;

    currentThread->space->PrintState(); // 输出内存情况（BitMap）
}
#endif //end BITMAP

//----------------------------------------------------------------------
// IncrementPCRegs
// 	Because when Nachos cause the exception. The PC won't increment
//  (i.e. PC+4) in Machine::OneInstruction in machine/mipssim.cc.
//  Thus, when invoking a system call, we need to advance the program
//  counter. Or it will cause the infinity loop.
//----------------------------------------------------------------------
// pyq: comments from lines14-15: Don't forget to increment the pc before returning. 
// (Or else you'll loop making the same system call forever!)
void IncrementPCRegs(void) {
    // Debug usage (pyq: update previous, current and next PC)
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg)); 
    // Advance program counter
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4); 
}

//--------------------------------------------------------------------------
