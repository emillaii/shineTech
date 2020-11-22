// the following ifndef is for preventing double includes of this header file
#if !defined(__USBI2CIO_H__)
  #define __USBI2CIO_H__


#if defined(__cplusplus)
extern "C" {
#endif


#ifdef _DEBUG
  #define DbgWrStr(sDebug) OutputDebugString((sDebug))
#else
  #define DbgWrStr(sDebug)
#endif



//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// UsbI2cIo maximum devices
#define USBI2CIO_MAX_DEVICES             127

// I2C transaction constants
#define USBI2CIO_I2C_HEADER_SIZE         6
#define USBI2CIO_I2C_MAX_DATA            1088

// Block I/O transaction constants
#define USBI2CIO_IO_MAX_DATA             1088


// I2C transaction types
typedef enum {
  // standard transaction types
  I2C_TRANS_NOADR = 0x00,         // read or write with no address cycle
  I2C_TRANS_8ADR,                 // read or write with 8 bit address cycle
  I2C_TRANS_16ADR,                // read or write with 16 bit address cycle
  I2C_TRANS_NOADR_NS,             // read or write with no address cycle, stop signaling inhibited
  I2C_TRANS_XICOR,                // read or write with 8 bit instruction cycle, non I2C compliant use of R/W bit

  // customer specific transactions types for accessing rams, only write is mode supported
  I2C_TRANS_8ADR_NONSEQ = 0x30,   // read or write with 8 bit address cycle, non sequential
  I2C_TRANS_16ADR_NONSEQ,         // read or write with 16 bit address cycle, non sequential
  I2C_TRANS_24ADR_NONSEQ          // read or write with 24 bit address cycle, non sequential

} I2C_TRANS_TYPE;


// BLock I/O transaction constants
typedef enum {
  IO_MODE_SINGLE = 0,
  IO_MODE_BLOCK_ABC,
  IO_MODE_BLOCK_A,
  IO_MODE_BLOCK_B,
  IO_MODE_BLOCK_C,
  IO_MODE_BLOCK_AB,
  IO_MODE_BLOCK_BC,
  IO_MODE_BLOCK_AC
} IO_MODE;


// Properties constants
typedef enum {
  // MAX PROPERTIES (byte 0)
  PROPERTY_WRCOMMAND_RDCOUNT = 0x00,          // when setting, indicates command, when reading indicates table size
  // I2C PROPERTIES (byte 1)
  PROPERTY_I2C_CONFIG,
  // FAST TRANSFER PROPERTIES (byte 2)
  PROPERTY_FAST_XFER_CONFIG,
  // I/O PINS GLOBAL PROPERTIES (byte 3)
  PROPERTY_IO_CONFIG_GLOBAL,
  // I/O PINS PORT CONFIG PROPERTIES (bytes 4-6)
  PROPERTY_IO_CONFIG_PORTA,
  PROPERTY_IO_CONFIG_PORTB,
  PROPERTY_IO_CONFIG_PORTC,
  // I/O PINS PORT OUTPUT PROPERTIES (bytes 7-9)
  PROPERTY_IO_OUTPUT_PORTA,
  PROPERTY_IO_OUTPUT_PORTB,
  PROPERTY_IO_OUTPUT_PORTC,
  // DEBUG PROPERTIES (bytes 10-12)
  PROPERTY_DEBUG_CONFIG_GLOBAL,
  PROPERTY_DEBUG_CONFIG_0,
  PROPERTY_DEBUG_CONFIG_1,
  // USER PROPERTIES (bytes 13-16)
  PROPERTY_USER_0,
  PROPERTY_USER_1,
  PROPERTY_USER_2,
  PROPERTY_USER_3,
  PROPERTY_MAX_PROPERTY_BYTES
} USBI2CIO_PROPERTY_OFFSETS;

typedef enum {
  PROPERTY_CMD_STORE_TABLE_TO_EEPROM = 0x00,
  PROPERTY_CMD_LOAD_TABLE_FROM_EEPROM,
  PROPERTY_CMD_DISABLE_EEPROM_TABLE,
  PROPERTY_CMD_RESET_TO_DEFAULTS
} USBI2CIO_PROPERTY_COMMANDS;


// MAX PROPERTIES (byte 0)
//#define PROP_MAX_PROPERTIES               0x00


// I2C PROPERTIES (byte 1)
//#define PROP_I2C_BYTE                     0x01

  #define PROP_I2C_RETRIES_FIELD          0x07
  #define PROP_I2C_IGNORE_NAK             0x08
  #define PROP_I2C_POLL_EEPROM_ACK        0x10
  #define PROP_I2C_RESERVED_FIELD         0xE0

// FAST TRANSFER PROPERTIES (byte 2)
//#define PROP_FAST_XFER_BYTE               0x02

  #define PROP_FAST_XFER_RD_FIELD         0x07
  #define PROP_FAST_XFER_RD_ENABLE        0x08
  #define PROP_FAST_XFER_WR_FIELD         0x70
  #define PROP_FAST_XFER_WR_ENABLE        0x80

// I/O PINS GLOBAL PROPERTIES (byte 3)
//#define PROP_IO_GLOBAL_CONFIG_BYTE        0x03
  #define PROP_IO_23BIT_MODE              0x01

// I/O PINS PORT CONFIG PROPERTIES (bytes 4-6)
//#define PROP_IO_PORTA_CONFIG_BYTE         0x04
//#define PROP_IO_PORTB_CONFIG_BYTE         0x05
//#define PROP_IO_PORTC_CONFIG_BYTE         0x06

// I/O PINS PORT OUTPUT PROPERTIES (bytes 7-9)
//#define PROP_IO_PORTA_OUTPUT_BYTE         0x07
//#define PROP_IO_PORTB_OUTPUT_BYTE         0x08
//#define PROP_IO_PORTC_OUTPUT_BYTE         0x09

// DEBUG PROPERTIES (bytes 10-12)
//  PROPERTY_DEBUG_CONFIG_GLOBAL,
  #define PROP_DBG_MAX_DISPLAY_FIELD      0x7F
  #define PROP_DBG_GLOBAL_ENABLE          0x80

//  PROPERTY_DEBUG_CONFIG_0,
//  PROPERTY_DEBUG_CONFIG_1,

// USER PROPERTIES (bytes 10-13)
//#define PROP_USER_0                       0x0A
//#define PROP_USER_1                       0x0B
//#define PROP_USER_2                       0x0C
//#define PROP_USER_3                       0x0D

//#define PROP_MAX_PROP_BYTES               0x0E



//-----------------------------------------------------------------------------
// Structure Definitions
//-----------------------------------------------------------------------------
typedef struct _DEVINFO {             // structure for device information
  BYTE byInstance;
  BYTE SerialId[9];
} DEVINFO, *LPDEVINFO;


#pragma pack(push, 1)                       // force byte alignment

// Byte alignment matters for the structures contained within these pack pragmas.
// That's because it they goes over the USB cable and are used by the USB-I2C/IO's
// micro-controller.  The two processors (host and device) must therefore agree
// on exactly how things are aligned in memory, and the actual size in memory
// must be the same.


typedef struct _I2C_TRANS {
  BYTE byTransType;
  BYTE bySlvDevAddr;
  WORD wMemoryAddr;
  WORD wCount;
  BYTE Data[1088];
} I2C_TRANS, *PI2C_TRANS;

#pragma pack(pop)                          // restore previous alignment

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// API Function Prototypes (exported)
//-----------------------------------------------------------------------------

// version calls
WORD _stdcall DAPI_GetDllVersion(void);
WORD _stdcall DAPI_GetDriverVersion(void);
BOOL _stdcall DAPI_GetFirmwareVersion(HANDLE hDevInstance, LPWORD lpwVersion);


// open/close/info calls
HANDLE _stdcall DAPI_OpenDeviceInstance(LPSTR lpsDevName, BYTE byDevInstance);
BOOL _stdcall DAPI_CloseDeviceInstance(HANDLE hDevInstance);
BOOL _stdcall DAPI_DetectDevice(HANDLE hDevInstance);
BYTE _stdcall DAPI_GetDeviceCount( LPSTR lpsDevName );
BYTE _stdcall DAPI_GetDeviceInfo( LPSTR lpsDevName, LPDEVINFO lpDevInfo);
HANDLE _stdcall DAPI_OpenDeviceBySerialId(LPSTR lpsDevName, LPSTR lpsDevSerialId);
BOOL _stdcall DAPI_GetSerialId(HANDLE hDevInstance, LPSTR lpsDevSerialId);


// I/O calls
BOOL _stdcall DAPI_ConfigIoPorts(HANDLE hDevInstance, ULONG ulIoPortConfig);
BOOL _stdcall DAPI_GetIoConfig(HANDLE hDevInstance, LPLONG lpulIoPortConfig);
BOOL _stdcall DAPI_ReadIoPorts(HANDLE hDevInstance, LPLONG lpulIoPortData);
BOOL _stdcall DAPI_WriteIoPorts(HANDLE hDevInstance, ULONG ulIoPortData, ULONG ulIoPortMask);
LONG _stdcall DAPI_BlockWriteIoPorts(HANDLE hDevInstance, WORD wBlockIoMode, WORD wBlockIoIndex, BYTE *pbyData, WORD wCount);
LONG _stdcall DAPI_BlockReadIoPorts(HANDLE hDevInstance, BYTE *pbyData, WORD wBlockIoMode, WORD wBlockIoIndex, WORD wCount);

// I2c calls
LONG _stdcall DAPI_ReadI2c(HANDLE hDevInstance, PI2C_TRANS TransI2C);
LONG _stdcall DAPI_WriteI2c(HANDLE hDevInstance, PI2C_TRANS TransI2C);

// debugging calls
LONG _stdcall DAPI_ReadDebugBuffer(LPSTR lpsDebugString, HANDLE hDevInstance, LONG ulMaxBytes);

// fast transfer calls
LONG _stdcall DAPI_WriteFastXferVr(HANDLE hDevInstance,
									WORD wCount, PBYTE pbyWriteData);
LONG _stdcall DAPI_ReadFastXferVr(HANDLE hDevInstance,
									PBYTE pbyReadData, WORD wCount);
// generic USB transfer calls
BOOL _stdcall DAPI_TransferData(HANDLE hDevInstance, BYTE byEndpoint, PBYTE pBuffer, PULONG pulLength);

LONG _stdcall DAPI_SetVendorRequest(HANDLE hDevInstance,
									BYTE byRequest, WORD wValue, WORD wIndex, WORD wLength, PBYTE pbySetData);

LONG _stdcall DAPI_GetVendorRequest(HANDLE hDevInstance,
									PBYTE pbyGetData, BYTE byRequest, WORD wValue, WORD wIndex, WORD wLength);
// property calls
BOOL _stdcall DAPI_SetProperty(HANDLE hDevInstance, BYTE byPropIndex, BYTE byPropValue);
BOOL _stdcall DAPI_GetProperty(HANDLE hDevInstance, LPBYTE lpbyPropValue, BYTE byPropIndex);

BOOL _stdcall DAPI_GetLastFirmwareError(HANDLE hDevInstance, LPLONG lplError);

// unimplemented calls
void _stdcall DAPI_EnablePolling(void);
void _stdcall DAPI_DisablePolling(void);
void _stdcall DAPI_GetPolledInfo(void);

#if defined(__cplusplus)
}
#endif

// the following #endif is for preventing double includes of this header file
#endif