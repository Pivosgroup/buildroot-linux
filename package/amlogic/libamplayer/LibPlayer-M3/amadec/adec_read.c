#include <stdio.h>
#include <string.h>  // strcmp 
#include <time.h>    // clock
#include <fcntl.h>

#include <sys/mman.h>
#include <audio-dec.h>
#include <adec-pts-mgt.h>




#define ASTREAM_DEV "/dev/uio0"
#define ASTREAM_ADDR "/sys/class/astream/astream-dev/uio0/maps/map0/addr"
#define ASTREAM_SIZE "/sys/class/astream/astream-dev/uio0/maps/map0/size"
#define ASTREAM_OFFSET "/sys/class/astream/astream-dev/uio0/maps/map0/offset"



#define AIU_AIFIFO_CTRL                            0x1580
#define AIU_AIFIFO_STATUS                          0x1581
#define AIU_AIFIFO_GBIT                            0x1582
#define AIU_AIFIFO_CLB                             0x1583
#define AIU_MEM_AIFIFO_START_PTR                   0x1584
#define AIU_MEM_AIFIFO_CURR_PTR                    0x1585
#define AIU_MEM_AIFIFO_END_PTR                     0x1586
#define AIU_MEM_AIFIFO_BYTES_AVAIL                 0x1587
#define AIU_MEM_AIFIFO_CONTROL                     0x1588
#define AIU_MEM_AIFIFO_MAN_WP                      0x1589
#define AIU_MEM_AIFIFO_MAN_RP                      0x158a
#define AIU_MEM_AIFIFO_LEVEL                       0x158b
#define AIU_MEM_AIFIFO_BUF_CNTL                    0x158c
#define AIU_MEM_AIFIFO_BUF_WRAP_COUNT              0x158d
#define AIU_MEM_AIFIFO2_BUF_WRAP_COUNT             0x158e
#define AIU_MEM_AIFIFO_MEM_CTL                     0x158f

volatile unsigned* reg_base = 0;
#define READ_MPEG_REG(reg) reg_base[reg-AIU_AIFIFO_CTRL]
#define WRITE_MPEG_REG(reg, val) reg_base[reg-AIU_AIFIFO_CTRL]=val
#define AIFIFO_READY  (((READ_MPEG_REG(AIU_MEM_AIFIFO_CONTROL)&(1<<9))))
#define min(x,y) ((x<y)?(x):(y))

static unsigned long  get_num_infile(char *file)
{
	int fd;
	char buf[24]="";
	unsigned long num=0;
	if ((fd = open(file, O_RDONLY)) < 0) {
		adec_print("iunable to open file\n");
		return 0;
	}
	read(fd, buf, sizeof(buf));
	num = strtoul(buf, NULL, 0);
	close(fd);
	return num;
}

int uio_init(int fd){
//	int fd = -1; 
	int pagesize = getpagesize();
	int phys_start;
	int phys_size;
	int phys_offset;
	volatile unsigned memmap;	


	fd = open(ASTREAM_DEV, O_RDWR);
	if(fd < 0){
		adec_print("error open UIO 0\n");
		return -1;
	}
	phys_start = get_num_infile(ASTREAM_ADDR);
	phys_size = get_num_infile(ASTREAM_SIZE);
	phys_offset = get_num_infile(ASTREAM_OFFSET);

	adec_print("add=%08x, size=%08x, offset=%08x\n", phys_start, phys_size, phys_offset);

	phys_size = (phys_size + pagesize -1) & (~ (pagesize-1));
	memmap = mmap(NULL, phys_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0* pagesize);
	
	adec_print("memmap = %x , pagesize = %x\n", memmap,pagesize);
	if(memmap == MAP_FAILED){
		adec_print("map /dev/uio0 failed\n");
		return -1;
	}	 
	 
	reg_base = memmap + phys_offset;
	return 0;
}


static inline void waiting_bits(int bits)
{
	int bytes;
	bytes=READ_MPEG_REG(AIU_MEM_AIFIFO_BYTES_AVAIL);
	while(bytes*8<bits){
		usleep(1000);
		bytes=READ_MPEG_REG(AIU_MEM_AIFIFO_BYTES_AVAIL);
	}	
}

extern int exit_decode_thread;

#define EXTRA_DATA_SIZE 128
int read_buffer(unsigned char *buffer,int size)
{
	int bytes;
	int len;
	unsigned char *p=buffer; 
	int tmp;
	int space;
	int i;
	int wait_times=0,fifo_ready_wait = 0;
	
	int iii;

	iii = READ_MPEG_REG(AIU_MEM_AIFIFO_LEVEL)-EXTRA_DATA_SIZE;
//	adec_print("read_buffer start iii = %d!!\n", iii);

	static int cc = 0;
	while( size >=  iii){
		cc++ ;
		usleep(1000);
		iii = READ_MPEG_REG(AIU_MEM_AIFIFO_LEVEL)-EXTRA_DATA_SIZE;
		if (cc%2000 == 0)
			adec_print("read_buffer start in while iii = %d!!\n", iii);
		if(exit_decode_thread)
			goto out;
	}
	
//	adec_print("read_buffer start while iii= %d!!\n", iii);
	for(len=0;len<size;)
	{	
			space=(size-len);
			bytes=READ_MPEG_REG(AIU_MEM_AIFIFO_BYTES_AVAIL);
			//adec_print("read_buffer start AIU_MEM_AIFIFO_BYTES_AVAIL bytes= %d!!\n", bytes);
			wait_times=0;
			while(bytes==0)
			{
				waiting_bits((space>128)?128*8:(space*8));	/*wait 32 bytes,if the space is less than 32 bytes,wait the space bits*/
				bytes=READ_MPEG_REG(AIU_MEM_AIFIFO_BYTES_AVAIL);
				
				adec_print("read_buffer while AIU_MEM_AIFIFO_BYTES_AVAIL = %d!!\n", bytes);
				wait_times++;
				if(wait_times>10) {					
					adec_print("goto out!!\n");
					goto out;
				}
			}
			bytes=min(space,bytes);
			
			//adec_print("read_buffer while bytes = %d!!\n", bytes);
			for(i=0;i<bytes;i++)
			{
				while(!AIFIFO_READY){
					fifo_ready_wait++;
					usleep(1000);
					if(fifo_ready_wait > 100){
						adec_print("FATAL err,AIFIFO is not ready,check!!\n");
						return 0;
					}	
				}
				WRITE_MPEG_REG(AIU_AIFIFO_GBIT,8);
				tmp=READ_MPEG_REG(AIU_AIFIFO_GBIT);
				//adec_print("read_buffer while tmp = %d!!\n", tmp);
				
				*p++=tmp&0xff;
				fifo_ready_wait = 0;

			}
			len+=bytes;
	}
out:
	//stream_in_offset+=len;
	return len;
}                                                 
                  
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
