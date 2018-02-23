#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/sg.h> 
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef ASC_ASCQ
#include "asc-num.h"
#endif

/* 
 * HISTORY:
 * 09.09.11 Added SG_IO call for REPORT LUNS along with get sg version number to validate SG_IO is supported,
 *          existing calls use deprecated ioctl interface.  It still works, but is deprecated, so starting
 *          to convert calls to SG_IO with REPORT LUNS most likely to benefit from much larger data xfer size
 *          hopefully.
 * 09.09.11 Increased buffer passed in to x7FF for RTPGs and REPORT LUNS -- limit is 1 memory page and xFF
 *          seemed to be limiting data that could be returned.  Am getting -1 status back on REPORT LUNS
 *          with IBM storage at customer site... not sure what this is due to but expect list limit exceeded.
 * 06.20.11 Add designator_type table from T10 SCSI Primary Commands-4 for pg83 decode
 * 06.20.11 Add EUI-64 format decode if ident_type=2h (EUI-64)
 * 06.20.11 Add NAA    format decode if ident_type=3h (NAA)
 * 06.20.11 Fixed computation error of read capacity MiB output
 *
 */
/*
 * TODO: change to SG_IO so we don't call scsi_cmd_ioctl which is deprecated.
 * {ed:  current interface can only handle 1 page of data max.}
 */

/* 
 * MAINTAINER: bubrown@redhat.com
 */
#define VERSION "01.00-044-13092011"		/* Tue Sep 13 10:34:49 2011 bubrown*/


typedef struct scsi_inqdata_t {
        unsigned        device_type     :5;     /* Peripheral Device Type */
        unsigned        qualifier       :3;     /* Peripheral Qualifier  */

        unsigned        device_type_mod :7;	/* Device-type modifier */
        unsigned        rmb             :1;     /* Removable Medium Bit */

        unsigned        ansi_version    :3;     /* ANSI Version */
        unsigned        ecma_version    :3;     /* ECMA Version */
        unsigned        iso_version     :2;     /* ISO Version */

        unsigned        response_format :4;     /* Response Data Format */
        unsigned        reserved1       :2;     /* Reserved */
        unsigned        TrmIOP          :1;     /* TrmIOP */
        unsigned        AENC            :1;     /* AENC */
	
        unsigned char   additional_length;	/* Additional Length (total_length-4) */
						/* Byte5 - Reserved for version < certain level */
	unsigned        protection      :1;	/* Protection */
	unsigned        _resv5_1        :2;	/* Reserved<1:2> */
	unsigned	third_party     :1;	
	unsigned        tpgs            :2;	/* Target Port Groups Supported (ALUA) */
	unsigned        acc             :1;
	unsigned        sccs            :1;
						/* Byte6 */
	unsigned        addr16          :1;
	unsigned        addr32          :1;
	unsigned        ack_req_q       :1;
	unsigned        mchngr          :1;
	unsigned        multip          :1;
	unsigned        vs              :1;
	unsigned        enc_serv        :1;
	unsigned        r               :1;
						/* Byte7 */
	unsigned        sft_re_vs       :1;
	unsigned        cmd_que         :1;
	unsigned        trans_dis       :1;
	unsigned        linked          :1;
        unsigned        sync            :1;
        unsigned        wbus16          :1;
        unsigned        wbus32          :1;
        unsigned        reladr          :1;

 	         char   vendor_id[8];		/* Vendor Identification (ASCII)      */
	         char   product_id[16];         /* Product Identification (ASCII)     */
	unsigned char   revision_level[4];	/* Revision Level */
	unsigned char   vendor_specific[20];    /* Vendor Specific - Optional */
        unsigned char   reserved2[40];          /* Reserved - Optional */
                                                /* Additional vendor-specific information may be returned */
} scsi_inqdata_t;

typedef struct scsi_inqdata_pg00_t {
        unsigned        device_type     :5;     /* Peripheral Device Type */
        unsigned        qualifier       :3;     /* Peripheral Qualifier   */

	unsigned char   page_code;		/* Must! be 0x00          */
	unsigned char   _resv1;                 /* Reserved               */
        unsigned char   page_len;               /* Page length (n-3) bytes -- 3 bytes for above */ 

        unsigned char   supported_pages[255-6]; /* Maximum serial number array size, len=page_len-1 */
} scsi_inqdata_pg00_t;

typedef struct scsi_inqdata_pgxx_t {
        unsigned        device_type     :5;     /* Peripheral Device Type */
        unsigned        qualifier       :3;     /* Peripheral Qualifier   */

	unsigned char   page_code;		/* Must! be 0xXX          */
	unsigned char   _resv1;                 /* Reserved               */
        unsigned char   page_len;               /* Page length (n-3) bytes -- 3 bytes for above */ 

        unsigned char   data[255-6]; 
} scsi_inqdata_pgxx_t;

typedef struct vpd_t
{
    int code_min;
    int code_max;
    char desc[256];
} vpd_t;
/* Draft: T10-spc4r16-11.08-SCSI_Primary_Commands-SPC-4.pdf */
vpd_t pg00_vpd[] =
{
  { 0x00, 0x00, "00h Supported VPD Pages               7.7.11 Mandatory" },
  { 0x01, 0x7F, "01h to 7Fh ASCII Information          7.7.2  Optional"  },
  { 0x80, 0x80, "80h Unit Serial Number                7.7.12 Optional"  },
  { 0x81, 0x81, "81h Obsolete 3.3.7 (was:implemented operating definitions page)" },
  { 0x82, 0x82, "82h Obsolete 3.3.7 (was:ASCII implemented operating definitions page)" },
  { 0x83, 0x83, "83h Device Identification             7.7.3  Mandatory" },
  { 0x84, 0x84, "84h Software Interface Identification 7.7.10 Optional"  },
  { 0x85, 0x85, "85h Management Network Addresses      7.7.5  Optional"  },
  { 0x86, 0x86, "86h Extended INQUIRY Data             7.7.4  Optional"  },
  { 0x87, 0x87, "87h Mode Page Policy                  7.7.6  Optional"  },
  { 0x88, 0x88, "88h SCSI Ports                        7.7.7  Optional"  },
  { 0x89, 0x89, "89h ATA Information                   SAT-2  Optional"  },
  { 0x8A, 0x8F, "8Ah to 8Fh Reserved"                                    },
  { 0x90, 0x90, "90h Protocol Specific Logical Unit Information 7.7.8 Protocol specific" },
  { 0x91, 0x91, "91h Protocol Specific Port Information         7.7.9 Protocol specific" },
  { 0x92, 0xAF, "92h to AFh Reserved"                                    },
  { 0xB0, 0xBF, "B0h to BFh (See specific device type)"                  },
  { 0xC0, 0xFF, "C0h to FFh Vendor specific"                             },
  {   -1,   -1, ""}
};


typedef struct scsi_inqdata_pg80_t {
        unsigned        device_type     :5;     /* Peripheral Device Type */
        unsigned        qualifier       :3;     /* Peripheral Qualifier   */

	unsigned char   page_code;		/* Must! be 0x80          */
	unsigned char   _resv1;                 /* Reserved               */
        unsigned char   page_len;               /* Page length (n-3) bytes -- 3 bytes for above */ 

        unsigned char   serial_number[255-6];   /* Maximum serial number array size, len=page_len-1 */
} scsi_inqdata_pg80_t;

/* Page 83h has a header followed by a series of packed ident records... */
typedef struct scsi_inqdata_pg83_t
{
        unsigned        device_type     :5;     /* Peripheral Device Type */
        unsigned        qualifier       :3;     /* Peripheral Qualifier   */

	unsigned char   page_code;		/* Must! be 0x80          */
	unsigned char   _resv1;                 /* Reserved               */
        unsigned char   page_len;               /* Page length (n-3) bytes -- 3 bytes for above */ 

	unsigned char   ident_records[255-6];
} scsi_inqdata_pg83_t;

typedef struct pg83_ident_t
{
        unsigned        code_set        :4;     /* Code Set               */
        unsigned        protocol_ident  :4;     /* Protocol identifier    */

	unsigned        ident_type      :4;	/* Identifier             */
        unsigned        assoc           :2;     /* Association            */
        unsigned        _resv1          :1;
        unsigned        piv             :1;

	unsigned char   _resv2;
	unsigned char   ident_len;
        unsigned char   ident[256];
} pg83_ident_t;


/* T10/1731-D Revision 16 29 July 2008 */
/* 4.4.3 Variable type data field requirements */
char pg83_codeset[16][256] =
{
 { /* 0h */ "Reserved" },
 { /* 1h */ "Binary"   }, 
 { /* 2h */ "ASCII"    },
 { /* 3h */ "UTF-8 (see 3.1.180)" },
 { /* 4h */ "Reserved" },
 { /* 5h */ "Reserved" },
 { /* 6h */ "Reserved" },
 { /* 7h */ "Reserved" },
 { /* 8h */ "Reserved" },
 { /* 9h */ "Reserved" },
 { /* Ah */ "Reserved" },
 { /* Bh */ "Reserved" },
 { /* Ch */ "Reserved" },
 { /* Dh */ "Reserved" },
 { /* Eh */ "Reserved" },
 { /* Fh */ "Reserved" }
};


/* T10/1731-D Revision 16 29 July 2008 */
char pg83_designator_type[16][256] =
{
 { /* 0h */ "Vendor specific                 (7.7.3.3)" },
 { /* 1h */ "T10 vendor ID based             (7.7.3.4)" },
 { /* 2h */ "EUI-64 based                    (7.7.3.5)" },
 { /* 3h */ "NAA                             (7.7.3.6)" },
 { /* 4h */ "Relative target port identifier (7.7.3.7) {aka Port # ;1=A, 2=B}" },
 { /* 5h */ "Target port group               (7.7.3.8)" },
 { /* 6h */ "Logical unit group              (7.7.3.9)" },
 { /* 7h */ "MD5 logical unit identifier     (7.7.3.10)" },
 { /* 8h */ "SCSI name string                (7.7.3.11)" },
 { /* 9h */ "Reserved"},
 { /* Ah */ "Reserved"},
 { /* Bh */ "Reserved"},
 { /* Ch */ "Reserved"},
 { /* Dh */ "Reserved"},
 { /* Eh */ "Reserved"},
 { /* Fh */ "Reserved"}
};

/* T10/1731-D Revision 16 29 July 2008 */
/* ident_len dictates format if/when ident_type=2 */
typedef struct code_t
{
    int code;
    char desc[256];
} code_t;



code_t pg83_eui64[] =
{
   { 0x08, "EUI-64 identifier               (7.7.3.5.2)" },
   { 0x0C, "EUI-64 based 12-byte identifier (7.7.3.5.3)" },
   { 0x10, "EUI-64 based 16-byte identifier (7.7.3.5.4)" },
   {   -1, "Reserved" }
};

code_t pg83_naa[] =
{
   { 0x2, "2h IEEE Extended         (7.7.3.6.2)" },
   { 0x3, "Locally Assigned         (7.7.3.6.3)" },
   { 0x5, "IEEE Registered          (7.7.3.6.4)" },
   { 0x6, "IEEE Registered Extended (7.7.3.6.5)" },
   {   -1, "Reserved" }
};

typedef struct scsi_rtpg_descriptor_t {
        unsigned        alua_state      :3;     /* Asymmetric Access State     */
	unsigned        _reserved_1     :4;
        unsigned        pref            :1;	/* preferred target port group */

	unsigned        ao_sup          :1;
	unsigned        an_sup          :1;
	unsigned        s_sup           :1;
	unsigned        u_sup           :1;
        unsigned        _reserved_2     :3;
        unsigned        t_sup           :1;	
	
	unsigned short   tpg_id;		/* target port group (id) */
	unsigned char   _reserved_3;
	unsigned char   status;
	unsigned char   vendor_specific;
	unsigned char   port_count;
	
	unsigned char   port_id[1][4];		/* list of target ports */
} scsi_rtpg_descriptor_t; 

char rtpg_pref_tbl[2][128] =
{
   { "Non-Preferred Port" },
   { "Preferred Port" } 
};

char rtpg_access_state_tbl[16][256] =
{
    { /* 0h */ "Active/optimized    " },
    { /* 1h */ "Active/non-optimized" },
    { /* 2h */ "Standby             " },
    { /* 3h */ "Unavailable         " },
    { /* 4h */ "<Reserved>          " },
    { /* 5h */ "<Reserved>          " },
    { /* 6h */ "<Reserved>          " },
    { /* 7h */ "<Reserved>          " },
    { /* 8h */ "<Reserved>          " },
    { /* 9h */ "<Reserved>          " },
    { /* Ah */ "<Reserved>          " },
    { /* Bh */ "<Reserved>          " },
    { /* Ch */ "<Reserved>          " },
    { /* Dh */ "<Reserved>          " },
    { /* Eh */ "<Reserved>          " },
    { /* Fh */ "Transitioning...    " }
};



code_t rtpg_status_tbl[] =
{
  { 0x00, "No status available."},
  { 0x01, "Target port group asymmetric access state altered by SET TARGET PORT GROUPS command."},
  { 0x02, "Target port group asymmetric access state altered by implicit asymmetrical logical unit access behavior."},
  { -1,   "<Reserved >" }
};


typedef struct sense_buffer_t
{
	unsigned char	error_code;
	unsigned char	segment_number;
	unsigned char	sense_key;
	unsigned char   information[4];
	unsigned char   addl_sense_length;
	unsigned char	command_specific[4];
	unsigned char 	asc;
	unsigned char	ascq;
	unsigned char	fru;
	unsigned char	sense_key_specific[3];
} sense_buffer_t;

typedef struct scsi_readcap_t
{
	unsigned int    max_logblock_address;
	unsigned int    block_length;
} scsi_readcap_t;


char scsi_dev_type[32][256] = 
{
 { "00h - direct-access device (e.g., magnetic disk)"},
 { "01h - sequential-access device (e.g., magnetic tape)"},
 { "02h - printer device"},
 { "03h - processor device"},
 { "04h - write-once device"},
 { "05h - CDROM device"},
 { "06h - scanner device"},
 { "07h - optical memory device (e.g., some optical disks)"},
 { "08h - medium Changer (e.g. jukeboxes)"},
 { "09h - communications device"},
 { "0Ah - defined by ASC IT8 (Graphic arts pre-press devices)"},
 { "0Bh - defined by ASC IT8 (Graphic arts pre-press devices)"},
 { "0Ch - Storage array controller device (e.g., RAID)"},
 { "0Dh - Enclosure services device"},
 { "0Eh - Simplified direct-access device (e.g., magnetic disk)"},
 { "0Fh - Optical card reader/writer device"},
 { "10h - Reserved for bridging expanders"},
 { "11h - Object-based Storage Device"},
 { "12h - Automation/Drive Interface"},
 { "13h - reserved"},
 { "14h - reserved"},
 { "15h - reserved"},
 { "16h - reserved"},
 { "17h - reserved"},
 { "18h - reserved"},
 { "19h - reserved"},
 { "1Ah - reserved"},
 { "1Bh - reserved"},
 { "1Ch - reserved"},
 { "1Dh - reserved"},
 { "1Eh - Well known logical unit"},
 { "1Fh - unknown or no device type"}
};
char scsi_per_qual[8][256] = 
{
 { "0h  - Connected {active path}" },
 { "1h  - Not Connected {standby path}" },
 { "2h  - Reserved by standard" },
 { "3h  - Not Capable {see standard}" },
 { "4h  - Vendor Specific" },
 { "5h  - Vendor Specific" },
 { "6h  - Vendor Specific" },
 { "7h  - Vendor Specific" }
};

char *response_byte5(scsi_inqdata_t *response);
char *pg83_eui64fmt (pg83_ident_t *ident );
char *pg83_naafmt (pg83_ident_t *ident );
char *rtpg_supported_states( scsi_rtpg_descriptor_t *rtpg );
char *rtpg_status(unsigned char status );
#ifdef ASC_ASCQ 
char *decode_asc_ascq(int asc, int ascq);
#endif

char *scsi_status(int status);
char *scsi_sense_key( unsigned char key );

char device_path[256];
int  verbose = 0;
int  sleeping= 0;
int  looping = 0;

int main(int argc, char *argv[])
{
    char *path;
    int fd;
    int rc;
    typedef struct sdata_t {
        u_int32_t inlen;
        u_int32_t outlen;
        unsigned char cmd[4096];
    } sdata;
    sdata inq;
    scsi_inqdata_t *response;
    scsi_inqdata_pgxx_t *pgxx;
    scsi_inqdata_pg00_t *pg00;
    scsi_inqdata_pg80_t *pg80;
    scsi_inqdata_pg83_t *pg83;
    pg83_ident_t        *pg83_ident;
    scsi_readcap_t *readcap;
    scsi_rtpg_descriptor_t *rtpg_desc;
    char  reason[256], sav, *buf;
    int   status, ndx, n;
    int   buflen;
    int   page_list[256];
    int   page_cnt=0;
    int   pagno;
    char  ascii_buffer[256], *ascii;
    int   sg_verno;

    parse_args(argc, argv);

again:
    fd = open (device_path, O_RDONLY);
    if (fd == -1) {
        sprintf(reason, "%%F-open of %s failed", device_path );
	perror (device_path);
	exit(1);
    }
   
    sg_verno = 0; 
    if ((status=(ioctl(fd, SG_GET_VERSION_NUM, &sg_verno)) < 0) || (sg_verno < 30000)) { 
	if (status < 0)
	    perror("%%E-SG_GET_VERSION_NUM (errno)");
        printf("%%W-%s does not support SG_IO\n", device_path );
	sg_verno = 0;
    } else if (verbose >= 1)
	printf("%%I-%s is sg device, SG verno=%d\n", device_path, sg_verno );



    /* INQUIRY (standard) --------------------------------------------------*/ 
    memset (&inq     , 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0xFF;
   
    inq.cmd[0] = 0x12;          /* INQUIRY */
    inq.cmd[1] = 0x00;          /* lun=0, evpd=0 */
    inq.cmd[2] = 0x00;          /* page code = 0 */
    inq.cmd[3] = 0x00;          /* (reserved) */
    inq.cmd[4] = 0xFF;          /* allocation length */
    inq.cmd[5] = 0x00;          /* control */

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl INQUIRY status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl INQUIRY status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-INQUIRY (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
	    if ((n % 16) == 0) printf("\n[%04d]  ", n );
	    printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }
  
    if ((verbose >= 2) && (status == 0)) 
    { 
    response = (scsi_inqdata_t *)&inq.cmd[0];
    printf( "Peripheral Device Type: %s\n", scsi_dev_type[ response->device_type ] );
    printf( "  Peripheral Qualifier: %s\n", scsi_per_qual[ response->qualifier   ] );
    printf( "  Device-type Modifier: %d\n", response->device_type_mod );
    printf( "  Removable Medium Bit: %d\n", response->rmb );
    printf( "          ANSI Version: %d\n", response->ansi_version );
    printf( "          ECMA Version: %d\n", response->ecma_version );
    printf( "           ISO Version: %d\n", response->iso_version );
    printf( "  Response Data Format: %d\n", response->response_format );
    printf( "                TrmIOP: %d\n", response->TrmIOP );
    printf( "                  AENC: %d\n", response->AENC );
    printf( "     Additional Length: %d\n", response->additional_length );
    if (inq.cmd[5] == 0)
    printf( "                 Byte5: %02X\n", inq.cmd[5] );
    else
    printf( "                 Byte5: %s\n", response_byte5(response) );
    printf( "                 Byte6: %02X\n", inq.cmd[6] );
    printf( "                 Byte7: %02X\n", inq.cmd[7] );
    sav = response->vendor_id[8];
          response->vendor_id[8] = 0;
    printf( " Vendor Identification: '%s'\n", response->vendor_id );
          response->vendor_id[8] = sav;

    sav = response->product_id[16];
          response->product_id[16] = 0;
    printf( "Product Identification: '%s'\n", response->product_id );
          response->product_id[16] = sav;
    printf( "        Revision Level: %02X.%02X.%02X.%02X '",
	  response->revision_level[0],
	  response->revision_level[1],
	  response->revision_level[2],
	  response->revision_level[3] );
    for (n=0; n<4; n++)
    {
	if ((response->revision_level[n] >= ' ') && (response->revision_level[n] <= '~'))
	    printf( "%c", response->revision_level[n] );
	else
	    printf( "%c", "." );
    }
    printf( "'\n");
    if (inq.cmd[5] != 0)
	tgps_byte5(response);

    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );
    /* ---------------------------------------------------------------------*/ 

    /* INQUIRY (pg00, vpd list) --------------------------------------------*/ 
    memset (&inq     , 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0xFF;
    
    inq.cmd[0] = 0x12;          /* INQUIRY */
    inq.cmd[1] = 0x01;          /* lun=0, evpd=0 */
    inq.cmd[2] = 0x00;          /* page code = 0 */
    inq.cmd[3] = 0x00;          /* (reserved) */
    inq.cmd[4] = 0xFF;          /* allocation length */
    inq.cmd[5] = 0x00;          /* control */

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl INQUIRY (PG00) status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl INQUIRY (PG00) status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-INQUIRY (PG00) (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
	    if ((n % 16) == 0) printf("\n[%04d]  ", n );
	    printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }
    if ((verbose >= 2) && (status == 0))
    { int x;
    pg00 = (scsi_inqdata_pg00_t *)&inq.cmd[0];
    printf( "Peripheral Device Type: %s\n", scsi_dev_type[ pg00->device_type ] );
    printf( "  Peripheral Qualifier: %d\n",    pg00->qualifier );
    printf( "             Page Code: %02Xh\n", pg00->page_code );
    printf( "           Page Length: %d\n",    pg00->page_len );
    n = 0;
    while(n<pg00->page_len)
    {
	for (x=0; x<255; x++)
	{
	    if (pg00_vpd[x].code_min == -1)
	    {
		printf( "supported page %02Xh : ???\n", pg00->supported_pages[n] );
		break;
	    }
	    if ((pg00->supported_pages[n] >= pg00_vpd[x].code_min) &&
	        (pg00->supported_pages[n] <= pg00_vpd[x].code_max))
	    {
		printf( "supported page %02Xh : %s\n", pg00->supported_pages[n], pg00_vpd[x].desc );	
		page_list[ page_cnt++ ] = pg00->supported_pages[n];
		break;
	    }
	}
	n++;
    }
    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );
    /* ---------------------------------------------------------------------*/ 


    /* INQUIRY (pg80, serial number) ---------------------------------------*/ 
    memset (&inq     , 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0xFF;
    
    inq.cmd[0] = 0x12;          /* INQUIRY */
    inq.cmd[1] = 0x01;          /* lun=0, evpd=0 */
    inq.cmd[2] = 0x80;          /* page code = 0 */
    inq.cmd[3] = 0x00;          /* (reserved) */
    inq.cmd[4] = 0xFF;          /* allocation length */
    inq.cmd[5] = 0x00;          /* control */

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl INQUIRY (PG80) status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl INQUIRY (PG80) status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-INQUIRY (PG80) (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
	    if ((n % 16) == 0) printf("\n[%04d]  ", n );
	    printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }
    if ((verbose >= 2) && (status == 0))
    { int x;
    pg80 = (scsi_inqdata_pg80_t *)&inq.cmd[0];
    printf( "Peripheral Device Type: %s\n", scsi_dev_type[ pg80->device_type ] );
    printf( "  Peripheral Qualifier: %d\n", pg80->qualifier );
    printf( "             Page Code: %02Xh\n", pg80->page_code );
    printf( "           Page Length: %d\n", pg80->page_len );
    n = 0;
    while(n<pg80->page_len)
    {
	for (x=0; x<16; x++)
	{
	    if ((n+x) < pg80->page_len)
	        printf( "%02X ", pg80->serial_number[n+x] );
	    else
		printf( "   " );
	}
	printf( "   '" );
	for (x=0; x<16; x++)
	{
	    if ((n+x) < pg80->page_len)
  	    {
		if ((pg80->serial_number[n+x] >= ' ') && (pg80->serial_number[n+x] <= '~'))
			printf( "%c", pg80->serial_number[n+x] );
		else
			printf( "." );
	    }
	}
	n += 16;
	printf( "'\n");
    }
    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );
    /* ---------------------------------------------------------------------*/ 
  
    /* INQUIRY (pg03, vpd list) --------------------------------------------*/ 
    memset (&inq     , 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0xFF;
    
    inq.cmd[0] = 0x12;          /* INQUIRY */
    inq.cmd[1] = 0x01;          /* lun=0, evpd=0 */
    inq.cmd[2] = 0x83;          /* page code = 0 */
    inq.cmd[3] = 0x00;          /* (reserved) */
    inq.cmd[4] = 0xFF;          /* allocation length */
    inq.cmd[5] = 0x00;          /* control */

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl INQUIRY (PG83) status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl INQUIRY (PG83) status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-INQUIRY (PG83) (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
	    if ((n % 16) == 0) printf("\n[%04d]  ", n );
	    printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }
    if ((verbose >= 2) && (status == 0))
    { int x,l,r; unsigned char *ptr;
    pg83 = (scsi_inqdata_pg83_t *)&inq.cmd[0];
    printf( "Peripheral Device Type: %s\n", scsi_dev_type[ pg83->device_type ] );
    printf( "  Peripheral Qualifier: %d\n", pg83->qualifier );
    printf( "             Page Code: %02Xh\n", pg83->page_code );
    printf( "           Page Length: %d\n", pg83->page_len );
    n = 0;
    r = 0;
    ptr = &pg83->ident_records[0];
    while(n<pg83->page_len)
    {
	pg83_ident = (pg83_ident_t *)ptr;
        n += pg83_ident->ident_len+4;
        ptr = &ptr[pg83_ident->ident_len+4];
        printf( "----------Ident Record: %d\n", r );
        printf( "              Code Set: %Xh %s\n",  pg83_ident->code_set, pg83_codeset[pg83_ident->code_set & 0xF] );
        printf( "   Protocol Identifier: %Xh\n", pg83_ident->protocol_ident );
        printf( "            Identifier: %Xh - %s\n", pg83_ident->ident_type, pg83_designator_type[pg83_ident->ident_type & 0xF]);
        printf( "           Association: %d\n", pg83_ident->assoc );
  	printf( "                   PIV: %d\n", pg83_ident->piv   );
	printf( "            (reserved): %d\n", pg83_ident->_resv2 );
	printf( "     Identifier Length: %d %s\n",pg83_ident->ident_len, pg83_eui64fmt(pg83_ident) );
	pg83_naafmt(pg83_ident);
	l = 0;
	while (l<pg83_ident->ident_len)
        {
	    for (x=0; x<16; x++)
	    {
	        if ((l+x) < pg83_ident->ident_len)
	            printf( "%02X ", pg83_ident->ident[l+x] );
	        else
		    printf( "   " );
	    }
	    printf( "   '" );
	    for (x=0; x<16; x++)
	    {
	        if ((l+x) < pg83_ident->ident_len)
  	        {
		    if ((pg83_ident->ident[l+x] >= ' ') && (pg83_ident->ident[l+x] <= '~'))
			printf( "%c", pg83_ident->ident[l+x] );
		    else
			printf( "." );
	        }
	    }
	    l += 16;
	    printf( "'\n");
	}
	r++;
    }
    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );
    /* ---------------------------------------------------------------------*/ 
  
    /* INQUIRY (pgXX)                ---------------------------------------*/ 
    for (pagno = 0; pagno < page_cnt; pagno++)
    { char explain[128];

	if (page_list[pagno] == 0x00) continue;
        if (page_list[pagno] == 0x80) continue;
        if (page_list[pagno] == 0x83) continue;

	/* some other page, try getting its data and dumping raw. */
        memset (&inq     , 0, sizeof (sdata));

        inq.inlen = 0;
        inq.outlen = 0xFF;
    
        inq.cmd[0] = 0x12;          /* INQUIRY */
        inq.cmd[1] = 0x01;          /* lun=0, evpd=1 */
        inq.cmd[2] = page_list[pagno]; /* page code = ? */
        inq.cmd[3] = 0x00;          /* (reserved) */
        inq.cmd[4] = 0xFF;          /* allocation length */
        inq.cmd[5] = 0x00;          /* control */

        status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
        if (verbose >= 1)
            printf( "\n%%I-ioctl INQUIRY (PG%02X) status=%d %s\n", page_list[pagno], status, scsi_status(status) );
        else if (status != 0)
        {
            printf( "\n%%E-ioctl INQUIRY (PG%02X) status=%d %s\n", page_list[pagno], status, scsi_status(status) );
	    if (status == -1) 
	    {
	        sprintf( explain, "%%E-INQUIRY (PG%02X) (perrno)", page_list[pagno] );
                perror( explain );
	    }
        }

        if (verbose >= 3)
        {
	    ascii = ascii_buffer;
            for (n=0;n<256;n++)
            {
                if ((n % 16) == 0) { *ascii = 0; printf("%s\n[%04d]   ", ascii_buffer, n ); ascii = ascii_buffer; }
                printf("%02X ", inq.cmd[n] );
                if ((inq.cmd[n] >= ' ') && (inq.cmd[n] <= '~')) *ascii++ = inq.cmd[n];
                else                                            *ascii++ = '.';
            }
            printf( "\n");
        }
    
        if ((verbose >= 2) && (status == 0))
        { int x;
        pgxx = (scsi_inqdata_pgxx_t *)&inq.cmd[0];
        printf( "Peripheral Device Type: %s\n", scsi_dev_type[ pgxx->device_type ] );
        printf( "  Peripheral Qualifier: %d\n",    pgxx->qualifier );
        printf( "             Page Code: %02Xh\n", pgxx->page_code );
        printf( "           Page Length: %d\n",    pgxx->page_len );
        }
        if ((verbose >= 2) && (status != 0)) 
            scsi_sense_buffer( &inq.cmd[0] );
    }
    /* ---------------------------------------------------------------------*/ 

    memset (&inq, 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0xFF;

    inq.cmd[0] = 0x25;          /* READ CAPACITY */
    inq.cmd[1] = 0x00;          /* lun=0, resv=0 */
    inq.cmd[2] = 0x00;          /* Logical Block Address = 0 */
    inq.cmd[3] = 0x00;          /* Logical Block Address = 0 */
    inq.cmd[4] = 0x00;          /* Logical Block Address = 0 */
    inq.cmd[5] = 0x00;          /* Logical Block Address = 0 */
    inq.cmd[6] = 0x00;          /* reserved */
    inq.cmd[7] = 0x00;          /* reserved */
    inq.cmd[8] = 0x00;          /* reserved */
    inq.cmd[9] = 0x00;          /* control */

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl READ CAPACITY status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl READ CAPACITY status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-READ CAPACITY (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
            if ((n % 16) == 0) printf("\n[%04d]  ", n );
            printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }

    if ((verbose >= 2) && (status == 0))
    {
    readcap = (scsi_readcap_t *)&inq.cmd[0];
    byteswapU32( (unsigned char *)&readcap->max_logblock_address );
    byteswapU32( (unsigned char *)&readcap->block_length         );
    printf( "Maximum logical block address available: %10u (0x%08lX)\n", 
	readcap->max_logblock_address, 
	readcap->max_logblock_address );
    printf( "                  Block length in bytes: %10u (0x%08lX)\n", 
	readcap->block_length,
	readcap->block_length );
    printf( "                               Capacity: %ldMiB\n", 
	(ulong)((ulong)readcap->max_logblock_address * (ulong)readcap->block_length) / (ulong)(1024*1024) );
    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );

    memset (&inq     , 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0xFF;
    
    inq.cmd[0] = 0x00;          /* TUR */
    inq.cmd[1] = 0x00;          
    inq.cmd[2] = 0x00;          
    inq.cmd[3] = 0x00;          
    inq.cmd[4] = 0x00;
    inq.cmd[5] = 0x00;
    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl TEST UNIT READY status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl TEST UNIT READY status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-TEST UNIT READY (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
            if ((n % 16) == 0) printf("\n[%04d]  ", n );
            printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );

    /***************************************************************************/
    /*** CMD: RTPG                                                           ***/
    memset (&inq, 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0x7FF;

    inq.cmd[ 0] = 0xA3;          /* REPORT TARGET PORT GROUPS */
    inq.cmd[ 1] = 0x0A;          /* <3-Reserved><5-ServiceAction> (0x0a) */
    inq.cmd[ 2] = 0x00;          /* reserved   */
    inq.cmd[ 3] = 0x00;          /* reserved   */
    inq.cmd[ 4] = 0x00;          /* reserved   */
    inq.cmd[ 5] = 0x00;          /* reserved   */
    inq.cmd[ 6] = 0x00;          /* length-msb */
    inq.cmd[ 7] = 0x00;          /* length-    */
    inq.cmd[ 8] = 0x00;          /* length-    */
    inq.cmd[ 9] = 0x00;          /* length-lsb */
    inq.cmd[10] = 0x00;          /* reserved   */
    inq.cmd[11] = 0x00;          /* control    */

    buflen = 0x7FF;
    inq.cmd[6] = (buflen >> 24) & 0xff;
    inq.cmd[7] = (buflen >> 16) & 0xff;
    inq.cmd[8] = (buflen >>  8) & 0xff;
    inq.cmd[9] = (buflen      ) & 0xff;

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl REPORT TARGET PORT GROUPS status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl REPORT TARGET PORT GROUPS status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-REPORT TARGET PORT GROUPS (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
            if ((n % 16) == 0) printf("\n[%04d]  ", n );
            printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }
    if ((verbose >= 2) && (status == 0))
    { int bytes; 
	buf = &inq.cmd[0];
        byteswapU32( (unsigned char *)buf );
	bytes = *(unsigned int *)buf;
	buf = &buf[4];
        printf( "          Total Length: %d bytes\n", bytes );
	while (bytes > 0)
        {
	    if (bytes < sizeof( scsi_rtpg_descriptor_t ))
	    {
		printf( "%%W-%d bytes left < %d bytes needed for target port group descriptor\n",
			bytes, sizeof( scsi_rtpg_descriptor_t ) );
	    }

	    rtpg_desc = (scsi_rtpg_descriptor_t *)buf;
            byteswapU16(&rtpg_desc->tpg_id );
            printf( "-----Target Port Group: %d\n", rtpg_desc->tpg_id );
            printf( "             Preferred: %d %s\n", rtpg_desc->pref, rtpg_pref_tbl[ rtpg_desc->pref ] );
            printf( "          Access State: %X %s\n", rtpg_desc->alua_state, rtpg_access_state_tbl[ rtpg_desc->alua_state ] );
            printf( "      Supported States: %s\n",    rtpg_supported_states( rtpg_desc ) );
            printf( "                Status: %X %s\n", rtpg_desc->status, rtpg_status( rtpg_desc->status ) );
            printf( "       Vendor Specific: %X\n", rtpg_desc->vendor_specific );
	    printf( "            Port Count: %d\n", rtpg_desc->port_count );
           

	    bytes -= sizeof( scsi_rtpg_descriptor_t );
	    bytes += 4;
	    buf    = (char *)&rtpg_desc->port_id[0][0];
	    for (ndx=0; ndx < rtpg_desc->port_count; ndx++)
	    {
                byteswapU32( (unsigned char *)buf );
	        printf( "              Port[%2d]: %04X\n", ndx, *((int *)buf) );
		bytes -= 4;
		buf    = &buf[4];
	    }
	    rtpg_desc = (scsi_rtpg_descriptor_t *)buf;
	}
	if (bytes < 0)
	    printf( "%%W-completed scan of returned rtpg data resulted in %d bytes overrun\n", bytes );
	if (bytes > 0)
	    printf( "%%W-completed scan of returned rtpg data resulted in %d bytes underrun\n", bytes );
    }
    if ((verbose >= 2) && (status != 0)) 
        scsi_sense_buffer( &inq.cmd[0] );



    /***************************************************************************/
    /*** CMD: REPORT LUNS                                                    ***/
    memset (&inq, 0, sizeof (sdata));

    inq.inlen = 0;
    inq.outlen = 0x7FF;

    inq.cmd[ 0] = 0xA0;          /* REPORT LUNS */
    inq.cmd[ 1] = 0x00;          /* reserved   */
    inq.cmd[ 2] = 0x00;          /* SELECT REPORT type 00, 01, 02 */
    inq.cmd[ 3] = 0x00;          /* reserved   */
    inq.cmd[ 4] = 0x00;          /* reserved   */
    inq.cmd[ 5] = 0x00;          /* reserved   */
    inq.cmd[ 6] = 0x00;          /* length-msb */
    inq.cmd[ 7] = 0x00;          /* length-    */
    inq.cmd[ 8] = 0x00;          /* length-    */
    inq.cmd[ 9] = 0x00;          /* length-lsb */
    inq.cmd[10] = 0x00;          /* reserved   */
    inq.cmd[11] = 0x00;          /* control    */

    buflen = 0x7FF;
    inq.cmd[6] = (buflen >> 24) & 0xff;
    inq.cmd[7] = (buflen >> 16) & 0xff;
    inq.cmd[8] = (buflen >>  8) & 0xff;
    inq.cmd[9] = (buflen      ) & 0xff;

    status = ioctl(fd, SCSI_IOCTL_SEND_COMMAND, &inq);
    if (verbose >= 1)
        printf( "\n%%I-ioctl REPORT LUNS status=%d %s\n", status, scsi_status(status) );
    else if (status != 0)
    {
        printf( "\n%%E-ioctl REPORT LUNS status=%d %s\n", status, scsi_status(status) );
	if (status == -1)
            perror( "%E-REPORT LUNS (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
            if ((n % 16) == 0) printf("\n[%04d]  ", n );
            printf(" %02X", inq.cmd[n] );
        }
        printf( "\n");
    }

    if ((verbose >= 2) && (status == 0))
    { int bytes, n=0; unsigned long lun;
        buf = &inq.cmd[0];
        byteswapU32( (unsigned char *)buf );
        bytes = *(unsigned int *)buf;
        buf = &buf[4];
        printf( "          Total Length: %d bytes\n", bytes );
        printf( "          <reserved:4>: %d\n", *(unsigned int *)buf );
        buf = &buf[4];
        while (bytes > 0)
        {
            byteswapLUN( (unsigned char *)buf );
            lun = *(unsigned long *)buf;
            buf = &buf[8];
            printf( "              lun[%3d]: %016lX (%ld)\n",  n++, lun, lun );
            bytes -= 8;
        }
    }
    if ((verbose >= 2) && (status != 0))
        scsi_sense_buffer( &inq.cmd[0] );

    if (sg_verno > 0)
        sg_report_luns(fd);

    if (sleeping != 0)
    {
	printf( "%%W-holding device open and going to sleep now, ^C to kill when ready...\n");
	while(sleeping != 0)
	    sleep(1);
    }
    if (looping != 0)
    {
	printf( "%%W-holding device open and going to sleep now, will loop in 15...\n");
	sleep(15);
	printf( "%%I-waking up...\n");
	goto again;
    }
    close(fd);


}


/*
 * might want to check the following sys limit if the lun list seems
 * big...
 *
 * max_scsi_report_luns: the maximum number of LUNS that will be
 * returned from the REPORT LUNS command. 8 times this value must
 * be allocated. In theory this could be up to an 8 byte value, but
 * in practice, the maximum number of LUNs suppored by any device
 * is about 16k.
 *
 * static unsigned int max_scsi_report_luns = 511;
 *
 */

int sg_report_luns(int fd)
{
    typedef struct sdata_t {
        u_int32_t inlen;
        u_int32_t outlen;
        unsigned char cmd[256];
    } sdata;
    sdata                inq;
    scsi_inqdata_t      *response;
    sg_io_hdr_t          io_hdr;
    char                 sense_buffer[ 256];
    char                 data_buffer [8192];
    char                 *buf;
    int                  buflen, n;
    int			 status;


    memset(&inq,          0, sizeof(sdata       ));
    memset(&io_hdr,       0, sizeof(sg_io_hdr_t ));
    memset(&data_buffer,  0, sizeof(data_buffer ));
    memset(&sense_buffer, 0, sizeof(sense_buffer));

    inq.cmd[ 0] = 0xA0;          /* REPORT LUNS */
    inq.cmd[ 1] = 0x00;          /* reserved   */
    inq.cmd[ 2] = 0x00;          /* SELECT REPORT type 00, 01, 02 */
    inq.cmd[ 3] = 0x00;          /* reserved   */
    inq.cmd[ 4] = 0x00;          /* reserved   */
    inq.cmd[ 5] = 0x00;          /* reserved   */
    inq.cmd[ 6] = 0x00;          /* length-msb */
    inq.cmd[ 7] = 0x00;          /* length-    */
    inq.cmd[ 8] = 0x00;          /* length-    */
    inq.cmd[ 9] = 0x00;          /* length-lsb */
    inq.cmd[10] = 0x00;          /* reserved   */
    inq.cmd[11] = 0x00;          /* control    */

    buflen = 0x1FFF;		 /* 8191       */
    inq.cmd[6] = (buflen >> 24) & 0xff;
    inq.cmd[7] = (buflen >> 16) & 0xff;
    inq.cmd[8] = (buflen >>  8) & 0xff;
    inq.cmd[9] = (buflen      ) & 0xff;

    io_hdr.interface_id    = 'S';
    io_hdr.cmd_len         =  12;
    io_hdr.iovec_count     =   0; 
    io_hdr.mx_sb_len       = 255;		/* sense buffer is 256 bytes, but that's bigger than unsigned char allows */
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len       = buflen;
    io_hdr.dxferp          = data_buffer;
    io_hdr.cmdp            = inq.cmd;
    io_hdr.sbp             = sense_buffer;
    io_hdr.timeout         = 15000;             /* 15000 millisecs == 15 seconds   */
    io_hdr.flags           = 0;                 /* take defaults: indirect IO, etc */
    io_hdr.pack_id         = 0; 
    io_hdr.usr_ptr         = NULL; 

    status = ioctl(fd, SG_IO, &io_hdr);
    if (verbose >= 1)
    {
        printf( "\n%%I-SG ioctl REPORT LUNS status=%d %s\n", status, scsi_status(status) );
        printf("%%I-SG REPORT LUNS return code (scsi,host,driver) %02X.%02X.%02X scsi=%s {sg_info=%d}\n", 
		io_hdr.status,  io_hdr.host_status, io_hdr.driver_status, scsi_status( io_hdr.status ), io_hdr.info );
        printf("%%I-SG REPORT LUNS duration=%u millisecs, resid=%d\n",
               io_hdr.duration, io_hdr.resid);
    }
    else if (status != 0)
    {
        printf( "\n%%E-SG ioctl REPORT LUNS status=%d %s\n", status, scsi_status(status) );
        printf("%%E-SG REPORT LUNS return code (scsi,host,driver) %02X.%02X.%02X scsi=%s {sg_info=%d}\n", 
		io_hdr.status,  io_hdr.host_status, io_hdr.driver_status, scsi_status( io_hdr.status ), io_hdr.info );
        if (status == -1)
            perror( "%E-SG REPORT LUNS (perrno)");
    }

    if (verbose >= 3)
    {
        for (n=0;n<256;n++)
        {
            if ((n % 16) == 0) printf("\n[%04d]  ", n );
            printf(" %02X", data_buffer[n] );
        }
    }

    if ((verbose >= 2) && (status == 0))
    { int bytes, n=0; unsigned long lun;
        buf = &data_buffer[0];
        byteswapU32( (unsigned char *)buf );
        bytes = *(unsigned int *)buf;
        buf = &buf[4];
        printf( "          Total Length: %d bytes\n", bytes );
        printf( "          <reserved:4>: %d\n", *(unsigned int *)buf );
        buf = &buf[4];
        while (bytes > 0)
        {
            byteswapLUN( (unsigned char *)buf );
            lun = *(unsigned long *)buf;
            buf = &buf[8];
            printf( "              lun[%3d]: %016lX (%ld)\n",  n++, lun, lun );
            bytes -= 8;
        }
    }

    if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) 
    {
        printf("%%I-SG REPORT LUNS sense data reported %d bytes\n", io_hdr.sb_len_wr );
        scsi_sense_buffer( &sense_buffer[0] );
    }
    return(1);
}
 

int byteswapU16( unsigned char *data )
{
unsigned char bytes[2];
    /* A1 94 1A 1D e.g. */
    bytes[1] = data[0];
    bytes[0] = data[1];

    data[0]  = bytes[0];
    data[1]  = bytes[1];
    return(0);
}

int byteswapU32( unsigned char *data )
{
unsigned char bytes[4];
    /* A1 94 1A 1D e.g. */
    bytes[3] = data[0];
    bytes[2] = data[1];
    bytes[1] = data[2];
    bytes[0] = data[3];

    /* 1D 1A 94 A1 */
    data [0] = bytes[0];
    data [1] = bytes[1];
    data [2] = bytes[2];
    data [3] = bytes[3];
    return(0);
}

int byteswapU64( unsigned char *data )
{
unsigned char bytes[8];
    /* A1 94 1A 1D e.g. */
    bytes[7] = data[0];
    bytes[6] = data[1];
    bytes[5] = data[2];
    bytes[4] = data[3];
    bytes[3] = data[4];
    bytes[2] = data[5];
    bytes[1] = data[6];
    bytes[0] = data[7];

    /* 1D 1A 94 A1 */
    data [0] = bytes[0];
    data [1] = bytes[1];
    data [2] = bytes[2];
    data [3] = bytes[3];
    data [4] = bytes[4];
    data [5] = bytes[5];
    data [6] = bytes[6];
    data [7] = bytes[7];
    return(0);
}

int byteswapLUN( unsigned char *data )
{
unsigned char bytes[8];
    /* 00 01 00 00 00 00 00 00 00 = 00 01 ? */
    bytes[7] = data[0];
    bytes[6] = data[1];
    bytes[5] = data[2];
    bytes[4] = data[3];
    bytes[3] = data[4];
    bytes[2] = data[5];
    bytes[1] = data[6];
    bytes[0] = data[7];

    /* 1D 1A 94 A1 */
    /* 00 01 00 00 00 00 00 00 */
    data [0] = bytes[6];
    data [1] = bytes[7];
    data [2] = bytes[0];
    data [3] = bytes[1];
    data [4] = bytes[2];
    data [5] = bytes[3];
    data [6] = bytes[4];
    data [7] = bytes[5];
    return(0);
}




int parse_args(int argc, char *argv[])
{
int match = 0;
int ndx;
int argndx = 1;
int strndx = 0;
char *ptr;

    if (argc < 2)
    {
        useage();
        exit(0);
    }

    while (argndx<argc)
    {
        if (argv[argndx][0] == '-')
        {
            ptr = &argv[argndx][1];
            while (*ptr != 0)
            {
                switch (*ptr)
                {
                    case 'v': /* verbose mode */ verbose++;  break;
		    case 's': /* sleep mode   */ sleeping++; break;
	  	    case 'l': /* loop mode    */ looping++;  break;
                }
                ptr++;
            }
        }
        else
        {
            switch (strndx++)
            {
                case 0: strcpy(device_path, argv[argndx] );
	    }
	}
	argndx++;
    }
}
int useage()
{
    printf( "Useage:\n");
    printf( "  scsi_inq <target> {-v}\n");
    printf( "  scsi_inq -v\n");
    printf( "       -v : verbose mode, output returned inquiry buffer\n");
    printf( "\n");

}

char *scsi_status(int status)
{
    switch(status)
    {
	case 0x00: return( "(GOOD - Completed Successfully)" ); break;
	case 0x02: return( "(CHECK CONDITION)" ); break;
	case 0x08: return( "(BUSY)" ); break;
        case 0x18: return( "(RESERVATION CONFLICT)" ); break;
	case 0x28: return( "(QUEUE FULL)" ); break;
	default  : return( "<Reserved>" ); break;
    }
}

int scsi_sense_buffer( unsigned char *sense )
{
int byt = 0;
int asc,ascq;

    printf( "Sense Buffer [%02d] %02X ", byt, *sense );
    if ((*sense & 0x7F) == 0x70) printf( " Valid/current " );
    if ((*sense & 0x7F) == 0x71) printf( " Valid/deferred" );
    printf( "\n" );
    sense++; byt++;

    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X  Sense Key %s\n", byt, *sense, scsi_sense_key(*sense) ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X  Add'l Length\n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X  ASC\n", byt, *sense ); asc = *sense++; byt++;
    printf( "             [%02d] %02X  ASCQ", byt, *sense ); ascq = *sense++; byt++;
#ifdef ASC_ASCQ
    printf( " -- %02X/%02X %s", asc, ascq, decode_asc_ascq(asc, ascq) );
#endif 
    printf( "\n");
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;
    printf( "             [%02d] %02X \n", byt, *sense ); sense++; byt++;


    return(0);
}

char *scsi_sense_key( unsigned char skey )
{
    switch(skey)
    {
	case 0x0: return( "(No Sense)" ); break;
	case 0x1: return( "(Recovered Error)" ); break;
	case 0x2: return( "(Not Ready)" ); break;
	case 0x3: return( "(Medium Error)" ); break;
	case 0x4: return( "(Hardware Error)" ); break;
	case 0x5: return( "(Illegal Request)" ); break;
	case 0x6: return( "(Unit Attention)" ); break;
	case 0x7: return( "(Data Protect)" ); break;
	case 0x8: return( "(Blank Check)" ); break;
	case 0x9: return( "(Vendor Specific)" ); break;
	case 0xA: return( "(Copy Aborted)" ); break;
	case 0xB: return( "(Aborted Command)" ); break;
	case 0xC: return( "(Equal)" ); break;
	case 0xD: return( "(Volume Overflow)" ); break;
	case 0xE: return( "(Miscompare)" ); break;
	case 0xF: return( "(Reserved)" ); break;
        default : return( "(?)" ); break;
    }
}

char *pg83_eui64fmt (pg83_ident_t *ident )
{
static char tmpline[128];
int ndx;

     sprintf( tmpline, "" );
     if (ident->ident_type != 2) return(tmpline);
     for (ndx=0; pg83_eui64[ndx].code != -1; ndx++)
     {
	if (pg83_eui64[ndx].code == ident->ident_len)
	{
	    sprintf( "- %s", pg83_eui64[ndx].desc );
	    return(tmpline);
	}
     }
     sprintf( "- %s", pg83_eui64[ndx].desc );
     return(tmpline);
}

char *pg83_naafmt (pg83_ident_t *ident )
{
static char tmpline[128];
int ndx;
int code;

     sprintf( tmpline, "" );
     if (ident->ident_type != 3) return(tmpline);
     code = (ident->ident[0] >> 4) & 0xF;
     for (ndx=0; pg83_naa[ndx].code != -1; ndx++)
     {
	if (pg83_naa[ndx].code == ident->ident_len)
	    printf( "     Identifier Format: %X - %s\n",code, pg83_naa[ndx].desc );
	    return(pg83_naa[ndx].desc );
     }
     printf( "     Identifier Format: %X - %s\n",code, pg83_naa[ndx].desc );
     return(pg83_eui64[ndx].desc);
};


char *rtpg_supported_states( scsi_rtpg_descriptor_t *rtpg )
{
static char tmpline[128];
unsigned char *ptr;

    ptr = (unsigned char *)rtpg;
    ptr++;
    if ( *ptr == 0 )
        return("00 - Vendor unique asymmetric state definitions");
    sprintf( tmpline, "%02X - Act/Opt=%c Act/Non=%c Standby=%c Unavail=%c Transit=%c", *ptr,
	((rtpg->ao_sup==0) ? 'N' : 'Y'),
	((rtpg->an_sup==0) ? 'N' : 'Y'),
	((rtpg->s_sup ==0) ? 'N' : 'Y'),
	((rtpg->u_sup ==0) ? 'N' : 'Y'),
	((rtpg->t_sup ==0) ? 'N' : 'Y'));
    return(tmpline);
}

char *rtpg_status(unsigned char status )
{
static char tmpline[128];
int ndx;
int code;

     sprintf( tmpline, "" );
     code = status;
     for (ndx=0; rtpg_status_tbl[ndx].code != -1; ndx++)
     {
	if (rtpg_status_tbl[ndx].code == code )
	    return(rtpg_status_tbl[ndx].desc );
     }
     return(rtpg_status_tbl[ndx].desc);
}


char *response_byte5(scsi_inqdata_t *response)
{ 
static char tmpline[128];

    /* SCCS<n> ACCx<n> TGPS<n> 3PCx<n> RESV<n> PROT<n> */
    sprintf( tmpline, "SCCS<%d> ACC_<%d> TPGS<%d> 3PC_<%d> RESV<%d> PROT<%d>",
	response->sccs, response->acc, response->tpgs, response->third_party,
	response->_resv5_1, response->protection );
    return(tmpline);
}
int tgps_byte5(scsi_inqdata_t *response)
{ 
static char tmpline[128];

    if (response->tpgs != 0)
    {
	printf( "6.4.2 ... TPGS field\n");
	if (response->tpgs == 1)
	    printf( "    01b Only implicit ALUA supported (see 5.9.2.7)\n" );
	if (response->tpgs == 2)
	    printf( "    10b Only explicit ALUA supported (see 5.9.2.8)\n" );
	if (response->tpgs == 3)
	    printf( "    11b Both implicit and explicit ALUA supported (see 5.9.2.7/8)\n" );
    }
    return(1);
}

#ifdef ASC_ASCQ 
char *decode_asc_ascq(int asc, int ascq)
{
static char tmpline[128];
char *rsn = NULL;
int   ndx = 0;
    if (asc >= 0x80) return( "<Vendor-Specific>" );
    while (asc_ascq[ndx].asc != 0xFF)
    {
	if (asc_ascq[ndx].asc == asc)
	{
	    if (asc_ascq[ndx].ascq == ascq) return(asc_ascq[ndx].description);
	    if (rsn == NULL) rsn = asc_ascq[ndx].description;
	}
	ndx++;
    }
    if (rsn != NULL) 
    {
	sprintf( tmpline, "<NotExact> %s", rsn );
        return(tmpline);
    }
    return("<NotFound>");
}
#endif

