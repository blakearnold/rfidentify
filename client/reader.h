/**
 * @file reader.h
 *
 * Device library for an RFID reader.
 * Particularly the DLP-RFID1.
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 */

#include "list.h"
#include <ftdi.h>

/// Device error.
#define EDEVERR -1

/// True.
#define BEEP 1

/// False.
#define NOBEEP 0

/// Possible return codes from reader.
typedef enum { RC_SUCCESS = 0,
	       RC_ERROR,
	       RC_NULL_ERROR,
	       RC_IO_ERROR,
	       RC_NO_PING,
	       RC_NOT_CONNECTED
} ReturnCode;

#define RFID1_VID   0x0403
#define RFID1_PID   0xfbfc
#define RFID1_DESC   "DLP-RFID1"
#define RFID1_BAUDRATE  115200

/*
  Inferred packet format:

  '01'      SOP (might signifiy something?)
  LEN       1 byte giving length in bytes
  '0003'    Some sort of code that is constant
  '04'      Another code that is constant (Write request?)

  ADDR      1byte I think is the address to write to
  DATA      (LEN-7) bytes of data to write I think

  '0000'    EOP
*/
#define RFID1_PKT_PING          "0108000304FF0000"
#define RFID1_PKT_AGCTGL        "0109000304F0000000"
#define RFID1_PKT_AMPMTGL       "0109000304F1FF0000"
#define RFID1_PKT_PASSBEEP      "010900030419F00000"
#define RFID1_PKT_FAILBEEP      "010900030420F00000"
#define RFID1_PKT_15693INVRQ    "010B000304140601000000"
#define RFID1_PKT_TISIDPOLL     "010B000304340050000000"
#define RFID1_PKT_MODEMEM       "010C00030410002101000000"
#define RFID1_PKT_MODEUID       "010C00030410002101020000"
#define RFID1_PKT_TAGIT         "010C00030410002101130000"


#define RFID1_EOP_CHAR  '>'

#define RFID_ID_LEN 20  //??

/// Completely identifies a currently connected RFID reader.
struct reader {
  int connected;
  struct ftdi_context *ftdic;
  struct usb_device *dev;
};

/// Return a newly allocated reader.
struct reader *reader_create();

/// Initialize a newly allocated reader.
void reader_init(struct reader *reader,
		 struct ftdi_context *ftdic,
		 struct usb_device *dev);

/// An RFID identification.
struct tag {
  char *id;              /// Unique ID.
  struct reader *parent; /// The reader who identified the tag.
};

/// Returns a newly allocated tag.
struct tag *tag_create();

/// Initializes a newly allocated tag.
void tag_init(struct tag *tag,
	      char *id,
	      struct reader *parent);

/// Returns a list of all connected readers.
list *find_all_readers(list* readers);

/// Connects to a given reader.
ReturnCode reader_connect(struct reader *reader,
			  int beep);

/// Disconnects from a given reader.
ReturnCode reader_disconnect(struct reader *reader);

/// Resets a given reader.
ReturnCode reader_reset(struct reader *reader);

/// Causes the reader to emit the PASS beep.
ReturnCode reader_pass_beep(struct reader *reader);

/// Causes the reader to emit the FAIL beep.
ReturnCode reader_fail_beep(struct reader *reader);

/// Pings a given reader.
ReturnCode reader_ping(struct reader *reader);

/// Purges a given reader.
ReturnCode reader_purge(struct reader *reader);

/// Reads a packet from a reader into a buffer.
int reader_read_packet(struct reader *r,
		       char *buf,
		       int maxlen);

/// TODO.
int reader_txPacketRxReply(struct reader *reader,
			   char *tx_packet,
			   int tx_packet_len,
			   char *rx_buf,
			   int rx_buf_len,
			   int *payload_index);

/// Returns if successfully acknowledged.
ReturnCode reader_txPacketCheckAck(struct reader *reader,
				   char *tx_packet,
				   int tx_packet_len);

/// Parses a packet returned from a reader poll.
int reader_parse_poll_packet(struct reader *reader,
			     struct list *tagList,
			     char *packetPayload,
			     int packetPayload_len);

/// Causes the reader to poll for and ISO15693 tag.
int reader_poll15693(struct reader *reader,
		     list *tagList);

/// Attempts to poll the reader for various tags.
int reader_pollTi(struct reader *reader,
		  list *tagList);		
