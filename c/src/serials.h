#ifndef SERIALS_H
#define SERIALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef SERIALS_ECHO_RX
#define SERIALS_ECHO_RX			0
#endif

#ifndef SERIALS_APPEND_CRLF
#define SERIALS_APPEND_CRLF		1
#endif

#ifndef SERIALS_ARG_TO_ASCII
#define SERIALS_ARG_TO_ASCII	0
#endif

// Byte-stuffing bytes
#define SERIALS_BS_BEGIN	0x3C 	// '<'
#define SERIALS_BS_END	    0x3E 	// '>'
#define SERIALS_BS_ESC	    0x2A	// '*'

// Packet structure (without byte stuffing)
// [CMD] [ARG] [DATA]*
// The argument belongs to the CMD while DATA is set
// by the user. All fields are 1-byte wide, except DATA which is a
// byte array.
// Examples:
// <s,Hello World>
// <s,*<Hello*>>

#define SERIALS_ARG_DATA	0x2C	// ','
#define SERIALS_ARG_NODATA	0x00	// null

typedef enum serials_cmd_e
{
	SERIALS_CMD_NULL		= 0x00,
	SERIALS_CMD_ACK 		= 0x6b,	// 'k'
	SERIALS_CMD_ERR 		= 0x65,	// 'e'
	SERIALS_CMD_BYTES		= 0x62, // 'b'
	SERIALS_CMD_STRING		= 0x73, // 's'
} serials_cmd_t;

typedef struct serials_pkt_s
{
	serials_cmd_t 	cmd;
	uint8_t		arg;
	uint8_t		*data;
	uint32_t	data_count;
} serials_pkt_t;

typedef struct serials_init_s
{
	uint8_t *rxbuf;
	uint32_t rxbuf_len;
	void (*fn_send_byte)(uint8_t data);
	void (*cb_rxpkt_ready)(void);
} serials_init_t;

typedef struct serials_s
{
	serials_pkt_t rxpkt;
	uint8_t *rxbuf;
	uint32_t rxbuf_len;
	uint32_t rx_cnt;
	bool bs_esc;
	bool overflow;
	bool rxpkt_ready;
	bool rxing;
	void (*fn_send_byte)(uint8_t data);
	void (*cb_rxpkt_ready)(void);
} serials_t;


void serials_init(serials_t *instance, serials_init_t *init);
void serials_reset(serials_t *instance);
void serials_parse_bytes(serials_t *instance, uint8_t *data, uint32_t count);
bool serials_overflow(serials_t *instance);
bool serials_rxpkt_ready(serials_t *instance);
void serials_rxpkt_ready_clear(serials_t *instance);
serials_pkt_t* serials_rxpkt(serials_t *instance);

void serials_send_ack(serials_t *instance, uint8_t cmd);
void serials_send_err(serials_t *instance, uint8_t err_code);
void serials_send_bytes(serials_t *instance, uint8_t *data, uint32_t count);
void serials_send_string(serials_t *instance, char *str);

#ifdef __cplusplus
}
#endif

#endif // SERIALS_H
