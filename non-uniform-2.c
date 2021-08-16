#include <stdio.h>
#include <stdlib.h>
#include "kfifo.h"

#define FIFO_LEN	(1024*1024)	//1MB, must be power of 2

int main(int argc, char *argv[])
{
	struct kfifo_rec_ptr_2 fifo; // every record can has a maximum 65536 bytes length
	char * fifo_buf;
	unsigned int i, val, len;
	char  buf[100];

	fifo_buf = malloc(FIFO_LEN);

	kfifo_init(&fifo, fifo_buf, FIFO_LEN);	

	len = kfifo_size(&fifo);
	printf("The size of fifo: %u bytes\n", len);

	strcpy(buf, "hello");
	kfifo_in(&fifo, buf, strlen(buf) + 1);

	strcpy(buf, "hi");
	kfifo_in(&fifo, buf, strlen(buf) + 1);

	strcpy(buf, "this is a variable length test");
	kfifo_in(&fifo, buf, strlen(buf) + 1);

	len = kfifo_avail(&fifo);
	printf("After insert 3 string, the available bytes of fifo: %u bytes\n", len);
	len = kfifo_len(&fifo);
	printf("\t used  bytes of fifo: %u bytes\n", len);

	while(len)
	{
		len = kfifo_out(&fifo, buf, sizeof(buf));
		printf("get a string: %s, len: %d\n", buf, len);
		len = kfifo_len(&fifo);
        	printf("\tAfter out 1 value, used  bytes of fifo: %u bytes\n", len);	
	}

	return 0;
}
