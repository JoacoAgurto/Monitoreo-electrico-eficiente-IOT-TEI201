
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>

const char* AP_SSID = "Monitor-ESP32";
const char* AP_PASSWORD = "corriente123";

WebServer server(80);

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;
bool ads1OK = false;
bool ads2OK = false;

const int SD_CS = 5;
const int SPI_SCK = 18;
const int SPI_MISO = 19;
const int SPI_MOSI = 23;
const char* ARCHIVO = "/corriente_conectores.csv";

bool sdOK = false;
bool sdArchivoListo = false;
String estadoSD = "Inicializando";
unsigned long registros = 0;

const float AMPERIOS_POR_VOLTIO = 100.0;
const int NUM_MUESTRAS = 256;
const unsigned long INTERVALO_MS = 1000;

const char* nombres[4] = {
  "Conector 1",
  "Conector 2",
  "Conector 3",
  "Conector 4"
};

struct Medicion {
  float corriente;
  bool disponible;
};

Medicion mediciones[4] = {};
unsigned long ultimaMedicion = 0;

const char PAGINA[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Monitor de corriente</title>
<style>
:root {
  color-scheme:dark;
  --bg:#0b1220;
  --card:#152238;
  --line:#30425c;
  --text:#f1f5ff;
  --muted:#a8b9ce;
}
* {box-sizing:border-box}
body {
  margin:0;
  background:var(--bg);
  color:var(--text);
  font-family:Arial,system-ui,sans-serif;
}
header {
  background:#10213a;
  padding:24px 18px;
  border-bottom:1px solid var(--line);
}
header h1 {margin:0 0 8px}
header p {margin:0;color:var(--muted)}
main {max-width:1100px;margin:auto;padding:16px}
.status {
  display:flex;
  flex-wrap:wrap;
  gap:10px;
  margin-bottom:18px;
}
.pill {
  background:#1a2b45;
  border:1px solid var(--line);
  border-radius:20px;
  padding:9px 13px;
  font-size:14px;
}
.grid {
  display:grid;
  grid-template-columns:repeat(2,minmax(0,1fr));
  gap:15px;
}
.card {
  min-width:0;
  background:var(--card);
  border:1px solid var(--line);
  border-radius:15px;
  padding:18px;
}
.top {
  display:flex;
  justify-content:space-between;
  align-items:center;
  gap:10px;
}
h2 {font-size:19px;margin:0}
.value {
  font-size:40px;
  font-weight:bold;
  margin-top:18px;
}
.unit {
  font-size:17px;
  font-weight:normal;
  color:var(--muted);
}
.info {
  color:var(--muted);
  font-size:13px;
  margin-top:8px;
}
.toggle {
  font-size:13px;
  color:var(--muted);
  white-space:nowrap;
}
.toggle input {
  width:17px;
  height:17px;
  vertical-align:middle;
}
.graph {margin-top:18px}
.graph[hidden] {display:none}
canvas {
  display:block;
  width:100%;
  height:190px;
  background:#0d192b;
  border:1px solid var(--line);
  border-radius:10px;
}
.history {margin-top:18px}
button,a.download {
  display:inline-block;
  background:#245782;
  border:1px solid #5486ad;
  color:white;
  border-radius:9px;
  padding:12px 16px;
  text-decoration:none;
  font-size:15px;
  cursor:pointer;
}
button:hover,a.download:hover {background:#3273a7}
.actions {display:flex;gap:10px;flex-wrap:wrap;margin-top:16px}
.warning {color:#ffb2a7}
@media(max-width:700px) {
  .grid {grid-template-columns:1fr}
  .card {padding:14px}
  .value {font-size:35px}
}
</style>
</head>
<body>
<header>
  <h1>Monitor de corriente</h1>
  <p>Cuatro conectores independientes · ESP32</p>
</header>
<main>
  <div class="status">
    <div class="pill">ESP32: <b id="esp">Conectando...</b></div>
    <div class="pill">microSD: <b id="sd">Consultando...</b></div>
    <div class="pill">Registros: <b id="records">0</b></div>
  </div>

  <div class="grid" id="channels"></div>

  <section class="card history">
    <h2>Historial</h2>
    <p class="info">
      Se registran los cuatro conectores aunque sus
      gráficos estén ocultos.
    </p>
    <p id="sdmsg" class="info"></p>
    <div class="actions">
      <a class="download" href="/download">Descargar CSV</a>
      <button onclick="update()">Actualizar</button>
    </div>
    <p id="error" class="warning"></p>
  </section>
</main>

<script>
const names = [
  "Conector 1",
  "Conector 2",
  "Conector 3",
  "Conector 4"
];
const colors = [
  "#65b4ff",
  "#51d5a0",
  "#ffbd70",
  "#cb9cff"
];
const histories = [[],[],[],[]];
const maxPoints = 90;
const visible = [true,true,true,true];
const $ = id => document.getElementById(id);

names.forEach((name,i)=>{
  try {
    visible[i] = localStorage.getItem("chart"+i)!=="false";
  } catch(e) {}

  const card = document.createElement("section");
  card.className = "card";
  card.innerHTML = `
    <div class="top">
      <h2>${name}</h2>
      <label class="toggle">
        <input type="checkbox" id="toggle${i}"
          ${visible[i]?"checked":""}>
        Mostrar gráfico
      </label>
    </div>
    <div class="value">
      <span id="amps${i}">--</span>
      <span class="unit">A RMS</span>
    </div>
    <div class="info" id="state${i}">Esperando datos...</div>
    <div class="graph" id="wrap${i}"
      ${visible[i]?"":"hidden"}>
      <canvas id="chart${i}" width="680" height="220"></canvas>
      <div class="info">Últimas 90 lecturas</div>
    </div>
  `;
  $("channels").appendChild(card);

  $("toggle"+i).addEventListener("change",e=>{
    visible[i] = e.target.checked;
    $("wrap"+i).hidden = !visible[i];
    try {
      localStorage.setItem("chart"+i,String(visible[i]));
    } catch(err) {}
    if(visible[i]) draw(i);
  });
});

function draw(i) {
  if(!visible[i]) return;

  const canvas = $("chart"+i);
  const ctx = canvas.getContext("2d");
  const w = canvas.width;
  const h = canvas.height;
  const left = 52;
  const right = 14;
  const top = 15;
  const bottom = 32;
  const values = histories[i];

  ctx.clearRect(0,0,w,h);

  const valid = values.filter(Number.isFinite);
  const maximum = Math.max(0.1,...valid)*1.15;

  ctx.font = "13px Arial";
  ctx.fillStyle = "#a8b9ce";
  ctx.strokeStyle = "#30425c";
  ctx.lineWidth = 1;

  for(let k=0;k<=4;k++) {
    const y = top+(h-top-bottom)*k/4;
    ctx.beginPath();
    ctx.moveTo(left,y);
    ctx.lineTo(w-right,y);
    ctx.stroke();
    ctx.fillText(
      (maximum*(1-k/4)).toFixed(maximum<1?2:1),
      5,y+4
    );
  }

  if(values.length<2) {
    ctx.fillText("Esperando lecturas...",left+10,h/2);
    return;
  }

  ctx.beginPath();
  ctx.strokeStyle = colors[i];
  ctx.lineWidth = 2.7;
  let started = false;

  values.forEach((v,k)=>{
    if(!Number.isFinite(v)) {
      started = false;
      return;
    }
    const x = left+(w-left-right)*k/(maxPoints-1);
    const y = h-bottom-(h-top-bottom)*Math.min(v,maximum)/maximum;

    if(!started) {
      ctx.moveTo(x,y);
      started = true;
    } else {
      ctx.lineTo(x,y);
    }
  });

  ctx.stroke();
  ctx.fillStyle = "#a8b9ce";
  ctx.fillText("Tiempo",w-75,h-8);
}

let busy = false;

async function update() {
  if(busy) return;
  busy = true;

  try {
    const response = await fetch("/api/data",{cache:"no-store"});
    if(!response.ok) throw Error("HTTP "+response.status);

    const data = await response.json();
    $("esp").textContent = "Conectado";
    $("sd").textContent = data.sd?"Disponible":"No disponible";
    $("records").textContent = data.records;
    $("sdmsg").textContent = data.sdMessage;
    $("error").textContent = "";

    data.channels.forEach((ch,i)=>{
      if(i>=4) return;

      $("amps"+i).textContent =
        ch.available ? ch.amps.toFixed(3) : "--";

      $("state"+i).textContent =
        ch.available ? "Lectura actualizada" :
        "ADS1115 no disponible";

      histories[i].push(ch.available?ch.amps:null);
      if(histories[i].length>maxPoints) histories[i].shift();
      draw(i);
    });

  } catch(e) {
    $("esp").textContent = "Sin conexión";
    $("error").textContent =
      "No se pudo comunicar con el ESP32.";
  }

  busy = false;
}

update();
setInterval(update,1000);
</script>
</body>
</html>
)HTML";

Medicion medirCanal(Adafruit_ADS1115 &ads,int canal) {
  Medicion m = {0.0f,true};

  double suma = 0;
  double sumaCuadrados = 0;

  for(int i=0;i<NUM_MUESTRAS;i++) {
    int16_t raw;

    if(canal==0)
      raw = ads.readADC_Differential_0_1();
    else
      raw = ads.readADC_Differential_2_3();

    double v = ads.computeVolts(raw);
    suma += v;
    sumaCuadrados += v*v;
  }

  double media = suma/NUM_MUESTRAS;
  double varianza =
    sumaCuadrados/NUM_MUESTRAS-media*media;

  if(varianza<0) varianza=0;

  float voltajeRMS = sqrt(varianza);
  m.corriente = voltajeRMS*AMPERIOS_POR_VOLTIO;

  return m;
}

void tomarMediciones() {
  for(int i=0;i<4;i++) {
    mediciones[i].corriente = 0;
    mediciones[i].disponible = false;
  }

  // Conector 1: ADS2 A2-A3
  // Conector 2: ADS2 A0-A1
  if(ads2OK) {
    mediciones[0] = medirCanal(ads2,1);
    mediciones[1] = medirCanal(ads2,0);
  }

  // Conector 3: ADS1 A2-A3
  // Conector 4: ADS1 A0-A1
  if(ads1OK) {
    mediciones[2] = medirCanal(ads1,1);
    mediciones[3] = medirCanal(ads1,0);
  }
}

bool iniciarSD() {
  Serial.println("Intentando detectar microSD...");
  SPI.begin(SPI_SCK,SPI_MISO,SPI_MOSI,SD_CS);
  pinMode(SD_CS,OUTPUT);
  digitalWrite(SD_CS,HIGH);

  // 1 MHz: velocidad conservadora para pruebas.
  if(!SD.begin(SD_CS,SPI,1000000)) {
    estadoSD = "No detectada: revisar tarjeta y conexiones";
    Serial.println("ERROR: SD.begin fallo");
    return false;
  }

  uint8_t tipo = SD.cardType();

  if(tipo==CARD_NONE) {
    estadoSD = "No se reconoce ninguna tarjeta";
    Serial.println("ERROR: ninguna tarjeta reconocida");
    SD.end();
    return false;
  }

  Serial.print("Tipo de tarjeta: ");
  if(tipo==CARD_MMC) Serial.println("MMC");
  else if(tipo==CARD_SD) Serial.println("SD");
  else if(tipo==CARD_SDHC) Serial.println("SDHC");
  else Serial.println("Desconocido");

  Serial.print("Capacidad MB: ");
  Serial.println(SD.cardSize()/(1024ULL*1024ULL));

  estadoSD = "Tarjeta detectada";
  return true;
}

void asegurarCSV() {
  if(!sdOK) return;

  if(SD.exists(ARCHIVO)) {
    sdArchivoListo = true;
    estadoSD = "CSV disponible";
    return;
  }

  File f = SD.open(ARCHIVO,FILE_WRITE);

  if(!f) {
    sdArchivoListo = false;
    estadoSD = "Error al crear el CSV";
    Serial.println("ERROR: no se pudo crear CSV");
    return;
  }

  f.println(
    "tiempo_ms,Conector_1_A,Conector_2_A,"
    "Conector_3_A,Conector_4_A"
  );
  f.close();

  sdArchivoListo = true;
  estadoSD = "CSV creado correctamente";
}

void registrarCSV() {
  if(!sdOK || !sdArchivoListo) return;

  File f = SD.open(ARCHIVO,FILE_APPEND);

  if(!f) {
    estadoSD = "Error al abrir CSV para escritura";
    Serial.println("ERROR: escritura SD");
    return;
  }

  f.print(millis());

  for(int i=0;i<4;i++) {
    f.print(",");
    if(mediciones[i].disponible)
      f.print(mediciones[i].corriente,3);
  }

  f.println();
  f.close();

  registros++;
  estadoSD = "Guardando correctamente";
}

String jsonData() {
  String out;
  out.reserve(700);

  out = "{\"sd\":";
  out += (sdOK && sdArchivoListo)?"true":"false";

  out += ",\"sdMessage\":\"";
  out += estadoSD;
  out += "\",\"records\":";
  out += String(registros);
  out += ",\"channels\":[";

  for(int i=0;i<4;i++) {
    if(i) out += ",";

    out += "{\"available\":";
    out += mediciones[i].disponible?"true":"false";
    out += ",\"amps\":";
    out += String(mediciones[i].corriente,3);
    out += "}";
  }

  out += "]}";
  return out;
}

void handleRoot() {
  server.send_P(
    200,
    "text/html; charset=utf-8",
    PAGINA
  );
}

void handleData() {
  server.sendHeader("Cache-Control","no-store");
  server.send(
    200,
    "application/json; charset=utf-8",
    jsonData()
  );
}

void handleDownload() {
  if(!sdOK || !sdArchivoListo || !SD.exists(ARCHIVO)) {
    server.send(
      503,
      "text/plain; charset=utf-8",
      "MicroSD o archivo CSV no disponible"
    );
    return;
  }

  File f = SD.open(ARCHIVO,FILE_READ);

  if(!f) {
    server.send(
      500,
      "text/plain",
      "No se pudo abrir el archivo"
    );
    return;
  }

  server.sendHeader(
    "Content-Disposition",
    "attachment; filename=\"corriente_conectores.csv\""
  );

  server.streamFile(f,"text/csv");
  f.close();
}

void setup() {
  Serial.begin(115200);
  delay(1200);

  Serial.println();
  Serial.println("===== MONITOR ESP32 =====");

  Wire.begin(21,22);

  ads1OK = ads1.begin(0x48);
  ads2OK = ads2.begin(0x49);

  if(ads1OK) {
    ads1.setGain(GAIN_ONE);
    ads1.setDataRate(RATE_ADS1115_860SPS);
    Serial.println("ADS1 detectado: 0x48");
  } else {
    Serial.println("ERROR ADS1: 0x48");
  }

  if(ads2OK) {
    ads2.setGain(GAIN_ONE);
    ads2.setDataRate(RATE_ADS1115_860SPS);
    Serial.println("ADS2 detectado: 0x49");
  } else {
    Serial.println("ERROR ADS2: 0x49");
  }

  sdOK = iniciarSD();

  if(sdOK) {
    asegurarCSV();
    Serial.println("microSD inicializada");
  } else {
    Serial.println("microSD no disponible");
    Serial.println("El monitor web seguira funcionando");
  }

  WiFi.mode(WIFI_AP);

  bool apOK = WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );

  if(apOK) {
    Serial.println("Wi-Fi creada");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("Clave: ");
    Serial.println(AP_PASSWORD);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("ERROR: no se pudo crear Wi-Fi");
  }

  server.on("/",HTTP_GET,handleRoot);
  server.on("/api/data",HTTP_GET,handleData);
  server.on("/download",HTTP_GET,handleDownload);

  server.begin();

  tomarMediciones();
  ultimaMedicion = millis();

  Serial.println("Servidor web iniciado");
  Serial.println("===== SISTEMA LISTO =====");
}

void loop() {
  server.handleClient();

  unsigned long ahora = millis();

  if(ahora-ultimaMedicion>=INTERVALO_MS) {
    ultimaMedicion = ahora;

    tomarMediciones();

    Serial.println();
    Serial.println("---- CORRIENTES ----");

    for(int i=0;i<4;i++) {
      Serial.print(nombres[i]);
      Serial.print(": ");

      if(mediciones[i].disponible) {
        Serial.print(mediciones[i].corriente,3);
        Serial.println(" A RMS");
      } else {
        Serial.println("No disponible");
      }
    }

    registrarCSV();
  }
}
