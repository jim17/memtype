mttool depends on libusb.

Despite the -static option has been added to the makefile, there are some OS where it's not supported.

Check library dependencies on dynamic linking:

Linux:
ldd file
or:
readelf -d file

MacosX:
otool -L
or 
otool -l


