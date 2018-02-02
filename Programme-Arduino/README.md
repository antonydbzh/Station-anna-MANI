# Il y a plusieurs programmes :
* Plusieurs versions de celui qui doit être chargé sur le Wemos avec le logiciel Arduino.
* La bibliothèque adafruit modifiée pour le capteur BMP280 version GY

# Programme "station-meteo-anna-mani-manuelle-V1"
Dans ce programme on rentre manuellement le nom du réseau Wifi ainsi que le mot de passe de celui-ci.
Vous pouvez également :
* La Led de la carte clignote tant que la connexion n'est pas établie (elle est allumée si la connexion fonctionne).
* Activer la publication sur Thingspeak (avec une clé d'API ET en dé-commentant la fonction Thingspeak() dans la boucle "Void Loop".
* Modifier le nom de domaine de la station.

# Programme "station-meteo-anna-mani-v2"
* Version en cours de détermination !
Idées :
* régler le problème "thingspeak" : boucle trop rapide si pas de capteur DHT qui provoque la déconnexion.
* meilleur suivi des opération sur le moniteur série
* bouton sur une page web.
* second bouton qui twitte ou autre (type IFTTT).
* Se fixer comme objectif de se passer de l'IDE Arduino une foi le programme chargé ?
*...

# Programme " station-meteo-anna-mani-v3"
* En cours de modification pour activer l'autoconnexion avec un bouton poussoir
dans ce programme vous pouvez manuellement :
* Activer la publication sur Thingspeak (avec une clé d'API ET en dé-commentant la fonction Thingspeak() dans la boucle "Void Loop"
* Modifier le nom de domaine de la station

Un tutoriel explique la modification faite à la bibliothèque adafruit.
https://www.tala-informatique.fr/wiki/index.php/Arduino_BMP280 
