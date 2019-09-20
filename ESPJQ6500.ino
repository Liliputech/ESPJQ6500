
////////////////////////////////////////////////////////////////////////
//Refonte du projet, appelle moi si tu veux qu'on en parle, bon weekend.
////////////////////////////////////////////////////////////////////////



//------Bibliothèques et Settings------
#include <PubSubClient.h>//https://pubsubclient.knolleary.net/api.html
#include <ESP8266WiFi.h>//https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html

#include "WifiConfig.h"


//4 lignes nécessaires pour éviter un flash intempestif récurrent...
// -- The core to run FastLED.show()
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
#define FASTLED_USING_NAMESPACE
#define FASTLED_SHOW_CORE 0
//

#define NUM_LEDS 300
#define DATA_PIN 0 
#define COLOR_ORDER GRB //GRB et non RGB car le rouge et le vert sont inversés pour les WS2813B
#define BRIGHTNESS  30


/*
//sizeof : https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/
//determine la taille de l'array automatiquement, pour éviter de casser le code si on ajoute des fonctions.
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//Helper macro to cycle through the leds
#define CYCLE_LED(A) ((A) = ((A)+1) % NUM_LEDS)  // forward
#define REVERSE_CYCLE_LED(A) ((A) = ((A)-1) % NUM_LEDS)  //backward
*/


//-----VARIABLES------
CRGB leds[NUM_LEDS];// Define the array of leds
static uint8_t hue = 0;//hue variable

//SSID et IP du Broker MQTT à indiquer dans fichier WifiConfig.h
char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;
IPAddress mon_broker(BROKER_IP);

WiFiClient ESP01client;
PubSubClient client(ESP01client);

//Variables pour recevoir le nombre de boucles et les patterns publiés par l'extérieur (Ubuntu ou Paho par ex.)
byte ledPattern = 0; // pattern # : 255 patterns maximum
byte holdPattern = 1; // number of loops : 255 loop maximum , 0 = perpetual loop
bool blocLoop = false; //avoid infinite loop()

String clientID = WiFi.hostname();//nom DHCP de l'ESP, quelque chose comme : ESP_XXXXX, il est dans l'ESP de base
String topicName = clientID + "/#";//Topic individuel nominatif pour publier un message unique à un ESP unique

bool global_enabled = false; //FLAG : ne pas lancer de ledPattern tant que pas de messages reçu en MQTT



//-------------FONCTION WIFI : connexion, IP -------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
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
  Serial.println("connexion au Broker Mosquitto OK !!");
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
//anciennement fadeall()
void p0() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
    FastLED.show();
    delay(10);
  }
  blocLoop = false;//close flag : la boucle ne continue pas plus que "holdPattern" fois
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
  for (int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to White
    leds[i] = CRGB::White;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

//---------Path-drik.ino : allumage non maintenu en arrière--------
void p4()
{
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    // Set the i'th led to White
    leds[i] = CRGB::White;
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
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
/*
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
*/
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
 // int ledPattern = 0;

  global_enabled = true; // open flag : PLAY !
  
  //Affichage dans le moniteur (topic welcome) du -t topic et du - m message
  debugMessage = clientID + " - Topic entrant : [" + topic + "] ";

  
 //----------- durée en secondes à jouer un pattern ----------------
  //exemple de publication pour un pattern d'une durée de 10 secondes : mosquitto_pub -t holdPattern -m 10
  if ( (strcmp(topic, (clientID + "/holdPattern").c_str()) == 0) //pour ce client uniquement
       || (strcmp(topic, "holdPattern") == 0)) { //pour tous les clients
    holdPattern = payloadToInt(payload,length);
    debugMessage += holdPattern;
  }



  //------Si le topic est ledstate (simile)-----
      //------appel d'un ledPattern de lumière+sons pour tous les clients ou ce client -----
  //exemple de publication pour appeler le pattern 1 càd la function p1() pour ce client spécifiquement : 
 if ( (strcmp(topic, (clientID + "/ledstate").c_str()) == 0) //mosquitto_pub -t ESP_304B27/ledstate -m 1
       || (strcmp(topic, "ledstate") == 0)) { // pour tous les clients: mosquitto_pub -t ledstate -m 1
    ledPattern = payloadToInt(payload,length);
   // setPattern(ledPattern);
    debugMessage += ledPattern;
    blocLoop = true;//orienter le processeur vers le nb de boucles à effectuer
  }
 
    //---------- Pour visualiser le traffic arrivant aux ESP ----------------
    // durées : mosquitto_sub -t welcome/holdPattern ou mosquitto_sub -t welcome/holdPattern
    // patterns: mosquitto_sub -t welcome/ledPattern
    // mode d'enchainement : // mosquitto_sub -t welcome/modePattern
  client.publish("welcome", debugMessage.c_str()); 
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
voidfunc func[] = {p0, p1, p2, p3, p4, p5};//, p6, p7, p8, p9, p10};//le tableau func[] est de type voidfunc, càd pointeur vers fonction

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
  pinMode(3, FUNCTION_0); //(RX) pin (nommé également pin3 dans la doc) devient GPIO 3
  //**************************************************************************************
  
  setup_wifi();

  //IP et port du Broker https://pubsubclient.knolleary.net/api.html#setserver
  client.setServer(mon_broker, 1883);

  //fonction qui permet de traiter -t -m
  //https://pubsubclient.knolleary.net/api.html#callback
  client.setCallback(callback);


  FastLED.addLeds<WS2812, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);// set master brightness control.
  FastLED.setBrightness(BRIGHTNESS);// (255) = puissance maximale -----> ATTENTION POWER !

/*
   FastLED.addLeds<WS2812, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(84);
  */
  //indique dans le moniteur le nombre de fonctions à disposition
  Serial.print("le nombre de fonctions diponibles est :");
  Serial.println(nFunc);
  Serial.println();

}



void loop() {
  if (WiFi.status() != WL_CONNECTED)
    setup_wifi();

  //if not connected to MQTT tries to connect
  if (!client.connected())
    connect_mqtt();

  //MQTT Refresh message queue
  client.loop();

  if (!global_enabled) //Global "display enable" flag
    return;


  //BOUCLAGE INFINI
  if (holdPattern == 0) { //si la variable holdPattern est 0 on boucle perpétuellement setPattern()
    setPattern();//on appelle effectivement la fonction correspondante au message MQTT

    Serial.print(">");
    Serial.print(ledPattern);//on imprime le n° de ledPattern à chaque fois qu'on le rejoue
    Serial.println("<");
  }


  //BOUCLAGE "holdPattern" FOIS
  else  { //patterns qui doivent se reproduire en nombre de bouclages définis par la variable holdPattern
    if (blocLoop == true) { // open flag : cette condition permet d'éviter que le pattern ci-dessous ne continue indéfiniement dans loop()
      for (int i = 0; i < holdPattern ; i++); { //on va répéter la fonction setPattern() "holdPattern" fois
        setPattern();//on appelle effectivement la fonction correspondante au message MQTT

        Serial.print(">");
        Serial.print(ledPattern);//on imprime le n° de ledPattern à chaque fois qu'on le rejoue
        Serial.println("<");
      }
     // p0();//on fait un fondu général pour ne rien laisser allumer ("fadeall, done!")

      //cette ligne afin d'éviter que le dernier pattern en mémoire ne démarre automatiquement si on fait une publication du type:
      //mosquitto_pub -t holdPattern -m 0 // càd une demande de bouclage infinie juste avant de lancer un nouveau pattern
      //ledPattern = 0;

      blocLoop = false;//close flag : la boucle ne continue pas plus que "holdPattern" fois
      Serial.println("fadeall done!");


      /*TO DO
//https://github.com/marmilicious/FastLED_examples/blob/master/every_n_timers.ino
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
    */
    }
  }
}
