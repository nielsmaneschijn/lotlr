/* Basic Neopixel voorbeeld met OTA (over the air) upgrade
Verbind de Wemos D1 mini module als volgt met je ledring:
5V -> +5 (voedingsspanning) (rood draadje)
G -> - (ground) (zwart draadje)
RX -> DI (data in) (geel draadje)
 */
#include <NeoPixelBus.h>
// #include <WiFiUdp.h>
#include <WiFiManager.h>  
#include <ArduinoOTA.h>
// deze krijg je al via WiFiManager denk ik
// #include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>
const uint16_t PixelCount = 12; // aantal leds
const uint8_t PixelPin = 2;  // op de Esp8266 altijd de RX pin

#define colorSaturation 255 // leds niet maximaal helder ivm stroomverbruik en fel aan de oogjes

// geef dit een unieke naam als je je device wilt kunnen herkennen! Dit wordt ook de hostname ("ledring.local") dus geen spaties enzo
const char* SSID = "ledring";

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

String color;
HtmlColor htmlColor;
HslColor hslColor;

int i = 0; // teller voor opschuiven patroon
float j = 1;
int mode = 0; // in welke modus zitten we?
float brightness = 1.0;

// maak een webserver op poort 80
ESP8266WebServer server(80);

void setup_wifi() {
  WiFiManager wifiManager;
  wifiManager.autoConnect(SSID);
}

void setupOTA(){
    //ArduinoOTA
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(SSID);

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

void handleRoot() {
  for (uint8_t i=0; i<server.args(); i++){
    if (server.argName(i) == "color") {color = server.arg(i);};
    if (server.argName(i) == "mode") {mode = server.arg(i).toInt();};
    if (server.argName(i) == "brightness") {brightness = server.arg(i).toFloat();};
  }
  // Serial.println(color);


  String page =  "<html><body><h1>Kies een kleur!</h1><form action=\"\"><input style=\"width: 300px; height: 300px; margin: 10px;\" name=\"color\" type=\"color\" onchange=\"javascript:this.form.submit()\" value=";
  page += color;
  page += "><h2>MODE SELEKTOR</h2><input type=\"range\" name=\"mode\" min=0 max=2 onchange=\"javascript:this.form.submit()\" value=";
  page += mode;
  page += "><h2>Brightness</h2><input type=\"range\" name=\"brightness\" min=0 max=1 step=0.01 onchange=\"javascript:this.form.submit()\" value=";
  page += brightness;
  page += "></form></body></html>";
  server.send(200, "text/html", page);
}  

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setupWebserver(){
//   if (MDNS.begin("esp8266")) {
//     Serial.println("MDNS responder started");
//   }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
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

  setupWebserver();
  ring.SetPixelColor(3, purple); // status info: webserver setup
  ring.Show();
}

void mode0(){
  htmlColor.Parse<HtmlColorNames>(color);
  ring.ClearTo(htmlColor);
  ring.Show();
}

void mode1(){
   for (int x=0; x<PixelCount; x++) { 
      // geef wat pixels een kleur
      if ((x + i) % 3 == 0) { ring.SetPixelColor(x, HslColor::LinearBlend<NeoHueBlendShortestDistance>(HslColor(black), HslColor(red), brightness)); } 
      if ((x + i) % 3 == 1) { ring.SetPixelColor(x, HslColor::LinearBlend<NeoHueBlendShortestDistance>(HslColor(black), HslColor(green), brightness)); } 
      if ((x + i) % 3 == 2) { ring.SetPixelColor(x, HslColor::LinearBlend<NeoHueBlendShortestDistance>(HslColor(black), HslColor(blue), brightness)); } 
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
    ring.SetPixelColor(x, HslColor(fmodf((1.0F/PixelCount*x + j/10000.0F),1.0F), 1.0F, brightness));
  }
  ring.Show();
  j=sin(1.0F*i/1000.0F)*10000.0F;
  i++;
}

// arduino conventie: de code in loop() wordt oneindig vaak uitgevoerd na het doorlopen van setup()
void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  if (mode == 0) {
    mode0();
  }
  if (mode == 1) {
    mode1();
  }
  if (mode == 2) {
    mode2();
  }
}

