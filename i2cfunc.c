/*******************************
 * i2cfunc.c v3
 * wrapper for I2C functions
 * rev 3.1 June 2019 shabaz
 *******************************/

#include <stdio.h> 
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include "i2cfunc.h"

typedef struct {
  uint32_t C;
  uint32_t S;
  uint32_t DLEN;
  uint32_t A;
  uint32_t FIFO;
  uint32_t DIV;
  uint32_t DEL;
  uint32_t CLKT;
} i2c_reg_map_t;

int i2c_set_timeout(int val)
{
  int handle;
  void* addr;
  i2c_reg_map_t *i2c_mem;

  handle=open("/dev/mem", O_RDWR | O_SYNC);
  if (handle<0)
  {
    printf("error opening /dev/mem!\n");
    return(1);
  }
  addr=mmap(NULL, 32, PROT_READ | PROT_WRITE, MAP_SHARED, handle, I2C_BASE);
  if (addr == MAP_FAILED)
  {
    printf("error performing mmap!\n");
    close(handle);
    return(1);
  }
  i2c_mem=(i2c_reg_map_t *)addr;
  i2c_mem->CLKT=val;
  munmap(addr, 32);
  close(handle);
  return(0);
}

int i2c_open(unsigned char bus, unsigned char addr)
{
  int file;
  char filename[16];
  sprintf(filename,"/dev/i2c-%d", bus);
  if ((file = open(filename,O_RDWR)) < 0)
  {
    fprintf(stderr, "i2c_open open error: %s\n", strerror(errno));
    return(file);
  }
  if (ioctl(file,I2C_SLAVE,addr) < 0)
  {
    fprintf(stderr, "i2c_open ioctl error: %s\n", strerror(errno));
    return(-1);
  }
  return(file);
}

int i2c_write(int handle, unsigned char* buf, unsigned int length)
{
  int ret;
  ret = write(handle, buf, length);
  if (ret != (int)length)
  {
    fprintf(stderr, "i2c_write error%d: %s\n", errno, strerror(errno));
    return(-1);
  }
  return(length);
}

int i2c_write_byte(int handle, unsigned char val)
{
  if (write(handle, &val, 1) != 1)
  {
    fprintf(stderr, "i2c_write_byte error: %s\n", strerror(errno));
    return(-1);
  }
  return(1);
}

int i2c_read(int handle, unsigned char* buf, unsigned int length)
{
  if (read(handle, buf, length) != (int)length)
  {
    fprintf(stderr, "i2c_read error: %s\n", strerror(errno));
    return(-1);
  }
  return(length);
}

int i2c_read_byte(int handle, unsigned char* val)
{
  if (read(handle, val, 1) != 1)
  {
    fprintf(stderr, "i2c_read_byte error: %s\n", strerror(errno));
    return(-1);
  }
  return(1);
}

int i2c_close(int handle)
{
  if ((close(handle)) != 0)
  {
    fprintf(stderr, "i2c_close error: %s\n", strerror(errno));
    return(-1);
  }
  return(0);
}

int i2c_write_read(int handle,
                   unsigned char addr_w, unsigned char *buf_w, unsigned int len_w,
                   unsigned char addr_r, unsigned char *buf_r, unsigned int len_r)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[2];
	
	msgs[0].addr=addr_w;
	msgs[0].len=len_w;
	msgs[0].flags=0;
	msgs[0].buf=buf_w;
	
	msgs[1].addr=addr_r;
	msgs[1].len=len_r;
	msgs[1].flags=1;
	msgs[1].buf=buf_r;
	
	msgset.nmsgs=2;
	msgset.msgs=msgs;
	
	if (ioctl(handle,I2C_RDWR,(unsigned long)&msgset)<0)
  {
		fprintf(stderr, "i2c_write_read error: %s\n",strerror(errno));
    return -1;
  } 
  return(len_r);
}

int i2c_write_ignore_nack(int handle,
                          unsigned char addr_w, unsigned char* buf, unsigned int length)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[1];
	
	msgs[0].addr=addr_w;
	msgs[0].len=length;
	msgs[0].flags=0 | I2C_M_IGNORE_NAK;
	msgs[0].buf=buf;
	
	msgset.nmsgs=1;
	msgset.msgs=msgs;
	
	if (ioctl(handle,I2C_RDWR,(unsigned long)&msgset)<0)
  {
		fprintf(stderr, "i2c_write_ignore_nack error: %s\n",strerror(errno));
    return -1;
  } 
  return(length);
}

int i2c_read_no_ack(int handle, 
                    unsigned char addr_r, unsigned char* buf, unsigned int length)
{
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg msgs[1];
	
	msgs[0].addr=addr_r;
	msgs[0].len=length;
	msgs[0].flags=I2C_M_RD | I2C_M_NO_RD_ACK;
	msgs[0].buf=buf;
	
	msgset.nmsgs=1;
	msgset.msgs=msgs;
	
	if (ioctl(handle,I2C_RDWR,(unsigned long)&msgset)<0)
  {
		fprintf(stderr, "i2c_read_no_ack error: %s\n",strerror(errno));
    return -1;
  } 
  return(length);
}

int delay_ms(unsigned int msec)
{
  int ret;
  struct timespec a;
  if (msec>999)
  {
    fprintf(stderr, "delay_ms error: delay value needs to be less than 999\n");
    msec=999;
  }
  a.tv_nsec=((long)(msec))*1E6d;
  a.tv_sec=0;
  if ((ret = nanosleep(&a, NULL)) != 0)
  {
    fprintf(stderr, "delay_ms error: %s\n", strerror(errno));
  }
  return(0);
}
