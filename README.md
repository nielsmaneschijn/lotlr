# IoT for Dummies workshop!
Maak je eigen buienradarlamp!

# Instructies
Hardware:
- Verbind de Wemos D1 mini module als volgt met je ledring:
5V -> +5 (voedingsspanning) (rood draadje)
G -> - (ground) (zwart draadje)
RX -> DI (data in) (geel draadje)

Installeren ontwikkeltools:
- installeer git als je dat nog niet hebt: https://git-scm.com/downloads . PlatformIO gebruikt git om dependencies op te halen.
- installeer PlatformIO volgens de instructies hier: https://platformio.org/platformio-ide (klik op Install for VSCode). Atom kan ook maar werd ik minder blij van.
- installeer de NeoPixelBus, WifiManager en NtpClientLib libraries onder PIO Home -> Libraries -> Registry. Bekijk ook even de examples aldaar, de meeste libraries hebben goede voorbeelden.
- (installeer espressif8266 onder Platforms indien deze er nog niet staat)
- (installeer d1_mini onder Boards indien deze er nog niet staat)
- clone deze repo ergens: git clone https://github.com/nielsmaneschijn/lotlr.git (alternatief als je geen git geïnstalleerd hebt: download de zip van https://github.com/nielsmaneschijn/lotlr/archive/master.zip)

Hello World: simpele test van je Neopixels
- open de lotlr folder (File -> Open Folder)
- build je project met het vinkje-icoon onderaan (of ctrl-alt-b)
- indien succesvol gebuild voor d1_mini: sluit je Wemos aan via USB
- upload met het pijl-icoontje onderaan (of ctrl-alt-u)
- als het goed is branden nu de leds op je ring!
- druk op het stekker icoontje onderaan, zo open je een debug terminal en kan je meekijken met de debug info (Serial.println()).
- open src-ledring/main.cpp, kijk hoe e.e.a. werkt en pas iets aan. 
- upload je nieuwe code met ctrl-alt-u
- voor de avonturiers: laat je programma crashen door een oneindige lus te maken binnen de loop(). Dit zal de watchdog timer af laten gaan die automatisch de chip reset.
- gebruik een yield() of delay(0) binnen je oneindige lus om de watchdog timer te resetten. Onthoud dit voor als je later in een reset-loop terechtkomt.

Buienradarlamp: het internet op
- open platformio.ini
- wijzig de src_dir in src-buienradarlamp
- un-comment de WifiManager en NtpClientLib libraries (goede kans dat je de versie moet veranderen in 0.14 of gewoon weg kunt halen)
- open src-buienradarlamp/main.cpp
- zoek de GPS coordinaten van je crib op met Google Maps en pas de constr String LON en LAT regels hiermee aan
- pas de const char* SSID aan zodat je weet welk access point van jou is
- code builden en uploaden: ctrl-alt-u
- als de ESP geen netwerk kan vinden start hij op in de wifi access point mode (dankzij Wifimanager). Connect met dit access point (als het goed is redirect je automatisch naar 192.168.4.1) en vul de credentials van het wifi netwerk in. 
- hierna reboot de ESP en als alles goed gegaan is is je buienradarlamp online!

Werkt alles? Dan kan je zelf verder aan de slag!
Suggesties:
- de NeoPixelBus library heeft uitgebreide mogelijkheden voor animaties, bekijk de examples hiervoor
- het is niet handig om steeds firmware updates via USB te moeten doen, dit kan ook via het netwerk (kan je de lampjes in je kerstboom laten hangen, en het gaat nog sneller ook). Bekijk hiervoor de EasyOTA library.
- voeg een http server toe zodat je zelf een kleur kunt kiezen (pak aan, Philips Hue!)
- je kunt WifiManager ook gebruiken om extra parameters in te stellen, zoals je GPS coordinaten.