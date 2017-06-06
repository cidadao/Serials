import binascii
from serials import Serials, Packet

s1 = Serials()
s2 = Serials()

s1_buf = bytearray()
s2_buf = bytearray()

#--------------------------------------------------------------------
def s1_tx(data):
	global s2_buf
	print("S1 TX: %02X" % data)
	s2_buf += bytearray(data)

def s1_rx():
	global s1_buf
	if len(s1_buf) > 0:
		print("S1 RX: %s" % s1_buf)
	rxdata = s1_buf; s1_buf = bytearray()
	return rxdata

def s1_rxpkt_ready():
	pkt = s1.rxpkt
	text = pkt.data
	print("S1 RXPKT: %02X %02X %s" % (pkt.cmd, pkt.arg, text))

def s2_tx(data):
	global s1_buf
	print("S2 TX: %02X" % data)
	s1_buf += bytearray(data)

def s2_rx():
	global s2_buf
	if len(s2_buf) > 0:
		print("S2 RX: %s" % s2_buf)
	rxdata = s2_buf; s2_buf = bytearray()
	return rxdata

def s2_rxpkt_ready():
	pkt = s2.rxpkt
	text = pkt.data
	print("S2 RXPKT: %02X %02X %s" % (pkt.cmd, pkt.arg, text))
	#s2.send_string("okidoki")
	s2.send_bytes([0x41, 0x41])


#--------------------------------------------------------------------

def main():
	print("Serials - Example 1")

	s1_callbacks = {
		"tx": s1_tx,
		"rx": s1_rx,
		"rxpkt_ready": s1_rxpkt_ready,
	}
	s2_callbacks = {
		"tx": s2_tx,
		"rx": s2_rx,
		"rxpkt_ready": s2_rxpkt_ready,
	}

	s1.set_callbacks(s1_callbacks)
	s2.set_callbacks(s2_callbacks)

	s1.send_bytes([0x41, 0x42])
	s2.run(); s1.run()

	s1.send_string("hello!")
	s2.run(); s1.run()


if __name__ == '__main__':
	main()