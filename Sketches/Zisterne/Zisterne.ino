// Zisterne

/*********

 Originalcode: Rui Santos

 Complete project details at https://randomnerdtutorials.com

 Full example: https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/

 Dashboard: 192.168.222.1:1880/ui

*********/


// WiFi

#include <WiFi.h>


// MQTT

#include <PubSubClient.h>



// WLAN name and password

const char* ssid = "thielepi5_uj";

const char* password = "Pi4ever!";



// IP-Adress of MQTT-broker (running on raspberrypi)

const char* mqtt_server = "192.168.222.1";

const char* mqtt_client = "ESP32_Ultraschall";


// ESP32 is a client of the broker

WiFiClient espClient;

PubSubClient client(espClient);



// Variables for messages

long lastMsg = 0;

char msg[50];

int value = 0;

char entfernungch[50];


// Ultraschall

int trigger = 33; //Trigger-Pin des Ultraschallsensors

int echo = 32; // Echo-Pim des Ultraschallsensors


long dauer = 0; // Das Wort dauer ist jetzt eine Variable, unter der die Zeit gespeichert wird, die eine Schallwelle bis zur Reflektion und zurück benötigt. Startwert ist hier 0.

long entfernung = 0; // Das Wort „entfernung“ ist jetzt die variable, unter der die berechnete Entfernung gespeichert wird. Info: Anstelle von „int“ steht hier vor den beiden Variablen „long“. Das hat den Vorteil, dass eine größere Zahl gespeichert werden kann. Nachteil: Die Variable benötigt mehr Platz im Speicher.


//// Pin LED

//const int ledcontrol = 33;

//

//

//// Pin of switch

//const int sw1 = 25;




/*********

 * setup()

 * Prinzipieller Aufbau

 * ...

 * —> setup_wifi()        WIFI starten (Funktion befindet sich am Dateiende)

 * ...

 * —> setCallback(callback)   Der ESP32 ruft Nachrichten vom Broker ab und ruft dann callback auf.

 *                Dort wird entschieden, wie mit den Nachrichten weiter verfahren wird.

 *

 */


void setup() {

 Serial.begin(115200);



 setup_wifi();             // WiFi-Connetion



 client.setServer(mqtt_server, 1883); // MQTT: set broker and port

 client.setCallback(callback);     // set callback routine


 pinMode(trigger, OUTPUT); // Trigger-Pin ist ein Ausgang

 pinMode(echo, INPUT); // Echo-Pin ist ein Eingang


}





/*********

 * callback()                    Aktion, wenn Nachricht empfangen

 * Prinzipieller Aufbau

 *

 * Mehr unter

 * https://techtutorialsx.com/2017/04/24/esp32-subscribing-to-mqtt-topic/

 *

 * Diese Funktion wird aufgerufen, wenn eine Nachricht empfangen wird.

 * Diese Funktion wird von der MQTT-Klasse client aufgerufen (siehe ganz oben).

 * In dieser Funktion werden alle Steuerungsaktionen ausgeführt (hier LED an/aus).

 *

 * (Es wird nicht die letzte, sondern die nächste Nachricht empfangen.

 * Eventuell gibt es noch weitere Nachrichten in der Pipeline.)

 *

 * ...

 * --> String messageTemp;             Zunächst wird die Nachricht in einem String gespeichert

 *                         message könnte evtl. auch ein Bild enthalten!

 * ...

 * --> if (String(topic) == "esp32/output") {   Wenn unter diesem Kanal etwas ankommt --> LED schalten

 * ...                       topic: der Kanal  messageTemp: der Inhalt der Nachricht

 *   }

 *

 */


void callback(char* topic, byte* message, unsigned int length) {

 String messageTemp;   // Message wird in String gespeichert

 for (int i = 0; i < length; i++) {

  messageTemp += (char)message[i];

 }



 Serial.print("Topic: ");

 Serial.print(topic);

 Serial.print(" Message: ");

 Serial.println(messageTemp);


// if (String(topic) == "esp32/light1") {    // message on topic esp32/setled

//  if(messageTemp == "on"){

//   digitalWrite(1, HIGH);

//  } else if(messageTemp == "off"){

//   digitalWrite(1, LOW);

//  }

// }

}



/*********

 * loop()

 * Prinzipieller Aufbau

 *

 * —> reconnect()        Zunächst verbinden, falls nötig (Funktion befindet sich am Dateiende)

 *

 * —> client.loop()       Ruft Nachrichten vom Broker ab und daraufhin die Funktion callback

 *                https://techtutorialsx.com/2017/04/24/esp32-subscribing-to-mqtt-topic/

 *                „In the main loop function, we will need to call the loop method of the PubSubClient.

 *                As indicated in the documentation of the library, the function should be called on a

 *                regular basis, in order to allow the client to process incoming messages and maintain

 *                the connection to the MQTT server.“

 *

 * —> client.publish("esp32/feedback", bString);

 *                Alle fünf Sekunden wird angezeigt, wie oft das Licht an- und ausgeschaltet wurde.

 *                Das wird im Dashboard dargestellt.

 *                Hier ist der passende Platz, um beispielsweise eine Temperatur zu veröffentlichen!

 */


int state1;

int oldstate1 = LOW;


void loop() {

 if (!client.connected()) {

  reconnect();

 }

 client.loop();


 long now = millis();              // Aktuelle Zeit


 if (now - lastMsg > 2000) {

  lastMsg = now;                // Zeit merken für nächsten Durchlauf


  digitalWrite(trigger, LOW); //Hier nimmt man die Spannung für kurze Zeit vom Trigger-Pin, damit man später beim senden des Trigger-Signals ein rauschfreies Signal hat.

  delay(5); //Dauer: 5 Millisekunden

  digitalWrite(trigger, HIGH); //Jetzt sendet man eine Ultraschallwelle los.

  delay(10); //Dieser „Ton“ erklingt für 10 Millisekunden.

  digitalWrite(trigger, LOW);//Dann wird der „Ton“ abgeschaltet.



  dauer = pulseIn(echo, HIGH); //Mit dem Befehl „pulseIn“ zählt der Mikrokontroller die Zeit in Mikrosekunden, bis der Schall zum Ultraschallsensor zurückkehrt.

  entfernung = (dauer / 2) * 0.03432; //Nun berechnet man die Entfernung in Zentimetern. Man teilt zunächst die Zeit durch zwei (Weil man ja nur eine Strecke berechnen möchte und nicht die Strecke hin- und zurück). Den Wert multipliziert man mit der Schallgeschwindigkeit in der Einheit Zentimeter/Mikrosekunde und erhält dann den Wert in Zentimetern.

  if (entfernung >= 500 || entfernung <= 0) //Wenn die gemessene Entfernung über 500cm oder unter 0cm liegt,…

  {

   Serial.println("Kein Messwert"); //dann soll der serial monitor ausgeben „Kein Messwert“, weil Messwerte in diesen Bereichen falsch oder ungenau sind.

   client.publish("esp32/ultraschal","Kein Messwert");

  }

  else // Ansonsten…

  {

   Serial.print(entfernung); //…soll der Wert der Entfernung an den serial monitor hier ausgegeben werden.

   Serial.println(" cm"); // Hinter dem Wert der Entfernung soll auch am Serial Monitor die Einheit "cm" angegeben werden.

   dtostrf(entfernung,6,1,entfernungch);

   client.publish("esp32/ultraschall",entfernungch); // String publizieren

  }


 }



}









/*********

 * setup_wifi()    WIFI-Verbindung einrichten

 *

 *

 */



void setup_wifi() {

 delay(10);

 // Connecting to a WiFi network

 Serial.print("Connecting to ");

 Serial.println(ssid);

 WiFi.begin(ssid, password);


 // Waiting for connection

 while (WiFi.status() != WL_CONNECTED) {

  delay(500);

  Serial.print(".");

 }


 // Is connected

 Serial.print("IP address: ");

 Serial.println(WiFi.localIP());

}



/*********

 * reconnect()                   Verbindung wird zu Beginn der loop()-Schleife gemacht

 * Prinzipieller Aufbau               Das ist eigentlich eine setup-Routine, aber die Verbindung könnte ja abbrechen

 *

 * -->  while (!client.connected()) {       Solange nicht zum MQTT-Server verbunden

 *     ...                   --> versuche Verbindung

 *     if (client.connect("ESP32Client")) {  Wenn verbunden

 *      client.subscribe("esp32/#");     Melde zu allen esp32-Kanälen an; Achtung: auch das feedback ist beinhaltet!

 *     } else {

 *      ...                  Warte mit nächster Anmeldung

 *     }

 *    }

 *

 */


void reconnect() {

 while (!client.connected()) {             // Loop until we're reconnected

  // Serial.print("Attempting MQTT connection...");

  if (client.connect(mqtt_client)) {        // Attempt to connect

   // Serial.println("connected");           // Subscribe to all of this channel:

   // client.subscribe("esp32/#");

  } else {

   Serial.print("Attemption of MQTT connection...");

   Serial.print("failed, rc=");

   Serial.println(client.state());

   delay(5000);                   // Wait 5 seconds before retrying

  }

 }

}
