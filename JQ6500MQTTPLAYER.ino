/* appeler le son n°17
      mosquitto_pub -t audiostate -m 17

  régler le volume sur 60
      mosquitto_pub -t volstate -m 60

  mettre le fichier son en boucle    (0 = pas de boucle, 1 = en boucle)
      mosquitto_pub -t loopstate -m 1

*/

//------Bibliothèques et Settings------

//AUDIO LIBS
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <JQ6500_Serial.h>
//#include "WifiConfig.h"

//MQTT LIBS
#include <PubSubClient.h>//https://pubsubclient.knolleary.net/api.html
#include <ESP8266WiFi.h>//https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/station-class.html


//------------VARIABLES--------------//
JQ6500_Serial mp3; //les deux GPIO sont utilisés par la lib. SoftwareSerial comme ports série virtuels
/*
  unsigned int audiofile; // numéro du fichier audio à lire
  unsigned int numFiles; // Total number of files on media (autodetected in setup())
  byte mediaType;        // Media type (autodetected in setup())
  //*/

//SSID et IP du Broker MQTT à indiquer dans fichier WifiConfig.h
char ssid[] = "Vishnu";//WIFI_SSID;
char password[] = "BrahmaParticules";//WIFI_PASSWORD;
IPAddress mon_broker(192, 168, 0, 12); //(BROKER_IP);

WiFiClient ESP01client;
PubSubClient client(ESP01client);

//Variables pour recevoir le nombre de boucles et les patterns publiés par l'extérieur (Ubuntu ou Paho par ex.)
//byte holdPattern = 10; // nombre de secondes à jouer le ledAudioPattern courant
byte audioPattern;//soundfile number
byte volumePattern = 20;
byte loopPattern = 1;// play soundfile loopPattern times, nombre de boucle, 0 = boucle infinie
bool blocLoop = false; //avoid infinite loop()

String clientID = WiFi.hostname();//nom DHCP de l'ESP, quelque chose comme : ESP_XXXXX. Je ne choisi pas ce nom il est dans l'ESP de base
String topicName = clientID + "/#";//Topic individuel nominatif pour publier un message unique à un ESP unique

bool global_enabled = false; //ne pas lancer de patterns tant que pas de messages reçu en MQTT

//-------------FONCTION WIFI : connexion, IP -------------
void setup_wifi() {
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
    if (client.connect(clientID.c_str()))
      break;
    Serial.print('.');
    delay(10);
  }

  //client.subscribe("holdstate");
  client.subscribe("audiostate");
  client.subscribe("volstate");
  client.subscribe("loopstate");
  client.subscribe(topicName.c_str());

  client.publish("welcome", clientID.c_str()); // publie son nom sur le topic welcome lors de la connexion
  Serial.printf("topic welcome : %s\n", clientID.c_str());
}


//----- Correspondance MQTT Pattern -> Leds Fonctions---------
//------------------------REF----------------------------------
//array of function pointers
//https://www.geeksforgeeks.org/how-to-declare-a-pointer-to-a-function/
//https://www.geeksforgeeks.org/function-pointer-in-c/

//------ array de pointeurs vers des fonctions ------
//https://forum.arduino.cc/index.php?topic=610508.0
//l'idée est de faire pointer la variable ledAudioPattern, vers la fonction respective
//exemple:  ledstate = 1 -> appel de p1()


//function "macro" pour traiter le message MQTT arrivant au MC et retourner la partie utile : un entier
int payloadToInt(byte* payload, int length) {
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

  global_enabled = true; // open flag : permettre à tous les messages MQTT qui arrivent de mettre en marche loop() et donc d'être traités

  //Affichage dans le moniteur (topic welcome) du -t topic et du - m message
  debugMessage = clientID + " - Topic entrant : [" + topic + "] ";

  //On identifie le topic que l'on veut traiter grâce à strcmp() pour "String Compare" : strcmp retourne un 0 si les deux string sont équivalentes
  //référence de strcmp() : http://www.cplusplus.com/reference/cstring/strcmp/


  //------appel d'un audioPattern de sons pour tous les clients ou ce client -----
  //exemple de publication pour appeler le son N°1  pour ce client spécifiquement :
  if ( (strcmp(topic, (clientID + "/audiostate").c_str()) == 0) //mosquitto_pub -t ESP_304B27/audiostate -m 1
       || (strcmp(topic, "audiostate") == 0)) { // pour tous les clients: mosquitto_pub -t ledstate -m 1
    audioPattern = payloadToInt(payload, length);
    debugMessage += audioPattern;
    blocLoop = true; // soudfile PLAY open flag : permet au fichier son spécifiquement d'être traité dans loop()
  }

  //MQTT VOLUME à un niveau de 30 : mosquitto_pub -t volstate -m 30
  if ( (strcmp(topic, (clientID + "/volstate").c_str()) == 0) //mosquitto_pub -t ESP_304B27/audiostate -m 1
       || (strcmp(topic, "volstate") == 0)) { // pour tous les clients: mosquitto_pub -t ledstate -m 1
    volumePattern = payloadToInt(payload, length);
    debugMessage += volumePattern;
  }


  if ( (strcmp(topic, (clientID + "/loopstate").c_str()) == 0)
       || (strcmp(topic, "loopstate") == 0)) {
    loopPattern = payloadToInt(payload, length);
    debugMessage += loopPattern;
  }

  //---------- Pour visualiser le traffic arrivant aux ESP ----------------
  // durées : mosquitto_sub -t welcome/holdPattern ou mosquitto_sub -t welcome/holdPattern
  // patterns: mosquitto_sub -t welcome/ledAudioPattern
  // mode d'enchainement : // mosquitto_sub -t welcome/modePattern
  client.publish("welcome", debugMessage.c_str());
}


//////////////////////////////////////////////////////////////


//----------------------------------------SETUP--------------------------------------

void setup() {
  Serial.begin(74880);

  //////////// ESP01 settings //////////
  //********************* CHANGE ESP01 PIN FUNCTION **************************************
  pinMode(3, FUNCTION_0); //(RX) pin (nommé également pin3 dans la doc) est réinitialisé à son état normal au cas où ...
  //**************************************************************************************

  ///////////// JQ6500 settings ///////////
  /*** CONNEXIONS ***
      ESP01 Pin 0 -> JQ6500 TX
      ESP01 Pin 2 -> JQ6500 RX
  */
  mp3.begin(9600, 0, 2);
  mp3.reset();
  // mp3.setVolume(volumePattern);
  // mp3.setLoopMode(MP3_LOOP_NONE);
  mp3.setEqualizer(MP3_EQ_NORMAL);
  mp3.setSource(MP3_SRC_BUILTIN);
  ///////////////////////////////////////

  setup_wifi();

  //IP et port du Broker https://pubsubclient.knolleary.net/api.html#setserver
  client.setServer(mon_broker, 1883);

  //fonction qui permet de traiter -t -m
  //https://pubsubclient.knolleary.net/api.html#callback
  client.setCallback(callback);
  //---------------------------------------- --------------------------------------
}

/*
  void loopTrack() {
  if (loopPattern = 0)  {
    mp3.setLoopMode(MP3_LOOP_ONE_STOP);//(MP3_LOOP_NONE);//Play one track then stop
  }
  else  {
    mp3.setLoopMode(MP3_LOOP_ONE);
  }
  }
*/

void loop() {
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

  mp3.setVolume(volumePattern);
  // loopTrack();
  //mp3.setLoopMode(MP3_LOOP_ONE_STOP);//Default, plays track and stops // (MP3_LOOP_NONE)?
  //----------------- JQ6500 ------------------//

  ////////////////////////////////////////////////////////////////////////////
  /* NE MARCHE PAS : si 0 = loop infinie, sinon loopPattern fois
    // blocloop = open flag : cette condition permet d'éviter que le fichier son ci-dessous ne continue indéfiniement dans loop()
    if (blocLoop == true) {
      if (loopPattern == 0) { //si la variable loopPattern est 0 on boucle perpétuellement le fichier
        if (mp3.getStatus() != MP3_STATUS_PLAYING)
        {
          mp3.playFileByIndexNumber(audioPattern);
          mp3.play();
        }
      }

      //BOUCLAGE "loopPattern" FOIS
      else  { //patterns qui doivent se reproduire en nombre de bouclages définis par la variable loopPattern
        for (int i = 0; i < loopPattern ; i++); {
          if (mp3.getStatus() != MP3_STATUS_PLAYING)
          {
            mp3.playFileByIndexNumber(audioPattern);
            mp3.play();
          }
        }
        blocLoop = false;//close flag : la boucle ne continue pas plus que "loopPattern" fois
  */
  ////////////////////////////////////////////////////////////////////////////
  if (blocLoop == true) {

    for (int i = 0; i < loopPattern ; i++); {
      if (mp3.getStatus() != MP3_STATUS_PLAYING)
      {
        mp3.playFileByIndexNumber(audioPattern);
        mp3.play();
      }
    }
    blocLoop = false;//close flag : la boucle ne continue pas plus que "loopPattern" fois
  }
}
