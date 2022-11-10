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
byte data_system1_count = 0;
byte data_system2_count = 0;
byte data_master_count = 0;
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
unsigned long interval = 500;   //led interval
unsigned long currentTime;

//system status
int System = 0;

//Functions for LCD to display
void lcd_start_setup()
{
  lcd.setCursor(1,0);
  lcd.print("Press # To ARM");
  lcd.setCursor(1,1);
  lcd.print("Press C To CPW");
}

void lcd_enter_password()
{
  lcd.setCursor(1,0);
  lcd.print("Enter Password");
}

void lcd_new_password()
{
  lcd.setCursor(1,0);
  lcd.print("New Password:");
}

void lcd_current_password()
{
  lcd.setCursor(0,0);
  lcd.print("Current Password:");
}

//LCD Display
unsigned long lcd_show_correct = 0;
unsigned long lcd_show_incorrect = 0;
unsigned long lcd_show_succeed = 0;
int lcd_state;

//Functions to clear entries in data
void clearData1()
{
  // Go through array and clear data
  while (data_system1_count != 0)
  {
    data[data_system1_count--] = 0;
  }
  return;
}

void clearData2()
{
  // Go through array and clear data
  while (data_system2_count != 0)
  {
    data[data_system2_count--] = 0;
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
  lcd_start_setup();
}

void loop()
{
  currentTime = millis();

  //Servos receive data from pycharm
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
    lcd_enter_password();
    }

    //Press C to change password
    else if(custom_key == 'C' && System == 0)
    {
    lcd_state = 0;
    System = 2;
    lcd.clear();
    lcd_current_password();
    }

    //When system is ARMED
    //Store inputs to variable data
    else if(System == 1)
    {
    data[data_system1_count] = custom_key;
    lcd.setCursor(data_system1_count,1);
    lcd.print("*");
    data_system1_count++;
    }

    //Entering changing password state when system is UNARM
    //Store inputs to variable data
    else if(System == 2)
    {
      //Go back to initial if press C again
      if(custom_key == 'C')
      {
        System = 0;
        lcd.clear();
        lcd_start_setup();
      }
      else
      {
      data[data_system2_count] = custom_key;
      lcd.setCursor(data_system2_count,1);
      lcd.print("*");
      data_system2_count++;
      }
    }

    //Changing master password
    else if(System == 3)
    {
      master[data_master_count] = custom_key;
      lcd.setCursor(data_master_count,1);
      lcd.print("*");
      data_master_count++;
    }
  }

  //checking data with current password before changing to new password
  if (data_system2_count == Password_Length - 1)
  {
    lcd.clear();
    //if data match current password then the system is ready to change master password
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
    //If data exceed 4 characters then clear data
    clearData2();
  }

  //if master password has 4 character then reset everything to initial state
  if (data_master_count == Password_Length - 1)
  {
    System = 0;
    lcd.clear();
    lcd.print("    Succeed");
    lcd_show_succeed = currentTime;
    lcd_state = 5;
    data_master_count = 0;
  }

  //When system is ARMED, check data with master to DISARM
  if (data_system1_count == Password_Length - 1)
  {
    lcd.clear();
    //if data match master password then the system is UNARM
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
    //If data exceed 4 characters then clear data
    clearData1();
  }

  //Displaying result for 3s on lcd
  //Show correct after entering correct password and go to set up screen
  if(lcd_state == 1)
  {
    if(currentTime - lcd_show_correct >= 2000)
    {
      lcd_start_setup();
    }
  }

  //show incorrect after entering wrong password and keep asking for password
  if(lcd_state == 2)
  {
    if(currentTime - lcd_show_incorrect >= 2000)
    {
      lcd_enter_password();
    }
  }

  //Show correct after entering correct current password and ask for new password
  if(lcd_state == 3)
  {
    if(currentTime - lcd_show_correct >= 2000)
    {
      lcd_new_password();
    }
  }

  //Show incorrect after entering wrong current password and keep asking for current password
  if(lcd_state == 4)
  {
    if(currentTime - lcd_show_incorrect >= 2000)
    {
      lcd_current_password();
    }
  }

  //Show succeed after new password is entered and go to set up screen
  if(lcd_state == 5)
  {
    if(currentTime - lcd_show_succeed >= 2000)
    {
      lcd_state = 0;
      lcd.clear();
      lcd_start_setup();
    }
  }
  
  //Alarm
  if(System == 1)
  {
    if(vals_Dec_Rec[2]>0)
    {
      //Every 0.5s, changing the state of the LED
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
