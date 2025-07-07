#include <Arduino.h>
#include <Bounce2.h>
#include <BleGamepad.h>

// Configuration du gamepad
BleGamepad bleGamepad("Declic", "LFPxPeugeot", 100);

// Définition des pins
#define JOYSTICK_X_PIN 34  // Pin analogique pour l'axe X
#define JOYSTICK_Y_PIN 35  // Pin analogique pour l'axe Y
#define JOYSTICK_BTN_PIN 32 // Pin numérique pour le bouton du joystick

#define BTN_A_PIN 25  // Bouton A
#define BTN_B_PIN 26  // Bouton B
#define BTN_X_PIN 27  // Bouton X
#define BTN_Y_PIN 14  // Bouton Y

// Création des objets Bounce pour les boutons
Bounce joystickBtn = Bounce();
Bounce btnA = Bounce();
Bounce btnB = Bounce();
Bounce btnX = Bounce();
Bounce btnY = Bounce();

// Variables pour le joystick
int joystickXCenter = 2048;  // Valeur centrale pour l'axe X (12 bits ADC)
int joystickYCenter = 2048;  // Valeur centrale pour l'axe Y (12 bits ADC)
const int deadzone = 100;     // Zone morte autour du centre

void setup() {
  Serial.begin(115200);
  Serial.println("Démarrage du gamepad Bluetooth ESP32");

  // Configuration des pins du joystick
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  
  // Configuration des boutons avec pull-up interne
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP);
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  pinMode(BTN_B_PIN, INPUT_PULLUP);
  pinMode(BTN_X_PIN, INPUT_PULLUP);
  pinMode(BTN_Y_PIN, INPUT_PULLUP);
  
  // Initialisation des objets Bounce
  joystickBtn.attach(JOYSTICK_BTN_PIN);
  joystickBtn.interval(10);
  
  btnA.attach(BTN_A_PIN);
  btnA.interval(10);
  
  btnB.attach(BTN_B_PIN);
  btnB.interval(10);
  
  btnX.attach(BTN_X_PIN);
  btnX.interval(10);
  
  btnY.attach(BTN_Y_PIN);
  btnY.interval(10);
  
  // Calibrage du joystick au démarrage
  delay(100);
  joystickXCenter = analogRead(JOYSTICK_X_PIN);
  joystickYCenter = analogRead(JOYSTICK_Y_PIN);
  
  // Démarrage du gamepad Bluetooth
  bleGamepad.begin();
  
  // Attendre la connexion Bluetooth
  Serial.println("En attente de connexion Bluetooth...");
}

void loop() {
  // Mise à jour des boutons
  joystickBtn.update();
  btnA.update();
  btnB.update();
  btnX.update();
  btnY.update();
  
  if (bleGamepad.isConnected()) {
    // Lecture des valeurs du joystick
    int xValue = analogRead(JOYSTICK_X_PIN);
    int yValue = analogRead(JOYSTICK_Y_PIN);
    
    // Application de la zone morte
    if (abs(xValue - joystickXCenter) < deadzone) {
      xValue = joystickXCenter;
    }
    if (abs(yValue - joystickYCenter) < deadzone) {
      yValue = joystickYCenter;
    }
    
    // Conversion des valeurs ADC (0-4095) vers les valeurs du gamepad (-32767 à 32767)
    int16_t gamepadX = map(xValue, 0, 4095, -32767, 32767);
    int16_t gamepadY = map(yValue, 0, 4095, -32767, 32767);
    
    // Envoi des valeurs du joystick
    bleGamepad.setAxes(gamepadX, gamepadY, 0, 0, 0, 0, 0, 0);
    
    // Gestion du bouton du joystick (bouton 1)
    if (joystickBtn.fell()) {
      bleGamepad.press(BUTTON_1);
      Serial.println("Bouton joystick pressé");
    }
    if (joystickBtn.rose()) {
      bleGamepad.release(BUTTON_1);
      Serial.println("Bouton joystick relâché");
    }
    
    // Gestion du bouton A (bouton 2)
    if (btnA.fell()) {
      bleGamepad.press(BUTTON_2);
      Serial.println("Bouton A pressé");
    }
    if (btnA.rose()) {
      bleGamepad.release(BUTTON_2);
      Serial.println("Bouton A relâché");
    }
    
    // Gestion du bouton B (bouton 3)
    if (btnB.fell()) {
      bleGamepad.press(BUTTON_3);
      Serial.println("Bouton B pressé");
    }
    if (btnB.rose()) {
      bleGamepad.release(BUTTON_3);
      Serial.println("Bouton B relâché");
    }
    
    // Gestion du bouton X (bouton 4)
    if (btnX.fell()) {
      bleGamepad.press(BUTTON_4);
      Serial.println("Bouton X pressé");
    }
    if (btnX.rose()) {
      bleGamepad.release(BUTTON_4);
      Serial.println("Bouton X relâché");
    }
    
    // Gestion du bouton Y (bouton 5)
    if (btnY.fell()) {
      bleGamepad.press(BUTTON_5);
      Serial.println("Bouton Y pressé");
    }
    if (btnY.rose()) {
      bleGamepad.release(BUTTON_5);
      Serial.println("Bouton Y relâché");
    }
    
    // Petit délai pour éviter de surcharger le Bluetooth
    delay(10);
  }
}