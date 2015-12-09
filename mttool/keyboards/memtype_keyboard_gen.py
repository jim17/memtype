from array import array
import platform
import usb.core
import usb.control
import usb.util
import time
from keyboard import *

USB_VID					= 0x1209
USB_PID					= 0xA033

CMD_USB_KEYBOARD_PRESS  = 0x69

def keyboardSend(dev, data1, data2):
	packetToSend = array('B')
	packetToSend.append(CMD_USB_KEYBOARD_PRESS);
	packetToSend.append(data1)
	packetToSend.append(data2)
	packetToSend.append(0)
	packetToSend.append(0)
	packetToSend.append(0)
	packetToSend.append(0)
	packetToSend.append(0)

	usbhidSetReport(dev, packetToSend, 2)

	msg = usbhidGetReport(dev, 2, 8)
	time.sleep(0.05)


def keyboardTestKey(device, KEY, MODIFIER):
	if( KEY in KEYTEST_BAN_LIST ): return ''
	keyboardSend(device, KEY, MODIFIER)
	keyboardSend(device, 0, 0)
	keyboardSend(device, KEY, MODIFIER)
	keyboardSend(device, 0, 0)
	keyboardSend(device, KEY_RETURN, 0)
	keyboardSend(device, 0, 0)
	string = raw_input()
	if (string == ''):
		return string
	return string[0]

def keyboardKeyMap(device, key):
	if ( (key & 0x3F) == KEY_EUROPE_2 ):
		if (key & SHIFT_MASK):
			return keyboardTestKey(device, KEY_EUROPE_2_REAL, KEY_SHIFT)
		elif (key & ALTGR_MASK):
			return keyboardTestKey(device, KEY_EUROPE_2_REAL, KEY_RIGHT_ALT)
		else:
			return keyboardTestKey(device, KEY_EUROPE_2_REAL, 0)

	elif (key & SHIFT_MASK):
		return keyboardTestKey(device, key & ~SHIFT_MASK, KEY_SHIFT)

	if (key & ALTGR_MASK):
		return keyboardTestKey(device, key & ~ALTGR_MASK, KEY_RIGHT_ALT)

	else:
		return keyboardTestKey(device, key, 0)

def keyboardTest(device):
	fileName = raw_input("Name of the Keyboard (example: ES): ");

	# dictionary to store the 
	Layout_dict = dict()
	
	# No modifier combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(device, bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
		else:
			Layout_dict.update({output: bruteforce})
	
	# SHIFT combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(device, SHIFT_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
		else:
			Layout_dict.update({output : SHIFT_MASK|bruteforce})

	# ALTGR combinations
	for bruteforce in range(KEY_EUROPE_2, KEY_SLASH+1):
		output = keyboardKeyMap(device, ALTGR_MASK|bruteforce)
		if (output == ''): continue
		if output in Layout_dict:
			print "Already stored"
		else:
			Layout_dict.update({output : ALTGR_MASK|bruteforce})

	# Create Memtype .txt format
	# Initialize from 0 - 0x20 (0 - ' '), ENTER, DELETE, TAB keycodes are fixed
	hid_define_str = "0,0,0,0,0,0,0,0,42,43,40,0,0,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"

	for key in KeyboardAscii:
		keycode = str(Layout_dict[key])+","
		hid_define_str = hid_define_str + keycode

	# Save array into .txt file
	text_file = open("keyboard_"+fileName+".txt", "w")
	text_file.write(hid_define_str+'0') # end with 0 to have 128 bytes
	text_file.close()

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

def readInfo(dev):
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

	if( usbhidSetReport(dev, info_packet, reportId) != 8):
		print "Error Set Report"
		return None
	
	msg = usbhidGetReport(dev, 2, 8)
	print "SW Version %d.%d.%d" % (msg[1],msg[2],msg[3])
	print "Cred Size: %d bytes" % (msg[5] << 8 + msg[4])

	return 0


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
	# Read Info
	readInfo(hid_device)

	# Return device
	return hid_device

if __name__ == '__main__':
	# Main function
	print ""
	print "Memtype USB client" 
	
	# Search for the mooltipass and read hid data
	hid_device = findHIDDevice(USB_VID, USB_PID, True)
	if hid_device is None:
		print "some Error appeared"
		sys.exit(0)
	else:
		keyboardTest(hid_device)

	usb.util.dispose_resources(hid_device)
