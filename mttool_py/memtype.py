from array import array
import platform
import usb.core
import usb.control
import usb.util
import time
import noekeon

def encryptCredentialList( cl=[] ):
	tmp = array('B')
	for c in cl:
		encryptedBlock = c.encrypt()
		tmp.append(c.name)
		tmp.append(len(encryptedBlock))
		tmp.append(encryptedBlock)

	return tmp

def decryptCredentialList():
	return


def findHIDDevice(vendor_id, product_id, print_debug):	
	# Find our device
	hid_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# Was it found?
	if hid_device is None:
		if print_debug:
			print "Device not found"
		return None

	# Device found
	if print_debug:
		print "MemType found"

	if platform.system() == "Linux":
		# Need to do things differently
		try:
			#hid_device.detach_kernel_driver(0)
			hid_device.reset()
		except Exception, e:
			pass # Probably already detached
	else:
		# Set the active configuration. With no arguments, the first configuration will be the active one
		try:
			hid_device.set_configuration()
		except Exception, e:
			if print_debug:
				print "Cannot set configuration the device:" , str(e)
			return None

	# Return device
	return hid_device

def usbhidSetReport(device, buff, reportId):
							 # bmRequestType, bmRequest, wValue            wIndex
	if device.ctrl_transfer(0x20,          0x09,      0x0300 | reportId,   0,  buff, 5000) != len(buff):
		print "Error usbhidSetReport"
		return -1
	return len(buff)

def usbhidGetReport(device, reportId, l):
							 # bmRequestType, bmRequest, wValue            wIndex
	buff = device.ctrl_transfer(0xA0,          0x01,  0x0300 | reportId,   0,  l , 5000);
	if buff == None:
		print "Error usbhidGetReport"
		return -1
	return buff

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
	def __init__(self, vid=0x1209, pid=0xA033, printDebug=True):
		# Initialize with VID and PID
		self.vid = vid
		self.pid = pid
		self.printDebug = printDebug
		self.connect()

	def connect(self):
		# connect 
		self.dev = findHIDDevice(self.vid, self.pid, True)
		if self.dev is None:
			if self.printDebug: print "some Error appeared"
			return -1
		else:
			return 0
	
	def disconnect(self):
		usb.util.dispose_resources(self.dev)

	def info(self):
		reportId = 2
		cmd = 5
		info_packet = array('B')
		info_packet.append(cmd)
		info_packet.append(0)
		info_packet.append(0)
		info_packet.append(0)
		info_packet.append(0)
		info_packet.append(0)
		info_packet.append(0)
		info_packet.append(0)
		
		if self.printDebug: print info_packet

		if( usbhidSetReport(self.dev, info_packet, reportId) != 8):
			if self.printDebug: print "Error Set Report"
			return None
	
		msg = usbhidGetReport(self.dev, 2, 8)
		version = [msg[1],msg[2],msg[3]]
		credSize = msg[5] << 8 + msg[4]
		info = deviceInfo(version, credSize)
		if self.printDebug: print info

		return info

if __name__ == '__main__':
	# Search for the memtype and read hid data
	memtypeObj = memtype()
	print memtypeObj.info()
	memtypeObj.disconnect()
	
