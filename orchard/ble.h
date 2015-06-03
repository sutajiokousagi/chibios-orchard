#ifndef __BLE_H__
#define __BLE_H__

#ifndef NRF_DEBUG
#define NRF_DEBUG 1
#define NRF_VERBOSE_DEBUG 1
#endif

typedef uint8_t nRFLen;
typedef uint8_t nRFPipe;

struct _BLEDevice;
typedef struct _BLEDevice BLEDevice;
extern BLEDevice BLE1;

#include "ble-registers.h"
#include "ble-data.h"
#include "ble-service.h"

#define NRF_RX_BUFFERS 5

#if NRF_DEBUG
#include "chprintf.h"
#define nrf_debug(msg) chprintf(stream, msg "\r\n")
#define nrf_debugnl(msg) chprintf(stream, msg "\r\n")
#else
#define nrf_debug(msg)
#define nrf_debugnl(msg)
#endif

// event handlers
typedef void (*nRFEventHandler) (BLEDevice *ble, nRFEvent *);
typedef void (*nRFCommandResponseHandler) (BLEDevice *ble, uint8_t opcode, uint8_t status);
typedef void (*nRFTemperatureHandler) (BLEDevice *ble, float tempC);
typedef void (*nRFBatteryLevelHandler) (BLEDevice *ble, float voltage);
typedef void (*nRFDeviceVersionHandler) (BLEDevice *ble, uint16_t configId,
    uint8_t aciVersion, uint8_t setupFormat, uint8_t configStatus);
typedef void (*nRFDeviceAddressHandler) (BLEDevice *ble, uint8_t *address,
    uint8_t addressType);
typedef void (*nRFDynamicDataHandler) (BLEDevice *ble, uint8_t seqNo, uint8_t *data);
typedef void (*nRFConnectedHandler) (BLEDevice *ble, uint8_t addressType, uint8_t *peerAddress,
    void *connectionData);
typedef void (*nRFDisconnectedHandler) (BLEDevice *ble, uint8_t aciStatus, uint8_t btLeStatus);
typedef void (*nRFBondStatusHandler) (BLEDevice *ble, void *bondStatusData);
typedef void (*nRFKeyRequestHandler) (BLEDevice *ble, uint8_t keyType);
typedef void (*nRFPipeErrorHandler) (BLEDevice *ble, nRFPipe servicePipeNo,
    uint8_t errorCode, uint8_t *errorData);
typedef void (*nRFDataReceivedHandler) (BLEDevice *ble, nRFPipe servicePipeNo,
    uint8_t *data);
typedef void (*nRFDataAckHandler) (BLEDevice *ble, nRFPipe servicePipeNo);


void bleReset(BLEDevice *ble);
void bleStart(BLEDevice *ble, SPIDriver *spip);

void bleDebugEvent(BLEDevice *ble, nRFEvent *event);

nRFTxStatus blePoll(BLEDevice *ble, uint16_t timeout);
nRFDeviceState bleDeviceState(BLEDevice *ble);
nRFCmd bleSetup(BLEDevice *ble, const struct ble_service *service);

uint8_t bleCreditsAvailable(BLEDevice *ble);
uint8_t bleIsConnected(BLEDevice *ble);
nRFConnectionStatus bleGetConnectionStatus(BLEDevice *ble);

uint8_t bleIsPipeOpen(BLEDevice *ble, nRFPipe servicePipeNo);

nRFTxStatus bleTest(BLEDevice *ble, uint8_t feature);
nRFTxStatus bleSleep(BLEDevice *ble);
nRFTxStatus bleGetDeviceVersion(BLEDevice *ble);
nRFTxStatus bleEcho(BLEDevice *ble, nRFLen dataLength, uint8_t *data);
nRFTxStatus bleWakeup(BLEDevice *ble);
nRFTxStatus bleGetBatteryLevel(BLEDevice *ble);
nRFTxStatus bleGetTemperature(BLEDevice *ble);
nRFTxStatus bleSetTxPower(BLEDevice *ble, uint8_t powerLevel);
nRFTxStatus bleGetDeviceAddress(BLEDevice *ble);
nRFTxStatus bleConnect(BLEDevice *ble, uint16_t timeout, uint16_t advInterval);
nRFTxStatus bleRadioReset(BLEDevice *ble);
nRFTxStatus bleBond(BLEDevice *ble, uint16_t timeout, uint16_t advInterval);
nRFTxStatus bleDisconnect(BLEDevice *ble, uint8_t reason);
nRFTxStatus bleChangeTimingRequest(BLEDevice *ble, uint16_t intervalMin,
                           uint16_t intervalMax,
                           uint16_t slaveLatency,
                           uint16_t timeout);
nRFTxStatus bleOpenRemotePipe(BLEDevice *ble, nRFPipe servicePipeNo);
nRFTxStatus bleCloseRemotePipe(BLEDevice *ble, nRFPipe servicePipeNo);
nRFTxStatus bleDtmCommand(BLEDevice *ble, uint16_t dtmCmd);
nRFTxStatus bleReadDynamicData(BLEDevice *ble);
nRFTxStatus bleWriteDynamicData(BLEDevice *ble, uint8_t seqNo,
                           nRFLen dataLength,
                           uint8_t *data);
nRFTxStatus bleSetApplLatency(BLEDevice *ble, uint8_t applLatencyMode,
                         uint16_t latency);
nRFTxStatus bleSetKey(BLEDevice *ble, uint8_t keyType, uint8_t *key);
nRFTxStatus bleOpenAdvPipe(BLEDevice *ble, uint64_t advServiceDataPipes);
nRFTxStatus bleBroadcast(BLEDevice *ble, uint16_t timeout, uint16_t advInterval);
nRFTxStatus bleBondSecurityRequest(BLEDevice *ble);
nRFTxStatus bleDirectedConnect(BLEDevice *ble);
nRFTxStatus bleSendData(BLEDevice *ble, nRFPipe servicePipeNo,
                   nRFLen dataLength,
                   uint8_t *data);
nRFTxStatus bleRequestData(BLEDevice *ble, nRFPipe servicePipeNo);
nRFTxStatus bleSetLocalData(BLEDevice *ble, nRFPipe servicePipeNo,
                       nRFLen dataLength,
                       uint8_t *data);
nRFTxStatus bleSendDataAck(BLEDevice *ble, nRFPipe servicePipeNo);
nRFTxStatus bleSendDataNack(BLEDevice *ble, nRFPipe servicePipeNo,
                       uint8_t errorCode);

void bleSetEventHandler(BLEDevice *ble, nRFEventHandler handler);
void bleSetCommandResponseHandler(BLEDevice *ble, nRFCommandResponseHandler handler);
void bleSetTemperatureHandler(BLEDevice *ble, nRFTemperatureHandler handler);
void bleSetBatteryLevelHandler(BLEDevice *ble, nRFBatteryLevelHandler handler);
void bleSetDeviceVersionHandler(BLEDevice *ble, nRFDeviceVersionHandler handler);
void bleSetDeviceAddressHandler(BLEDevice *ble, nRFDeviceAddressHandler handler);
void bleSetDynamicDataHandler(BLEDevice *ble, nRFDynamicDataHandler handler);
void bleSetConnectedHandler(BLEDevice *ble, nRFConnectedHandler handler);
void bleSetDisconnectedHandler(BLEDevice *ble, nRFDisconnectedHandler handler);
void bleSetBondStatusHandler(BLEDevice *ble, nRFBondStatusHandler handler);
void bleSetKeyRequestHandler(BLEDevice *ble, nRFKeyRequestHandler handler);
void bleSetPipeErrorHandler(BLEDevice *ble, nRFPipeErrorHandler handler);
void bleSetDataReceivedHandler(BLEDevice *ble, nRFDataReceivedHandler handler);
void bleSetDataAckHandler(BLEDevice *ble, nRFDataAckHandler handler);

#endif /* __BLE_H__ */
