#include <cvzone.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

//Password Setup
#define Password_Length 5

//LED set up
#define LED 13
int LED_state = 0;

//Python setup
SerialData serialDetection(3,3);
int vals_Dec_Rec[3];

//LCD set up
LiquidCrystal_I2C lcd(0x27, 16 ,2);

//Password setup
char data[Password_Length];
char master[Password_Length] = "1234";
byte data_count = 0;
char custom_key;

//Keypad setup
const byte rows = 4; 
const byte cols = 4; 
char hexaKeys[rows][cols] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte row_pins[rows] = {9,8,7,6};
byte col_pins[cols] = {5,4,3,2};
//keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), row_pins, col_pins, rows, cols);

//Servo setup
Servo servoHor;
Servo servoVer;

//Testing
unsigned long startTime = 0;
unsigned long interval = 500;
unsigned long currentTime;

//Function for LCD Setup and Reset
void lcd_setup() {
  lcd.setCursor(1,0);
  lcd.print("Enter Password");
}

void setup() {
  serialDetection.begin();
  lcd.init();
  lcd.backlight();
  pinMode(LED,OUTPUT);
  servoHor.attach(10);
  servoVer.attach(11);
  lcd_setup(); //Calling LCD Setup
}

void loop() {
  currentTime = millis();
  serialDetection.Get(vals_Dec_Rec);
  servoHor.write(vals_Dec_Rec[0]);
  servoVer.write(vals_Dec_Rec[1]);

  //Look for keypress
  custom_key = customKeypad.getKey();
  if(custom_key)
  {
    data[data_count] = custom_key;
    lcd.setCursor(data_count,1);
    lcd.print(data[data_count]);
    data_count++;
      if(!strcmp(data, master)) {
      lcd.clear();
      lcd.print("Correct");
      digitalWrite(LED,LOW); //Sets our alarm to OFF
      delay(2000);
      lcd.clear();
      lcd_setup();
    }
  }
  if(vals_Dec_Rec[2]>0)
  {
    if(currentTime - startTime >= interval)
    {
      startTime = currentTime;
      if(LED_state == 0)
      {
        LED_state = 1;
      }
      else
      {
        LED_state = 0;
      }
      digitalWrite(LED,LED_state);
    }
  }
  else
  {
   digitalWrite(LED,LOW);
  }
  
}
