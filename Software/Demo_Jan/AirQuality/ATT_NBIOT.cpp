/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2018 AllThingsTalk
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ATT_NBIOT.h"
#include "Sodaq_wdt.h"
#include "keys.h"  // Device credentials

//#define DEBUG  // We want debug info

// Response definitions
#define STR_AT "AT"
#define STR_RESPONSE_OK "OK"
#define STR_RESPONSE_ERROR "ERROR"
#define STR_RESPONSE_CME_ERROR "+CME ERROR:"
#define STR_RESPONSE_CMS_ERROR "+CMS ERROR:"

#define DEBUG_STR_ERROR "[ERROR]: "

// Convert chars to hex
#define NIBBLE_TO_HEX_CHAR(i) ((i <= 9) ? ('0' + i) : ('A' - 10 + i))
#define HIGH_NIBBLE(i) ((i >> 4) & 0x0F)
#define LOW_NIBBLE(i) (i & 0x0F)

// Debug stream
#ifdef DEBUG
#define debugPrintLn(...) { if (!this->_disableDiag && this->_diagStream) this->_diagStream->println(__VA_ARGS__); }
#define debugPrint(...) { if (!this->_disableDiag && this->_diagStream) this->_diagStream->print(__VA_ARGS__); }
#warning "Debug mode is ON"
#else
#define debugPrintLn(...)
#define debugPrint(...)
#endif

//
#define DEFAULT_CID "1"

// Socket response
#define SOCKET_FAIL -1
#define SOCKET_COUNT 7

#define NOW (uint32_t)millis()

typedef struct NameValuePair {
    const char* Name;
    const char* Value;
} NameValuePair;

const uint8_t nConfigCount = 3; //6
static NameValuePair nConfig[nConfigCount] = {
  { "AUTOCONNECT", "TRUE" },
  { "CR_0354_0338_SCRAMBLING", "TRUE" },
  { "CR_0859_SI_AVOID", "TRUE" },
  //{ "COMBINE_ATTACH" , "FALSE" },
  //{ "CELL_RESELECTION" , "FALSE" },
  //{ "ENABLE_BIP" , "FALSE" },
};

class Sodaq_nbIotOnOff : public Sodaq_OnOffBee
{
  public:
    Sodaq_nbIotOnOff();
    void init(int onoffPin);
    void on();
    void off();
    bool isOn();
  private:
    int8_t _onoffPin;
    bool _onoff_status;
};

static Sodaq_nbIotOnOff sodaq_nbIotOnOff;

static inline bool is_timedout(uint32_t from, uint32_t nr_ms) __attribute__((always_inline));
static inline bool is_timedout(uint32_t from, uint32_t nr_ms)
{
  return (millis() - from) > nr_ms;
}

/****
 * Constructor
 */
 
ATT_NBIOT::ATT_NBIOT() : _lastRSSI(0), _CSQtime(0), _minRSSI(-113) // dBm
{
  // Get credentials from keys.h file
  _deviceId = DEVICE_ID;
  _deviceToken = DEVICE_TOKEN;
  _apn = APN;
}
 
ATT_NBIOT::ATT_NBIOT(const char* deviceId, const char* deviceToken) : _lastRSSI(0), _CSQtime(0), _minRSSI(-113) // dBm
{
  // Get credentials from keys.h file
  _deviceId = deviceId;
  _deviceToken = deviceToken;
  _apn = APN;
}

/****
 * Returns true if the modem replies to "AT" commands without timing out
 */
bool ATT_NBIOT::isAlive()
{
  _disableDiag = true;
  println(STR_AT);
  
  return (readResponse(NULL, 450) == ResponseOK);
}

/***
 * Manually set or override the credentials from the keys.h file
 */
void ATT_NBIOT::setAttDevice(const char* deviceId, const char* deviceToken, const char* apn)
{
  _deviceId = deviceId;
  _deviceToken = deviceToken;
  _apn = apn;
}

/****
 * Initializes the modem instance
 * Sets the modem stream and the on-off power pins
 */
void ATT_NBIOT::init(Stream& stream, Stream& debug, int8_t onoffPin)
{
  debugPrintLn("[init] started.");

  initBuffer();  // Safe to call multiple times

  setModemStream(stream);
  //setDiag(debug);

  sodaq_nbIotOnOff.init(onoffPin);
  _onoff = &sodaq_nbIotOnOff;
}

/****
 *
 */
bool ATT_NBIOT::setRadioActive(bool on)
{
  print("AT+CFUN=");
  println(on ? "1" : "0");

  return (readResponse() == ResponseOK);
}

/****
 * Set the network url
 */
bool ATT_NBIOT::setApn(const char* apn)
{
    print("AT+CGDCONT=" DEFAULT_CID ",\"IP\",\"");
    print(apn);
    println("\"");

    return (readResponse() == ResponseOK);
}

void ATT_NBIOT::purgeAllResponsesRead()
{
    uint32_t start = millis();

    // Make sure all the responses within the timeout have been read
    while ((readResponse(0, 1000) != ResponseTimeout) && !is_timedout(start, 2000)) {}
}

/****
 * Connect and activate data connection
 *  1. Turn on the modem
 *  2. Turn off the radio
 *  3. Apply configuration
 *  4. Reboot
 *  5. Turn on the modem
 *  6. Set apn
 *  7. Turn on the radio
 *  8. [optional] Set/force the operator
 *  9. Check signal quality
 * 10. Connect the bee
 * 11. Create DGRAM socket
 *  Success!
 */
bool ATT_NBIOT::connect()
{
  // nb-iot network
  //const char* apn = "iot.orange.be";
  const char* forceOperator;  // "20610" for Orange Belgium

  // AllThingsTalk endpoint
  _udp = "40.68.172.187";
  _port = "8891";
  
  if(!on())
    return false;

  purgeAllResponsesRead();

  if(!setRadioActive(false))
    return false;

  if(!checkAndApplyNconfig())
    return false;

  reboot();

  if(!on())
    return false;

  purgeAllResponsesRead();

  if(!setApn(_apn))
    return false;

  if(!setRadioActive(true))
    return false;

  if(forceOperator && forceOperator[0] != '\0')
  {
    if(!setOperator(forceOperator))
      return false;
  }
  else if(!setOperator())
    return false;

  if(!waitForSignalQuality())
    return false;

  if(!attachBee())
    return false;
  
  if(createSocket(3000) == -1)  // Create a DGRAM socket
    return false;
  
  delay(50);
  
  // If we got this far we succeeded
  return true;
}

/****
 * Reboot the bee
 */
void ATT_NBIOT::reboot()
{
  println("AT+NRB");

  // Wait up to 2000ms for the modem to come up
  uint32_t start = millis();
  while ((readResponse() != ResponseOK) && !is_timedout(start, 2000)) { }
}

/****
 * Retrieve and apply modem configuration
 */
bool ATT_NBIOT::checkAndApplyNconfig()
{
  bool applyParam[nConfigCount];

  println("AT+NCONFIG?");

  if (readResponse<bool, uint8_t>(_nconfigParser, applyParam, NULL) == ResponseOK)
  {
    for (uint8_t i = 0; i < nConfigCount; i++)
    {
      debugPrint(nConfig[i].Name);
      if (!applyParam[i])
      {
        debugPrintLn("... CHANGE");
        setNconfigParam(nConfig[i].Name, nConfig[i].Value);
      }
      else
      {
        debugPrintLn("... OK");
      }
    }
    return true;
  }
  return false;
}

/****
 * Set a forced operator
 */
bool ATT_NBIOT::setOperator(const char* forceOperator)
{
  print("AT+COPS=1,2,\"");
  print(forceOperator);
  println("\"");

  return readResponse() == ResponseOK;
}

bool ATT_NBIOT::setOperator()
{
  println("AT+COPS=0");
  
  return readResponse() == ResponseOK;
}

/****
 * Set socket
 */
int ATT_NBIOT::createSocket(uint16_t localPort)
{
  // Only Datagram/UDP is supported
  print("AT+NSOCR=\"DGRAM\",17,");
  print(localPort);
  println(",1");  // Enable incoming message URC (NSONMI)

  uint8_t socket;

  if (readResponse<uint8_t, uint8_t>(_createSocketParser, &socket, NULL) == ResponseOK)
    return socket;

  return SOCKET_FAIL;
}

/****
 * Set a specific parameter
 */
bool ATT_NBIOT::setNconfigParam(const char* param, const char* value)
{
  print("AT+NCONFIG=\"");
  print(param);
  print("\",\"");
  print(value);
  println("\"");
  
  return readResponse() == ResponseOK;
}

/****
 * Connect the bee
 */
bool ATT_NBIOT::attachBee(uint32_t timeout)
{
  uint32_t start = millis();
  uint32_t delay_count = 500;

  while (!is_timedout(start, timeout))
  {
    if (isConnected())
      return true;

    sodaq_wdt_safe_delay(delay_count);

    // Next time wait a little longer, but not longer than 5 seconds
    if (delay_count < 5000)
      delay_count += 1000;
  }
  return false;
}

/****
 * Disconnects the modem from the network
 */
bool ATT_NBIOT::disconnect()
{
  println("AT+CGATT=0");
  return (readResponse(NULL, 40000) == ResponseOK);
}

/****
 * Returns true if the modem is connected to the network and has an activated data connection
 */
bool ATT_NBIOT::isConnected()
{
  uint8_t value = 0;

  println("AT+CGATT?");
  if (readResponse<uint8_t, uint8_t>(_cgattParser, &value, NULL) == ResponseOK)
    return (value == 1);

  return false;
}

/****
 * Gets the Received Signal Strength Indication in dBm and Bit Error Rate.
 * Returns true if successful.
 */
bool ATT_NBIOT::getRSSIAndBER(int8_t* rssi, uint8_t* ber)
{
    static char berValues[] = { 49, 43, 37, 25, 19, 13, 7, 0 };  // 3GPP TS 45.008 [20] subclause 8.2.4

    println("AT+CSQ");

    int csqRaw = 0;
    int berRaw = 0;

    if (readResponse<int, int>(_csqParser, &csqRaw, &berRaw) == ResponseOK) {
        *rssi = ((csqRaw == 99) ? 0 : convertCSQ2RSSI(csqRaw));
        *ber = ((berRaw == 99 || static_cast<size_t>(berRaw) >= sizeof(berValues)) ? 0 : berValues[berRaw]);

        return true;
    }

    return false;
}

/****
 * The range is the following:
 * 0: -113 dBm or less
 * 1: -111 dBm
 * 2..30: from -109 to -53 dBm with 2 dBm steps
 * 31: -51 dBm or greater
 * 99: not known or not detectable or currently not available
 */
int8_t ATT_NBIOT::convertCSQ2RSSI(uint8_t csq) const
{
  return -113 + 2 * csq;
}

uint8_t ATT_NBIOT::convertRSSI2CSQ(int8_t rssi) const
{
  return (rssi + 113) / 2;
}

bool ATT_NBIOT::startsWith(const char* pre, const char* str)
{
  return (strncmp(pre, str, strlen(pre)) == 0);
}

bool ATT_NBIOT::waitForSignalQuality(uint32_t timeout)
{
  uint32_t start = millis();
  const int8_t minRSSI = getMinRSSI();
  int8_t rssi;
  uint8_t ber;

  uint32_t delay_count = 500;

  while (!is_timedout(start, timeout))
  {
    if (getRSSIAndBER(&rssi, &ber))
    {
      if (rssi != 0 && rssi >= minRSSI)
      {
        _lastRSSI = rssi;
        _CSQtime = (int32_t)(millis() - start) / 1000;
        return true;
      }
    }

    sodaq_wdt_safe_delay(delay_count);

    // Next time wait a little longer, but not longer than 5 seconds
    if (delay_count < 5000)
      delay_count += 1000;
  }

  return false;
}

/****
 * Send payloads as basic json
 * One value (integer, double, boolean or string) to one asset
 */

bool ATT_NBIOT::sendMessage(int value, const char* asset)
{
  String message;
  message += String(_deviceId);
  message += "\n";
  message += String(_deviceToken);
  message += "\n{\"";
  message += asset;
  message += "\":{\"value\":";
  message += String(value);
  message += "}}";
  return sendMessage((const uint8_t*)message.c_str(), strlen(message.c_str()));
}

bool ATT_NBIOT::sendMessage(double value, const char* asset)
{
  String message;
  message += String(_deviceId);
  message += "\n";
  message += String(_deviceToken);
  message += "\n{\"";
  message += asset;
  message += "\":{\"value\":";
  message += String(value);
  message += "}}";
  return sendMessage((const uint8_t*)message.c_str(), strlen(message.c_str()));
}

bool ATT_NBIOT::sendMessage(bool value, const char* asset)
{
  String message;
  message += String(_deviceId);
  message += "\n";
  message += String(_deviceToken);
  message += "\n{\"";
  message += asset;
  message += "\":{\"value\":";
  message += value == true ? "true" : "false";
  message += "}}";
  return sendMessage((const uint8_t*)message.c_str(), strlen(message.c_str()));
}

bool ATT_NBIOT::sendMessage(String value, const char* asset)
{
  sendMessage(value.c_str(), asset);
}

bool ATT_NBIOT::sendMessage(const char* value, const char* asset)
{
  String message;
  message += String(_deviceId);
  message += "\n";
  message += String(_deviceToken);
  message += "\n{\"";
  message += asset;
  message += "\":{\"value\":\"";
  message += String(value);
  message += "\"}}";
  return sendMessage((const uint8_t*)message.c_str(), strlen(message.c_str()));
}

/****
 *
 */
bool ATT_NBIOT::sendMessage(const uint8_t* buffer, size_t size)
{
  if (size > 512)
    return false;
  
  print("AT+NSOST=0,\"");
  print(_udp);
  print("\",");
  print(_port);
  print(",");
  print(size);  // Number of bytes in message
  print(",\"");
  
  for (uint16_t i = 0; i < size; ++i)
  {
    print(static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(buffer[i]))));
    print(static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(buffer[i]))));
  }
  println("\"");
  
  return (readResponse() == ResponseOK);
}

/****
 * Create binary payload
 */
bool ATT_NBIOT::sendPayload(void* packet, unsigned char size)
{
  int lng = 72;  // Fixed 72 chars "deviceid\ndevicetoken\n"

  // Print AT command
  print("AT+NSOST=0,\"");
  print(_udp);
  print("\",");
  print(_port);
  print(",");
  print(lng+size);  // Length of ATT credentials + actual sensor data part of the payload
  print(",\"");

  // Print ATT device credentials part of payload
  char buf[lng];
  sprintf(buf,"%s\n%s\n", _deviceId, _deviceToken);
  for (uint16_t i = 0; i < lng; ++i)
  {
    print(static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(buf[i]))));
    print(static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(buf[i]))));
  }
  
  // Print actual payload from binary buffer
  char hexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	for (unsigned char i = 0; i < size; i++)
  {
		print(hexTable[((unsigned char*)packet)[i] / 16]);
 		print(hexTable[((unsigned char*)packet)[i] % 16]);
	}
  println("\"");
  
  return (readResponse() == ResponseOK);
}

/****
 * Create cbor payload
 */
bool ATT_NBIOT::sendCbor(unsigned char* data, unsigned int size)
{
  int lng = strlen(_deviceId) + strlen(_deviceToken) + 2;  // Fixed chars "deviceid\ndevicetoken\n"

  // Print AT command
  print("AT+NSOST=0,\"");
  print(_udp);
  print("\",");
  print(_port);
  print(",");
  print(lng+size);  // Length of ATT credentials + actual sensor data part of the payload
  print(",\"");

  // Print ATT device credentials part of payload
  char buf[lng];
  sprintf(buf,"%s\n%s\n", _deviceId, _deviceToken);
  for (uint16_t i = 0; i < lng; ++i)
  {
    print(static_cast<char>(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(buf[i]))));
    print(static_cast<char>(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(buf[i]))));
  }
  
  // Print actual payload from cbor buffer
  char hexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  for (unsigned char i = 0; i < size; i++)
  {
	  print(hexTable[data[i] / 16]);
    print(hexTable[data[i] % 16]);
  }
  println("\"");

  return (readResponse() == ResponseOK);
}

/****
 *
 */
int ATT_NBIOT::getSentMessagesCount(SentMessageStatus filter)
{
  println("AT+NQMGS");

  uint16_t pendingCount = 0;
  uint16_t errorCount = 0;

  if (readResponse<uint16_t, uint16_t>(_nqmgsParser, &pendingCount, &errorCount) == ResponseOK)
  {
    if (filter == Pending)
      return pendingCount;
    else if (filter == Error)
      return errorCount;
  }

  return -1;
}

// ==============================
// AT Response parsing
// ==============================

/****
 * Read the next response from the modem
 *
 * Notice that we're collecting URC's here. And in the process we could
 *  be updating:
 * _socketPendingBytes[] if +UUSORD: is seen
 * _socketClosedBit[] if +UUSOCL: is seen
 */
ResponseTypes ATT_NBIOT::readResponse(char* buffer, size_t size,
                                        CallbackMethodPtr parserMethod, void* callbackParameter, void* callbackParameter2,
                                        size_t* outSize, uint32_t timeout)
{
  ResponseTypes response = ResponseNotFound;
  uint32_t from = NOW;

  do
  {
    // 250ms, how many bytes at which baudrate?
    int count = readLn(buffer, size, 250);
    sodaq_wdt_reset();

    if (count > 0)
    {
      if (outSize)
        *outSize = count;

      if (_disableDiag && strncmp(buffer, "OK", 2) != 0)
        _disableDiag = false;

      debugPrint("[rdResp]: ");
      debugPrintLn(buffer);
      
      // Handle FOTA URC
      int param1, param2;
      if (sscanf(buffer, "+UFOTAS: %d,%d", &param1, &param2) == 2)
      { 
        uint16_t blkRm = param1;
        uint8_t transferStatus = param2;

        debugPrint("Unsolicited: FOTA: ");
        debugPrint(blkRm);
        debugPrint(", ");
        debugPrintLn(transferStatus);

        continue;
      }

      if (startsWith(STR_AT, buffer))
        continue;  // Skip echoed back command

      _disableDiag = false;

      if (startsWith(STR_RESPONSE_OK, buffer))
        return ResponseOK;

      if (startsWith(STR_RESPONSE_ERROR, buffer) ||
          startsWith(STR_RESPONSE_CME_ERROR, buffer) ||
          startsWith(STR_RESPONSE_CMS_ERROR, buffer))
      {
        return ResponseError;
      }

      if (parserMethod)
      {
        ResponseTypes parserResponse = parserMethod(response, buffer, count, callbackParameter, callbackParameter2);

        if ((parserResponse != ResponseEmpty) && (parserResponse != ResponsePendingExtra))
          return parserResponse;
        else
        {
          // ?
          // ResponseEmpty indicates that the parser was satisfied
          // Continue until "OK", "ERROR", or whatever else.
        }

        // Prevent calling the parser again.
        // This could happen if the input line is too long. It will be split
        // and the next readLn will return the next part.
        // The case of "ResponsePendingExtra" is an exception to this, thus waiting for more replies to be parsed.
        if (parserResponse != ResponsePendingExtra)
          parserMethod = 0;
      }

      // at this point, the parserMethod has ran and there is no override response from it,
      // so if there is some other response recorded, return that
      // (otherwise continue iterations until timeout)
      if (response != ResponseNotFound)
      {
        debugPrintLn("** response != ResponseNotFound");
        return response;
      }
    }

    delay(10);
  }
  while (!is_timedout(from, timeout));

  if (outSize)
    *outSize = 0;

  debugPrintLn("[rdResp]: timed out");
  return ResponseTimeout;
}

ResponseTypes ATT_NBIOT::_createSocketParser(ResponseTypes& response, const char* buffer, size_t size, uint8_t* socket, uint8_t* dummy)
{
  if (!socket)
    return ResponseError;

  int value;

  if (sscanf(buffer, "%d", &value) == 1)
  {
    *socket = value;
    return ResponseEmpty;
  }

  return ResponseError;
}

ResponseTypes ATT_NBIOT::_nconfigParser(ResponseTypes& response, const char* buffer, size_t size, bool* nconfigEqualsArray, uint8_t* dummy)
{
  if (!nconfigEqualsArray)
    return ResponseError;

  char name[32];
  char value[32];

  if (sscanf(buffer, "+NCONFIG: \"%[^\"]\",\"%[^\"]\"", name, value) == 2)
  {
    for (uint8_t i = 0; i < nConfigCount; i++)
    {
      if (strcmp(nConfig[i].Name, name) == 0)
      {
        if (strcmp(nConfig[i].Value, value) == 0)
        {
          nconfigEqualsArray[i] = true;
          break;
        }
      }
    }
    return ResponsePendingExtra;
  }
  return ResponseError;
}

ResponseTypes ATT_NBIOT::_cgattParser(ResponseTypes& response, const char* buffer, size_t size, uint8_t* result, uint8_t* dummy)
{
  if (!result)
    return ResponseError;

  int val;
  if (sscanf(buffer, "+CGATT: %d", &val) == 1)
  {
    *result = val;
    return ResponseEmpty;
  }

  return ResponseError;
}

ResponseTypes ATT_NBIOT::_csqParser(ResponseTypes& response, const char* buffer, size_t size, int* rssi, int* ber)
{
  if (!rssi || !ber)
    return ResponseError;

  if (sscanf(buffer, "+CSQ: %d,%d", rssi, ber) == 2)
    return ResponseEmpty;

  return ResponseError;
}

ResponseTypes ATT_NBIOT::_nqmgsParser(ResponseTypes& response, const char* buffer, size_t size, uint16_t* pendingCount, uint16_t* errorCount)
{
  if (!pendingCount || !errorCount)
    return ResponseError;

  int pendingValue;
  int errorValue;

  if (sscanf(buffer, "PENDING=%d,SENT=%*d,ERROR=%d", &pendingValue, &errorValue) == 2)
  {
    *pendingCount = pendingValue;
    *errorCount = errorValue;

    return ResponseEmpty;
  }

  return ResponseError;
}

// ==============================
// on/off class
// ==============================
Sodaq_nbIotOnOff::Sodaq_nbIotOnOff()
{
  _onoffPin = -1;
  _onoff_status = false;
}

/****
 * Initializes the instance
 */
void Sodaq_nbIotOnOff::init(int onoffPin)
{
  if (onoffPin >= 0)
  {
    _onoffPin = onoffPin;
    // First write the output value, and only then set the output mode
    digitalWrite(_onoffPin, LOW);
    pinMode(_onoffPin, OUTPUT);
  }
}

/****
 * Turn on the bee
 */
void Sodaq_nbIotOnOff::on()
{
  if (_onoffPin >= 0)
    digitalWrite(_onoffPin, HIGH);

  _onoff_status = true;
}

/****
 * Turn off the bee
 */
void Sodaq_nbIotOnOff::off()
{
  if (_onoffPin >= 0)
    digitalWrite(_onoffPin, LOW);

  // Should be instant
  // Let's wait a little, but not too long
  delay(50);
  _onoff_status = false;
}

/****
 * Check the status of the bee
 */
bool Sodaq_nbIotOnOff::isOn()
{
#if defined(ARDUINO_ARCH_AVR)
  // Use the onoff pin, which is close to useless
  bool status = digitalRead(_onoffPin);
  return status;
#elif defined(ARDUINO_ARCH_SAMD)
  // There is no status pin. On SAMD we cannot read back the onoff pin.
  // So, our own status is all we have.
  return _onoff_status;
#endif

  // Let's assume it is on.
  return true;
}
