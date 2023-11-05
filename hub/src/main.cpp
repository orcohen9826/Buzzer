#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <TM1637Display.h>

//Setting
const int resetTime = 5000; // 5 seconds


// Predefined MAC addresses of the buzzers
const int predefinedBuzzersCount = 5;
uint8_t predefinedBuzzers[predefinedBuzzersCount][6] = { 
  // your predefined MAC addresses here, for example:
  {0x50, 0x02, 0x91, 0xFC, 0xEE, 0x15},//buzzer 1
  {0x84, 0xCC, 0xA8, 0xAA, 0x0F, 0x18},//buzzer 2
  {0x84, 0xCC, 0xA8, 0xA9, 0xAC, 0x8E},//buzzer 3
  {0x84, 0xCC, 0xA8, 0xA8, 0xEF, 0x47},//buzzer 4
  {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},//buzzer 5
};

// function declarations
void OnDataRecv(const uint8_t *macAddr, const uint8_t *data, int dataLen);
void addBuzzer(const uint8_t *macAddr);
void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t sendStatus);
void resetBuzzers();
void prinToLCD(String str , int row , int col = 0);
int getBuzzerName(const uint8_t *macAddr);
void printMacAddress(const uint8_t *macAddr);
void printDetail(uint8_t type, int value);



// Messages to send to buzzers
const char *resetMsg = "ResetBuzzer";
const char *activateMsg = "Activate";
const char *deactivateMsg = "Deactivate";
const char *connectedMsg = "Connected";
const char *connectingMsg = "Connecting";
const char *pressedMsg = "Pressed";

// LCD MAIN SCREEN
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// DFPlayer - MP3
SoftwareSerial mySoftwareSerial(27 , 26);  // RX, TX
DFRobotDFPlayerMini myDFPlayer;
const int mp3Volum = 25; // 0-30
int soundForAll = 99; // the mp3 file 0099 for all buzzers
int initSound = 100; // the mp3 file 0100 for init sound
boolean differentSound = true ; // if true the sound will be different for each buzzer


//TM1637  4-Digit Display.
const int CLK = 18; //pins definitions for TM1637 and can be changed to other ports
const int DIO = 19;
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.



// MAC address of the receivers
uint8_t buzzers[10][6]; // Stores up to 10 buzzer MAC addresses
int buzzerCount = 0;
unsigned long lastPressTime = 0;
boolean firstTime = true;



void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    prinToLCD("ESP-NOW initialization failed", 0 , 0);
    return;
  }
  else{
    Serial.println("ESP-NOW initialization success");
    }

  esp_now_register_recv_cb(OnDataRecv);
  //esp_now_register_send_cb(OnDataSent);
  //Serial.println("MAC address: " + WiFi.macAddress()); //print the mac address of the esp32 for new device if needed


  //Initialize the LCD
  lcd.init();
  lcd.backlight();
  prinToLCD("HELLO OHAD!", 0,0);
  //lcd.clear();


  //Initialize the DFPlayer try 5 times
  mySoftwareSerial.begin(9600);
  Serial.println("Initializing DFPlayer...");
  int retries = 0;
  do{
    retries++;
    if(retries >0){
      Serial.printf("Mp3 unable to begin");
    }
    delay(1000);

  }while (!myDFPlayer.begin(mySoftwareSerial) && retries < 5);

  if (retries >= 5){
    Serial.println("Unable to begin continue without MP3:");
    prinToLCD("Error, insert SD card", 1 , 0);
    
  }
  else{
    prinToLCD("MP3 ready", 1 , 0);
    Serial.println("DFPlayer Mini ready.");
  }
  myDFPlayer.volume(mp3Volum);  // Set volume between 0 and 30  
  myDFPlayer.playMp3Folder(initSound);   // play inishlize sound
  

  //Initialize the TM1637
  display.setBrightness(0x0f);
  display.showNumberDec(0, false);
  display.clear();


}




//the function build to work without predefine buzzers and can register new buzzers automaticly. the adjustmants for predefine buzzers 
// is in the function getBuzzerName 
void OnDataRecv(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
  esp_err_t result;
  char* receivedMsg = new char[dataLen + 1];
  memcpy(receivedMsg, data, dataLen);
  receivedMsg[dataLen] = '\0';// add null to the end of the string

  // if massage == connect , add buzer and break
  if(strcmp(receivedMsg , connectingMsg) == 0){
    addBuzzer(macAddr);
    delay(25);//maybe not needed
    result = esp_now_send(macAddr, (uint8_t *)connectedMsg, strlen(connectedMsg));
    return;
  }
  if(firstTime){// flag to prevent multiple presses
   firstTime = false;// set flag to false so only the first press will be counted
   
   Serial.println("\n");
   Serial.println("\n");

   // Record the time and buzzer of the last press
   lastPressTime = millis();
   // Add the buzzer if it's new
   //addBuzzer(macAddr);// for automaticly register new buzzers
    





    myDFPlayer.volume(mp3Volum);
    if(differentSound){
      myDFPlayer.play(getBuzzerName(macAddr));// play the mp3 file of the buzzer that pressed by the buzzer actual number
    //myDFPlayer.playMp3Folder(getBuzzerName(macAddr));// play the mp3 file of the buzzer that pressed by the buzzer actual number
    }
    else{
      //myDFPlayer.play(soundForAll);// play the mp3 file of the buzzer that pressed by the buzzer actual number
      myDFPlayer.playMp3Folder(soundForAll);// play the mp3 file of the buzzer that pressed by the buzzer actual number
    }

   // Reply to the buzzer that just pressed
   Serial.println("Sending Activate reply...");
   result = esp_now_send(macAddr, (uint8_t *)activateMsg, strlen(activateMsg));
   Serial.printf("Send Status: %s\n", esp_err_to_name(result));
   printMacAddress(macAddr);  // Print the MAC address of the buzzer that pressed

   // Reply "Deactivate" to all other buzzers
   Serial.println("Sending Deactivate reply...");
   uint8_t firstPress = 0;
   int retries = 0;
   for (int i = 0; i < buzzerCount; i++) {
     if (memcmp(buzzers[i], macAddr, 6) != 0) {
       do{
         result = esp_now_send(buzzers[i], (uint8_t *)deactivateMsg, strlen(deactivateMsg));
         if(result == ESP_OK){
           Serial.printf("Send Status: %s\n", esp_err_to_name(result));  
           break;      
         }
         Serial.printf("attempt number: %d\n", retries);
         retries++;
         delay(10);
       }while(retries < 3);
       Serial.printf("Send Status: %s\n", esp_err_to_name(result));        
     }
     
   }
   Serial.print("Buzzer that pressed first is number: ");
   Serial.println(getBuzzerName(macAddr));
   Serial.println("\n");
   lcd.clear();
   prinToLCD("Last Winner : " + String(getBuzzerName(macAddr)), 1 , 0);
   display.showNumberDec(getBuzzerName(macAddr), false);
   
  }
  
}










void loop() {
  // Check if it's time to reset the buzzers
  
  if (lastPressTime > 0 && millis() - lastPressTime >= resetTime) {
    resetBuzzers();
    lastPressTime = 0;
  }
}




void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t sendStatus) {
  if (sendStatus == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Message sent to buzzer");
  }
  else {
    Serial.println("Message failed to send");
  }
}

void resetBuzzers(){//sand reset message to all buzzers
  esp_err_t result;
  Serial.print("resetting buzzers ");
  for (int i = 0; i < buzzerCount; i++) { 
    result = esp_now_send(buzzers[i], (uint8_t *)resetMsg, strlen(resetMsg));
  }
  Serial.printf("Send Status: %s\n", esp_err_to_name(result));
  lastPressTime = 0;
  prinToLCD("Ready", 0 ,0 );
  firstTime = true;
  display.clear();
}

void addBuzzer(const uint8_t *macAddr) {//add buzzer to a temprary list of buzzers
  // Check if this MAC is already known
  for (int i = 0; i < buzzerCount; i++) {
    if (memcmp(buzzers[i], macAddr, 6) == 0) {
      return;
    }
  }
  
  // Add the new MAC to the list if there's room
  if (buzzerCount < 10) {
    memcpy(buzzers[buzzerCount], macAddr, 6);
    buzzerCount++;

    // Create and initialize peer information
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, macAddr, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add to peer list
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }

    Serial.print("Adding buzzer number: ");
    Serial.println(buzzerCount);
    prinToLCD("Connected B': " + String(buzzerCount), 0 , 0);
    //prinToLCD("buzzers: " + String(buzzerCount), 1 , 0);

  }
}


void prinToLCD(String str , int row , int col){ // row 0/1 , col 0-15  
   //lcd.clear();
   lcd.setCursor(col, row);
   lcd.print(str);
}

int getBuzzerName(const uint8_t *macAddr) {//return 0 if not found or 1-10 if found
  for (int i = 0; i < predefinedBuzzersCount; i++) { // Adjust the loop limit according to the number of predefined buzzers
    if (memcmp(predefinedBuzzers[i], macAddr, 6) == 0) {
      return i + 1;  // Return buzzer number, starting from 1
    }
  }
  return 0; // means buzzer not found
}

void printMacAddress(const uint8_t *macAddr) {//print the mac address in sutable format for esp now
   for (int i = 0; i < 6; i++) {
     Serial.print("0x");
     if (macAddr[i] < 16) { // Print a leading zero for single-digit hex values
       Serial.print("0");
     }
     Serial.print(macAddr[i], HEX);
     if (i < 5) { // Print a comma except after the last item
       Serial.print(", ");
     }
   }
}

// void printDetail(uint8_t type, int value){
//   switch (type) {
//     case TimeOut:
//       Serial.println(F("Time Out!"));
//       break;
//     case WrongStack:
//       Serial.println(F("Stack Wrong!"));
//       break;
//     case DFPlayerCardInserted:
//       Serial.println(F("Card Inserted!"));
//       break;
//     case DFPlayerCardRemoved:
//       Serial.println(F("Card Removed!"));
//       break;
//     case DFPlayerCardOnline:
//       Serial.println(F("Card Online!"));
//       break;
//     case DFPlayerUSBInserted:
//       Serial.println("USB Inserted!");
//       break;
//     case DFPlayerUSBRemoved:
//       Serial.println("USB Removed!");
//       break;
//     case DFPlayerPlayFinished:
//       Serial.print(F("Number:"));
//       Serial.print(value);
//       Serial.println(F(" Play Finished!"));
//       break;
//     case DFPlayerError:
//       Serial.print(F("DFPlayerError:"));
//       switch (value) {
//         case Busy:
//           Serial.println(F("Card not found"));
//           break;
//         case Sleeping:
//           Serial.println(F("Sleeping"));
//           break;
//         case SerialWrongStack:
//           Serial.println(F("Get Wrong Stack"));
//           break;
//         case CheckSumNotMatch:
//           Serial.println(F("Check Sum Not Match"));
//           break;
//         case FileIndexOut:
//           Serial.println(F("File Index Out of Bound"));
//           break;
//         case FileMismatch:
//           Serial.println(F("Cannot Find File"));
//           break;
//         case Advertise:
//           Serial.println(F("In Advertise"));
//           break;
//         default:
//           break;
//       }
//       break;
//     default:
//       break;
//   }
  
// }