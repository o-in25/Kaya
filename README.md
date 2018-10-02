# Kaya
The Kaya Operating System Project

The Kaya OS described below is similar to the T.H.E. system outlined by Dijkstra back in 1968[4]. Dijkstra’s paper described an OS divided into six layers. Each layer i was an ADT or abstract machine to layer i + 1; successively building up the capabilities of the system for each new layer to build upon. The OS described here also contains six layers, though the final OS is not as complete as Dijkstra’s.
Kaya is actually the latest instantiation of an older “learning” operating system design. Ozalp Babaoglu and Fred Schneider originally described this operating system, calling it the HOCA OS[3], for implemention on the Cornell Hypothetical Instruction Processor (CHIP)[2, 1]. Later, Renzo Davoli and Mauro Morsiani reworked HOCA, calling it TINA[6] and ICAROS[5], for implementation on the Microprocessor (without) Pipeline Stages (MPS)[7, 6].

## Level 0: The base hardware of μMPS2. ##
There are two versions of the μMPS hardware; the original μMPS emulator, and μMPS2, a 100% backwards compatible extension of μMPS with an improved GUI and multiprocessor support. While both emulators are still available (though the original μMPS is no longer supported), this guide assumes the use of μMPS2 due to its superior GUI and additional hardware features.

## Level 1: The additional services provided in ROM. ## 
This includes the services provided by the ROM-Excpt handler (i.e. processor state save and load), the ROM-TLB-Refill handler (i.e. searching PTE’s for matching entries and loading them into the TLB), and the additional ROM services/instructions (i.e. LDST, FORK, PANIC, and HALT). The μMPS2 Principles of Operation contains a complete description of both Level 0 and 1.

## Level 2: The Queues Manager (Phase 1). ## 
Based on the key operating systems concept that active entities at one layer are just data structures at lower layers, this layer supports the management of queues of structures; ProcBlk’s.


## Level 3: The Kernel (Phase 2). ##
This level implements eight new kernel-mode process management and synchronization primitives in addition to multiprogramming, a process scheduler, device interrupt handlers, and deadlock detection.

## Level 4: The Support Level (Phase 3). ## 
Level 3 is extended to support a system that can support multiple user-level cooperating processes that can request I/O and which run in their own virtual address space. Furthermore, this level adds user-level synchronization, and a pro- cess sleep/delay facility.

## Level 5: The File System (Phase 4) ## 
This level implements the abstraction of a flat file system by implementing primitives necessary to create, rename, delete, open, close, and modify files.

## Level 6: The Interactive Shell ## 
– why not?
