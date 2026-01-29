/*
 * ========================================
 * ESP32 IoT Akıllı Ortam Kontrol Sistemi
 * ========================================
 * 
 * Özellikler:
 * - DHT22 ile sıcaklık/nem ölçümü
 * - LDR ile ışık seviyesi ölçümü
 * - 2 Adet transistörlü röle kontrolü
 * - Web tabanlı kontrol arayüzü
 * - EEPROM'a ayar kaydetme
 * - Durum LED'i
 * 
 * Pin Yapısı:
 * - GPIO 18: DHT22
 * - GPIO 34: LDR (ADC1 - WiFi uyumlu)
 * - GPIO 17: Işık rölesi
 * - GPIO 16: Fan rölesi
 * - GPIO 14: Durum LED'i
 * 
 * Geliştirici: IoT Projesi
 * Versiyon: 1.0 Final
 * ========================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DHT.h>

// ============ PIN TANIMLARI ============
#define DHT_PIN 18
#define LDR_PIN 34
#define LIGHT_RELAY_PIN 17
#define FAN_RELAY_PIN 16
#define STATUS_LED 14

// ============ SENSÖR VE SERVER ============
DHT dht(DHT_PIN, DHT22);
WebServer server(80);
Preferences prefs;

// ============ WİFİ BİLGİLERİ ============
const char* ssid = "speedygonzales";       // WiFi adını yaz
const char* password = "19982023"; // WiFi şifresini yaz

// ============ SİSTEM DEĞİŞKENLERİ ============
float tempThreshold = 28.0;
int lightThreshold = 2000;
float currentTemp = 0;
float currentHumidity = 0;
int currentLight = 0;
bool fanState = false;
bool lightState = false;

void setup() {
  // Pin konfigürasyonu
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  digitalWrite(LIGHT_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(STATUS_LED, LOW);
  
  // Serial başlatma
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n========================================");
  Serial.println("ESP32 IoT Sistem Baslatiliyor...");
  Serial.println("========================================\n");
  
  // Başlangıç LED sinyali (3x yanıp sön)
  for(int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(200);
    digitalWrite(STATUS_LED, LOW);
    delay(200);
  }
  
  // ADC konfigürasyonu
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  
  // Sensör başlatma
  dht.begin();
  Serial.println("[OK] DHT22 sensoru baslatildi");
  
  // EEPROM'dan ayarları yükleme
  prefs.begin("iot-settings", false);
  tempThreshold = prefs.getFloat("tempThr", 28.0);
  lightThreshold = prefs.getInt("lightThr", 2000);
  prefs.end();
  Serial.println("[OK] Ayarlar hafizadan yuklendi");
  Serial.print("    Sicaklik esigi: ");
  Serial.print(tempThreshold);
  Serial.println(" C");
  Serial.print("    Isik esigi: ");
  Serial.println(lightThreshold);
  
  // WiFi bağlantısı
  Serial.print("\n[WiFi] Baglaniyor");
  WiFi.begin(ssid, password);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 40) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[OK] WiFi baglandi!");
    Serial.print("    IP Adresi: ");
    Serial.println(WiFi.localIP());
    Serial.print("    Tarayicida ac: http://");
    Serial.println(WiFi.localIP());
    
    // WiFi başarılı - LED sürekli yanar
    digitalWrite(STATUS_LED, HIGH);
  } else {
    Serial.println("[HATA] WiFi baglanamadi!");
    
    // WiFi hatası - LED hızlı yanıp söner
    for(int i = 0; i < 10; i++) {
      digitalWrite(STATUS_LED, HIGH);
      delay(100);
      digitalWrite(STATUS_LED, LOW);
      delay(100);
    }
  }
  
  // Web server rotaları
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/setTemp", HTTP_GET, handleSetTemp);
  server.on("/setLight", HTTP_GET, handleSetLight);
  server.on("/manual", HTTP_GET, handleManual);
  
  server.begin();
  Serial.println("[OK] Web server baslatildi");
  Serial.println("\n========================================");
  Serial.println("Sistem hazir ve calisiyor!");
  Serial.println("========================================\n");
}

void loop() {
  server.handleClient();
  
  // LED heartbeat (2 saniyede bir yanıp söner)
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 2000) {
    lastBlink = millis();
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }
  
  // Sensör okuma (3 saniyede bir)
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 3000) {
    lastRead = millis();
    readSensorsAndControl();
  }
}

// ============ SENSÖR OKUMA VE KONTROL ============
void readSensorsAndControl() {
  // DHT22 okuma
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t) && !isnan(h)) {
    currentTemp = t;
    currentHumidity = h;
  }
  
  // LDR okuma
  currentLight = analogRead(LDR_PIN);
  
  // Serial çıktı
  Serial.print("Temp: ");
  Serial.print(currentTemp, 1);
  Serial.print("C | Hum: ");
  Serial.print(currentHumidity, 0);
  Serial.print("% | LDR: ");
  Serial.print(currentLight);
  Serial.print(" | Fan: ");
  Serial.print(fanState ? "ON" : "OFF");
  Serial.print(" | Light: ");
  Serial.println(lightState ? "ON" : "OFF");
  
  // Fan kontrolü (sıcaklık bazlı)
  if (currentTemp >= tempThreshold) {
    digitalWrite(FAN_RELAY_PIN, HIGH);
    fanState = true;
  } else {
    digitalWrite(FAN_RELAY_PIN, LOW);
    fanState = false;
  }
  
  // Işık kontrolü (LDR bazlı - karanlıkta aç)
  if (currentLight < lightThreshold) {
    digitalWrite(LIGHT_RELAY_PIN, HIGH);
    lightState = true;
  } else {
    digitalWrite(LIGHT_RELAY_PIN, LOW);
    lightState = false;
  }
}

// ============ WEB ARAYÜZÜ ============
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang='tr'>
<head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>ESP32 IoT Kontrol</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:'Segoe UI',Arial,sans-serif;background:linear-gradient(135deg,#000000 0%,#151b54 100%);min-height:100vh;padding:20px}
.container{max-width:650px;margin:0 auto;background:#fff;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,0.4);padding:30px}
h1{text-align:center;color:#667eea;margin-bottom:30px;font-size:26px}
.card{background:#f8f9fa;border-radius:15px;padding:20px;margin-bottom:20px;border-left:5px solid #667eea;transition:transform 0.2s}
.card:hover{transform:translateY(-2px)}
.sensor-label{color:#666;font-size:12px;text-transform:uppercase;letter-spacing:1px;margin-bottom:5px}
.sensor-value{font-size:32px;font-weight:bold;color:#2ecc71;margin:5px 0}
.sensor-sub{color:#999;font-size:14px;margin-top:5px}
.control-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:15px}
.control-title{font-size:18px;font-weight:600;color:#333}
.status{display:inline-block;padding:6px 16px;border-radius:20px;font-size:12px;font-weight:bold}
.status-on{background:#27ae60;color:#fff}
.status-off{background:#c0392b;color:#fff}
.slider-container{margin:15px 0}
.slider-label{display:flex;justify-content:space-between;margin-bottom:8px;font-weight:600;color:#333;font-size:14px}
input[type=range]{width:100%;height:8px;border-radius:5px;background:#ddd;outline:none;-webkit-appearance:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border-radius:50%;background:#667eea;cursor:pointer;box-shadow:0 2px 5px rgba(0,0,0,0.2)}
input[type=range]::-moz-range-thumb{width:22px;height:22px;border-radius:50%;background:#667eea;cursor:pointer;border:none}
.btn-group{display:flex;gap:10px;margin-top:15px}
.btn{flex:1;padding:12px;border:none;border-radius:10px;font-weight:600;cursor:pointer;transition:all 0.3s;font-size:13px}
.btn-on{background:#27ae60;color:#fff}
.btn-off{background:#c0392b;color:#fff}
.btn:hover{opacity:0.85;transform:scale(1.02)}
.btn:active{transform:scale(0.98)}
.footer{text-align:center;color:#999;font-size:12px;margin-top:25px;padding-top:20px;border-top:1px solid #eee}
.ip-info{background:#e3f2fd;border:2px solid #2196f3;border-radius:10px;padding:12px;margin-bottom:20px;text-align:center;font-size:14px;color:#1565c0}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.7}}
.updating{animation:pulse 1s infinite}
</style>
</head>
<body>
<div class='container'>
<h1> ESP32 Akıllı Ortam Kontrol</h1>

<div class='ip-info'>
 IP: <strong id='ipAddr'>Yükleniyor...</strong>
</div>

<div class='card'>
<div class='sensor-label'>SICAKLIK</div>
<div class='sensor-value' id='temp'>--°C</div>
<div class='sensor-sub'>Nem: <span id='humidity'>--</span>%</div>
</div>

<div class='card'>
<div class='sensor-label'>IŞIK SEVİYESİ</div>
<div class='sensor-value' id='light'>--</div>
<div class='sensor-sub'>0 = Karanlık | 4095 = Aydınlık</div>
</div>

<div class='card'>
<div class='control-header'>
<div class='control-title'> FAN</div>
<span class='status' id='fanStatus'>KAPALI</span>
</div>
<div class='slider-container'>
<div class='slider-label'>
<span>Sıcaklık Eşiği</span>
<span><strong id='tempValue'>28</strong>°C</span>
</div>
<input type='range' id='tempSlider' min='20' max='40' value='28' step='0.5'>
</div>
<div class='btn-group'>
<button class='btn btn-on' onclick='manual("fan",1)'>MANUEL AÇ</button>
<button class='btn btn-off' onclick='manual("fan",0)'>MANUEL KAPAT</button>
</div>
</div>

<div class='card'>
<div class='control-header'>
<div class='control-title'> IŞIK</div>
<span class='status' id='lightStatus'>KAPALI</span>
</div>
<div class='slider-container'>
<div class='slider-label'>
<span>Işık Eşiği</span>
<span><strong id='lightValue'>2000</strong></span>
</div>
<input type='range' id='lightSlider' min='0' max='4095' value='2000' step='100'>
</div>
<div class='btn-group'>
<button class='btn btn-on' onclick='manual("light",1)'>MANUEL AÇ</button>
<button class='btn btn-off' onclick='manual("light",0)'>MANUEL KAPAT</button>
</div>
</div>

<div class='footer'>
<div id='updateTime'>Son güncelleme: --</div>
<div style='margin-top:8px;color:#bbb'>ESP32 | DHT22 | LDR | Transistörlü Röle</div>
</div>
</div>

<script>
const tempSlider=document.getElementById('tempSlider');
const lightSlider=document.getElementById('lightSlider');
const tempValue=document.getElementById('tempValue');
const lightValue=document.getElementById('lightValue');

fetch('/data').then(r=>r.json()).then(d=>{
  document.getElementById('ipAddr').textContent=window.location.hostname;
});

tempSlider.oninput=function(){
  tempValue.textContent=this.value;
  fetch('/setTemp?v='+this.value);
};

lightSlider.oninput=function(){
  lightValue.textContent=this.value;
  fetch('/setLight?v='+this.value);
};

function manual(device,state){
  fetch('/manual?dev='+device+'&state='+state)
  .then(()=>setTimeout(updateData,300));
}

function updateData(){
  const container=document.querySelector('.container');
  container.classList.add('updating');
  
  fetch('/data')
  .then(r=>r.json())
  .then(d=>{
    document.getElementById('temp').textContent=d.temp.toFixed(1)+'°C';
    document.getElementById('humidity').textContent=d.hum.toFixed(0);
    document.getElementById('light').textContent=d.light;
    
    const fanStatus=document.getElementById('fanStatus');
    fanStatus.textContent=d.fanState?'AÇIK':'KAPALI';
    fanStatus.className='status '+(d.fanState?'status-on':'status-off');
    
    const lightStatus=document.getElementById('lightStatus');
    lightStatus.textContent=d.lightState?'AÇIK':'KAPALI';
    lightStatus.className='status '+(d.lightState?'status-on':'status-off');
    
    tempSlider.value=d.tempThr;
    tempValue.textContent=d.tempThr;
    lightSlider.value=d.lightThr;
    lightValue.textContent=d.lightThr;
    
    const now=new Date();
    document.getElementById('updateTime').textContent='Son güncelleme: '+now.toLocaleTimeString('tr-TR');
    
    container.classList.remove('updating');
  })
  .catch(()=>container.classList.remove('updating'));
}

updateData();
setInterval(updateData,3000);
</script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

// ============ JSON VERİ GÖNDERİMİ ============
void handleData() {
  String json = "{";
  json += "\"temp\":" + String(currentTemp, 1) + ",";
  json += "\"hum\":" + String(currentHumidity, 0) + ",";
  json += "\"light\":" + String(currentLight) + ",";
  json += "\"fanState\":" + String(fanState ? "true" : "false") + ",";
  json += "\"lightState\":" + String(lightState ? "true" : "false") + ",";
  json += "\"tempThr\":" + String(tempThreshold, 1) + ",";
  json += "\"lightThr\":" + String(lightThreshold);
  json += "}";
  
  server.send(200, "application/json", json);
}

// ============ SICAKLIK EŞİĞİ AYARLAMA ============
void handleSetTemp() {
  if (server.hasArg("v")) {
    tempThreshold = server.arg("v").toFloat();
    prefs.begin("iot-settings", false);
    prefs.putFloat("tempThr", tempThreshold);
    prefs.end();
    readSensorsAndControl();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error");
  }
}

// ============ IŞIK EŞİĞİ AYARLAMA ============
void handleSetLight() {
  if (server.hasArg("v")) {
    lightThreshold = server.arg("v").toInt();
    prefs.begin("iot-settings", false);
    prefs.putInt("lightThr", lightThreshold);
    prefs.end();
    readSensorsAndControl();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error");
  }
}

// ============ MANUEL KONTROL ============
void handleManual() {
  if (server.hasArg("dev") && server.hasArg("state")) {
    String device = server.arg("dev");
    int state = server.arg("state").toInt();
    
    if (device == "fan") {
      digitalWrite(FAN_RELAY_PIN, state ? HIGH : LOW);
      fanState = state;
    } 
    else if (device == "light") {
      digitalWrite(LIGHT_RELAY_PIN, state ? HIGH : LOW);
      lightState = state;
    }
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error");
  }
}
