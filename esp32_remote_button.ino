#include <BleKeyboard.h>
#include <Preferences.h>

Preferences preferences;
BleKeyboard bleKeyboard("Furry-Remote", "Flammever", 100);

const int buttonPin = 4;
const int shutdownPin = 2;
bool buttonPressed = false;
bool wasConnected = false;
unsigned long lastPressTime = 0;
const unsigned long pressDelay = 500;
unsigned long shutdownPressTime = 0;
const unsigned long shutdownHoldTime = 3000;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // Try every 5 seconds

// Stored Bluetooth data
String lastConnectedDevice = "";
bool shouldReconnect = false;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(shutdownPin, INPUT_PULLUP);
  
  // Initialize preferences (like EEPROM but better)
  preferences.begin("bluetooth", false);
  
  // Load last connected device from memory
  lastConnectedDevice = preferences.getString("lastDevice", "");
  shouldReconnect = preferences.getBool("shouldReconnect", false);
  
  // Start BLE
  startBLE();
}

void startBLE() {
  bleKeyboard.begin();
  
  // If we have a last device and should reconnect, start advertising
  if (shouldReconnect && lastConnectedDevice != "") {
    // BLE will automatically try to reconnect to the last device
  }
}

void loop() {
  bool isConnected = bleKeyboard.isConnected();
  
  // Handle connection state changes
  if (isConnected && !wasConnected) {
    // Just connected - save this device for future reconnection
    saveConnectionInfo(true);
    wasConnected = true;
  }
  
  if (!isConnected && wasConnected) {
    // Just disconnected
    wasConnected = false;
    lastReconnectAttempt = millis();
  }
  
  // Auto-reconnect if we lost connection but should be connected
  if (!isConnected && shouldReconnect) {
    if (millis() - lastReconnectAttempt > reconnectInterval) {
      // Restart BLE to try reconnecting
      bleKeyboard.end();
      delay(100);
      bleKeyboard.begin();
      lastReconnectAttempt = millis();
    }
  }
  
  // Check shutdown button
  checkShutdownButton();
  
  // Take pictures when connected
  if (isConnected) {
    bool buttonState = digitalRead(buttonPin);
    
    if (buttonState == LOW && !buttonPressed) {
      if (millis() - lastPressTime > pressDelay) {
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        buttonPressed = true;
        lastPressTime = millis();
      }
    }
    
    if (buttonState == HIGH && buttonPressed) {
      buttonPressed = false;
    }
  }
  
  delay(50);
}

void saveConnectionInfo(bool connected) {
  if (connected) {
    // When connected, remember we should reconnect
    preferences.putBool("shouldReconnect", true);
    // Note: We can't easily get the actual device MAC address with BleKeyboard
    // but we set the flag to attempt reconnection
  } else {
    // When manually shutdown, don't reconnect
    preferences.putBool("shouldReconnect", false);
  }
  preferences.end();
}

void checkShutdownButton() {
  if (digitalRead(shutdownPin) == LOW) {
    if (shutdownPressTime == 0) {
      shutdownPressTime = millis();
    }
    
    if (millis() - shutdownPressTime >= shutdownHoldTime) {
      shutdownDevice();
    }
  } else {
    shutdownPressTime = 0;
  }
}

void shutdownDevice() {
  // Save that we're manually disconnecting (don't auto-reconnect)
  saveConnectionInfo(false);
  
  // Disconnect Bluetooth
  bleKeyboard.end();
  
  // Stop operation
  while(true) {
    delay(1000);
  }
}
