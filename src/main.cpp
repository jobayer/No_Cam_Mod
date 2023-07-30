#include <Arduino.h>
#include <MFRC522.h> // RFID

// do not change here!
#define BAUD_RATE 115200

// ultrasonic sensors
#define US1_TRIG 12
#define US1_ECHO 13
#define US2_TRIG 27
#define US2_ECHO 14

// rfid module
#define RFID_SPI 21
#define RFID_RESET 26
#define RFID_UID "73eb3c15"

int userAuthenticated;

// function declaration
void initRFID();
String rfidUID(MFRC522);
void checkForAuthReq();
void checkBinLevel();
float objectDist(int);
void serialPrint(String, int);

MFRC522 rfid(RFID_SPI, RFID_RESET);

void preproc()
{
  Serial.begin(BAUD_RATE);
}

void setupPins()
{
  pinMode(US1_TRIG, OUTPUT);
  pinMode(US1_ECHO, INPUT);
  pinMode(US2_TRIG, OUTPUT);
  pinMode(US2_ECHO, INPUT);
}

void initVars()
{
  userAuthenticated = 0;
}

void setup()
{
  preproc();
  setupPins();
  initVars();
  initRFID();
}

void loop()
{

  // rfid-based auth
  checkForAuthReq();

  // us-based bin level
  if (userAuthenticated == 1)
  {
    checkBinLevel();
  }

  delay(1000);
}

// ==== rfid ====

void initRFID()
{
  SPI.begin();
  rfid.PCD_Init();
  serialPrint("Waiting for new card approach...", 1);
  delay(4);
}

String rfidUID(MFRC522 rfid)
{
  String data;
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    data.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    data.concat(String(rfid.uid.uidByte[i], HEX));
  }
  data.trim();
  data.replace(" ", "");
  return data;
}

void stopRFID(MFRC522 rfid)
{
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void checkForAuthReq()
{
  if (!rfid.PICC_IsNewCardPresent())
  { // no card swiped
    return;
  }

  if (!rfid.PICC_ReadCardSerial())
  { // no valid card found
    return;
  }

  String uid = rfidUID(rfid);

  userAuthenticated = uid.equals(RFID_UID);

  if (userAuthenticated == 1)
  {
    serialPrint("User autheticated...", 1);
    serialPrint("Proceed with your waste...", 1);
  }
  else
  {
    serialPrint("User not autheticated!", 1);
  }

  stopRFID(rfid);
}

// ==== ultrasonic sensor ====

void checkBinLevel()
{
  float dist = objectDist(1);
  serialPrint("Distance: ", 0);
  serialPrint(String(dist), 0);
  serialPrint(" cm", 1);
}

float objectDist(int usModule)
{
  float dist;
  int trigPin;
  int echoPin;

  switch (usModule)
  {
  case 1:
    trigPin = US1_TRIG;
    echoPin = US1_ECHO;
    break;
  case 2:
    trigPin = US2_TRIG;
    echoPin = US2_ECHO;
    break;
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long pulseDuration = pulseIn(echoPin, HIGH);

  dist = pulseDuration * 0.01715;

  return dist; // distance in centimeter
}

// ==== misc ====

void serialPrint(String msg, int newLine)
{
  if (newLine == 1)
  {
    Serial.println(msg);
  }
  else
    Serial.print(msg);
}