#include<SoftwareSerial.h>
SoftwareSerial sim800l(10, 11);
#define RESET_PIN 12
unsigned long previousMillis = 0;
#include <Keypad.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 16 column and 2 rows

boolean getResponse(String expected_answer, unsigned int timeout) {
  boolean flag = false;
  String response = "";
  unsigned long previous;
  for (previous = millis(); (millis() - previous) < timeout;) {
    while (sim800l.available()) {
      response = sim800l.readString();
      if (response.indexOf(expected_answer) > 0) {
        flag = true;
        goto OUTSIDE;
      }
    }
  }
OUTSIDE:
  if (response != "") {
    //    Serial.println(response);
  }
  return flag;
}

boolean tryATcommand(String cmd, String expected_answer, int timeout, int total_tries) {
TryAgain:
  for (int i = 1; i <= total_tries; i++) {
    sim800l.println(cmd);
    //    Serial.print("Trying this command: ");
    //    Serial.println(cmd);
    if (getResponse(expected_answer, timeout) == true) {
      //      Serial.print("success with ");
      //      Serial.println(cmd);
      break;
    }
    else {
      //      Serial.println("Serial.readString(): " + sim800l.readString());
      //      Serial.print(".");
    }
    if (i == total_tries) {
      //      Serial.println("Restarting...");
      digitalWrite(RESET_PIN, LOW);
      delay(100);
      digitalWrite(RESET_PIN, HIGH);
      goto TryAgain;
    }
  }
}

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
#define Password_Lenght 8 // Give enough room for six chars + NULL char
#define sim_number_length 12
char Data[Password_Lenght]; // 6 is the number of chars it can hold + the null char = 7
char Data2[sim_number_length];
char Data3[Password_Lenght];
char password[Password_Lenght] = "";
char password3[Password_Lenght] = "";
char sim_number[sim_number_length] = "";
byte data_count = 0, master_count = 0;
char customKey;

bool activated_text_sent = 0;
bool send_door_twisted_sent = 0;
bool send_camera_movement_sent = 0;

int touch_sensor_1 = A3;

int motion_sensor = A2;
int siren_pin = A0;
int camera_pin = A1;
int burglar = 0;
int door_touch = 0;
void setup() {
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  sim800l.begin(9600);
  Serial.begin(9600);
  pinMode(touch_sensor_1, INPUT_PULLUP);
  pinMode(motion_sensor, INPUT);
  pinMode(siren_pin, OUTPUT);
  digitalWrite(siren_pin, HIGH);
  pinMode(camera_pin, OUTPUT);
  digitalWrite(camera_pin, HIGH);
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight

  my_lcd(7, 1, "SIM-IS", 3000);
  tryATcommand("AT", "OK", 2000, 20);
  tryATcommand("AT+CFUN=?", "OK", 2000, 20);
  tryATcommand("AT+CPIN?", "OK", 2000, 20);
  tryATcommand("AT+CMGF=1", "OK", 2000, 20);
  tryATcommand("AT+CMGF=1", "OK", 2000, 20);
  lcd.clear();
}

void loop() {
  lcd.clear();
  while (strlen(password) == 0) {
    set_password();
  }
  lcd.clear();
  while (strlen(sim_number) == 0) {
    set_sim();
  }
  lcd.clear();
  my_lcd(1, 1, "SIM-IS is Active.", 500);
  send_activated_text();
  check_deactivation_mode_button();
  check_door_and_window_sensor();
  check_motion_sensor();
  check_inbox();
}
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void check_door_and_window_sensor() {
  int  door_touch = digitalRead(touch_sensor_1);
  if (door_touch == 1) {
    //    Serial.println("door and/or window was touched!!!");
    digitalWrite(siren_pin, LOW);
    send_door_movement();
  }
}
void check_motion_sensor() {
  burglar = digitalRead(motion_sensor);
  if (burglar == 1) {
    //    Serial.println("The has been motion detected inside the house!!! Activating camera for 5 seconds");
    send_camera_movement();
    digitalWrite(camera_pin, LOW);
    delay(5000);
    digitalWrite(camera_pin, HIGH);

  }
}

void check_inbox() {
  sim800l.println("AT+CNMI=1,2,0,0,0"); // AT Command to receive a live SMS
  delay(200);
  for (int i = 0; i < 10; i++) {
    if (sim800l.available() > 0) {
      String received_message = sim800l.readString();
      //      Serial.println("received_message:" + received_message);
      if ((received_message.indexOf("+CMT:") > 0) && (received_message.indexOf("+63" + String(&sim_number[1])) > 0)) {
        if (received_message.indexOf(String(password)) > 0) {
          //Serial.println("Deactivation command received.");
          send_deactivated_text();
          lcd.clear();
          my_lcd(0, 0, "Deactivated.", 2000);
          clear_all();
          lcd.clear();
          delay(1000);
          resetFunc();
        }
        else {
          sim800l.print("AT+CMGS=\"" + String(sim_number) + "\"\r");
          delay(500);
          sim800l.print("Wrong deactivation code.\nTry again.");
          delay(500);
          sim800l.print((char)26);// (required according to the datasheet)
          delay(500);
          sim800l.println();
          delay(500);
        }

      }
      else {
        //        Serial.println("Received either a wrong deactivation code and/or from an unrecognized number");
      }
    }
  }//for
}

void send_deactivated_text() {
  sim800l.print("AT+CMGS=\"" + String(sim_number) + "\"\r");
  delay(500);
  sim800l.print("SIM-IS is successfully deactivated.");
  delay(500);
  sim800l.print((char)26);// (required according to the datasheet)
  delay(500);
  sim800l.println();
  delay(1000);
}

void send_activated_text() {
  if (!activated_text_sent) {
    sim800l.print("AT+CMGS=\"" + String(sim_number) + "\"\r");
    delay(500);
    sim800l.print("SIM-IS is activated.\nDeactivation Code: " + String(password));
    delay(500);
    sim800l.print((char)26);// (required according to the datasheet)
    delay(500);
    sim800l.println();
    delay(500);
    activated_text_sent = 1;
  }
}

void send_door_movement() {
  if (!send_door_twisted_sent) {
    sim800l.print("AT+CMGS=\"" + String(sim_number) + "\"\r");
    delay(500);
    sim800l.print("SIM-IS: Door knob movement sensor triggered!\nSiren activated.\nSend deactivation code to deactivate SIM-IS.");
    delay(500);
    sim800l.print((char)26);// (required according to the datasheet)
    delay(500);
    sim800l.println();
    delay(500);
    send_door_twisted_sent = 1;
  }
}
void send_camera_movement() {
  if (!send_camera_movement_sent) {
    sim800l.print("AT+CMGS=\"" + String(sim_number) + "\"\r");
    delay(500);
    sim800l.print("SIM-IS: Motion sensor triggered!\nCaptured image.\nSend deactivation code to deactivate SIM-IS.");
    delay(500);
    sim800l.print((char)26);// (required according to the datasheet)
    delay(500);
    sim800l.println();
    delay(500);
    send_camera_movement_sent = 1;
  }
}
void my_lcd(int my_row, int my_column, String message, int my_duration) {
  lcd.setCursor(my_row, my_column);
  lcd.print(message);
  delay(my_duration);
}

void set_password()
{
  lcd.setCursor(0, 0);
  lcd.print("Set a passcode:");

  customKey = customKeypad.getKey();
  if (customKey) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
  {
    lcd.setCursor(0, 1);
    lcd.print("*******");     //To hide your PASSWORD, make sure its the same lenght as your password
    Data[data_count] = customKey; // store char into data array
    lcd.setCursor(data_count, 1); // move cursor to show each new char
    lcd.print(Data[data_count]); // print char at said cursor
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
    if (data_count == (Password_Lenght - 1)) {
      for (int i = 0; i <= 7; i++) {
        password[i] = Data[i];
      }
      // Serial.println(Data);
      //      Serial.print("password set was: ");
      //      Serial.println(password);
      clear_data();
    }
  }
}

void clear_data() {
  data_count = 0;
  for (int i = 0; i <= 8; i++) {
    Data[i] = 0;
  }
}
void clear_data3() {
  data_count = 0;
  for (int i = 0; i <= 8; i++) {
    Data3[i] = 0;
    password3[i] = 0;
  }
}
void clear_all() {
  data_count = 0;
  for (int i = 0; i <= 8; i++) {
    Data[i] = 0;
    Data3[i] = 0;
    password[i] = 0;
    password3[i] = 0;
  }
  for (int i = 0; i <= 12; i++) {
    Data2[i] = 0;
    sim_number[i] = 0;
  }
}

void set_sim()
{
  lcd.setCursor(0, 0);
  lcd.print("Set SIM Number(11):");
  customKey = customKeypad.getKey();
  if (customKey) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
  {
    lcd.setCursor(0, 1);
    //lcd.print("***********");     //To hide your PASSWORD, make sure its the same lenght as your password
    Data2[data_count] = customKey; // store char into data array
    lcd.setCursor(data_count, 1); // move cursor to show each new char
    lcd.print(Data2[data_count]); // print char at said cursor
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
    if (data_count == (sim_number_length - 1)) {
      for (int i = 0; i <= 11; i++) {
        sim_number[i] = Data2[i];
      }
      //Serial.println(Data2);
      //      Serial.print("sim num set was: ");
      //      Serial.println(sim_number);
      clear_data();
    }
  }
}

void check_deactivation_mode_button() {
  customKey = customKeypad.getKey();
  if (customKey == 'D') {
    //    Serial.println("D was pressed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter passcode:");
    delay(3000);
    while (data_count < (Password_Lenght - 1)) {

      customKey = customKeypad.getKey();
      if (customKey) { // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
        lcd.setCursor(0, 1);
        lcd.print("*******");     //To hide your PASSWORD, make sure its the same lenght as your password
        Data3[data_count] = customKey; // store char into data array
        lcd.setCursor(data_count, 1); // move cursor to show each new char
        lcd.print(Data3[data_count]); // print char at said cursor
        data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
      }
    }

    for (int i = 0; i <= 6; i++) {
      password3[i] = Data3[i];
    }
    //    Serial.print("Password3: ");
    //    Serial.println(password3);
    //    Serial.println(strlen(password3));
    //    Serial.print("Password: ");
    //    Serial.println(password);
    //    Serial.println(strlen(password));

    if (strcmp(password, password3) == 0) {
      lcd.clear();
      //      Serial.println("correct. deactivated!");
      //      Serial.print("Password3: ");
      //      Serial.println(password3);
      //      Serial.print("Password: ");
      //      Serial.println(password);
      my_lcd(0, 0, "Deactivated.", 2000);
      clear_all();
      lcd.clear();
      send_deactivated_text();
      delay(1000);
      resetFunc();
    }
    else {
      lcd.clear();
      //      Serial.println("Wrong!!");
      //      Serial.print("Password3: ");
      //      Serial.println(password3);
      //      Serial.print("Password: ");
      //      Serial.println(password);
      my_lcd(0, 0, "Wrong. Try again.", 2000);
      delay(3000);
      //      Serial.print("current password: ");
      //      Serial.println(password);
      //clear_data3();
      data_count = 0;
      for (int i = 0; i <= 6; i++) {
        password3[i] = 0;
      }
      //      Serial.print("current passwordB: ");
      //      Serial.println(password);
    }
  }
}
