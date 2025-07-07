# Manette Bluetooth ESP32

Une manette de jeu Bluetooth personnalisée utilisant un ESP32 avec joystick analogique et 4 boutons ABXY.

## 📋 Caractéristiques

- **Joystick analogique XY** avec bouton intégré
- **4 boutons d'action** (A, B, X, Y)
- **Connexion Bluetooth BLE**
- **Compatible** avec PC, smartphones, tablettes

## 🛠️ Matériel requis

- ESP32 (ESP32 DevKit ou équivalent)
- 1 joystick analogique avec bouton (module KY-023 ou similaire)
- 4 boutons poussoirs
- Câbles de connexion
- Breadboard (optionnel)

## 📚 Bibliothèques nécessaires

Installer via le gestionnaire de bibliothèques Arduino IDE :

1. **Bounce2** by Thomas O Fredericks
   - https://github.com/thomasfredericks/Bounce2
   
2. **ESP32-BLE-Gamepad** by lemmingDev
   - https://github.com/lemmingDev/ESP32-BLE-Gamepad

## 🔌 Schéma de connexion

### Joystick analogique
| Joystick | ESP32 |
|----------|-------|
| VCC      | 3.3V  |
| GND      | GND   |
| VRx      | GPIO 34 |
| VRy      | GPIO 35 |
| SW       | GPIO 32 |

### Boutons d'action
| Bouton | ESP32 |
|--------|-------|
| A      | GPIO 25 |
| B      | GPIO 26 |
| X      | GPIO 27 |
| Y      | GPIO 14 |

**Note** : Connecter chaque bouton entre le GPIO correspondant et GND. Les résistances pull-up internes sont activées dans le code.

## 📝 Configuration Arduino IDE

1. **Installer l'ESP32** dans Arduino IDE :
   - Fichier → Préférences
   - Ajouter dans "URL de gestionnaire de cartes" :
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Outils → Type de carte → Gestionnaire de carte → Installer "esp32"

2. **Sélectionner la carte** :
   - Outils → Type de carte → ESP32 Arduino → ESP32 Dev Module


## 🚀 Utilisation

1. **Téléverser le code** sur l'ESP32

2. **Ouvrir le moniteur série** (115200 bauds) pour voir les messages de debug

3. **Activer le Bluetooth** sur votre appareil (PC, smartphone, etc.)

4. **Rechercher** le périphérique "Declic"

5. **Appairer** le gamepad

6. **Tester** dans un jeu ou une application de test de gamepad

## 🎮 Mapping des contrôles

| Contrôle | Fonction gamepad |
|----------|-----------------|
| Joystick X | Axe X (-32767 à 32767) |
| Joystick Y | Axe Y (-32767 à 32767) |
| Bouton Joystick | BUTTON_1 |
| Bouton A | BUTTON_2 |
| Bouton B | BUTTON_3 |
| Bouton X | BUTTON_4 |
| Bouton Y | BUTTON_5 |

## ⚙️ Paramètres ajustables

### Zone morte du joystick
```cpp
const int deadzone = 100;  // Ajuster selon vos besoins
```

### Intervalle de debounce
```cpp
btnA.interval(10);  // En millisecondes
```

## 🔧 Dépannage

### Le gamepad n'apparaît pas en Bluetooth
- Vérifier que le code est bien téléversé
- Redémarrer l'ESP32
- Désactiver/réactiver le Bluetooth sur l'appareil

### Les boutons ne répondent pas correctement
- Vérifier les connexions
- Tester avec le moniteur série pour voir les messages de debug
- Ajuster l'intervalle de debounce si nécessaire

### Le joystick dérive
- Augmenter la valeur de la zone morte
- Vérifier l'alimentation du joystick (3.3V stable)

## 📄 Licence

Ce projet est distribué sous la même licence que le dépôt LFPxDeclic.

## 🤝 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à :
- Proposer des améliorations
- Signaler des problèmes
- Partager vos modifications et adaptations