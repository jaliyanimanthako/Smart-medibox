#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>

// Constants for OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3c

// Pin definitions
#define BUZZER 19
#define LED 32
#define LED_TEMP 33
#define LED_HUM 25
#define PB 27
#define PB_OK 14
#define PB_UP 26
#define PB_DOWN 12
#define HUM 18

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     0
#define UTC_OFFSET_DST 0

// Objects initialization
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

// Variables for time, alarms, and modes
int days = 0;
int hours = 0;
int minutes = 0; 
int seconds = 0;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 1, 2};
int alarm_minutes[] = {1, 10, 30}; 
bool alarm_triggered[] = {false, false, false};

int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

int current_mode = 0;
int max_modes = 5;
String modes[] = {"1- Set Time Zone", "2- Set Alarm 1", "3- Set Alarm 2", "4- Set Alarm 3", "5- Disabling alarms"};

void setup() {
  // Initialization routine
  Serial.begin(115200);

  // Display initialization
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    print_line("Connecting to Wi-Fi", 2, 2, 2);
    delay(250);

  }


  display.clearDisplay();
  print_line("Connected to Wi-Fi", 2, 2, 2);


  configTime(5.5 * 3600, 0, "asia.pool.ntp.org"); // Set UTC offset for Asia/Colombo

  // Display welcome message
  display.display();
  delay(2000);
  display.clearDisplay();
  print_line("Welcome to medibox", 2, 2, 2);
  display.clearDisplay();

  // Pin mode setup
  pinMode(LED, OUTPUT);
  pinMode(LED_TEMP, OUTPUT);
  pinMode(LED_HUM, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(PB, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_DOWN, INPUT);

  // DHT sensor setup
  dhtSensor.setup(HUM, DHTesp::DHT22);
}

void loop() {
  // Main loop
  update_time_with_check_alarms();

  // Check if OK button is pressed
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }

  // Check temperature
  check_temp();
  delay(750);
  
}

void print_line(String text, int column, int row, int text_size) {
  // Function to print a line of text on the display
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();

}

void print_time_now() {
  // Function to print current time on the display
  display.clearDisplay(); // Clear the display once at the beginning
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(days);
  display.print(":");
  display.print(hours);
  display.print(":");
  display.print(minutes);
  display.print(":");
  display.println(seconds);
  display.display(); // Display all the updates at once
}

void update_time() {
  // Function to update the current time
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  hours = atoi(timeHour);

  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  minutes = atoi(timeMinute);

  char timeSecond[3];
  strftime(timeSecond, 3, "%S", &timeinfo);
  seconds = atoi(timeSecond);

  char timeDay[3];
  strftime(timeDay, 3, "%d", &timeinfo);
  days = atoi(timeDay);
}


void ring_alarm() {
  // Function to ring the a+larm
  display.clearDisplay();
  print_line("Medicine time", 0, 0, 2);
  digitalWrite(LED, HIGH);

  bool break_happened = false;

  while (break_happened == false && digitalRead(PB) == HIGH) {
    for (int i = 0; i < n_notes ; i++) {
      if (digitalRead(PB) == LOW) {
        delay(200);
        break_happened = true;
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }

  digitalWrite(LED, LOW);
  display.clearDisplay();
}

void update_time_with_check_alarms() {
  // Function to update time and check for alarms
  update_time();
  print_time_now();

  if (alarm_enabled == true) {
    for (int i = 0; i < n_alarms; i++ ) {
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes) {
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}


int wait_for_button_press() {
  // Function to wait for a button press and return the pressed button
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB) == LOW) {
      delay(200);
      return PB;
    }
    update_time();
  }
}

void go_to_menu() {
  // Function to navigate through the menu
  while (digitalRead(PB) == HIGH) {
    display.clearDisplay();
    print_line(modes[current_mode], 0, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP) {
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      current_mode -= 1;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      Serial.println(current_mode);
      run_mode(current_mode);
    }
    else if (pressed == PB) {
      delay(200);
      break;
    }
  }
}

void set_alarm(int alarm_number) {
  // Function to set alarm
  int temp_hour = alarm_hours[alarm_number];

  while (true) {
    display.clearDisplay();

    print_line("Enter Hour" + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    }
    else if (pressed == PB_OK) {
      alarm_hours[alarm_number - 1] = temp_hour;
      break;
    }

    else if (pressed == PB) {
      delay(200);
      break;
    }
  }

  int temp_minutes = alarm_minutes[alarm_number]; // Corrected variable name


  while (true) {
    display.clearDisplay();
    print_line("Enter minutes" + String(temp_minutes), 0, 0, 2); // Corrected label
    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_minutes += 1;
      temp_minutes = temp_minutes % 60;
    }
    else if (pressed == PB_DOWN  )  {
      delay(200);
      temp_minutes -= 1;
      if (temp_minutes < 0) {
        temp_minutes = 59;
      }
    }
    else if (pressed == PB_OK) {
      alarm_minutes[alarm_number - 1] = temp_minutes;
      break;
    }

    else if (pressed == PB) {
      delay(200);
      break;
    }
  }
}

void run_mode(int mode) {
  // Function to run the selected mode
  if (mode == 0) {
    set_time_zone();
  }
  else if (mode == 1) {
    set_alarm(1);
  }
  else if (mode == 2) {
    set_alarm(2);
  }
  else if (mode == 3) {
    set_alarm(3);
  }
  else if (mode == 4) {
    disable_alarms();
  }
}


void disable_alarms() {
  alarm_enabled = false;
  display.clearDisplay();
  print_line("Alarms Disabled", 0, 0, 2);
}

void check_temp() {
  // Function to check temperature and humidity
  
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  
  if (data.temperature > 32) {
    print_line("Temp High",0,40,1); // Display all the updates at once
    tone(BUZZER, 494);
    digitalWrite(LED_TEMP, HIGH);
    delay(500);
    digitalWrite(LED_TEMP, LOW);
    noTone(BUZZER);
    delay(2);
  }
  else if (data.temperature < 26) {
    print_line("Temp Low",0,40,1);
    tone(BUZZER, 494);
    digitalWrite(LED_TEMP, HIGH);
    delay(500);
    digitalWrite(LED_TEMP, LOW);
    noTone(BUZZER);
    delay(2);
  }
  if (data.humidity > 80) {
     // Clear the display once at the beginning
    print_line("Humidity High",0,50,1);
    tone(BUZZER, 494);
    digitalWrite(LED_HUM, HIGH);
    delay(500);
    digitalWrite(LED_HUM, LOW);
    noTone(BUZZER);
    delay(2);
  }
  else if (data.humidity < 60) {
    print_line("Humidity Low",0,50,1);
    tone(BUZZER, 494);
    digitalWrite(LED_HUM, HIGH);
    delay(500);
    digitalWrite(LED_HUM, LOW);
    noTone(BUZZER);
    delay(2);
  }
}

void set_time_zone() {
    // Function to set the time zone based on UTC offset
    int utc_offset = 0; // Default UTC offset

    
    while (true) {
        display.clearDisplay();
        print_line("Enter UTC offset:" + String(utc_offset), 0, 0, 2);

        int pressed = wait_for_button_press();

        if (pressed == PB_UP) {
            delay(200);
            utc_offset += 1;
        } else if (pressed == PB_DOWN) {
            delay(200);
            utc_offset -= 1;
        } else if (pressed == PB_OK) {
            delay(200);
            break;
        } else if (pressed == PB) {
            delay(200);
            return; // Return to the menu
        }

        
        if (utc_offset > 12) {
            utc_offset = 12;
        } else if (utc_offset < -12) {
            utc_offset = -12;
        }
    }

    // Apply the new UTC offset
    configTime(utc_offset * 3600, 0, NTP_SERVER);
    display.clearDisplay();
    print_line("Time Zone Set", 0, 0, 2);
}