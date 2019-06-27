/********************************************************************************
    Ark Cryptocurrency BridgeChain IOT Faucet
    This projects runs on an ESP8266 or ESP32 microcontroller and provides a way to easily send Ark Bridgechain Cryptocurrency to a wallet address that is sent via Telegram app.
    This faucet is intended to provide a simple way to distribute free tokens on a testnet during development.

    ArkBridgechain_IOT_faucet.ino
    2019 @phillipjacobsen

    Program Features:
    This program has been tested with ESP32 Adafruit Huzzah however it should also work with ESP8266 modules with minor changes to hardware connections and wifi libraries.
    This sketch uses the ARK Cpp-Client API to interact with an Ark V2.4 Bridgechain Testnet/Devnet node.
    Ark Cpp Client available from Ark Ecosystem <info@ark.io>
    Ark API documentation:  https://docs.ark.io/sdk/clients/usage.html

    This program will provide similar features/settings as the ARK devnet faucet for DARK coins by deadlock-delegate
    https://github.com/deadlock-delegate/faucet

    faucetAddress - address of your faucet
    vendorField - what message you wish to add to each payout
    payoutAmount - what amount you wish to payout per request
    payoutCooldown - for how long will user not be able to request a payment for
    dailyPayoutLimit - what's the faucet's max (overall) daily payout



    Electronic Hardware Peripherals:


  Description of the desired program flow.  status/debug info is also regularly sent to serial terminal
  TBD




********************************************************************************/

// conditional assembly
//
#define NYBBLE   //this configures system for my custom bridgechain. If undefined then system will be configured for Ark Devnet.

#define ARDUINOJSON_USE_LONG_LONG 1     //required for compatiblity between Telegram Library and ArkCrypto library.

//select your device. Only 1 device should be defined at a time
//#define ESP8266
#define ESP32



/********************************************************************************
                              Library Requirements
********************************************************************************/
const int ledPin = 13;    //LED integrated in Adafruit HUZZAH32
int ledStatus = 0;


/********************************************************************************
   Arduino Json Libary - works with Version5.  NOT compatible with Version6
    Available through Arduino Library Manager
    Data returned from Ark API is in JSON format.
    This libary is used to parse and deserialize the reponse
********************************************************************************/
#include <ArduinoJson.h>


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
#ifdef NYBBLE
const char* peer = "159.203.42.124";  //BridgeChain Testnet/Devnet .Peer make sure to set epoch in devnet.h to     "2019-06-18T22:44:26.320Z"  // Epoch
int port = 4003;
#else
const char* peer = "167.114.29.49"; //make sure to set epoch in devnet.h to     "2017-03-21T13:00:00.000Z"  // Epoch
int port = 4003;
#endif



#ifdef NYBBLE
//Wallet Address on bridgechain that holds the tokens
const char* FaucetAddress = "TRv5prdxiSSTeBELxyKKp3pj6UtvwkQyCh";   //NYBBLE testnet address that holds faucet funds
const char* ArkPublicKey = "028dad72086b512564f697da0c813bfaf5358badcdbf8eb6ef2497bd05f873d28d";       //
const char* yourSecretPassphrase = "antique depart senior usage lesson wash antique indoor hero matrix drum green";

const char* recipientId = "TUG1LSi9Di7dBTHze7GYN653pU3mhGSAPQ";
#else
const char* FaucetAddress = "DHy5z5XNKXhxztLDpT88iD2ozR7ab5Sw2w";   //NYBBLE testnet address that holds faucet funds
//const char* ArkPublicKey = "028dad72086b512564f697da0c813bfaf5358badcdbf8eb6ef2497bd05f873d28d";       //
const char* yourSecretPassphrase = "butter truly next bike bonus brass destroy ripple chef sheriff actor helmet";

const char* recipientId = "DHy5z5XNKXhxztLDpT88iD2ozR7ab5Sw2w";

#endif


//char VendorID[64];      //this needs to be expanded




char vendorField[Sha256::BLOCK_LEN + 1] = { '\0' };

/**
   This is how you define a connection while speficying the API class as a 'template argument'
   You instantiate a connection by passing a IP address as a 'c_string', and the port as an 'int'.
*/
Ark::Client::Connection<Ark::Client::Api> connection(peer, port);


//I think a structure here for transaction details would be better form
//I need to do some work here to make things less hacky
//struct transactionDetails {
//   const char*  id;
//   int amount;
//   const char* senderAddress;
//   const char* vendorField;
//};

//--------------------------------------------
// these are used to store the received transation details returned from wallet search
const char*  id;            //transaction ID
int amount;                 //transactions amount
const char* senderAddress;  //transaction address of sender
//const char* vendorField;    //vendor field

int lastRXpage;             //page number of the last received transaction in wallet
int searchRXpage;           //page number that is used for wallet search

int ARK_mtbs = 4000;      //mean time(in ms) between polling Ark API for new transactions
long ARKscan_lasttime;   //last time Ark API poll has been done




/********************************************************************************
  Time Library
  required for internal clock to syncronize with NTP server.
  I need to do a bit more work in regards to Daylight savings time and the periodic sync time with the NTP service after initial syncronization
********************************************************************************/
#include "time.h"
int timezone = -6;        //set timezone:  MST
int dst = 0;              //To enable Daylight saving time set it to 3600. Otherwise, set it to 0. Not sure if this works.


unsigned long timeNow;  //variable used to hold current millis() time.
//unsigned long payment_Timeout;
unsigned long timeAPIfinish;  //variable used to measure API access time
unsigned long timeAPIstart;  //variable used to measure API access time

/********************************************************************************
  WiFi Library
  If using the ESP8266 you will need to #include <ESP8266WiFi.h> instead of WiFi.h
********************************************************************************/
#ifdef ESP32
#include <WiFi.h>
#endif

#ifdef ESP8266
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
  Telegram BOT
    Bot name: Nybble (Ark BridgeChain) Faucet
    Bot username: NybbleFaucet_bot
    t.me/NybbleFaucet_bot
    Token: 833803898:AAG9mcKAEzdd7W7p_RtjIZqp48Lt4X-tTMw
  Keep your token secure and store it safely, it can be used by anyone to control your bot.

  For a description of the Bot API, see this page: https://core.telegram.org/bots/api
  Use this link to create/manage bot via BotFather: https://core.telegram.org/bots#6-botfather

  Telegram library written by Giacarlo Bacchio (Gianbacchio on Github)
  adapted by Brian Lough
  https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot


  after bot has been created use the following in BotFather to change the list of commands supported by your bot.
  Users will see these commands as suggestions when they type / in the chat with your bot.

  /setcommands
  then enter commands like this. all in one chat. It seems you have to add all commands at once. I am not sure how to just add a new command to the list.
  start - Show list of commands
  options -Show options keyboard
  name - Get Bot name
  time - Get current Time
  balance - Get balance of wallet
  address - Get address of faucet
  request_######wallet address######
********************************************************************************/
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOTtoken "833803898:AAG9mcKAEzdd7W7p_RtjIZqp48Lt4X-tTMw"  // your Bot Token (Get from Botfather)
#define telegram_chat_id "-344083892"  //Add @RawDataBot to your group chat to find the chat id.

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


int Bot_mtbs = 2800; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;






/********************************************************************************
  Function prototypes
  Arduino IDE normally does its automagic here and creates all the function prototypes for you.
  We have put functions in other files so we need to manually add some prototypes as the automagic doesn't work correctly
********************************************************************************/


/********************************************************************************
  End Function Prototypes
********************************************************************************/
void handleNewMessages(int numNewMessages);




//  sendTX(vendorField);












void loop() {

  delay(1);

  if (millis() > Bot_lasttime + Bot_mtbs)  {

    timeAPIstart = millis();  //get time that API read started

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    timeNow = millis() - timeAPIstart;  //get current time
    Serial.print("Telegram get update time:");
    Serial.println(timeNow);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }




  /*
     char hashBuffer[Sha256::BLOCK_LEN + 1] = { '0' };
     idHashToBuffer(hashBuffer);
     Transaction transaction = txFromHash(hashBuffer);
     char jsonBuffer[576] = { '\0' };
     snprintf(&jsonBuffer[0], 576, "{\"transactions\":[%s]}", transaction.toJson().c_str());
     std::string jsonStr(jsonBuffer);

    //   Ark::Client::Connection<Ark::Client::Api> connection(peer, port);

     std::string txSendResponse = connection.api.transactions.send(jsonStr);
     Serial.print("\ntxSendResponse: ");
     Serial.println(txSendResponse.c_str());
     esp_deep_sleep_start();

  */

}
