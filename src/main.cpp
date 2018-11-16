
/*
TODO 
- knipperen bij statusverandering
- http server voor debug info
- klok modus
- animaties
- etc etc
*/

// http client om buienradar api mee aan te roepen
#include <ESP8266HTTPClient.h>
// wifimanager maakt een access point om je wifi credentials mee in te kunnen stellen
#include <WiFiManager.h>  
// NTP client om de tijd op te halen
#include <NtpClientLib.h>
// ws2812b ledstrip/ring
#include <NeoPixelBus.h>

const uint16_t PixelCount = 12; // aantal leds
const uint8_t PixelPin = 2;  // op de Esp8266 altijd de RX pin

#define colorSaturation 64 // leds niet maximaal helder ivm stroomverbruik en fel aan de oogjes

// geef dit een unieke naam als je je device wilt kunnen herkennen!
#define SSID "Pretty fly for a wifi"
// stel hier de coordinaten van je crib in!
#define LAT 53.19
#define LON 6.56

// er zijn verschillende constructors voor verschillende varianten leds, met name de volgorde van de kleuren
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> ring(PixelCount, PixelPin); // GRB!
// NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> ring(PixelCount, PixelPin); // RGB!

// wat prefab kleurtjes
RgbColor red(colorSaturation, 0, 0);
RgbColor pink(colorSaturation, 0, colorSaturation);
RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, colorSaturation/2, 0);
RgbColor purple(colorSaturation/8, 0, colorSaturation);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

// wanneer staat 'ie aan
const int poweron = 7; //aan om 7u
const int poweroff = 23; //uit om 23u

void setup_wifi() {
  // maak een access point om je wifi netwerk in te kunnen stellen
  WiFiManager wifiManager;
  wifiManager.autoConnect(SSID); 
  
//   Serial.println(WiFi.localIP());
}


// arduino conventie: setup() wordt bij boot eenmalig uitgevoerd
void setup() {
  // Initialize the BUILTIN_LED pin as an output
  pinMode(BUILTIN_LED, OUTPUT);     
  // seriele poort openzetten voor debug data (zorg dat je terminal dezelfde baudrate gebruikt)
  Serial.begin(115200);

  // wifi access point opzetten indien nodig, anders verbinden met het eerder ingestelde netwerk
  setup_wifi();

  // tijd ophalen van een NTP server
  NTP.begin ("pool.ntp.org", 1, true, 0);

  //ledring init
  ring.Begin();
  ring.Show();

}

// is het al laat?
boolean powersave() {
  return hour() < poweron || hour() >= poweroff; 
}

// loop over alle leds in de ring en kleur ze met de gewenste kleur
void paint(RgbColor color) {
  // 's nachts zetten we de leds op zwart
  if (powersave()) { color = black;}

    for (int x=0; x<PixelCount; x++) {
      // this is where the magic happens
      ring.SetPixelColor(x, color); 
    }
    // nou ja eigenlijk hier: hier wordt de buffer weggeschreven naar de ledstrip/ring
    ring.Show();
}

void error(RgbColor color) {
  // 3 manieren om een fout af te handelen:
  // 1: doe niks -> de laatste kleur blijft staan
  
  // 2: in geval van twijfel -> niks tonen. paint it black!
  //  paint(black);

  // 3: informatieve maar irritante kleurtjes
  // paint(color);
}

// de timestamps van buienradar terugrekenen naar seconden sinds middernacht
time_t decode(String hrs, String min) {
  return 3600 * (hrs.toInt()) + 60 * (min.toInt());
  }

// ophalen van buienradar api data en kijken of het gaat regenen
  //Neerslagintensiteit = 10^((waarde-109)/32), maar voor nu kijken we gewoon of het wel of niet 000 is
void raincheck() {
  bool somerain = false;

  if(WiFi.status()== WL_CONNECTED){ //Check WiFi connection status
  
    time_t ntpTime = NTP.getTime();
    if (ntpTime > 0) { //soms gaat het tijd syncen mis
      time_t nu = elapsedSecsToday(ntpTime) - 300;
      time_t straks = nu + 2100;

      HTTPClient http; //Declare an object of class HTTPClient
      http.begin("http://gpsgadget.buienradar.nl/data/raintext/?lat=LAT&lon=LON"); //Specify request destination
      int httpCode = http.GET(); //Send the request
      if (httpCode == 200 ) { //Check the returning code 
        String payload = http.getString(); //Get the request response payload
        Serial.println(payload); //Print the response payload
        boolean lineFound = false; // bijhouden of er wel data in het bericht zat (soms niet, ondanks de HTTP 200)

        // read 24 lines
        for (int x=0; x<24; x++) {
          // pluk de tijd uit de dataregel
          time_t linetime = decode(payload.substring((x*11)+4, (x*11)+6), payload.substring((x*11)+7, (x*11)+9));
          // kijk of de timestamp tussen nu en 30 minuten valt
          if (linetime > nu && linetime < straks) {
            lineFound = true;
            Serial.println(payload.substring((x*11)+4, (x*11)+9));
            // als de eerste 3 karakters niet 000 zijn gaat het regenen!
            somerain = somerain || payload.substring(x*11, (x*11)+3) != "000";
          }
        }
        // hebben we relevante data gekregen?
        if (lineFound) {
          Serial.println(somerain ? "rain" : "no rain");
          // als het gaat regenen: zet alle pixels op blauw en zet de kleine led op de Wemos module aan. anders groen en uit.
          if (somerain) {
            paint(blue);
            digitalWrite(BUILTIN_LED, 0);
          } else {
            paint(green);
            digitalWrite(BUILTIN_LED, 1);
          } 
        } else {
            Serial.println("no data :(");
            error(orange);
        }
        
      } else {
        Serial.println("geen http 200");   
        error(yellow);
      }

      http.end(); //Close connection
    } else {
      Serial.println("NTP bork");   
      error(pink);
    }
  } else {
    Serial.println("Error in WiFi connection");   
    error(white);
  }
}

// arduino conventie: de code in loop() wordt oneindig vaak uitgevoerd na het doorlopen van setup()
void loop() {

  Serial.println (NTP.getTimeDateString()); 

  raincheck();
  delay(30000); //wacht 30 seconden en goto 10!

}

