Configuration — Seuils et Calibration

Cette page resume la configuration exposee par l’API et l’UI integrée.

Pages
- `/config` — configuration interactive (pluie + ensoleillement)
- `/advanced` — vue avancee (lecture seule des mesures detaillees)

API
- GET `/api/config`
  {
    "rain": { "green_min": 70, "orange_min": 30, "cal": { "dry_raw": 3500, "wet_raw": 800 } },
    "light": { "green_min": 70, "orange_min": 30, "cal": { "lux_min": 0, "lux_max": 50000 } }
  }
- POST `/api/config` (application/x-www-form-urlencoded)
  - Pluie
    - `rain_green_min` (0..100)
    - `rain_orange_min` (0..100, force <= green)
    - `rain_dry_raw` (0..4095)
    - `rain_wet_raw` (0..4095)
    - `rain_capture` = `dry` | `wet` (capture la valeur ADC moyenne courante)
  - Ensoleillement
    - `light_green_min` (0..100)
    - `light_orange_min` (0..100, force <= green)
    - `lux_min` (>=0)
    - `lux_max` (>0)
    - `lux_capture` = `min` | `max` (capture la valeur lux courante)

Notes
- Les parametres sont stockes en NVS (Preferences) et restaures au boot.
- `lm393_percent` = 0% mouille, 100% sec; calcule par interpolation lineaire entre `wet_raw` et `dry_raw`.
- `light_percent` calcule par interpolation lineaire entre `lux_min` (0%) et `lux_max` (100%).
