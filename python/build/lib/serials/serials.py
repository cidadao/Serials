


class Packet(object):
	
	def __init__(self):
		self.cmd = Serials.CMD_NULL
		self.arg = Serials.ARG_NODATA
		self.data = ""

	def reset(self):
		self.cmd = Serials.CMD_NULL
		self.arg = Serials.ARG_NODATA
		self.data = ""

	def build(self, data):
		self.cmd = data[0]
		self.arg = data[1]
		if len(data) > 2:
			self.data = data[2:]
	

class Serials(object):

	BS_BEGIN	= 0x3C 	# '<'
	BS_END		= 0x3E	# '>'
	BS_ESC 		= 0x2A	# '*'

	ARG_DATA	= 0x2C	# ','
	ARG_NODATA	= 0x00	# null

	CMD_NULL	= 0x00
	CMD_ACK		= 0x6B	# 'k'
	CMD_ERR		= 0x65	# 'e'
	CMD_BYTES	= 0x62	# 'b'
	CMD_STRING	= 0x73	# 's'

	def __init__(self):
		self._callbacks = {
			"tx": None,
			"rx": None,
			"rxpkt_ready": None,
		}
		self.rxpkt_ready = False
		self.bs_esc = False
		self.overflow = False
		self.rxing = False
		self.rxbuf = ""
		self.rxpkt = Packet()

	def reset(self):
		self.rxpkt_ready = False
		self.bs_esc = False
		self.rxing = False
		self.rxbuf = ""

	def rxpkt(self):
		return self.rxpkt

	def set_callback(self, name, cb):
		self._callbacks[name] = cb


	def _run_callback(self, name, *args):
		if self._callbacks[name] is None:
			raise NotImplementedError("Callback \"%s\" is not implemented" % name)
		else:
			if len(args) > 0:
				return self._callbacks[name](args)
			else:
				return self._callbacks[name]()
	

	def _tx(self, data):
		return self._run_callback("tx", data)

	def _rx(self):
		return self._run_callback("rx")

	def _parse_bytes(self, data):
		for b in data:

			if b == Serials.BS_ESC and (self.bs_esc is False):
				self.bs_esc = True
				continue
			if b == Serials.BS_BEGIN and (self.bs_esc is False):
				self.rxbuf = ""
				self.rxpkt_ready = False
				self.rxing = True
				continue
			if b == Serials.BS_END and (self.bs_esc is False):
				self.rxpkt.build(self.rxbuf)
				self.rxing = False
				self.rxpkt_ready = True
				if self._callbacks["rxpkt_ready"] != None:
					self._callbacks["rxpkt_ready"]()
				continue
			
			self.bs_esc = False
			if not self.rxing:
				continue

			self.rxbuf.append(b)

	def _send_byte(self, b):
		self._tx(b)

	def _send_data(self, b):
		if b == Serials.BS_BEGIN or b == Serials.BS_END or b == Serials.BS_ESC:
			self._send_byte(Serials.BS_ESC)
		self._send_byte(b)

	def _send_pkt(self, pkt):
		self._send_byte(Serials.BS_BEGIN)
		self._send_data(pkt.cmd)
		self._send_data(pkt.arg)
		if len(pkt.data) > 0:
			for b in pkt.data: self._send_data(b)
		self._send_byte(Serials.BS_END)
		
	def send_ack(self, cmd):
		pkt = Packet()
		pkt.cmd = Serials.CMD_ACK
		pkt.arg = cmd
		self._send_pkt(pkt)

	def send_err(self, err_code):
		pkt = Packet()
		pkt.cmd = Serials.CMD_ERR
		pkt.arg = err_code
		self._send_pkt(pkt)

	def send_bytes(self, data):
		pkt = Packet()
		pkt.cmd = Serials.CMD_BYTES
		pkt.arg = Serials.ARG_DATA
		pkt.data = data
		self._send_pkt(pkt)

	def send_string(self, data):
		pkt = Packet()
		pkt.cmd = Serials.CMD_STRING
		pkt.arg = Serials.ARG_DATA
		pkt.data = data
		self._send_pkt(pkt)

	def run(self):
		data = self._rx()
		if len(data) > 0:
			self._parse_bytes(data)

