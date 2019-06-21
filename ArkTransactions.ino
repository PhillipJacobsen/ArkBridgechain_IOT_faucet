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


/*
bool checkArkNodeStatus() {
 
*/

  /**
     The following method can be used to get the Status of a Node.
     This is equivalant to calling '167.114.29.49:4003/api/v2/node/status'
     The response should be a json-formatted object
     The "pretty print" version would look something like this:

     {
      "data": {
        "synced": true,
        "now": 1178395,
        "blocksCount": 0
       }
     }
 */

 /*
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

*/


void sendTX(char vfBuffer[Sha256::BLOCK_LEN + 1]) {
    int idByteLen = 6;
    uint64_t chipId = ESP.getEfuseMac();

    uint8_t *bytArray = *reinterpret_cast<uint8_t(*)[sizeof(uint64_t)]>(&chipId);
    std::reverse(&bytArray[0], &bytArray[idByteLen]);

    const auto shaHash = Sha256::getHash(&bytArray[0], idByteLen);
    memmove(vfBuffer, BytesToHex(&shaHash.value[0], &shaHash.value[0] + shaHash.HASH_LEN).c_str(), Sha256::BLOCK_LEN);

    Transaction transaction = Ark::Crypto::Transactions::Builder::buildTransfer(recipientId, 1, vfBuffer, yourSecretPassphrase);
    const auto txJson = transaction.toJson();

    char jsonBuffer[576] = { '\0' };
    snprintf(&jsonBuffer[0], 576, "{\"transactions\":[%s]}", txJson.c_str());
    std::string jsonStr = jsonBuffer;

    const auto txSendResponse = connection.api.transactions.send(jsonStr);

        Serial.print("\ntxSendResponse: ");
    Serial.println(txSendResponse.c_str());
}
