#include <MQTTClient.h>
#include <system.h>
#include "ESP8266WiFi.h"

//#include <DHT11.h>

#include <SPI.h>
#include <MFRC522.h>

#include <Servo.h>

Servo m;

constexpr uint8_t RST_PIN = 5;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 4;     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

int led = D8;

//DHT11 dht11(D7); 

const char* ssid = "Ruchid"; //ganti
const char* password = "13019717";//ganti

//Wifi
WiFiClient net;

//MQTT
MQTTClient client;
const char* mqttuser = "lucidplayer"; //manut wae
const char* mqttpassword = "lucidplayer"; //manut nggonmu
const char* deviceId = "nathanielclarence";//terserah

const String roomNumber = "02"; //enter room number
const String topic = "/iot/temp/" + roomNumber;//topic kalo mau diganti boleh
const String sendLock = "/iot/lock/" + roomNumber;//ini juga sama
const String lockStat = "/iot/lock/" + roomNumber + "/stat/lock";//utk lock/unlock rfid
const String doorStat = "/iot/lock/" + roomNumber + "/stat/door";//utk lock/unlock pintu
const String sendUser = "/iot/lock/" + roomNumber + "/stat/user";
//const String sendID = "/iot/lock/" + roomNumber + "/stat/userID";
const String statReq = "/iot/lock/" + roomNumber + "/stat";

/*
const char* opendoor = "/iot/servo";
const char* lockHandling = "/iot/lockHandle";
*/
int lock = 0;//rfid
int door = 0;//pintu
String room;
String user;//nama pengguna
String userID;//no KTP/user ID

void setup() {
  Serial.begin(115200);
  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);// Initialize serial communications with the PC
  
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  m.attach(2);//ganti pin
  m.write(0);
  pinMode(led, OUTPUT);
  //connect();
  //client.publish(lockStat,(String)lock);
  //client.publish(doorStat, (String)door);
}

void connect(){
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  //connect to shiftr.io 
  Serial.println("Connecting to broker.shifter.io....");
  while(!client.connect(deviceId, mqttuser, mqttpassword)){
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to broker.shiftr.io");
  client.subscribe(sendLock);
  client.subscribe(statReq);
}

void loop() {
  //int err;
  //float temp, humi;

  //mqtt
  client.loop();
  if(!client.connected()){
    connect();
  }
/*
  //dht11
  /*if((err=dht11.read(humi,temp))==0){
  Serial.print("Current temperature: ");
  Serial.println(temp);
  client.publish(topic, String(temp));
  }*/

  if(lock==0){
    // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    //some variables we need
    byte block;
    byte len;
    MFRC522::StatusCode status;

    //-------------------------------------------

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    Serial.println(F("**Card Detected:**"));

    //-------------------------------------------

    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

      //---------------------------------------- GET Room
    Serial.println();
    Serial.print(F("Room: "));

    byte buffer1[18];
    block = 1;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    status = mfrc522.MIFARE_Read(block, buffer1, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    //PRINT Room
    for (uint8_t i = 0; i < 16; i++) {
      Serial.write(buffer1[i] );
    }
  
   room = String((char*)buffer1);
   room = room.substring(0,room.indexOf(" "));
   Serial.println(room);
   

  Serial.println();
    Serial.print(F("User: "));

    block = 8;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    status = mfrc522.MIFARE_Read(block, buffer1, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    //PRINT User
    for (uint8_t i = 0; i < 16; i++) {
      Serial.write(buffer1[i] );
    }

  user = String((char*)buffer1);
   user = user.substring(0,user.indexOf(" "));
  
  Serial.println();
    Serial.print(F("UID: "));

    block = 12;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    status = mfrc522.MIFARE_Read(block, buffer1, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    //PRINT UID
    for (uint8_t i = 0; i < 16; i++) {
      Serial.write(buffer1[i] );
    }

  userID = String((char*)buffer1);
   userID = userID.substring(0,userID.indexOf(" "));
   
    //open sesame
      if(roomNumber == room){
        /*int pos = 120;
        m.write(pos);
        delay(10000);
        m.write(-pos);
        Serial.println(pos);*/
        digitalWrite(led, HIGH);
  
        if(door == 0)
          
          lockDoor();
        else
          openDoor();

          delay(1000);
  digitalWrite(led, LOW);
      }
      delay(1000);
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }else{
      Serial.println("this door is locked");
    }
  
  //publish lock status

  user = "";
  room = "";
  userID = "";
  
  delay(1000);
}

//callback
void messageReceived(String &topic, String &payload){
  Serial.println("Incoming: " + topic + " - " + payload);

  if(topic == sendLock && payload == "unlock"){
    if(lock == 0){
      //client.publish(lockHandling, "This door is already unlocked");
    }else{
      //client.publish(lockHandling, "Door is unlocked");
      lock = 0;
      client.publish(lockStat, "0");
    }
  }else if(topic == sendLock && payload == "lock"){
    if(lock == 1){
      //client.publish(lockHandling, "this door is already locked");
    }else{
     // client.publish(lockHandling, "Door is locked");
      lock = 1;
      client.publish(lockStat, "1");
    }
  }else if(topic == sendLock){
    if(payload == "open"){
      if(lock == 0){
        if(door == 1)
          openDoor();
        else
          Serial.println("door already open");
      }else{
        Serial.println("door is locked by server");
      }
    }else if(payload == "close"){
      if(lock == 0){
        if(door == 0)
          lockDoor();
        else
          Serial.println("door already locked");
      }else{
        Serial.println("door is locked by server");
      }
    }
  }else if(topic == statReq){
    client.publish(lockStat,(String) lock);
    client.publish(doorStat,(String) door);
  }
}

void lockDoor(){
  int pos = 120;
  m.write(pos);
  door = 1;
  client.publish(doorStat, "1");
  //client.publish(sendUser, user);
  //client.publish(sendID, userID);
  Serial.println("door locked");
  
}

void openDoor(){
  int pos = 0;
  m.write(pos);
  door = 0;
  client.publish(doorStat, "0");
  //client.publish(sendUser, user);
  //client.publish(sendID, userID);
  Serial.println("door unlocked");
  
}

