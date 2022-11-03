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
byte data_count_cur_pw = 0;
byte data_count_test = 0;
char custom_key;

//Keypad setup
const byte rows = 4; 
const byte cols = 4; 
char hexaKeys[rows][cols] ={
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

//delay function
unsigned long startTime = 0;
unsigned long interval = 500;   //interval_led
unsigned long currentTime;

//system status
int System = 0;

//Function for LCD to display
void lcd_password_setup()
{
  lcd.setCursor(1,0);
  lcd.print("Enter Password");
}

void lcd_start_setup()
{
  lcd.setCursor(1,0);
  lcd.print("Press # To ARM");
  lcd.setCursor(1,1);
  lcd.print("Press C To CPW");
}

void lcd_changepw_setup()
{
  lcd.setCursor(1,0);
  lcd.print("New Password:");
}

void lcd_ask_current_pw_setup()
{
  lcd.setCursor(0,0);
  lcd.print("Current Password:");
}

//LCD Display
unsigned long lcd_show_correct = 0;
unsigned long lcd_show_incorrect = 0;
int lcd_state;
/*state 0 : Idle
 *state 1 : Correct
 *state 2 : Incorrect
 */

void clearData()
{
  // Go through array and clear data
  while (data_count != 0)
  {
    data[data_count--] = 0;
  }
  return;
}

void clearDataCur()
{
  // Go through array and clear data
  while (data_count_cur_pw != 0)
  {
    data[data_count_cur_pw--] = 0;
  }
  return;
}

void setup()
{
  serialDetection.begin();
  lcd.init();
  lcd.backlight();
  pinMode(LED,OUTPUT);
  servoHor.attach(10);
  servoVer.attach(11);
  lcd_start_setup(); //Calling LCD Setup
}

void loop()
{
  currentTime = millis();
  serialDetection.Get(vals_Dec_Rec);
  servoHor.write(vals_Dec_Rec[0]);
  servoVer.write(vals_Dec_Rec[1]);

  //Look for keypress
  custom_key = customKeypad.getKey(); 

  if(custom_key)
  {
    //Press # to ARM
    if(custom_key == '#' && System == 0)
    {
    lcd_state = 0;
    System = 1;
    lcd.clear();
    lcd_password_setup();
    }

    else if(custom_key == 'C' && System == 0)
    {
    lcd_state = 0;
    System = 2;
    lcd.clear();
    lcd_ask_current_pw_setup();
    }

    else if(System == 1)
    {
    data[data_count] = custom_key;
    lcd.setCursor(data_count,1);
    //lcd.print(data[data_count]);
    lcd.print("*");
    data_count++;
    }
    
    else if(System == 2)
    {
    data[data_count_cur_pw] = custom_key;
    lcd.setCursor(data_count_cur_pw,1);
    lcd.print("*");
    data_count_cur_pw++;
    }

    else if(System == 3)
    {
      master[data_count_test] = custom_key;
      lcd.setCursor(data_count_test,1);
      lcd.print("*");
      data_count_test++;
    }
  }

  //Checking password
  if (data_count_cur_pw == Password_Length - 1)
  {
    lcd.clear();
    if (!strcmp(data, master))
    {
      System = 3;
      lcd.print("    Correct");
      lcd_show_correct = currentTime;
      lcd_state = 3;
    }
    else
    {
      lcd.print("   Incorrect");
      lcd_show_incorrect = currentTime;
      lcd_state = 4;
    }
    clearDataCur();
  }
  
  if (data_count_test == Password_Length - 1)
  {
    System = 0;
    lcd_state = 0;
    lcd.clear();
    lcd_start_setup();
    data_count_test = 0;
  }
  
  if (data_count == Password_Length - 1)
  {
    lcd.clear();
    if (!strcmp(data, master))
    {
      System = 0;
      lcd.print("    Correct");
      lcd_show_correct = currentTime;
      lcd_state = 1;
    }
    else
    {
      lcd.print("   Incorrect");
      lcd_show_incorrect = currentTime;
      lcd_state = 2;
    }
    clearData();
  }

  //Displaying result for 3 sec on lcd
  if(lcd_state == 1)
  {
    if(currentTime - lcd_show_correct >= 2000)
    {
      lcd_start_setup();
    }
  }

  if(lcd_state == 2)
  {
    if(currentTime - lcd_show_incorrect >= 2000)
    {
      lcd_password_setup();
    }
  }
  
  if(lcd_state == 3)
  {
    if(currentTime - lcd_show_correct >= 2000)
    {
      lcd_changepw_setup();
    }
  }
  
  if(lcd_state == 4)
  {
    if(currentTime - lcd_show_incorrect >= 2000)
    {
      lcd_ask_current_pw_setup();
    }
  }
  
  //Alarm
  if(System == 1)
  {
    if(vals_Dec_Rec[2]>0)
    {
      if(currentTime - startTime >= interval)
      {
        startTime = currentTime;
        if(LED_state == 0)
        {
          LED_state = 1;
        } else {
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
  else
  {
    digitalWrite(LED,LOW);
  }
}
