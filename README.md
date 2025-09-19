# LFPxDeclic — Les Frères Poulain x Peugeot DÉCLIC

Bienvenue dans le dépôt des projets LFPxDeclic. Ce repo rassemble des réalisations open source présentées avec Peugeot DÉCLIC et Les Frères Poulain.

## Projets

Les deux projets sont au même niveau et complémentaires. Choisissez selon votre besoin — IoT météo ou interaction BLE.

### Station météo (ESP32)
Interface web embarquée pour suivre température, humidité, pression (BME280), luminosité (BH1750), vitesse du vent (anémomètre à reed) et pluie (LM393 AO).

- Matériel: ESP32, BME280 (I2C), BH1750 (I2C), anémomètre reed, capteur pluie LM393 (AO)
- Interface: serveur web local (`/`, `/advanced`, `/config`), API JSON (`/api/sensors`, `/api/config`)
- Fonctionnalités: calibration pluie/ensoleillement, affichage simple et avancé, seuils de couleurs

[Accéder au projet station-meteo](./station-meteo/README.md)

Démarrage rapide (résumé):
1. Ouvrir `station-meteo/station-meteo.ino` dans l’IDE Arduino
2. Renseigner `WIFI_SSID` et `WIFI_PASSWORD`
3. Téléverser sur un ESP32 et visiter `http://<ip_esp32>/`
4. Faire la calibration depuis `/config`

### Manette Bluetooth ESP32 (BLE)
Gamepad BLE personnalisable compatible PC, smartphones et tablettes (joystick analogique + boutons ABXY).

- Matériel: ESP32, joystick analogique, 4 boutons, câblage simple avec pull-ups internes
- Bibliothèques: Bounce2, ESP32-BLE-Gamepad
- Fonctionnalités: mapping axes/boutons, debounce, zone morte réglable

[Accéder au projet manette-bluetooth-esp32](./manette-bluetooth-esp32/README.md)

Démarrage rapide (résumé):
1. Installer le support ESP32 dans Arduino IDE (cartes Espressif)
2. Installer `Bounce2` et `ESP32-BLE-Gamepad`
3. Flasher le code sur un ESP32
4. Appairer le périphérique Bluetooth nommé « Declic »

## Où nous trouver
- Chaîne Peugeot DÉCLIC: https://www.youtube.com/c/D%C3%89CLICbyPeugeot
- Les Frères Poulain: https://www.youtube.com/c/lesfrerespoulain

## Contribuer
Les contributions sont les bienvenues:
- Partagez vos réalisations basées sur ces projets
- Ouvrez des issues/PR pour corrections et améliorations
- Proposez de nouvelles idées

## Licence
Les projets de ce dépôt sont open source. Consultez les README de chaque dossier pour les détails spécifiques.

## Remerciements
Merci à Peugeot DÉCLIC pour leur confiance et l’occasion de partager des projets utiles et accessibles.

— « L’ingéniosité au service du quotidien » — LFPxDeclic
