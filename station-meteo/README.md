ESP32 Station Meteo — Interface Web

Ce projet fournit un sketch Arduino pour ESP32 avec une interface web (serveur embarque) pour visualiser:
- BME280 (I2C): temperature, humidite, pression
- BH1750 (I2C): luminosite (lux)
- Anemometre a reed: vitesse du vent
- LM393 (pluie): lecture analogique AO uniquement

Demarrage rapide
- Ouvrir `station-meteo.ino` dans l’IDE Arduino
- Renseigner `WIFI_SSID` et `WIFI_PASSWORD`
- Ajuster les pins si besoin puis televerser
- Acceder a `http://<ip_esp32>/` (ou AP `ESP32-Station` si STA echoue)

Librairies requises
- Adafruit BME280 Library
- Adafruit Unified Sensor
- BH1750

Cablage (defaut)
- I2C BME280+BH1750: SDA=21, SCL=22, 3V3, GND
- Anemometre: un fil -> GPIO `ANEMOMETER_PIN` (27), autre -> GND, pull‑up active
- LM393 pluie (AO): AO -> GPIO ADC (33 par defaut), 3V3, GND

Pages Web
- `/` Dashboard simple (valeurs principales, emojis vent/pluie, ensoleillement % colore)
- `/advanced` Vue avancee (grille complete: lux, % ensoleillement, vent pps/rpm, pluie %/V/ADC)
- `/config` Configuration (seuils pluie/ensoleillement + calibration pluie et lux)

Mise en route (calibration)

1) Flasher et se connecter
   - Ouvrir `station-meteo.ino`, renseigner `WIFI_SSID`/`WIFI_PASSWORD`, televerser.
   - Acceder au device: `http://<ip_esp32>/` (ou AP `ESP32-Station`).

2) Ouvrir la page `/config` (lien en haut du dashboard)

3) Pluie (LM393 AO)
   - Assurer que la plaque est bien seche → bouton « Capturer SEC maintenant ».
   - Mouiller franchement la plaque → bouton « Capturer MOUILLE maintenant ».
   - Ajuster au besoin les seuils couleur (orange/vert) pour l’affichage.

4) Ensoleillement (BH1750)
   - Placer le capteur dans l’obscurite voulue → « Capturer MIN maintenant » (0%).
   - Placer en pleine lumiere (max local) → « Capturer MAX maintenant » (100%).
   - Ajuster au besoin les seuils couleur (orange/vert) pour le %.

5) Verifier
   - Dashboard `/`: vent avec libelle/emoji, pluie « Sec/Pluvieux » + emoji, ensoleillement % colore.
   - Page `/advanced`: toutes les valeurs detaillees pour controle.

API
- GET `/api/sensors` -> mesures JSON:
  - `temperature_c`, `humidity_pct`, `pressure_hpa`
  - `lux` et `light_percent` (si `lux_min`/`lux_max` calibres)
  - `wind`: `pulses`, `pulses_per_sec`, `rpm`, `speed_mps`, `speed_kmh`
  - `lm393_analog`, `lm393_voltage_v`, `lm393_percent`
- GET `/api/config` ->
  {
    "rain": { "green_min": 70, "orange_min": 30, "cal": { "dry_raw": 3500, "wet_raw": 800 } },
    "light": { "green_min": 70, "orange_min": 30, "cal": { "lux_min": 0, "lux_max": 50000 } }
  }
- POST `/api/config` (application/x-www-form-urlencoded)
  - Pluie: `rain_green_min`, `rain_orange_min`, `rain_dry_raw`, `rain_wet_raw`, `rain_capture`=`dry|wet`
  - Ensoleillement: `light_green_min`, `light_orange_min`, `lux_min`, `lux_max`, `lux_capture`=`min|max`

Pluie (calibration AO)
- Capturer SEC (vers 4095) et MOUILLE (vers 0) depuis `/config`
- Interpolation lineaire: 0% = mouille, 100% = sec (valeurs persistees en NVS)

Ensoleillement (BH1750)
- Calibrer `lux_min` (0%) et `lux_max` (100%) depuis `/config` (ou capture live)
- Le dashboard colore le % en fonction des seuils configures (vert/orange/rouge)

Anemometre
- Spec: 1 tour = 2 impulsions; m/s = Hz x 0.34
- Dans le code: `ANEMO_PULSES_PER_ROTATION=2`, `ANEMO_MPS_PER_ROTATION=0.68`

Seuils couleur (UI)
- Pluie: vert >= `rain.green_min`, orange `[rain.orange_min, green_min)`, rouge `< rain.orange_min`
- Ensoleillement: vert >= `light.green_min`, orange `[light.orange_min, green_min)`, rouge `< light.orange_min`

Depannage
- BME280/BH1750 "NOT FOUND": verifier bus, adresses, alim
- Anemo a 0 pps: cablage + pull‑up
- LM393: utiliser la calibration AO; la sortie DO n’est pas utilisee
