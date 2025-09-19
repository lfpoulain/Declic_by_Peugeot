# Station météo ESP32 — Interface web

Ce projet fournit un sketch Arduino pour ESP32 avec une interface web (serveur embarqué) permettant d’afficher :
- BME280 (I2C) : température, humidité, pression
- BH1750 (I2C) : luminosité (lux)
- Anémomètre à reed : vitesse du vent
- LM393 (pluie) : lecture analogique AO uniquement

## Démarrage rapide
- Ouvrir `station-meteo.ino` dans l’IDE Arduino
- Renseigner `WIFI_SSID` et `WIFI_PASSWORD`
- Ajuster les pins si besoin puis téléverser
- Accéder à `http://<ip_esp32>/` (ou au point d’accès `ESP32-Station` si la connexion STA échoue)

## Bibliothèques requises
- Adafruit BME280 Library
- Adafruit Unified Sensor
- BH1750

## Câblage (défaut)
- I2C BME280 + BH1750 : SDA=21, SCL=22, 3V3, GND
- Anémomètre : un fil → GPIO `ANEMOMETER_PIN` (27), l’autre → GND, pull‑up activée
- LM393 pluie (AO) : AO → GPIO ADC (33 par défaut), 3V3, GND

## Pages web
- `/` Tableau de bord simple (valeurs principales, emojis vent/pluie, % d’ensoleillement coloré)
- `/advanced` Vue avancée (grille complète : lux, % ensoleillement, vent pps/rpm, pluie %/V/ADC)
- `/config` Configuration (seuils pluie/ensoleillement + calibration pluie et lux)

## Mise en route (calibration)

1) Flasher et se connecter
   - Ouvrir `station-meteo.ino`, renseigner `WIFI_SSID`/`WIFI_PASSWORD`, téléverser.
   - Accéder au device : `http://<ip_esp32>/` (ou AP `ESP32-Station`).

2) Ouvrir la page `/config` (lien en haut du dashboard)

3) Pluie (LM393 AO)
   - S’assurer que la plaque est bien sèche, puis cliquer sur « Capturer SEC maintenant ».
   - Mouiller franchement la plaque, puis cliquer sur « Capturer MOUILLÉ maintenant ».
   - Ajuster si besoin les seuils de couleur (orange/vert) pour l’affichage.

4) Ensoleillement (BH1750)
   - Placer le capteur dans l’obscurité souhaitée puis cliquer sur « Capturer MIN maintenant » (0%).
   - Placer en pleine lumière (max local) puis cliquer sur « Capturer MAX maintenant » (100%).
   - Ajuster au besoin les seuils de couleur (orange/vert) pour le %.

5) Vérifier
   - Dashboard `/` : vent avec libellé/emoji, pluie « Sec/Pluvieux » + emoji, ensoleillement % coloré.
   - Page `/advanced` : toutes les valeurs détaillées pour contrôle.

## API
- GET `/api/sensors` → mesures JSON :
  - `temperature_c`, `humidity_pct`, `pressure_hpa`
  - `lux` et `light_percent` (si `lux_min`/`lux_max` calibrés)
  - `wind` : `pulses`, `pulses_per_sec`, `rpm`, `speed_mps`, `speed_kmh`
  - `lm393_analog`, `lm393_voltage_v`, `lm393_percent`
- GET `/api/config` →
  ```json
  {
    "rain": { "green_min": 70, "orange_min": 30, "cal": { "dry_raw": 3500, "wet_raw": 800 } },
    "light": { "green_min": 70, "orange_min": 30, "cal": { "lux_min": 0, "lux_max": 50000 } }
  }
  ```
- POST `/api/config` (application/x-www-form-urlencoded)
  - Pluie : `rain_green_min`, `rain_orange_min`, `rain_dry_raw`, `rain_wet_raw`, `rain_capture`=`dry|wet`
  - Ensoleillement : `light_green_min`, `light_orange_min`, `lux_min`, `lux_max`, `lux_capture`=`min|max`

## Détails de calibration

### Pluie (calibration AO)
- Capturer SEC (vers 4095) et MOUILLÉ (vers 0) depuis `/config`.
- Interpolation linéaire : 0% = mouillé, 100% = sec (valeurs persistées en NVS).

### Ensoleillement (BH1750)
- Calibrer `lux_min` (0%) et `lux_max` (100%) depuis `/config` (ou capture live).
- Le dashboard colore le % en fonction des seuils configurés (vert/orange/rouge).

## Anémomètre
- Spéc. : 1 tour = 2 impulsions ; m/s = Hz × 0,34
- Dans le code : `ANEMO_PULSES_PER_ROTATION = 2`, `ANEMO_MPS_PER_ROTATION = 0.68`

## Seuils de couleur (UI)
- Pluie : vert ≥ `rain.green_min`, orange `[rain.orange_min, green_min)`, rouge `< rain.orange_min`
- Ensoleillement : vert ≥ `light.green_min`, orange `[light.orange_min, green_min)`, rouge `< light.orange_min`

## Dépannage
- BME280/BH1750 « NOT FOUND » : vérifier bus, adresses, alimentation
- Anémomètre à 0 pps : vérifier câblage + pull-up
- LM393 : utiliser la calibration AO ; la sortie DO n’est pas utilisée

