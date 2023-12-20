#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_NeoPixel.h>

int conectedStatus = -1;  // have change recently
int lastSendStatus = -1;
// Define variables
const int minDelay = 50;
const int maxDelay = 250;
// led strip
const int LED_COUNT = 10 ;
const int RGB_BRIGHTNESS = 200;// 0-255

// Define the pins used
const int buttonPin = D3;  // Button connected to GPIO4 (D2)
const int  LED_PIN = D4 ; //LED connected to GPIO14 (D3)


// Define massages
const char *resetMsg = "ResetBuzzer";
const char *activateMsg = "Activate";
const char *deactivateMsg = "Deactivate";
const char *pressedMsg = "Pressed";
const char *connectedMsg = "Connected";
const char *connectingMsg = "Connecting";
 


// led strip
Adafruit_NeoPixel NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// put function declarations here:
void set_led_color(String color);
void sendPressedMsg();



// Button state
int buttonState = 0;

// Define the MAC address of the receiver module
uint8_t remoteAddress[] = {0x7C, 0x9E, 0xBD, 0xF6, 0x69, 0x1C};

void OnDataSent(uint8_t *macAddr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    //Serial.println("Delivery success");
  } else {
    //Serial.println("Delivery failed");
  }
  lastSendStatus = sendStatus;
  //Serial.println("resultfromlasr: " + String(lastSendStatus));
}

void OnDataRecv(uint8_t *macAddr, uint8_t *data, uint8_t dataLen) {
  Serial.print(dataLen);
  data[dataLen] = '\0';// for preventing gibberish
  String receivedMsg = String((char*)data);

  Serial.print("Received: ");
  Serial.println(receivedMsg);

  if (strcmp(receivedMsg.c_str(), activateMsg) == 0) {
    set_led_color("RED");

    //Serial.println("Red LED activated.");
  } 
  if (strcmp(receivedMsg.c_str(), deactivateMsg) == 0) {
    set_led_color("OFF");
    //Serial.println("LEDs deactivated.");
  }
  if (strcmp(receivedMsg.c_str(), resetMsg) == 0) {
    set_led_color("GREEN");
    //Serial.println("Green LED activated.");
  }
  if(strcmp(receivedMsg.c_str(), connectedMsg) == 0){
    conectedStatus = 0;    
    //Serial.println("inishes secssfully.");
  }

}

void setup() {
  Serial.begin(9600);

  // Initialize pins
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    //Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);


  // Initialize random seed for delay between prassed messages
  randomSeed(analogRead(0));

  //led strip
  NeoPixel.begin();
  NeoPixel.setBrightness(RGB_BRIGHTNESS);
  set_led_color("BLUE");

  // Send initial connection message
  do{
    esp_now_send(remoteAddress, (uint8_t *)connectingMsg, strlen(connectingMsg));
    Serial.println("result: " + String(lastSendStatus));
    delay(200);// delay for ondatasent func to update lastSendStatus
    if (conectedStatus == 0) {
      //Serial.println("Send Status: Success");
      set_led_color("GREEN");
    } else {
      //Serial.println("Send Status: Failure");
      set_led_color("OFF");
      delay(500);
      set_led_color("BLUE");
      delay(500);

    }
  }while(conectedStatus != 0);
  
}


void loop() {
  // if buttomn fress one waite (with millis) until alaoud to press again
  time_t now = millis();
  static time_t lastPress = 0;
  if (now - lastPress < 1000) {
    return;
  }

  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) { // Button is pressed
    sendPressedMsg();
    lastPress = millis();
  }
  
}



void sendPressedMsg(){
  Serial.println("Button pressed");
  lastSendStatus = -1;// have change recently
  
  do{
    esp_now_send(remoteAddress, (uint8_t *)pressedMsg, strlen(pressedMsg));
    delay(10);// delay for ondatasent func to update lastSendStatus
    //Serial.println("result: " + String(lastSendStatus));
    if (lastSendStatus == 0) {
      //Serial.println("Send Status: Success");
      break;
    } else {
      //Serial.println("Send Status: Failure");
      delay(random(minDelay , maxDelay));
    }

  }while (lastSendStatus != 0);
}

void set_led_color(String color)
{
  Serial.println(color);
  uint32_t set_color = NeoPixel.Color(0, 0, 0);

  if (color == "RED")
    set_color = NeoPixel.Color(255, 0, 0);
  else if (color == "GREEN")
    set_color = NeoPixel.Color(0, 255, 0);
  else if (color == "BLUE")
    set_color = NeoPixel.Color(0, 0, 255);
   else if (color == "OFF")
    set_color = NeoPixel.Color(0, 0, 0);

  for(int i = 0; i < LED_COUNT; i++)
    NeoPixel.setPixelColor(i, set_color);

  NeoPixel.show();  // this is needed to update the pixels
}

