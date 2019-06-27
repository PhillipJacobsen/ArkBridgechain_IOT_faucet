/********************************************************************************
  This file contains functions that interact with Ark client C++ API
  code here is a hack right now. Just learning the API and working on basic program flow and function
********************************************************************************/







/********************************************************************************
  This routine checks to see if Ark node is syncronized to the chain.
  This is a maybe a good way to see if node communication is working correctly.
  This might be a good routine to run periodically
  Returns True if node is synced
********************************************************************************/



  bool checkArkNodeStatus() {

  const auto nodeStatus = connection.api.node.status();
  Serial.print("\nNode Status: ");
  Serial.println(nodeStatus.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + 50;
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(nodeStatus.c_str());

  JsonObject& data = root["data"];
  bool data_synced = data["synced"]; // true
  //long data_now = data["now"]; // 1178395
  //int data_blocksCount = data["blocksCount"]; // 0

  return data_synced;


  }



/*
void sendTX(char vfBuffer[Sha256::BLOCK_LEN + 1]) {

  Transaction transaction = Ark::Crypto::Transactions::Builder::buildTransfer(recipientId, 1, "candy", yourSecretPassphrase);

  const auto txJson = transaction.toJson();

  char jsonBuffer[576] = { '\0' };
  snprintf(&jsonBuffer[0], 576, "{\"transactions\":[%s]}", txJson.c_str());
  std::string jsonStr = jsonBuffer;
  //std::string jsonStr(jsonBuffer);      //this is used by arkstoretoblockchainsimple


  //  const auto txSendResponse = connection.api.transactions.send(jsonStr);
  std::string txSendResponse = connection.api.transactions.send(jsonStr);
  Serial.print("\ntxSendResponse: ");
  Serial.println(txSendResponse.c_str());
}

*/
