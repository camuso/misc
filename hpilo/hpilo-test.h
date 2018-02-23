// hpilo-test.h
// 
// Header file for hpilo-test.c
// Must be precompiled before distribution.
//

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

