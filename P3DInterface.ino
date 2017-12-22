#include "pin_definitions.h"
#include "color_definitions.h"
#include "comm_select_definitions.h"

#include <Elegoo_GFX.h>
#include <Elegoo_TFTLCD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <gfxfont.h>

// MISC ---------------------------------------------------------
#define TEST_STRING "---"
#define AP_SELECT_RESET_SECONDS 10    // how long is the ap value selected for editing?

#define GEAR_UP 1
#define GEAR_DN 0

#define OLED_DRAW_INTERVAL 5000UL     // milliseconds
#define OLED_RESET 35
Adafruit_SSD1306 display(OLED_RESET);
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// GLOBAL VARIABLES ----------------------------------------------------------------------------------------------

// NORMAL PROGRAM VARIABLES --------------------------------------
byte CodeIn = 0;// The normal declearations go here
bool change = "true";
unsigned long OLED_draw_timer = millis();
int comm_select = NONE;   // which radio value is selected for changing
unsigned long comm_select_last_action;// we need to track when the last movement for comm_select was to reset selection
int ap_select = AP_NONE;     // which autopilot value is selected for changing

// debouncer variables for switches
int debounceDelay = 10;
unsigned long lastDebounceTime = 0;

// SWITCH POSITION VARIABLES --------------------------------------
int rotary_freq_decimal_clk_state = 0;
int rotary_freq_decimal_clk_last_state = 0;
int rotary_freq_decimal_dt_state = 0;
int rotary_freq_decimal_dt_last_state = 0;

int rotary_freq_fraction_clk_state = 0;
int rotary_freq_fraction_clk_last_state = 0;
int rotary_freq_fraction_dt_state = 0;
int rotary_freq_fraction_dt_last_state = 0;

int rotary_ap_clk_state = 0;
int rotary_ap_clk_last_state = 0;
int rotary_ap_dt_state = 0;
int rotary_ap_dt_last_state = 0;

int switch_gear = GEAR_DN;
unsigned long ap_last_action;// we need to track when the last movement for ap was to reset selection
int gear_select = GEAR_DN;

// TEST STRING VARIABLES -------------------------------------------
String Com1freq = TEST_STRING;
String Com2freq = TEST_STRING;
String Com1SBfreq = TEST_STRING;
String Com2SBfreq = TEST_STRING;

String Nav1freq = TEST_STRING;
String Nav2freq = TEST_STRING;
String Nav1SBfreq = TEST_STRING;
String Nav2SBfreq = TEST_STRING;
String ADF1freq = TEST_STRING;

String Transponder = TEST_STRING;

String HDGSet = TEST_STRING;
String CRSSet = TEST_STRING;
String AltSet = TEST_STRING;
String SpdSet = TEST_STRING;

String AltimeterSettingmBar = TEST_STRING;
String AltimeterSettinginHg = TEST_STRING;

String SAT = TEST_STRING;
String TAT = TEST_STRING;

String LocalTimeHour = TEST_STRING;
String LocalTimeMinute = TEST_STRING;
String ZuluTimeHour = TEST_STRING;
String ZuluTimeMinute = TEST_STRING;

String ElevatorTrim = TEST_STRING;
String RudderTrim = TEST_STRING;
String AileronTrim = TEST_STRING;

// SETUP ----------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pins_init();

  // ROTARIES' INIT -----------------------------------------
  rotary_freq_decimal_clk_last_state = digitalRead(ROTARY_FREQ_DECIMAL_CLK);
  rotary_freq_fraction_clk_last_state = digitalRead(ROTARY_FREQ_FRACTION_CLK);
  rotary_ap_clk_last_state = digitalRead(ROTARY_AP_CLK);

  // DISPLAYS' INIT -----------------------------------------
  oled_init();
  tft_init();
  draw_tft_pattern();
}

void loop() {
  {
    OTHER(); // Check for "Other" things to do. (Non extraction stuff)
  }
  if (Serial.available()) {  //Check if anything there
    CodeIn = getChar();      //Get a serial read if there is.
    if (CodeIn == '=') {
      EQUALS(); // The first identifier is "=" ,, goto void EQUALS
    }
    if (CodeIn == '<') {
      LESSTHAN(); // The first identifier is "<" ,, goto void LESSTHAN
    }
    if (CodeIn == '?') {
      QUESTION(); // The first identifier is "?" ,, goto void QUESTION
    }
    if (CodeIn == '/') {
      SLASH(); // The first identifier is "/"  ,, goto void SLASH (Annunciators)
    }
    if (CodeIn == '#') {
      HASH(); // The first identifier is "#"
    }
  }

  tft.setTextSize(2);
}

/*
    drawTrims takes coordinates and sizes for drawing the trim states of the aircraft.

    trim states are given in +000 percent.
    +100 is max up
    -100 is max down
*/
void drawTrims(int xPos, int yPos, int height, int width, String elevTrim, String RudderTrim, String AileronTrim) {

  // Position of the elevator line in respect to the drawn crosshair
  double elevPixelPos = (((double)((width - 2) / 2) / 100) * elevTrim.toDouble());
  display.drawLine(xPos + 5, (yPos + width / 2) - elevPixelPos, xPos + width - 5, (yPos + width / 2) - elevPixelPos, WHITE);

  // Position of the rudder line in respect to the drawn crosshair
  double rudderPixelPos = (((double)((width - 2) / 2) / 100) * -RudderTrim.toDouble());
  display.drawLine((xPos + width / 2) - rudderPixelPos, yPos + width + 2, (xPos + width / 2) - rudderPixelPos, yPos + height - 2,  WHITE);

  // Position of the aileron line in respect to the drawn crosshair
  double aileronPixelPos = (((double)((width - 2) / 2) / 100) * AileronTrim.toDouble());
  display.drawLine((xPos + width / 2) + aileronPixelPos, yPos + 6, (xPos + width / 2) + aileronPixelPos, yPos + width - 6, WHITE);

  display.drawLine(xPos, yPos, xPos + width , yPos, WHITE); // horizontal 1
  display.drawLine(xPos, yPos + height, xPos + width , yPos + height, WHITE); // horizontal 2
  display.drawLine(xPos, yPos + width, xPos + width , yPos + width, WHITE); // horizontal 3
  display.drawLine(xPos, yPos, xPos, yPos + height, WHITE); // vertical 1
  display.drawLine(xPos + width, yPos, xPos + width, yPos + height, WHITE); // vertical 1

  display.drawLine(xPos, yPos + (int)width / 2, xPos + 5, yPos + (int)width / 2, WHITE); // crossair horizontal 1
  display.drawLine(xPos + width - 5, yPos + (int)width / 2, xPos + width, yPos + (int)width / 2, WHITE); // crossair horizontal 2
  display.drawLine(xPos + (int)width / 2, yPos, xPos + (int)width / 2, yPos + 5, WHITE); // crossair vertical 1
  display.drawLine(xPos + (int)width / 2, yPos + width - 5, xPos + (int)width / 2, yPos + width, WHITE); // crossair vertical 2
}

char getChar()// Get a character from the serial buffer(Dont touch)
{
  while (Serial.available() <= 0); // wait for data (Dont touch)
  return (Serial.read()); // (Dont touch) Thanks Doug
}

// reads multiple chars from serial and returns them as a string.
String readMultipleChars(int numberOfChars) {
  String value = "";
  for (int i = 0; i < numberOfChars; i++) {
    value += (String)getChar();
  }
  return value;
}

void OTHER() {
  /* In here you would put code that uses other data that
    cant be put into an "extraction void" that references something else.
    Also in here you would put code to do something that was not
    relying on a current extraction.
    (Remember - The identifier voids only trigger when it receives that identifier)
  */

  scan_rotary_encoders();
  switch_gear_check();
  button_press_ap_select();
  button_press_comm_selects();
  reset_selects();

  // only draw the display at certain intervals and only if the rotaries are not needed
  if (millis() + OLED_DRAW_INTERVAL > OLED_draw_timer and !(ap_select == AP_NONE and comm_select == NONE)) {
    OLED_2();
    OLED_draw_timer = millis();
  }
}

void scan_rotary_encoders() {
  // ROTARY ENCODER FOR COMM DECIMAL PART
  rotary_freq_decimal_clk_state = digitalRead(ROTARY_FREQ_DECIMAL_CLK);
  if (rotary_freq_decimal_clk_state != rotary_freq_decimal_clk_last_state) {
    lastDebounceTime = millis();
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(ROTARY_FREQ_DECIMAL_DT) != rotary_freq_decimal_clk_state) {
      comm_rotary_increment(true);
    } else {
      comm_rotary_decrement(true);
    }
    comm_select_last_action = millis();
  }
  while (rotary_freq_decimal_clk_state != rotary_freq_decimal_clk_last_state or millis() - lastDebounceTime < debounceDelay) {
    rotary_freq_decimal_clk_state = digitalRead(ROTARY_FREQ_DECIMAL_CLK);
  }

  // ROTARY ENCODER FOR COMM FRACTION PART
  rotary_freq_fraction_clk_state = digitalRead(ROTARY_FREQ_FRACTION_CLK);
  if (rotary_freq_fraction_clk_state != rotary_freq_fraction_clk_last_state) {
    lastDebounceTime = millis();
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(ROTARY_FREQ_FRACTION_DT) != rotary_freq_fraction_clk_state) {
      comm_rotary_increment(false);
    } else {
      comm_rotary_decrement(false);
    }
    comm_select_last_action = millis();
  }
  while (rotary_freq_fraction_clk_state != rotary_freq_fraction_clk_last_state or millis() - lastDebounceTime < debounceDelay) {
    rotary_freq_fraction_clk_state = digitalRead(ROTARY_FREQ_FRACTION_CLK);
  }

  // ROTARY ENCODER FOR AP
  rotary_ap_clk_state = digitalRead(ROTARY_AP_CLK);
  if (rotary_ap_clk_state != rotary_ap_clk_last_state) {
    lastDebounceTime = millis();
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(ROTARY_AP_DT) != rotary_ap_clk_state) {
      ap_rotary_increment();
    } else {
      ap_rotary_decrement();
    }
    ap_last_action = millis();
  }
  while (rotary_ap_clk_state != rotary_ap_clk_last_state or millis() - lastDebounceTime < debounceDelay) {
    rotary_ap_clk_state = digitalRead(ROTARY_AP_CLK);
  }
}

void switch_gear_check() {
  if (digitalRead(SWITCH_GEAR) == LOW and switch_gear == GEAR_UP)
  {
    Serial.println("C02");
    switch_gear = GEAR_DN;
  }

  if (digitalRead(SWITCH_GEAR) == HIGH and switch_gear == GEAR_DN)
  {
    Serial.println("C01");
    switch_gear = GEAR_UP;
  }
}

// checks if the autopilot select button was pressed and changes the selection accordingly
void button_press_ap_select() {
  if (digitalRead(BUTTON_ROTARY_AP_SELECT) == LOW) {
    lastDebounceTime = millis();
    ap_select++;
    // AP_SPD is the last item in the selection list. Here the last item must be used.
    if (ap_select == AP_SPD + 1) {
      ap_select = AP_NONE;
    }
    Serial.println(ap_select);
    while (digitalRead(BUTTON_ROTARY_AP_SELECT) == LOW or millis() - lastDebounceTime < debounceDelay);
    ap_last_action = millis();
  }
}

// after a certain time we want the selection to disappear. This function deselects AP value.
void reset_selects() {
  if (millis() > (ap_last_action + (AP_SELECT_RESET_SECONDS * 1000UL))) {
    ap_select = AP_NONE;
    ap_last_action = millis();
  }

  if (millis() > (comm_select_last_action + (AP_SELECT_RESET_SECONDS * 1000UL))) {
    comm_select = NONE;
    comm_select_last_action = millis();
  }
}

void button_press_comm_selects() {
  if (digitalRead(BUTTON_COMM_SELECT_COM_1) == LOW) {
    Serial.println("com1");
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    comm_select = COM_1;
  }

  if (digitalRead(BUTTON_COMM_SELECT_COM_2) == LOW) {
    Serial.println("com2");
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    comm_select = COM_2;
  }

  if (digitalRead(BUTTON_COMM_SELECT_NAV_1) == LOW) {
    Serial.println("nav1");
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    comm_select = NAV_1;
  }

  if (digitalRead(BUTTON_COMM_SELECT_NAV_2) == LOW) {
    Serial.println("nav2");
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    comm_select = NAV_2;
  }

  if (digitalRead(BUTTON_ROTARY_ADF_SELECT) == LOW) {
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    comm_select = ADF_1;
  }

  if (digitalRead(BUTTON_ROTARY_XPDR_SELECT) == LOW) {
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    comm_select = XPDR;
  }

  if (digitalRead(BUTTON_SBY_ACT_SWITCH) == LOW) {
    Serial.println("x");
    lastDebounceTime = millis();
    comm_select_last_action = millis();
    switch (comm_select) {
      case COM_1:
        Serial.println("A06");
        break;
      case COM_2:
        Serial.println("A12");
        break;
      case NAV_1:
        Serial.println("A18");
        break;
      case NAV_2:
        Serial.println("A24");
        break;
    }
  }

  while (digitalRead(BUTTON_COMM_SELECT_COM_1) == LOW or digitalRead(BUTTON_COMM_SELECT_COM_2) == LOW or digitalRead(BUTTON_COMM_SELECT_NAV_1) == LOW or digitalRead(BUTTON_COMM_SELECT_NAV_2) == LOW or digitalRead(BUTTON_SBY_ACT_SWITCH) == LOW or millis() - lastDebounceTime < debounceDelay) {
    Serial.println("low");
  }
}

// increasing a comm rotary can mean a lot of things.
// we need to find out which setting is activated and increase exactly that.
void comm_rotary_increment(bool decimal) {
  String adf;
  String adf2;

  if (decimal) {
    switch (comm_select) {
      case COM_1:
        Serial.println("A02");
        break;
      case COM_2:
        Serial.println("A08");
        break;
      case NAV_1:
        Serial.println("A14");
        break;
      case NAV_2:
        Serial.println("A20");
        break;
      case ADF_1:
        adf = ADF1freq;

        adf.remove(5);
        adf.remove(4);
        adf.remove(0, 3);

        if (adf == "9") {
          Serial.println("A26");

          adf2 = ADF1freq;
          adf2.remove(5);
          adf2.remove(4);
          adf2.remove(3);
          adf2.remove(0, 2);
          if (adf2 == "9") {
            Serial.println("A25");
          }
        }
        Serial.println("A27");
        break;

      case XPDR:
        // TODO: Logik für die 2 ersten Digits des XPDR-Codes finden
        // from xpdr 1234 remove 1, 3 and 4 and check if remaining is 9
        String tt = Transponder;
        tt.remove(3);
        tt.remove(2);
        tt.remove(0, 1);
        Serial.println(tt);
        if (tt == "7") {
          Serial.println("A34");
        }
        Serial.println("A35");
        break;
    }
  } else {
    switch (comm_select) {
      case COM_1:
        Serial.println("A04");
        break;
      case COM_2:
        Serial.println("A10");
        break;
      case NAV_1:
        Serial.println("A16");
        break;
      case NAV_2:
        Serial.println("A22");
        break;
      case ADF_1:
        Serial.println("A28");
        break;
      case XPDR:
        // TODO: Logik für die 2 letzten Digits des XPDR-Codes finden
        // from xpdr 1234 remove 1, 2 and 3 and check if remaining is 9
        String tt = Transponder;
        tt.remove(0, 3);
        Serial.println(tt);
        if (tt == "7") {
          Serial.println("A36");
        }
        Serial.println("A37");
        break;
    }
  }
}

// increasing a comm rotary can mean a lot of things.
// we need to find out which setting is activated and increase exactly that.
void comm_rotary_decrement(bool decimal) {
  String adf;
  String adf2;

  if (decimal) {
    switch (comm_select) {
      case COM_1:
        Serial.println("A01");
        break;
      case COM_2:
        Serial.println("A07");
        break;
      case NAV_1:
        Serial.println("A13");
        break;
      case NAV_2:
        Serial.println("A19");
        break;
      case ADF_1:
        adf = ADF1freq;

        adf.remove(5);
        adf.remove(4);
        adf.remove(0, 3);

        if (adf == "0") {
          Serial.println("A30");

          adf2 = ADF1freq;
          adf2.remove(5);
          adf2.remove(4);
          adf2.remove(3);
          adf2.remove(0, 2);
          if (adf2 == "9") {
            Serial.println("A29");
          }
        }
        Serial.println("A31");
        break;
      case XPDR:
        // TODO: Logik für die 2 ersten Digits des XPDR-Codes finden
        // from xpdr 1234 remove 1, 3 and 4 and check if remaining is 0
        String tt = Transponder;
        tt.remove(3);
        tt.remove(2);
        tt.remove(0, 1);
        if (tt == "0") {
          Serial.println("A38");
        }
        Serial.println("A39");
        break;
    }
  } else {
    switch (comm_select) {
      case COM_1:
        Serial.println("A03");
        break;
      case COM_2:
        Serial.println("A09");
        break;
      case NAV_1:
        Serial.println("A15");
        break;
      case NAV_2:
        Serial.println("A21");
        break;
      case ADF_1:
        // TODO: codeswitch für alle Frequenzen finden
        Serial.println("A32");
        break;
      case XPDR:
        // TODO: Logik für die 2 letzten Digits des XPDR-Codes finden
        // from xpdr 1234 remove 1, 2 and 3 and check if remaining is 0
        String tt = Transponder;
        tt.remove(0, 3);
        Serial.println(Transponder);
        Serial.println(tt);
        if (tt == "0") {
          Serial.println("A40");
        }
        Serial.println("A41");
        break;
    }
  }
}

// increasing the ap rotary can mean a lot of things.
// we need to find out which setting is activated and increase exactly that.
void ap_rotary_increment() {
  switch (ap_select) {
    case AP_ALT:
      Serial.println("B11");
      break;
    case AP_HDG:
      Serial.println("A57");
      break;
    case AP_CRS:
      Serial.println("A56");
      break;
    case AP_SPD:
      Serial.println("B15");
      break;
  }
}

// increasing the ap rotary can mean a lot of things.
// we need to find out which setting is activated and increase exactly that.
void ap_rotary_decrement() {
  switch (ap_select) {
    case AP_ALT:
      Serial.println("B12");
      break;
    case AP_HDG:
      Serial.println("A58");
      break;
    case AP_CRS:
      Serial.println("A55");
      break;
    case AP_SPD:
      Serial.println("B16");
      break;
  }
}

void EQUALS() {     // The first identifier was "="
  CodeIn = getChar(); // Get another character
  switch (CodeIn) { // Now lets find what to do with it
    case 'A'://Found the second identifier
      Com1freq = readMultipleChars(7);;
      tft.setCursor(5, 5);
      tft.setTextColor(GREEN, BLACK);
      tft.println(Com1freq);
      break;
    case 'B'://Found the second identifier
      Com1SBfreq = readMultipleChars(7);;
      tft.setCursor(5, 37);
      tft.setTextColor(AMBER, BLACK);
      if (comm_select == COM_1) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.println(Com1SBfreq);
      break;
    case 'C'://Found the second identifier
      Com2freq = readMultipleChars(7);;
      tft.setCursor(5, 69);
      tft.setTextColor(GREEN, BLACK);
      tft.println(Com2freq);
      break;
    case 'D'://Found the second identifier
      Com2SBfreq = readMultipleChars(7);;
      tft.setCursor(5, 101);
      tft.setTextColor(AMBER, BLACK);
      if (comm_select == COM_2) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.println(Com2SBfreq);
      break;
    case 'E'://Found the second identifier
      Nav1freq = readMultipleChars(6);
      tft.setCursor(125, 5);
      tft.setTextColor(GREEN, BLACK);
      tft.println(Nav1freq);
      break;
    case 'F'://Found the second identifier
      Nav1SBfreq = readMultipleChars(6);
      tft.setCursor(125, 37);
      tft.setTextColor(AMBER, BLACK);
      if (comm_select == NAV_1) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.println(Nav1SBfreq);
      break;
    case 'G'://Found the second identifier
      Nav2freq = readMultipleChars(6);
      tft.setCursor(125, 69);
      tft.setTextColor(GREEN, BLACK);
      tft.println(Nav2freq);
      break;
    case 'H'://Found the second identifier
      Nav2SBfreq = readMultipleChars(6);
      tft.setCursor(125, 101);
      tft.setTextColor(AMBER, BLACK);
      if (comm_select == NAV_2) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.println(Nav2SBfreq);
      break;
    case 'I'://Found the second identifier
      ADF1freq = readMultipleChars(6);
      tft.setCursor(5, 133);
      tft.setTextColor(GREEN, BLACK);
      if (comm_select == ADF_1) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.println(ADF1freq);
      break;
    case 'J':
      Transponder = readMultipleChars(4);
      tft.setCursor(125, 133);
      tft.setTextColor(GREEN, BLACK);
      if (comm_select == XPDR) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.println(Transponder);
      break;
    case 'b':
      AltSet = readMultipleChars(5);
      tft.setCursor(5, 195);
      tft.setTextColor(GREEN, BLACK);
      if (ap_select == AP_ALT) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.print("ALT: " + AltSet + " ft");
      break;
    case 'd':
      HDGSet = readMultipleChars(3);
      tft.setCursor(5, 235);
      tft.setTextColor(GREEN, BLACK);
      if (ap_select == AP_HDG) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.print("HDG:   " + HDGSet);
      break;
    case 'e':
      CRSSet = readMultipleChars(3);
      tft.setCursor(5, 215);
      tft.setTextColor(GREEN, BLACK);
      if (ap_select == AP_CRS) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.print("CRS:   " + CRSSet);
      break;
    case 'f':
      SpdSet = readMultipleChars(3);
      tft.setCursor(5, 255);
      tft.setTextColor(GREEN, BLACK);
      if (ap_select == AP_SPD) {
        tft.setTextColor(CYAN, BLACK);
      }
      tft.print("SPD:   " + SpdSet + " kts");
      break;
    case 'W':
      //Do something
      break;
    case 'a':
      if (getChar() == '1') {
        tft.setCursor(15, 280);
        tft.setTextColor(GREEN, BLACK);
        tft.print("AP");
      }
      else {
        tft.setCursor(15, 280);
        tft.setTextColor(BLACK, BLACK);
        tft.print("AP");
      }
      break;
    case 'j':
      if (getChar() == '1') {
        tft.setCursor(95, 300);
        tft.setTextColor(GREEN, BLACK);
        tft.print("HDG");
      }
      else {
        tft.setCursor(95, 300);
        tft.setTextColor(BLACK, BLACK);
        tft.print("HDG");
      }
      break;
    case 'k':
      if (getChar() == '1') {
        tft.setCursor(175, 280);
        tft.setTextColor(GREEN, BLACK);
        tft.print("ALT");
      }
      else {
        tft.setCursor(175, 280);
        tft.setTextColor(BLACK, BLACK);
        tft.print("ALT");
      }
      break;
    case 'o':
      if (getChar() == '1') {
        tft.setCursor(95, 280);
        tft.setTextColor(GREEN, BLACK);
        tft.print("NAV");
      }
      else {
        tft.setCursor(95, 280);
        tft.setTextColor(BLACK, BLACK);
        tft.print("NAV");
      }
      break;
    case 's':
      if (getChar() == '1') {
        tft.setCursor(175, 300);
        tft.setTextColor(GREEN, BLACK);
        tft.print("NAV");
      }
      else {
        tft.setCursor(175, 300);
        tft.setTextColor(BLACK, BLACK);
        tft.print("NAV");
      }
      break;
      //etc etc etc
      // You only need the "Case" testing for the identifiers you expect to use.
  }

  change = true;
}

void LESSTHAN() {   // The first identifier was "<"
  //Do something (See void EQUALS)
  String value = "";
  CodeIn = getChar(); // Get another character
  switch (CodeIn) { // Now lets find what to do with it
    case 'A':
      value = readMultipleChars(3);
      if (value == "000") {
        digitalWrite(LED_GEAR_NOSE_MOTION, HIGH);
        digitalWrite(LED_GEAR_NOSE_DOWN, HIGH);
      } else if (value == "050") {
        digitalWrite(LED_GEAR_NOSE_MOTION, LOW);
        digitalWrite(LED_GEAR_NOSE_DOWN, HIGH);
      } else if (value == "100") {
        digitalWrite(LED_GEAR_NOSE_MOTION, HIGH);
        digitalWrite(LED_GEAR_NOSE_DOWN, LOW);
      }
      break;
    case 'B':
      value = readMultipleChars(3);
      if (value == "000") {
        digitalWrite(LED_GEAR_LEFT_MOTION, HIGH);
        digitalWrite(LED_GEAR_NOSE_DOWN, HIGH);
      } else if (value == "050") {
        digitalWrite(LED_GEAR_LEFT_MOTION, LOW);
        digitalWrite(LED_GEAR_LEFT_DOWN, HIGH);
      } else if (value == "100") {
        digitalWrite(LED_GEAR_LEFT_MOTION, HIGH);
        digitalWrite(LED_GEAR_LEFT_DOWN, LOW);
      }
      break;
    case 'C':
      value = readMultipleChars(3);
      if (value == "000") {
        digitalWrite(LED_GEAR_RIGHT_MOTION, HIGH);
        digitalWrite(LED_GEAR_RIGHT_DOWN, HIGH);
      } else if (value == "050") {
        digitalWrite(LED_GEAR_RIGHT_MOTION, LOW);
        digitalWrite(LED_GEAR_RIGHT_DOWN, HIGH);
      } else if (value == "100") {
        digitalWrite(LED_GEAR_RIGHT_MOTION, HIGH);
        digitalWrite(LED_GEAR_RIGHT_DOWN, LOW);
      }
      break;

    case 'H':
      ElevatorTrim = readMultipleChars(4);
      OLED_1();
      break;
  }
  change = true;
}

void QUESTION() {   // The first identifier was "?"
  //Do something (See void EQUALS)
  String value = "";
  CodeIn = getChar(); // Get another character
  switch (CodeIn) { // Now lets find what to do with it
    case 'Z':
      RudderTrim = readMultipleChars(4);
      OLED_1();
      break;
    case 'k':
      AltimeterSettinginHg = readMultipleChars(5);
      break;
  }
  change = true;
}

void HASH() {   // The first identifier was "?"
  //Do something (See void EQUALS)
  String value = "";
  CodeIn = getChar(); // Get another character
  switch (CodeIn) { // Now lets find what to do with it
    case 'A':
      value = readMultipleChars(1);
      if (value == "0") {
        switch_gear == GEAR_UP;
      } else {
        switch_gear == GEAR_DN;
      }
      break;
    case 'a':
      AltimeterSettingmBar = readMultipleChars(4);
      break;
    case 'b':
      SAT = readMultipleChars(3);
      break;
    case 'c':
      TAT = readMultipleChars(3);
      break;
    case 'd':
      LocalTimeHour = readMultipleChars(2);
      break;
    case 'e':
      LocalTimeMinute = readMultipleChars(2);
      break;
    case 'f':
      ZuluTimeHour = readMultipleChars(2);
      break;
    case 'g':
      ZuluTimeMinute = readMultipleChars(2);
      break;
  }
  change = true;
}

void SLASH() {   // The first identifier was "/" (Annunciators)
  //Do something (See void EQUALS)
  change = true;
}

void oled_init() {
  // OLED DISPLAY INIT -------------------------------------------------------------------------------------
  Serial.println("OLED init ...");

  digitalWrite(DISPLAY_1, HIGH);
  digitalWrite(DISPLAY_2, HIGH);

  // by default, we'll generate the LOW voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)


  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.clearDisplay();
  display.setCursor(10, 10);
  display.println("Display init");
  display.display();

  digitalWrite(DISPLAY_1, LOW);
  digitalWrite(DISPLAY_2, LOW);

  Serial.println("...finished");
}

void OLED_2() {
  digitalWrite(DISPLAY_2, HIGH);
  display.clearDisplay();

  display.setRotation(1);
  drawTrims(0, 0, 92, 62, ElevatorTrim, RudderTrim, AileronTrim);
  display.display();

  digitalWrite(DISPLAY_2, LOW);
}

void OLED_1() {
  digitalWrite(DISPLAY_1, HIGH);
  display.clearDisplay();
  display.setRotation(3);

  display.setCursor(0, 0);
  display.println(" " + AltimeterSettingmBar + " mBar");

  display.setCursor(0, 8);
  display.println(AltimeterSettinginHg + " inHg");

  display.setCursor(0, 20);
  display.println("  " + SAT + " SAT");

  display.setCursor(0, 28);
  display.println("  " + TAT + " TAT");

  display.drawLine(0, 108, 64, 108, WHITE);
  display.drawLine(0, 110, 64, 110, WHITE);

  display.setCursor(0, 112);
  display.println(LocalTimeHour + ":" + LocalTimeMinute + " LOC");

  display.setCursor(0, 120);
  display.println(ZuluTimeHour + ":" + ZuluTimeMinute + " UTC");

  display.display();
  digitalWrite(DISPLAY_1, LOW);
}

void tft_init() {
  // TFT DISPLAY INIT --------------------------------------------------------------------------------------

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  } else if (identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if (identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0101)
  {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  }
  else if (identifier == 0x1111)
  {
    identifier = 0x9328;
    Serial.println(F("Found 0x9328 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier = 0x9328;

  }
  tft.begin(identifier);
  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.setTextColor(WHITE);
  tft.setTextSize(3);

  tft.setRotation(2);
  tft.fillScreen(BLACK);
  tft.setCursor(10, 10);
  tft.println("Display init");
}

void draw_tft_pattern() {
  // TFT LINES AND STATIC TEXT ------------------------------------------------------------------------------
  tft.setTextSize(1);

  tft.fillScreen(BLACK);
  tft.drawLine(120, 0, 120, 185, GREEN);    // vertical center green line

  // print COM1 in the freq box
  tft.setCursor(110, 5);
  tft.setTextColor(GREEN, BLACK);
  tft.println("C");
  tft.setCursor(110, 15);
  tft.println("O");
  tft.setCursor(110, 25);
  tft.println("M");
  tft.setCursor(110, 45);
  tft.println("1");

  // print NAV1 in the freq box
  tft.setCursor(230, 5);
  tft.setTextColor(GREEN, BLACK);
  tft.println("N");
  tft.setCursor(230, 15);
  tft.println("A");
  tft.setCursor(230, 25);
  tft.println("V");
  tft.setCursor(230, 45);
  tft.println("1");

  tft.drawLine(0, 60, 240, 60, GREEN);      // first green line radio

  // print COM2 in the freq box
  tft.setCursor(110, 65);
  tft.setTextColor(GREEN, BLACK);
  tft.println("C");
  tft.setCursor(110, 75);
  tft.println("O");
  tft.setCursor(110, 85);
  tft.println("M");
  tft.setCursor(110, 100);
  tft.println("2");

  // print NAV2 in the freq box
  tft.setCursor(230, 65);
  tft.setTextColor(GREEN, BLACK);
  tft.println("N");
  tft.setCursor(230, 75);
  tft.println("A");
  tft.setCursor(230, 85);
  tft.println("V");
  tft.setCursor(230, 100);
  tft.println("2");

  tft.drawLine(0, 124, 240, 124, GREEN);    // second green line radio

  // print COM2 in the freq box
  tft.setCursor(110, 129);
  tft.setTextColor(GREEN, BLACK);
  tft.println("A");
  tft.setCursor(110, 139);
  tft.println("D");
  tft.setCursor(110, 149);
  tft.println("F");
  tft.setCursor(110, 164);
  tft.println("1");

  // print NAV2 in the freq box
  tft.setCursor(230, 129);
  tft.setTextColor(GREEN, BLACK);
  tft.println("X");
  tft.setCursor(230, 139);
  tft.println("P");
  tft.setCursor(230, 149);
  tft.println("D");
  tft.setCursor(230, 159);
  tft.println("R");

  tft.drawLine(0, 185, 240, 185, GREEN);    // third green line radio
  tft.drawLine(0, 275, 240, 275, GREEN);    // separator autopilot data and indicators
  tft.drawLine(80, 275, 80, 320, GREEN);    // AP left indicator separator
  tft.drawLine(160, 275, 160, 320, GREEN);  // AP right indicator separator

  tft.setTextSize(2);
}

void force_P3D_data_refresh() {
  Serial.println("REF");
}

