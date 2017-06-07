/*
MIT License

Copyright (c) Mario Ribeiro <mario@rasgo.xyz>
URL: https://github.com/rasgo-xyz/Serials

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "SerialsWrapper.h"

SerialsWrapper* SerialsWrapper::_currentInstance = 0;

void SerialsWrapper::_setInstance(SerialsWrapper *instance)
{
    SerialsWrapper::_currentInstance = instance;
}

void SerialsWrapper::_sendByte(uint8_t data)
{
    SerialsWrapper *instance = SerialsWrapper::_currentInstance;
    if(instance == 0) return;

    instance->_fn_sendByte(data);
}

void SerialsWrapper::_rxPktReady(void)
{
    SerialsWrapper *instance = SerialsWrapper::_currentInstance;
    if(instance == 0) return;

    instance->_cb_rxPktReady();
}

SerialsWrapper::SerialsWrapper()
{
    serials_init_t init;
    init.fn_send_byte = &SerialsWrapper::_sendByte;
    init.cb_rxpkt_ready = &SerialsWrapper::_rxPktReady;
    serials_init(&_serials, &init);
}

SerialsWrapper::~SerialsWrapper()
{

}

void SerialsWrapper::setBuffer(uint8_t *buf, uint32_t size)
{
    _serials.rxbuf = buf;
    _serials.rxbuf_len = size;
}
void SerialsWrapper::setFuncSendByte(Func_SendByte_t fn)
{
    _fn_sendByte = fn;
}
void SerialsWrapper::setCallbackRxPktReady(Callback_RxPktReady_t cb)
{
    _cb_rxPktReady = cb;
}

void SerialsWrapper::reset()
{
    serials_reset(&_serials);
}

void SerialsWrapper::parseBytes(uint8_t *data, uint32_t count)
{
    SerialsWrapper::_setInstance(this);
    serials_parse_bytes(&_serials, data, count);
}

serials_pkt_t* SerialsWrapper::rxPkt()
{
    return serials_rxpkt(&_serials);
}

void SerialsWrapper::sendBytes(uint8_t *data, uint32_t count)
{
    SerialsWrapper::_setInstance(this);
    serials_send_bytes(&_serials, data, count);
}

void SerialsWrapper::sendString(char *str)
{
    SerialsWrapper::_setInstance(this);
    serials_send_string(&_serials, str);
}

void SerialsWrapper::sendError(uint8_t errCode)
{
    SerialsWrapper::_setInstance(this);
    serials_send_err(&_serials, errCode);
}

void SerialsWrapper::sendAck(uint8_t cmd)
{
    SerialsWrapper::_setInstance(this);
    serials_send_ack(&_serials, cmd);
}