#include <stdio.h>
#include <stdint.h>
#include "serials.h"

//----------------------------------------------------------------------------
// Circular buffers, to be used in a producer-consumer fashion, simulating a
// communication channel

typedef struct
{
    uint8_t *buf;
    uint32_t buf_len;
    uint32_t rd_i;
    uint32_t wr_i;
    uint32_t count;
} cbuf_t;

void cbuf_init(cbuf_t *cbuf, uint8_t *buf, uint32_t len)
{
    cbuf->buf = buf;
    cbuf->buf_len = len;
    cbuf->rd_i = cbuf->wr_i = 0;
}
uint32_t cbuf_available(cbuf_t *cbuf)
{
    return cbuf->count;
}
uint32_t cbuf_write(cbuf_t *cbuf, uint8_t *data, uint32_t count)
{
    uint32_t to_write, max;
    to_write = max = cbuf->count + count > cbuf->buf_len ? cbuf->buf_len - cbuf->count : count;
    while(to_write--)
    {
        cbuf->buf[cbuf->wr_i] = *data++;
        cbuf->wr_i += 1;
        cbuf->count += 1;
    }
    return max;
}

uint32_t cbuf_read(cbuf_t *cbuf, uint8_t *dest, uint32_t max)
{
    uint32_t available = cbuf_available(cbuf);
    uint32_t to_read = max = max > available ? available : max;

    while(to_read--)
    {
        *dest = cbuf->buf[cbuf->rd_i];
        cbuf->rd_i = cbuf->rd_i < cbuf->buf_len - 1? cbuf->rd_i + 1 : 0;
        dest++;
    }

    return max;
}
//----------------------------------------------------------------------------
// Setup the communication channels (circular buffers)
#define BUF_LEN 64
uint8_t m_buf[BUF_LEN], s_buf[BUF_LEN];
cbuf_t m_cbuf, s_cbuf;

void setup_buffers()
{
    cbuf_init(&m_cbuf, m_buf, BUF_LEN);
    cbuf_init(&s_cbuf, s_buf, BUF_LEN);
}

uint32_t m_read(uint8_t *buf, uint32_t max)
{
    return cbuf_read(&m_cbuf, buf, max);
}

uint32_t s_read(uint8_t *buf, uint32_t max)
{
    return cbuf_read(&s_cbuf, buf, max);
}

void m_send_byte(uint8_t b)
{
    cbuf_write(&s_cbuf, &b, 1);
}

void s_send_byte(uint8_t b)
{
    cbuf_write(&m_cbuf, &b, 1);
}

//----------------------------------------------------------------------------
void dump_data(uint8_t *data, uint32_t count)
{
    uint32_t i;
    printf("Dump data (count: %d)\n", count);
    for(i = 0; i < count; )
    {
        printf("%02X ", data[i]);
        i++;
        if((i % 8) == 0)
            printf("\n");
    }
}
//----------------------------------------------------------------------------

// Buffers used to store data received from the communication channels
#define DUMMY_LEN 8
uint8_t m_rx_buf[BUF_LEN], s_rx_buf[BUF_LEN];

int main()
{
    printf("Setup buffers\n");
    setup_buffers();

    printf("Init LightSerial instances\n");
    serials_t master, slave;

    serials_init_t init;
    init.rx_buf_len = BUF_LEN;
    init.cb_rxpkt_ready = 0;

    init.rx_buf = m_rx_buf;
    init.fn_send_byte = m_send_byte;
    serials_init(&master, &init);

    init.rx_buf = s_rx_buf;
    init.fn_send_byte = s_send_byte;
    serials_init(&slave, &init);

    uint32_t i, count;
    uint8_t dummy[DUMMY_LEN];
    uint8_t rx_buf[BUF_LEN];
    for(i=0; i < DUMMY_LEN; i++) dummy[i] = i;

    printf("Master sends bytes to slave\n");
    dump_data(dummy, DUMMY_LEN);
    serials_send_bytes(&master, dummy, DUMMY_LEN);

    printf("Slave read bytes\n");
    count = s_read(rx_buf, BUF_LEN);
    printf("Slave parse bytes\n");
    serials_parse_bytes(&slave, rx_buf, count);
    if(serials_rxpkt_ready(&slave))
    {
        printf("Slave received a packet!\n");
        serials_pkt_t *pkt = serials_rxpkt(&slave);
        printf("CMD:%02X ARG:%02X\n", pkt->cmd, pkt->arg);
        dump_data(pkt->data, pkt->data_count);

        printf("Slave sends ERR\n");
        serials_send_err(&slave, 0x12);
    }

    printf("Master read bytes\n");
    count = m_read(rx_buf, BUF_LEN);
    printf("Master parse bytes\n");
    serials_parse_bytes(&master, rx_buf, count);
    if(serials_rxpkt_ready(&master))
    {
        printf("Master received a packet!\n");
        serials_pkt_t *pkt = serials_rxpkt(&master);
        printf("CMD:%02X ARG:%02X\n", pkt->cmd, pkt->arg);
        dump_data(pkt->data, pkt->data_count);
    }

    puts("Done");
    return 0;
}
