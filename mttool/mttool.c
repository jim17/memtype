/* Name: mttool.c
 * Project: memtype Area0x33.com
 * Authors: Noel Carriqui, Miguel Angel Borrego
 * Based on: hidtool example by  Christian Starkjohann
 * Creation Date: 2014-04-11
 * Tabsize: 4
 * Copyright: (c) 2014 by Area0x33.com
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "hiddata.h"
#include "../usbconfig.h"  /* for device VID, PID, vendor name and product name */
#include "../ucp.h"
#include <unistd.h> /* to sleep ms */
#include <getopt.h>
#include "sxmlc/src/sxmlc.h"
#include "sxmlc/src/sxmlsearch.h"

#include "NessieInterfaces.h"

#define MAX_FILE_LEN    4096

/* ------------------------------------------------------------------------- */

//Global argument storage
struct globalArgs_t
{
    int rFlag;              //Read
    int wFlag;              //Write
    int iFlag;              //Info
    int pFlag;              //setPin
    int hashFlag;           //readPinHash
    int kFlag;             //keyboard
    char pin[5];            //Pin itself
    const char* keyboard;    //Keyboard filename
    int fFlag;              //to File?
    int offset;             //Offset
    int size;               //Size
    char* fileName;   //File name
    FILE* file;             //the file pointer to use
} globalArgs;

void printUsage(char* ename)
{
    printf("\nUsage: %s [ -options ] [ -f filename ]\n\n",ename);
    printf("Options:\n");
    printf("-h, -?\t\tShow this message.\n");
    printf("-r\t\tRead from device. Only one action at a time, don't combine with -w or -i.\n");
    printf("-i\t\tGet info from device. Only one action at a time, don't combine with -w or -r.\n");
    printf("-w\t\tWrite from file to device. Only one action at a time, don't combine with -r or -i.\n");
    printf("-o offset\t\tSpecify offset in bytes. Default offset = 0.\n");
    printf("-p pin\t\tSet PIN to memtype device (if not read or write action).\n");
    printf("-P\t\tRead PIN Hash from memtype device.\n");
    printf("-s size\t\tSpecify size in bytes. Default size = 512.\n");
    printf("-f filename\tSpecify a filename to read from or write to.\n\n");
    printf("-k filename\tLoad a keyboard file into the memType.\n\n");

    printf("NOTE: Remember to use -p 0000 on write and read to encrypt and decrypt!.\n\n");

    exit(EXIT_FAILURE);
}



static char *usbErrorMessage(int errCode)
{
static char buffer[80];

    switch(errCode){
        case USBOPEN_ERR_ACCESS:      return "Access to device denied";
        case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
        case USBOPEN_ERR_IO:          return "Communication error with device";
        default:
            sprintf(buffer, "Unknown USB error %d", errCode);
            return buffer;
    }
    return NULL;    /* not reached */
}

static usbDevice_t  *openDevice(void)
{
usbDevice_t     *dev = NULL;
unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
char            vendorName[] = {USB_CFG_VENDOR_NAME, 0}, productName[] = {USB_CFG_DEVICE_NAME, 0};
int             vid = rawVid[0] + 256 * rawVid[1];
int             pid = rawPid[0] + 256 * rawPid[1];
int             err;

    if((err = usbhidOpenDevice(&dev, vid, vendorName, pid, productName, 0)) != 0){
        fprintf(stderr, "error finding %s: %s\n", productName, usbErrorMessage(err));
        return NULL;
    }
    return dev;
}

/* ------------------------------------------------------------------------- */

static void hexdump(uint8_t *buffer, int len)
{
int     i;
FILE    *fp = stderr;

    for(i = 0; i < len; i++){
        if(i != 0){
            if(i % 16 == 0){
                fprintf(fp, "\n");
            }else{
                fprintf(fp, " ");
            }
        }
        fprintf(fp, "0x%02x", buffer[i] & 0xff);
    }
    if(i != 0)
        fprintf(fp, "\n");
}


/* Specific protocol */
typedef struct __attribute__((packed))
{
    uint8_t reportid;
    uint8_t cmd;
    uint8_t buff[7];
} ucp_cmd_t;

typedef struct __attribute__((packed))
{
    uint8_t reportid;
    uint8_t data[8];
} ucp_request_t;

typedef struct __attribute__((packed))
{
    uint8_t reportid;
    uint8_t data[8];
} ucp_response_t;

typedef struct __attribute__((packed))
{
    uint8_t versionMajor;
    uint8_t versionMinor;
    uint8_t versionPatch;
    uint16_t credSize;
    uint16_t dummy;
    
} info_t;

typedef struct __attribute__((packed))
{
   uint16_t offset;
   uint16_t size;
   uint8_t type;
}read_t;

typedef struct
{
    uint8_t* name;
    uint8_t* line1;
    uint8_t* hop;
    uint8_t* line2;
    uint8_t* submit;
    uint16_t next;
    uint8_t* nextaddr;
} credentialFlash_t;

read_t deviceRead;
info_t deviceInfo;
uint8_t flashMemory[MAX_FILE_LEN];
void cmdKeyboard(usbDevice_t *dev);
void cmdReset(ucp_cmd_t* cmd);
void cmdRead(usbDevice_t *dev, uint16_t len);
void cmdWrite(usbDevice_t *dev, uint8_t* buff, uint16_t len);
void cmdSetPin(usbDevice_t *dev);
void cmdReadPinHash(usbDevice_t *dev);
void cmdData(ucp_cmd_t* cmd);
void cmdInfo(info_t* info);
void cmdEcho(ucp_cmd_t* cmd);
uint16_t readXML(char * filePath);

uint8_t HIDread(usbDevice_t *dev, uint8_t reportId, uint8_t *buff, uint16_t size);

void readAllCredentials(uint8_t* buff, char* filePath);
uint8_t* readCredential(uint8_t* current, uint8_t* buff, FILE* file);
void XML_createTag(FILE* file, char *tagName, char* text);

uint16_t ncrypt(uint8_t* src, uint8_t* dst, uint16_t len);
uint16_t dcrypt(uint8_t* src, uint8_t* dst, uint16_t len);

int main(int argc, char **argv)
{


    int option = 0;      // For getopt temp option

    usbDevice_t *dev;
    ucp_cmd_t buffer;    //room for dummy report ID
    int         err;

    //Initialize globalArgs before we get to work
    globalArgs.rFlag = 0;   //False
    globalArgs.iFlag = 0;   //False
    globalArgs.wFlag = 0;   //False
    globalArgs.fFlag = 0;   //False
    globalArgs.pFlag = 0;   //False
    globalArgs.hashFlag = 0;   //False
    globalArgs.kFlag = 0;   //False
    strncpy(globalArgs.pin,"0000",sizeof(globalArgs.pin)); //Default pin
    globalArgs.offset = 0; //Default
    globalArgs.size = 512; //Default
    globalArgs.fileName = NULL;
    globalArgs.file = NULL;

    //Check if no arguments at all
    if(argc==1) printUsage(argv[0]);

    //If there's some argument, parse them
    while((option = getopt(argc, argv, "rwif:o:p:Pk:s:h?")) !=-1){
    //Check option flags
    switch(option){
        case 'r':
            globalArgs.rFlag = 1; //True
            break;
        case 'w':
            globalArgs.wFlag = 1; //True
            break;
        case 'i':
            globalArgs.iFlag = 1; //True
            break;
        case 'f':
            globalArgs.fFlag = 1; //True
            globalArgs.fileName = optarg;
            printf("File: %s\n",globalArgs.fileName);
            break;
        case 'o':
            globalArgs.offset = atoi(optarg);
            break;
        case 'p':
            globalArgs.pFlag = 1; //True
            strncpy(globalArgs.pin,optarg, sizeof(globalArgs.pin));
            break;
        case 'P':
            globalArgs.hashFlag=1; //True
            break;
        case 'k':
            globalArgs.kFlag = 1; //True
            globalArgs.keyboard = optarg;
            printf("Keyboard: %s\n", globalArgs.keyboard);
        case 's':
            globalArgs.size = atoi(optarg);
            break;
        case 'h':
        case '?':
            printUsage(argv[0]);
            break;
        //Unknown flag, don't know what to do
        default:
            //After getopt prints the error
            printUsage(argv[0]);
            break;
    }
    }

    //Check that only one action is done at a time
    if(globalArgs.rFlag + globalArgs.wFlag + globalArgs.iFlag > 1) printUsage(argv[0]);
    //Check that not set PIN and read HASH are given at a time
    if(globalArgs.pFlag + globalArgs.hashFlag > 1) printUsage(argv[0]);
    //Check that if write from file to device, we are given a file and a pin to encrypt!
    if(globalArgs.wFlag  && !globalArgs.fFlag) printUsage(argv[0]);
    if(globalArgs.wFlag && !globalArgs.pFlag) printUsage(argv[0]);
    //Also check pin for read flag
    if(globalArgs.rFlag && !globalArgs.pFlag) printUsage(argv[0]);

    //Try to open the device, exit if no device present.
    if((dev = openDevice()) == NULL) exit(1);

    //Clean the buffer before working with it
    memset((void*)&buffer, 0, sizeof(buffer));
    

    //Check and perform the desired commands
    if(globalArgs.rFlag){  //READ COMMAND
        buffer.cmd = UCP_CMD_READ;
        deviceRead.offset = globalArgs.offset;
        deviceRead.size = globalArgs.size;
        memcpy((void*)&buffer.buff, (void*)&deviceRead, sizeof(deviceRead));

        //give some feedback
        fprintf(stderr,"Reading from the MemType: offset=%d bytes, size=%d bytes.\n",deviceRead.offset, deviceRead.size);

    }else if(globalArgs.wFlag){  //WRITE COMMAND
        buffer.cmd = UCP_CMD_WRITE;
        deviceRead.offset = globalArgs.offset;
        deviceRead.size = readXML(globalArgs.fileName);
        memcpy((void*)&buffer.buff, (void*)&deviceRead, sizeof(deviceRead));

        //give some feedback
        fprintf(stderr,"Writing to the MemType: offset=%d bytes, size=%d bytes.\n",deviceRead.offset, deviceRead.size);

    }else if(globalArgs.iFlag){  //INFO COMMAND
        buffer.cmd = UCP_CMD_INFO;
    }else if(globalArgs.hashFlag){ //READ PIN HASH COMMAND
        buffer.cmd = UCP_CMD_READ_PIN;
    }else if(globalArgs.pFlag){ //SET PIN COMMAND
        buffer.cmd = UCP_CMD_SET_PIN;
        buffer.buff[0] = 16; //sizeof hash in bytes
    }else if(globalArgs.kFlag){
        buffer.cmd = UCP_CMD_KEYBOARD;
    }
       
        //Add a dummy report ID and send data to device 
        buffer.reportid = 2;
        if((err = usbhidSetReport(dev, (char*)&buffer, sizeof(buffer))) != 0)
            fprintf(stderr, "Error sending data to device: %s\n", usbErrorMessage(err));

        //Read back report 
        int len = sizeof(buffer);
        if((err = usbhidGetReport(dev, 2, (char*)&buffer, &len)) != 0)
        {   //... if not OK, print error
            fprintf(stderr, "Error reading data from device: %s\n", usbErrorMessage(err));
        }
        else //... if OK, do things :)
        {
            fprintf( stderr, "\nMemType CMD Response: ");
            hexdump( (void*)&buffer.cmd, sizeof(buffer.cmd)+sizeof(buffer.buff));
            fprintf( stderr, "Received data from the device: \n");
            switch(buffer.cmd)
            {
                case UCP_CMD_RESET:
                    fprintf( stderr, "RESET\n");
                    break;
                case UCP_CMD_READ:
                    fprintf( stderr, "READ\n");
                    cmdRead(dev, deviceRead.size);
                    break;
                case UCP_CMD_WRITE:
                    fprintf( stderr, "WRITE\n");
                    cmdWrite(dev, flashMemory, deviceRead.size);
                    fprintf(stderr, "[ENCRYPTION TEST] only hexdump");
                    break;
                case UCP_CMD_SET_PIN:
                    fprintf(stderr, "SET PIN\n");
                    cmdSetPin(dev);
                    break;
                case UCP_CMD_READ_PIN:
                    fprintf(stderr, "READ PIN HASH\n");
                    cmdReadPinHash(dev);
                    break;
                case UCP_CMD_KEYBOARD:
                    fprintf(stderr, "KEYBOARD\n");
                    cmdKeyboard(dev);
                    break;
                case UCP_CMD_DATA:
                    fprintf( stderr, "DATA\n");
                    break;
                case UCP_CMD_INFO:
                    fprintf( stderr, "sizeof(info) -> %lu\n", sizeof(deviceInfo));
                    memcpy((void*)&deviceInfo, (void*)buffer.buff, sizeof(deviceInfo));
                    
                    /* Call info */
                    cmdInfo(&deviceInfo);
                    break;
                case UCP_CMD_ERROR:  //Wait! the device returned one error!
                    switch( (unsigned char) buffer.buff[0] )
                    {
                        case UCP_ERR:
                            fprintf( stderr, "GENERIC ERROR\n");
                            break;
                        case UCP_ERR_PACKET:
                            fprintf( stderr, "PACKET ERROR\n");
                            break;
                        case UCP_ERR_CMD:
                            fprintf( stderr, "CMD ERROR\n");
                            break;
                        case UCP_ERR_ADDR:
                            fprintf( stderr, "ADDR ERROR\n");
                            break;
                        case UCP_ERR_SIZE:
                            fprintf( stderr, "SIZE ERRROR\n");
                            break;
                        case UCP_ERR_PROTOCOL:
                            fprintf( stderr, "PROTOCOL ERROR\n");
                            break;
                        case UCP_ERR_LOCKED:
                            fprintf( stderr, "DEVICE LOCKED ERROR\n");
                            break; 
                        default:
                            fprintf( stderr, "UNKNOWN ERROR\n");
                    }
                    break;
                default:
                    fprintf( stderr, "UNKNOWN CMD ERROR\n");
            }
        }

    usbhidCloseDevice(dev);
    
    return 0;
}

uint8_t HIDwrite(usbDevice_t *dev, uint8_t reportId, uint8_t *buff, uint16_t size)
{
    unsigned int i;
    int len;
    int err;
    ucp_request_t request;
    ucp_response_t response;
    
    for(i=0; i<size; i+= 8)
    {
        request.reportid = reportId;
        memcpy((void*)&request.data[0], (void*)&buff[i], sizeof(request.data));
        
        if((err = usbhidSetReport(dev, (char*)&request, sizeof(request))) != 0)   /* add a dummy report ID */
            fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));        
        
        len = sizeof(response);
        if((err = usbhidGetReport(dev, reportId, (char*)&response, &len)) != 0)
        {
            fprintf(stderr, "error reading data: %s\n", usbErrorMessage(err));
        }
        else
        {
            hexdump( (void*)&response.data[0], 8);            
        }
        
                
        if(usleep(50000) < 0) // sleep 50 ms
        {

            fprintf( stderr, "sleep ERR\n");
        }
    }
    
    return 0;
}

uint8_t HIDread(usbDevice_t *dev, uint8_t reportId, uint8_t *buff, uint16_t size)
{
    unsigned int i;
    int err;
    
    for(i=0; i<size; i+= 8)
    {
        ucp_response_t response;
        int len = sizeof(response);
        if((err = usbhidGetReport(dev, reportId, (char*)&response, &len)) != 0)
        {
            fprintf(stderr, "error reading data: %s\n", usbErrorMessage(err));
        }
        else
        {
            memcpy((void*)&buff[i], (void*)&response.data, sizeof(response.data));
            hexdump( (void*)&buff[i], 8);
        }
    }
    
    return 0;
}

void cmdReset(ucp_cmd_t* cmd)
{

}

//Perform the read from device
void cmdRead(usbDevice_t *dev, uint16_t len)
{
    HIDread(dev, 2, flashMemory, len);
    if (globalArgs.fFlag)
    {
    	fprintf( stderr, "readAllCredentials\n");
        readAllCredentials(flashMemory, globalArgs.fileName);
    }
}

void cmdWrite(usbDevice_t *dev, uint8_t* buff, uint16_t len)
{
    HIDwrite(dev, 2, buff, len);
}
void cmdSetPin(usbDevice_t *dev){
    uint8_t pinHash[16];
    uint8_t ePinHash[16];
    //Build pinHash from globalArgs.pin
    memcpy(pinHash,globalArgs.pin,4);
    memcpy(&pinHash[4],globalArgs.pin,4);
    memcpy(&pinHash[8],globalArgs.pin,4);
    memcpy(&pinHash[12],globalArgs.pin,4);

    ncrypt(pinHash,ePinHash,sizeof(pinHash));
    HIDwrite(dev,2,ePinHash,16);
    hexdump(ePinHash,sizeof(ePinHash));
}


void cmdKeyboard(usbDevice_t *dev){
    const int keybSIZE = 1024;
    char keyb[keybSIZE];
    uint8_t buf[128];
    char* tok;
    int i = 0;

    FILE* f = fopen(globalArgs.keyboard, "r");

    if (f == NULL)
    {
        printf("Error opening keyboard File\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }
   
    if(fgets(keyb, keybSIZE, f) == NULL) {
        printf("Error reading keyboard File\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }


    tok = strtok(keyb,",");

    while(tok != NULL){

        fprintf(stderr,"TOK: %s\n",tok);
        buf[i++] = atoi(tok);
        tok = strtok(NULL,",");
    }

    HIDwrite(dev,2,buf,sizeof(buf));
    hexdump(buf,sizeof(buf));
}

void cmdReadPinHash(usbDevice_t *dev){
    HIDread(dev,2,flashMemory,16);
}

void cmdData(ucp_cmd_t* cmd)
{
    
}

void cmdInfo(info_t* deviceInfo)
{
    
    fprintf( stderr, "INFO: v%d.%d.%d size:%d\n", deviceInfo->versionMajor, 
                                                        deviceInfo->versionMinor,
                                                        deviceInfo->versionPatch,
                                                        deviceInfo->credSize);
    fprintf(stdout, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");//xml, bitch
    fprintf(stdout,"<mt>\n"); //openning MT tag
    fprintf(stdout,"<info>\n"); //openning info tag
    
    fprintf(stdout, "<version>%u.%u.%u</version>\n",deviceInfo->versionMajor,
                                                     deviceInfo->versionMinor,
                                                     deviceInfo->versionPatch );
    fprintf(stdout, "<memsize>%u</memsize>\n", deviceInfo->credSize);
    
    fprintf(stdout,"</info>\n"); //closing info tag
    fprintf(stdout,"</mt>"); //closing MT tag
    
}

void cmdEcho(ucp_cmd_t* cmd)
{
    
}

uint8_t* readCredential(uint8_t* c, uint8_t* buff, FILE* file)
{   
    
	credentialFlash_t cred;
	uint8_t tempDecryptBuff[4096];
	uint8_t* ptr;
	int ctr, i, offset;
    
	/* Credential */
	ptr = c;
    cred.name = c;
    while(*ptr++ != 0);
    cred.next = ((uint16_t)ptr[1] << 8) + ptr[0];
    cred.nextaddr = &buff[cred.next];

    /* Decrypt */
    ptr += 2;
    offset = 0;
    ctr=0;
    while(ctr < 4)
    {

    	dcrypt( &ptr[offset], &tempDecryptBuff[offset], 16);
    	for(i=0; i<16; i++)
    	{
    		if(tempDecryptBuff[offset+i] == 0)
    		{
    			ctr++;
    		}
    	}
    	offset += 16;
    }

    cred.line1 = tempDecryptBuff;
    ptr = tempDecryptBuff;
    while(*ptr++ != 0);
    cred.hop = ptr;
    while(*ptr++ != 0);
    cred.line2 = ptr;
    while(*ptr++ != 0);
    cred.submit = ptr;

    /* Create Tags */
	XML_createTag(file, "name",(char*) cred.name);
	XML_createTag(file, "user",(char*) cred.line1);
	XML_createTag(file, "hop",(char*) cred.hop);
	XML_createTag(file, "pass",(char*) cred.line2);
	XML_createTag(file, "submit",(char*) cred.submit);

    return cred.nextaddr;
}

void readAllCredentials(uint8_t* buff, char* filePath)
{
    fprintf( stderr, "testPoint\n");
    FILE* file;
    fprintf( stderr, "File Path: %s\n", filePath);
    file = fopen(filePath, "w");
//    file = stderr;
    uint8_t* ptr = buff;
    int i;
    if (file != NULL)
    {
        fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");//xml, bitch
        fprintf(file,"<mt>\n"); //openning MT tag
        for(i=0; i<4096; i++)
        {
           fprintf(file,"<cred>\n"); //openning CRED tag
           ptr = readCredential(ptr, buff, file);
           fprintf(file,"</cred>\n"); //closing CRED tag
           if(ptr == buff || ptr == 0 ) break; //No more credentials left
        }
        fprintf(file,"</mt>"); //closing MT tag
    }
    fclose(file);
}

/* Abstraction function to create XML tags */
void XML_createTag(FILE* file, char *tagName, char* text)
{
    /* Open tag*/
    fprintf( file, "<%s>", tagName);
    if(text != 0)
    {
        fprintf( file, "%s", text);
    }
    /* Close tag*/
    fprintf( file, "</%s>\n", tagName);
}

/* ------------------------------------------------------------------------- */

uint16_t readXML(char* filePath)
{
    XMLDoc doc;
    XMLSearch search;
    XMLNode* node;
//    XMLNode* child;
    
    XMLNode *name,*user,*hop,*pass,*submit; 

    XMLDoc_init(&doc);
    XMLDoc_parse_file_DOM(filePath, &doc);

    XMLSearch_init(&search);
    XMLSearch_search_set_tag(&search, "cred");
    //XMLSearch_search_add_attribute(&search, "presence", NULL);
    //XMLSearch_search_add_attribute(&search, "value", "a*");
    
    uint8_t* buffptr = flashMemory;
    uint8_t* nextCredential_ptr;
    uint8_t* encryptStart;
    uint16_t length;

    node = doc.nodes[doc.i_root];
    if (XMLSearch_node_matches(node, &search))
            fprintf( stderr, "Found match: <%s>\n", node->tag);
    while ((node = XMLSearch_next(node, &search)) != NULL) {
            
        //child = XMLNode_next(node);
        //XMLNode* XMLNode_get_child(const XMLNode* node, int i_child);
        if (XMLNode_get_children_count(node) != 5)
        {
            fprintf( stderr, "[ERROR] BAD XML FILE");
        }
        else
        {
            name = XMLNode_get_child(node, 0);
            user = XMLNode_get_child(node, 1);
            hop = XMLNode_get_child(node, 2);
            pass = XMLNode_get_child(node, 3);
            submit = XMLNode_get_child(node, 4);

            fprintf( stderr, "Name: %s\n", name->text);
            fprintf( stderr, "User: %s\n", user->text);
            fprintf( stderr, "Hop:  %s\n", hop->text);
            fprintf( stderr, "Pass: %s\n", pass->text);
            fprintf( stderr, "Submit: %s\n", submit->text);
            
            
            if(name->text != 0)
                buffptr = buffptr + sprintf((char*)buffptr, "%s", name->text);
            else
                *buffptr = 0;
            buffptr += 1;
            
            nextCredential_ptr = buffptr;
            buffptr += 2;
            
            encryptStart = buffptr; // encryption must start here ! 
            
            if(user->text != 0)
                buffptr = buffptr + sprintf((char*)buffptr, "%s", user->text);
            else
                *buffptr = 0;
            buffptr += 1;
            
            if(hop->text != 0)
                buffptr = buffptr + sprintf((char*)buffptr, "%s", hop->text);
            else
                *buffptr = 0;
            buffptr += 1;
            
            if(pass->text != 0)
                buffptr = buffptr + sprintf((char*)buffptr, "%s", pass->text);
            else
                *buffptr = 0;
            buffptr += 1;
            
            if(submit->text != 0)
                buffptr = buffptr + sprintf((char*)buffptr, "%s", submit->text);
            else
                *buffptr = 0;
            buffptr += 1;
            
            // call encryption here !
            uint8_t source[4096];
            uint8_t dest[4096];
            uint16_t len = buffptr - encryptStart;
            // buffptr must be incremented to meet block size of encryption
            
	    if( (len % 16) != 0)
            {
            // block must be multiple of 16
	    len = ((len / 16) + 1) * 16;
            buffptr = encryptStart + len;
	    }


            memset((void*)source, 0, sizeof(source));
            memset((void*)dest, 0, sizeof(source));

            // copy credentials into 
            memcpy((void*)(source), (void*)encryptStart, len);

            fprintf(stderr, "\n PLAIN BLOCK TEXT\n" );

            fprintf(stderr, "\n ENCRYPTED BLOCK LEN %d\n",len );
            // start encryption here!
            length = ncrypt(source, dest, len);
            hexdump(encryptStart, len);
            memcpy((void*)encryptStart, dest, len);
            hexdump(encryptStart, len);
            fprintf(stderr, "\n -----------------\n " );


            uint16_t ptr = ((buffptr) - flashMemory);
            *nextCredential_ptr = (ptr & 0x00FF);
            *(nextCredential_ptr+1) = (ptr & 0xFF00)>> 8;

       }
        
    }

    *nextCredential_ptr = 0;
    *(nextCredential_ptr+1) = 0;

    uint16_t size = (buffptr - flashMemory);

    hexdump(flashMemory, size);
    
    XMLSearch_free(&search, true);
    XMLDoc_free(&doc);
    
    return size;
}

// returns the block length
uint16_t ncrypt(uint8_t* src, uint8_t* dst, uint16_t len)
{

    uint8_t key[16];
    //Build pinHash from globalArgs.pin
    memcpy(key,globalArgs.pin,4);
    memcpy(&key[4],globalArgs.pin,4);
    memcpy(&key[8],globalArgs.pin,4);
    memcpy(&key[12],globalArgs.pin,4);

    uint16_t i;
    uint8_t* ptr_src;
    uint8_t* ptr_dst;
    struct NESSIEstruct keystruct;
    uint8_t swap[4];

    // change 32 bit endiannes to be as same as the device
    for(i=0; i<sizeof(key); i+=4)
    {
        swap[0] = key[i];
        swap[1] = key[i+1];
        swap[2] = key[i+2];
        swap[3] = key[i+3];

        key[i] = swap[3];
        key[i+1] = swap[2];
        key[i+2] = swap[1];
        key[i+3] = swap[0];
    }
    ptr_src = src;
// change 32 bit endiannes to be as same as the device
    for(i=0; i<len; i+=4)
    {
        swap[0] = ptr_src[i];
        swap[1] = ptr_src[i+1];
        swap[2] = ptr_src[i+2];
        swap[3] = ptr_src[i+3];

        ptr_src[i] = swap[3];
        ptr_src[i+1] = swap[2];
        ptr_src[i+2] = swap[1];
        ptr_src[i+3] = swap[0];
    }
    
    NESSIEkeysetup (key,&keystruct);
    ptr_src = src;
    ptr_dst = dst;
    
    for(i=0; i<len; i+=16)
    {
        NESSIEencrypt (&keystruct,ptr_src,ptr_dst);
        ptr_src += 16;
        ptr_dst += 16;
    }

    ptr_src = dst;
// change 32 bit endiannes to be as same as the device
    for(i=0; i<len; i+=4)
    {
        swap[0] = ptr_src[i];
        swap[1] = ptr_src[i+1];
        swap[2] = ptr_src[i+2];
        swap[3] = ptr_src[i+3];

        ptr_src[i] = swap[3];
        ptr_src[i+1] = swap[2];
        ptr_src[i+2] = swap[1];
        ptr_src[i+3] = swap[0];
    }
    
    return i;

}

uint16_t dcrypt(uint8_t* src, uint8_t* dst, uint16_t len)
{

    uint8_t key[16];
    //Build pinHash from globalArgs.pin
    memcpy(key,globalArgs.pin,4);
    memcpy(&key[4],globalArgs.pin,4);
    memcpy(&key[8],globalArgs.pin,4);
    memcpy(&key[12],globalArgs.pin,4);
    
    uint16_t i;
    uint8_t* ptr_src;
    uint8_t* ptr_dst;
    uint8_t swap[4];

    // change 32 bit endiannes to be as same as the device
    for(i=0; i<sizeof(key); i+=4)
    {
        swap[0] = key[i];
        swap[1] = key[i+1];
        swap[2] = key[i+2];
        swap[3] = key[i+3];

        key[i] = swap[3];
        key[i+1] = swap[2];
        key[i+2] = swap[1];
        key[i+3] = swap[0];
    }

    ptr_src = src;
// change 32 bit endiannes to be as same as the device
    for(i=0; i<len; i+=4)
    {
        swap[0] = ptr_src[i];
        swap[1] = ptr_src[i+1];
        swap[2] = ptr_src[i+2];
        swap[3] = ptr_src[i+3];

        ptr_src[i] = swap[3];
        ptr_src[i+1] = swap[2];
        ptr_src[i+2] = swap[1];
        ptr_src[i+3] = swap[0];
    }
    
    struct NESSIEstruct keystruct;
    
    NESSIEkeysetup (key,&keystruct);
    ptr_src = src;
    ptr_dst = dst;
    
    for(i=0; i<len; i+=16)
    {
        NESSIEdecrypt (&keystruct,ptr_src,ptr_dst);
        ptr_src += 16;
        ptr_dst += 16;
    }
    
    ptr_src = dst;
// change 32 bit endiannes to be as same as the device
    for(i=0; i<len; i+=4)
    {
        swap[0] = ptr_src[i];
        swap[1] = ptr_src[i+1];
        swap[2] = ptr_src[i+2];
        swap[3] = ptr_src[i+3];

        ptr_src[i] = swap[3];
        ptr_src[i+1] = swap[2];
        ptr_src[i+2] = swap[1];
        ptr_src[i+3] = swap[0];
    }
    return i;

}
