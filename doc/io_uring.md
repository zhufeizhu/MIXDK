## io_uring
采用IO_URING的原因是(本质上是使用poll的方式)
1. 利用了其polling的方式，能够最大化高速设备的速率。
2. 因为有Switch进行任务分发，采用多队列的方式进行处理调度更加方便。
3. 避免了同步方式下阻塞等待的时间。

https://segmentfault.com/a/1190000019361819?utm_source=tag-newest


## pmdk
libpmemblk provides an array of blocks in persistent memory (pmem) such that updates to a single block are atomic. 
This library is intended for applications **using direct access storage (DAX), which is storage that supports load/store access without paging blocks from a block storage device**

