#include <string.h>

#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "orchard-events.h"

#include "ble.h"
#include "gpiox.h"
#include "hex.h"

struct _BLEDevice {
  SPIDriver                 *spip;
  uint64_t                  pipes_open;
  nRFEventHandler           listener;
  uint8_t                   credits;
  nRFDeviceState            device_state;
  int                       svc_msg_num;
  nRFConnectionStatus       connection_status;
  int                       old_baudrate;

  nRFCommandResponseHandler command_response_handler;
  nRFTemperatureHandler     temperature_handler;
  nRFBatteryLevelHandler    battery_level_handler;
  nRFDeviceVersionHandler   device_version_handler;
  nRFDeviceAddressHandler   device_address_handler;
  nRFDynamicDataHandler     dynamic_data_handler;
  nRFConnectedHandler       connected_handler;
  nRFDisconnectedHandler    disconnected_handler;
  nRFBondStatusHandler      bond_status_handler;
  nRFKeyRequestHandler      key_request_handler;
  nRFPipeErrorHandler       pipe_error_handler;
  nRFDataReceivedHandler    data_received_handler;
  nRFDataAckHandler         data_ack_handler;
};

BLEDevice BLE1;

static nRFTxStatus ble_transmit_receive(BLEDevice *ble, nRFCommand *txCmd, uint16_t timeout);
static nRFTxStatus ble_transmit_command(BLEDevice *ble, uint8_t command);
static nRFTxStatus ble_transmit_pipe_command(BLEDevice *ble, uint8_t command, nRFPipe pipe);

nRFDeviceState bleDeviceState(BLEDevice *ble) {

    return ble->device_state;
}

nRFCmd bleSetup(BLEDevice *ble, const struct ble_service *service)
{
  int last_svc_msg_num = -1;
  int loop = 0;

  while ((ble->device_state != nRFSetupState)
      && (ble->device_state != PreSetup)) {
    nrf_debug("Pumping Tx/Rx queue, waiting for 'Standby' state...\r\n");
    ble_transmit_receive(ble, NULL, 0);
  }

  nrf_debug("State is 'Standby'.  Starting to send service commands...\r\n");
  for (ble->svc_msg_num = 0;
      (ble->svc_msg_num >= 0) && (ble->svc_msg_num < service->count);
       loop++) {
#if NRF_DEBUG
    chprintf(stream, "sending setup message number %d/%d\r\n",
            ble->svc_msg_num + 1, service->count);
#endif
    if (ble->svc_msg_num != last_svc_msg_num) {
      last_svc_msg_num = ble->svc_msg_num;
      ble_transmit_receive(ble, (nRFCommand *)service->data[ble->svc_msg_num].buffer, 0);
    }
    else {
      ble_transmit_receive(ble, NULL, 0);
    }

    if (ble->device_state == Standby) {
      nrf_debug("device in Standby state, returning");
      return cmdSuccess;
    }
  }
  return cmdSetupError;
}

static void ble_select(BLEDevice *ble) {

  (void)ble;
  spiAcquireBus(ble->spip);
  ble->old_baudrate = ble->spip->spi->BR;
  ble->spip->spi->C1 |= (SPIx_C1_LSBFE /*| SPIx_C1_CPHA*/);
  ble->spip->spi->BR = 0x3; /* Divide 12 MHz clock by 16 */
  palClearPad(GPIOE, 18);
}

static void ble_unselect(BLEDevice *ble) {

  (void)ble;
  palSetPad(GPIOE, 18);
  ble->spip->spi->C1 &= ~(SPIx_C1_LSBFE /*| SPIx_C1_CPHA*/);
  ble->spip->spi->BR = ble->old_baudrate;
  spiReleaseBus(ble->spip);
}

static int ble_is_ready(BLEDevice *ble) {

  (void)ble;
  return palReadPad(GPIOC, 3) == 0;
}

static void ble_assert_reset(BLEDevice *ble) {

  (void)ble;
  gpioxClearPad(GPIOX, 5);
}

static void ble_deassert_reset(BLEDevice *ble) {

  (void)ble;
  gpioxSetPad(GPIOX, 5);
}

void bleReset(BLEDevice *ble) {

  ble->device_state = Initial;
  ble->credits = 0;
  ble->svc_msg_num = -2;
  ble->connection_status = Disconnected;

  ble_assert_reset(ble);
  chThdSleepMicroseconds(1);
  ble_deassert_reset(ble);
}

void bleStart(BLEDevice *ble, SPIDriver *spip) {

  nrf_debug("Initializing");

  ble->spip = spip;
  // Zero all the handlers
  ble->listener = 0;
  ble->command_response_handler = 0;
  ble->temperature_handler = 0;
  ble->battery_level_handler = 0;
  ble->device_version_handler = 0;
  ble->device_address_handler = 0;
  ble->dynamic_data_handler = 0;
  ble->connected_handler = 0;
  ble->disconnected_handler = 0;
  ble->bond_status_handler = 0;
  ble->key_request_handler = 0;
  ble->pipe_error_handler = 0;
  ble->data_received_handler = 0;
  ble->data_ack_handler = 0;

  /* Release SPI CS pin by setting CS high */
  palSetPad(GPIOE, 18);

  /* Set RESET pin to be an output */
  gpioxSetPadMode(GPIOX, 5, GPIOX_OUT_PUSHPULL);

  bleReset(ble);

  // Load up the first setup message and start interrupts
}

void ble_print_address(uint8_t *address)
{
  chprintf(stream, "0x");
  for (int i = NRF_ADDRESS_LENGTH - 1; i >= 0; i--)
    chprintf(stream, "%02x", address[i]);
}

void bleDebugEvent(BLEDevice *ble, nRFEvent *event)
{

  (void)ble;

  chprintf(stream, "EVENT debug:%d length:%d event:",
      event->debug, event->length);

  switch (event->event) {
  case NRF_DEVICESTARTEDEVENT:
    chprintf(stream, "DeviceStartedEvent");

    chprintf(stream, "Operating mode: ");
    switch (event->msg.deviceStarted.operatingMode) {
    case 0x01:
      chprintf(stream, "Test");
      break;
    case 0x02:
      chprintf(stream, "Setup");
      break;
    case 0x03:
      chprintf(stream, "Standby");
      break;
    default:
      chprintf(stream, "%x", event->msg.deviceStarted.operatingMode);
      break;
    }
    chprintf(stream, "\r\n");

    chprintf(stream, "Hardware error: ");
    switch (event->msg.deviceStarted.hwError) {
    case 0x00:
      chprintf(stream, "No error");
      break;
    case 0x01:
      chprintf(stream, "Fatal error");
      break;
    default:
      chprintf(stream, "%x", event->msg.deviceStarted.hwError);
      break;
    }
    chprintf(stream, "\r\n");

    chprintf(stream, "DataCreditAvailable: ");
    chprintf(stream, "%d\r\n", event->msg.deviceStarted.dataCreditAvailable);
    break;

  case NRF_ECHOEVENT:
    chprintf(stream, "EchoEvent\r\n");
    break;

  case NRF_HARDWAREERROREVENT:
    chprintf(stream, "HardwareErrorEvent!  Line: %d, File: %s\r\n",
          event->msg.hardwareError.lineNo,
          (char *)event->msg.hardwareError.fileName);
    break;

  case NRF_COMMANDRESPONSEEVENT:
    chprintf(stream, "CommandResponseEvent\r\n");

    chprintf(stream, "Status: ");
    switch (event->msg.commandResponse.status) {
    case NRF_STATUS_SUCCESS:
      chprintf(stream, "Success\r\n");
      break;
    case NRF_STATUS_TRANSACTION_CONTINUE:
      chprintf(stream, "Transaction continuation status\r\n");
      break;
    case NRF_STATUS_TRANSACTION_COMPLETE:
      chprintf(stream, "Transaction completed\r\n");
      break;
    case NRF_STATUS_EXTENDED:
      chprintf(stream, "Extended status, further checks needed\r\n");
      break;
    case NRF_STATUS_ERROR_UNKNOWN:
      chprintf(stream, "Unknown error\r\n");
      break;
    case NRF_STATUS_ERROR_INTERNAL:
      chprintf(stream, "Internal error\r\n");
      break;
    case NRF_STATUS_ERROR_CMD_UNKNOWN:
      chprintf(stream, "Unknown command\r\n");
      break;
    case NRF_STATUS_ERROR_DEVICE_STATE_INVALID:
      chprintf(stream, "Command invalid in the current device state\r\n");
      break;
    case NRF_STATUS_ERROR_INVALID_LENGTH:
      chprintf(stream, "Invalid length\r\n");
      break;
    case NRF_STATUS_ERROR_INVALID_PARAMETER:
      chprintf(stream, "Invalid input parameters\r\n");
      break;
    case NRF_STATUS_ERROR_BUSY:
      chprintf(stream, "Busy\r\n");
      break;
    case NRF_STATUS_ERROR_INVALID_DATA:
      chprintf(stream, "Invalid data format or contents\r\n");
      break;
    case NRF_STATUS_ERROR_CRC_MISMATCH:
      chprintf(stream, "CRC mismatch\r\n");
      break;
    case NRF_STATUS_ERROR_UNSUPPORTED_SETUP_FORMAT:
      chprintf(stream, "Unsupported setup format\r\n");
      break;
    case NRF_STATUS_ERROR_INVALID_SEQ_NO:
      chprintf(stream, 
               "Invalid sequence number during "
               "a write dynamic data sequence\r\n");
      break;
    case NRF_STATUS_ERROR_SETUP_LOCKED:
    chprintf(stream, 
              "Setup data is locked and cannot be modified\r\n");
      break;
    case NRF_STATUS_ERROR_LOCK_FAILED:
      chprintf(stream, 
              "Setup error due to lock verification failure\r\n");
      break;
    case NRF_STATUS_ERROR_BOND_REQUIRED:
      chprintf(stream, 
              "Bond required: Local service pipes need "
              "bonded/trusted peer\r\n");
      break;
    case NRF_STATUS_ERROR_REJECTED:
      chprintf(stream, 
              "Command rejected as a transaction is still pending\r\n");
      break;
    case NRF_STATUS_ERROR_DATA_SIZE:
      chprintf(stream, 
              "Pipe Error Event: Data size exceeds size specified "
              "for pipe, Transmit failed\r\n");
      break;
    case NRF_STATUS_ERROR_PIPE_INVALID:
      chprintf(stream, 
              "Pipe Error Event: Transmit failed, "
              "Invalid or unavailable Pipe number or "
              "unknown pipe type\r\n");
    break;
    case NRF_STATUS_ERROR_CREDIT_NOT_AVAILABLE:
      chprintf(stream, 
              "Pipe Error Event: Credit not available\r\n");
      break;
    case NRF_STATUS_ERROR_PEER_ATT_ERROR:
      chprintf(stream, 
              "Pipe Error Event: Peer device has sent an error on "
              "an pipe operation on the remote characteristic\r\n");
      break;
    case NRF_STATUS_ERROR_ADVT_TIMEOUT:
      chprintf(stream, 
              "Connection was not established before the BTLE "
              "advertising was stopped\r\n");
      break;
    case NRF_STATUS_ERROR_PEER_SMP_ERROR:
      chprintf(stream, 
              "Remote device triggered a Security Manager Protocol "
              "error\r\n");
      break;
    case NRF_STATUS_ERROR_PIPE_TYPE_INVALID:
      chprintf(stream, 
              "Pipe Error Event: Pipe type invalid for the "
              "selected operation\r\n");
      break;
    case NRF_STATUS_ERROR_PIPE_STATE_INVALID:
      chprintf(stream, "Pipe Error Event: Pipe state invalid "
              "for the selected operation\r\n");
      break;
    case NRF_STATUS_ERROR_INVALID_KEY_SIZE:
      chprintf(stream, "Invalid key size provided\r\n");
      break;
    case NRF_STATUS_ERROR_INVALID_KEY_DATA:
      chprintf(stream, "Invalid key data provided\r\n");
      break;
    default:
      chprintf(stream, "%x\r\n", event->msg.commandResponse.status);
      break;
    }

    chprintf(stream, "Op: ");
    switch (event->msg.commandResponse.opcode) {
    case NRF_TEST_OP:
      chprintf(stream, "Test\r\n");
      break;
    case NRF_GETDEVICEVERSION_OP:
      chprintf(stream, "GetDeviceVersion\r\n");
      chprintf(stream, "ConfigurationId=%d",
          event->msg.commandResponse.data.getDeviceVersion.configurationId);
      chprintf(stream, " ACIVersion=%d",
          event->msg.commandResponse.data.getDeviceVersion.aciVersion);
      chprintf(stream, " SetupFormat=%d",
          event->msg.commandResponse.data.getDeviceVersion.setupFormat);
      chprintf(stream, " SetupID=%d",
          event->msg.commandResponse.data.getDeviceVersion.setupId);
      chprintf(stream, " ConfigurationStatus=Setup%s\r\n",
        (event->msg.commandResponse.data.getDeviceVersion.configurationStatus == 1) ? "Locked" : "Open");
      break;
    case NRF_WAKEUP_OP:
      chprintf(stream, "Wakeup\r\n");
      break;
    case NRF_GETBATTERYLEVEL_OP:
      chprintf(stream, "GetBatteryLevel: %d mV\r\n",
              event->msg.commandResponse.data.voltage*3.52, 2);
      break;
    case NRF_GETTEMPERATURE_OP:
      chprintf(stream, "GetTemperature: %d C\r\n",
        event->msg.commandResponse.data.temperature/4.0, 2);
      break;
    case NRF_SETUP_OP:
      chprintf(stream, "Setup\r\n");
      break;
    case NRF_SETTXPOWER_OP:
      chprintf(stream, "SetTxPower\r\n");
      break;
    case NRF_GETDEVICEADDRESS_OP:
      chprintf(stream, "GetDeviceAddress\r\n");
      chprintf(stream, "Address: ");
      ble_print_address(event->msg.commandResponse.data.getDeviceAddress.deviceAddress);
      chprintf(stream, "\r\n");
      break;
    case NRF_CONNECT_OP:
      chprintf(stream, "Connect\r\n");
      break;
    case NRF_RADIORESET_OP:
      chprintf(stream, "RadioReset\r\n");
      break;
    case NRF_BOND_OP:
      chprintf(stream, "Bond\r\n");
      break;
    case NRF_DISCONNECT_OP:
      chprintf(stream, "Disconnect\r\n");
      break;
    case NRF_CHANGETIMINGREQUEST_OP:
      chprintf(stream, "ChangeTimingRequest\r\n");
      break;
    case NRF_OPENREMOTEPIPE_OP:
      chprintf(stream, "OpenRemotePipe\r\n");
      break;
    case NRF_CLOSEREMOTEPIPE_OP:
      chprintf(stream, "CloseRemotePipe\r\n");
      break;
    case NRF_DTMCOMMAND_OP:
      chprintf(stream, "DtmCommand\r\n");
      chprintf(stream, "DTM data: 0%x\r\n",
                event->msg.commandResponse.data.dtmEvent);
      break;
    case NRF_READDYNAMICDATA_OP:
      chprintf(stream, "ReadDynamicData\r\n");
      chprintf(stream, "Sequence no=%d\r\n",
        event->msg.commandResponse.data.readDynamicData.sequenceNo);
      chprintf(stream, "TODO: data here\r\n");
      break;
    case NRF_WRITEDYNAMICDATA_OP:
      chprintf(stream, "WriteDynamicData\r\n");
      break;
    case NRF_SETAPPLICATIONLATENCY_OP:
      chprintf(stream, "SetApplLatency\r\n");
      break;
    case NRF_SETKEY_OP:
      chprintf(stream, "SetKey\r\n");
      break;
    case NRF_OPENADVPIPE_OP:
      chprintf(stream, "OpenAdvPipe\r\n");
      break;
    case NRF_BROADCAST_OP:
      chprintf(stream, "Broadcast\r\n");
      break;
    case NRF_BONDSECREQUEST_OP:
      chprintf(stream, "BondSecurityRequest\r\n");
      break;
    case NRF_DIRECTEDCONNECT_OP:
      chprintf(stream, "DirectedConnect\r\n");
      break;
    case NRF_SETLOCALDATA_OP:
      chprintf(stream, "SetLocalData\r\n");
      break;
    default:
      chprintf(stream, "%02x\r\n", event->msg.commandResponse.opcode);
      break;
    }
    break;
  case NRF_CONNECTEDEVENT:
    chprintf(stream, "ConnectedEvent\r\n");
    chprintf(stream, "Address type: ");
    switch (event->msg.connected.addressType) {
    case 0x01:
      chprintf(stream, "Public address\r\n");
      break;
    case 0x02:
      chprintf(stream, "Random static address\r\n");
      break;
    case 0x03:
      chprintf(stream, "Random private address (resolvable)\r\n");
      break;
    case 0x04:
      chprintf(stream, "Random private adddress (non-resolvable)\r\n");
      break;
    default:
      chprintf(stream, "0x%x\r\n", event->msg.connected.addressType);
      break;
    }

    chprintf(stream, "Peer address: ");
    ble_print_address(event->msg.connected.peerAddress);
    chprintf(stream, "\r\n");

    chprintf(stream, "Connection interval: %d ms\r\n",
          event->msg.connected.connectionInterval/1.25);

    chprintf(stream, "Slave latency: %d\r\n",
          event->msg.connected.slaveLatency);

    chprintf(stream, "Supervision timeout: %d\r\n",
          event->msg.connected.supervisionTimeout);

    chprintf(stream, "Master clock accuracy: ");
    switch (event->msg.connected.masterClockAccuracy) {
    case 0x00:
      chprintf(stream, "500 ppm\r\n");
      break;
    case 0x01:
      chprintf(stream, "250 ppm\r\n");
      break;
    case 0x02:
      chprintf(stream, "150 ppm\r\n");
      break;
    case 0x03:
      chprintf(stream, "100 ppm\r\n");
      break;
    case 0x04:
      chprintf(stream, "75 ppm\r\n");
      break;
    case 0x05:
      chprintf(stream, "50 ppm\r\n");
      break;
    case 0x06:
      chprintf(stream, "30 ppm\r\n");
      break;
    case 0x07:
      chprintf(stream, "20 ppm\r\n");
      break;
    default:
      chprintf(stream, "%d\r\n", event->msg.connected.masterClockAccuracy);
      break;
    }
    break;
  case NRF_DISCONNECTEDEVENT: {
    chprintf(stream, "DisconnectedEvent\r\n");

    chprintf(stream, "ACI status: ");
    switch (event->msg.disconnected.aciStatus) {
    case 0x03:
      chprintf(stream, "Check the Bluetooth low energy status code\r\n");
      break;
    case 0x93:
      chprintf(stream, "Timeout while advertising, "
                       "unable to establish connection\r\n");
      break;
    case 0x8d:
      chprintf(stream, "Bond required to proceed with connection\r\n");
      break;
    default:
      chprintf(stream, "0x%02x\r\n", event->msg.disconnected.aciStatus);
      break;
    }

    chprintf(stream, "BtLeStatus: 0x%02x\r\n", 
      event->msg.disconnected.btLeStatus);
    break;
  } 
  case NRF_BONDSTATUSEVENT:
    chprintf(stream, "BondStatusEvent\r\n");
    chprintf(stream, "TODO: bond status data\r\n");
    break;
  case NRF_PIPESTATUSEVENT:
    chprintf(stream, "PipeStatusEvent\r\n");
          
    chprintf(stream, "Open: ");
    for (int i = 0; i < 64; i++) {
      if (event->msg.pipeStatus.pipesOpen & ((uint64_t)1)<<i) {
        chprintf(stream, " %d", i);
      }
    }
    chprintf(stream, "\r\n");

    chprintf(stream, "Closed: ");
    for (int i = 0; i < 64; i++) {
      if (event->msg.pipeStatus.pipesClosed & ((uint64_t)1)<<i) {
        chprintf(stream, " %d", i);
      }
    }
    chprintf(stream, "\r\n");
    break;
  case NRF_TIMINGEVENT:
    chprintf(stream, "TimingEvent\r\n");

    chprintf(stream, "Connection interval: %d\r\n",
            event->msg.timing.connectionInterval/1.25);

    chprintf(stream, "Slave latency: %d\r\n",
            event->msg.timing.slaveLatency, 2);

    chprintf(stream, "Supervision timeout: %d ms\r\n",
            event->msg.timing.supervisionTimeout*10.0);
    break;
  case NRF_DISPLAYKEYEVENT:
    chprintf(stream, "DisplayKeyEvent\r\n");
    chprintf(stream, "Passkey: \r\n");
    for (int i = 0; i < NRF_PASSKEY_LENGTH; i++) {
      chprintf(stream, "%02x", event->msg.passkey[i]);
    }
    chprintf(stream, "\r\n");
    break;
  case NRF_KEYREQUESTEVENT:
    chprintf(stream, "KeyRequestEvent\r\n");
    break;
  case NRF_DATACREDITEVENT:
    chprintf(stream, "DataCreditEvent credits=%d\r\n",
            event->msg.dataCredits);
    break;
  case NRF_PIPEERROREVENT:
    chprintf(stream, "PipeErrorEvent\r\n");
    chprintf(stream, "Pipe: %d\r\n",
            event->msg.pipeError.servicePipeNo);

    chprintf(stream, "Error code: 0x%02x\r\n",
            event->msg.pipeError.errorCode);
    break;
  case NRF_DATARECEIVEDEVENT:
    chprintf(stream, "DataReceivedEvent\r\n");
    chprintf(stream, "Pipe: %d\r\n",
            event->msg.dataReceived.servicePipeNo);
    break;
  case NRF_DATAACKEVENT:
    chprintf(stream, "DataAckEvent\r\n");
    chprintf(stream, "Pipe: %d\r\n", event->msg.servicePipeNo);
    break;
  default:
    chprintf(stream, "Unknown %d\r\n", event->event);
    break;
  }
}

static nRFTxStatus ble_handle_packet(BLEDevice *ble, nRFEvent *rxEvent)
{

  chprintf(stream, "Got event (%d bytes):\r\n", rxEvent->length);
  print_hex(stream, rxEvent, rxEvent->length + 2, 0);
  bleDebugEvent(ble, rxEvent);

  // Return immediately if we didn't receive anything
  if (!rxEvent->length)
    return Success;

  // Handle response
  switch (rxEvent->event) {
  case NRF_DEVICESTARTEDEVENT:
    ble->credits = rxEvent->msg.deviceStarted.dataCreditAvailable;

    switch (rxEvent->msg.deviceStarted.operatingMode) {
    case 0x01:
      ble->device_state = Test;
      break;
    case 0x02:
      ble->device_state = nRFSetupState;
      break;
    case 0x03:
      if (ble->device_state == Initial)
        // Before we are setup, Nordic calls it "Standby",
        // but that's different from post-setup Standby
        ble->device_state = PreSetup;
      else
        ble->device_state = Standby;
    }
    break;
  case NRF_COMMANDRESPONSEEVENT: {
    if (rxEvent->msg.commandResponse.status != 0x00) {
#if NRF_DEBUG
      chprintf(stream, "non-success command response event: 0x%02x\r\n",
               rxEvent->msg.commandResponse.status);
#endif
      if (ble->command_response_handler)
        ble->command_response_handler(
                        ble,
                        rxEvent->msg.commandResponse.opcode,
                        rxEvent->msg.commandResponse.status);

      if (rxEvent->msg.commandResponse.opcode == NRF_SETUP_OP
       && rxEvent->msg.commandResponse.status == NRF_STATUS_TRANSACTION_CONTINUE) {
        ble->svc_msg_num++;
        nrf_debug("ready for next setup message");
      }
      else if (rxEvent->msg.commandResponse.opcode == NRF_SETUP_OP
            && rxEvent->msg.commandResponse.status == NRF_STATUS_TRANSACTION_COMPLETE) {
        nrf_debug("setup complete");
        ble->svc_msg_num = -1;
        ble->device_state = Standby;
      }
      break;
    }

    switch (rxEvent->msg.commandResponse.opcode) {
    // We only do handling of some of these messages
    case NRF_SETUP_OP:
      break;
    case NRF_GETTEMPERATURE_OP:
      if (ble->temperature_handler)
        ble->temperature_handler(ble,
                rxEvent->msg.commandResponse.data.temperature / 4.0);
      break;
    case NRF_GETBATTERYLEVEL_OP:
      if (ble->battery_level_handler)
        ble->battery_level_handler(ble,
            rxEvent->msg.commandResponse.data.voltage * 0.00352);
      break;
    case NRF_GETDEVICEVERSION_OP:
      if (ble->device_version_handler)
        ble->device_version_handler(ble,
        rxEvent->msg.commandResponse.data.getDeviceVersion.configurationId,
        rxEvent->msg.commandResponse.data.getDeviceVersion.aciVersion,
        rxEvent->msg.commandResponse.data.getDeviceVersion.setupFormat,
        rxEvent->msg.commandResponse.data.getDeviceVersion.configurationStatus);
      break;
    case NRF_GETDEVICEADDRESS_OP:
      if (ble->device_address_handler)
        ble->device_address_handler(ble,
          rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress,
          rxEvent->msg.commandResponse.data.getDeviceAddress.addressType);
      break;
    case NRF_READDYNAMICDATA_OP:
      if (ble->dynamic_data_handler)
        ble->dynamic_data_handler(ble,
          rxEvent->msg.commandResponse.data.readDynamicData.sequenceNo,
          rxEvent->msg.commandResponse.data.readDynamicData.dynamicData);
      break;
    default:
      if (ble->command_response_handler)
        ble->command_response_handler(ble,
          rxEvent->msg.commandResponse.opcode,
          rxEvent->msg.commandResponse.status);
      break;
    }

    // Dispatch event
    break;
  }
  case NRF_CONNECTEDEVENT:
    ble->connection_status = Connected;
    if (ble->connected_handler)
      ble->connected_handler(ble,
                  rxEvent->msg.connected.addressType,
                  rxEvent->msg.connected.peerAddress,
                  0);
              // TODO: put in other data
    break;
  case NRF_DISCONNECTEDEVENT:
    ble->connection_status = Disconnected;
    ble->pipes_open = 0;
    if (ble->disconnected_handler)
      ble->disconnected_handler( ble,
                  rxEvent->msg.disconnected.aciStatus,
                  rxEvent->msg.disconnected.btLeStatus);
    break;
  case NRF_DATACREDITEVENT:
    ble->credits += rxEvent->msg.dataCredits;
    break;
  case NRF_PIPESTATUSEVENT:
    ble->pipes_open = rxEvent->msg.pipeStatus.pipesOpen;
    break;
  case NRF_BONDSTATUSEVENT:
    if (ble->bond_status_handler)
      ble->bond_status_handler(ble, 0);
      // TODO: put in bond status data
    break;
  case NRF_KEYREQUESTEVENT:
    if (ble->key_request_handler)
      ble->key_request_handler(ble, rxEvent->msg.keyType);
    break;
  case NRF_PIPEERROREVENT:
    if (ble->pipe_error_handler)
      ble->pipe_error_handler( ble,
                  rxEvent->msg.pipeError.servicePipeNo,
                  rxEvent->msg.pipeError.errorCode,
                  rxEvent->msg.pipeError.rawData);
    break;
  case NRF_DATARECEIVEDEVENT:
    if (ble->data_received_handler)
      ble->data_received_handler(
                  ble,
                  rxEvent->msg.dataReceived.servicePipeNo,
                  rxEvent->msg.dataReceived.data);
    break;
  case NRF_DATAACKEVENT:
    if (ble->data_ack_handler)
      ble->data_ack_handler(ble, rxEvent->msg.servicePipeNo);
    break;
  case NRF_HARDWAREERROREVENT:
    break;
  default:
    break;
  }

  // Dispatch event
  if (ble->listener)
    // first to generic handler
    ble->listener(ble, rxEvent);

  return Success;
}

// Transmit a command, and simultaneously receive a message from nRF8001 if
// there is one. To just receive without transmitting anything, call this
// function with a NULL argument.
static nRFTxStatus ble_transmit_receive(BLEDevice *ble,
                                        nRFCommand *txCmd,
                                        uint16_t timeout)
{
  // Buffer that we will receive into
  uint8_t rxBuffer[sizeof(nRFEvent)];
  uint8_t txBuffer[sizeof(nRFEvent)];
  uint8_t txLength, txCommand;

  nRFEvent *rxEvent = (nRFEvent *)rxBuffer;

  memset(rxBuffer, 0, sizeof(rxBuffer));
  memset(txBuffer, 0, sizeof(txBuffer));

  // Transmit length
  if (txCmd) {
    txLength = txCmd->length;
    txCommand = txCmd->command;
    memcpy(txBuffer, txCmd, txLength + 1);
  }
  else {
    txLength = 0;
    txCommand = 0;
  }

  osalDbgAssert(txLength <= NRF_MAX_PACKET_LENGTH, "BLE packet too long");

  // Enough credits?
  if (txLength &&
     (txCmd->command == NRF_SENDDATA_OP
   || txCmd->command == NRF_REQUESTDATA_OP
   || txCmd->command == NRF_SENDDATAACK_OP
   || txCmd->command == NRF_SENDDATANACK_OP)) {
    if (ble->credits < 1) {
      nrf_debug("transmitReceive fail, not enough credits");
      return InsufficientCredits;
    }

    // Use a credit
    ble->credits--;
  }

  if (txLength > 0) {
    ble_select(ble);
#if NRF_VERBOSE_DEBUG
    chprintf(stream, "transmitReceive: called with transmission, "
            "command %d\r\n", txCommand);
    print_hex(stream, txBuffer, txLength + 1, 0);
#endif
  } 
#if NRF_VERBOSE_DEBUG
  else
    nrf_debug("transmitReceive: called in polling mode");
#endif
    
  // TODO: Timeout

  if (txLength > 0 || timeout == 0) {
    // Wait for RDYN low indefinitely
#if NRF_VERBOSE_DEBUG
    nrf_debug("waiting for RDYN low indefinitely");
#endif
    while (!ble_is_ready(ble));
  }
  else {
#if NRF_VERBOSE_DEBUG
    chprintf(stream, "polling for %d ms, waiting for BLE to be ready\r\n", timeout);
#endif
    int rdy = 0;
    int wait_periods;
    // Wait for RDYN low for 1 ms at a time
    for (wait_periods = 0; wait_periods < timeout; wait_periods++) {
      if (ble_is_ready(ble)) {
        rdy = 1;
        break;
      }
      else
        chThdSleepMilliseconds(1);
    }

    if (!rdy)
      return Timeout;
  }

  if (txLength == 0)
    ble_select(ble);

#if NRF_VERBOSE_DEBUG
  nrf_debug("RDYN is low, ready to transmit/receive");
#endif

  // Send length and command bytes,
  // receive debug and length bytes
  spiExchange(ble->spip, 2, txBuffer, rxBuffer);

  osalDbgAssert(rxEvent->length <= NRF_MAX_PACKET_LENGTH,
      "BLE Rx packet too long");

  // bytes_left points to the next byte to be transferred in
  // txBuffer, or the next byte to be received in rxBuffer.
  // Subtract the two bytes already transmitted/received.
  // For TX, packets are data + 1 byte (length)
  // For RX, packets are data + 2 bytes (debug and length)
  int bytes_left = rxEvent->length - 0;
  if (bytes_left < (txLength - 1))
    bytes_left = txLength - 1;

  chprintf(stream, "Finishing up transfer, have %d bytes left (%d, %d)\r\n", bytes_left, rxEvent->length, txLength);
  spiExchange(ble->spip, bytes_left, txBuffer + 2, rxBuffer + 2);

  // Bring REQN high
  ble_unselect(ble);

  // Wait for chip to acknowledge
  while (ble_is_ready(ble));

  return ble_handle_packet(ble, rxEvent);
}

// Informational functions

uint8_t bleCreditsAvailable(BLEDevice *ble)
{
  return ble->credits;
}

uint8_t bleIsConnected(BLEDevice *ble) {
  return ble->connection_status == Connected;
}

nRFConnectionStatus bleGetConnectionStatus(BLEDevice *ble)
{
  return ble->connection_status;
}

// Receive functions

nRFTxStatus blePoll(BLEDevice *ble, uint16_t timeout)
{
  return ble_transmit_receive(ble, 0, timeout);
}

uint8_t bleIsPipeOpen(BLEDevice *ble, nRFPipe servicePipeNo)
{
  return (ble->pipes_open & ((uint64_t)1) << servicePipeNo) != 0;
}

// Transmit functions

static nRFTxStatus ble_transmit_command(BLEDevice *ble, uint8_t command)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nrf_debug("calling transmitCommand");

  nRFCommand cmd;
  cmd.length = 1;
  cmd.command = command;
  return ble_transmit_receive(ble, &cmd, 0);
}

static nRFTxStatus ble_transmit_pipe_command(BLEDevice *ble, uint8_t command, nRFPipe pipe)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nrf_debug("calling transmitPipeCommand");

  nRFCommand cmd;
  cmd.length = 2;
  cmd.command = command;
  cmd.content.servicePipeNo = pipe;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleTest(BLEDevice *ble, uint8_t feature)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nrf_debug("calling test");

  nRFCommand cmd;
  cmd.length = 2;
  cmd.command = NRF_TEST_OP;
  cmd.content.testFeature = feature;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleSleep(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_SLEEP_OP);
}

nRFTxStatus bleEcho(BLEDevice *ble, nRFLen dataLength, uint8_t *data)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  if (dataLength > NRF_MAX_ECHO_MESSAGE_LENGTH) {
    nrf_debug("data too long");
    return DataTooLong;
  }

  nrf_debug("calling echo");

  nRFCommand cmd;
  cmd.length = dataLength + 1;
  cmd.command = NRF_ECHO_OP;
  memcpy(cmd.content.echoData, data, dataLength);
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleWakeup(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_WAKEUP_OP);
}

nRFTxStatus bleGetBatteryLevel(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_GETBATTERYLEVEL_OP);
}

nRFTxStatus bleGetTemperature(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_GETTEMPERATURE_OP);
}

nRFTxStatus bleSetTxPower(BLEDevice *ble, uint8_t powerLevel)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 2;
  cmd.command = NRF_SETTXPOWER_OP;
  cmd.content.radioTxPowerLevel = powerLevel;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleGetDeviceAddress(BLEDevice *ble)
{
    return ble_transmit_command(ble, NRF_GETDEVICEADDRESS_OP);
}

nRFTxStatus bleGetDeviceVersion(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_GETDEVICEVERSION_OP);
}

nRFTxStatus bleConnect(BLEDevice *ble, uint16_t timeout, uint16_t advInterval)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  ble->connection_status = Connecting;

  nrf_debug("connecting");

  nRFCommand cmd;
  cmd.length = 5;
  cmd.command = NRF_CONNECT_OP;
  cmd.content.connect.timeout = timeout;
  cmd.content.connect.advInterval = advInterval;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleRadioReset(BLEDevice *ble)
{
  nrf_debug("sending radio reset");
  nRFCommand cmd;
  cmd.length = 1;
  cmd.command = NRF_RADIORESET_OP;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleBond(BLEDevice *ble, uint16_t timeout, uint16_t advInterval)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 5;
  cmd.command = NRF_BOND_OP;
  cmd.content.bond.timeout = timeout;
  cmd.content.bond.advInterval = advInterval;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleDisconnect(BLEDevice *ble, uint8_t reason)
{
  if (ble->device_state != Standby || ble->connection_status != Connected) {
    nrf_debug("device not in standby and connected state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 2;
  cmd.command = NRF_DISCONNECT_OP;
  cmd.content.disconnectReason = reason;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleChangeTimingRequest(BLEDevice *ble, uint16_t intervalMin,
                                   uint16_t intervalMax,
                                   uint16_t slaveLatency,
                                   uint16_t timeout)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.command = NRF_CHANGETIMINGREQUEST_OP;

  if (intervalMin > 0 || intervalMax > 0
   || slaveLatency > 0 || timeout >> 0) {
    cmd.length = 9;
    cmd.content.changeTimingRequest.intervalMin = intervalMin;
    cmd.content.changeTimingRequest.intervalMax = intervalMax;
    cmd.content.changeTimingRequest.slaveLatency = slaveLatency;
    cmd.content.changeTimingRequest.timeout = timeout;
  }
  else
      cmd.length = 1;

  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleOpenRemotePipe(BLEDevice *ble, nRFPipe servicePipeNo)
{
  return ble_transmit_pipe_command(ble, NRF_OPENREMOTEPIPE_OP, servicePipeNo);
}

nRFTxStatus bleCloseRemotePipe(BLEDevice *ble, nRFPipe servicePipeNo)
{
  return ble_transmit_pipe_command(ble, NRF_CLOSEREMOTEPIPE_OP, servicePipeNo);
}

nRFTxStatus bleDtmCommand(BLEDevice *ble, uint16_t dtmCmd)
{
  nRFCommand cmd;
  cmd.length = 3;
  cmd.command = NRF_DTMCOMMAND_OP;
  cmd.content.dtmCommand = dtmCmd;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleReadDynamicData(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_READDYNAMICDATA_OP);
}

nRFTxStatus bleWriteDynamicData(BLEDevice *ble, uint8_t seqNo,
                                nRFLen dataLength, uint8_t *data)
{
    if (ble->device_state != Standby) {
        nrf_debug("device not in Standby state");
        return InvalidState;
    }

    nRFCommand cmd;
    cmd.length = dataLength + 2;
    cmd.command = NRF_WRITEDYNAMICDATA_OP;
    cmd.content.writeDynamicData.sequenceNo = seqNo;
    memcpy(cmd.content.writeDynamicData.dynamicData, data, dataLength);
    return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleSetApplLatency(BLEDevice *ble, uint8_t applLatencyMode,
                              uint16_t latency)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 4;
  cmd.command = NRF_SETAPPLICATIONLATENCY_OP;
  cmd.content.setApplLatency.applLatencyMode = applLatencyMode;
  cmd.content.setApplLatency.latency = latency;
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleSetKey(BLEDevice *ble, uint8_t keyType, uint8_t *key)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.command = NRF_SETKEY_OP;
  cmd.content.setKey.keyType = keyType;

  if (keyType == 0)
    cmd.length = 2;
  else if (keyType == 1) {
    cmd.length = 2 + NRF_PASSKEY_LENGTH;
    memcpy(cmd.content.setKey.key, key, NRF_PASSKEY_LENGTH);
  }
  else
    return InvalidParameter;

  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleOpenAdvPipe(BLEDevice *ble, uint64_t advServiceDataPipes)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 9;
  cmd.command = NRF_OPENADVPIPE_OP;
  cmd.content.advServiceDataPipes = advServiceDataPipes;

  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleBroadcast(BLEDevice *ble, uint16_t timeout, uint16_t advInterval)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 5;
  cmd.command = NRF_BROADCAST_OP;
  cmd.content.broadcast.timeout = timeout;
  cmd.content.broadcast.advInterval = advInterval;

  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleBondSecurityRequest(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_BONDSECREQUEST_OP);
}

nRFTxStatus bleDirectedConnect(BLEDevice *ble)
{
  return ble_transmit_command(ble, NRF_DIRECTEDCONNECT_OP);
}

nRFTxStatus bleSendData(BLEDevice *ble, nRFPipe servicePipeNo,
    nRFLen dataLength, uint8_t *data)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  if (ble->connection_status != Connected) {
    nrf_debug("device not connected");
    return NotConnected;
  }

  if (!(ble->pipes_open & ((uint64_t)1)<<servicePipeNo)) {
    nrf_debug("pipe not open");
    return PipeNotOpen;
  }

  if (ble->credits <= 0) {
    nrf_debug("not enough credits");
    return InsufficientCredits;
  }

  if (dataLength > NRF_DATA_LENGTH) {
    nrf_debug("data too long");
    return DataTooLong;
  }

  nRFCommand cmd;
  cmd.command = NRF_SENDDATA_OP;
  cmd.length = dataLength + 2;
  cmd.content.data.servicePipeNo = servicePipeNo;
  memcpy(cmd.content.data.data, data, dataLength);
  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleRequestData(BLEDevice *ble, nRFPipe servicePipeNo)
{
  return ble_transmit_pipe_command(ble, NRF_REQUESTDATA_OP, servicePipeNo);
}

nRFTxStatus bleSetLocalData(BLEDevice *ble, nRFPipe servicePipeNo,
                            nRFLen dataLength, uint8_t *data)
{
  if (ble->device_state != Standby) {
    nrf_debug("device not in Standby state");
    return InvalidState;
  }

  nRFCommand cmd;
  cmd.length = 2 + dataLength;
  cmd.command = NRF_SETLOCALDATA_OP;
  cmd.content.data.servicePipeNo = servicePipeNo;

  memcpy(cmd.content.data.data, data, dataLength);

  return ble_transmit_receive(ble, &cmd, 0);
}

nRFTxStatus bleSendDataAck(BLEDevice *ble, nRFPipe servicePipeNo)
{
    return ble_transmit_pipe_command(ble, NRF_SENDDATAACK_OP, servicePipeNo);
}

nRFTxStatus bleSendDataNack(BLEDevice *ble, nRFPipe servicePipeNo,
                            uint8_t errorCode)
{
    if (ble->device_state != Standby) {
        nrf_debug("device not in Standby state");
        return InvalidState;
    }

    nRFCommand cmd;
    cmd.length = 3;
    cmd.command = NRF_SENDDATANACK_OP;
    cmd.content.sendDataNack.servicePipeNo = servicePipeNo;
    cmd.content.sendDataNack.errorCode = errorCode;

    return ble_transmit_receive(ble, &cmd, 0);
}

// Event handler registration

void bleSetEventHandler(BLEDevice *ble, nRFEventHandler handler)
{
    ble->listener = handler;
}

void bleSetCommandResponseHandler(BLEDevice *ble, nRFCommandResponseHandler handler)
{
    ble->command_response_handler = handler;
}

void bleSetTemperatureHandler(BLEDevice *ble, nRFTemperatureHandler handler)
{
    ble->temperature_handler = handler;
}

void bleSetBatteryLevelHandler(BLEDevice *ble, nRFBatteryLevelHandler handler)
{
    ble->battery_level_handler = handler;
}

void bleSetDeviceVersionHandler(BLEDevice *ble, nRFDeviceVersionHandler handler)
{
    ble->device_version_handler = handler;
}

void bleSetDeviceAddressHandler(BLEDevice *ble, nRFDeviceAddressHandler handler)
{
    ble->device_address_handler = handler;
}

void bleSetDynamicDataHandler(BLEDevice *ble, nRFDynamicDataHandler handler)
{
    ble->dynamic_data_handler = handler;
}

void bleSetConnectedHandler(BLEDevice *ble, nRFConnectedHandler handler)
{
    ble->connected_handler = handler;
}

void bleSetDisconnectedHandler(BLEDevice *ble, nRFDisconnectedHandler handler)
{
    ble->disconnected_handler = handler;
}

void bleSetBondStatusHandler(BLEDevice *ble, nRFBondStatusHandler handler)
{
    ble->bond_status_handler = handler;
}

void bleSetKeyRequestHandler(BLEDevice *ble, nRFKeyRequestHandler handler)
{
    ble->key_request_handler = handler;
}

void bleSetPipeErrorHandler(BLEDevice *ble, nRFPipeErrorHandler handler)
{
    ble->pipe_error_handler = handler;
}

void bleSetDataReceivedHandler(BLEDevice *ble, nRFDataReceivedHandler handler)
{
    ble->data_received_handler = handler;
}

void bleSetDataAckHandler(BLEDevice *ble, nRFDataAckHandler handler)
{
    ble->data_ack_handler = handler;
}
