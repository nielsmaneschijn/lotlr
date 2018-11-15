
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

// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor pink(colorSaturation, 0, colorSaturation);
RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, colorSaturation/2, 0);
RgbColor purple(colorSaturation/8, 0, colorSaturation);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

const int poweron = 9; //turn leds on at 9 o'clock
const int poweroff = 18; //off at six

void setup_wifi() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("Pretty fly for a wifi");
  
//   Serial.println(WiFi.localIP());
}



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();

        NTP.begin ("pool.ntp.org", 1, true, 0);
        // NTP.setInterval (63);

  //leds
      // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

}

boolean powersave() {
  return hour() < poweron || hour() >= poweroff; 
}

void paint(RgbColor color) {
  if (powersave()) { color = black;}

    for (int x=0; x<PixelCount; x++) {
      strip.SetPixelColor(x, color); 
    }
    strip.Show();
}

void error(RgbColor color) {
  // 3 manieren om een fout af te handelen:
  // 1: doe niks -> de laatste kleur blijft staan
  
  // 2: in geval van twijfel -> niks tonen. paint it black!
  //  paint(black);

  // 3: informatieve maar irritante kleurtjes
  // paint(color);
}

time_t decode(String hrs, String min) {
  return 3600 * (hrs.toInt()) + 60 * (min.toInt());
  }

void raincheck() {
  //Neerslagintensiteit = 10^((waarde-109)/32)
 bool somerain = false;

if(WiFi.status()== WL_CONNECTED){ //Check WiFi connection status
  
    time_t ntpTime = NTP.getTime();
    if (ntpTime > 0) {
      time_t nu = elapsedSecsToday(ntpTime) - 300;
      time_t straks = nu + 2100;

      HTTPClient http; //Declare an object of class HTTPClient
      http.begin("http://gpsgadget.buienradar.nl/data/raintext/?lat=53.19&lon=6.56"); //Specify request destination
      int httpCode = http.GET(); //Send the request
      if (httpCode == 200 ) { //Check the returning code 
        String payload = http.getString(); //Get the request response payload
        Serial.println(payload); //Print the response payload
        boolean lineFound = false;

        // read 24 lines
        for (int x=0; x<24; x++) {
          time_t linetime = decode(payload.substring((x*11)+4, (x*11)+6), payload.substring((x*11)+7, (x*11)+9));
          if (linetime > nu && linetime < straks) {
            lineFound = true;
            Serial.println(payload.substring((x*11)+4, (x*11)+9));
            somerain = somerain || payload.substring(x*11, (x*11)+3) != "000";
          }
        }
        if (lineFound) {
          Serial.println(somerain ? "rain" : "no rain");
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


void loop() {

Serial.println (NTP.getTimeDateString()); 

raincheck();
delay(30000); //Send a request every 30 seconds

}

