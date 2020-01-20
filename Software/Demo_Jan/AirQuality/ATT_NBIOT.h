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

#ifndef _Sodaq_nbIOT_h
#define _Sodaq_nbIOT_h

#include "Arduino.h"
#include "Sodaq_AT_Device.h"

class ATT_NBIOT: public Sodaq_AT_Device
{
  public:
    ATT_NBIOT();
    ATT_NBIOT(const char* deviceId, const char* deviceToken);

    enum SentMessageStatus {
      Pending,
      Error
    };

    typedef ResponseTypes(*CallbackMethodPtr)(ResponseTypes& response, const char* buffer, size_t size, void* parameter, void* parameter2);

    bool setRadioActive(bool on);
    bool setApn(const char* apn);

    // Returns true if the modem replies to "AT" commands without timing out.
    bool isAlive();

    // Returns the default baud rate of the modem.
    // To be used when initializing the modem stream for the first time.
    uint32_t getDefaultBaudrate() { return 9600; };

    // Create an instance of an AllThingsTalk device
    void setAttDevice(const char* deviceId, const char* deviceToken, const char* apn);
    
    // Initializes the modem instance. Sets the modem and debug stream and the on-off power pins.
    void init(Stream& stream, Stream& debug, int8_t onoffPin);

    // Turns on and initializes the modem, then connects to the network and activates the data connection.
    //bool connect(const char* apn, const char* udp, const char* port, const char* forceOperator = 0);
    bool connect();

    // Disconnects the modem from the network.
    bool disconnect();

    // Returns true if the modem is connected to the network and has an activated data connection.
    bool isConnected();

    // Gets the Received Signal Strength Indication in dBm and Bit Error Rate.
    // Returns true if successful.
    bool getRSSIAndBER(int8_t* rssi, uint8_t* ber);
    int8_t convertCSQ2RSSI(uint8_t csq) const;
    uint8_t convertRSSI2CSQ(int8_t rssi) const;

    void setMinRSSI(int rssi) { _minRSSI = rssi; }
    void setMinCSQ(int csq) { _minRSSI = convertCSQ2RSSI(csq); }
    int8_t getMinRSSI() const { return _minRSSI; }
    uint8_t getCSQtime() const { return _CSQtime; }
    int8_t getLastRSSI() const { return _lastRSSI; }

    int createSocket(uint16_t localPort = 0);
    
    // Generic send functions
    bool sendMessage(const uint8_t* buffer, size_t size);
    
    // Send 1 value to 1 asset
    bool sendMessage(int value, const char* asset);
    bool sendMessage(bool value, const char* asset);
    bool sendMessage(double value, const char* asset);
    bool sendMessage(const char* value, const char* asset);
    bool sendMessage(String value, const char* asset);
    
    // Send binary payload
    bool sendPayload(void* packet, unsigned char size);
    
    // Send cbor payload
    bool sendCbor(unsigned char* data, unsigned int size);
    
    int getSentMessagesCount(SentMessageStatus filter);
    
  protected:
    // override
    ResponseTypes readResponse(char* buffer, size_t size, size_t* outSize, uint32_t timeout = SODAQ_AT_DEVICE_DEFAULT_READ_MS)
    {
        return readResponse(_inputBuffer, _inputBufferSize, NULL, NULL, NULL, outSize, timeout);
    };

    ResponseTypes readResponse(char* buffer, size_t size,
                               CallbackMethodPtr parserMethod, void* callbackParameter, void* callbackParameter2 = NULL,
                               size_t* outSize = NULL, uint32_t timeout = SODAQ_AT_DEVICE_DEFAULT_READ_MS);

    ResponseTypes readResponse(size_t* outSize = NULL, uint32_t timeout = SODAQ_AT_DEVICE_DEFAULT_READ_MS)
    {
        return readResponse(_inputBuffer, _inputBufferSize, NULL, NULL, NULL, outSize, timeout);
    };

    ResponseTypes readResponse(CallbackMethodPtr parserMethod, void* callbackParameter,
                               void* callbackParameter2 = NULL, size_t* outSize = NULL, uint32_t timeout = SODAQ_AT_DEVICE_DEFAULT_READ_MS)
    {
        return readResponse(_inputBuffer, _inputBufferSize,
                            parserMethod, callbackParameter, callbackParameter2,
                            outSize, timeout);
    };

    template<typename T1, typename T2>
    ResponseTypes readResponse(ResponseTypes(*parserMethod)(ResponseTypes& response, const char* parseBuffer, size_t size, T1* parameter, T2* parameter2),
                               T1* callbackParameter, T2* callbackParameter2,
                               size_t* outSize = NULL, uint32_t timeout = SODAQ_AT_DEVICE_DEFAULT_READ_MS)
    {
        return readResponse(_inputBuffer, _inputBufferSize, (CallbackMethodPtr)parserMethod,
                            (void*)callbackParameter, (void*)callbackParameter2, outSize, timeout);
    };

    void purgeAllResponsesRead();
  private:
    //uint16_t _socketPendingBytes[SOCKET_COUNT]; // TODO add getter
    //bool _socketClosedBit[SOCKET_COUNT];

    // This is the value of the most recent CSQ
    // Notice that CSQ is somewhat standard. SIM800/SIM900 and Ublox
    // compute to comparable numbers. With minor deviations.
    // For example SIM800
    //   1              -111 dBm
    //   2...30         -110... -54 dBm
    // For example UBlox
    //   1              -111 dBm
    //   2..30          -109 to -53 dBm
    int8_t _lastRSSI;  // 0 not known or not detectable

    // This is the number of second it took when CSQ was record last
    uint8_t _CSQtime;

    // This is the minimum required RSSI to continue making the connection
    // Use convertCSQ2RSSI if you have a CSQ value
    int _minRSSI;
    
    // Network and device credentials
    const char* _udp;
    const char* _port;
    const char* _deviceId;
    const char* _deviceToken;
    const char* _apn;

    static bool startsWith(const char* pre, const char* str);

    bool checkAndApplyNconfig();
    bool setOperator(const char* forceOperator);
    bool setOperator();
    void reboot();
    bool attachBee(uint32_t timeout = 30 * 1000);
    bool waitForSignalQuality(uint32_t timeout = 60L * 1000);
    bool setNconfigParam(const char* param, const char* value);
    
    static ResponseTypes _csqParser(ResponseTypes& response, const char* buffer, size_t size, int* rssi, int* ber);
    static ResponseTypes _createSocketParser(ResponseTypes& response, const char* buffer, size_t size, uint8_t* socket, uint8_t* dummy);
    static ResponseTypes _nqmgsParser(ResponseTypes& response, const char* buffer, size_t size, uint16_t* pendingCount, uint16_t* errorCount);
    static ResponseTypes _cgattParser(ResponseTypes& response, const char* buffer, size_t size, uint8_t* result, uint8_t* dummy);
    static ResponseTypes _nconfigParser(ResponseTypes& response, const char* buffer, size_t size, bool* nconfigEqualsArray, uint8_t* dummy);
};

#endif