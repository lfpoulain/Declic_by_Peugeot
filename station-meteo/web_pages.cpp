#include "web_pages.h"

const char MINI_DASH_ASCII[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Station Meteo ESP32</title>
  <style>
    :root {
      --fond:#0b1220;
      --panel:#11203a;
      --bord:#1f2d46;
      --texte:#f8fafc;
      --muted:#94a3b8;
      --accent:#38bdf8;
      --ok:#10b981;
      --warn:#f59e0b;
      --err:#ef4444;
    }
    * { box-sizing:border-box; margin:0; padding:0; }
    body {
      min-height:100vh;
      background:linear-gradient(180deg,#0b1220,#060b16);
      color:var(--texte);
      font:15px/1.5 "Segoe UI",sans-serif;
      display:flex;
      flex-direction:column;
    }
    header {
      padding: 14px 16px;
      border-bottom: 1px solid var(--bord);
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      flex-wrap: wrap;
    }
    @media (min-width: 768px) {
      header {
        padding: 18px 24px;
        gap: 20px;
        flex-wrap: nowrap;
      }
    }
    .brand { display: flex; align-items: center; gap: 10px; }
    .dot { width:10px; height:10px; border-radius:50%; background:var(--accent); box-shadow:0 0 10px var(--accent); }
    nav {
      display: flex;
      gap: 8px;
      flex-wrap: wrap;
    }
    @media (min-width: 768px) {
      nav { gap: 12px; flex-wrap: nowrap; }
    }
    .nav-link {
      padding: 6px 12px;
      border-radius: 999px;
      border: 1px solid transparent;
      color: var(--muted);
      text-decoration: none;
      font-size: 13px;
      white-space: nowrap;
    }
    @media (min-width: 768px) {
      .nav-link {
        padding: 6px 14px;
        font-size: 14px;
      }
    }
    .nav-link:hover { color:var(--texte); border-color:rgba(148,163,184,0.4); }
    .nav-link.active { background:rgba(56,189,248,0.16); border-color:rgba(56,189,248,0.35); color:var(--texte); }
    main {
      flex: 1;
      padding: 20px 24px 36px;
      max-width: 1200px;
      margin: 0 auto;
      display: flex;
      flex-direction: column;
      gap: 24px;
    }
    @media (min-width: 768px) {
      main { padding: 28px 40px 36px; }
    }
    @media (min-width: 1024px) {
      main { padding: 28px 60px 36px; }
    }
    @media (min-width: 1024px) {
      main { max-width: 1400px; }
    }
    .status {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-bottom: 8px;
    }
    @media (max-width: 640px) {
      .status { flex-direction: column; }
      .status-pill { justify-content: space-between; }
    }
    .status-pill {
      padding:8px 14px;
      border-radius:999px;
      background:rgba(12,22,40,0.8);
      border:1px solid var(--bord);
      color:var(--muted);
      font-size:13px;
      display:flex;
      align-items:center;
      gap:8px;
    }
    .status-pill strong { color:var(--texte); font-weight:600; }
    .badge { padding:2px 8px; border-radius:999px; font-size:12px; border:1px solid transparent; }
    .badge.ok { border-color:rgba(16,185,129,0.5); background:rgba(16,185,129,0.15); }
    .badge.warn { border-color:rgba(245,158,11,0.5); background:rgba(245,158,11,0.15); }
    .badge.err { border-color:rgba(239,68,68,0.5); background:rgba(239,68,68,0.15); }
    .grid {
      display: grid;
      gap: 20px;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      align-items: start;
    }
    .grid.compact { grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); }
    .grid.sensors { grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); }
    @media (min-width: 768px) {
      .grid.main { grid-template-columns: repeat(4, 1fr); }
      .grid.environmental { grid-template-columns: repeat(3, 1fr); }
      .grid.weather { grid-template-columns: repeat(2, 1fr); }
    }
    .carte {
      background: rgba(17,32,58,0.92);
      border: 1px solid var(--bord);
      border-radius: 16px;
      padding: 20px;
      display: flex;
      flex-direction: column;
      gap: 10px;
      transition: all 0.2s ease;
      min-height: 120px;
    }
    .carte.primary { border-color: rgba(56,189,248,0.4); box-shadow: 0 0 20px rgba(56,189,248,0.1); }
    .carte.environmental { border-color: rgba(16,185,129,0.3); }
    .carte.weather { border-color: rgba(245,158,11,0.3); }
    .carte.sensor { border-color: rgba(168,85,247,0.3); }
    .carte:hover { border-color:rgba(56,189,248,0.35); }
    .label { font-size:12px; color:var(--muted); text-transform:uppercase; letter-spacing:0.2px; }
    .valeur { font-size:34px; font-weight:600; }
    .unite { font-size:14px; color:var(--muted); }
    .commentaire { font-size:12px; color:var(--muted); margin-top:auto; }
    footer { text-align:center; color:var(--muted); font-size:12px; padding:18px; border-top:1px solid var(--bord); }
  </style>
  <script>
    function parseConfigText(txt){
      if(!txt) return null;
      const s = String(txt).trim();
      try { return JSON.parse(s); } catch(_){ /* fallback */ }
      const g = Array.from(s.matchAll(/"green_min"\s*:\s*([0-9]+)/g)).map(m=>+m[1]);
      const o = Array.from(s.matchAll(/"orange_min"\s*:\s*([0-9]+)/g)).map(m=>+m[1]);
      const dry = (s.match(/"dry_raw"\s*:\s*([0-9]+)/)||[])[1];
      const wet = (s.match(/"wet_raw"\s*:\s*([0-9]+)/)||[])[1];
      const lmin = (s.match(/"lux_min"\s*:\s*([0-9.]+)/)||[])[1];
      const lmax = (s.match(/"lux_max"\s*:\s*([0-9.]+)/)||[])[1];
      const rain = { green_min: g[0], orange_min: o[0], cal: { dry_raw: dry?+dry:undefined, wet_raw: wet?+wet:undefined } };
      const light = { green_min: g[1], orange_min: o[1], cal: { lux_min: lmin?+lmin:undefined, lux_max: lmax?+lmax:undefined } };
      return { rain, light };
    }

    const PLACEHOLDER = '--';
    let cfg = null;
    let lastUpdate = 0;
    let timerId = 0;

    function setNavActive() {
      const current = document.location.pathname;
      document.querySelectorAll('.nav-link').forEach(link => {
        if (link.getAttribute('href') === current) link.classList.add('active');
      });
    }

    function setValue(id, value, unit) {
      const v = document.getElementById(id);
      const u = document.getElementById(id + '-unit');
      if (!v) return;
      if (value === null || value === undefined || Number.isNaN(value)) {
        v.textContent = PLACEHOLDER;
        if (u) u.textContent = '';
        return;
      }
      const fixed = Math.abs(value) >= 100 ? Number(value).toFixed(0) : Number(value).toFixed(1);
      v.textContent = fixed;
      if (u) u.textContent = unit || '';
    }

    function setBadge(id, text, tone) {
      const badge = document.getElementById(id);
      if (!badge) return;
      badge.textContent = text;
      badge.className = 'badge ' + tone;
    }


    function ventCategorie(mps) {
      if (mps === null || Number.isNaN(mps)) return ['inconnu', 'warn'];
      if (mps < 0.5) return ['calme', 'ok'];
      if (mps < 2) return ['petite brise', 'ok'];
      if (mps < 5) return ['modere', 'ok'];
      if (mps < 10) return ['soutenu', 'warn'];
      if (mps < 20) return ['fort', 'warn'];
      return ['tres fort', 'err'];
    }

    function pluieCategorie(pct, orange, vert) {
      if (pct === null || Number.isNaN(pct)) return ['inconnu', 'warn'];
      if (pct >= vert) return ['sec', 'ok'];
      if (pct >= orange) return ['humide', 'warn'];
      return ['pluvieux', 'err'];
    }

    async function chargerConfig() {
      try {
        const res = await fetch('/api/config', { cache: 'no-store' });
        if (!res.ok) throw new Error('http ' + res.status);
        cfg = await res.json();
      } catch (err) {
        console.warn('config', err);
      }
    }

    async function chargerCapteurs() {
      try {
        const res = await fetch('/api/sensors', { cache: 'no-store' });
        if (!res.ok) throw new Error('http ' + res.status);
        const data = await res.json();
        lastUpdate = Date.now();

        setValue('temp', data.temperature_c, 'C');
        setValue('hum', data.humidity_pct, '%');
        setValue('pres', data.pressure_hpa, 'hPa');

        if (data.light_percent != null) {
          setValue('lum', data.light_percent, '%');
          const lux = data.lux == null || Number.isNaN(data.lux) ? 'n/a' : Number(data.lux).toFixed(0) + ' lx';
          const note = document.getElementById('lum-note'); if (note) note.textContent = 'Lux: ' + lux;
        } else {
          setValue('lum', data.lux, 'lx');
          const note = document.getElementById('lum-note'); if (note) note.textContent = 'Calibrer min/max dans Config';
        }

        setValue('vent-ms', data.wind.speed_mps, 'm/s');
        setValue('vent-kmh', data.wind.speed_kmh, 'km/h');
        setValue('pluie', data.lm393_percent, '%');
        setValue('pluie-v', data.lm393_voltage_v, 'V');


        const orange = cfg && cfg.rain ? cfg.rain.orange_min : 30;
        const vert = cfg && cfg.rain ? cfg.rain.green_min : 70;

        const [ventLabel, ventTone] = ventCategorie(data.wind.speed_mps);
        setBadge('vent-badge', ventLabel, ventTone);

        const [pluieLabel, pluieTone] = pluieCategorie(data.lm393_percent, orange, vert);
        setBadge('pluie-badge', pluieLabel, pluieTone);
      } catch (err) {
        console.warn('capteurs', err);
      }
    }

    async function init() {
      setNavActive();
      await chargerConfig();
      await chargerCapteurs();
      if (timerId) clearInterval(timerId);
      timerId = setInterval(chargerCapteurs, 2000);
    }

    window.addEventListener('load', init);
  </script>
</head>
<body>
  <header>
    <div class="brand">
      <span class="dot"></span>
      <div>
        <div style="font-weight:600;">Station Meteo</div>
        <div style="font-size:12px;color:var(--muted);">ESP32</div>
      </div>
    </div>
    <nav>
      <a class="nav-link" href="/">Tableau de bord</a>
      <a class="nav-link" href="/advanced">Avance</a>
      <a class="nav-link" href="/config">Configuration</a>
    </nav>
  </header>
  <main>
    <div class="status">
      <div class="status-pill"><strong>Vent</strong><span id="vent-badge" class="badge warn">--</span></div>
      <div class="status-pill"><strong>Pluie</strong><span id="pluie-badge" class="badge warn">--</span></div>
    </div>
    <!-- Conditions environnementales -->
    <h2 style="margin-top:0;margin-bottom:16px;color:var(--texte);font-size:18px;">🌡️ Conditions Environnementales</h2>
    <div class="grid environmental">
      <div class="carte environmental primary"><div class="label">Temperature</div><div class="valeur" id="temp">--</div><div class="unite" id="temp-unit"></div><div class="commentaire">Zone de confort 18-26°C</div></div>
      <div class="carte environmental"><div class="label">Humidite</div><div class="valeur" id="hum">--</div><div class="unite" id="hum-unit"></div><div class="commentaire">Cible interieur 40-60%</div></div>
      <div class="carte environmental"><div class="label">Pression</div><div class="valeur" id="pres">--</div><div class="unite" id="pres-unit"></div><div class="commentaire">Reference mer 1013 hPa</div></div>
    </div>

    <!-- Conditions meteorologiques -->
    <h2 style="margin-top:24px;margin-bottom:16px;color:var(--texte);font-size:18px;">🌤️ Conditions Meteorologiques</h2>
    <div class="grid weather">
      <div class="carte weather primary"><div class="label">Vent m/s</div><div class="valeur" id="vent-ms">--</div><div class="unite" id="vent-ms-unit"></div><div class="commentaire">Vitesse scientifique</div></div>
      <div class="carte weather"><div class="label">Vent km/h</div><div class="valeur" id="vent-kmh">--</div><div class="unite" id="vent-kmh-unit"></div><div class="commentaire">Vitesse quotidienne</div></div>
    </div>

    <!-- Capteurs specialises -->
    <h2 style="margin-top:24px;margin-bottom:16px;color:var(--texte);font-size:18px;">🔬 Capteurs Specialises</h2>
    <div class="grid sensors">
      <div class="carte sensor"><div class="label">Luminosite</div><div class="valeur" id="lum">--</div><div class="unite" id="lum-unit"></div><div class="commentaire" id="lum-note">En attente calibration</div></div>
      <div class="carte sensor"><div class="label">Pluie %</div><div class="valeur" id="pluie">--</div><div class="unite" id="pluie-unit"></div><div class="commentaire">100% = sec, 0% = trempe</div></div>
    </div>
  </main>
  <footer>Actualisation toutes les 2 s - servie par l'ESP32</footer>
</body>
</html>
)HTML";

const char CONFIG_ASCII_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Station Meteo - Configuration</title>
  <style>
    :root{--fond:#0b1220;--panel:#11203a;--bord:#1f2d46;--texte:#f8fafc;--muted:#94a3b8;--accent:#38bdf8;}
    body{margin:0;background:var(--fond);color:var(--texte);font:15px/1.5 "Segoe UI",sans-serif;display:flex;flex-direction:column;min-height:100vh}
    header {
      padding: 14px 16px;
      border-bottom: 1px solid var(--bord);
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      flex-wrap: wrap;
    }
    @media (min-width: 768px) {
      header {
        padding: 18px 24px;
        gap: 20px;
        flex-wrap: nowrap;
      }
    }
    .brand { display: flex; align-items: center; gap: 10px; }
    .dot { width: 10px; height: 10px; border-radius: 50%; background: var(--accent); box-shadow: 0 0 10px var(--accent); }
    nav {
      display: flex;
      gap: 8px;
      flex-wrap: wrap;
    }
    @media (min-width: 768px) {
      nav { gap: 12px; flex-wrap: nowrap; }
    }
    .nav-link {
      padding: 6px 12px;
      border-radius: 999px;
      border: 1px solid transparent;
      color: var(--muted);
      text-decoration: none;
      font-size: 13px;
      white-space: nowrap;
    }
    @media (min-width: 768px) {
      .nav-link {
        padding: 6px 14px;
        font-size: 14px;
      }
    }
    .nav-link:hover { color: var(--texte); border-color: rgba(148,163,184,0.4); }
    .nav-link.active { background: rgba(56,189,248,0.16); border-color: rgba(56,189,248,0.35); color: var(--texte); }
    main {\n      flex: 1;\n      padding: 20px 24px 34px;\n      max-width: 1200px;\n      margin: 0 auto;\n      display: flex;\n      flex-direction: column;\n      gap: 20px;\n    }\n    @media (min-width: 768px) {\n      main { padding: 26px 40px 34px; }\n    }\n    @media (min-width: 1024px) {\n      main { padding: 26px 60px 34px; }\n    }
    .panel {
      background: rgba(17,32,58,0.92);
      border: 1px solid var(--bord);
      border-radius: 16px;
      padding: 20px;
      display: flex;
      flex-direction: column;
      gap: 16px;
      margin-bottom: 8px;
    }
    .panel.config { border-color: rgba(56,189,248,0.3); }
    .panel.calibration { border-color: rgba(245,158,11,0.3); }
    h2{margin:0;font-size:16px}
    p{margin:0;color:var(--muted);font-size:13px}
    .preview {
      display: grid;
      gap: 14px;
      grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
    }
    @media (min-width: 768px) {
      .preview { grid-template-columns: repeat(3, 1fr); }
    }
    @media (min-width: 1200px) {
      .preview { grid-template-columns: repeat(6, 1fr); }
    }
    .preview-item{background:rgba(11,19,32,0.9);border:1px solid var(--bord);border-radius:12px;padding:14px;display:flex;flex-direction:column;gap:6px}
    .preview-item .label{font-size:12px;color:var(--muted);text-transform:uppercase}
    .preview-item .val{font-size:20px;font-weight:600}
    .preview-item .unit{font-size:13px;color:var(--muted)}
    form{display:grid;gap:12px}
    .control{display:flex;flex-direction:column;gap:6px}
    label{font-size:13px;color:var(--muted)}
    input{padding:10px 12px;border-radius:10px;border:1px solid var(--bord);background:rgba(9,16,30,0.85);color:var(--texte)}
    .saved{font-size:11px;color:var(--muted)}
    button{padding:10px 14px;border-radius:10px;border:1px solid rgba(56,189,248,0.35);background:rgba(56,189,248,0.15);color:var(--texte);cursor:pointer}
    .button-row{display:flex;gap:10px;flex-wrap:wrap}
    .status{font-size:12px;padding:6px 10px;border-radius:8px;border:1px solid transparent;color:var(--muted);min-height:26px}
    .status.ok{color:var(--texte);border-color:rgba(16,185,129,0.5);background:rgba(16,185,129,0.2)}
    .status.err{color:var(--texte);border-color:rgba(239,68,68,0.5);background:rgba(239,68,68,0.2)}
    main{flex:1;padding:20px 24px 34px;max-width:1200px;margin:0 auto;display:flex;flex-direction:column;gap:20px}
    @media (min-width:768px){main{padding:26px 40px 34px}}
    @media (min-width:1024px){main{padding:26px 60px 34px}}
  </style>
  <script>
    function parseConfigText(txt){
      if(!txt) return null;
      const s = String(txt).trim();
      try { return JSON.parse(s); } catch(_){ /* fallback */ }
      const g = Array.from(s.matchAll(/"green_min"\s*:\s*([0-9]+)/g)).map(m=>+m[1]);
      const o = Array.from(s.matchAll(/"orange_min"\s*:\s*([0-9]+)/g)).map(m=>+m[1]);
      const dry = (s.match(/"dry_raw"\s*:\s*([0-9]+)/)||[])[1];
      const wet = (s.match(/"wet_raw"\s*:\s*([0-9]+)/)||[])[1];
      const lmin = (s.match(/"lux_min"\s*:\s*([0-9.]+)/)||[])[1];
      const lmax = (s.match(/"lux_max"\s*:\s*([0-9.]+)/)||[])[1];
      const rain = { green_min: g[0], orange_min: o[0], cal: { dry_raw: dry?+dry:undefined, wet_raw: wet?+wet:undefined } };
      const light = { green_min: g[1], orange_min: o[1], cal: { lux_min: lmin?+lmin:undefined, lux_max: lmax?+lmax:undefined } };
      return { rain, light };
    }

    const PLACEHOLDER='--';
    let cfg=null;

    function setNavActive(){
      const current=document.location.pathname;
      document.querySelectorAll('.nav-link').forEach(link=>{
        if(link.getAttribute('href')===current) link.classList.add('active');
      });
    }

    function setInput(id,val){
      const el=document.getElementById(id);
      if(!el) return;
      el.value=(val===undefined||val===null)?'':String(val);
    }

    function setStored(id,val,decimals){
      const span=document.getElementById(id);
      if(!span) return;
      if(val===undefined||val===null||Number.isNaN(val)){span.textContent='--';return;}
      if(decimals!==undefined) span.textContent=Number(val).toFixed(decimals);
      else span.textContent=String(val);
    }

    function setStatus(id,text,tone){const el=document.getElementById(id); if(!el) return; el.textContent=text; el.className='status '+tone;}

    function setPreview(id,value,unit){
      const wrap=document.getElementById(id);
      if(!wrap) return;
      const val=wrap.querySelector('.val');
      const unitSpan=wrap.querySelector('.unit');
      if(value==null||Number.isNaN(value)){
        if(val) val.textContent=PLACEHOLDER;
        if(unitSpan) unitSpan.textContent='';
        return;
      }
      if(val) val.textContent=Number(value).toFixed(1);
      if(unitSpan) unitSpan.textContent=unit||'';
    }

    function appliquerConfig(data){
      cfg=data||{};
      const rain=(cfg.rain)||{};
      const rainCal=rain.cal||{};
      const light=(cfg.light)||{};
      const lightCal=light.cal||{};
      setInput('rain-vert', rain.green_min);
      setStored('rain-vert-saved', rain.green_min);
      setInput('rain-orange', rain.orange_min);
      setStored('rain-orange-saved', rain.orange_min);
      setInput('rain-sec', rainCal.dry_raw);
      setStored('rain-sec-saved', rainCal.dry_raw);
      setInput('rain-humide', rainCal.wet_raw);
      setStored('rain-humide-saved', rainCal.wet_raw);
      setInput('lum-vert', light.green_min);
      setStored('lum-vert-saved', light.green_min);
      setInput('lum-orange', light.orange_min);
      setStored('lum-orange-saved', light.orange_min);
      setInput('lux-min', lightCal.lux_min);
      setStored('lux-min-saved', lightCal.lux_min, 1);
      setInput('lux-max', lightCal.lux_max);
      setStored('lux-max-saved', lightCal.lux_max, 1);
    }

    async function chargerConfig(){
      try{
        const res=await fetch('/api/config',{cache:'no-store'});
        if(!res.ok) throw new Error('http '+res.status);
        const raw=await res.text();
        const data = parseConfigText(raw);
        if(data){ appliquerConfig(data); } else { console.warn('Config parse failed'); }
        await chargerApercu();
      }catch(err){
        console.warn('chargerConfig',err);
        setStatus('status-main','Erreur config','err');
      }
    }

    async function onSeuilPluie(e){
      e.preventDefault();
      const vert=document.getElementById('rain-vert').value;
      const orange=document.getElementById('rain-orange').value;
      try{
        await postForm({rain_green_min:String(vert),rain_orange_min:String(orange)});
        setStatus('status-rain','Enregistre','ok');
      }catch(err){
        console.warn(err);
        setStatus('status-rain','Echec','err');
      }
    }
    async function onCalPluie(e){
      e.preventDefault();
      const sec=document.getElementById('rain-sec').value;
      const humide=document.getElementById('rain-humide').value;
      try{
        await postForm({rain_dry_raw:String(sec),rain_wet_raw:String(humide)});
        setStatus('status-cal','Enregistre','ok');
      }catch(err){
        console.warn(err);
        setStatus('status-cal','Echec','err');
      }
    }
    async function onSeuilLumiere(e){
      e.preventDefault();
      const vert=document.getElementById('lum-vert').value;
      const orange=document.getElementById('lum-orange').value;
      try{
        await postForm({light_green_min:String(vert),light_orange_min:String(orange)});
        setStatus('status-lux-seuil','Enregistre','ok');
      }catch(err){
        console.warn(err);
        setStatus('status-lux-seuil','Echec','err');
      }
    }
    async function onPlageLux(e){
      e.preventDefault();
      const min=document.getElementById('lux-min').value;
      const max=document.getElementById('lux-max').value;
      try{
        await postForm({lux_min:String(min),lux_max:String(max)});
        setStatus('status-lux','Enregistre','ok');
      }catch(err){
        console.warn(err);
        setStatus('status-lux','Echec','err');
      }
    }

    async function postForm(data) {
      const form = new FormData();
      for (const [key, value] of Object.entries(data)) {
        form.append(key, value);
      }
      const res = await fetch('/api/config', {
        method: 'POST',
        body: form,
        cache: 'no-store'
      });
      if (!res.ok) throw new Error('http ' + res.status);
      const result = await res.text();
      const parsed = parseConfigText(result);
      if (parsed) appliquerConfig(parsed);
      await chargerApercu();
    }

    async function chargerApercu() {
      try {
        const res = await fetch('/api/sensors', { cache: 'no-store' });
        if (!res.ok) throw new Error('http ' + res.status);
        const data = await res.json();
        setPreview('prev-temp', data.temperature_c, 'C');
        setPreview('prev-hum', data.humidity_pct, '%');
        setPreview('prev-pres', data.pressure_hpa, 'hPa');
        setPreview('prev-lux', data.lux, 'lx');
        const pluieEl = document.getElementById('prev-pluie');
        const adcEl = document.getElementById('prev-pluie-adc');
        if (pluieEl) pluieEl.textContent = data.lm393_percent == null ? PLACEHOLDER : Number(data.lm393_percent).toFixed(1);
        if (adcEl) adcEl.textContent = data.lm393_analog == null ? PLACEHOLDER : String(data.lm393_analog);
      } catch (err) {
        console.warn('chargerApercu', err);
      }
    }

    async function init(){
      setNavActive();
      await chargerConfig();
      await chargerApercu();
      document.getElementById('form-rain').addEventListener('submit',onSeuilPluie);
      document.getElementById('form-rain-cal').addEventListener('submit',onCalPluie);
      document.getElementById('form-lux-seuil').addEventListener('submit',onSeuilLumiere);
      document.getElementById('form-lux-plage').addEventListener('submit',onPlageLux);
      setInterval(chargerApercu,4000);
    }

    window.addEventListener('load',init);
  </script>
</head>
<body>
  <header>
    <div class="brand">
      <span class="dot"></span>
      <div>
        <div style="font-weight:600;">Station Meteo</div>
        <div style="font-size:12px;color:#94a3b8;">Configuration</div>
      </div>
    </div>
    <nav>
      <a class="nav-link" href="/">Tableau de bord</a>
      <a class="nav-link" href="/advanced">Avance</a>
      <a class="nav-link" href="/config">Configuration</a>
    </nav>
  </header>
  <main>
    <section class="panel config">
      <h2>📊 Apercu en direct</h2>
      <p>Surveillez les valeurs pendant les reglages.</p>
      <div class="preview">
        <div class="preview-item" id="prev-temp"><span class="label">Temperature</span><span class="val">--</span><span class="unit"></span></div>
        <div class="preview-item" id="prev-hum"><span class="label">Humidite</span><span class="val">--</span><span class="unit"></span></div>
        <div class="preview-item" id="prev-pres"><span class="label">Pression</span><span class="val">--</span><span class="unit"></span></div>
        <div class="preview-item" id="prev-lux"><span class="label">Luminosite</span><span class="val">--</span><span class="unit"></span></div>
        <div class="preview-item"><span class="label">Pluie %</span><span class="val" id="prev-pluie">--</span><span class="unit"></span></div>
        <div class="preview-item"><span class="label">Pluie ADC</span><span class="val" id="prev-pluie-adc">--</span></div>
      </div>
    </section>
    <section class="panel config">
      <h2>🌧️ Seuils pluie (%)</h2>
      <form id="form-rain">
        <div class="control"><label for="rain-vert">Vert min (%)</label><input id="rain-vert" type="number" min="0" max="100" step="1" /><div class="saved">Enregistre : <span id="rain-vert-saved">--</span></div></div>
        <div class="control"><label for="rain-orange">Orange min (%)</label><input id="rain-orange" type="number" min="0" max="100" step="1" /><div class="saved">Enregistre : <span id="rain-orange-saved">--</span></div></div>
        <button type="submit">Enregistrer</button>
        <div id="status-rain" class="status"></div>
      </form>
    </section>
    <section class="panel calibration">
      <h2>⚙️ Calibration pluie (ADC)</h2>
      <form id="form-rain-cal">
        <div class="control"><label for="rain-sec">Valeur sec</label><input id="rain-sec" type="number" min="0" max="4095" step="1" /><div class="saved">Enregistre : <span id="rain-sec-saved">--</span></div></div>
        <div class="control"><label for="rain-humide">Valeur humide</label><input id="rain-humide" type="number" min="0" max="4095" step="1" /><div class="saved">Enregistre : <span id="rain-humide-saved">--</span></div></div>
        <button type="submit">Enregistrer</button>
        <div id="status-cal" class="status"></div>
      </form>
    </section>
    <section class="panel config">
      <h2>💡 Seuils lumiere (%)</h2>
      <form id="form-lux-seuil">
        <div class="control"><label for="lum-vert">Vert min (%)</label><input id="lum-vert" type="number" min="0" max="100" step="1" /><div class="saved">Enregistre : <span id="lum-vert-saved">--</span></div></div>
        <div class="control"><label for="lum-orange">Orange min (%)</label><input id="lum-orange" type="number" min="0" max="100" step="1" /><div class="saved">Enregistre : <span id="lum-orange-saved">--</span></div></div>
        <button type="submit">Enregistrer</button>
        <div id="status-lux-seuil" class="status"></div>
      </form>
    </section>
    <section class="panel calibration">
      <h2>⚙️ Plage lux -> %</h2>
      <form id="form-lux-plage">
        <div class="control"><label for="lux-min">Lux min (0%)</label><input id="lux-min" type="number" min="0" step="1" /><div class="saved">Enregistre : <span id="lux-min-saved">--</span></div></div>
        <div class="control"><label for="lux-max">Lux max (100%)</label><input id="lux-max" type="number" min="1" step="1" /><div class="saved">Enregistre : <span id="lux-max-saved">--</span></div></div>
        <button type="submit">Enregistrer</button>
        <div id="status-lux" class="status"></div>
      </form>
    </section>
  </main>
</body>
</html>
)HTML";

const char ADV_ASCII_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Station Meteo - Avance</title>
  <style>
    body{margin:0;background:#0b1220;color:#f8fafc;font:15px/1.5 "Segoe UI",sans-serif;display:flex;flex-direction:column;min-height:100vh}
    header {
      padding: 14px 16px;
      border-bottom: 1px solid #1f2d46;
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      flex-wrap: wrap;
    }
    @media (min-width: 768px) {
      header {
        padding: 18px 24px;
        gap: 20px;
        flex-wrap: nowrap;
      }
    }
    .brand { display: flex; align-items: center; gap: 10px; }
    .dot { width: 10px; height: 10px; border-radius: 50%; background: #38bdf8; box-shadow: 0 0 10px #38bdf8; }
    nav {\n      display: flex;\n      gap: 8px;\n      flex-wrap: wrap;\n    }\n    @media (min-width: 768px) {\n      nav { gap: 12px; flex-wrap: nowrap; }\n    }
    .nav-link {
      padding: 6px 12px;
      border-radius: 999px;
      border: 1px solid transparent;
      color: #94a3b8;
      text-decoration: none;
      font-size: 13px;
      white-space: nowrap;
    }
    @media (min-width: 768px) {
      .nav-link {
        padding: 6px 14px;
        font-size: 14px;
      }
    }
    .nav-link:hover { color: #f8fafc; border-color: rgba(148,163,184,0.4); }
    .nav-link.active { background: rgba(56,189,248,0.16); border-color: rgba(56,189,248,0.35); color: #f8fafc; }
    main {\n      flex: 1;\n      padding: 20px 24px 34px;\n      max-width: 1200px;\n      margin: 0 auto;\n      display: flex;\n      flex-direction: column;\n      gap: 20px;\n    }\n    @media (min-width: 768px) {\n      main { padding: 26px 40px 34px; }\n    }\n    @media (min-width: 1024px) {\n      main { padding: 26px 60px 34px; }\n    }
    .status{display:flex;flex-wrap:wrap;gap:12px}
    .status-pill{padding:8px 14px;border-radius:999px;background:rgba(11,20,36,0.85);border:1px solid #1f2d46;color:#94a3b8;font-size:13px;display:flex;gap:8px;align-items:center}
    .status-pill strong{color:#f8fafc}
    .grid {
      display: grid;
      gap: 20px;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      align-items: start;
    }
    .grid.compact { grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); }
    .grid.sensors { grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); }
    @media (min-width: 768px) {
      .grid.main { grid-template-columns: repeat(4, 1fr); }
      .grid.environmental { grid-template-columns: repeat(3, 1fr); }
      .grid.weather { grid-template-columns: repeat(2, 1fr); }
    }
    .carte {
      background: rgba(17,32,58,0.92);
      border: 1px solid #1f2d46;
      border-radius: 16px;
      padding: 20px;
      display: flex;
      flex-direction: column;
      gap: 10px;
      transition: all 0.2s ease;
      min-height: 130px;
    }
    .carte:hover {
      border-color: rgba(56,189,248,0.4);
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(0,0,0,0.2);
    }
    .etiquette{font-size:12px;color:#94a3b8;text-transform:uppercase}
    .valeur{font-size:28px;font-weight:600}
    .unite{font-size:13px;color:#94a3b8}
    pre{background:rgba(9,16,30,0.9);border:1px solid #1f2d46;border-radius:16px;padding:18px;color:#94a3b8;font-size:13px;max-height:260px;overflow:auto}
    main{flex:1;padding:20px 24px 34px;max-width:1200px;margin:0 auto;display:flex;flex-direction:column;gap:20px}
    @media (min-width:768px){main{padding:26px 40px 34px}}
    @media (min-width:1024px){main{padding:26px 60px 34px}}
  </style>
  <script>
    function parseConfigText(txt){
      if(!txt) return null;
      const s = String(txt).trim();
      try { return JSON.parse(s); } catch(_){ /* fallback */ }
      const g = Array.from(s.matchAll(/"green_min"\s*:\s*([0-9]+)/g)).map(m=>+m[1]);
      const o = Array.from(s.matchAll(/"orange_min"\s*:\s*([0-9]+)/g)).map(m=>+m[1]);
      const dry = (s.match(/"dry_raw"\s*:\s*([0-9]+)/)||[])[1];
      const wet = (s.match(/"wet_raw"\s*:\s*([0-9]+)/)||[])[1];
      const lmin = (s.match(/"lux_min"\s*:\s*([0-9.]+)/)||[])[1];
      const lmax = (s.match(/"lux_max"\s*:\s*([0-9.]+)/)||[])[1];
      const rain = { green_min: g[0], orange_min: o[0], cal: { dry_raw: dry?+dry:undefined, wet_raw: wet?+wet:undefined } };
      const light = { green_min: g[1], orange_min: o[1], cal: { lux_min: lmin?+lmin:undefined, lux_max: lmax?+lmax:undefined } };
      return { rain, light };
    }

    const PLACEHOLDER='--';
    let lastUpdate=0;
    let timerId=0;

    function setNavActive(){
      const current=document.location.pathname;
      document.querySelectorAll('.nav-link').forEach(link=>{
        if(link.getAttribute('href')===current) link.classList.add('active');
      });
    }

    function setValue(id, val, unit) {
      const v = document.getElementById(id);
      const u = document.getElementById(id + '-unit');
      if (!v) return;
      if (val == null || Number.isNaN(val)) {
        v.textContent = PLACEHOLDER;
        if (u) u.textContent = '';
        return;
      }
      const fixed = Math.abs(val) >= 100 ? Number(val).toFixed(0) : Number(val).toFixed(2);
      v.textContent = fixed;
      if (u) u.textContent = unit || '';
    }

    async function charger() {
      try {
        const res = await fetch('/api/sensors', { cache: 'no-store' });
        if (!res.ok) throw new Error('http ' + res.status);
        const data = await res.json();
        lastUpdate = Date.now();

        setValue('temp', data.temperature_c, 'C');
        setValue('hum', data.humidity_pct, '%');
        setValue('pres', data.pressure_hpa, 'hPa');
        setValue('lux', data.lux, 'lx');
        setValue('lum', data.light_percent, '%');
        setValue('vent-pps', data.wind.pulses_per_sec, 'pps');
        setValue('vent-rpm', data.wind.rpm, 'rpm');
        setValue('vent-ms', data.wind.speed_mps, 'm/s');
        setValue('vent-kmh', data.wind.speed_kmh, 'km/h');
        setValue('pluie-pct', data.lm393_percent, '%');
        setValue('pluie-v', data.lm393_voltage_v, 'V');

        const adc = document.getElementById('pluie-adc');
        if (adc) adc.textContent = data.lm393_analog == null ? PLACEHOLDER : Number(data.lm393_analog).toFixed(0);

        const total = document.getElementById('total-imp');
        if (total) total.textContent = data.wind.pulses ?? '--';

        document.getElementById('bloc-json').textContent = JSON.stringify(data, null, 2);
      } catch (err) {
        console.warn('capteurs', err);
      }
    }

    async function init() {
      setNavActive();
      await charger();
      if (timerId) clearInterval(timerId);
      timerId = setInterval(charger, 2000);
    }

    window.addEventListener('load',init);
  </script>
</head>
<body>
  <header>
    <div class="brand">
      <span class="dot"></span>
      <div>
        <div style="font-weight:600;">Station Meteo</div>
        <div style="font-size:12px;color:#94a3b8;">Vue avancée</div>
      </div>
    </div>
    <nav>
      <a class="nav-link" href="/">Tableau de bord</a>
      <a class="nav-link" href="/advanced">Avance</a>
      <a class="nav-link" href="/config">Configuration</a>
    </nav>
  </header>
  <main>
    <!-- Capteurs environnementaux -->
    <div style="margin-bottom:24px;">
      <h2 style="margin:0 0 16px 0;color:#f8fafc;font-size:18px;">🌡️ Environnement</h2>
      <div class="grid environmental">
        <div class="carte" style="border-color:rgba(16,185,129,0.4);"><div class="etiquette">Temperature</div><div class="valeur" id="temp">--</div><div class="unite" id="temp-unit"></div></div>
        <div class="carte" style="border-color:rgba(16,185,129,0.3);"><div class="etiquette">Humidite</div><div class="valeur" id="hum">--</div><div class="unite" id="hum-unit"></div></div>
        <div class="carte" style="border-color:rgba(16,185,129,0.3);"><div class="etiquette">Pression</div><div class="valeur" id="pres">--</div><div class="unite" id="pres-unit"></div></div>
      </div>
    </div>

    <!-- Capteurs luminosite -->
    <div style="margin-bottom:24px;">
      <h2 style="margin:0 0 16px 0;color:#f8fafc;font-size:18px;">💡 Luminosite</h2>
      <div class="grid weather">
        <div class="carte" style="border-color:rgba(245,158,11,0.4);"><div class="etiquette">Lux brut</div><div class="valeur" id="lux">--</div><div class="unite" id="lux-unit"></div></div>
        <div class="carte" style="border-color:rgba(245,158,11,0.3);"><div class="etiquette">Lumiere %</div><div class="valeur" id="lum">--</div><div class="unite" id="lum-unit"></div></div>
      </div>
    </div>

    <!-- Capteurs vent -->
    <div style="margin-bottom:24px;">
      <h2 style="margin:0 0 16px 0;color:#f8fafc;font-size:18px;">🌬️ Vent</h2>
      <div class="grid main">
        <div class="carte" style="border-color:rgba(56,189,248,0.4);"><div class="etiquette">Total impulsions</div><div class="valeur" id="total-imp">--</div></div>
        <div class="carte" style="border-color:rgba(56,189,248,0.4);"><div class="etiquette">Impulsions/sec</div><div class="valeur" id="vent-pps">--</div><div class="unite" id="vent-pps-unit"></div></div>
        <div class="carte" style="border-color:rgba(56,189,248,0.3);"><div class="etiquette">Tours/min</div><div class="valeur" id="vent-rpm">--</div><div class="unite" id="vent-rpm-unit"></div></div>
        <div class="carte" style="border-color:rgba(56,189,248,0.4);"><div class="etiquette">Vitesse m/s</div><div class="valeur" id="vent-ms">--</div><div class="unite" id="vent-ms-unit"></div></div>
        <div class="carte" style="border-color:rgba(56,189,248,0.3);"><div class="etiquette">Vitesse km/h</div><div class="valeur" id="vent-kmh">--</div><div class="unite" id="vent-kmh-unit"></div></div>
      </div>
    </div>

    <!-- Capteurs pluie -->
    <div style="margin-bottom:24px;">
      <h2 style="margin:0 0 16px 0;color:#f8fafc;font-size:18px;">🌧️ Pluie</h2>
      <div class="grid environmental">
        <div class="carte" style="border-color:rgba(168,85,247,0.4);"><div class="etiquette">Humidite %</div><div class="valeur" id="pluie-pct">--</div><div class="unite" id="pluie-pct-unit"></div></div>
        <div class="carte" style="border-color:rgba(168,85,247,0.3);"><div class="etiquette">Tension V</div><div class="valeur" id="pluie-v">--</div><div class="unite" id="pluie-v-unit"></div></div>
        <div class="carte" style="border-color:rgba(168,85,247,0.3);"><div class="etiquette">Valeur ADC</div><div class="valeur" id="pluie-adc">--</div></div>
      </div>
    </div>
    <pre id="bloc-json">chargement...</pre>
  </main>
</body>
</html>
)HTML";






