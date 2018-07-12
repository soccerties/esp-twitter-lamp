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
#include <Ticker.h>
#include <DoubleResetDetector.h>

// user options
int redPin = 14;
int greenPin = 12;
int bluePin = 15;
int period = 5000; // fade in fade out period
String currentColor = "white"; // starting LED color
static char const twitterSearchString[] = "mongo OR cassandra OR dse OR datastax";
unsigned long twitter_update_interval = 20; // seconds

// twitter auth
static char const consumer_key[]    = "";
static char const consumer_sec[]    = "";
static char const accesstoken[]     = "";
static char const accesstoken_sec[] = "";

const char *ntp_server = "pool.ntp.org";
int timezone = -7;

int brightness = 750;

bool CHECK_TWITTER = false;
Ticker tick;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, timezone*3600, 60000);  // NTP server pool, offset (in seconds), update interval (in milliseconds)
TwitterClient tcr(timeClient, consumer_key, consumer_sec, accesstoken, accesstoken_sec);

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 5

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Turn on built in LED until wifi is setup
  digitalWrite(LED_BUILTIN, LOW);
  updateLed();

  WiFiManager wifiManager;

  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected, erasing wifi configs");
    wifiManager.resetSettings();
  }

  // 3 min wifi setup timeout
  wifiManager.setTimeout(180);

  if (wifiManager.autoConnect("esp-lamp")) {
    tcr.startNTP();
    tick.attach(twitter_update_interval, checkTwitter);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("Timed out waiting for wifi setup");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // calculate LED brightness
  brightness = 500+400*cos(2*PI/period*millis());

  if (CHECK_TWITTER) {
    searchTwitter();
    CHECK_TWITTER = false;
  }

  updateLed();
  yield();
  drd.loop();
  delay(20);
}

void checkTwitter() {
  CHECK_TWITTER = true;
}

String searchTwitter() {
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
  }
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

void updateLed() {
  if (currentColor == "red") {
    setLedColor(brightness, 0, 0);
  } else if (currentColor == "green") {
    setLedColor(0, brightness, 0);
  } else if (currentColor == "blue") {
    setLedColor(0, 0, brightness);
  } else if (currentColor == "white") {
    setLedColor(brightness, brightness, brightness);
  }
}

void setLedColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}
