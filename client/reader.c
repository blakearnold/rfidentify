/**
 * reader.c
 *
 * Device library for an RFID reader.
 * Particularly the DLP-RFID1.
 *
 * @author Willi Ballenthin
 * @date   Spring, 2010
 */
#include "reader.h"
#include <string.h>
#include <stdio.h>

struct reader *reader_create() {
  return malloc(1 * sizeof(struct reader));
}

void reader_init(struct reader *reader,
				 struct ftdi_context *ftdic,
				 struct usb_device *dev) {
  reader->connected = 0;
  reader->ftdic = ftdic;
  reader->dev   = dev;
}

struct tag *tag_create() {
  return malloc(1 * sizeof(struct tag));
}

void tag_init(struct tag *tag,
			  char *id,
			  struct reader *parent) {
	tag->id = id;
	tag->parent = parent;
}

/**
* @param readers Probably should be an empty list.
*/
list *find_all_readers(struct list *readers) {
    int rc;
    struct ftdi_device_list *devlist;
    struct ftdi_context ftdi;

    ftdi_init(&ftdi);
    rc = ftdi_usb_find_all(&ftdi, &devlist, RFID1_VID, RFID1_PID);
    ftdi_deinit(&ftdi);

    if(rc < 0) return NULL;

    struct ftdi_device_list *dp = devlist;
    while(dp){
        struct ftdi_context *c = malloc(sizeof(struct ftdi_context));
        if(c == NULL){
            // Out of memory !!
            // TODO: Clean up
            return NULL;
        }
        ftdi_init(c);

		struct reader *r = reader_create();
		reader_init(r, c, dp->dev);
		list_push(readers, r);

        dp = dp->next;
    }

    ftdi_list_free(&dp);

    return readers;
}

ReturnCode reader_connect(struct reader *reader,
						  int beep) {
	ReturnCode rc;

    if(reader->ftdic == NULL || reader->dev == NULL) {
	  printf("reader entry null\n");
	  return RC_NULL_ERROR;
	}

    if(ftdi_usb_open_dev(reader->ftdic, reader->dev) < 0){
	  printf("ftdi usb open dev failed\n");
	  return RC_IO_ERROR;
    }

    if((rc = reader_reset(reader)) != RC_SUCCESS){
		printf("reset failed\n");
        ftdi_usb_close(reader->ftdic);
        return rc;
    }

    if((rc = reader_purge(reader)) != RC_SUCCESS){
		printf("purge failed\n");
        ftdi_usb_close(reader->ftdic);
        return rc;
    }

    if(ftdi_set_baudrate(reader->ftdic, RFID1_BAUDRATE) < 0){
		printf("setbaudrate failed\n");
        ftdi_usb_close(reader->ftdic);
        return RC_IO_ERROR;
    }

    usleep(10000);


    if((rc = reader_ping(reader)) != RC_SUCCESS){
        ftdi_usb_close(reader->ftdic);
        return rc;
    }

    reader->connected = 1;

    if(beep) {
        reader_pass_beep(reader);
    }


    return RC_SUCCESS;
}

ReturnCode reader_disconnect(struct reader *reader) {
    if(!reader->connected) return RC_NOT_CONNECTED;
    reader->connected = 0;
    if(ftdi_usb_close(reader->ftdic) < 0){
        return RC_IO_ERROR;
    }
    return RC_SUCCESS;
}

ReturnCode reader_reset(struct reader *reader)
{
    if(ftdi_setdtr(reader->ftdic, 1) < 0) return RC_IO_ERROR;
    usleep(20000);
    if(ftdi_setdtr(reader->ftdic, 0) < 0) return RC_IO_ERROR;
    usleep(110000);
    return RC_SUCCESS;
}

ReturnCode reader_pass_beep(struct reader *reader)
{
    if(!reader->connected) return RC_NOT_CONNECTED;
    return reader_txPacketCheckAck(reader,
								   RFID1_PKT_PASSBEEP,
								   strlen(RFID1_PKT_PASSBEEP));
}

ReturnCode reader_fail_beep(struct reader *reader)
{
    if(!reader->connected) return RC_NOT_CONNECTED;
    return reader_txPacketCheckAck(reader,
								   RFID1_PKT_FAILBEEP,
								   strlen(RFID1_PKT_FAILBEEP));

}

ReturnCode reader_ping(struct reader *reader)
{
    int rc;
	int i;
    for(i=0; i<5; i++){
        rc = reader_txPacketCheckAck(reader,
									 RFID1_PKT_PING,
									 strlen(RFID1_PKT_PING));
        if(rc == 0) return RC_SUCCESS;
    }
    return RC_NO_PING;
}

ReturnCode reader_purge(struct reader *reader)
{
    if(ftdi_usb_purge_buffers(reader->ftdic) < 0){
        return RC_IO_ERROR;
    }else{
        return RC_SUCCESS;
    }
}

int reader_read_packet(struct reader *reader,
					   char *buf,
					   int maxlen)
{
    int rc;
	int cnt;
    if(maxlen > 0)
	  buf[0] = '\0'; // in case of fail nul buffer

    for(cnt = 0; cnt < maxlen-1; cnt++){

        int timeout_cnt = 10;
        do {
		  // why 1?
            rc = ftdi_read_data(reader->ftdic, (unsigned char *)buf, 1);
            if(rc < 0){
                // Error, return error code
                return -1;
            }else if( rc > 0){
                // Process it
                break;
            }
            usleep(1000);
            timeout_cnt--;
        } while(timeout_cnt);

        if(rc == 0) {
            ftdi_usb_purge_rx_buffer(reader->ftdic);
            return 0;
        }

        // EOP char reached, return
        if(*buf == RFID1_EOP_CHAR){
            *buf = '\0';
            return cnt;
        }

        buf++;
    }

    // Reached maxlen, purge buffers and return
    buf[maxlen-1] = '\0';
    ftdi_usb_purge_rx_buffer(reader->ftdic);
    return maxlen;
}

int reader_txPacketRxReply(struct reader *reader,
						   char *tx_packet,
						   int tx_packet_len,
						   char *rx_buf,
						   int rx_buf_len,
						   int *payload_index)
{
    int tx_pktlen = tx_packet_len;
    int rc = ftdi_write_data(reader->ftdic, (unsigned char *)tx_packet, tx_pktlen);
	int i;

    if(rc < 0){
        return -1; // Transmit error
    }else if(rc < tx_pktlen){
        return -1; // Didn't transmit complete packet
        // (should this be a different error or ignored?)
    }

    // Have 3 goes at receiving the right packet then give up.
    for(i=0; i<10; i++){
        rc = reader_read_packet(reader, rx_buf, rx_buf_len);
        if(rc < 0){
            return -2; // Receive error
        }

        // Check for echo of tx packet, if the received packet is shorter
        // than the transmitted packet then obviously it's not right
        // (but first we have to check the rx buffer was big enough,
        //  if it wasn't we don't bother checking for a match and just
        //  return 'no echo received'.)
        if(rx_buf_len < tx_pktlen) return RC_SUCCESS;

        if(rc < tx_pktlen){
            usleep(1000);
            continue;
        }

        // Look for the echo of the transmitted packet
        char *p = strstr(rx_buf, tx_packet);
		if (p == NULL) {
        //int pos = rx_buf.find(tx_packet);
        //if(pos == string::npos) {
            usleep(1000);
            continue;
        }
        int pos = p - rx_buf;

        // We put return a pointer to the payload if rx_payload is non-null.
        if(payload_index != NULL){
            // Move to the end of the packet
            pos += tx_pktlen;

            if(pos + 3 >= rc || pos + 3 > rx_buf_len){
                (*payload_index) = -1;
                return rc;
            }

            // Echo should be terminated by cr, nl
            if(rx_buf[pos] != '\r' || rx_buf[pos+1] != '\n'){
                (*payload_index) = -1;
                return rc;
            }

            // Skip over the cr/nl and set that value for rx_payload
            (*payload_index) = pos + 2;
            return rc;
        }

        return rc;
    }

    // Return 'no echo received'
    return 0;
}


ReturnCode reader_txPacketCheckAck(struct reader *reader,
								   char *tx_packet,
								   int tx_packet_len) {
	char buf[100];
	int payload_index = 1;
    int rc = reader_txPacketRxReply(reader, tx_packet, tx_packet_len, buf, 100, &payload_index);
    if(rc < 1)
	  return RC_ERROR;
    return RC_SUCCESS;
}

int reader_parse_poll_packet(struct reader *reader,
							 struct list *tagList,
							 char *packetPayload,
							 int packetPayload_len)
{
    const char *p = packetPayload;
    char idbuf[RFID_ID_LEN+1];
    int pos;

    for(;;){
        while(*p != '['){
            if(*p == '\0') return list_size(tagList);
            p++;
        }
        p++;
        pos = 0;
        // While we have a valid ID character, construct the ID.
        while((*p >= '0' && *p <= '9') ||
              (*p >= 'A' && *p <= 'F') ||
              (*p >= 'a' && *p <= 'f')){

            if(pos >= RFID_ID_LEN) break;
            idbuf[pos++] = *p++;
        }


        // Check it's a valid ID (must be followed by a ',' delimiter
        if(*p == ',' && pos > 0){
			struct tag *tag;
            idbuf[pos] = '\0';
			tag = tag_create();
			tag_init(tag, strdup(idbuf), reader);
            if(tag != NULL){
				list_push(tagList, tag);
            }
        }
        else if(*p == '\0') {
            return list_size(tagList);
        }
    }
}

int reader_poll15693(struct reader *reader,
					 list *tagList)
{
	char buf[6000];
    int payload_index;
    int rc;
    if(reader_txPacketCheckAck(reader, RFID1_PKT_MODEUID, strlen(RFID1_PKT_MODEUID)))
	  return -1;
    if(reader_txPacketCheckAck(reader, RFID1_PKT_AGCTGL, strlen(RFID1_PKT_AGCTGL)))
	  return -1;
    if(reader_txPacketCheckAck(reader, RFID1_PKT_AMPMTGL, strlen(RFID1_PKT_AMPMTGL)))
	  return -1;

    if(reader_purge(reader))
	  return -1;

    rc = reader_txPacketRxReply(reader,
								RFID1_PKT_15693INVRQ,
								strlen(RFID1_PKT_15693INVRQ),
								buf,
								6000,
								&payload_index);

    if(rc < 1)
	  return -1;
    if(payload_index < 0)
	  return -1;

	// this may not be good. destructive on buf?
	char *payload = buf + payload_index;
    return reader_parse_poll_packet(reader, tagList, payload, strlen(payload));
}

int reader_pollTi(struct reader *reader,
				  list *tagList)
{
	char buf[6000];
    int payload_index;
    int rc;
    if(reader_txPacketCheckAck(reader, RFID1_PKT_TAGIT, strlen(RFID1_PKT_TAGIT)))
	  return -1;
    if(reader_txPacketCheckAck(reader, RFID1_PKT_AGCTGL, strlen(RFID1_PKT_AGCTGL)))
	  return -1;
    if(reader_txPacketCheckAck(reader, RFID1_PKT_AMPMTGL, strlen(RFID1_PKT_AMPMTGL)))
	  return -1;

    if(reader_purge(reader))
	  return -1;

    rc = reader_txPacketRxReply(reader,
								RFID1_PKT_TISIDPOLL,
								strlen(RFID1_PKT_TISIDPOLL),
								buf,
								6000,
								&payload_index);

    if(rc < 1)
	  return -1;
    if(payload_index < 0)
	  return -1;

	// this may not be good. destructive on buf?
	char *payload = buf + payload_index;
    return reader_parse_poll_packet(reader, tagList, payload, strlen(payload));
}
