
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#define WRITE_DEBUG 1
#define READ_DEBUG 1

/**
 * ISO15693 CRC-16
 */
uint16_t calc_crc(uint8_t *data, int len) {
  const uint16_t preset     = 0xFFFF;
  const uint16_t polynomial = 0x8408;
  uint16_t crc;
  uint8_t lsb, msb;
  int i, j;

  crc = preset;

  for (i = 0; i < len; i++) {
	crc ^= data[i];

	for (j = 0; j < 8; j++) {
	  if (crc & 0x1) { // "If CRCValue And &H1 Then"
		crc = (crc >> 1) ^ polynomial;
	  }
	  else {
		crc >>= 1;
	  }
	}
  }
  
  crc = ~(crc - 65536);
  lsb = crc % 256;
  msb = (crc - lsb) / 256;

  printf("low: %X\n", lsb);
  printf("high: %X\n", msb);

  return crc;
}

uint16_t apsx_crc(uint8_t *data, int len) {
  if (len < 3)
	return 0;

  return calc_crc(data + 2, len - 2);
}


int init_serial(const char *dev) {
	int fd;
	
	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		perror("open_port: Unable to open device.");
		return 0;
	} else {
		fcntl(fd, F_SETFL, 0);
	}
  
	struct termios options;
	tcgetattr(fd, &options);
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	tcsetattr(fd, TCSANOW, &options);
	return fd;
}

int serial_write(int fd, uint8_t* buffer, size_t num)
{
#ifdef WRITE_DEBUG
	uint8_t* p = buffer;
	int i;
	fprintf(stderr, "writing: ");
	for(i = 0; i < num; i++)
	{
		fprintf(stderr, "%X ", *(p + i));
	}
	fprintf(stderr, "\n");
#endif

	int n = write(fd, buffer, num);
	return n;
}

int serial_read(int fd, uint8_t* buffer, size_t num)
{
	int n = read(fd, buffer, num);

#ifdef READ_DEBUG
	int i;
	uint8_t* p = buffer;
	fprintf(stderr, "reading (%d): ", n);
	for (i = 0; i < n; i++)
	{
		fprintf(stderr, "%X ", *(p + i));
	}
	fprintf(stderr, "\n");
#endif

	return n;
}



void red_on(int fd) {

  uint8_t data[7] = { 0x06,  // six bytes sent
					  0x00,  // 0 bytes expected
					  0x00,  // flag
					  0xFD,  // red on
					  0x00,  // block
					  0x00,  // crc low
					  0x00}; // crc high

  uint16_t crc = apsx_crc(data, 5);
  
  data[5] = crc % 256;
  data[6] = (crc - (crc % 256)) / 256;

  serial_write(fd, data, 7);
}


void rfid_fa(int fd) {

  uint8_t data[1] = { 0xFA };

  serial_write(fd, data, 1);

}




int main() {


  uint8_t data[3] = { 0x27, 0x01, 0x00 };

  calc_crc(data, 3);


  int fd;

  fd = init_serial("/dev/ttyUSB0");

  if (fd == 0) {
	printf("unable to open\n");
  }
  else {
	printf("opened\n");
  }

  uint8_t buf[1024];


  if (errno) {
	printf("error: %s\n", strerror(errno));
  }

  rfid_fa(fd);

  if (errno) {
	printf("error: %s\n", strerror(errno));
  }

  serial_read(fd, buf, 1023);

  if (errno) {
	printf("error: %d %s\n",errno,  strerror(errno));
  }
  
  return 1;
}
