// Based on Shatenzo's ino. Sorry for not forking,
// realized my mistake after I added the repo.


// Make sure you have the Arduino program set to
// USB Keyboard mode.


// Amount of rows and columns in your keyboard matrix
const int ROWS       = 5;
const int COLUMNS    = 14;

// Debounce delay. 4-5 is fine for me. Too slow, and you
// get unresponsive keys, too fast and you get double presses.
const int DEBOUNCE   = 4;

// Bogus values. The values are never used, the names are used
// to compare stuff inside functions.
const int MODIFIERKEY_FNLOCK = 255;
const int MODIFIERKEY_FN     = 254;
const int MODIFIERKEY_ALTGR  = 253;

// True for Layer 1, false for Layer 2.
// Want more layers? Use an int instead.
boolean isLayerOne = true;

// Pins for rows and columns. Corresponds the pins used on your Teensy.
byte rowPins[ROWS]       = {1, 2, 3, 4, 5};
byte columnPins[COLUMNS] = {19, 18, 17, 16, 15, 14, 20,
                                          12, 11, 10, 9, 8, 7, 6};

// Layer layouts

int layer1[ROWS][COLUMNS] {
  {KEY_TILDE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE},
  {KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRACE, KEY_RIGHT_BRACE, KEY_BACKSLASH},
  {MODIFIERKEY_FNLOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, 0, KEY_ENTER},
  {MODIFIERKEY_SHIFT, 0, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, 0, MODIFIERKEY_SHIFT},
  {MODIFIERKEY_CTRL, MODIFIERKEY_GUI, MODIFIERKEY_ALT, 0, 0, 0, KEY_SPACE, 0, 0, 0, MODIFIERKEY_ALTGR, MODIFIERKEY_FN, MODIFIERKEY_CTRL, KEY_DELETE}
};

int layer2[ROWS][COLUMNS] {
  {KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_BACKSPACE},
  {KEY_TAB, KEY_HOME, KEY_UP, KEY_END, KEY_PAGE_UP, 0, 0, 0, 0, 0, 0, 0, KEY_PAUSE, KEY_DELETE},
  {MODIFIERKEY_FNLOCK, KEY_LEFT, KEY_DOWN, KEY_RIGHT, KEY_PAGE_DOWN, 0, 0, 0, 0, 0, 0, 0, KEY_PRINTSCREEN, KEY_ENTER},
  {MODIFIERKEY_SHIFT, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_CAPS_LOCK, KEY_SCROLL_LOCK, KEY_NUM_LOCK, MODIFIERKEY_SHIFT},
  {MODIFIERKEY_CTRL, MODIFIERKEY_GUI, MODIFIERKEY_ALT, 0, 0, 0, KEY_SPACE, 0, 0, 0, MODIFIERKEY_ALTGR, MODIFIERKEY_FN, MODIFIERKEY_CTRL, KEY_INSERT}
};

// STATE CHECKS

byte lastState[ROWS][COLUMNS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte currentState[ROWS][COLUMNS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// METHODS

// * Returns the value from one of the layers.
// * 
// * const int i :: row
// * const int k :: column
int getKey(const int i, const int k) {

  switch(isLayerOne) {
    case true:
      return layer1[i][k];
    case false:
      return layer2[i][k];
  }
}

// * Presses the key on the keyboard. Special cases for
// * modifiers and AltGr. The modifier lock only changes
// * the state on keydown.
// *
// * boolean down :: true is pressing key, false is releasing key
// * int getKey :: the key that should be pressed. Comes from the
// *               getKey() function above.
void handleKey(boolean down, int getKey) {

  if((down && getKey == MODIFIERKEY_FNLOCK) || getKey == MODIFIERKEY_FN) {
    isLayerOne = !isLayerOne;
  }

  if(getKey == MODIFIERKEY_ALTGR) {
    altGr(down);
  }
  else {
    switch(down) {
      case true:
        Keyboard.press(getKey);
        break;
      case false:
        Keyboard.release(getKey);
    }
  }

  Keyboard.send_now();
}

// * AltGr simulation.
// *
// * boolean down :: true for press key, false for release key
void altGr(boolean down) {
  switch(down) {
    case true:
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_ALT);
      break;
    case false:
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release(KEY_LEFT_ALT);
  }
}

// * Keeps track of key states. Shamelessly stolen from
// * Shatenzo's ino.
void stateChange() {
  int i;
  int k;

  for (i = 0; i < ROWS; ++i) {
    for (k = 0; k < COLUMNS; ++k) {
      if (currentState[i][k] != lastState[i][k]) {
        if(currentState[i][k] == 1) {
          handleKey(true, getKey(i, k));
        }
        else {
          handleKey(false, getKey(i, k));
        }
      }
      lastState[i][k] = currentState[i][k];
      currentState[i][k] = 0;
    }
  }
}

// * Constantly checks the keyboard for potential key presses.
// * The rows are set to HIGH one at a time, and if a column
// * changes to HIGH when the row is HIGH you know you have a
// * key press.
// *
// * Shamelessly stolen from Shatenzo's ino. Changed the way the
// * debounce behaves to give some delay between each row's HIGH
// * switch, as it sometimes caused two rows to be HIGH at the
// * same time.
void keyCheck() {
  int i = 0;
  int k = 0;

  for (i = 0; i < ROWS; ++i) {
      for (k = 0; k < COLUMNS; ++k) {
        currentState[i][k] = 0;
        pinMode(rowPins[i], HIGH);

        boolean columnPressed = digitalRead(columnPins[k]) == 0;
        if (columnPressed) {
          pinMode(rowPins[i], LOW);

          boolean currentKeyDown = digitalRead(columnPins[k]) == 0;
          if (currentKeyDown) {
            currentState[i][k] = 1;
            
//            Debug stuff
//            Serial.print(i); Serial.print(":"); Serial.println(k);
          }
          pinMode(rowPins[i], HIGH);
        }
        pinMode(rowPins[i], LOW);
    }
    pinMode(rowPins[i], LOW);
    delay(DEBOUNCE);
  }
  stateChange();
}

// * setup
void setup() {

  int i = 0;

  for (i = 0; i < ROWS; ++i) {
      pinMode(rowPins[i], OUTPUT);
  }
  for (i = 0; i < COLUMNS; ++i) {
      pinMode(columnPins[i], INPUT_PULLUP);
  }
}

// * loop
void loop() {
  keyCheck();
}
