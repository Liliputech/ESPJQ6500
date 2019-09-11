/*
 * FastLED DOCS
 * demoReel100 getstarted https://www.reddit.com/r/FastLED/comments/b2ceka/new_to_fastled_and_trying_to_understand_examples/
 * Overview https://github.com/FastLED/FastLED/wiki/Overview 
 * 
 * FastLED's builtin functions https://github.com/FastLED/FastLED/blob/master/lib8tion.h
 * 
 * 1) fadeToBlack()
 * Each time fadeToBlack is called it is dimming whatever current values a pixel has. 
 * Again, because these functions are being called continuosly from each time through the programs main (void) loop, 
 * pixels will keep getting darker and darker until they go to black or are assigned new values.
 * fadeToBlackBy( leds, NUM_LEDS, 20); 
 * This operates on the "leds" array, fading all pixels in this case 
 * (since NUM_LEDS is specified, vs something less then NUM_LEDS), 
 * and it fades by an amount of 20/256th, or about 7.8%.
 * 
 * 
 * 2) beatsin16() 
 * It takes the arguments beats/minute, a low value, and high value (and optionally also a time base and phase offset). 
 * So in this case:
 * int pos = beatsin16( 13, 0, NUM_LEDS-1 );
 * is setup to do 13 BPM, cycling from 0 to NUM_LEDS-1 (ie from the first pixel to the last pixel in the strip). 
 * That resulting pos number becomes the pixel position, leds[pos], when assigning color.

 * 3) random() : nombreuse fonction random décrites dans lib8tion.h (lien plus haut)
 * random16( n)    == random from 0..(N-1)
 * random8()       == random from 0..255
 * 
*/

 
 
//------Bibliothèques et Settings------

//AUDIO LIBS
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <JQ6500_Serial.h>
#include "WifiConfig.h"

//MQTT LIBS
#include <PubSubClient.h>//https://pubsubclient.knolleary.net/api.html
#include <ESP8266WiFi.h>//https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html

//For OTA Update
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//LEDSTRIPS LIBS
//4 lignes nécessaires pour éviter un flash intempestif récurrent...
// -- The core to run FastLED.show()
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

//LEDSTRIPS SETTINGS
#define FASTLED_USING_NAMESPACE
#define FASTLED_SHOW_CORE 0
#define NUM_LEDS 30 // 5 mètres de WS2813B en 60 leds/mètre
#define DATA_PIN 0 // ledstrip connecté au GPIO 0 de l'ESP01
#define BRIGHTNESS  30
#define FRAMES_PER_SECOND  120
#define COLOR_ORDER GRB //GRB et non RGB car le rouge et le vert sont inversés pour les WS2813B
#define STRIPMODEL WS2812B


//sizeof : https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/
//determine la taille de l'array automatiquement, pour éviter de casser le code si on ajoute des fonctions.
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//Helper macro to cycle through the leds
#define CYCLE_LED(A) A = (A+1) % NUM_LEDS  // forward
#define REVERSE_CYCLE_LED(A) A = (A-1) % NUM_LEDS  //backward

//------------VARIABLES--------------//

CRGB leds[NUM_LEDS];// Define the array of leds
static uint8_t hue = 0;//hue variable

JQ6500_Serial mp3; //les deux GPIO sont utilisés par la lib. SoftwareSerial comme ports série virtuels
/*
unsigned int audiofile; // numéro du fichier audio à lire
unsigned int numFiles; // Total number of files on media (autodetected in setup())
byte mediaType;        // Media type (autodetected in setup())
//*/

//SSID et IP du Broker MQTT à indiquer dans fichier WifiConfig.h
char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
IPAddress mon_broker(BROKER_IP);

WiFiClient ESP01client;
PubSubClient client(ESP01client);

//Variables pour recevoir le nombre de boucles et les patterns publiés par l'extérieur (Ubuntu ou Paho par ex.)
byte ledAudioPattern = 0; // pattern # : 255 patterns maximum
byte holdPattern = 10; // nombre de secondes à jouer le ledAudioPattern courant
byte modePattern = 0; //mode d'enchainement des patterns : 0 = enchainer

String clientID = WiFi.hostname();//nom DHCP de l'ESP, quelque chose comme : ESP_XXXXX. Je ne choisi pas ce nom il est dans l'ESP de base
String topicName = clientID + "/#";//Topic individuel nominatif pour publier un message unique à un ESP unique

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

bool global_enabled = false; //ne pas lancer de patterns dans le ledstrip tant que pas de messages reçu en MQTT

//-------------FONCTION WIFI : connexion, IP -------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//-----------FONCTION MQTT/Mosquitto :connexion, souscription
void connect_mqtt()
{
  // Loop until we're reconnected
  Serial.println("tentative de connexion au broker Mosquitto...");
  while (!client.connected()) {
    if(client.connect(clientID.c_str()))
      break;
    Serial.print('.');
    delay(10);
  }

  client.subscribe("ledstate");
  client.subscribe("holdstate");
  client.subscribe(topicName.c_str());

  client.publish("welcome", clientID.c_str()); // publie son nom sur le topic welcome lors de la connexion
  Serial.printf("topic welcome : %s\n", clientID.c_str());
}

//---------DemoReel100: addGlitter--------
void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

//-------FADE OUT--------
void p0() {
  static int i = 0;
  fadeToBlackBy(leds, NUM_LEDS, 0);//https://github.com/FastLED/FastLED/wiki/RGBSet-Reference
  delay(5);
  CYCLE_LED(i);
}

//--------FONCTION DE BOUCLAGE DES PATTERNS-------
//////////////////////////////////// PATTERNS p1() à p10() ///////////////////////////////////////
//-------allumage successif maintenu en avant--------
void p1()
{
  static int i = 0;
  // Set the i'th led to red
  leds[i] = CHSV(hue++, 255, 255);
  delay(30);
  CYCLE_LED(i);
}

//---------allumage successif maintenu en arrière--------
void p2()
{
  static int i = NUM_LEDS;
  // Set the i'th led to red
  leds[i] = CHSV(hue++, 255, 255);
  delay(30);
  REVERSE_CYCLE_LED(i);
}

//---------Path-drik.ino : allumage successif non maintenu en avant--------
//https://github.com/FastLED/FastLED/wiki/RGBSet-Reference
void p3()
{
  // First slide the led in one direction
  static int i = 0;
  if ( i > 0) leds[i - 1] = CRGB::Black;
  leds[i] = CRGB::Red;
  delay(5);
  CYCLE_LED(i);
}

//---------Path-drik.ino : allumage non maintenu en arrière--------
void p4()
{
  static int i = NUM_LEDS;
  if ( i < NUM_LEDS) leds[i + 1] = CRGB::Black;
  leds[i] = CRGB::Blue;
  delay(5);
  REVERSE_CYCLE_LED(i);
}

//--------- DemoReel100: juggle--------
void p5() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
    delay(20);
  }
}

//---------DemoReel100: bpm--------
void p6() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  static int i=0;
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  delay(20);
  CYCLE_LED(i);
}

//---------DemoReel100: sinelon--------
void p7()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
  delay(20);
}

//---------DemoReel100: confetti--------
void p8()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  delay(20);
}

//---------DemoReel100: rainbow--------
void p9()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  delay(20);
}

//---------DemoReel100: rainbowWithGlitter--------
void p10()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  p9();
  addGlitter(80);
  delay(20);
}


//TEST ROUGE
void p11() {
   // First slide the led in one direction
  static int i = 0;
  if ( i > 0) leds[i - 1] = CRGB::Black;
  leds[i] = CRGB::Red;
  delay(5);
  CYCLE_LED(i);
}
//TEST BLEU
void p12() {
 // First slide the led in one direction
  static int i = 0;
  if ( i > 0) leds[i - 1] = CRGB::Black;
  leds[i] = CRGB::Red;
  delay(5);
  CYCLE_LED(i);
}

//TEST VERT
void p13() {
   // First slide the led in one direction
  static int i = 0;
  if ( i > 0) leds[i - 1] = CRGB::Black;
  leds[i] = CRGB::Red;
  delay(5);
  CYCLE_LED(i);
}

/////////////////////////////////////////////////////////

//function "macro" pour traiter le message MQTT arrivant au MC et retourner la partie utile : un entier
int payloadToInt(byte* payload, int length){
  String payloadFromMQTT = "";
  for (int i = 0; i < length; i++) { //itérer dans toute la longueur message, length est fourni dans le callback MQTT
    if (isDigit(payload[i]))// tester si le payload est bien un chiffre décimal
      payloadFromMQTT += (char)payload[i];//caster en type char et ajouter/placer dans la variable
  }
  return payloadFromMQTT.toInt();//convertir le String en entier et retourner la valeur
}

//-------------FONCTION CALLBACK------------
//l'étoile * means a pointer to the given type. Explained in chapter 1 of any C book
//https://stackoverflow.com/questions/55637077/looking-for-explanation-about-simple-function-declaration
void callback(char* topic, byte* payload, unsigned int length)
{
  String debugMessage = "";
  global_enabled = true; // open flag : PLAY !
  
  //Affichage dans le moniteur (topic welcome) du -t topic et du - m message
  debugMessage = clientID + " - Topic entrant : [" + topic + "] ";

  //On identifie le topic que l'on veut traiter grâce à strcmp() pour "String Compare" : strcmp retourne un 0 si les deux string sont équivalentes
  //référence de strcmp() : http://www.cplusplus.com/reference/cstring/strcmp/


  //----------- durée en secondes à jouer un pattern ----------------
  //exemple de publication pour un pattern d'une durée de 10 secondes : mosquitto_pub -t holdstate -m 10
  if ( (strcmp(topic, (clientID + "/holdstate").c_str()) == 0) //pour ce client uniquement
       || (strcmp(topic, "holdstate") == 0)) { //pour tous les clients
    holdPattern = payloadToInt(payload,length);
    debugMessage += holdPattern;
  }

  //------appel d'un ledAudioPattern de lumière+sons pour tous les clients ou ce client -----
  //exemple de publication pour appeler le pattern 1 càd la function p1() pour ce client spécifiquement : 
  if ( (strcmp(topic, (clientID + "/ledstate").c_str()) == 0) //mosquitto_pub -t ESP_304B27/ledstate -m 1
       || (strcmp(topic, "ledstate") == 0)) { // pour tous les clients: mosquitto_pub -t ledstate -m 1
    ledAudioPattern = payloadToInt(payload,length);
    debugMessage += ledAudioPattern;
  }

  //-----Si le topic est modestate :
  //0 = enchainement de tout le tableau gPatterns en restant sur chaque pattern holdPattern secondes 
  //1 = jouer uniquement le pattern ledAudioPattern pendant holdPattern secondes 
  if (strcmp(topic, "modestate") == 0) {
    modePattern = payloadToInt(payload,length);
    debugMessage += modePattern;
  }

//---------- Pour visualiser le traffic arrivant aux ESP ----------------
    // durées : mosquitto_sub -t welcome/holdPattern ou mosquitto_sub -t welcome/holdPattern
    // patterns: mosquitto_sub -t welcome/ledAudioPattern
    // mode d'enchainement : // mosquitto_sub -t welcome/modePattern
  client.publish("welcome", debugMessage.c_str()); 
}

//////////////////////////////////////////////////////////////


//----- Correspondance MQTT Pattern -> Leds Fonctions---------
//------------------------REF----------------------------------
//array of function pointers
//https://www.geeksforgeeks.org/how-to-declare-a-pointer-to-a-function/
//https://www.geeksforgeeks.org/function-pointer-in-c/

//------ array de pointeurs vers des fonctions ------
//https://forum.arduino.cc/index.php?topic=610508.0
//l'idée est de faire pointer la variable ledAudioPattern, vers la fonction respective
//exemple:  ledstate = 1 -> appel de p1()

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 };

void setPattern() {
  if ((ledAudioPattern >= 0) && (ledAudioPattern < ARRAY_SIZE(gPatterns)))
  {
    gCurrentPatternNumber = ledAudioPattern;
  }
}




//----------------------------------------SETUP--------------------------------------

void setup() {
  Serial.begin(115200);
  delay(10);

  //////////// ESP01 settings //////////
  //********************* CHANGE ESP01 PIN FUNCTION **************************************
  pinMode(3, FUNCTION_3); //(RX) pin (nommé également pin3 dans la doc) devient GPIO 3
  //**************************************************************************************

  //////////// ATTENTION ////////////////////////////////////////////////
  //surtout pas configurer le GPIO3 en OUTPUT !!
  //pinMode(3, OUTPUT);
  //SoftwareSerial.h en fait un port série virtuel, pas une sortie data classique !!
  ///////////////////////////////////////////////////////////////////////

 ///////////// JQ6500 settings ///////////
  mp3.begin(9600,3,2);
  mp3.reset();
  mp3.setVolume(40);
  mp3.setLoopMode(MP3_LOOP_NONE);
  mp3.setEqualizer(MP3_EQ_NORMAL);
  mp3.setSource(MP3_SRC_BUILTIN);
  ///////////////////////////////////////

  setup_wifi();

  //IP et port du Broker https://pubsubclient.knolleary.net/api.html#setserver
  client.setServer(mon_broker, 1883);

  //fonction qui permet de traiter -t -m
  //https://pubsubclient.knolleary.net/api.html#callback
  client.setCallback(callback);

  FastLED.addLeds<STRIPMODEL, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);// set master brightness control.
  FastLED.setBrightness(BRIGHTNESS);// (255) = puissance maximale -----> ATTENTION POWER !

  /*A REVOIR
  indique dans le moniteur le nombre de fonctions à disposition
  Serial.print("le nombre de fonctions diponibles est :");
  Serial.println(nFunc); // ?
  Serial.println();
 */
 
  //OTA Update setup
  MDNS.begin(clientID);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}





//---------------------------------------- --------------------------------------

void nextPattern()
{ // Si modestate = 0 ALORS enchainer les patterns SINON ne jouer que le ledAudioPattern appelé
  //exemple: mosquitto_pub -t modestate -m 0
  if (modePattern = 0) {
    // enchainer = add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  }
}


void loop() {
  //OTA Update Check
  httpServer.handleClient();

  //if not connected to Wifi tries to connect
  if (WiFi.status() != WL_CONNECTED)
    setup_wifi();

  //if not connected to MQTT tries to connect
  if (!client.connected())
    connect_mqtt();

  //MQTT Refresh message queue
  client.loop();

  if (!global_enabled) //Global "display enable" flag
    return;

  //----------------- JQ6500 ------------------//
  if (mp3.getStatus() != MP3_STATUS_PLAYING)
  {
    mp3.playFileByIndexNumber(ledAudioPattern);
    mp3.play();
  }

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  EVERY_N_SECONDS( holdPattern ) {
    nextPattern();  // change patterns periodically
  }
}
