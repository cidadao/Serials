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

#ifndef SERIALSWRAPPER_H
#define SERIALSWRAPPER_H

#include "serials.h"
#include <functional>


class SerialsWrapper
{
public:
    typedef std::function<void(uint8_t)>    Func_SendByte_t;
    typedef std::function<void(void)>       Callback_RxPktReady_t;

    SerialsWrapper();
    ~SerialsWrapper();

    void setBuffer(uint8_t *buf, uint32_t size);
    void setFuncSendByte(Func_SendByte_t cb);
    void setCallbackRxPktReady(Callback_RxPktReady_t cb);

    void reset();
    void parseBytes(uint8_t *data, uint32_t count);
    serials_pkt_t* rxPkt();

    void sendBytes(uint8_t *data, uint32_t count);
    void sendString(char *str);
    void sendError(uint8_t errCode);
    void sendAck(uint8_t cmd);

protected:
    static void _setInstance(SerialsWrapper *instance);
    static void _sendByte(uint8_t data);
    static void _rxPktReady(void); 
    static SerialsWrapper *_currentInstance;

    Func_SendByte_t         _fn_sendByte;
    Callback_RxPktReady_t   _cb_rxPktReady;

    serials_t _serials;
};

#endif // SERIALSWRAPPER_H
