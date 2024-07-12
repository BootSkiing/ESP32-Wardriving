#include <WiFi.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include <TimeLib.h>
#include <HardwareSerial.h>

#define UTC_offset -5   // EST
#define SD_CS 29        // VSPI SS Pin (for SD Card)
#define TX2 17          // Transmit Pin (for GPS)
#define RX2 16          // Recieve Pin (for GPS)
#define LOG_RATE 50    // How often to log (milliseconds)
char currentTime[5];

String logFileName = "";
int networks = 0;

// fill out these pins and set it up and stuff (SPI)
#define REASSIGN_PINS
int sck = -1;
int miso = -1;
int mosi = -1;
int cs = -1;

HardwareSerial SerialGPS(2); // GPS UART on Serial 2
TinyGPSPlus tinyGPS;
const int Serial2RX = 16;
const int Serial2TX = 17;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, Serial2RX, Serial2TX);
  //while (!Serial);

  WiFi.mode(WIFI_STA); WiFi.disconnect();

  Serial.println("Starting Wardriver...");

  /* Initialize SD Card Reader */
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR: SD card not found!");
    while (!SD.begin(SD_CS));
  }
  Serial.println("INFO: SD Card found! Initilizing...");
  initializeSD();

  /* initialize GPS */
  delay(5000);
  if (SerialGPS.available() > 0) {
    Serial.println("INFO: GPS Found!");
  }
  else {
    Serial.println("ERROR: GPS not found!");
  }
  while (!tinyGPS.location.isValid()) {
    Serial.println(tinyGPS.location.isValid());
    smartDelay(500);
  }
  Serial.println("INFO: Current location: (" + String(tinyGPS.location.lat(), 5) + "," + String(tinyGPS.location.lng(), 5) + ")");
  Serial.println("INFO: Finished setup!");
}

void loop() {
  Serial.println("INFO: Starting new loop");
  if (tinyGPS.location.isValid()) {
    setTime(tinyGPS.time.hour(), tinyGPS.time.minute(), tinyGPS.time.second(), tinyGPS.date.day(), tinyGPS.date.month(), tinyGPS.date.year());
    adjustTime(UTC_offset * SECS_PER_HOUR);  
    lookForNetworks();
  }
  smartDelay(LOG_RATE);
  if (millis() > 5000 && tinyGPS.charsProcessed() < 10)
    Serial.println("No GPS data received: check wiring");
}

void initializeSD() { // create new CSV file and add WiGLE headers
  int i = 0; logFileName = "log0.csv";
  while (SD.exists(logFileName)) {
    i++; logFileName = "log" + String(i) + ".csv";
  }
  File logFile = SD.open(logFileName, FILE_WRITE);
  Serial.println("INFO: Created " + logFileName);
  if (logFile) {
    logFile.println("WigleWifi-1.4,appRelease=2.53,model=D1-Mini-Pro,release=0.0.0,device=NetDash,display=SSD1306,board=ESP8266,brand=Wemos");
    logFile.println("MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type");
  }
  logFile.close();
}

void lookForNetworks() {
  Serial.println("INFO: Looking for networks...");
  sprintf_P(currentTime, PSTR("%02d:%02d"),hour(),minute());

  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("INFO: No networks found :(");
  }
  else {
    Serial.println("INFO: " + String(n) + " networks found!");
    for (int i = 0; i < n; ++i) {
      if ((WiFi.channel(i) > 0) && (WiFi.channel(i) < 15)) {

        sprintf_P(currentTime, PSTR("%02d:%02d"),hour(),minute());  
        networks++;

        String ssid = WiFi.SSID(i);
        String bssid = WiFi.BSSIDstr(i);
        bssid.replace(":", "");
        String enc = getEncryption(i);

        Serial.println("INFO: " + ssid);
        Serial.println("INFO:   " + enc + ", " + bssid);

        File logFile = SD.open(logFileName, FILE_WRITE);
        logFile.print(bssid);  logFile.print(',');
        logFile.print(ssid); logFile.print(',');
        logFile.print(enc); logFile.print(',');
        logFile.print(year());   logFile.print('-');
        logFile.print(month());  logFile.print('-');
        logFile.print(day());    logFile.print(' ');
        logFile.print(hour());   logFile.print(':');
        logFile.print(minute()); logFile.print(':');
        logFile.print(second()); logFile.print(',');
        logFile.print(WiFi.channel(i)); logFile.print(',');
        logFile.print(WiFi.RSSI(i)); logFile.print(',');
        logFile.print(tinyGPS.location.lat(), 6); logFile.print(',');
        logFile.print(tinyGPS.location.lng(), 6); logFile.print(',');
        logFile.print(tinyGPS.altitude.meters(), 1); logFile.print(',');
        logFile.print(tinyGPS.hdop.value(), 1); logFile.print(',');
        logFile.println("WIFI");
        logFile.close();
      }
    }
    Serial.println("INFO: " + String(networks) + " total networks collected");
  }
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (SerialGPS.available())
      tinyGPS.encode(SerialGPS.read());
  } while (millis() - start < ms);
}

String getEncryption(uint8_t network) { // return encryption for WiGLE or print
  byte encryption = WiFi.encryptionType(network);
  switch (encryption) {
    case 2:
      return "[WPA-PSK-CCMP+TKIP][ESS]";
    case 5:
      return "[WEP][ESS]";
    case 4:
      return "[WPA2-PSK-CCMP+TKIP][ESS]";
    case 7:
      return "[ESS]";
  }
  return "[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][ESS]";      
}
