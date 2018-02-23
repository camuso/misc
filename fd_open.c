/* 
 * fd_open.c 
 *
 * This program is not intended to be supported, or disseminated outside hp.
 * 
 * Shamelessly modeled on ABS:hp-snmp-agents:hpqci/smiftest/p.c
 *
 * (C) Copyright 2008 Hewlett-Packard Development Company, L.P.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/errno.h>

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
#define SIMPLE_SMIF(cmd) \
	(spkt->seq=0xDEAD, spkt->command=cmd, spkt->reserved=0, spkt->length=8)

#pragma pack()

/* sends and receives a smif packet */ 
int simple_pkt(int cmd, int fd)
{
	int 		err = 0, len;
	int 		wr_len, rd_len;
	unsigned long	timeout;
	unsigned char	buf[4096];
	unsigned char	rbuf[4096];
	smif_pkthdr    *spkt = (smif_pkthdr *)buf;
	pkt_8002       *pkt;

	memset(buf, 0, sizeof(buf));
	memset(rbuf, 0, sizeof(rbuf));

	len = SIMPLE_SMIF(cmd);
	wr_len = write(fd, buf, len); 
	if (wr_len < 0) {
		err = errno;
		printf("send failed tried %d returned %d errno %d\n", 
		       len, wr_len, errno);
		goto out_err;
	}

	len = sizeof(rbuf);
	rd_len = read(fd, rbuf, len);
	if (rd_len < 0) {
		err = errno;
		printf("recv failed tried %d returned %d errno %d\n", 
		       len, rd_len, errno);
	}

	/* 
	 * you could hexdump the rbuf, or write fancy parsing routines
	 * to look at the rbuf as a smif_pkthdr *, parse the command,
	 * and dump the fields (start at rbuf+8) according to the 
	 * smif specification.
	 */
	pkt = (pkt_8002 *)(rbuf + 8);

	printf("pkt version 0x%x serial %s\n", 
	       pkt->firmware_version, pkt->board_serial_number);
out_err:
	return err;
}
static char *cpqci_file[] = {
	"/dev/ilo_ccb0",
	"/dev/ilo_ccb1",
	"/dev/ilo_ccb2",
	"/dev/ilo_ccb3",
	"/dev/ilo_ccb4",
	"/dev/ilo_ccb5",
	"/dev/ilo_ccb6",
	"/dev/ilo_ccb7"
};

int main(int argc, char *argv[])
{
	int 		i, fd, err, iter;

	/* open a channel */
	for (i=0; i < 8; i++) {
		fd = open(cpqci_file[i], O_RDWR);
		if (fd == -1) {
			printf("%d failed to create a channel %d\n", i, errno);
			exit(errno);
		}
		printf("opened file %s\n", cpqci_file[i]);
		/* process command(s) */
		iter = 1;
		do {
			iter--;
			err = simple_pkt(0x0002, fd);
		} while (iter && !err);

		/* close channel */
		close(fd);
		if (err)
			break;
	}
out:
	exit(err);
}
