## 测试代码


使用blktrace抓取block io并使用blkparse作图
```shell
blktrace -d /dev/nvme0n1
blkparse -i nvme0n1 -d nvme0n1.blktrace.bin
iowatcher -t nvme0n1.blktrace.bin -o disk.svg
```
生成矢量图


## 功能测试
- [x] 小写nvm:实际写到nvm区域
- [x] 小写ssd:实际写到buffer区域
- [x] 大写ssd:先查询buffer并clear重复块的元数据 然后写到ssd区域
- [x] 小读nvm:从nvm区域中读：
- [x] 小读ssd:先查buffer再从ssd中读
- [x] 大读ssd:将buffer和ssd的读取结果合并
- [x] buffer迁移:buffer区读满以后迁移到ssd中
- [x] 应用重启后重新简历索引



## 测试结果

- block_size: 4k
- size: 4G
- rw: sequential write

是否预热 | 单队列 | 多队列 
--- | --- | --- 
否 | 6.5s  | 3.5s
是 | 3.5s  | 2s



- cmd: fio -filename=/dev/nvme0n1 -direct=1 -thread -rw=write -bs=16k -ioengine=psync -size=4g -iodepth=1 -numjobs=1 -group_reporting -name=test

块大小\线程数 | 1 | 2 | 4 | 8 | 16 | 24
 --- | --- | --- | --- | --- | --- | --- 
  4k | 0.31G | 0.6G | 1.23G | 2.3G | 2.4G | 2.4G
  8k | 0.55G | 1.1G | 2.2G | 2.9G | 3.0G | 3.0G 
  12k | 0.7G | 1.45G | 2.6G | 3G | 3G | 3G
  16k | 0.88G | 1.73G | 3G | 3G | 3G | 3G 
  24k | 1.12G | 2.4G | 3G | 3.1G | 3.1G | 3.1G 

  mixdk测试结果
  块大小 | 4k | 8k | 12k | 16K
   --- | --- | --- | --- | ---
  bw |  0.95G | 1.27G | 2.38G | 
  
buffer区测结果
写总大小 | 4M | 8M | 12M | 16M(未触发migrate) | 16M(触发migrate)
  --- | --- | --- | ---| --- | --- 
时间 | 7ms | 14ms | 19ms | 25ms | 83ms
带宽 | 570Mb/s | 570Mb/s | 630Mb/s | 640Mb/s | 192Mb/s

# 测试
1. 安装pmdk的一些依赖 执行如下命令
```shell
sudo apt install autoconf automake pkg-config libglib2.0-dev libfabric-dev pandoc libncurses5-dev
```
2. 安装编译环境
```shell
sudo apt install gcc g++
```
3. 下载pmdk
```shell
git clone https://github.com/pmem/pmdk
```
如果下载不了 就从https://github.com/pmem/pmdk/archive/master.zip下载
4. 编译
```shell
cd pmdk && make
```
如果make遇到问题 例如某些deb的版本较低 可以到http://mirrors.aliyun.com/ubuntu/pool/main/n/ndctl/下载对应的deb 然后使用dpkg -i进行安装
5. 安装
```shell
make install
```

## libpmemblk测试

一共写4G

块大小\时间(ms) | 4k | 8k | 12k | 16K | 24k | 32k
--- | --- | --- | --- | --- | --- | ---
cold | 55138 | 50333 | 48834 | 48243 | 47171 | 46563
hot |  13507 | 9589 |  8228 | 7535 | 6905 | 6580


## filebench测试
1. 安装filebench
https://blog.csdn.net/chongdajerry/article/details/79976660

2. 到/usr/local/share/filebench/workloads目录下通过
```shell
  filebench -f xxxx.f
```
执行对应的workloads 例如createfiles.f
```
set $dir=/root/test
set $nfiles=50000
set $meandirwidth=100
set $meanfilesize=16k
set $iosize=1m
set $nthreads=16

set mode quit firstdone

define fileset name=bigfileset,path=$dir,size=$meanfilesize,entries=$nfiles,dirwidth=$meandirwidth

define process name=filecreate,instances=1
{
  thread name=filecreatethread,memsize=10m,instances=$nthreads
  {
    flowop createfile name=createfile1,filesetname=bigfileset,fd=1
    flowop writewholefile name=writefile1,dsync,fd=1,iosize=$iosize
    flowop closefile name=closefile1,fd=1
  }
}

echo  "Createfiles Version 3.0 personality successfully loaded"

run 60
```
需要在write时加上dsync保证每次写的数据落盘


## 创建文件系统
ext4文件系统默认是分组的 虽然有flex_bg模式 默认让每16个组作为一个组群 但是实际上每个组群需要的元数据大小大致为(1+1+512) 加起来就是
16 * 514 * 4096 = 32.125M  可以使用packed_meta_blocks模式将所有的元数据压缩到整个硬盘的开始 在465.8 GiB的nvme0n1上 所有的元数据
加起来的大小为superblock + reserved superblock + gdt + reserved gdt + block bitmap + inode bitmap + inode table = (1928252 + 512) * 4k ≈ 7.35G

同时对于ext4来说存在lazy_init的问题 因此需要在挂载的时候添加选项 -E lazy_itable_init=0,lazy_journal_init=0来关闭懒加载
```shell
mke2fs -t ext4 -E lazy_itable_init=0,lazy_journal_init=0 packed_meta_blocks=1 /dev/nvme0n1
```

查看创建文件系统的布局
```shell
debugfs -R stats /dev/nvme0n1
```
结果如[ext4布局](layout.txt)所示 可以看到
1. 一共有3727各组 每个组占的blk bitmap占1block,inode bitmap占1block,inode table占512 bloc
2. 使用packed元数据模式下第一个group的block bitmap在第1084块上 inode bitmap在第4811块上 inode table的第一块在第8538块上 最后一个group 3726的block bitmap在第4810块上
inode bitmap在第8537块上 inode table的第一块在第1928252块上
3. 根据计算 (4810 - 1084 + 1) = 3727  (8537 - 4811 + 1) = 3727 (1928252 - 8538 + 512) = 512 * 3727都是符合的

block bitmap  inode bitmap    inode table
  1084-4810     4811-8537    8538-1928764

## 处理trace
通过blkparse的format和filter来预处理trace
```shell
blkparse nvme0n1 -f "%a,%d,%S,%n\n" -a queue -o output.txt 
```
这样过滤掉所有的非队列操作 显示的信息包括
```
a   Action, a (small) string (1 or 2 characters) -- see table below for more details
d   RWBS field, a (small) string (1-3 characters)  -- see section below for more details
S   Sector number
n   Number of blocks
```