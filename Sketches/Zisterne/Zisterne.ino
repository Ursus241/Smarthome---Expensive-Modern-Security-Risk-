// Zisterne

/*********
 Originalcode: Rui Santos
 Complete project details at https://randomnerdtutorials.com
 Full example: https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
 Dashboard: 192.168.222.1:1880/ui
*********/

#include <WiFi.h>

//MQTT
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

void setup() 
{
 Serial.begin(115200);
 setup_wifi();             // WiFi-Connetion
 client.setServer(mqtt_server, 1883); // MQTT: set broker and port
 client.setCallback(callback);     // set callback routine
 pinMode(trigger, OUTPUT); // Trigger-Pin ist ein Ausgang
 pinMode(echo, INPUT); // Echo-Pin ist ein Eingang
}

void callback(char* topic, byte* message, unsigned int length) 
{
 String messageTemp;   // Message wird in String gespeichert
 for (int i = 0; i < length; i++) {
 messageTemp += (char)message[i];
 }

 Serial.print("Topic: ");
 Serial.print(topic);
 Serial.print(" Message: ");
 Serial.println(messageTemp);

int state1;
int oldstate1 = LOW;

void loop() 
{
 if (!client.connected()) 
 {
  reconnect();
 }
 
 client.loop();

 long now = millis();              // Aktuelle Zeit

 if (now - lastMsg > 2000) 
 {
  lastMsg = now;                // Zeit merken für nächsten Durchlauf
  digitalWrite(trigger, LOW); //Hier nimmt man die Spannung für kurze Zeit vom Trigger-Pin, damit man später beim senden des Trigger-Signals ein rauschfreies Signal hat.
  delay(5); //Dauer: 5 Millisekunden
  digitlWrite(trigger, HIGH); //Jetzt sendet man eine Ultraschallwelle los.
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
