/*------------------------------------------------------------------------------
  24/04/2022
  Author: Crypto Generation 
  Platforms: ESP8266
  Language: C++/Arduino
  File: crypto_generation_display.ino
  ------------------------------------------------------------------------------
  Description: 
  Code for YouTube video demonstrating how to use the coingecko API to fetch the 
  price of Multipe cryptocurrency and display it on a 0.96" OLED over I2C.
  ------------------------------------------------------------------------------
  License:
  Please see attached LICENSE.txt file for details.
------------------------------------------------------------------------------*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//---------------------------------------------------------------
//On board LED Connected to GPIO2
#define LED LED_BUILTIN


int i = 0;
int Status;
String st;
String payload = "{}";


#define apiurl "Please Enter Coingecko Api"
const char *finger  = "Please Enter Fingerprint";

//Function Declaration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

ESP8266WebServer server(80); //Server on port 80

//Our HTML webpage contents in program memory
const char configuration_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html><head><style>
body {
  background-color: #86d8e7;
}
</style>

<body>
<center><h1 style="color:blue;">WiFi Configuration Setup!</h1>
<form action="/configuration">
  <label for="ssid">SSID:</label>
  <input type="text" id="ssid" name="ssid">
  <p style="color:red;">Please Enter Your Wifi SSID</p><br>
  <label for="passString">PASS:</label>
  <input type="text" id="passString" name="passString">
  <p style="color:red;">Please Enter Your Wifi PassWord</p><br>
  <input type="submit" value="SAVE">
</form>

</center>

</body>
</head>
</html>
)=====";






//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================




void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      String s = configuration_page; //Read HTML contents
      server.send(200, "text/html", s);

    });



    server.on("/configuration", []() {
      String essid = server.arg("ssid");
      String epassString = server.arg("passString");


      if (essid.length() > 0 && epassString.length() > 0) {
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }

        for (int i = 0; i < essid.length(); ++i)
        {
          EEPROM.write(i, essid[i]);
        }
        for (int i = 0; i < epassString.length(); ++i)
        {
          EEPROM.write(32 + i, epassString[i]);
        }



        EEPROM.commit();
        ESP.reset();
      } else {

      }

    });
  }
}

//==============================================================
//                  SETUP
//==============================================================
void setup(void) {
  EEPROM.begin(512);
  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  delay(100);
  pinMode(LED, OUTPUT);

  String r_ssid;
  for (int i = 0; i < 32; ++i)
  {
    r_ssid += char(EEPROM.read(i));
  }

  String r_passString;
  for (int i = 32; i < 96; ++i)
  {
    r_passString += char(EEPROM.read(i));
  }


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x32
    for (;;);
  }
  delay(100);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(WHITE);
  display.setCursor(45, 8);
  display.println("Crypto");
  display.setCursor(35, 20);
  display.println("Generation");
  display.drawRect(4, 4, 120, 28, WHITE);
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25, 2);
  display.println("Created By");
  display.setCursor(0, 18);
  display.println("cryptob31@gmail.com");
  display.fillRect(0, 12, 120, 1, WHITE);
  display.display();
  delay(2000);






  WiFi.begin(r_ssid.c_str(), r_passString.c_str());
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 16);
  display.println("Waiting For WiFi");
  display.display();

  if (testWifi())
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 2);
    display.println("Connected");
    display.setCursor(20, 20);
    display.print(WiFi.localIP());
    display.display();
    delay(1000);
    return;
  }
  else
  {
      display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(40, 2);
  display.println("Failed!");
  display.setCursor(20, 18);
  display.println("Not Connected");
  display.display();
  delay(1000);
  
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 2);
    display.println("HotSpot On");
    display.setCursor(28,18);
    display.print(WiFi.softAPIP());
    display.display();


    launchWeb();
    setupAP();// Setup accesspoint or HotSpot
  }


  while ((WiFi.status() != WL_CONNECTED))
  {
    delay(100);
    server.handleClient();
  }
}




//==============================================================
//                     LOOP
//==============================================================
void loop() {
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    client.setFingerprint(finger);
    HTTPClient https;
    payload = "{}";
    https.begin(client, apiurl);

    int httpsCode = https.GET();

    if (httpsCode > 0) {
      payload = https.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      if (error)
      {
        delay(5000);
        return;
      }
      JsonObject bitcoin = doc["bitcoin"];
      long bitcoin_usd_24h_change = bitcoin["usd_24h_change"];
      long bitcoin_usd = bitcoin["usd"];

      
      JsonObject ethereum = doc["ethereum"];
      long ethereum_usd_24h_change = ethereum["usd_24h_change"];
      long ethereum_usd = ethereum["usd"];


      
      JsonObject bnb = doc["binancecoin"];
      long bnb_usd_24h_change = bnb["usd_24h_change"];
      long bnb_usd = bnb["usd"];

      
      JsonObject polygon = doc["matic-network"];
      long polygon_usd_24h_change = polygon["usd_24h_change"];
      long polygon_usd = polygon["usd"];


      JsonObject fantom = doc["fantom"];
      long fantom_usd_24h_change = fantom["usd_24h_change"];
      long fantom_usd = fantom["usd"];

      JsonObject vechain = doc["vechain"];
      long vechain_usd = vechain["usd"];
      long vechain_usd_24h_change = vechain["usd_24h_change"];

      JsonObject shibainu = doc["shiba-inu"];
      long shibainu_usd_24h_change = shibainu["usd_24h_change"];
      long shibainu_usd = shibainu["usd"];

      JsonObject the_graph = doc["the-graph"];
      long the_graph_usd_24h_change = the_graph["usd_24h_change"];
      long the_graph_usd = the_graph["usd"];

      JsonObject raca = doc["radio-caca"];
      long raca_usd_24h_change = raca["usd_24h_change"];
      long raca_usd = raca["usd"];

      JsonObject blok = doc["bloktopia"];
      long blok_usd = blok["usd"];
      long blok_usd_24h_change = blok["usd_24h_change"];

      JsonObject flare = doc["flare-token"];
      long flare_usd_24h_change = flare["usd_24h_change"];
      long flare_usd = flare["usd"];

      JsonObject atlas = doc["star-atlas"];
      long atlas_usd_24h_change = atlas["usd_24h_change"];
      long atlas_usd = atlas["usd"];



      JsonObject solana = doc["solana"];
      long solana_usd_24h_change = solana["usd_24h_change"];
      long solana_usd = solana["usd"];

      
      JsonObject polkadot = doc["polkadot"];
      long polkadot_usd_24h_change = polkadot["usd_24h_change"];
      long polkadot_usd = polkadot["usd"];


      
      JsonObject bitcoin_cash = doc["bitcoin-cash"];
      long bitcoin_cash_usd_24h_change = bitcoin_cash["usd_24h_change"];
      long bitcoin_cash_usd = bitcoin_cash["usd"];



      
      JsonObject dogecoin = doc["dogecoin"];
      long dogecoin_usd_24h_change = dogecoin["usd_24h_change"];
      long dogecoin_usd = dogecoin["usd"];
      

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(bitcoin_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("BTC");
    display.setCursor(0, 18);
    display.println(bitcoin_usd);
    display.display();
    delay(2000);

    

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(ethereum_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("ETH");
    display.setCursor(0, 18);
    display.println(ethereum_usd);
    display.display();
    delay(2000);


    

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(bnb_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("BNB");
    display.setCursor(0, 18);
    display.println(bnb_usd);
    display.display();
    delay(2000);

    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(polkadot_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("DOT");
    display.setCursor(0, 18);
    display.println(polkadot_usd);
    display.display();
    delay(2000);



    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(bitcoin_cash_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("BCH");
    display.setCursor(0, 18);
    display.println(bitcoin_cash_usd);
    display.display();
    delay(2000);

    

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(fantom_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("FTM");
    display.setCursor(0, 18);
    display.println(fantom_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(polygon_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("POLY");
    display.setCursor(0, 18);
    display.println(polygon_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(vechain_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("VET");
    display.setCursor(0, 18);
    display.println(vechain_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(the_graph_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("GRT");
    display.setCursor(0, 18);
    display.println(the_graph_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(solana_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("SOL");
    display.setCursor(0, 18);
    display.println(solana_usd);
    display.display();
    delay(2000);


    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(dogecoin_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("DOGE");
    display.setCursor(0, 18);
    display.println(dogecoin_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(shibainu_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("SHIB");
    display.setCursor(0, 18);
    display.println(shibainu_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(raca_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("RACA");
    display.setCursor(0, 18);
    display.println(raca_usd);
    display.display();
    delay(2000);

    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(blok_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("BLOK");
    display.setCursor(0, 18);
    display.println(blok_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(atlas_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("ATLS");
    display.setCursor(0, 18);
    display.println(atlas_usd);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(60, 4);
    display.println("24h ");
    display.setCursor(85, 4);
    display.print(flare_usd_24h_change);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("FLR");
    display.setCursor(0, 18);
    display.println(flare_usd);
    display.display();
    delay(2000);

    }
    https.end();
  }
  else
  {
  }

}







//Functions used for saving WiFi credentials and to connect to it which you do not need to change 
bool testWifi(void)
{
  int c = 0;
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    c++;
  }
  return false;
}

void launchWeb()
{
  if (WiFi.status() == WL_CONNECTED)
  display.println(WiFi.localIP());
  display.println(WiFi.softAPIP());
  createWebServer();
  server.begin();
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);
  WiFi.softAP("Crypto-Generation", "");
  launchWeb();
}
