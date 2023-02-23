/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
  https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-get-post-arduino/
  http://www.steves-internet-guide.com/using-arduino-pubsub-mqtt-client/
*********/

#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "######";
const char* password = "######";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 30 seconds (30000)
unsigned long timerDelay = 30000;
char myMAC[17];
char myIP[16];

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.60.20";
const char* clientName = "ESP32-Client01";
const char* statusTopic = "esp32/status";
const char* ledTopic = "esp32/client01/led";
const char* buttonTopic = "esp32/client01/button";
char pubMsg[40];

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long resetEsp = 0;
char msg[50];
int value = 0;
int connectFailed = 0;
// how many Buttons with LED
#define numBtns 1
#define espPin 2

// Variables will change:
int btnState[numBtns];
int lastBtnState[numBtns];
//char keyPress[numBtns] = {0};
int btnPin[numBtns] = {14};
int ledPin[numBtns] = {25};
int gndPin[numBtns*2] = {32, 27};
int reading[numBtns];
int pinNum;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[numBtns];  // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);
  // default settings
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //Setup Buttons and LEDS
  for (int i = 0; i < numBtns; i++) {
    pinMode(btnPin[i], INPUT_PULLUP);
    pinMode(ledPin[i], OUTPUT);
    digitalWrite(ledPin[i], HIGH);
    lastBtnState[i] = HIGH;
  }
  
  for (int i = 0; i < numBtns*2; i++) {
    pinMode(gndPin[i], OUTPUT);
    digitalWrite(gndPin[i], LOW);
  }
  pinMode(espPin, OUTPUT);
  digitalWrite(espPin, HIGH);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  sprintf(myIP, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  sprintf(myMAC, "%d-%d-%d-%d-%d-%d", WiFi.macAddress()[0], WiFi.macAddress()[1], WiFi.macAddress()[2], WiFi.macAddress()[3], WiFi.macAddress()[4], WiFi.macAddress()[5]);
  Serial.println(myIP);
}

void callback(char* topic, byte* message, unsigned int length) {
  String receivedTopic = String(topic);
  int ledNum;
  int ledState;
  int intMsg;
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  // Feel free to add more if statements to control more GPIOs with MQTT

  // The message to be received will be 2 digits. The 1º digit representes de LED Number anda the
  // 2º Digit is the state "0" for Off and "1" for On. 
  // Changes the output state according to the message
  if (messageTemp.length() == 2) {
    intMsg = messageTemp.toInt();
    ledNum = intMsg / 10;
    ledState = intMsg % 10;
    Serial.print("Changing output of LED ");
    Serial.print(ledNum);
    Serial.print(" to ");
    if(ledState == 1){
      Serial.println("On");
      digitalWrite(ledPin[ledNum], HIGH);
    }
    else if(ledState == 0){
      Serial.println("Off");
      digitalWrite(ledPin[ledNum], LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Subscribe
      Serial.print("Subscribing to ");
      Serial.println(ledTopic);
      client.subscribe(ledTopic, 1);
      sprintf(pubMsg, "client Name is %s", clientName);
      client.publish(statusTopic, pubMsg, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      connectFailed++;
    }
    Serial.print("Connect Failed = ");
    Serial.println(connectFailed);
    if (connectFailed > 10){
      ESP.restart();
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // read the state of the switch into a local variable:
  for (int i = 0; i < numBtns; i++) {
    reading[i] = digitalRead(btnPin[i]);
    //Serial.println(reading[i]);
  // check to see if you just pressed the button
  // (i.e. the input went from HIGH to LOW), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  
    if (reading[i] != lastBtnState[i]) {
      // reset the debouncing timer
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:
  
      // if the button state has changed:
      if (reading[i] != btnState[i]) {
        btnState[i] = reading[i];
  
        // only toggle the LED if the new button state is HIGH
        if (btnState[i] == LOW) {
          Serial.println(i);
          sprintf(pubMsg, "%d", i);
          client.publish(buttonTopic, pubMsg, 1);
          sprintf(pubMsg, "Button %d Pressed", i);
          lastTime = lastTime - timerDelay;
        }
      }
    }

    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastBtnState[i] = reading[i];
  }

  //Reset after one hour
  if (millis() - resetEsp > 3600000) {
    ESP.restart();
  }
}
