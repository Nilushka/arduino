const int led_pin = LED_BUILTIN;
const int button_pin = 3;
const int open_drive_pin = 6;
const int close_drive_pin = 8;
const int push_delay = 3000;
unsigned long timestamp;
byte state = 0;
int button = 0;

void setup() {
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(open_drive_pin, OUTPUT);
  pinMode(close_drive_pin, OUTPUT);
  Serial.println("STARTING");
  open_drive();
  Serial.println("ENTERING THE LOOP");
}

void loop() {
  button = digitalRead(button_pin);
  if (button == LOW) {
    if (state == LOW) {
      close_drive();
      digitalWrite(led_pin, HIGH);
      state = !state;
    }
    else
    {
      open_drive();
      digitalWrite(led_pin, LOW);
      state = !state;
    }
  }
}

void blink_and_delay() {
  Serial.println("Running delay with blinking");
  timestamp = millis();
  while (millis() - timestamp < push_delay) {
    digitalWrite(led_pin, HIGH);
    delay(150);
    digitalWrite(led_pin, LOW);
    delay(150);
  }
}

void open_drive() {
  Serial.println("opening the exhaust");
  digitalWrite(open_drive_pin, HIGH);
  blink_and_delay();
  digitalWrite(open_drive_pin, LOW);
}

void close_drive() {
  Serial.println("closing the exhaust");
  digitalWrite(close_drive_pin, HIGH);
  digitalWrite(open_drive_pin, HIGH);
  blink_and_delay();
  digitalWrite(close_drive_pin, LOW);
  digitalWrite(open_drive_pin, LOW);
}
