//MAQUETTE CORRIDOR AVEC ARTHUR - SEPTEMBRE 2019

/*Serial Monitor Arduino en 74880 baud
 * REGLE 1
   TOUS les ESP souscrivent automatiquement aux topics = ledstate et holdstate   
   On peut donc tous les déclencher simultanément

   exemple:
   mosquitto_pub -t holdstate -m 3 //on veut 3 boucles pour tous les ESP
   mosquitto_pub -t ledstate -m 2 //on déclenche la fonction p2() pour tous les ESP

----

   Chaque ESP souscrit aussi au topic de son nom DHCP propre et unique grâce à la fonction WIFI.hostname, qui ressemble à ESP_XXXXX
   Pour trouver ce nom je peux regarder les baux actifs dans ma freebox ou dans la console de mon Broker qui indique:
   "New CLient connected from [adresse_IP_de_l'ESP] as ESP_XXXXX" 
   
   Et quand on lui envoie un message sur le topic ESP_XXXXX/ledstate ou holdstate
   il sait que le payload ne s'adresse qu'à lui seul

   exemple:
   mosquitto_pub -t ESP_XXXX/ledstate -m 239 (on déclenche le pattern n°239 sur cet ESP spécifiquement

-----

   mosquitto_pub -t welcome -m 0 //on obtient le nom de l'ESP_XXXX sur le topic welcome
   mosquitto_pub -t welcome -m hello world

-----

  TOPIC HOLDSTATE
  On peut choisir le nombre de boucles désirées, càd le nombre de fois qu'un pattern doit-être rejoué
  Par défaut le nombre de boucles est initialisé à 1 boucle (la variable hold = 1)

  Pour boucler en continue (loop infinie) il faut mettre la variable hold = 0

  comme ceci:
   mosquitto_pub -t holdstate -m 0 //on boucle perpétuellement le pattern qui va être publier juste derrière.

  exemple:
  mosquitto_pub -t holdstate -m 0
  mosquitto_pub -t ledstate -m 3

-----
 
  PATTERNS AUTO-BOUCLéS / INFINIS

    en distinguant dès la fonction payload les patterns qui doivent-être "de base" bouclés perpétuellement
    il n'y a plus besoin de mettre hold = 0 manuellement avant de les jouer.

    Les autres patterns, ceux qui doivent être joué un  nombre "hold" de fois, seront effectivement joué le nombre de fois stocké dans
    la variable hold et si hold = 0 alors ils seront eux-même bouclés perpétuellement.

    Par exemple, par défaut le pattern 3 tourne en loop infinie, pour le stopper il faut modifier holdstate pour dire que l'on veut juste une boucle (ou x boucle)
  
  exemple:
  mosquitto_pub -t holdstate -m 1
  
*/

//subscription à un topic avec des wildcards voir :
//https://github.com/knolleary/pubsubclient/issues/610


///////////////// AUDIO /////////////////
/* 3 GPIOs sur l'ESP01 : controle d'un JQ6500 et d'un ledstrip

    Assignation du pin 3 de l'ESP01 en GPIO afin de bénéficier de 3 GPIOs :
    2 pour le JQ6500 et 1 pour une bande de leds

*/

/*** CONNEXIONS ***
    ESP01 GPIO 3 (noté RX) -> JQ6500 TX
    ESP01 GPIO 2 -> JQ6500 RX
*/



//------Bibliothèques et Settings------

//AUDIO LIBS
#include <Arduino.h>
#include <SoftwareSerial.h>
//#include <JQ6500_Serial.h>


//MQTT LIBS
#include <PubSubClient.h>//https://pubsubclient.knolleary.net/api.html
#include <ESP8266WiFi.h>//https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html


//LEDSTRIPS LIBS
//4 lignes nécessaires pour éviter un flash intempestif récurrent...
// -- The core to run FastLED.show()
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

//LEDSTRIPS SETTINGS
#define FASTLED_USING_NAMESPACE
#define FASTLED_SHOW_CORE 0
#define NUM_LEDS 300 // 5 mètres de WS2813B en 60 leds/mètre
#define LEDSTRIP 0 // ledstrip connecté au GPIO 0 de l'ESP01




//------------VARIABLES--------------//

CRGB leds[NUM_LEDS];// Define the array of leds
static uint8_t hue = 0;//hue variable
/*
JQ6500_Serial mp3(3, 2); //les deux GPIO sont utilisés par la lib. SoftwareSerial comme ports série virtuels
unsigned int audiofile; // numéro du fichier audio à lire
unsigned int numFiles; // Total number of files on media (autodetected in setup())
byte mediaType;        // Media type (autodetected in setup())                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
*/

char ssid[] = "";
char password[] = "";
IPAddress mon_broker(192, 168, 0, 16);

WiFiClient ESP01client;
PubSubClient client(ESP01client);


uint8_t gHue = 0; // rotating "base color" used by many of the patterns

//Variables pour recevoir le nombre de boucles et les patterns publiés par l'extérieur (Ubuntu ou Paho par ex.)
byte ledPattern = 0; // pattern # : 255 patterns maximum
byte hold = 1; // number of loops : 255 loop maximum , 0 = perpetual loop
bool blocLoop = false; //avoid infinite loop()
//bool blocLoop2 = false;

String clientID = WiFi.hostname();//nom DHCP de l'ESP, quelque chose comme : ESP_XXXXX. Je ne choisi pas ce nom il est dans l'ESP de base
String topicName = clientID + "/#";//Topic individuel nominatif pour publier un message unique à un ESP unique


/*
  std::String defName = WiFi.hostname().c_str();
  char * S = new char[defName.length()+1];
  std::strcpy(S, defName.c_str());
  //https://stackoverflow.com/questions/12862739/convert-string-to-char
  //http://www.cplusplus.com/forum/general/100714/
*/

/*
  String clientID = WiFi.hostname().c_str();//on récupère le nom de l'ESP en tant que String
  char * defName = (char*)clientID.c_str(); //que l'on converti en char pour compatibilité avec la fonction callback
*/

/*
//Concaténer 1? https://www.baldengineer.com/multiple-mqtt-topics-pubsubclient.html
//String stringTwo;
//stringTwo.concat((char)payload[i]);

//Concaténer 2 ?: https://www.arduino.cc/en/Tutorial/StringAdditionOperator
//char * defNameSharp = (char*)clientID.c_str() + '/' + '#';// on ajout /# à ESP_XXXXX donc ça fait ESP_XXXX/#

//https://github.com/knolleary/pubsubclient/issues/105
//////////////////////////////////////////////////////////////////////////////////////////////

//OTA: https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html

*/


//-------------FONCTION WIFI : connexion, IP -------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/generic-class.html#mode
  //WiFi.mode(m): set mode to WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
  //WiFi.mode(WIFI_STA);// Inutile puisqu'on a WiFi.begin

  ///https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html#start-here
  //inutile de spécifier le mode avec  WiFi.mode(WIFI_STA);
  //begin : Switching the module to Station mode is done with begin function
  //et par défaut l'ESP va tenter de se reconnecter au réseau Wifi indiqué s'il en est déconnecté

  //https://github.com/esp8266/Arduino/issues/2826  -> set esp name
  //https://github.com/esp8266/Arduino/issues/5695 -> issue DHCP hostname

  //prints ESP_XXXX
  //Serial.printf("la variable defname contient: %s\n", defName);
  //Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());

  //WiFi.hostname("WebPool");//nommer l'esp WebPool dans la freebox > DHCP
  WiFi.begin(ssid, password);
  //WiFi.setHostname("WebPool");//selon la lib avant ou après begin

  //https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html#hostname


  //defName = WiFi.hostname().c_str();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    //You can pass flash-memory based strings to Serial.print() by wrapping them with F().
    //file:///Applications/Arduino.app/Contents/Java/reference/www.arduino.cc/en/Serial/Print.html
    Serial.print(F("."));
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//nom de l'ESP
void getName() {
  //https://pubsubclient.knolleary.net/api.html#publish1
  client.publish("welcome", clientID.c_str());//defName.c_str()) ; //WiFi.hostname().c_str());//clientId.c_str());//publie le nom DHCP de l'esp01S sur le topic welcome
  Serial.printf("topic welcome : %s\n", clientID.c_str());
  //Serial.printf("topic welcome : %s\n", defNameSharp);//nothing in monitor

}


//-----------FONCTION MQTT/Mosquitto :connexion, souscription
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("tentative de connexion au broker Mosquitto...");
    Serial.println();

    ////////////////////////////////////////
    // Nom généré aléatoirement. todo: comment récupérer le nom de l'ESP
    //clientId += String(random(0xffff), HEX);
    ////////////////////////////////////////

    //(clientID, username, password) https://pubsubclient.knolleary.net/api.html#connect3
    if (client.connect(clientID.c_str(), "usr", "mqttpass"));//(defName.c_str(), "usr", "mqttpass"))
    {
      Serial.println("connexion au Broker Mosquitto OK !!");

      //publie Hello world ... et ESP8266Client-XXXX sur le topic welcome
      //client.publish("welcome", "hello world, ici ESP01");
      //client.publish("welcome", defName.c_str());//clientId.c_str());

      //Souscription topics ledstate et holdstate
      //boolean subscribe (topic, [qos])  https://pubsubclient.knolleary.net/api.html#subscribe
      client.subscribe("ledstate");
      client.subscribe("holdstate");
      client.subscribe(topicName.c_str());

      //client.subscribe(WiFi.hostname().c_str());//souscrit au topic qui porte son nom DHCP
      //client.subscribe(defName + '/' +'#');
      //client.subscribe(defNameSharp.c_str());
      delay(10);
    }
  }
}




//---------DemoReel100: addGlitter--------
void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}


//-------FADE OUT--------
//anciennement fadeall()
void p0() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
    FastLED.show();
    delay(10);
  }
  blocLoop = false;//close flag : la boucle ne continue pas plus que "hold" fois
  Serial.println("fadeall done!");
}


//--------FONCTION DE BOUCLAGE DES PATTERNS-------
//////////////////////////////////// PATTERNS p1() à p10() ///////////////////////////////////////
//-------allumage successif maintenu en avant--------
void p1()
{
  for (int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    //fadeall();
    // Wait a little bit before we loop around and do it again
    delay(30);
  }
}

//---------allumage successif maintenu en arrière--------
void p2()
{
  for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    //fadeall();
    // Wait a little bit before we loop around and do it again
    delay(30);
  }
}

//---------Path-drik.ino : allumage successif non maintenu en avant--------
void p3()
{
  // First slide the led in one direction
  static int i = 0;
    // Set the i'th led to White
    leds[i] = CRGB::Red;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    // Wait a little bit before we loop around and do it again
    delay(5);
  i = (i+1)%NUM_LEDS;
}

//---------Path-drik.ino : allumage non maintenu en arrière--------
void p4()
{
  static int i = 0 ;
    // Set the i'th led to White
    leds[i] = CRGB::Blue;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    // Wait a little bit before we loop around and do it again
    delay(5);
  i=(i+1)%NUM_LEDS;
}

//--------- DemoReel100: juggle--------
void p5() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    delay(20);
  }
}

//---------DemoReel100: bpm--------
void p6() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    delay(20);
  }
}
//---------DemoReel100: sinelon--------
void p7()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//---------DemoReel100: confetti--------
void p8()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//---------DemoReel100: rainbow--------
void p9()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//---------DemoReel100: rainbowWithGlitter--------
void p10()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  p9();
  addGlitter(80);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

  //TEST ROUGE
void p11(){
  //audiofile = 1;
 for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(5);
  }
}
  //TEST BLEU
void p12(){
  //audiofile = 2;
 for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(5);
  }
}

  //TEST VERT
  void p13(){
 //audiofile = 3;
 for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(5);
  }
}
/////////////////////////////////////////////////////////




  
//-------------FONCTION CALLBACK------------
//l'étoile * means a pointer to the given type. Explained in chapter 1 of any C book
//https://stackoverflow.com/questions/55637077/looking-for-explanation-about-simple-function-declaration
void callback(char* topic, byte* payload, unsigned int length)
{

  //Affichage dans le moniteur du -t topic et du - m message
  Serial.print("Topic entrant : [");
  Serial.print(topic);
  Serial.print("] ");

  //-----Si le topic est holdstate-----
  //On identifie le topic que l'on veut traiter grâce à strcmp() pour "String Compare" : strcmp retourne un 0 si les deux string sont équivalentes
  //https://www.baldengineer.com/multiple-mqtt-topics-pubsubclient.html
  //référence de strcmp() : http://www.cplusplus.com/reference/cstring/strcmp/
  if (strcmp(topic, "holdstate") == 0) {//si le contenu de topic est holdstate alors retourne 0
    
    //----On récupère le message-----
    /*
      MQTT transmets tout en ASCII donc pour obtenir un digit :
     
      1) on récupère chaque caractère : on itère dans cellules du tableau payload
      afin recueillir les caractères transmis et les placer dans un byte.
      Donc si MQTT : [2][5][5] vers ESP01 : 255 (255 maximum car byte)
      et
      2) on transpose ASCII en nombre, car en ASCII 48 = 0

      explications: si depuis le Broker j'envoie hold = 5 et ledPattern 3 (par ex.) alors j'obtiens dans le moniteur Arduino 53 fois le ledPattern 3
      étant donné que ascii '5' vaut 53 donc je comprends que depuis le Broker j'envoies en ASCII et pas en nombre
    */
    for (int i = 0; i < length; i++) {
      hold = (byte)payload[i] - 48; //
      Serial.print(hold);
    }
    Serial.println();
  }



  //------Si le topic est ledstate (simile)-----
  else if  (strcmp(topic, "ledstate") == 0) {
    for (int i = 0; i < length; i++) {
      ledPattern = (byte)payload[0] - 48;
      Serial.print(ledPattern);
      
      if (ledPattern == 3 || ledPattern == 4 || ledPattern == 11 || ledPattern == 12 || ledPattern == 13) {//si les fonctions 3 ou 4 sont appelées
        blocLoop = true;//orienter le processeur vers le nb de boucles à effectuer
      }
      //      else {
      //        hold = 0;//BOUCLAGE PERPETUEL
      //        client.publish("holdstate", "0"); //(-t , -m) càd publier l'état de la variable hold sur le topic "holdstate", pour une màj de l'interface graphique Paho-JS
    }
    Serial.println();
  }



 
  //Si le topic est clientID/ledstate - 
  //exemple : mosquitto_pub -t ESP_2ABD4E/ledstate -m 3
  else  if (strcmp(topic, (clientID + "/ledstate").c_str()) == 0) {
    for (int i = 0; i < length; i++) {
      ledPattern = (byte)payload[i] - 48;
      Serial.print(ledPattern);
      if (ledPattern == 3 || ledPattern == 4 || ledPattern == 11 || ledPattern == 12 || ledPattern == 13) {
        blocLoop = true;//orienter le processeur vers le nb de boucles à effectuer
      }
      //      else {
      //        hold = 0;//BOUCLAGE PERPETUEL
      //        client.publish("holdstate", "0"); //(-t , -m) càd publier l'état de la variable hold sur le topic "holdstate", pour une màj de l'interface graphique Paho-JS
    }
    Serial.println(); 
  }


  
  //Si le topic est clientID/holdstate - simile
else if (strcmp(topic, (clientID + "/holdstate").c_str()) == 0) {
   for (int i = 0; i < length; i++) {
      hold = (byte)payload[i] - 48; //
      Serial.print(hold);
    }
    Serial.println();
  }

/*
 * https://github.com/knolleary/pubsubclient/issues/334
 * The publish functions expect char[] types to be passed in rather than Strings.
 * You need to use the String.toCharArray() function to convert your strings to the necessary type.
 * Si le topic est "welcome"
*/
/*
else if (strcmp(topic, "welcome") == 0) {
  
  //toCharArray() -> https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/tochararray/
  int[] welcomeTOchar = welcome.toCharArray();
  
  client.publish(welcomeTOchar[], clientID); 
      Serial.print(clientID);
    }
    Serial.println();
  }
*/

}




//----- Correspondance MQTT Pattern -> Leds Fonctions---------
//array of function pointers
//https://www.geeksforgeeks.org/how-to-declare-a-pointer-to-a-function/
//https://www.geeksforgeeks.org/function-pointer-in-c/

//------ array de pointeurs vers des fonctions ------
//https://forum.arduino.cc/index.php?topic=610508.0
//l'idée est de faire pointer la variable ledPattern qui est un byte entre 0 et 9, vers les fonctions respectives p0() à p9()
//l'utilisation d'un typedef éclairci l'écriture

typedef void (*voidfunc)();// avec typedef on créé un type inexistant
voidfunc func[] = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13};//le tableau func[] est de type voidfunc, càd pointeur vers fonction

//sizeof : https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/
//determine la taille de l'array automatiquement, pour éviter de casser le code si on ajoute des fonctions.
int nFunc = sizeof(func) / sizeof(func[0]); // sizeof returns the total number of bytes.

void setPattern() {
  if ((ledPattern >= 1) && (ledPattern <= nFunc))
  {
    func[ledPattern](); //-1 car sinon, en envoyant un message MQTT 1, il me joue l'index func[1] soit la fonction p2() et non la fonction p1()
  }
}

/*IDEM
  void (*func_ptr[])() = {p0, p1, p2, p3, p4, p5, p6, p7, p8, p9};

  void setPattern(){

  if((ledPattern>=0)&&(ledPattern<=9))
  {
   (*func_ptr[ledPattern])();
  }
  }
*/
//////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(74880);
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


//////////////////// JQ6500 settings ////////////////////
/*
//----------------Initialisation module audio--------------
  mp3.begin(9600);
  mp3.reset();
  mp3.setVolume(40);
  mp3.setLoopMode(MP3_LOOP_NONE);
  mp3.setEqualizer(MP3_EQ_NORMAL);

// we select the built in source NOT SD card source
  mp3.setSource(MP3_SRC_BUILTIN);
  numFiles = mp3.countFiles(MP3_SRC_BUILTIN);
  mediaType = MP3_SRC_BUILTIN;
//////////////////////////////////////
*/

  setup_wifi();

  //IP et port du Broker https://pubsubclient.knolleary.net/api.html#setserver
  client.setServer(mon_broker, 1883);

  //fonction qui permet de traiter -t -m
  //https://pubsubclient.knolleary.net/api.html#callback
  client.setCallback(callback);

  LEDS.addLeds<WS2812B, LEDSTRIP, GRB>(leds, NUM_LEDS);//GRB et non RGB car le rouge et le vert sont inversés pour les WS2813B
  LEDS.setBrightness(84);// (255) = puissance maximale -----> ATTENTION POWER !

  //indique dans le moniteur le nombre de fonctions à disposition
  Serial.print("le nombre de fonctions diponibles est :");
  Serial.println(nFunc);
  Serial.println();

  getName();
}



void loop() {
/*
//----------------- JQ6500 ------------------//
  if (mp3.getStatus() != MP3_STATUS_PLAYING)
  {
    mp3.playFileByIndexNumber(audiofile);
    mp3.play();
  }
*/
  
  //do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();


  //BOUCLAGE INFINI
  if (hold == 0) { //si la variable hold est 0 on boucle perpétuellement setPattern()
    setPattern();//on appelle effectivement la fonction correspondante au message MQTT

    Serial.print(">");
    Serial.print(ledPattern);//on imprime le n° de ledPattern à chaque fois qu'on le rejoue
    Serial.println("<");
  }


  //BOUCLAGE "HOLD" FOIS
  else  { //patterns qui doivent se reproduire en nombre de bouclages définis par la variable hold
    if (blocLoop == true) { // open flag : cette condition permet d'éviter que le pattern ci-dessous ne continue indéfiniement dans loop()
      for (int i = 0; i < hold ; i++); { //on va répéter la fonction setPattern() "hold" fois
        setPattern();//on appelle effectivement la fonction correspondante au message MQTT

        Serial.print(">");
        Serial.print(ledPattern);//on imprime le n° de ledPattern à chaque fois qu'on le rejoue
        Serial.println("<");
      }
      p0();//on fait un fondu général pour ne rien laisser allumer ("fadeall, done!")

      //cette ligne afin d'éviter que le dernier pattern en mémoire ne démarre automatiquement si on fait une publication du type:
      //mosquitto_pub -t holdstate -m 0 // càd une demande de bouclage infinie juste avant de lancer un nouveau pattern
      ledPattern = 0;

      blocLoop = false;//close flag : la boucle ne continue pas plus que "hold" fois
      Serial.println("fadeall done!");
    }
  }
}
