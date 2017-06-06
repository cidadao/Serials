#include "serials.h"
#include <string.h>

void _serials_pkt_init(serials_pkt_t *pkt);
void _serials_pkt_send(serials_t *instance, serials_pkt_t *pkt);
void _serials_rxpkt_build(serials_t *instance);
void _serials_send_byte(serials_t *instance, uint8_t data);
void _serials_send_data(serials_t *instance, uint8_t *data, uint32_t count);

#if SERIALS_ARG_TO_ASCII
static void _ui8tohex(uint8_t n, char *buf)
{
//    char const digit[] = "0123456789";
//    uint8_t ch_cnt = 0;
//    char *b = buf;
//    do
//    {
//        *b = digit[i%10];
//        i = i/10;
//        b++;
//        ch_cnt++;
//    }while(i > 0);
//    *b = '\0';
//    return ch_cnt;
	buf[1] = (n & 0xF) + '0';
	buf[0] = ((n >> 4) & 0xF) + '0';
}
#endif

void serials_init(serials_t *instance, serials_init_t *init)
{
    instance->rxbuf = init->rxbuf;
    instance->rxbuf_len = init->rxbuf_len;

    instance->fn_send_byte = init->fn_send_byte;
    instance->cb_rxpkt_ready = init->cb_rxpkt_ready;

    serials_reset(instance);
}

void serials_reset(serials_t *instance)
{
    instance->rxpkt_ready = false;
    instance->rx_cnt = 0;
    instance->bs_esc = false;
    instance->overflow = false;
    instance->rxing = false;

    _serials_pkt_init(&(instance->rxpkt));
}

void serials_parse_bytes(serials_t *instance, uint8_t *data, uint32_t count)
{
    uint32_t i;
    for(i = 0; i < count; i++)
    {
        uint8_t b = data[i];
#if SERIALS_ECHO_RX
        instance->fn_send_byte(b);
#endif

        if(b == SERIALS_BS_ESC && !instance->bs_esc)
        {
            instance->bs_esc = true;
            continue;
        }

        if(b == SERIALS_BS_BEGIN && !instance->bs_esc)
        {
            _serials_pkt_init(&(instance->rxpkt));
            instance->rx_cnt = 0;
            instance->rxpkt_ready = false;
            instance->rxing = true;
            continue;
        }

        if(b == SERIALS_BS_END && !instance->bs_esc)
        {
            _serials_rxpkt_build(instance);
            instance->rxing = false;
            instance->rxpkt_ready = true;
            if(instance->cb_rxpkt_ready != 0)
            {
                instance->cb_rxpkt_ready();
            }
            continue;
        }

        instance->bs_esc = false;

        if(instance->rxing == false)
        {
        	continue;
        }

        if(instance->rx_cnt < (instance->rxbuf_len - 1))
        {
            instance->rxbuf[instance->rx_cnt] = b;
            instance->rx_cnt += 1;
        }
        else
        {
            instance->overflow = true;
        }
    }
}

bool serials_overflow(serials_t *instance)
{
    return instance->overflow;
}

bool serials_rxpkt_ready(serials_t *instance)
{
    return instance->rxpkt_ready;
}

void serials_rxpkt_ready_clear(serials_t *instance)
{
    instance->rxpkt_ready = false;
}

serials_pkt_t* serials_rxpkt(serials_t *instance)
{
    if(instance->rxpkt_ready)
        return &(instance->rxpkt);
    else
        return 0;
}

void _serials_send_byte(serials_t *instance, uint8_t data)
{
	if(instance->fn_send_byte != 0)
		instance->fn_send_byte(data);
}
void _serials_send_data(serials_t *instance, uint8_t *data, uint32_t count)
{
    uint32_t i;
    for(i = 0; i < count; i++)
    {
        if(data[i] == SERIALS_BS_BEGIN ||
           data[i] == SERIALS_BS_END ||
           data[i] == SERIALS_BS_ESC)
        {
            _serials_send_byte(instance, SERIALS_BS_ESC);
        }
        _serials_send_byte(instance, data[i]);
    }
}

void _serials_pkt_send(serials_t *instance, serials_pkt_t *pkt)
{
    _serials_send_byte(instance, SERIALS_BS_BEGIN);
    _serials_send_data(instance, (uint8_t*)&(pkt->cmd), 1);
#if SERIALS_ARG_TO_ASCII
    char arg_hex[2];
    if(pkt->arg != SERIALS_ARG_NODATA && pkt->data_count == 0)
    {
        _ui8tohex(pkt->arg, arg_hex);
        pkt->arg = SERIALS_ARG_DATA;
        pkt->data = (uint8_t*) arg_hex;
        pkt->data_count = 2;
    }
#endif
    _serials_send_data(instance, &(pkt->arg), 1);

    if(pkt->data != 0 && pkt->data_count > 0)
        _serials_send_data(instance, pkt->data, pkt->data_count);
    _serials_send_byte(instance, SERIALS_BS_END);
#if SERIALS_APPEND_CRLF
    _serials_send_byte(instance, (uint8_t) '\r');
    _serials_send_byte(instance, (uint8_t) '\n');
#endif
}

void _serials_pkt_init(serials_pkt_t *pkt)
{
	pkt->cmd = SERIALS_CMD_NULL;
    pkt->arg = SERIALS_ARG_NODATA;
    pkt->data = 0;
    pkt->data_count = 0;
}

void _serials_rxpkt_build(serials_t *instance)
{
    uint8_t *rxdata = instance->rxbuf;
    uint32_t rxdata_count = instance->rx_cnt;
    serials_pkt_t *pkt = &(instance->rxpkt);
    if(rxdata_count >= 2)
    {
		pkt->cmd = (serials_cmd_t) rxdata[0];
		pkt->arg = rxdata[1];
		pkt->data_count = rxdata_count - 2;
		if(pkt->data_count > 0)
		{
			pkt->data = rxdata + 2;
			if(pkt->cmd == SERIALS_CMD_STRING)
			{
				pkt->data[pkt->data_count] = '\0';
			}
		}
    }
}

void serials_send_ack(serials_t *instance, uint8_t cmd)
{
    serials_pkt_t pkt;
    _serials_pkt_init(&pkt);
    pkt.cmd = SERIALS_CMD_ACK;
    pkt.arg = cmd;
    _serials_pkt_send(instance, &pkt);
}

void serials_send_err(serials_t *instance, uint8_t err_code)
{
    serials_pkt_t pkt;
    _serials_pkt_init(&pkt);
    pkt.cmd = SERIALS_CMD_ERR;
    pkt.arg = err_code;
    _serials_pkt_send(instance, &pkt);
}

void serials_send_bytes(serials_t *instance, uint8_t *data, uint32_t count)
{
    serials_pkt_t pkt;
    _serials_pkt_init(&pkt);
    pkt.cmd = SERIALS_CMD_BYTES;
    pkt.arg = SERIALS_ARG_DATA;
    pkt.data = data;
    pkt.data_count = count;
    _serials_pkt_send(instance, &pkt);
}

void serials_send_string(serials_t *instance, char *str)
{
	serials_pkt_t pkt;
	_serials_pkt_init(&pkt);
	pkt.cmd = SERIALS_CMD_STRING;
	pkt.arg = SERIALS_ARG_DATA;
	pkt.data = (uint8_t*) str;
	pkt.data_count = strlen(str);
	_serials_pkt_send(instance, &pkt);
}

