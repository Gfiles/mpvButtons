/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
  https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-get-post-arduino/
  https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2018/08/ESP32-DOIT-DEVKIT-V1-Board-Pinout-30-GPIOs-Copy.png?quality=100&strip=all&ssl=1
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "YDreams Eventos";
const char* password = "YDreams2021";

//YOWL our Domain name with URL path or IP address with path
String serverName = "https://yowl.ydreams.global/senninha/inc/ClientDataUpdate.php";
String espStatus = "Runnning";
int app_status = 1;
bool firstHttp = true;
String serverPath;

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
#define numBtns 3
#define espPin 2

// Variables will change:
int btnState[numBtns];
int lastBtnState[numBtns];
//char keyPress[numBtns] = {0};
int btnPin[] = {13, 25, 34};
int ledPin[] = {12};
int gndPin[] = {14, 26, 32, 33};
int reading[numBtns];
int pinNum;
int espLEDState;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[numBtns];  // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

String upTimeText() {
  long timeNow = millis();
  String timeText = "";
  int days, hours, mins, sec;
  if (timeNow > 86400000) {
    days = timeNow/86400000;
    timeText = days + " d ";
    timeNow = timeNow - days*86400000;
  }
  if (timeNow > 3600000) {
    hours = timeNow/3600000;
    timeText = timeText + hours + " hr ";
    timeNow = timeNow - hours*3600000;
  }
  if (timeNow > 60000) {
    mins = timeNow/60000;
    timeText = timeText + mins + " min ";
    timeNow = timeNow - mins*60000;
  }
  sec = timeNow/1000;
  timeText = timeText + sec + " sec";
  return timeText;
}

void setup() {
  Serial.begin(115200);
  // default settings
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //Setup Buttons and LEDS
  for (int i = 0; i<sizeof btnPin/sizeof ledPin[0]; i++) {
    pinMode(btnPin[i], INPUT_PULLUP);
  }

  for (int i = 0; i<sizeof ledPin/sizeof ledPin[0]; i++) {
    pinMode(ledPin[i], OUTPUT);
    digitalWrite(ledPin[i], HIGH);
    lastBtnState[i] = HIGH;
  }
  
  for (int i = 0; i<sizeof gndPin/sizeof gndPin[0]; i++) {
    pinMode(gndPin[i], OUTPUT);
    digitalWrite(gndPin[i], LOW);
  }
  pinMode(espPin, OUTPUT);
  digitalWrite(espPin, HIGH);
  espLEDState = HIGH;
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
    connectFailed++;
    if (connectFailed > 10) {
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  sprintf(myIP, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  sprintf(myMAC, "%d-%d-%d-%d-%d-%d", WiFi.macAddress()[0], WiFi.macAddress()[1], WiFi.macAddress()[2], WiFi.macAddress()[3], WiFi.macAddress()[4], WiFi.macAddress()[5]);
  //sprintf(subscribeTopic, "esp32/%s/#", myMAC);
  Serial.println(myIP);
  //sprintf(ledTopic, "esp32/%s/led/", myMAC);
  //sprintf(buttonTopic, "esp32/%s/button/", myMAC);
  //sprintf(clientName, "ESP32-%s" , myMAC);  
}

void callback(char* topic, byte* message, unsigned int length) {
  String receivedTopic = String(topic);
  int ledNum;
  int ledState;
  int intMsg;
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  Serial.println(receivedTopic);
  //Serial.println(subscribeTopic);
  //receivedTopic.replace(ledTopic , "");
  //receivedTopic.replace("led/" , "");
  Serial.println(receivedTopic);
  //Serial.println(receivedTopic.length());
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
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
    //if (client.connect("ESP32Client")) {
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Subscribe
      //sprintf(charSub, "%s#", subscribeTopic);
      Serial.print("Subscribing to ");
      Serial.println(ledTopic);
      client.subscribe(ledTopic);
      sprintf(pubMsg, "client Name is %s", clientName);
      client.publish(statusTopic, pubMsg, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      connectFailed++;
      if (connectFailed > 10){
        ESP.restart();
      }
    }
    Serial.print("Connect Failed = ");
    Serial.println(connectFailed);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (espLEDState) {
    espLEDState = LOW;
  } else {
    espLEDState = HIGH;
  }
  digitalWrite(espPin, espLEDState);
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
          espStatus = pubMsg;
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
  
  //Send an HTTP POST request every 1 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      if (firstHttp) {
        serverPath = serverName + "?mac=" + myMAC + "&client_name=" + clientName + "&uptime=" + upTimeText() + "&app_name=" + espStatus + "&type=Wi-Fi&os_version=Arduino&app_status=" + app_status + "&yowl_name=Arduino.2023.2.8&client_ip=" + myIP;  
        firstHttp = false;
      } else {
        serverPath = serverName + "?mac=" + myMAC + "&uptime=" + upTimeText() + "&app_name=" + espStatus + "&app_status=" + app_status;
        espStatus = "Running";
      }
      // Your Domain name with URL path or IP address with path
      serverPath.replace(' ', '+');
      Serial.println(serverPath);
      http.begin(serverPath.c_str());
      //http.addHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:55.0) Gecko/20100101 Firefox/55.0");
      
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        if (espStatus == "Rebooting") {
          ESP.restart();
        }
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        if (payload == "restart-app" or payload == "reboot-computer") {
          espStatus = "Rebooting";
          app_status = 0;
          lastTime = lastTime - timerDelay;
        }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
    sprintf(pubMsg, "%s is Alive", clientName);
    client.publish("esp32/status", pubMsg);
  }
}
