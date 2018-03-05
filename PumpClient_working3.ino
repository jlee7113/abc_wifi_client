/*Implements an ESP8266 that connects to a remote TCP server
 *
 */ 
//to do: add messaging for lost connection
#include <ESP8266WiFi.h>
#include <Arduino.h>

WiFiClient client;

//Specify Wifi network credentials
char ssid[] = "CookieMonster";
char password[] = "d4tt3Bay0!";

//Specify Host information 
String hostip = ""; //global var for host IP
//const char* server = "162.197.58.153"; //remote ip (default)
int portnum = 31337;

//flags used in user setup routine to determine when to exit while loops
//int user_config_complete = 0;
int wifi_setup_complete = 0;
int host_setup_complete = 0; 
int hostip_entered = 0;

//LED pin definition used for error signalling
const short int BUILTIN_LED2 = 16;  //GPIO16 on NodeMCU (ESP-12)

//WiFi status values
const char *str_status[]= {
  "WL_IDLE_STATUS",
  "WL_NO_SSID_AVAIL",
  "WL_SCAN_COMPLETED",
  "WL_CONNECTED",
  "WL_CONNECT_FAILED",
  "WL_CONNECTION_LOST",
  "WL_DISCONNECTED"
};
const char *str_mode[]= { "WIFI_OFF", "WIFI_STA", "WIFI_AP", "WIFI_AP_STA" };

//-----MAIN-------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting up...");
  //delay(500);
  pinMode(BUILTIN_LED2, OUTPUT);
  digitalWrite(BUILTIN_LED2, LOW);
  delay(100); 
  digitalWrite(BUILTIN_LED2, HIGH);
  delay(300); 
  //Serial.print("Chip ID: 0x");
  //Serial.println(ESP.getChipId(), HEX);
  Serial.println("Command Mode Enabled.");

  //Prompt user to connect to Wifi
  Serial.println("Type 1 to connect to Wifi using stored credentials.");
  while(wifi_setup_complete == 0) {
    if(Serial.available()) {
      byte data = Serial.read();
      int command = (int)data;
      //Serial.print("debug: ");
      //Serial.println(command);
      if(command == 49) { //'1' in ASCII
        WiFi.mode(WIFI_STA); //Set ESP8266 as wifi client (station) and not access point
        WiFi.begin(ssid, password);
        Serial.print("Connecting to ");
        Serial.print(ssid);
        Serial.print("...");
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          wifi_setup_complete = 1;
          Serial.print(" WiFi connected with local IP "); 
          Serial.print(WiFi.localIP());
          Serial.println(".");
          //Serial.print("WiFi mode: ");
          //Serial.println(str_mode[WiFi.getMode()]);
          //Serial.print("Status: " );
          //Serial.println(str_status[WiFi.status()]);
          delay(1000);    
        }
        else {
          Serial.println("WiFi connect failed. Type 1 to try again.");
        }        
      }
      else {
        Serial.println("Not a valid command.");
      }
    }
  } //end while for wifi setup prompt

  //Now prompt user to connect to Host
  Serial.println("Type 2 to connect to Host.");
  while(host_setup_complete == 0) {
    if(Serial.available()) {
      byte data = Serial.read();
      int command = (int)data;
      if(command == 50) { //'2' in ASCII
        Serial.println("Enter Host IP followed by *. ");
        while(hostip_entered == 0) {
          if(Serial.available()) {
            //read and parse input
            int len = Serial.available();
            char msg[len];
            Serial.readBytes(msg, len);
            delay(500);
            String hostip_raw = String(msg); //convert byte array to string
            //Serial.println(hostip_raw);
            int asteriskpos = hostip_raw.lastIndexOf('*');
            hostip = hostip_raw.substring(0, asteriskpos);

            //connect to server
            delay(1000);
            Serial.print("Attempting to connect to Host ");
            Serial.print(hostip);
            Serial.print(" on port ");
            Serial.print(portnum);
            Serial.println("...");
            if (client.connect(hostip, portnum)) {
              Serial.println("Connection to Host successful.");
              hostip_entered = 1;
              host_setup_complete = 1; //set flags to true to exit while loop and setup function
              Serial.println("Type message to send to server."); //then start main loop
            }
            else {
              Serial.println("Connection to Host failed. Enter Host IP again.");
            }
          }//end prompt for host IP
        } 
      }
      else {
        Serial.println("Not a valid selection.");
      }          
    }
  }
} // end setup

void loop() {
  if (client.connected()) {
    // get data from server
    while(client.available()) {  
      Serial.write(client.read());
    }
    //send serial data to server  
    if(Serial.available()){ 
      size_t len = Serial.available();
      uint8_t sbuf[len];
      Serial.readBytes(sbuf, len);
      if (client && client.connected()){
        client.write(sbuf, len);
        delay(10);
      }
    }      
  }
  else {
    Serial.println("ERROR: No longer connected to server.");
    while(1) {} // stop and hang
  }
}

//support function for error signalling
void signalError() {  // loop endless with LED blinking in case of error
  while(1) {
      digitalWrite(BUILTIN_LED2, LOW);
      delay(300); // ms
      digitalWrite(BUILTIN_LED2, HIGH);
      delay(300); // ms
  }
}
