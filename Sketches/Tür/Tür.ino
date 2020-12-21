//Tür

/* VERKABELUNG
  Servomotor:
 * * Braun GND
 * * Rot  5V
 * * Gelb  G14

  RFID:
 (siehe auch www.instructables.com/ESP32-With-RFID-Access-Control/)
 * * SDA  G21
 * * SCK  G18
 * * MOSI  G23
 * * MISO  G13
 * * IRQ  leer
 * * GND  GND
 * * RST  G22
 * * 3.3V  3.3V
*/

#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 21
#define RST_PIN 22
#define SERVOPIN 14


MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servoblau;
long lcode;
int state = 0;

void setup()
{
 Serial.begin(115200);
 SPI.begin();
 mfrc522.PCD_Init();
 pinMode (2, OUTPUT); // Der Pin 2 ist jetzt ein Ausgang (Hier wird eine LED angeschlossen)
 servoblau.attach(SERVOPIN); //Das Setup enthält die Information, dass das Servo an der Steuerleitung (gelb) mit Pin 8 verbunden wird. Hier ist natürlich auch ein anderer Pin möglich.}
}

long getcode(void)

{
 long code = 0;
 for (byte i = 0; i < mfrc522.uid.size; i++)
 {
  code = ((code + mfrc522.uid.uidByte[i]) * 10);
 }
 return (code);
}


void loop()
{
 if ( ! mfrc522.PICC_IsNewCardPresent())
 {
  return;
 }

 if ( ! mfrc522.PICC_ReadCardSerial())
 {
  return;
 }


 lcode = getcode();
 Serial.print("2182820  ");
 Serial.println(lcode);

 if ( lcode == 2182820 ) 
 {
  if ( state == 1 ) {
   state = 0;
   Serial.println("Closing");
   digitalWrite (2, LOW); // … und danach wieder aus gehen.
   servoblau.write(0); //Position 'geschlossen' ansteuern mit dem Winkel 0°
  }

  else 
  {
   state = 1;
   Serial.println("Opening");
   digitalWrite (2, HIGH); // ...dann soll die LED an Pin 2 leuchten...
   servoblau.write(180); //Position 'offen' ansteuern mit dem Winkel 0°
  }
 }

 delay(4000);
  
} // Sketch abschließen
