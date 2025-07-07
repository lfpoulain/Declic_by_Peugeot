#include <Arduino.h>
#include <Bounce2.h>
#include <BleGamepad.h>

// Configuration du gamepad
BleGamepadConfiguration bleGamepadConfig;
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
int joystickXCenter = 2048;
int joystickYCenter = 2048;
const int deadzone = 200;  // Zone morte élargie

// Filtrage simple
int lastXValue = 2048;
int lastYValue = 2048;
const float smoothingFactor = 0.8; // 0 = pas de lissage, 1 = lissage maximum

void setup() {
  Serial.begin(115200);
  Serial.println("Démarrage du gamepad Bluetooth ESP32");

  // Configuration du gamepad pour compatibilité web
  bleGamepadConfig.setAxesMin(0x0000); // 0 pour compatibilité navigateur
  bleGamepadConfig.setAxesMax(0x7FFF); // 32767
  bleGamepad.begin(&bleGamepadConfig);
  
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
  joystickBtn.interval(25);
  
  btnA.attach(BTN_A_PIN);
  btnA.interval(25);
  
  btnB.attach(BTN_B_PIN);
  btnB.interval(25);
  
  btnX.attach(BTN_X_PIN);
  btnX.interval(25);
  
  btnY.attach(BTN_Y_PIN);
  btnY.interval(25);
  
  // Calibrage du joystick
  delay(1000);
  calibrateJoystick();
  
  Serial.println("En attente de connexion Bluetooth...");
}

void calibrateJoystick() {
  Serial.println("Calibrage du joystick...");
  
  long xSum = 0;
  long ySum = 0;
  int samples = 100;
  
  for (int i = 0; i < samples; i++) {
    xSum += analogRead(JOYSTICK_X_PIN);
    ySum += analogRead(JOYSTICK_Y_PIN);
    delay(5);
  }
  
  joystickXCenter = xSum / samples;
  joystickYCenter = ySum / samples;
  
  // Initialiser les dernières valeurs
  lastXValue = joystickXCenter;
  lastYValue = joystickYCenter;
  
  Serial.print("Centre X: ");
  Serial.print(joystickXCenter);
  Serial.print(" Centre Y: ");
  Serial.println(joystickYCenter);
}

void loop() {
  // Mise à jour des boutons
  joystickBtn.update();
  btnA.update();
  btnB.update();
  btnX.update();
  btnY.update();
  
  if (bleGamepad.isConnected()) {
    // Lecture des valeurs brutes
    int xRaw = analogRead(JOYSTICK_X_PIN);
    int yRaw = analogRead(JOYSTICK_Y_PIN);
    
    // Application du filtre passe-bas simple
    int xFiltered = (smoothingFactor * lastXValue) + ((1 - smoothingFactor) * xRaw);
    int yFiltered = (smoothingFactor * lastYValue) + ((1 - smoothingFactor) * yRaw);
    
    // Sauvegarder pour le prochain cycle
    lastXValue = xFiltered;
    lastYValue = yFiltered;
    
    // Calculer la différence par rapport au centre
    int xDiff = xFiltered - joystickXCenter;
    int yDiff = yFiltered - joystickYCenter;
    
    // Variables pour les valeurs finales (0 à 32767 pour compatibilité web)
    int16_t gamepadX = 16384; // Centre à 16384 (milieu de 0-32767)
    int16_t gamepadY = 16384;
    
    // Application de la zone morte sur X
    if (abs(xDiff) > deadzone) {
      // Calculer la position relative en pourcentage
      float percentage;
      if (xDiff > 0) {
        // Côté positif (droite)
        int maxDistance = 4095 - joystickXCenter - deadzone;
        percentage = (float)(xDiff - deadzone) / maxDistance;
        percentage = constrain(percentage, 0.0, 1.0);
        gamepadX = 16384 + (int16_t)(percentage * 16383); // 16384 à 32767
      } else {
        // Côté négatif (gauche)
        int maxDistance = joystickXCenter - deadzone;
        percentage = (float)(-xDiff - deadzone) / maxDistance;
        percentage = constrain(percentage, 0.0, 1.0);
        gamepadX = 16384 - (int16_t)(percentage * 16384); // 0 à 16384
      }
    }
    
    // Application de la zone morte sur Y
    if (abs(yDiff) > deadzone) {
      // Calculer la position relative en pourcentage
      float percentage;
      if (yDiff > 0) {
        // Côté positif (bas)
        int maxDistance = 4095 - joystickYCenter - deadzone;
        percentage = (float)(yDiff - deadzone) / maxDistance;
        percentage = constrain(percentage, 0.0, 1.0);
        gamepadY = 16384 + (int16_t)(percentage * 16383); // 16384 à 32767
      } else {
        // Côté négatif (haut)
        int maxDistance = joystickYCenter - deadzone;
        percentage = (float)(-yDiff - deadzone) / maxDistance;
        percentage = constrain(percentage, 0.0, 1.0);
        gamepadY = 16384 - (int16_t)(percentage * 16384); // 0 à 16384
      }
    }
    
    // Contraindre les valeurs pour être sûr (0 à 32767)
    gamepadX = constrain(gamepadX, 0, 32767);
    gamepadY = constrain(gamepadY, 0, 32767);
    
    // Envoyer les valeurs
    bleGamepad.setAxes(gamepadX, gamepadY, 0, 0, 0, 0, 0, 0);
    
    // Debug périodique
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 200) {
      lastDebug = millis();
      Serial.print("Raw X:");
      Serial.print(xRaw);
      Serial.print(" Y:");
      Serial.print(yRaw);
      Serial.print(" -> Gamepad X:");
      Serial.print(gamepadX);
      Serial.print(" Y:");
      Serial.println(gamepadY);
    }
    
    // Gestion des boutons
    if (joystickBtn.fell()) {
      bleGamepad.press(BUTTON_1);
      Serial.println("Bouton joystick pressé");
    }
    if (joystickBtn.rose()) {
      bleGamepad.release(BUTTON_1);
      Serial.println("Bouton joystick relâché");
    }
    
    if (btnA.fell()) {
      bleGamepad.press(BUTTON_2);
      Serial.println("Bouton A pressé");
    }
    if (btnA.rose()) {
      bleGamepad.release(BUTTON_2);
      Serial.println("Bouton A relâché");
    }
    
    if (btnB.fell()) {
      bleGamepad.press(BUTTON_3);
      Serial.println("Bouton B pressé");
    }
    if (btnB.rose()) {
      bleGamepad.release(BUTTON_3);
      Serial.println("Bouton B relâché");
    }
    
    if (btnX.fell()) {
      bleGamepad.press(BUTTON_4);
      Serial.println("Bouton X pressé");
    }
    if (btnX.rose()) {
      bleGamepad.release(BUTTON_4);
      Serial.println("Bouton X relâché");
    }
    
    if (btnY.fell()) {
      bleGamepad.press(BUTTON_5);
      Serial.println("Bouton Y pressé");
    }
    if (btnY.rose()) {
      bleGamepad.release(BUTTON_5);
      Serial.println("Bouton Y relâché");
    }
  }
  
  delay(10); // Délai pour stabilité
}