
/*
  start - Show list of commands
  options -Show options keyboard
  name - Get Bot name
  time - Get current Time
  balance - Get balance of wallet
  address - Get address of faucet
  request_######wallet address######
*/



void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages: ");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    Serial.println(text);
    Serial.println(chat_id);

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    //response keyboard.
    if (text == "/options" || text == "/options@NybbleFaucet_bot") {
      String keyboardJson = "[ [\"/ledon\", \"/ledoff\", \"/status\" ] , [\"/balance\", \"/transactions\"] ]";

      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);
    }


    if (text == "/ledon" || text == "/ledon@NybbleFaucet_bot") {
      //digitalWrite(ledPin, LOW);   // turn the LED on (HIGH is the voltage level)   //esp8266
      digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)     //Feather ESP32
      ledStatus = 1;
      bot.sendMessage(chat_id, "LED is ON", "");
    }

    if (text == "/ledoff" || text == "/ledoff@NybbleFaucet_bot") {
      ledStatus = 0;
      //digitalWrite(ledPin, HIGH);    // turn the LED off (LOW is the voltage level)  //esp8266
      digitalWrite(ledPin, LOW);    // turn the LED off (LOW is the voltage level)    //Heather32

      bot.sendMessage(chat_id, "LED is OFF", "");
    }


    if (text == "/name" || text == "/name@NybbleFaucet_bot" ) {
      bot.sendMessage(chat_id, "I am Nybble(Ark BridgeChain) Faucet Bot", "");
    }


    if (text == "/time" || text == "/time@NybbleFaucet_bot") {
      //  time_t is used to store the number of seconds since the epoch (normally 01/01/1970)
      time_t now = time(nullptr);
      bot.sendMessage(chat_id, ctime(&now), "");      //return the current time

      Serial.print("time is: ");
      Serial.println(ctime(&now));
    }

    if (text == "/status" || text == "/status@NybbleFaucet_bot") {
      if (ledStatus) {
        bot.sendMessage(chat_id, "Status: Led is ON", "");
      } else {
        bot.sendMessage(chat_id, "Status: Led is OFF", "");
      }
    }


    if (text == "/address" || text == "/address@NybbleFaucet_bot") {
      bot.sendMessage(chat_id, FaucetAddress, "");
    }


    if (text.startsWith("/request_")) {

      if (text.charAt(9) == 'T')  {        //simple address verification
        String receiveaddress = text.substring(9, 9 + 34 + 1);       //Ark address is 34 digits long
        Serial.print("\nreceiveAddress: ");
        Serial.print(receiveaddress);

        //const char*


        char receiveaddress_char[40];
        receiveaddress.toCharArray(receiveaddress_char, 35);
        Serial.print("\nreceiveAddress char: ");
        Serial.print(receiveaddress_char);

        Transaction transaction = Ark::Crypto::Transactions::Builder::buildTransfer(receiveaddress_char, 1000000000, "faucet", yourSecretPassphrase);

        const auto txJson = transaction.toJson();

        char jsonBuffer[576] = { '\0' };
        snprintf(&jsonBuffer[0], 576, "{\"transactions\":[%s]}", txJson.c_str());
        std::string jsonStr = jsonBuffer;
        //std::string jsonStr(jsonBuffer);      //this is used by arkstoretoblockchainsimple

        //  const auto txSendResponse = connection.api.transactions.send(jsonStr);
        std::string txSendResponse = connection.api.transactions.send(jsonStr);
        Serial.print("\ntxSendResponse: ");
        Serial.println(txSendResponse.c_str());

        String transactionresult = "transaction result: ";

        transactionresult += txSendResponse.c_str();
        bot.sendMessage(chat_id, transactionresult, "");      //Add @RawDataBot to your group chat to find the chat id.

      }

      else {
        break;
      }
    }

    if (text == "/start" || text == "/start@NybbleFaucet_bot") {
      Serial.println("\nreceived start ");

      String welcome = "Welcome to Nybble Faucet Bot, " + from_name + ".\n";
      welcome += "I can send you free Nyb testnet coins.\n";
      welcome += "Nybble is a custom Ark Bridgechain\n";
      //      welcome += "Use http://159.203.42.124:4003 as seed server in Ark Desktop Wallet\n\n";
      welcome += "/options : display option keyboard\n";
//      welcome += "/name : Returns bot name\n";
//      welcome += "/time : Returns current Date and Time\n";
      welcome += "/address : Returns wallet address of faucet\n";
      welcome += "/balance : Returns balance of wallet\n";
      //      welcome += "/request_WALLETADDRESS : Replace WALLETADDRESS with your address\n";
      welcome += "/request\\_WALLETADDRESS : replace WALLETADDRESS with your address\n";
      bot.sendMessage(chat_id, welcome, "Markdown");


      Serial.println("sent start message to telegram");

    }


    if (text == "/balance" || text == "/balance@NybbleFaucet_bot" ) {
      //  retrieve balance from wallet
      std::string walletResponse = connection.api.wallets.get(FaucetAddress);
      Serial.print("\nWallet: ");
      Serial.println(walletResponse.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.
      //https://arduinojson.org/v5/assistant/
      /*
        {"data":{"address":"AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx","publicKey":null,"username":null,"secondPublicKey":null,"balance":4480000000,"isDelegate":false,"vote":null}}
      */
      //--------------------------------------------
      //  Deserialize the returned JSON
      //  All of the returned parameters are parsed which is not necessary but may be usefull for testing.
      const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(7) + 160;
      DynamicJsonBuffer jsonBuffer(capacity);

      //const char* json = "{\"data\":{\"address\":\"AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx\",\"publicKey\":null,\"username\":null,\"secondPublicKey\":null,\"balance\":4480000000,\"isDelegate\":false,\"vote\":null}}";

      JsonObject& root = jsonBuffer.parseObject(walletResponse.c_str());

      JsonObject& data = root["data"];
      //     const char* data_address = data["address"]; // "AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx"
      //long data_balance = data["balance"]; // 4480000000
      const char* data_balance2 = data["balance"]; // 4480000000    //I am not sure why this is a const char* instead of a long as generated by json parser tool
      //     bool data_isDelegate = data["isDelegate"]; // false


      Serial.print("balance: ");
      // Serial.println(data_balance);
      // Serial.println(String(data_balance));
      Serial.println(data_balance2);

      String balance = "balance: ";
      balance += String(data_balance2);
      bot.sendMessage(chat_id, balance, "");

    }



  }
}
