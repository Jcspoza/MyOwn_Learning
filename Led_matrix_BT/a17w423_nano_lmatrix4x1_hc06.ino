/* TOPIC : MATRIZ LED MAX 7219
 *  MCU : NANO ELEGOO
 *  DIA : 19-OCT 2017
 *  oBJETIVO : 
 *  
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <SoftwareSerial.h>

#define DEBUG
#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
 #define NODEBUG_SERIALEND
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define NODEBUG_SERIALEND         Serial.end()
#endif

/*
 Conector ICSP
               -------
MISO - ICSP-1  |*   *|   ICSP-2  - +5 volt
SCK  - ICSP-3  |*   *|   ICSP-4  - MOSI
RESET- ICSP-5  |*   *|   ICSP-6  - GND
               -------
 */
int pinCS = 7; // Attach CS to this pin, DIN to MOSI-ICSP4 and CLK to SCK ICSP3 (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

SoftwareSerial BT1(8,4); // RX nano en D8--> TX HC06, TX nano ne D4--> RX HC06recorder que se cruzan


String tape = " "; // inicializacion
int wait = 1000; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

void setup() {
  Serial.begin (9600);
  BT1.begin(9600);
  Serial.print("Skecht :");
  Serial.print(__FILE__);
  Serial.print("\t"); 
  Serial.println("SPI x conector ICSP, cd => D7 - Rotacion 3 - DEBUG");
  Serial.println("Nano, RX nano en D8--> TX HC06, TX nano ne D4--> RX HC06");
  Serial.println("Objetivo : probar una matriz 4x1 - texto: desde BT HC06");
 // NODEBUG_SERIALEND; // sI NO ESTOY EN DEBUG CIERRO EL PUERTO SERIA JUSTO DESPUES DE ESCRIBIR EL SKETCH

// Inicializa matriz led 
matrix.setIntensity(7); // Use a value between 0 and 15 for brightness

// Adjust to your own needs
matrix.setPosition(0, 3, 0); // The first display is at <0, 0>
matrix.setPosition(1, 2, 0); // The second display is at <1, 0>
matrix.setPosition(2, 1, 0); // The third display is at <2, 0>
matrix.setPosition(3, 0, 0); // And the last display is at <3, 0>
//  ...
matrix.setRotation(0, 3);    // The first display is position 90 counter clock
matrix.setRotation(1, 3);
matrix.setRotation(2, 3);
matrix.setRotation(3, 3);   // The same hold for the last display
matrix.fillScreen(LOW);
matrix.drawChar(0, 0, '?', HIGH, LOW, 1);
matrix.write(); // Send bitmap to display limpia matriz led
// Fin inicializacion matriz led

/*
 * depurar mucho
 * 1 longitud del texto 
 * 2 que el BT este conectado
 */
DEBUG_PRINTLN("espera texto");
 while (!BT1.available())delay(5);
 tape = GetLine();
 DEBUG_PRINT("Texto: ");
 DEBUG_PRINTLN(tape); 

}

String GetLine()
{   String S = "" ;
    char c = -1;
                while (BT1.available()>0 )            //Hasta que NO QUEDEN CARACTERES
                  {     c = BT1.read();
                        S = S + c ;
                        delay(25);
                  }
                return(S);
}

void loop() 
{
  for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ ) 
  {
    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
   
    // Write all letters: letter, letter-1 , etc each in x , x-witdth, etc
    while ( x + width - spacer >= 0 && letter >= 0 ) 
    { 
      if ( letter < tape.length() ) 
      {
        matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display

    //    
      delay(wait);
   }
}
