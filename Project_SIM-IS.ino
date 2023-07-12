#include <Keypad.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 16 column and 2 rows

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
#define Password_Lenght 7 // Give enough room for six chars + NULL char
#define sim_number_length 12
char Data[Password_Lenght]; // 6 is the number of chars it can hold + the null char = 7
char Data2[sim_number_length];
char Data3[Password_Lenght];
//char Master[Password_Lenght] = "123456";     //Change PASSWORD here
char password[Password_Lenght] = "";
char password3[Password_Lenght] = "";
char sim_number[sim_number_length] = "";
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;


int touch_sensor_1 = A3;
int motion_sensor = A2;
int siren_pin = A0;
int camera_pin = A1;
int burglar = 0;
int door_touch = 0;
void setup() {
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

  check_deactivation_mode_button();
  check_door_and_window_sensor();
  check_motion_sensor();
  //  check_inbox();

}

void check_door_and_window_sensor() {
  int  door_touch = digitalRead(touch_sensor_1);
  if (door_touch == 1) {
    Serial.println("door and/or window was touched!!!");
    digitalWrite(siren_pin, LOW);
    //    send_text("Someone is trying to open the door. Text me the passcode to deactivate.")
  }
}
void check_motion_sensor() {
  burglar = digitalRead(motion_sensor);
  if (burglar == 1) {
    Serial.println("The has been motion detected inside the house!!! Activating camera for 5 seconds");
    //    send_text("someone is inside. Text me the passcode to deactivate.");
    digitalWrite(camera_pin, LOW);
    delay(5000);
    digitalWrite(camera_pin, HIGH);
    
  }
}

void check_inbox() {
  //  if received text containing the passcode, deactivate and execute below,
  /*
        my_lcd(0, 0, "Deactivated.", 2000);
        clear_all();
        lcd.clear();
        delay(2000);
        resetFunc();
  */
  // else, 
  /*
   * send_text("Wrong passcode. Try again.")
   */
}

void send_text() {
  // this is activated when door is touched.
  // this is activated when person is inside and motion is detected.
  // make this function accept parameter "String my_message"
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
    lcd.print("******");     //To hide your PASSWORD, make sure its the same lenght as your password
    Data[data_count] = customKey; // store char into data array
    lcd.setCursor(data_count, 1); // move cursor to show each new char
    lcd.print(Data[data_count]); // print char at said cursor
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
    if (data_count == (Password_Lenght - 1)) {
      for (int i = 0; i <= 6; i++) {
        password[i] = Data[i];
      }
      // Serial.println(Data);
      Serial.print("password set was: ");
      Serial.println(password);
      clear_data();
    }
  }
}

void clear_data() {
  data_count = 0;
  for (int i = 0; i <= 7; i++) {
    Data[i] = 0;
  }
}
void clear_data3() {
  data_count = 0;
  for (int i = 0; i <= 7; i++) {
    Data3[i] = 0;
    password3[i] = 0;
  }
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void clear_all() {
  data_count = 0;
  for (int i = 0; i <= 7; i++) {
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
      Serial.print("sim num set was: ");
      Serial.println(sim_number);
      clear_data();
    }
  }
}

void check_deactivation_mode_button() {
  customKey = customKeypad.getKey();
  if (customKey == 'D') {
    Serial.println("D was pressed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter passcode:");
    delay(3000);
    while (data_count < (Password_Lenght - 1)) {

      customKey = customKeypad.getKey();
      if (customKey) { // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
        lcd.setCursor(0, 1);
        lcd.print("******");     //To hide your PASSWORD, make sure its the same lenght as your password
        Data3[data_count] = customKey; // store char into data array
        lcd.setCursor(data_count, 1); // move cursor to show each new char
        lcd.print(Data3[data_count]); // print char at said cursor
        data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
      }
    }

    for (int i = 0; i <= 6; i++) {
      password3[i] = Data3[i];
    }
    Serial.print("Password3: ");
    Serial.println(password3);
    Serial.println(strlen(password3));
    Serial.print("Password: ");
    Serial.println(password);
    Serial.println(strlen(password));

    if (strcmp(password, password3) == 0) {
      lcd.clear();
      Serial.println("correct. deactivated!");
      Serial.print("Password3: ");
      Serial.println(password3);
      Serial.print("Password: ");
      Serial.println(password);
      my_lcd(0, 0, "Deactivated.", 2000);

      clear_all();
      lcd.clear();
      delay(2000);
      resetFunc();
    }
    else {
      lcd.clear();
      Serial.println("Wrong!!");
      Serial.print("Password3: ");
      Serial.println(password3);
      Serial.print("Password: ");
      Serial.println(password);
      my_lcd(0, 0, "Wrong. Try again.", 2000);
      delay(3000);
      Serial.print("current passwordA: ");
      Serial.println(password);
      //clear_data3();
      data_count = 0;
      for (int i = 0; i <= 6; i++) {
        password3[i] = 0;
      }
      Serial.print("current passwordB: ");
      Serial.println(password);
    }

  }
}
