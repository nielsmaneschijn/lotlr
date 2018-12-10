/* Basic Neopixel voorbeeld
Verbind de Wemos D1 mini module als volgt met je ledring:
5V -> +5 (voedingsspanning) (rood draadje)
G -> - (ground) (zwart draadje)
RX -> DI (data in) (geel draadje)
 */
#include <NeoPixelBus.h>
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

// arduino conventie: setup() wordt bij boot eenmalig uitgevoerd
void setup() {
  
  // Initialize the BUILTIN_LED pin as an output
  pinMode(BUILTIN_LED, OUTPUT);     

  // seriele poort openzetten voor debug data (zorg dat je terminal dezelfde baudrate gebruikt)
  Serial.begin(115200);
  Serial.println("Hello world!"); //moi
  
  //ledring init
  ring.Begin();
  ring.Show();
}

// arduino conventie: de code in loop() wordt oneindig vaak uitgevoerd na het doorlopen van setup()
void loop() {
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

