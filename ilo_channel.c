/*
 * ilo_channel.c
 *
 * This program is not intended to be supported, or disseminated outside hp.
 *
 * Shamelessly modeled on lx-hprsm:hpqci/smiftest/p.c
 *
 * (C) Copyright 2006 Hewlett-Packard Development Company, L.P.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <poll.h>

#define PACKED __attribute__((packed))
#pragma pack(1)

/* command packet header */
typedef struct {
	unsigned short length, seq, command, reserved PACKED;
} smif_pkthdr;

typedef struct {
   unsigned int errcode                PACKED;
   unsigned short status_word            PACKED;
   unsigned short max_users              PACKED;
   unsigned short firmware_version       PACKED;
   unsigned int firmware_date          PACKED;
   unsigned int post_errcode           PACKED;
   unsigned int datetime               PACKED;
   unsigned int hw_revision            PACKED;
   char board_serial_number[20];
   unsigned short cable_status           PACKED;
   unsigned int latest_event_id        PACKED;
   unsigned int cfg_writecount         PACKED;
   unsigned int post_errcode_mask      PACKED;
#define MOUSE_INTF_ERROR	(1<<12)
#define NIC_ERROR		(1<<11)
#define NVRAM_RW_ERROR		(1<<8)
#define NVRAM_INTF_ERROR	(1<<7)
#define KB_INTF_ERROR		(1<<5)
#define UART_ERROR		(1<<4)
#define MEMTEST_ERROR		(1<<1)
#define BUSMASTER_IO_ERROR	(1<<0)
   unsigned char   class;
   unsigned char   subclass;
   unsigned char   reserved[30];
}pkt_8002;

/* fill out a packet header */
#define SMIF_FEED(cmd) \
	(spkt->seq=0xFEED, spkt->command=cmd, spkt->reserved=0, spkt->length=8)

#define SMIF_FACE(cmd) \
	(spkt->seq=0xFACE, spkt->command=cmd, spkt->reserved=0, spkt->length=8)

/* opaque data structure, used to open channel */
typedef struct {
	unsigned char _rsvd[128];
} cpqci_key;

#pragma pack()

// A couple of routines to acquire uS and convert to mS
//
unsigned long get_current_us()
{
	struct timeval tv;
	long current_us;

	gettimeofday(&tv, NULL);
	current_us = (unsigned long)((tv.tv_sec * 1000000) + tv.tv_usec);

	return current_us;
}

unsigned int us_to_ms(unsigned long us)
{
	return((unsigned int)(us/1000));
}

/*
 *   Routine to send and receive a smif packets
 */
int simple_pkt(void *cpqh, int cmd, int fd)
{
	int 		err, len;
	int 		wr_len, rd_len;
	unsigned long	timeout;
	unsigned char	buf[4096];
	unsigned char	rbuf[4096];
	smif_pkthdr    *spkt = (smif_pkthdr *)buf;
	smif_pkthdr    *rpkt = (smif_pkthdr *)rbuf;
	pkt_8002       *pkt;
	struct pollfd  *fds;
	int		fdcount = 1;
	int		rv;
	const int	poll_timeout = 5000;	// 5 second timeout
	unsigned long 	start_time;
	unsigned long	stop_time;
	unsigned int	elapsed_poll_time;

	// Init buffers
	//
	memset(buf, 0, sizeof(buf));
	memset(rbuf, 0, sizeof(rbuf));

	// Polled read initialization
	//
	fds = (struct pollfd *)calloc(fdcount, sizeof(*fds));
	fds[0].fd = fd;
	fds[0].events |= POLLIN;

	//==========================================================
	// write
	//==========================================================
	//
	len = SMIF_FEED(cmd);
	printf("\nWrite packet seq:0x%04X len:%d followed by a polled read\n",
		spkt->seq, len);

	wr_len = write(fd, buf, len);
	if (wr_len != len) {
		printf("simple_pkt failed (%d %d) on send command %d\n",
			wr_len, len, errno);
		goto out_err;
	}


	//==========================================================
	// Setup for polled read
	//==========================================================
	//
	start_time = get_current_us();
	rv = poll(fds, fdcount, poll_timeout);
	stop_time = get_current_us();
	elapsed_poll_time = us_to_ms(stop_time - start_time);

	if(rv == -1)
		printf("Polled Read FAILED: Return Value = -1.\n");
	else if(!(fds[0].revents & POLLIN))
		printf("Did not get expected POLLIN event.\n");
	else {
		printf("Perform the read...\n");
		len = sizeof(rbuf);

		start_time = get_current_us();
		rd_len = read(fd, rbuf, len);
		stop_time = get_current_us();

		if (rd_len <= 0) {
			printf("simple_pkt failed (%d %d) on recv command %d\n",
				rd_len, len, errno);
		}
		else {
			printf("Successful read of packet seq:0x%04X len:%d\n",
				rpkt->seq, rd_len);

			// Dump a couple of the fields that were read back.
			//
			pkt = (pkt_8002 *)(rbuf + 8);
			printf("pkt version: 0x%x serial: %s\n",
			       pkt->firmware_version, pkt->board_serial_number);

			printf("Elapsed poll time: %d mS\n", elapsed_poll_time);
			printf("Elapsed read time: %d mS\n",
				us_to_ms(stop_time - start_time));
		}
	}


	//==========================================================
	// Setup for interrupt write and read
	//==========================================================
	//
	//CpqCiSend(cpqh, buf, len, &err, NULL);

	len = SMIF_FACE(cmd);
	printf("\nWrite packet seq:0x%04X len:%d\n", spkt->seq, len);

	wr_len = write(fd, buf, len);
	if (wr_len != len) {
		printf("simple_pkt failed (%d %d) on send command %d\n",
			wr_len, len, errno);
		goto out_err;
	}
	printf("Successful write.\n");

	//timeout = 5000;
	//len = CpqCiRecv(cpqh, rbuf, sizeof(rbuf), &err, &timeout);
	len = sizeof(rbuf);

	start_time = get_current_us();
	rd_len = read(fd, rbuf, len);
	stop_time = get_current_us();

	if (rd_len <= 0) {
		printf("simple_pkt failed (%d %d) on recv command %d\n",
			rd_len, len, errno);
		goto out_err;
	}
	printf("Successful read of packet seq:0x%04X len:%d\n",
		((smif_pkthdr *)rbuf)->seq, rd_len);

	/*
	 * you could hexdump the rbuf, or write fancy parsing routines
	 * to look at the rbuf as a smif_pkthdr *, parse the command,
	 * and dump the fields (start at rbuf+8) according to the
	 * smif specification.
	 */
	pkt = (pkt_8002 *)(rbuf + 8);

	printf("pkt version: 0x%x serial: %s\n",
	       pkt->firmware_version, pkt->board_serial_number);

	printf("Elapsed read time: %d mS\n\n", us_to_ms(stop_time - start_time));
out_err:
	return err;
}

int main(int argc, char *argv[])
{
	int 		fd, err, iter;
	void   	       *handle;
	cpqci_key	key;

	/* open a channel */
//	err = CpqCiCreate(&key, &handle, 3, 4096, 3, 4096, 0, NULL);
	fd = open("/dev/hpilo/d0ccb0", O_RDWR);
	if (fd == -1) {
		printf("failed to create a channel %d\n", errno);
		exit(errno);
	}

	/* process command(s) */
	iter = 1;
	do {
		iter--;
		err = simple_pkt(handle, 0x0002, fd);
	} while (iter && !err);

	/* if (err) { goto out; } err = simple_pkt(handle, ...); */
out:
	/* close channel */
	//CpqCiClose(handle);
	close(fd);
	exit(err);
}
