from array import array
import platform
import usb.core
import usb.control
import usb.util
import time
from noekeon import *

SET_REPORT_TIME_WAIT = 0.05
GET_REPORT_TIME_WAIT = 0.05


def lo(num):
	return num & 0xFF


def hi(num):
	return (num & 0xFF00) >> 8

def to32BE(num1,num2,num3,num4):
	return (num1 << 24) + (num2 << 16) + (num3 << 8) + num4

def to32LE(num1=0,num2=0,num3=0,num4=0):
	return (num4 << 24) + (num3 << 16) + (num2 << 8) + num1

def group8to32(b=[]):
	tmp = []

	for i in range(len(b)):
		if(i%4 == 0):
			tmp.append(b[i])
		else:
			tmp[i/4] = tmp[i/4] + (b[i] << (8*(i%4)))

	return tmp

def ungroup32to8(b=[]):
	tmp = []
	for i in range(len(b)):
		tmp.append((b[i] & 0x000000FF) >> 0 )
		tmp.append((b[i] & 0x0000FF00) >> 8 )
		tmp.append((b[i] & 0x00FF0000) >> 16)
		tmp.append((b[i] & 0xFF000000) >> 24)


	return tmp


def encryptCredentialList(cl=[], key=[0,0,0,0]):
	tmp = []
	for c in cl:
		encryptedBlock = c.encrypt()
		tmp.append(c.name)
		tmp.append(len(encryptedBlock))
		tmp.append(encryptedBlock)

	return tmp

"""
Input: Credential Block in the form
	[CredName,Offset,[Encrypted: user,hop,pass,submit]] * Number of credentials
"""
def decryptCredentialList(cb=[], key=[0,0,0,0]):
	cred = credential()
	credList = []
	blockStart = 0
	blockEnd = 0
	step = 1
	for i in range(len(cb)):
		c = cb[i]
		# Step 1 find credential Name
		if(c == 0 and step == 1):
			step = 2
		elif(step == 1):
			cred.name = cred.name + chr(c)
		# Step 2 save encrypted Block
		elif(step == 2):
			blockStart = i+2
			blockEnd = cb[i] + ((cb[i+1] << 8) & 0xFF00)

			# Last credential has blockEnd set as 0
			if(blockEnd == 0):
				blockEnd = len(cb)

			cred.decrypt(key, cb[blockStart:blockEnd])
			credList.append(cred)
			step = 3

		# Step 3 wait until 'i' reaches blockEnd
		elif(step == 3 and i == (blockEnd-1)):
			step = 1
			cred = credential()
			blockStart = 0
			blockEnd = 0

	# Print Credential List, only for debug
	for cr in credList:
		print cr

	return credList


def findHIDDevice(vendor_id, product_id, print_debug):
	# Find our device
	hid_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# Was it found?
	if hid_device is None:
		if print_debug: print "Device not found"
		return None

	# Device found
	if print_debug: print "MemType found"

	if platform.system() == "Linux":
		# Need to do things differently
		try:
			hid_device.detach_kernel_driver(0)
			hid_device.reset()
		except Exception, e:
			pass # Probably already detached
	else:
		# Set the active configuration. With no arguments, the first configuration will be the active one
		try:
			hid_device.set_configuration()
		except Exception, e:
			if print_debug: print "Cannot set configuration the device:" , str(e)
			return None

	# Return device
	return hid_device

def usbhidSetReport(device, buff, reportId):
							 # bmRequestType, bmRequest, wValue            wIndex
	if device.ctrl_transfer(0x20,          0x09,      0x0300 | reportId,   0,  buff, 5000) != len(buff):
		print "Error usbhidSetReport"
		return -1
	time.sleep(SET_REPORT_TIME_WAIT)
	return len(buff)

def usbhidGetReport(device, reportId, l):
							 # bmRequestType, bmRequest, wValue            wIndex
	buff = device.ctrl_transfer(0xA0,          0x01,  0x0300 | reportId,   0,  l , 5000);
	if buff == None:
		print "Error usbhidGetReport"
		return -1
	time.sleep(GET_REPORT_TIME_WAIT)
	return buff.tolist()

class credential:
	def __init__(self, name="", user="", hop="", passw="", submit=""):
		# Initialize with VID and PID
		self.name = name
		self.user = user
		self.hop  = hop
		self.passw = passw
		self.submit = submit

	def __str__(self):
		# connect
		return "%s - %s - %s - %s - %s" % (self.name, self.user, self.hop, self.passw, self.submit)

	def encrypt(self, key):
		# Noekeon
		return

	def decrypt(self, key, encryptedBlock):
		tmp = []
		eb = encryptedBlock
		for i in range(0, len(eb), 16):
			if(i+16 <= len(eb)):
				tmp.extend( ungroup32to8( NoekeonDecrypt(key, group8to32(eb[i:i+16])) ) )

		# Read Credentials
		step = 1
		for i in range(len(tmp)):
			c = tmp[i]
			# STEP 1 -- Find User
			if(step == 1):
				if(c == 0):
					step = 2
				else:
					self.user = self.user + chr(c)
			# STEP 2 -- Find Hop
			if(step == 2):
				if(c == 0):
					step = 3
				else:
					self.hop = self.hop + chr(c)
			# STEP 3 -- Find Pass
			if(step == 3):
				if(c == 0):
					step = 4
				else:
					self.passw = self.passw + chr(c)
			# STEP 4 -- Find Submit
			if(step == 4):
				if(c == 0):
					step = 5
				else:
					self.submit = self.submit + chr(c)

		return
## Memtype COMM Types
class deviceInfo:
	def __init__(self, version=[0,0,0], credSize=0):
		self.version = version
		self.credSize = credSize
		self.major = self.version[0]
		self.minor = self.version[1]
		self.patch = self.version[2]

	def __str__(self):
		return "V:%03d.%03d.%03d CZ:%d" % (self.major, self.minor, self.patch, self.credSize)

class memtype:
	def __init__(self, vid=0x1209, pid=0xa033, printDebug=True):
		# Initialize with VID and PID
		self.vid = vid
		self.pid = pid
		self.reportId = 2
		self.printDebug = printDebug
		self.connect()

	def connect(self):
		# connect
		self.dev = findHIDDevice(self.vid, self.pid, True)
		if self.dev is None:
			if self.printDebug: print "ERR findHIDDevice"
			return -1
		else:
			return 0

	def disconnect(self):
		usb.util.dispose_resources(self.dev)

	def info(self):
		pkt = [5, 0, 0, 0, 0, 0, 0, 0]

		if self.printDebug: print pkt

		if( usbhidSetReport(self.dev, pkt, self.reportId) != 8):
			if self.printDebug: print "ERR Set Report"
			return None

		msg = usbhidGetReport(self.dev, self.reportId, 8)
		version = [msg[1],msg[2],msg[3]]
		credSize = msg[5] << 8 + msg[4]
		info = deviceInfo(version, credSize)
		if self.printDebug: print info

		return info

  	def read(self, credSize=512, offset=0):

		pkt = [2, lo(offset), hi(offset), lo(credSize), hi(credSize), 0,0,0]
		if( usbhidSetReport(self.dev, pkt, self.reportId) != 8):
			if self.printDebug: print "ERR Set Report"
			return None

		answer = usbhidGetReport(self.dev, self.reportId, 8)
		if(pkt != answer):
			if self.printDebug:
				print "Packet: ", pkt
				print "Answer: ", answer
				print "ERR pkt != answer"

		block = []
		for msg in range(0,credSize,8):
			answer = usbhidGetReport(self.dev, self.reportId, 8)
			block.extend(answer)

		return block

if __name__ == '__main__':
	# Search for the memtype and read hid data
#	memtypeObj = memtype(0x16c0,0x05dc)
#	print memtypeObj.info()
#	print memtypeObj.read()
#	memtypeObj.disconnect()
	block = [ 0x57,0x65,0x6c,0x63,0x6f,0x6d,0x65,0x00,0x3a,0x00,0x79,0x62,0x6f,0xc8,0x0f,0xeb,0x16,0x21,0x95,0xdd,0x0f,0x42,0xe5,0x33,0x9f,0xb7,0x8d,0x43,0xb4,0xaa,0xb7,0xac,0xf0,0xe3,0xc4,0x71,0xcd,0xe1,0xa2,0x4f,0x15,0x33,0x65,0x41,0x1d,0xee,0xac,0xf3,0x5c,0x00,0xd1,0x8f,0x62,0x28,0x79,0x61,0xf9,0xba,0x41,0x62,0x6f,0x75,0x74,0x00,0x72,0x00,0x16,0xdc,0x40,0xd1,0xe4,0x36,0x4d,0xa7,0x8a,0x3d,0x5c,0x19,0x94,0xe1,0x88,0xdf,0xbf,0x68,0x01,0xac,0x46,0x8c,0x4b,0x9d,0x01,0x55,0x24,0x07,0x85,0x80,0xa0,0x62,0x39,0xab,0x03,0xc8,0x84,0x64,0xfa,0xf1,0xa3,0xb7,0x74,0x7b,0xa3,0xf3,0x05,0xcb,0x52,0x6f,0x75,0x74,0x65,0x72,0x20,0x44,0x65,0x66,0x61,0x75,0x6c,0x74,0x00,0x00,0x00,0x08,0xa9,0x43,0x69,0xd2,0x61,0x54,0xe0,0x6c,0x18,0xd5,0x72,0x4b,0x0d,0xe6,0x62,0xeb,0x74,0xbb,0x43,0x3c,0x16,0x5a,0x04,0xd0,0x87,0xd0,0x35,0x36,0x66,0xca,0x72,0x00,0x00,0x00,0x00,0x00 ]
	decryptCredentialList(block, key=[0x30303030,0x30303030,0x30303030,0x30303030])

