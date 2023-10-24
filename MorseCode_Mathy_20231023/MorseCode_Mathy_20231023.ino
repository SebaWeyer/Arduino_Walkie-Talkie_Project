#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* 
    PARAMETROS DEL CODIGO MORSE 
   PUNTO: T
   RAYA:  3T
   ESPACIO ENTRE CARACTERES: T
   ESPACIO ENTRE LETRAS:    3T
   ESPACIO ENTRE PALABRAS:  7T
   
*/

const int morBUT  = 7;       // Pin BOTON MORSE
const int borrBUT = 3;       // Pin BOTON BORRADO
const int envBUT  = 4;       // Pin BOTON ENVIO
const int morLED  = 13;      // Pin LED MORSE
const int onLED   = 12;       // Pin LED ON
const int buzzer  = 9;		   // Pin BUZZER


int morLEDState = HIGH;        // Estado del morLED
int buttonState = LOW;      // Estado actual del boton
int lastButtonState = LOW;  // Estado anterior del boton

int T = 500;            // Parametro de Tiempo, determina tiempo de punto, raya y espacios (1seg en este caso)
int pause_value = 250;  // Tiempo que se encarga de determinar si estás ingresando un Punto o una Linea (cambiar si es muy lento o muy rapido)
int envChar = 250;      // Tiempo que tarda en enviar el caracter traducido

long signal_length = 0;	// Duracion de la Señal
long pause = 0;         // Se encarga de cortar la palabra y empezar una nueva. Salto de Linea

String morse = "";      // String que guarda los Puntos y Lineas
String linea = "-";     // String de Linea
String punto = "*";     // String de Punto
String palabra = "";    // String que almacena las letras
int cursor = 0;         // Variable para el LCD, para el borrado y la Impresion de las letras

boolean check = false;        // Se encarga de comprobar si se dejó de escribir la Letra, para luego traducirla
boolean linecheck = false;    // Se encarga de comprobar si se debe realizar un salto de linea

LiquidCrystal_I2C lcd(0x20, 16, 2); // Incializacion LCD I2C

void setup()
{
  Serial.begin(9600);
  
  //Inicializo el LCD y el Backlight
  lcd.init();
  lcd.backlight();
  
  pinMode(morBUT,  INPUT);
  pinMode(borrBUT, INPUT);
  pinMode(envBUT,  INPUT);
  pinMode(morLED,  OUTPUT);
  pinMode(buzzer,  OUTPUT);
  pinMode(onLED,   OUTPUT);
  
  morse = "";
  palabra = " ";
  
  Serial.println("\nBienvenido a la Prueba de traductor de Codigo Morse");
  Serial.println("Usa el boton para enviar un punto o una linea, despues será traducido y se mostrará en el SerialMonitor");
  Serial.println("https://i.pinimg.com/originals/73/68/b6/7368b695bbb97dfb1d61d6e262b28d6f.jpg Tabla de Codigo Morse");

  lcd.setCursor(0,0);
  lcd.print("TRADUCTOR  ");
  lcd.setCursor(0,1);
  lcd.print("MORSE <Ver1.2.1>"); //Ver 1.2.1 20/10 //Ver1.2 17/10 //Ver1.1 10/10 //Ver1.0 3/10
  delay(500);
  lcd.clear();
  
  while(!digitalRead(morBUT));
}

void loop() {
  
  digitalWrite(onLED, HIGH);
  buttonState = digitalRead(morBUT);
  
  if (buttonState && lastButtonState)       // basic state machine depending on the state of the signal from the button
  {
    ++signal_length;       
    if (signal_length<T)                    // Si la duracion de la señal es mayor al tiempo de pausa*2 significa que es un punto y no una linea
    {                                      
    tone(buzzer, 1500) ;                    // Este Buzzer indica un tono de Alta Frecuencia, UN PUNTO
    }
    else
    {
      tone(buzzer, 1000) ;                  // Este Buzzer indica un tono de una frecuencia menor, UNA LINEA
    }
  }
  else if(!buttonState && lastButtonState)          //Detecta el estado del Boton, dependiendo la logitud de la señal (if anterior) entonces se agrega un Punto o una Linea
  {
     if (signal_length>50 && signal_length<T )
     {
       morse =  morse + punto;                        // Señal Corta, PUNTO(*)
     } 
      else if (signal_length>T)                     // El valor (*) o (-) se guarda en "morse"
      {
        morse = morse +  linea;                      // Señal Larga, LINEA (-)
      }
    signal_length=0;                                // Acá reinicia el proceso de identificación, reiniciando la longitud de la señal a 0
    digitalWrite(morLED, LOW); 
    noTone(buzzer);
    
    // Esta parte envia los Puntos y Lineas al serialMonitor y al LCD
    Serial.print(morse);
    Serial.print("/");
    lcd.setCursor(0,1);
    lcd.print(morse);
  }
  else if(buttonState && !lastButtonState)          // Esta parte se encarga de reiniciar valores, iniciando una nueva letra
  {
    pause=0;
    digitalWrite(morLED, HIGH);  
    check = true;
    linecheck = true;
  }
  else if (!buttonState && !lastButtonState)
  {  
    ++pause;
    if (( pause>3*T ) && (check))      // De acuerdo a los parametros, el tiempo entre letra y letra es 3*T, asi que se usará eso para determinar el final de una palabra
    { 
      lcd.setCursor(cursor,0);
      traduccion(morse);               // Acá se envia lo que se tradujo en el SerialMonitor
      check = false;
      morse = "";                      // Acá se borra lo que hay en morse para empezar una nueva Palabra
      cursor = cursor + 1;             // Esta parte se encarga de borrar el Codigo morse mostrado en la parte de abajo del LCD
      Serial.print(cursor);
      lcd.setCursor(0,1);
      lcd.print("                ");
    }
  }
  if (digitalRead(borrBUT) == HIGH) {     // Acción del Boton de Borrado
    cursor = cursor - 1;
    lcd.setCursor(cursor, 0);
    lcd.print(" ");
    palabra =+ "";
    tone(buzzer, 2000);
    delay(500);
    noTone(buzzer);
    delay(1000);
    if (cursor < 0){
      cursor = 0;
    }
  }

  /*

   HACER LA SEMANA QUE VIENE (EN INTEGRACION DE ULTIMA)

   HACER ACA EL BOTON DE ENVIO. NO SEAS BOLUDO
   ACA EN O CUALQUIER LADO PERO HACERLO Y PROBAR EN FISICO (PROTOBOARD)
   TERMINA DE GUARDAR LA PALABRA EN EL STRING PARA ENVIARLA
   CHARLY YA TIENE UN INICIO DE COMO HACERLO
   PROBA EN SERIALMONITOR PRIMERO Y DESPUES EN LCD GORDO TETA
   NOS VEMOS UWU

  */

  if (digitalRead(envBUT) == HIGH){
    if (palabra == "" ){
      lcd.print("   NO MESSAGE");
      delay(1000);
      lcd.clear();
    }
    else{
        lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(palabra);
    lcd.setCursor(0,1);
    lcd.print("     Enviando...");
    delay(3000);
    lcd.setCursor(0,1);
    lcd.print("        Enviado!");
    delay(1000);
    lcd.clear();
    palabra = "";
    }
  }

  lastButtonState=buttonState;
  delay(1);
}


// Parte del codigo que se encarga de traducir los Puntos y Lineas por Letras o Numeros
// TRADUCCIÓN

void traduccion(String traduct)   
{                                 
  if (traduct=="*-")
  {
    lcd.setCursor (cursor,0);
    Serial.print("A");
    lcd.print("A");
    palabra += "A"; 
  }
  else if (traduct=="-***"){
    Serial.print("B");
    lcd.print("B");
    palabra += "B"; 
  }
  else if (traduct=="-*-*")  {
    Serial.print("C");
    lcd.print("C");
    palabra += "C"; 
  }
  else if (traduct=="-**")  {
    Serial.print("D");
    lcd.print("D");
  }
  else if (traduct=="*")  {
    Serial.print("E");
    lcd.print("E");
    palabra += "E"; 
  }
  else if (traduct=="**-*")  {
    Serial.print("F");
    lcd.print("F");
  }
  else if (traduct=="--*")  {
    Serial.print("G");
    lcd.print("G");
  }
  else if (traduct=="****")  {
    Serial.print("H");
    lcd.print("H");
    palabra += "H"; 
    
  }
  else if (traduct=="**")  {
    Serial.print("I");
    lcd.print("I");
    palabra += "I"; 
  }
  else if (traduct=="*---")  {
    Serial.print("J");
    lcd.print("J");
  }
  else if (traduct=="-*-")  {
    Serial.print("K");
    lcd.print("K");
  }
  else if (traduct=="*-**")  {
    Serial.print("L");
    lcd.print("L");
  }
  else if (traduct=="--")  {
    Serial.print("M");
    lcd.print("M");
    palabra += "M"; 
  }
  else if (traduct=="-*")  {
    Serial.print("N");
    lcd.print("N");
    palabra += "N"; 
  } 
  else if (traduct=="---")  {
    Serial.print("O");
    lcd.print("O");
    palabra += "O"; 
  }
  else if (traduct=="*--*")  {
    Serial.print("P");
    lcd.print("P");
  }
  else if (traduct=="--*-")  {
    Serial.print("Q");
    lcd.print("Q");
  }  
  else if (traduct=="*-*")  {
    Serial.print("R");
    lcd.print("R");
  }   
  else if (traduct=="***")  {
    Serial.print("S");
    lcd.print("S");
    palabra += "S"; 
  }   
  else if (traduct=="-")  {
    Serial.print("T");
    lcd.print("T");
  }    
  else if (traduct=="**-")  {
    Serial.print("U");
    lcd.print("U");
  }   
  else if (traduct=="***-")  {
    Serial.print("V");
    lcd.print("V");
  }    
  else if (traduct=="*--")  {
    Serial.print("W");
    lcd.print("W");
  }    
  else if (traduct=="-**-")  {
    Serial.print("X");
    lcd.print("X");
  }   
  else if (traduct=="-*--")  {
    Serial.print("Y");
    lcd.print("Y");
  }    
  else if (traduct=="--**")  {
    Serial.print("Z");
    lcd.print("Z");
  }


  else if (traduct=="*----")  {
    Serial.print("1");
    lcd.print("1");
  }    
  else if (traduct=="**---")  {
    Serial.print("2");
    lcd.print("2");
  }    
  else if (traduct=="***--")  {
    Serial.print("3");
    lcd.print("3");
  }    
  else if (traduct=="****-")  {
    Serial.print("4");
    lcd.print("4");
  } 
  else if (traduct=="*****")  {
    Serial.print("5");
    lcd.print("5");
  }   
  else if (traduct=="-****")  {
    Serial.print("6");
    lcd.print("6");
  }    
  else if (traduct=="--***")  {
    Serial.print("7");
    lcd.print("7");
  }    
  else if (traduct=="---**")  {
    Serial.print("8");
    lcd.print("8");
  }   
  else if (traduct=="----*")  {
    Serial.print("9");
    lcd.print("9");
  }  
  else if (traduct=="-----")  {
    Serial.print("0");
    lcd.print("0");
  }


  else if (traduct=="*-*-*-")  {
    Serial.print(".");
    lcd.print(".");
  } 
  else if (traduct=="-**-*")  {
    Serial.print("/");
    lcd.print("/");
  } 
  else if (traduct=="-****-")  {
    Serial.print("-");
    lcd.print("-");
  } 
  else if (traduct=="*-*-*")  {
    Serial.print("+");
    lcd.print("+");
  }
  else if (traduct=="-***-")  {
    Serial.print("=");
    lcd.print("=");
  }  
  else if (traduct=="**---**")  {
    Serial.print("?");
    lcd.print("?");
  } 

  Serial.print(" ");
  traduct=""; 
}