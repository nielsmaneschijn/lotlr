/* Basic Neopixel voorbeeld met OTA (over the air) upgrade
Verbind de Wemos D1 mini module als volgt met je ledring:
5V -> +5 (voedingsspanning) (rood draadje)
G -> - (ground) (zwart draadje)
RX -> DI (data in) (geel draadje)
 */
#include <NeoPixelBus.h>
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
#include <WiFiManager.h>  
#include <ArduinoOTA.h>
const uint16_t PixelCount = 12; // aantal leds
const uint8_t PixelPin = 2;  // op de Esp8266 altijd de RX pin

#define colorSaturation 64 // leds niet maximaal helder ivm stroomverbruik en fel aan de oogjes

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

int i = 0; // teller voor opschuiven patroon
float j = 1;

void setup_wifi() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("Ledring");
}

void setupOTA(){
    //ArduinoOTA
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("ledring");

    // No authentication by default
    // ArduinoOTA.setPassword((const char *)"123");

    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      ring.ClearTo(purple,0,(progress / (total / PixelCount)));
      ring.Show();
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

}


// arduino conventie: setup() wordt bij boot eenmalig uitgevoerd
void setup() {
  
  // Initialize the BUILTIN_LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);     

  // seriele poort openzetten voor debug data (zorg dat je terminal dezelfde baudrate gebruikt)
  Serial.begin(115200);
  Serial.println("Hello world!"); //moi
  
  //ledring init
  ring.Begin();
  ring.SetPixelColor(0, red); // status info: leds online!
  ring.Show();

  setup_wifi();
  ring.SetPixelColor(1, green); // status info: wifi online!
  ring.Show();

  setupOTA();
  ring.SetPixelColor(2, blue); // status info: OTA setup
  ring.Show();
}

void mode1(){
   for (int x=0; x<PixelCount; x++) { 
      // geef wat pixels een kleur
      if ((x + i) % 3 == 0) { ring.SetPixelColor(x, red); } 
      if ((x + i) % 3 == 1) { ring.SetPixelColor(x, green); } 
      if ((x + i) % 3 == 2) { ring.SetPixelColor(x, blue); } 
    }
    
    // ophogen teller voor animatie
    i++;
    Serial.println(i);
    
    // hier wordt de buffer weggeschreven naar de ledstrip/ring
    ring.Show();

    // hiermee kan je een oneindige lus maken voor de hooligans >:)
    // while (true) {/*yield();*/}
    
    delay(500); //halve seconde wachten
}

void mode2(){
  // pink fluffy unicorn mode!
  for (int x=0; x<PixelCount; x++) {
    ring.SetPixelColor(x, HslColor(fmodf((1.0F/PixelCount*x + j/10000.0F),1.0F), 1.0F, 0.5F));
  }
  ring.Show();
  j=sin(1.0F*i/1000.0F)*10000.0F;
  i++;
}

// arduino conventie: de code in loop() wordt oneindig vaak uitgevoerd na het doorlopen van setup()
void loop() {
  ArduinoOTA.handle();
  mode2();
}

