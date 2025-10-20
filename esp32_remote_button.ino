#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Camera-Remote", "Samsung", 100);

const int buttonPin = 4;
bool buttonPressed = false;
unsigned long lastPressTime = 0;
const unsigned long pressDelay = 500; // Minimum time between presses

void setup() {
  //Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP); // Use internal pull-up resistor
  
  //Serial.println("Starting ESP32 Camera Trigger");
  //Serial.println("Waiting for BLE connection...");
  bleKeyboard.begin();
}

void loop() {
  if (bleKeyboard.isConnected()) {
    // Read button state
    bool buttonState = digitalRead(buttonPin);
    
    // Button is pressed (LOW because of pull-up)
    if (buttonState == LOW && !buttonPressed) {
      if (millis() - lastPressTime > pressDelay) {
        //Serial.println("Triggering camera with Volume Up");
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        buttonPressed = true;
        lastPressTime = millis();

      }
    }
    
    // Button is released
    if (buttonState == HIGH && buttonPressed) {
      bleKeyboard.release(KEY_MEDIA_VOLUME_UP);
      buttonPressed = false;
    }
  } else {
    //Serial.println("BLE not connected to phone");
    delay(1000);
  }
  
  delay(50);
}
