// PIN DEFINITIONS ----------------------------------------------

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define DISPLAY_1 22
#define DISPLAY_2 23

#define ROTARY_FREQ_DECIMAL_CLK 24
#define ROTARY_FREQ_DECIMAL_DT 25
#define ROTARY_FREQ_FRACTION_CLK 26
#define ROTARY_FREQ_FRACTION_DT 27
#define ROTARY_AP_CLK 28
#define ROTARY_AP_DT 29

#define BUTTON_ROTARY_ADF_SELECT 30    // This button selects ADF 1 as variable to change.
#define BUTTON_ROTARY_XPDR_SELECT 31    // This button selects XPDR as variable to change.
#define BUTTON_ROTARY_AP_SELECT 32    // This button selects which AP variable is selected for change.
#define BUTTON_SBY_ACT_SWITCH 33      // This button switches the primary and standby value of the selected COMM values
#define BUTTON_COMM_SELECT_COM_1 35
#define BUTTON_COMM_SELECT_COM_2 36
#define BUTTON_COMM_SELECT_NAV_1 37
#define BUTTON_COMM_SELECT_NAV_2 38
#define SWITCH_GEAR 39

#define LED_GEAR_NOSE_MOTION 48
#define LED_GEAR_LEFT_MOTION 49
#define LED_GEAR_RIGHT_MOTION 50
#define LED_GEAR_NOSE_DOWN 51
#define LED_GEAR_LEFT_DOWN 52
#define LED_GEAR_RIGHT_DOWN 53

#define MIN_LED_PIN 47
#define MAX_PIN 53

void pins_init() {
  Serial.println("Setting up pins...");
  pinMode(ROTARY_FREQ_DECIMAL_CLK, INPUT);
  pinMode(ROTARY_FREQ_DECIMAL_DT, INPUT);
  pinMode(ROTARY_FREQ_FRACTION_CLK, INPUT);
  pinMode(ROTARY_FREQ_FRACTION_DT, INPUT);
  pinMode(ROTARY_AP_CLK, INPUT);
  pinMode(ROTARY_AP_DT, INPUT);
  pinMode(BUTTON_ROTARY_ADF_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_ROTARY_XPDR_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_ROTARY_AP_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_SBY_ACT_SWITCH, INPUT_PULLUP);
  pinMode(BUTTON_COMM_SELECT_COM_1, INPUT_PULLUP);
  pinMode(BUTTON_COMM_SELECT_COM_2, INPUT_PULLUP);
  pinMode(BUTTON_COMM_SELECT_NAV_1, INPUT_PULLUP);
  pinMode(BUTTON_COMM_SELECT_NAV_2, INPUT_PULLUP);
  
  pinMode(SWITCH_GEAR, INPUT_PULLUP);
  
  pinMode(DISPLAY_1, OUTPUT);
  pinMode(DISPLAY_2, OUTPUT);

  pinMode(LED_GEAR_NOSE_MOTION, OUTPUT);
  pinMode(LED_GEAR_LEFT_MOTION, OUTPUT);
  pinMode(LED_GEAR_RIGHT_MOTION, OUTPUT);
  pinMode(LED_GEAR_NOSE_DOWN, OUTPUT);
  pinMode(LED_GEAR_LEFT_DOWN, OUTPUT);
  pinMode(LED_GEAR_RIGHT_DOWN, OUTPUT);

  // switching on all output LEDs for init testing...
  for (int i = MIN_LED_PIN; i <= MAX_PIN; i++) {
    digitalWrite(i, LOW);
  }
  
  delay(1000); // ...keeping them on a second...

  // ... and shutting off all output LEDs
  for (byte i = MIN_LED_PIN; i <= MAX_PIN; i++) {
    digitalWrite(i, HIGH);
  }
  Serial.println("... done");
}

