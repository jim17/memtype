#!/usr/bin/python
import platform
import time
from array import array

import usb.control
import usb.core
import usb.util
from noekeon import *

SET_REPORT_TIME_WAIT = 0.01
GET_REPORT_TIME_WAIT = 0.01
DEFAULT_CREDENTIAL_SIZE = 2048


"""
Takes low byte of 16 bit var
"""
def lo(num):
    return num & 0xFF


"""
Takes high byte of 16 bit var
"""
def hi(num):
    return (num & 0xFF00) >> 8


"""
Input: 0xaa,0xbb,0xcc,0xdd
Outupt: 0xddccbbaa
"""
def group8to32(b=[]):
    tmp = []

    for i in range(len(b)):
        if (i % 4 == 0):
            tmp.append(b[i])
        else:
            tmp[i / 4] = tmp[i / 4] + (b[i] << (8 * (i % 4)))

    return tmp


"""
Input: 0xddccbbaa
Output: 0xaa,0xbb,0xcc,0xdd
"""
def ungroup32to8(b=[]):
    tmp = []
    for i in range(len(b)):
        tmp.append((b[i] & 0x000000FF) >> 0)
        tmp.append((b[i] & 0x0000FF00) >> 8)
        tmp.append((b[i] & 0x00FF0000) >> 16)
        tmp.append((b[i] & 0xFF000000) >> 24)

    return tmp


"""
Input: pin in string format "0000" to "9999" 4 digits
Output: memtype pin to key format
        "0000" => [ 0x30303030, 0x30303030, 0x30303030, 0x30303030 ]
"""
def pinToKey(pinStr=""):
    key = [0, 0, 0, 0]
    pin = 0000

    if (len(pinStr) != 4):
        print "ERR pin length"
    else:
        pin = int(pinStr)
        if (pin <= 9999) or (pin >= 0000):
            digit0 = ord(pinStr[3])
            digit1 = ord(pinStr[2])
            digit2 = ord(pinStr[1])
            digit3 = ord(pinStr[0])
            key = [(digit3 << 24) + (digit2 << 16) + (digit1 << 8) + digit0,
                   (digit3 << 24) + (digit2 << 16) + (digit1 << 8) + digit0,
                   (digit3 << 24) + (digit2 << 16) + (digit1 << 8) + digit0,
                   (digit3 << 24) + (digit2 << 16) + (digit1 << 8) + digit0]
        else:
            print "ERR pin value"

    return key


"""
Input: Credential List
Output: Credential Block in the form
	[CredName,Offset,[Encrypted: user,hop,pass,submit]] * Number of credentials
"""
def encryptCredentialList(cl=[], key=[0, 0, 0, 0]):
    tmp = []
    offset = 0
    offsetSize = 2
    eb = []
    for c in cl:
        eb = c.encrypt(key)
        cname = []
        cname.extend(c.name + '\0')
        # STEP 1 convert characters to int
        for i in range(0, len(cname)):
            cname[i] = ord(cname[i])

        # STEP 2 add credential into format:
        #  [CredName,Offset,[Encrypted: user,hop,pass,submit]]
        offset += len(cname) + offsetSize + len(eb)
        # print offset
        tmp.extend(cname)
        tmp.extend([lo(offset), hi(offset)])
        tmp.extend(eb)
    # Set Last credential offset to 0
    tmp[-(len(eb) + 2)] = 0
    tmp[-(len(eb) + 1)] = 0

    return tmp


"""
Input: Credential Block in the form
	[CredName,Offset,[Encrypted: user,hop,pass,submit]] * Number of credentials
Output: Credential List
"""
def decryptCredentialList(cb=[], key=[0, 0, 0, 0]):
    cred = credential()
    credList = []
    blockStart = 0
    blockEnd = 0
    step = 1
    for i in range(len(cb)):
        c = cb[i]
        # Step 1 find credential Name
        if (c == 0 and step == 1):
            step = 2
        elif (step == 1):
            cred.name = cred.name + chr(c)
        # Step 2 save encrypted Block
        elif (step == 2):
            blockStart = i + 2
            blockEnd = cb[i] + ((cb[i + 1] << 8) & 0xFF00)

            # Last credential has blockEnd set as 0
            if (blockEnd == 0):
                blockEnd = len(cb)

            cred.decrypt(key, cb[blockStart:blockEnd])
            credList.append(cred)
            step = 3

        # Step 3 wait until 'i' reaches blockEnd
        elif (step == 3 and i == (blockEnd - 1)):
            step = 1
            cred = credential()
            blockStart = 0
            blockEnd = 0

    # Print Credential List, only for debug
    for cr in credList:
        print cr

    return credList


"""
Find HID usb device based on VID and PID
"""
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
            # hid_device.detach_kernel_driver(0) # this line makes memtype crash in sending key codes
            hid_device.reset()
        except Exception, e:
            pass  # Probably already detached
    else:
        # Set the active configuration. With no arguments, the first configuration will be the active one
        try:
            hid_device.set_configuration()
        except Exception, e:
            if print_debug: print "Cannot set configuration the device:", str(e)
            return None

    # Return device
    return hid_device


"""
USB SetReport for HID USB device
"""
def usbhidSetReport(device, buff, reportId):
    # bmRequestType, bmRequest, wValue            wIndex
    if device.ctrl_transfer(0x20, 0x09, 0x0300 | reportId, 0, buff, 5000) != len(buff):
        print "Error usbhidSetReport"
        return []
    time.sleep(SET_REPORT_TIME_WAIT)
    return len(buff)


"""
USB GetReport for HID USB device
"""
def usbhidGetReport(device, reportId, l):
    # bmRequestType, bmRequest, wValue            wIndex
    buff = device.ctrl_transfer(0xA0, 0x01, 0x0300 | reportId, 0, l, 5000);
    if buff == None:
        print "Error usbhidGetReport"
        return []
    time.sleep(GET_REPORT_TIME_WAIT)
    return buff.tolist()


"""
USB Send message, always do a Set report and get back the answer
    the answer should be equal to Set report for a succesfull CMD
"""
def usbSendMsg(dev, msg, reportId, cmdLen=8):
    if (msg != 8):
        print "ERR msg length must be 8"
        return None

    pkt = array('B', msg)
    if (usbhidSetReport(dev, pkt, reportId) != 8):
        print "ERR Set Report"
        return None

    answer = usbhidGetReport(dev, reportId, 8)
    if (pkt[:cmdLen] != answer[:cmdLen]):
        print "Packet: ", pkt.tolist()
        print "Answer: ", answer.tolist()
        print "ERR pkt != answer"
        return None

    return answer.tolist()


"""
USB Get message, only do a Get Report used to read Data
"""
def usbGetMsg(dev, reportId):
    answer = usbhidGetReport(dev, reportId, 8)
    return answer.tolist()


"""
credential class
"""
class credential:
    def __init__(self, name="", user="", hop="", passw="", submit=""):
        # Initialize with VID and PID
        self.name = name
        self.user = user
        self.hop = hop
        self.passw = passw
        self.submit = submit

    def __str__(self):
        # connect
        return "%s - %s - %s - %s - %s" % (self.name, self.user, self.hop, self.passw, self.submit)

    def encrypt(self, key):
        # STEP 1 add string to block
        block = []
        block.extend(self.user + '\0')
        block.extend(self.hop + '\0')
        block.extend(self.passw + '\0')
        block.extend(self.submit + '\0')
        # STEP 2 convert characters to int
        for i in range(0, len(block)):
            block[i] = ord(block[i])

        # STEP 3 Fill empty fields to have a block multiple of 16
        p = 16 - len(block) % 16
        if (p != 16):
            block.extend([0] * p)

        # STEP 4 Encrypt
        tmp = []
        for i in range(0, len(block), 16):
            tmp.extend(ungroup32to8(NoekeonEncrypt(key, group8to32(block[i:i + 16]))))
        return tmp

    def decrypt(self, key, encryptedBlock):
        tmp = []
        eb = encryptedBlock
        for i in range(0, len(eb), 16):
            if (i + 16 <= len(eb)):
                tmp.extend(ungroup32to8(NoekeonDecrypt(key, group8to32(eb[i:i + 16]))))

        # Read Credentials
        step = 1
        for i in range(len(tmp)):
            c = tmp[i]
            # print chr(c)
            # STEP 1 -- Find User
            if (step == 1):
                if (c == 0):
                    step = 2
                else:
                    self.user = self.user + chr(c)
            # STEP 2 -- Find Hop
            elif (step == 2):
                if (c == 0):
                    step = 3
                else:
                    self.hop = self.hop + chr(c)
            # STEP 3 -- Find Pass
            elif (step == 3):
                if (c == 0):
                    step = 4
                else:
                    self.passw = self.passw + chr(c)
            # STEP 4 -- Find Submit
            elif (step == 4):
                if (c == 0):
                    step = 5
                else:
                    self.submit = self.submit + chr(c)

        return self


## Memtype COMM Types
class deviceInfo:
    def __init__(self, version=[0, 0, 0], credSize=0):
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
        if (self.dev != None):
            usb.util.dispose_resources(self.dev)

    def info(self):
        pkt = array('B', [5, 0, 0, 0, 0, 0, 0, 0])

        if self.printDebug: print pkt.tolist()

        if (usbhidSetReport(self.dev, pkt, self.reportId) != 8):
            if self.printDebug: print "ERR Set Report"
            return None

        msg = usbhidGetReport(self.dev, self.reportId, 8)
        version = [msg[1], msg[2], msg[3]]
        credSize = msg[5] << 8 + msg[4]
        info = deviceInfo(version, credSize)
        if self.printDebug: print info

        return info

    def write(self, block=[], offset=0):
        if (len(block) > self.info().credSize):
            if self.printDebug: print "ERR block size greater than expted !"
            return None

        # Block to send must be multiple of 8
        credSize = len(block)
        if (credSize % 8):
            block.extend([0] * (8 - (credSize % 8)))

        # Prepare Write CMD
        pkt = array('B', [3, lo(offset), hi(offset), lo(credSize), hi(credSize), 0, 0, 0])
        if (usbhidSetReport(self.dev, pkt, self.reportId) != 8):
            if self.printDebug: print "ERR Set Report"
            return None
        # Check CMD sent successfully
        answer = usbhidGetReport(self.dev, self.reportId, 8)
        if (pkt.tolist() != answer):
            if self.printDebug:
                print "Packet: ", pkt.tolist()
                print "Answer: ", answer
                print "ERR pkt != answer"

        # Send DATA, wait 50ms between transfers
        for i in range(0, len(block), 8):
            # Prepare packet
            pkt = array('B', block[i:i + 8])
            if (usbhidSetReport(self.dev, pkt, self.reportId) != 8):
                if self.printDebug: print "ERR Set Report"
                return None

            answer = usbhidGetReport(self.dev, self.reportId, 8)
            if (pkt.tolist() != answer):
                if self.printDebug:
                    print "Packet: ", pkt.tolist()
                    print "Answer: ", answer
                    print "ERR pkt != answer"

            # 50ms to let memtype do usbInterruptIsReady
            time.sleep(0.05)

        return block

    def read(self, credSize=DEFAULT_CREDENTIAL_SIZE, offset=0):
        pkt = array('B', [2, lo(offset), hi(offset), lo(credSize), hi(credSize), 0, 0, 0])

        if (usbhidSetReport(self.dev, pkt, self.reportId) != 8):
            if self.printDebug: print "ERR Set Report"
            return None

        answer = usbhidGetReport(self.dev, self.reportId, 8)
        if (pkt.tolist() != answer):
            if self.printDebug:
                print "Packet: ", pkt.tolist()
                print "Answer: ", answer
                print "ERR pkt != answer"

        block = []
        for msg in range(0, credSize, 8):
            answer = usbhidGetReport(self.dev, self.reportId, 8)
            block.extend(answer)

        return block


if __name__ == '__main__':
    # Search for the memtype and read hid data
    m = memtype()
    m.info()
    block = m.read()
    cl = decryptCredentialList(block, key=pinToKey("0000"))
    for i in range(len(cl)):
        cl[i].name = "credName%d" % i

    block = encryptCredentialList(cl, key=pinToKey("0000"))
    m.write(block)
    m.disconnect()

