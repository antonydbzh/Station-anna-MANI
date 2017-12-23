////////////////////////////
// STATION METEO Anna MANI//
///////////////////////////
/* Programme ludo-pédagogique d'initiation à la pogrammation et à l'internet des objets.
 
 Anna Mani (1918-2001) était une scientifique météorologue indienne. 
 Elle a réalisé des travaux de recherche sur la couche d'ozone, 
 inventé une sonde de mesure de l'ozone. 
 Elle a travaillé également sur l'énergie solaire thermique 
 et les parcs éoliens en inde. 
 Sa carrière scientifique débute dans le domaine de la physique, 
 sur les propriétés optiques des diamants et des rubis.
 Nous lui dédions cette station météorologique pédagogique, 
 puisse-t-elle inspirer les filles comme les garçons dans le goût pour les sciences et les technologies.
 Plus d'infos sur Anna MANI : https://fr.wikipedia.org/wiki/Anna_Mani
 
 Toute la documentation qui accompagne ce programme est disponible sur wikidebrouillard.org.
 Ce programme a été conçu par Julien Rat - les petits débrouillards/Les usines nouveles
 Modifié par Antony Auffret - les petits débrouillards - CC-By-Sa 2017

                            BROCHAGE
(les numéros interne correspondent aux broches équivalentes sur Arduino - GPIO)                      
                        _______________                   
                      /     D1 mini     \                      
                     |[ ]RST      1-TX[ ]|                   
   Entrée analogique |[ ]A0-      3-RX[ ]|                    
        LED1/Relais1 |[ ]D0-16    5-D1[ ]|  BMP280 - SCL
        LED2/Relais2 |[ ]D5-14    4-D2[ ]|  BMP280 - SDA
 Entrée bouton - BP1 |[ ]D6-12    0-D3[ ]|  DS18B20    
 Entrée bouton - BP2 |[ ]D7-13    2-D4[ ]|  LED_BUILTIN         
               DHT22 |[ ]D8-15     GND[ ]|                    
                     |[ ]3V3        5V[ ]|                    
                     |       +---+       |                     
                     |_______|USB|_______|                     
   ___
 / ___ \
|_|   | |
     /_/ 
     _   ___   _ 
    |_| |___|_| |_
         ___|_   _|
        |___| |_|
Les petits Débrouillards - CC-By-Sa http://creativecommons.org/licenses/by-nc-sa/3.0/
*/

//Bibliothèques Wifi//
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

ESP8266WebServer server(80);//objet serveur

const char* hostAP = "Anna-MANI";

/////////////////////////////

// Bibliothèques capteur d'humidité-température //
#include <Wire.h>
#include "DHT.h"
#define DHTPIN D8         // broche où est connectée le capteur DHT22 - D8
#define DHTTYPE DHT22     // choix du capteur : DHT22
DHT dht(DHTPIN, DHTTYPE); // création du capteur

// Bibliotheque du capteur de température DS18b20 //
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D3   // Capteur sur la broche D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Bibliotheque du capteur de pression BMP280 //
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;           // Initialisation du capteur bmp
/////////////////////////////////////////////////////////////////////////////////////////////////
// ATTENTION Pour le capteur version GY BMP280, la bibliothèque doit être modifiée.            //
// Pour le faire vous-même, copiez le fichier Adafruit_BMP280.h, renomez-le Adafruit_BMP280b.h //
// Identifiez la ligne : #define BMP280_ADDRESS (0x77)                                         //
// Modifiez la en : #define BMP280_ADDRESS (0x76)                                              //
// source https://www.tala-informatique.fr/wiki/index.php/Arduino_BMP280                       //
/////////////////////////////////////////////////////////////////////////////////////////////////

//Publication sur Thingspeak //
String apiKey = "XXXXXXXXXXXXXXXX";           //c'est ici qu'on place la clé d'API de Thingspeak 
const char* serverpub = "api.thingspeak.com"; //adresse du serveur thingspeak pour publier
WiFiClient client;                            //démarrage du client Wifi
int tempo = 0;                                //création de la variable tempo,
                                              //servira à temporiser l'envoie de donnée à Thingspeak

//////////Broches//////////
const int led1 = D0;
const int led2 = D5;
const int bp1 = D6;
const int bp2 = D7;

void setup() {

  Wire.begin();   // démarre le protocole I2C sur les broche par défaut SCL -> D1 et SDA -> D2
  
  //Prepare la broche GPIO2 (marquée D4 sur le Wemos)
  pinMode(2, OUTPUT);
  digitalWrite(2, 1); // éteint la led de la carte
  
  Serial.begin(9600); // démarrage de la connexion série
  dht.begin();        // démarrage du capteur dht11
  sensors.begin();    // demarrage capteur ds18b20
  bmp.begin(0x76);    // demarrage du bmp280 0x76

  //Auto connexion au cas ou le wemos ne trouve pas de réseau wifi "familier"
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");  
  
  Serial.println(""); 
  Serial.println("WiFi connecté");
  Serial.println("addresse IP : ");
  Serial.println(WiFi.localIP());  // impression sur le moniteur série de l'adresse IP du serveur du Wemos

  MDNS.begin(hostAP);
  MDNS.addService("http", "tcp", 80);
  serveur();

  server.begin();     // demarrage du serveur de fichiers
  digitalWrite(2, 0); // allumage de la led de la carte

  //Définition des broches des led1 et Led2 en sorties
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  //Définition des broches en entrées des boutons poussoirs
  pinMode(bp1, INPUT_PULLUP); //on utilise les résistances de pullup interne
  pinMode(bp2, INPUT_PULLUP);
  digitalWrite(bp2, HIGH);
}

void loop() {
  server.handleClient();
  thingspeak();
}

void thingspeak() { // Cette fonction envoie les données à Thingspeak
  if (tempo >= 20000){                           // Toutes les 20 secondes, les données sont envoyées
  float hygrometrie = dht.readHumidity();        // Lit l'Hygrométrie en % (par défaut)
  float temperature = dht.readTemperature();     // Lit la temperature en degrés Celsius (par défaut)
  int analog = analogRead(A0);                   // Lit l'entrée analogique de la station météo
  float pression = bmp.readPressure()/100.0;     // Lit la pression
  
  if (isnan(hygrometrie) || isnan(temperature)) {// Controle s'il y a des erreurs de lecture sur le capteur DHT11.
    Serial.println("");
    Serial.println("Lecture du capteur DHT : Echec !");
    thingspeak();
  }
  
  if (client.connect(serverpub,80)) {        // Contacte le site api.thingspeak.com
    String URL = apiKey;                     // Construit l'URL en ajoutant les différents éléments
           URL +="&field1=";
           URL += String(temperature);
           URL +="&field2=";
           URL += String(hygrometrie);
           URL +="&field3=";
           URL += String(pression);
           URL +="&field4=";
           URL += String(analog);
           URL += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(URL.length());
     client.print("\n\n");
     client.print(URL);
 
     Serial.println("Température: ");
     Serial.print(temperature);
     Serial.print(" degrés Celcius, Hygrométrie: ");
     Serial.print(hygrometrie);
     Serial.println("% Pression: ");
     Serial.print(pression);
     Serial.println("hPa Luninosité: ");
     Serial.print(analog);          
     Serial.println(" envoyés a Thingspeak");
     Serial.println(URL);
  }
  client.stop();
  Serial.println("patienter");
  tempo = 0;
  Serial.println(tempo);
  }
  // Le site Thingspeak a besoin de minimum 15 secondes de délais entre 2 envois de données
  tempo = tempo + 1;       //La variable tempo sert à compter 20 secondes (20 000 millisecondes).
  //Serial.println(tempo);  
  delay(1);                //Pause de 1 milliseconde
  return;
}

void serveur() { //Cette fonction nommée "Serveur" est celle qui va renvoyer les données lorsqu'on appelle les différentes pages
  // gestion de l'appel de la page http://adresseIP/temp
  // qui renvoit la valeur de la température mesurée par le capteur DHT11  
  server.on("/temp", HTTP_GET, []() {
    float temperature = dht.readTemperature();          // Lit la température et l'affecte à la variable température
    server.send(200, "text/json", String(temperature)); // Renvoie une page web avec le contenu de la variable température
  });

  // gestion de l'appel de la page http://adresseIP/hum
  // qui renvoit la valeur de l'hygrométrie mesurée par le capteur DHT11
  server.on("/hum", HTTP_GET, []() {                    
    float hygrometrie = dht.readHumidity();             // Lit l'hygrométrie et l'affecte à la variable hygrometrie
    server.send(200, "text/json", String(hygrometrie)); // Renvoie une page web avec le contenu de la variable hygrometrie
  });

  // gestion de l'appel de la page http://adresseIP/led1_on
  // qui met la broche "led1" en état haut.   
  server.on("/led1_on", HTTP_GET, []() {
    digitalWrite(led1, HIGH);                         // Met la broche led1 en HIGH, ce qui allume la led
    server.send(200, "text/json", "ON");              // Renvoie une page web avec le mot "ON"
  });
  
  // gestion de l'appel de la page http://adresseIP/led1_off
  // qui met la broche "led1" en état bas. 
  server.on("/led1_off", HTTP_GET, []() {
    digitalWrite(led1, LOW);                          // Met la broche led1 en LOW, ce qui éteint la led
    server.send(200, "text/json", "OFF");             // Renvoie une page web avec le mot "OFF"
  });

  // gestion de l'appel de la page http://adresseIP/led2_on
  // qui met la broche "led2" en état haut.   
  server.on("/led2_on", HTTP_GET, []() {
    digitalWrite(led2, HIGH);                         // Met la broche led2 en HIGH, ce qui allume la led
    server.send(200, "text/json", "ON");              // Renvoie une page web avec le mot "ON"
  });

  // gestion de l'appel de la page http://adresseIP/led2_off
  // qui met la broche "led2" en état bas. 
  server.on("/led2_off", HTTP_GET, []() {
    digitalWrite(led2, LOW);                          // Met la broche led2 en LOW, ce qui éteint la led
    server.send(200, "text/json", "OFF");             // Renvoie une page web avec le mot "OFF"
  });
  
  // gestion de l'appel de la page http://adresseIP/analog
  // qui renvoit la valeur reçue par la broche analogique du wemos - A0 
  server.on("/analog", HTTP_GET, []() {
    int analogique = analogRead(A0);                  // Lit la broche analogique A0 et met la valeur dans la variable "analogique" 
    Serial.println(analogique);                       // L'écrit sur le moniteur série
    server.send(200, "text/json", String(analogique));// Renvoie une page web avec le contenu de la variable "analogique"
  });

  // gestion de l'appel de la page http://adresseIP/bp1
  // qui renvoit la valeur de l'état du bouton poussoir 1  
  server.on("/bp1", HTTP_GET, []() {
    int etatbouton = digitalRead(bp1);                // Lit l'état du bouton et le met dans la variable "etatbouton" 
    if (etatbouton) {                                 // Si l'état du bouton est "vrai" (HIGH)
      server.send(200, "text/json", "OFF");           // Renvoie une page web avec le mot "OFF"
    } 
    else {                                            // Si l'état du bouton est "faux" (LOW - le bouton est relié au GND)
      server.send(200, "text/json", "ON");            // Renvoie une page web avec le mot "ON"
    }
  });

  // gestion de l'appel de la page http://adresseIP/bp2
  // qui renvoit la valeur de l'état du bouton poussoir 2 
  server.on("/bp2", HTTP_GET, []() {
    int etatbouton = digitalRead(bp2);                // Lit l'état du bouton et le met dans la variable "etatbouton" 
    if (etatbouton) {                                 // Si l'état du bouton est "vrai" (HIGH)
      server.send(200, "text/json", "OFF");           // Renvoie une page web avec le mot "OFF"
    } 
    else {                                            // Si l'état du bouton est "faux" (LOW - le bouton est relié au GND)
      server.send(200, "text/json", "ON");            // Renvoie une page web avec le mot "ON"
    }
  });

  // gestion de l'appel de la page http://adresseIP/temp_ds
  // qui renvoit la valeur de la température mesurée par le capteur ds18b20  
  server.on("/temp_ds", HTTP_GET, []() {
    sensors.requestTemperatures();
    int temperature = sensors.getTempCByIndex(0);       // Lit la température et l'affecte à la variable temperature
    server.send(200, "text/json", String(temperature)); // Renvoie une page web avec le contenu de la variable temperature
    digitalWrite(2, 0);
  });

  // gestion de l'appel de la page http://adresseIP/temp_bmp 
  // qui renvoit la valeur de la température mesurée par le capteur bmp280
  server.on("/temp_bmp", HTTP_GET, []() {   
    int temperature = bmp.readTemperature();            // Lit la température et l'affecte à la variable temperature
    server.send(200, "text/json", String(temperature)); // Renvoie une page web avec le contenu de la variable temperature
  });

  // gestion de l'appel de la page http://adresseIP/press_bmp 
  // qui renvoit la valeur de la pression mesurée par le capteur bmp280
  server.on("/press_bmp", HTTP_GET, []() {
    float pression = bmp.readPressure()/100.0;          // Lit la pression et l'affecte à la variable pression
    server.send(200, "text/json", String(pression));    // Renvoie une page web avec le contenu de la variable pression
  });
}
