/********************************************************************************
    Ark Cryptocurrency BridgeChain IOT Faucet
    This projects runs on an ESP8266 or ESP32 microcontroller and provides a way to easily send Ark Bridgechain Cryptocurrency to a wallet address that is sent via Telegram app.
    This faucet is intended to provide a simple way to distribute free tokens on a testnet during development.

    ArkBridgechain_IOT_faucet.ino
    2019 @phillipjacobsen

    Program Features:
    This program has been tested with ESP32 Adafruit Huzzah however it should also work with ESP8266 modules with minor changes to hardware connections and wifi libraries.
    This sketch uses the ARK Cpp-Client API to interact with an Ark V2 Devnet node.
    Ark Cpp Client available from Ark Ecosystem <info@ark.io>
    Ark API documentation:  https://docs.ark.io/sdk/clients/usage.html

    Electronic Hardware Peripherals:
    Adafruit TFT FeatherWing 2.4" 320x240 Touchscreen

  Description of the current program flow.  status/debug info is also regularly sent to serial terminal
  1. configure peripherals
  -setup wifi and display connection status and IP address on TFT Screen
  -setup time sync with NTP server and display current time
  -check to see if Ark node is synced and display status
  2. search through all received transactions on wallet. Wallet address is displayed
    -"searching wallet + page#" will be displayed. text will toggle between red/white every received transaction
  3. # of transactions in wallet will be displayed
  4. User Interface with 3 buttons are displayed(only 1 button currently functions("M&M").
  5. When user selects M&M's a QRcode is displayed along with a timeout displayed.
  4. QR code includes price, address and Vendor field = "ArkVend_(random number)" when scanned by Ark mobile wallet.
  5. wallet waits for transaction with vendor field to be received.
  6. If payment is received then it will indicated success and display in green text "Payment: ArkVend_(random_number)"
    if incorrect payment is received then received transaction will be ignored.
  7. Back to Step 4

********************************************************************************/

// Conditional Assembly

//select your device. Only 1 device should be defined
//#define ESP8266
#define ESP32



#include <Arduino.h>    //do we need this?


/********************************************************************************
                              Library Requirements
********************************************************************************/
const int ledPin = 13;    //LED integrated in Adafruit HUZZAH32
int ledStatus = 0;


/********************************************************************************
  Ark Client Library (version 1.2.0)
    Available through Arduino Library Manager
    https://github.com/ArkEcosystem/cpp-client

  Ark Crypto Library (version  )
    Available through Arduino Library Manager

********************************************************************************/
#include <arkClient.h>
#include <arkCrypto.h>

/**
    This is where you define the IP address of the Ark BridgeChain Testnet/Devnet Node.
    The Public API port for the V2 Ark network is '4003'
*/
const char* peer = "159.203.42.124";  //BridgeChain Testnet/Devnet Peer
int port = 4003;


//Wallet Address on bridgechain that holds the tokens
const char* ArkAddress = "TRv5prdxiSSTeBELxyKKp3pj6UtvwkQyCh";   //NYBBLE testnet address
const char* ArkPublicKey = "028dad72086b512564f697da0c813bfaf5358badcdbf8eb6ef2497bd05f873d28d";       //
const char* yourSecretPassphrase = "antique depart senior usage lesson wash antique indoor hero matrix drum green";

char VendorID[64];      //this needs to be expanded


const char* recipientId = "TUG1LSi9Di7dBTHze7GYN653pU3mhGSAPQ";






/********************************************************************************
  WiFi Library
  If using the ESP8266 I believe you will need to #include <ESP8266WiFi.h> instead of WiFi.h
********************************************************************************/
#ifdef  ESP32
#include <WiFi.h>
#endif

#ifdef  ESP8266
#include <ESP8266WiFi.h> 
#endif

//--------------------------------------------
//This is your WiFi network parameters that you need to configure

//const char* ssid = "xxxxxxxxxx";
//const char* password = "xxxxxxxxxx";

//h
const char* ssid = "TELUS0183";
const char* password = "6z5g4hbdxi";

//w
//const char* ssid = "TELUS6428";
//const char* password = "3mmkgc9gn2";






/********************************************************************************
  Time Library
  required for internal clock to syncronize with NTP server.
  I need to do a bit more work in regards to Daylight savings time and the periodic sync time with the NTP service after initial syncronization
********************************************************************************/
#include "time.h"
//#include <TimeLib.h>    //https://github.com/PaulStoffregen/Time
int timezone = -6;        //set timezone:  MST
int dst = 0;              //To enable Daylight saving time set it to 3600. Otherwise, set it to 0. Not sure if this works.


unsigned long timeNow;  //variable used to hold current millis() time.
unsigned long payment_Timeout;
unsigned long timeAPIfinish;  //variable used to measure API access time
unsigned long timeAPIstart;  //variable used to measure API access time







void idHashToBuffer(char hashBuffer[64]) {
  int idByteLen = 6;

#ifdef  ESP32  
  uint64_t chipId = ESP.getEfuseMac();
#endif

#ifdef  ESP8266
  uint64_t chipId = ESP.getChipId(); 
#endif

  uint8_t *bytArray = *reinterpret_cast<uint8_t(*)[sizeof(uint64_t)]>(&chipId);
  std::reverse(&bytArray[0], &bytArray[idByteLen]);

  const auto shaHash = Sha256::getHash(&bytArray[0], idByteLen);

  memmove(hashBuffer, BytesToHex(&shaHash.value[0], &shaHash.value[0] + shaHash.HASH_LEN).c_str(), Sha256::BLOCK_LEN);
}

Transaction txFromHash(char hashBuffer[64]) {
    return Ark::Crypto::Transactions::Builder::buildTransfer(recipientId, 1, hashBuffer, yourSecretPassphrase);
  //return Ark::Crypto::Transactions::Builder::buildTransfer(recipientId, 1000, "GiddyUp", yourSecretPassphrase);

}

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  setupWiFi();
  configTime(timezone * 3600, dst, "pool.ntp.org", "time.nist.gov");
  configTime(0, 0, "pool.ntp.org");
}

void loop() {
  char hashBuffer[Sha256::BLOCK_LEN + 1] = { '0' };
  idHashToBuffer(hashBuffer);
  Transaction transaction = txFromHash(hashBuffer);
  char jsonBuffer[576] = { '\0' };
  snprintf(&jsonBuffer[0], 576, "{\"transactions\":[%s]}", transaction.toJson().c_str());
  std::string jsonStr(jsonBuffer);

  Ark::Client::Connection<Ark::Client::Api> connection(peer, port);

  std::string txSendResponse = connection.api.transactions.send(jsonStr);
  Serial.print("\ntxSendResponse: ");
  Serial.println(txSendResponse.c_str());
  delay(1000);
#ifdef  ESP32  
  esp_deep_sleep_start();
#endif

#ifdef  ESP8266
while(true) {}
 // esp_deepSleep(0);
#endif

  
}