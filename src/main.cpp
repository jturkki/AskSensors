/*
* Mittaa BME280 sensorilla lämpötilaa, kosteutta sekä ilmanpainetta viiden minuutin välein,
* ja lähettää tiedot asksensors.com sivustolle.
*
* TIES536 Sulautettu Internet
* Harjoitus 7
* Jyri Turkki
*/

#include <Arduino.h>

/*
  Repeating WiFi Web Client

 This sketch connects to a a web server and makes a request
 using a WiFi equipped Arduino board.

 created 23 April 2012
 modified 31 May 2012
 by Tom Igoe
 modified 13 Jan 2014
 by Federico Vanzati
 modified 1 Nov 2022
 by Jukka Ihalainen
 modified 5 Nov 2022
 by Jyri Turkki

 http://www.arduino.cc/en/Tutorial/WifiWebClientRepeating
 This code is in the public domain.
 */
 

#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Funktioiden esittelyt
void printWifiStatus();
void mittaa();
void httpRequest();


char ssid[] = "netti";        // network SSID (name)
char pass[] = "";                       // network password 
char apikey1[] = "995t1C7CIASyIrsYRQpZyfs42TPzH7jU";   //apikey of first sensor
int keyIndex = 0;                       // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the WiFi client library and BME280
WiFiClient client;
Adafruit_BME280 bme;

// server address:
char server[] = "asksensors.com";

volatile int temp, pressure, humidity;
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 5L * 60L * 1000L; // delay 5 min between updates, in milliseconds

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  Serial.println(F("BME280 test"));
  
  unsigned statusBme = bme.begin(0x76);    // starting BME280 and testing if found

  if (!statusBme) {
    Serial.println("Could not find a valid BME280 sensor");
    Serial.print("SensorID was: 0x"); Serial.println("bme.sensorID(), 16");

    while(1) delay(10);
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

   String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}



void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  
  //After 5 min, get sensor readings and connect to server:
  if ((millis() - lastConnectionTime) > postingInterval) {

    mittaa();       // read BME280

    httpRequest(); //request function for all the other sensors
  }

  
}
 

//	This method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the NINA module
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");

    String url = "https://asksensors.com/api.asksensors/write/"; //For sensor
    url += apikey1;
    url += "?module1="; //Pressure: line
    url += pressure;
    url += "&module2="; //Humidity: line
    url += humidity;
    url += "&module3="; //Temperature: line
    url += temp;
    url += "&module4="; //Temperature: table
    url += temp;
    
    // send the HTTP GET request:
    Serial.print("********** requesting URL: ");
      Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n");
    client.println();
  
    
    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


void mittaa(){               // read BME280 sensor for temperature, pressure and humidity
  temp = bme.readTemperature();
  pressure = ( bme.readPressure() / 100.0F);
  humidity = bme.readHumidity();  
}



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
