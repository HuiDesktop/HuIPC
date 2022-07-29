HuIPC: Simple peer-to-peer packet-based IPC library using shared memory

hiEvent_*: OS-provided event system warpper supporting inter-processor. (event.h)
hiSharedMemory_*: OS-provided shared memory warpper (sharedMemory.h)
hiMQ_*: platform-independent peer-to-peer message queue supporting both in-process and inter-process (huMessageQueue.h)

How to use:
0. notice:
    * DO NOT transport and receive by using the SAME INSTANCE! Keep 2 channels instead.

1. create a channel:
    * hiMQ_create(void* ptr, uint32_t size)
        provide the buffer HuIPC can use, size should be the available byte count of the ptr.
    * hiMQ_createIPC(uint32_t size)
        provide the byte count you want available to IPC data size. May require larger size of shared memory.

2. (for IPC) get shared memory name
    This is not elegant, but anyway usable, right?
    createIPC() returns a pointer to hiMQInstance and a platform-based hiSharedMemory POINTER.
    For any platform implement, the hiSharedMemory pointer should directly point to the shared memory name.
    So this is the pointer to the name
    (const char *)(((hiMQInstance)inst) + 1)
    You can use the name in another processor and open the channel:
    hiMQ_openIPC(const char* name)

3. Send a packet:
    1. hiMQ_begin(self): Every session starts from this call.
    2. hiMQ_ensure(self, uint32_t size): Call this to ensure inst->current can obtain `size` byte long data.
    3. Write in inst->current and increase the pointer if you need to call hiMQ_ensure again
    4. hiMQ_end(self, uint32_t size, uint32_t setEvent):
        `size` tells HuIPC the remaining length of inst->current. (If you always keep inst->current pointing the end of the data, the size is 0)

4. Receive a packet:
    1. hiMQ_wait(self): Listen to events
    2. hiMQ_get(self): return 0 (no message) or (size - 4)
        e.g. 4: packet size = 0
    3. hiMQ_next(self): finish this packet and move to next packet.
        The return value has the same meaning as hiMQ_get
    *  Note that you must handle all the packets after calling  hiMQ_get()