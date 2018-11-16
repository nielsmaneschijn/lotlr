# IoT for Dummies workshop!
Maak je eigen buienradarlamp!

# Instructies
Hardware:
- Bedenk of je de ring rechtstreeks aan de module vast wilt solderen, of dat je pinheaders wilt gebruiken met jumper kabeltjes. Dat laatste is handig als je ook nog verder wilt spelen met schermpjes of sensors.
- Verbind de Wemos D1 mini module als volgt met je ledring:
5V -> +5 (voedingsspanning) (rood draadje)
G -> - (ground) (zwart draadje)
RX -> DI (data in) (geel draadje)

Installeren ontwikkeltools:
- Installeer PlatformIO volgens de instructies hier: https://platformio.org/platformio-ide (klik op Install for VSCode). Atom kan ook maar werd ik minder blij van.
- installeer de NeoPixelBus en NtpClientLib libraries onder PIO Home -> Libraries -> Registry. Bekijk ook even de examples aldaar, de meeste libraries hebben goede voorbeelden.
- installeer espressif8266 onder Platforms
- installeer d1_mini onder Boards
- clone deze repo ergens: git clone https://github.com/nielsmaneschijn/lotlr.git (alternatief als je geen git geïnstalleerd hebt: download de zip van https://github.com/nielsmaneschijn/lotlr/archive/master.zip)
- open de lotlr folder (File -> Open Folder)
- open src/main.cpp
- zoek de GPS coordinaten van je crib op met Google Maps en pas de #define LON en LAT regels hiermee aan
- pas de #define SSID aan zodat je weet welk access point van jou is
- build je project met het vinkje-icoon onderaan (of ctrl-alt-b)
- indien succesvol gebuild voor d1_mini: sluit je Wemos aan via USB
- upload met het pijl-icoontje onderaan (of ctrl-alt-u)
- als de ESP geen netwerk kan vinden start hij op in de wifi access point mode (dankzij Wifimanager). Connect met dit access point (als het goed is redirect je automatisch naar 192.168.4.1) en vul de credentials van het wifi netwerk in. 
- hierna reboot de ESP en als alles goed gegaan is is je buienradarlamp online!
- als je op het stekker icoontje klikt open je een debug terminal en kan je meekijken met de debug info (Serial.println()).

Werkt alles? Dan kan je zelf verder aan de slag!
Suggesties:
- de NeoPixelBus library heeft uitgebreide mogelijkheden voor animaties, bekijk de examples hiervoor
- het is niet handig om steeds firmware updates via USB te moeten doen, dit kan ook via het netwerk. Bekijk hiervoor de EasyOTA library.
- voeg een http server toe zodat je zelf een kleur kunt kiezen (pak aan, Philips Hue!)