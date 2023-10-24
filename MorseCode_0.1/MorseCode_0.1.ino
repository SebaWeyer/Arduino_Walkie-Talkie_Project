#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RF24.h>

/* 
  PARAMETROS DEL CODIGO MORSE 
  PUNTO: T
  RAYA:  3T
  ESPACIO ENTRE CARACTERES: T
  ESPACIO ENTRE LETRAS:    3T
  ESPACIO ENTRE PALABRAS:  7T
*/

#define MORSE_BUTTON 10 // Pulsador para código morse
#define DELETE_BUTTON 3 // Pulsador para borrar último caracter
#define SEND_BUTTON 2   // Pulsador para enviar mensaje
#define DOT_LED 6       // LED para mostrar punto durante la codificación en morse
#define LINE_LED 9      // LED para mostrar linea durante la codificación en morse
#define TX_LED 5        // LED para mostrar estado de emisión
#define RX_LED 4        // LED para mostrar estado de recepción

const String line = "-";
const String dot = "*";
const int T = 200;              // Parametro de tiempo que determina tiempo de punto, raya y espacios

String morse_character = "";    // String que guarda los Puntos y Lineas
String alfanum_character = "";  // String para guardar la traducción del código morse
String my_word = "";            // String que almacena las letras
int button_state = LOW;         // Estado actual del boton
int last_button_state = LOW;    // Estado anterior del boton
long signal_time = 0;           // Contador de duración de la señal (pulsador presionado)
long pause_time = 0;            // Contador de duración de espacio (pulsador sin presionar)
boolean char_check = false;     // Se encarga de comprobar si se dejó de escribir la Letra, para luego traducirla
boolean word_check = false;     // Se encarga de comprobar si se debe realizar un salto de linea
boolean transmitting_flag = false;

int cursor = 0;         // Variable para el LCD, para el borrado y la Impresion de las letras


String translate(String);
LiquidCrystal_I2C lcd(0x20, 16, 2);  // Incializacion LCD I2C

void setup() {
  Serial.begin(9600);  // Inicialización de la comunicación serial
  lcd.init();          // Inicialización del LCD en el programa
  lcd.backlight();     // Retroiluminado LCD

  pinMode(MORSE_BUTTON, INPUT);
  pinMode(DELETE_BUTTON, INPUT);
  pinMode(SEND_BUTTON, INPUT);
  pinMode(DOT_LED, OUTPUT);
  pinMode(LINE_LED, OUTPUT);
  pinMode(TX_LED, OUTPUT);
  pinMode(RX_LED, OUTPUT);

  Serial.println("\nBienvenido a la Prueba de traductor de Codigo Morse");
  Serial.println("Usa el boton para enviar un punto o una linea, despues será traducido y se mostrará en el monitor en serie");
  Serial.println("https://i.pinimg.com/originals/73/68/b6/7368b695bbb97dfb1d61d6e262b28d6f.jpg Tabla de Codigo Morse");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRADUCTOR");
  lcd.setCursor(0, 1);
  lcd.print("MORSE <Ver_0.1>");
  delay(500);
  lcd.clear();

  while (!digitalRead(MORSE_BUTTON));

}

void loop() {

  button_state = digitalRead(MORSE_BUTTON);

  if (button_state && last_button_state)  // Si el pulsador estaba pulsado y sigue pulsado...
  {
    signal_time++;
    if (signal_time < (3*T))
    {
      // Punto
      digitalWrite(DOT_LED, HIGH);
      digitalWrite(LINE_LED, LOW);
    } 
    else 
    {
      // Linea
      digitalWrite(DOT_LED, LOW);
      digitalWrite(LINE_LED, HIGH);
    }
  } 
  else if (!button_state && last_button_state)  // Si se soltó el pulsador pero antes estaba pulsado...
  {
    if (signal_time >= T && signal_time < (3*T)) 
    {
      morse_character += dot;    // Concatenar un punto (*)
    } 
    else if (signal_time >= (3*T))
    {
      morse_character += line;  // Concatenar una línea (-)
    }
    signal_time = 0;  // Reiniciar el conteo de tiempo de pulsado
    digitalWrite(DOT_LED, LOW);
    digitalWrite(LINE_LED, LOW);

    // Esta parte envia los Puntos y Lineas al serialMonitor y al LCD
    Serial.print(morse_character);
    lcd.setCursor(0, 1);
    lcd.print(morse_character);
  } 
  else if (button_state && !last_button_state)  // Si se presionó el pulsador pero antes no estaba pulsado...
  {
    // Se volvió a presionar el pulsador de morse, así que se vuelve a habilitar la posibilidad de un espacio
    pause_time = 0;
    char_check = true;
    word_check = true;
  }
  else if (!button_state && !last_button_state) // Si el pulsador no está pulsado ni lo estaba...
  {
    pause_time ++;
    if ((pause_time >= (3*T)) && char_check)
    {
      lcd.setCursor(cursor, 0);
      alfanum_character = translate(morse_character);
      Serial.print(alfanum_character);
      lcd.print(alfanum_character);
      my_word += alfanum_character;
      char_check = false;     // Deshabilitar la repetición de espacios
      morse_character = "";   // Limpiar lo que hay en morse para empezar un nuevo procesamiento
      cursor ++;
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    if ((pause_time >= (7*T)) && word_check)
    {
      Serial.print(" ");
      lcd.print(" ");
      my_word += " ";
      word_check = false;
      cursor++;
    }
  }
  
  // Acción del Boton de Borrado
  if (digitalRead(DELETE_BUTTON) == HIGH) 
  {  
    if (my_word.length() > 0) 
    {
      cursor --;
      lcd.setCursor(cursor, 0);
      lcd.print(" ");
      my_word.remove((my_word.length()-1), 1);
      Serial.println(my_word);
      digitalWrite(LINE_LED, HIGH);
      digitalWrite(DOT_LED, HIGH);
      delay(500);
      digitalWrite(LINE_LED, LOW);
      digitalWrite(DOT_LED, LOW);
    }
  }

  // Acción del Botón de Envío
  if (digitalRead(SEND_BUTTON) == HIGH) 
  {
    transmitting_flag = false;
    if (my_word == "") 
    {
      lcd.print("   NO MESSAGE");
      delay(1000);
      lcd.clear();
    } 
    else 
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(my_word);
      lcd.setCursor(0, 1);
      lcd.print("     Enviando...");
      delay(200);
      lcd.setCursor(0, 1);
      lcd.print("        Enviado!");
      delay(200);
      lcd.clear();
      my_word = "";
    }
  }

  last_button_state = button_state;
  delay(1);
}


// Parte del codigo que se encarga de traducir los Puntos y Lineas por Letras o Numeros
// TRADUCCIÓN
String translate (String traduct) {
  String translation = "";
  if (traduct == "*-") 
  {
    translation = "A";
  } 
  else if (traduct == "-***") 
  {
    translation = "B";
  }
  else if (traduct == "-*-*") 
  {
    translation = "C";
  } 
  else if (traduct == "-**") 
  {
    translation = "D";
  } 
  else if (traduct == "*") 
  {
    translation = "E";
  } 
  else if (traduct == "**-*") 
  {
    translation = "F";
  } 
  else if (traduct == "--*") 
  {
    translation = "G";
  } 
  else if (traduct == "****") 
  {
    translation = "H";
  }
  else if (traduct == "**") 
  {
    translation = "I";
  } 
  else if (traduct == "*---") 
  {
    translation = "J";
  }
  else if (traduct == "-*-") 
  {
    translation = "K";
  } 
  else if (traduct == "*-**") 
  {
    translation = "L";
  } 
  else if (traduct == "--") 
  {
    translation = "M";
  } 
  else if (traduct == "-*") 
  {
    translation = "N";
  } 
  else if (traduct == "---") 
  {
    translation = "O";
  }
  else if (traduct == "*--*") 
  {
    translation = "P";
  } 
  else if (traduct == "--*-") 
  {
    translation = "Q";
  } 
  else if (traduct == "*-*") 
  {
    translation = "R";
  } 
  else if (traduct == "***") 
  {
    translation = "S";
  } 
  else if (traduct == "-") 
  {
    translation = "T";
  } 
  else if (traduct == "**-") 
  {
    translation = "U";
  }
  else if (traduct == "***-") 
  {
    translation = "V";
  } 
  else if (traduct == "*--") 
  {
    translation = "W";
  } 
  else if (traduct == "-**-") 
  {
    translation = "X";
  } 
  else if (traduct == "-*--") 
  {
    translation = "Y";
  } 
  else if (traduct == "--**") 
  {
    translation = "Z";
  }
  else if (traduct == "*----") 
  {
    translation = "1";
  } 
  else if (traduct == "**---") 
  {
    translation = "2";
  } 
  else if (traduct == "***--") 
  {
    translation = "3";
  } 
  else if (traduct == "****-") 
  {
    translation = "4";
  } 
  else if (traduct == "*****") 
  {
    translation = "5";
  } 
  else if (traduct == "-****") 
  {
    translation = "6";
  } 
  else if (traduct == "--***") 
  {
    translation = "7";
  } 
  else if (traduct == "---**") 
  {
    translation = "8";
  } 
  else if (traduct == "----*") 
  {
    translation = "9";
  } 
  else if (traduct == "-----") 
  {
    translation = "0";
  }
  else if (traduct == "*-*-*-") 
  {
    translation = ".";
  } 
  else if (traduct == "-**-*") 
  {
    translation = "/";
  } 
  else if (traduct == "-****-") 
  {
    translation = "-";
  } 
  else if (traduct == "*-*-*") 
  {
    translation = "+";
  }
  else if (traduct == "-***-") 
  {
    translation = "=";
  } 
  else if (traduct == "**---**") 
  {
    translation = "?";
  }

  return translation;
}