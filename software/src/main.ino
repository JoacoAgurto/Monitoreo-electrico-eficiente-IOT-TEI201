
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>

const char* DEFAULT_SSID = "Monitor-ESP32";
const char* DEFAULT_PASS = "corriente123";
const char* CSV_PATH = "/corriente_conectores.csv";
const char* WIFI_PATH = "/wifi.cfg";

const int SD_CS = 5;
const unsigned long SAMPLE_INTERVAL = 1000;
const int SAMPLES = 128;
const float AMPS_PER_VOLT = 100.0f;

Adafruit_ADS1115 ads1, ads2;
WebServer server(80);

bool ads1OK = false;
bool ads2OK = false;
bool sdOK = false;
bool loggingPaused = false;

String wifiSSID = DEFAULT_SSID;
String wifiPass = DEFAULT_PASS;

float amps[4] = {0, 0, 0, 0};
bool available[4] = {false, false, false, false};

unsigned long lastSample = 0;
unsigned long records = 0;
unsigned long restartAt = 0;

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Monitor de corriente</title>
<style>
:root {
  color-scheme: dark;
  font-family: system-ui, Arial, sans-serif;
}
* { box-sizing: border-box; }
body {
  margin: 0;
  background: #0c1423;
  color: #eef4ff;
}
header {
  padding: 24px;
  background: #13233b;
  border-bottom: 1px solid #34465e;
}
header h1 { margin: 0 0 6px; }
header p { margin: 0; color: #acbdd0; }
main {
  max-width: 1100px;
  margin: auto;
  padding: 16px;
}
.status {
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
  margin-bottom: 16px;
}
.pill, .card {
  background: #17263e;
  border: 1px solid #34465e;
  border-radius: 14px;
}
.pill { padding: 9px 12px; }
.grid {
  display: grid;
  grid-template-columns: repeat(2,minmax(0,1fr));
  gap: 14px;
}
.card { padding: 18px; min-width: 0; }
.top {
  display: flex;
  justify-content: space-between;
  gap: 8px;
  align-items: center;
}
h2 { font-size: 20px; margin: 0 0 12px; }
.value {
  font-size: 38px;
  font-weight: 700;
  margin: 12px 0;
}
.unit, .muted {
  color: #acbdd0;
  font-size: 14px;
}
.chart { margin-top: 15px; }
.chart[hidden] { display: none; }
canvas {
  width: 100%;
  height: 180px;
  background: #0e1b2e;
  border-radius: 9px;
  border: 1px solid #34465e;
}
section.extra { margin-top: 15px; }
input[type=text], input[type=password] {
  width: 100%;
  padding: 11px;
  border-radius: 8px;
  background: #0c1728;
  border: 1px solid #536782;
  color: white;
  font-size: 16px;
}
label.field { display: block; margin: 12px 0; }
label.field span {
  display: block;
  margin-bottom: 6px;
}
button {
  background: #27618e;
  color: white;
  border: 1px solid #568ab1;
  border-radius: 9px;
  padding: 11px 15px;
  font-size: 15px;
  cursor: pointer;
}
button.danger {
  background: #8b3535;
  border-color: #c77878;
}
a { color: #9dd5ff; }
.actions {
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
  align-items: center;
}
.message {
  min-height: 1.4em;
  color: #ffd7a0;
}
@media(max-width:720px) {
  .grid { grid-template-columns: 1fr; }
  .value { font-size: 34px; }
}
</style>
</head>
<body>
<header>
  <h1>Monitor de corriente</h1>
  <p>Cuatro conectores independientes</p>
</header>
<main>
  <div class="status">
    <div class="pill">
      ESP32: <b id="connection">Conectando</b>
    </div>
    <div class="pill">
      microSD: <b id="sd">Consultando</b>
    </div>
    <div class="pill">
      Registros: <b id="records">0</b>
    </div>
  </div>

  <div id="cards" class="grid"></div>

  <section class="card extra">
    <h2>Historial CSV</h2>
    <p class="muted">
      Se registran los cuatro conectores
      aunque ocultes sus gráficos.
    </p>
    <div class="actions">
      <a href="/download">Descargar CSV</a>
      <button class="danger" id="reset">
        Reiniciar archivo CSV
      </button>
    </div>
    <p id="resetMsg" class="message"></p>
  </section>

  <section class="card extra">
    <h2>Configuración Wi-Fi</h2>
    <p class="muted">
      La red es creada por el ESP32, sin router.
      Los cambios se guardan en la microSD.
    </p>
    <form id="wifiForm">
      <label class="field">
        <span>Nombre de la red (SSID)</span>
        <input id="ssid" type="text"
          minlength="1" maxlength="32" required>
      </label>
      <label class="field">
        <span>Contraseña (mínimo 8 caracteres)</span>
        <input id="pass" type="password"
          minlength="8" maxlength="63" required>
      </label>
      <button type="submit">
        Guardar Wi-Fi y reiniciar red
      </button>
    </form>
    <p id="wifiMsg" class="message"></p>
  </section>
</main>

<script>
const names = [
  'Conector 1',
  'Conector 2',
  'Conector 3',
  'Conector 4'
];
const colors = [
  '#6bb7ff',
  '#56d7ac',
  '#ffbc70',
  '#c8a2ff'
];
const hist = [[],[],[],[]];
const visible = [true,true,true,true];
const maxPoints = 90;
const el = id => document.getElementById(id);

names.forEach((name,i) => {
  try {
    visible[i] =
      localStorage.getItem('visible'+i) !== 'false';
  } catch(e) {}

  const card = document.createElement('section');
  card.className = 'card';

  card.innerHTML = `
    <div class="top">
      <h2>${name}</h2>
      <label class="muted">
        <input type="checkbox" id="toggle${i}"
          ${visible[i] ? 'checked' : ''}>
        Gráfico
      </label>
    </div>
    <div class="value">
      <span id="amp${i}">--</span>
      <span class="unit">A RMS</span>
    </div>
    <div id="state${i}" class="muted">
      Esperando lectura
    </div>
    <div class="chart" id="chartbox${i}"
      ${visible[i] ? '' : 'hidden'}>
      <canvas id="canvas${i}"
        width="680" height="220"></canvas>
    </div>`;

  el('cards').appendChild(card);

  el('toggle'+i).addEventListener('change',e => {
    visible[i] = e.target.checked;
    el('chartbox'+i).hidden = !visible[i];

    try {
      localStorage.setItem(
        'visible'+i,
        String(visible[i])
      );
    } catch(err) {}

    if(visible[i]) draw(i);
  });
});

function draw(i) {
  if(!visible[i]) return;

  const c = el('canvas'+i);
  const ctx = c.getContext('2d');
  const w = c.width, h = c.height;
  const L = 50, R = 12, T = 15, B = 30;

  ctx.clearRect(0,0,w,h);

  const valid = hist[i].filter(Number.isFinite);
  const maximum = Math.max(.1,...valid)*1.15;

  ctx.font = '13px Arial';
  ctx.fillStyle = '#acbdd0';
  ctx.lineWidth = 1;

  for(let k=0;k<=4;k++) {
    const y = T+(h-T-B)*k/4;

    ctx.strokeStyle = '#354861';
    ctx.beginPath();
    ctx.moveTo(L,y);
    ctx.lineTo(w-R,y);
    ctx.stroke();

    ctx.fillText(
      (maximum*(1-k/4)).toFixed(maximum<1?2:1),
      5,y+4
    );
  }

  ctx.strokeStyle = colors[i];
  ctx.lineWidth = 2.5;
  ctx.beginPath();

  let started = false;

  hist[i].forEach((v,k) => {
    if(!Number.isFinite(v)) {
      started = false;
      return;
    }

    const x = L+(w-L-R)*k/(maxPoints-1);
    const y = h-B-(h-T-B)*
      Math.min(v,maximum)/maximum;

    if(!started) {
      ctx.moveTo(x,y);
      started = true;
    } else {
      ctx.lineTo(x,y);
    }
  });

  ctx.stroke();
}

let polling = false;

async function update() {
  if(polling) return;
  polling = true;

  try {
    const r = await fetch('/api/data',{
      cache:'no-store'
    });

    if(!r.ok) throw Error('HTTP '+r.status);

    const d = await r.json();

    el('connection').textContent = 'Conectado';
    el('sd').textContent =
      d.sd ? 'Disponible' : 'No disponible';
    el('records').textContent = d.records;

    d.channels.forEach((ch,i) => {
      el('amp'+i).textContent =
        ch.available ?
        Number(ch.amps).toFixed(3) : '--';

      el('state'+i).textContent =
        ch.available ?
        'Lectura actualizada' :
        'ADS1115 no disponible';

      hist[i].push(
        ch.available ? Number(ch.amps) : null
      );

      if(hist[i].length>maxPoints) {
        hist[i].shift();
      }

      draw(i);
    });

  } catch(e) {
    el('connection').textContent = 'Sin conexión';
  } finally {
    polling = false;
  }
}

async function loadWifi() {
  try {
    const r = await fetch('/api/wifi');
    const d = await r.json();

    el('ssid').value = d.ssid;
    el('pass').value = d.password;
  } catch(e) {}
}

el('reset').onclick = async () => {
  if(!confirm(
    '¿Borrar definitivamente todo el historial CSV '+
    'y comenzar desde cero?'
  )) return;

  el('resetMsg').textContent =
    'Reiniciando historial...';

  try {
    const r = await fetch('/api/resetcsv',{
      method:'POST'
    });

    el('resetMsg').textContent = await r.text();
    update();

  } catch(e) {
    el('resetMsg').textContent =
      'Error de comunicación';
  }
};

el('wifiForm').onsubmit = async e => {
  e.preventDefault();

  const ssid = el('ssid').value;
  const password = el('pass').value;

  if(!confirm(
    'La red Wi-Fi cambiará y tendrás que '+
    'reconectarte. ¿Continuar?'
  )) return;

  el('wifiMsg').textContent = 'Guardando...';

  try {
    const body = new URLSearchParams({
      ssid,password
    });

    const r = await fetch('/api/wifi',{
      method:'POST',
      headers:{
        'Content-Type':
          'application/x-www-form-urlencoded'
      },
      body
    });

    el('wifiMsg').textContent = await r.text();

  } catch(err) {
    el('wifiMsg').textContent =
      'Error de comunicación';
  }
};

loadWifi();
update();
setInterval(update,1000);
</script>
</body>
</html>
)HTML";

float measure(
  Adafruit_ADS1115 &ads,
  bool pair23
) {
  double sum = 0;
  double sum2 = 0;

  for(int i=0;i<SAMPLES;i++) {
    int16_t raw = pair23 ?
      ads.readADC_Differential_2_3() :
      ads.readADC_Differential_0_1();

    double v = ads.computeVolts(raw);
    sum += v;
    sum2 += v*v;
  }

  double mean = sum/SAMPLES;
  double variance = sum2/SAMPLES-mean*mean;

  return sqrt(fmax(0.0,variance))*
    AMPS_PER_VOLT;
}

void sampleChannels() {
  available[0] = ads2OK;
  available[1] = ads2OK;
  available[2] = ads1OK;
  available[3] = ads1OK;

  // Conector 1: ADS2 A2-A3
  // Conector 2: ADS2 A0-A1
  if(ads2OK) {
    amps[0] = measure(ads2,true);
    amps[1] = measure(ads2,false);
  }

  // Conector 3: ADS1 A2-A3
  // Conector 4: ADS1 A0-A1
  if(ads1OK) {
    amps[2] = measure(ads1,true);
    amps[3] = measure(ads1,false);
  }
}

bool createCSV() {
  if(!sdOK) return false;

  if(SD.exists(CSV_PATH)) return true;

  File f = SD.open(CSV_PATH,FILE_WRITE);

  if(!f) return false;

  size_t n = f.println(
    "tiempo_ms,Conector_1_A,Conector_2_A,"
    "Conector_3_A,Conector_4_A"
  );

  f.close();
  return n>0;
}

void appendCSV() {
  if(!sdOK || loggingPaused) return;

  File f = SD.open(CSV_PATH,FILE_APPEND);

  if(!f) return;

  f.print(millis());

  for(int i=0;i<4;i++) {
    f.print(',');

    if(available[i]) {
      f.print(amps[i],3);
    }
  }

  size_t n = f.println();
  f.close();

  if(n) records++;
}

void loadWiFiConfig() {
  wifiSSID = DEFAULT_SSID;
  wifiPass = DEFAULT_PASS;

  if(!sdOK || !SD.exists(WIFI_PATH)) return;

  File f = SD.open(WIFI_PATH,FILE_READ);
  if(!f) return;

  String s = f.readStringUntil('\n');
  String p = f.readStringUntil('\n');
  f.close();

  s.trim();
  p.trim();

  if(
    s.length()>=1 &&
    s.length()<=32 &&
    p.length()>=8 &&
    p.length()<=63
  ) {
    wifiSSID = s;
    wifiPass = p;

    Serial.println(
      "Wi-Fi cargado desde microSD"
    );
  } else {
    Serial.println(
      "Configuracion invalida: usando predeterminados"
    );
  }
}

bool saveWiFiConfig(
  const String &s,
  const String &p
) {
  if(!sdOK) return false;

  const char* temp = "/wifi.tmp";

  SD.remove(temp);

  File f = SD.open(temp,FILE_WRITE);
  if(!f) return false;

  bool ok = f.println(s)>0;
  ok = (f.println(p)>0) && ok;

  f.close();

  if(!ok) {
    SD.remove(temp);
    return false;
  }

  if(
    SD.exists(WIFI_PATH) &&
    !SD.remove(WIFI_PATH)
  ) {
    return false;
  }

  return SD.rename(temp,WIFI_PATH);
}

String jsonData() {
  String out;
  out.reserve(350);

  out = "{\"sd\":";
  out += sdOK ? "true" : "false";

  out += ",\"records\":";
  out += String(records);
  out += ",\"channels\":[";

  for(int i=0;i<4;i++) {
    if(i) out += ",";

    out += "{\"available\":";
    out += available[i] ? "true" : "false";

    out += ",\"amps\":";
    out += String(amps[i],3);
    out += "}";
  }

  out += "]}";

  return out;
}

void handleResetCSV() {
  if(!sdOK) {
    server.send(
      503,
      "text/plain; charset=utf-8",
      "microSD no disponible"
    );
    return;
  }

  // Pausar el registro durante el reinicio.
  loggingPaused = true;

  bool removed =
    !SD.exists(CSV_PATH) ||
    SD.remove(CSV_PATH);

  bool created = removed && createCSV();

  if(created) records = 0;

  // Reanudar el registro.
  loggingPaused = false;

  server.send(
    created ? 200 : 500,
    "text/plain; charset=utf-8",
    created ?
      "CSV reiniciado. El registro continua." :
      "No se pudo reiniciar el CSV."
  );
}

void handleWifiSave() {
  if(!sdOK) {
    server.send(
      503,
      "text/plain; charset=utf-8",
      "microSD no disponible. No se pueden guardar cambios."
    );
    return;
  }

  String s = server.arg("ssid");
  String p = server.arg("password");

  if(
    s.length()<1 ||
    s.length()>32 ||
    p.length()<8 ||
    p.length()>63 ||
    s.indexOf('\n')>=0 ||
    s.indexOf('\r')>=0 ||
    p.indexOf('\n')>=0 ||
    p.indexOf('\r')>=0
  ) {
    server.send(
      400,
      "text/plain; charset=utf-8",
      "SSID o contraseña no validos."
    );
    return;
  }

  if(!saveWiFiConfig(s,p)) {
    server.send(
      500,
      "text/plain; charset=utf-8",
      "Error al guardar configuracion en microSD."
    );
    return;
  }

  wifiSSID = s;
  wifiPass = p;

  server.send(
    200,
    "text/plain; charset=utf-8",
    "Configuracion guardada. Reconectate a la nueva red."
  );

  restartAt = millis()+2500;
}

void setup() {
  Serial.begin(115200);
  delay(900);

  Serial.println();
  Serial.println("=== MONITOR ESP32 ===");

  Wire.begin(21,22);

  ads1OK = ads1.begin(0x48);
  ads2OK = ads2.begin(0x49);

  if(ads1OK) {
    ads1.setGain(GAIN_ONE);
    ads1.setDataRate(RATE_ADS1115_860SPS);
  }

  if(ads2OK) {
    ads2.setGain(GAIN_ONE);
    ads2.setDataRate(RATE_ADS1115_860SPS);
  }

  Serial.printf(
    "ADS1: %s | ADS2: %s\n",
    ads1OK ? "OK" : "ERROR",
    ads2OK ? "OK" : "ERROR"
  );

  SPI.begin(18,19,23,SD_CS);

  pinMode(SD_CS,OUTPUT);
  digitalWrite(SD_CS,HIGH);

  sdOK = SD.begin(
    SD_CS,
    SPI,
    1000000
  ) && SD.cardType()!=CARD_NONE;

  Serial.println(
    sdOK ?
    "microSD detectada" :
    "microSD no disponible"
  );

  if(sdOK) {
    if(!createCSV()) {
      Serial.println(
        "ERROR: no se pudo crear CSV"
      );
    }
  }

  // Cargar la configuracion de la SD.
  // Si falla, usar los valores predeterminados.
  loadWiFiConfig();

  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(
    wifiSSID.c_str(),
    wifiPass.c_str()
  );

  Serial.printf(
    "Wi-Fi: %s (%s)\n",
    wifiSSID.c_str(),
    started ? "OK" : "ERROR"
  );

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/",HTTP_GET,[]() {
    server.send_P(
      200,
      "text/html; charset=utf-8",
      PAGE
    );
  });

  server.on("/api/data",HTTP_GET,[]() {
    server.sendHeader(
      "Cache-Control",
      "no-store"
    );

    server.send(
      200,
      "application/json",
      jsonData()
    );
  });

  server.on(
    "/api/resetcsv",
    HTTP_POST,
    handleResetCSV
  );

  server.on("/api/wifi",HTTP_GET,[]() {
    String s = wifiSSID;
    String p = wifiPass;

    s.replace("\\","\\\\");
    s.replace("\"","\\\"");

    p.replace("\\","\\\\");
    p.replace("\"","\\\"");

    server.send(
      200,
      "application/json",
      "{\"ssid\":\""+s+
      "\",\"password\":\""+p+"\"}"
    );
  });

  server.on(
    "/api/wifi",
    HTTP_POST,
    handleWifiSave
  );

  server.on("/download",HTTP_GET,[]() {
    if(
      !sdOK ||
      !SD.exists(CSV_PATH)
    ) {
      server.send(
        503,
        "text/plain",
        "CSV no disponible"
      );
      return;
    }

    File f = SD.open(
      CSV_PATH,
      FILE_READ
    );

    if(!f) {
      server.send(
        500,
        "text/plain",
        "No se pudo abrir CSV"
      );
      return;
    }

    server.sendHeader(
      "Content-Disposition",
      "attachment; filename=\"corriente_conectores.csv\""
    );

    server.streamFile(f,"text/csv");
    f.close();
  });

  server.begin();

  sampleChannels();
  lastSample = millis();

  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();

  if(
    restartAt &&
    (long)(millis()-restartAt)>=0
  ) {
    Serial.println(
      "Reiniciando ESP32 para aplicar Wi-Fi..."
    );

    delay(100);
    ESP.restart();
  }

  if(
    millis()-lastSample >=
    SAMPLE_INTERVAL
  ) {
    lastSample = millis();

    sampleChannels();
    appendCSV();

    for(int i=0;i<4;i++) {
      Serial.printf(
        "Conector %d: ",
        i+1
      );

      if(available[i]) {
        Serial.printf(
          "%.3f A RMS\n",
          amps[i]
        );
      } else {
        Serial.println(
          "ADS no disponible"
        );
      }
    }
  }
}
