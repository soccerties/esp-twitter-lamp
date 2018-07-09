#include <ESP8266mDNS.h>
#include <Client.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h> 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <TwitterWebAPI.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// user options
int redPin = 14;
int greenPin = 12;
int bluePin = 15;
int period = 5000; // fade in fade out period
String currentColor = "white"; // starting LED color
static char const twitterSearchString[] = "mongo OR cassandra OR dse OR datastax";
unsigned long twi_update_interval = 20; // seconds

// twitter auth tokens can be created here https://apps.twitter.com/app/new
static char const consumer_key[]    = "";
static char const consumer_sec[]    = "";
static char const accesstoken[]     = "";
static char const accesstoken_sec[] = "";

const char *ntp_server = "pool.ntp.org";
int timezone = -7;

int value;  // fade value holder

unsigned long api_mtbs = twi_update_interval * 1000; //mean time between api requests
unsigned long api_lasttime = 0; 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, timezone*3600, 60000);  // NTP server pool, offset (in seconds), update interval (in milliseconds)
TwitterClient tcr(timeClient, consumer_key, consumer_sec, accesstoken, accesstoken_sec);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  setColor(750, 750, 750);

  WiFiManager wifiManager;
  wifiManager.autoConnect("esp-lamp");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  tcr.startNTP();
}

void loop() {
  // calculate LED brightness 
  int value = 500+400*cos(2*PI/period*millis());
  
  if (currentColor == "red") {
    setColor(value, 0, 0);  
  } else if (currentColor == "green") {
    setColor(0, value, 0);
  } else if (currentColor == "blue") {
    setColor(0, 0, value);
  } else if (currentColor == "white") {
    setColor(value, value, value);
  }

  // skip API call if less than interval
  if (millis() > api_lasttime + api_mtbs) {
    digitalWrite(LED_BUILTIN, LOW);
    String tmsg = tcr.searchTwitter(twitterSearchString);
    Serial.println(tmsg);
    
    String foundtweet = parseJSON(tmsg);
    foundtweet.toLowerCase();
    Serial.println(foundtweet);
    
    if(foundtweet.indexOf("mongo") >= 0) {
      Serial.println("found Mongo!");
      currentColor = "green";
    } else if (foundtweet.indexOf("cassandra") >= 0){
      Serial.println("found Cassandra!");
      currentColor = "blue";
    } else if (foundtweet.indexOf("dse") >= 0 || foundtweet.indexOf("datastax") >= 0) {
      Serial.println("found Datastax!");
      currentColor = "red";
    } else {
      Serial.println("nothing found in tweet");
      currentColor = "white";
    }
    api_lasttime = millis();
  }
  
  yield();
  delay(20);
  digitalWrite(LED_BUILTIN, HIGH);
}

String parseJSON(String msg) {
  String myreturn;
  const char* msg2 = const_cast <char*> (msg.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject& response = jsonBuffer.parseObject(msg2);
  
  if (!response.success()) {
    return "No tweet found";
  }

  if (response.containsKey("statuses")) {
    String usert = response["statuses"][0]["user"]["screen_name"];
    String text = response["statuses"][0]["text"];
    return usert + " tweeted: " + text;
  } else if(response.containsKey("errors")) {
    String err = response["errors"][0];
    return err;
  } else {
    Serial.println("Error parsing response");
    return "";
  }
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}


