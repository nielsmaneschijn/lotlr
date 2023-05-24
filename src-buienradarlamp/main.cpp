
/*
TODO 
- knipperen bij statusverandering
- http server voor debug info/kleur zelf instellen
- klok modus
- meer animaties (zie https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelAnimator-object)
- OTA update met EasyOTA
- etc etc
*/
#include <Arduino.h>
// http client om buienradar api mee aan te roepen
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
// #include <ESP8266WiFi.h>
// wifimanager maakt een access point om je wifi credentials mee in te kunnen stellen
#include <WiFiManager.h>  
// NTP client om de tijd op te halen (hier krijg je Time.h bij)
#include <NtpClientLib.h>
// ws2812b ledstrip/ring
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 64; // aantal leds
const uint8_t PixelPin = 2;  // op de Esp8266 altijd de RX pin

#define colorSaturation 64 // leds niet maximaal helder ivm stroomverbruik en fel aan de oogjes

// geef dit een unieke naam als je je device wilt kunnen herkennen!
const char* SSID = "buienradar";
// stel hier de coordinaten van je crib in! (2 decimalen achter de komma)
//Wolddijk
// const String LAT = "53.25";
// const String LON = "6.57";
// const String LON = "6.60";
//Enshore HQ
const String LAT = "53.19";
const String LON = "6.56";

// er zijn verschillende constructors voor verschillende varianten leds, met name de volgorde van de kleuren (ook de datapin hangt hiervan af)
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> ring(PixelCount, PixelPin); // GRB!
// NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> ring(PixelCount, PixelPin); // RGB!

// wat prefab kleurtjes
RgbColor red(colorSaturation, 0, 0);
RgbColor pink(colorSaturation, 0, colorSaturation/8);
RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, colorSaturation/2, 0);
RgbColor purple(colorSaturation/8, 0, colorSaturation);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

// kleur als het niet regent
RgbColor allclear = green;

// wanneer staat 'ie aan
const int poweron = 8; //aan om 7u
const int poweroff = 18; //uit om 23u

// totaal regenintensiteit in vage log eenheid
int totalrain = 0;
// totaal regenintensiteit mm/u (eigenlijk 6x teveel)
float totalrainmm = 0;

// gaat het keihard regenen?
boolean codered = false;

// NeoPixel animation time management object
NeoPixelAnimator animations(PixelCount, NEO_MILLISECONDS);

void setup_wifi() {
  // maak een access point om je wifi netwerk in te kunnen stellen
  WiFiManager wifiManager;
  wifiManager.autoConnect(SSID); 
  
//   Serial.println(WiFi.localIP());
}


// arduino conventie: setup() wordt bij boot eenmalig uitgevoerd
void setup() {
  
  // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_BUILTIN, OUTPUT);     

  // seriele poort openzetten voor debug data (zorg dat je terminal dezelfde baudrate gebruikt)
  Serial.begin(115200);
  
  //ledring init
  ring.Begin();
  ring.SetPixelColor(0, red); // status info: leds online!
  ring.Show();

  // wifi access point opzetten indien nodig, anders verbinden met het eerder ingestelde netwerk
  setup_wifi();

  ring.SetPixelColor(1, green); // status info: wifi online!
  ring.Show();

  // tijd ophalen van een NTP server
  NTP.begin ("pool.ntp.org", 1, true, 0);
  NTP.setInterval(10,3600);

  ring.SetPixelColor(2, blue); // status info: NTP tijd sync gestart!
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

    for (int x=0; x<PixelCount; x++) { //makkelijker: ring.clearTo(color);
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
  ring.SetPixelColor(0, color);
  delay(3000);
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
  
    // time_t ntpTime = NTP.getTime(); //duurtlang
    time_t ntpTime = now(); //sneller?
    if (ntpTime > 0) { //soms gaat het tijd syncen mis
      time_t nu = elapsedSecsToday(ntpTime) - 300;
      time_t straks = nu + 2100;

      // TODO dit hoeft op zich maar 1x te gebeuren lijkt me
      HTTPClient http; //Declare an object of class HTTPClient
      // http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
      String url = "https://br-gpsgadget.azurewebsites.net/data/raintext?lat=" + LAT + "&lon=" + LON;
      // expliciet client maken en configgen zodat https cert niet gevalideerd wordt en je geen fingerprint nodig hebt
      BearSSL::WiFiClientSecure client;
      client.setInsecure();
      http.begin(client, url);

      // http.begin(url,"39 8E 01 A5 0C 66 8A 74 F0 10 4A 83 60 15 A2 6E 21 55 4C CE"); //Specify request destination and SHA fingerprint, zie https://forum.arduino.cc/index.php?topic=515541.0
      int httpCode = http.GET(); //Send the request //duurtlang
      if (httpCode == 200 ) { //Check the returning code 
        String payload = http.getString(); //Get the request response payload
        Serial.println(payload); //Print the response payload
        boolean lineFound = false; // bijhouden of er wel data in het bericht zat (soms niet, ondanks de HTTP 200)
        // reset nog wat globals (niet heel sjiek, bij een error kunnen we beter de oude state aanhouden)
        codered = false;
        totalrain = 0;
        totalrainmm = 0.00;

        // read 24 lines
        for (int x=0; x<24; x++) {
          // pluk de tijd uit de dataregel
          time_t linetime = decode(payload.substring((x*11)+4, (x*11)+6), payload.substring((x*11)+7, (x*11)+9));
          // kijk of de timestamp tussen nu en 30 minuten valt
          if (linetime > nu && linetime < straks) {
            lineFound = true;
            // als de eerste 3 karakters niet 000 zijn gaat het regenen!
            somerain = somerain || payload.substring(x*11, (x*11)+3) != "000";
            totalrain = totalrain + payload.substring(x*11, (x*11)+3).toInt();
            totalrainmm = totalrainmm + pow(10,((payload.substring(x*11, (x*11)+3).toInt()-109)/32));
            codered = codered || pow(10,((payload.substring(x*11, (x*11)+3).toInt()-109)/32)) > 10; // meer dan 10 mm/u == stortbui
            Serial.println(payload.substring((x*11)+4, (x*11)+9) + " " + pow(10,((payload.substring(x*11, (x*11)+3).toInt()-109)/32)));
          }
        }
        // hebben we relevante data gekregen?
        if (lineFound) {
          Serial.println(somerain ? "rain" : "no rain");
          // als het gaat regenen: zet alle pixels op blauw en zet de kleine led op de Wemos module aan. anders groen en uit.
          if (somerain) {
            // paint(blue); // vervangen door vet hippe animatie
            digitalWrite(LED_BUILTIN, 0);
          } else {
            paint(allclear);
            digitalWrite(LED_BUILTIN, 1);
          } 
        } else {
            Serial.println("no data :(");
            error(orange);
        }
        
      } else {
        Serial.println("geen http 200");   
        Serial.println(httpCode);
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

AnimUpdateCallback animUpdate = [=](const AnimationParam& param) {
    // progress will start at 0.0 and end at 1.0
    float hue = 0.666; // normaal blauw
    if (codered) { hue = 0.0;} // rood bij stortbui
    HslColor updatedColor = HslColor(hue,1.0,0.2-(param.progress/6));
    ring.SetPixelColor(param.index, updatedColor);
};


// arduino conventie: de code in loop() wordt oneindig vaak uitgevoerd na het doorlopen van setup()
void loop() {
  // Serial.println (NTP.getTimeDateString()); 
  Serial.print(now());
  Serial.print(" ");
  Serial.print(hour());
  Serial.print(":");
  Serial.println(minute());
  Serial.println(timeStatus()); // 0=timeNotSet, 1=timeNeedsSync, 2=timeSet
  raincheck();
  Serial.println(totalrain);
  Serial.println(totalrainmm);

  if (totalrain > 0 && !powersave()) {
    // paint(black);
    unsigned long t1 = millis();
    while (millis() - t1 < 30000) {
      //900/500 zwaar
      //998/2000 licht
      //totalrain loopt van 77 tot 1530 (max 255, min 77, keer 6)
      if (random(1000)>(1004-totalrain/10)) {
        animations.StartAnimation(random(PixelCount), 2000-totalrain, animUpdate);
      }
      animations.UpdateAnimations();  
      ring.Show();
      delay(1);
      // now();//kietel klok
    }
    if (powersave()) {
      paint(black); // voor als het nog regent als het power save interval ingaat, of als er dan gereset wordt
    }
  } else {
    delay(90000); //wacht 90 seconden en goto 10! kleur is al gezet in raincheck
    // API data ververst slechts elke 300 sec dus kan best minder vaak
  }
}

