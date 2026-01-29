# 🏠 ESP32 IoT Akıllı Ortam Kontrol Sistemi

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif" alt="ESP32">
  <img src="https://img.shields.io/badge/IDE-Arduino-00979D?style=for-the-badge&logo=arduino" alt="Arduino">
  <img src="https://img.shields.io/badge/Version-1.0-green?style=for-the-badge" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge" alt="License">
</p>

ESP32 mikrodenetleyici tabanlı, web arayüzü ile kontrol edilebilen akıllı ortam izleme ve kontrol sistemi.

---

## 📋 İçindekiler

- [Özellikler](#-özellikler)
- [Donanım Gereksinimleri](#-donanım-gereksinimleri)
- [Pin Bağlantıları](#-pin-bağlantıları)
- [Devre Şeması](#-devre-şeması)
- [Kurulum](#-kurulum)
- [Kullanım](#-kullanım)
- [Web Arayüzü](#-web-arayüzü)
- [API Endpoints](#-api-endpoints)
- [Sorun Giderme](#-sorun-giderme)
- [Lisans](#-lisans)

---

## ✨ Özellikler

| Özellik | Açıklama |
|---------|----------|
| 🌡️ **Sıcaklık/Nem Ölçümü** | DHT22 sensörü ile hassas ölçüm |
| 💡 **Işık Seviyesi Ölçümü** | LDR sensörü ile ortam ışığı takibi |
| 🔌 **Röle Kontrolü** | 2 adet transistörlü röle (Fan + Işık) |
| 🌐 **Web Arayüzü** | Responsive, modern tasarımlı kontrol paneli |
| 💾 **EEPROM Kayıt** | Ayarlar kalıcı olarak saklanır |
| 📶 **WiFi Bağlantısı** | Kablosuz erişim ve kontrol |
| 🔄 **Otomatik Mod** | Eşik değerlerine göre otomatik kontrol |
| 🎛️ **Manuel Kontrol** | Web üzerinden manuel açma/kapama |

---

## 🔧 Donanım Gereksinimleri

### Ana Bileşenler

| Bileşen | Adet | Açıklama |
|---------|------|----------|
| ESP32 DevKit | 1 | Ana mikrodenetleyici |
| DHT22 | 1 | Sıcaklık ve nem sensörü |
| LDR (Fotoresiztör) | 1 | Işık sensörü |
| 5V Röle Modülü | 2 | Fan ve ışık kontrolü için |
| NPN Transistör (2N2222/BC547) | 2 | Röle sürücü |
| 1K Direnç | 2 | Transistör baz direnci |
| 10K Direnç | 1 | LDR için pull-down |
| LED (Herhangi renk) | 1 | Durum göstergesi |
| 330Ω Direnç | 1 | LED akım sınırlayıcı |

### Opsiyonel

- Breadboard veya PCB
- Jumper kablolar
- 5V güç kaynağı
- Proje kutusu

---

## 📌 Pin Bağlantıları

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32 Pin Yapısı                     │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   GPIO 18  ──────►  DHT22 Data Pin                      │
│   GPIO 34  ──────►  LDR (ADC1 - WiFi uyumlu)            │
│   GPIO 17  ──────►  Işık Rölesi (Transistör üzerinden)  │
│   GPIO 16  ──────►  Fan Rölesi (Transistör üzerinden)   │
│   GPIO 14  ──────►  Durum LED'i                         │
│                                                         │
│   3.3V     ──────►  DHT22 VCC, LDR                      │
│   5V       ──────►  Röle modülleri VCC                  │
│   GND      ──────►  Ortak GND                           │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Detaylı Bağlantı Tablosu

| ESP32 Pin | Bileşen | Bağlantı Tipi |
|-----------|---------|---------------|
| GPIO 18 | DHT22 Data | Dijital Giriş |
| GPIO 34 | LDR | Analog Giriş (ADC1) |
| GPIO 17 | Işık Rölesi | Dijital Çıkış (Transistör) |
| GPIO 16 | Fan Rölesi | Dijital Çıkış (Transistör) |
| GPIO 14 | Status LED | Dijital Çıkış |
| 3.3V | DHT22 VCC | Güç |
| 5V | Röle VCC | Güç |
| GND | Tüm GND | Ortak Toprak |

> ⚠️ **Önemli:** GPIO 34 ADC1 kanalında olduğu için WiFi ile uyumludur. ADC2 pinleri WiFi kullanırken çalışmaz!

---

## 🔌 Devre Şeması

### LDR Bağlantısı
```
3.3V ─────┬───── LDR ─────┬───── GND
          │               │
          └───────────────┴───── GPIO 34
                    │
                   10K
                    │
                   GND
```

### Transistörlü Röle Bağlantısı
```
GPIO 16/17 ──── 1K ────┬──── Transistör Base
                       │
                      ─┴─ (NPN - 2N2222)
                       │
                       ├──── Röle Coil (-)
                       │
                      GND

5V ──── Röle Coil (+)
```

### DHT22 Bağlantısı
```
3.3V ──── VCC (Pin 1)
GPIO 18 ── DATA (Pin 2)
NC ─────── (Pin 3 - Boş)
GND ────── GND (Pin 4)
```

---

## 🚀 Kurulum

### 1. Arduino IDE Hazırlığı

1. [Arduino IDE](https://www.arduino.cc/en/software) indirin ve kurun
2. ESP32 board paketini ekleyin:
   - `Dosya` → `Tercihler` → `Ek Devre Kartları Yöneticisi URL'leri`
   - Şu URL'yi ekleyin: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. `Araçlar` → `Kart` → `Kart Yöneticisi` → "ESP32" arayın ve kurun

### 2. Kütüphanelerin Kurulumu

Arduino IDE'de `Araç` → `Kütüphaneleri Yönet` menüsünden şu kütüphaneleri kurun:

- **DHT sensor library** by Adafruit
- **Adafruit Unified Sensor** by Adafruit

### 3. Kod Yapılandırması

Kodu açın ve WiFi bilgilerinizi güncelleyin:

```cpp
const char* ssid = "WIFI_ADINIZ";         // WiFi ağ adınız
const char* password = "WIFI_SIFRENIZ";   // WiFi şifreniz
```

### 4. Yükleme

1. ESP32'yi USB ile bağlayın
2. `Araçlar` → `Kart` → `ESP32 Dev Module` seçin
3. Doğru COM portunu seçin
4. Yükle butonuna tıklayın

---

## 📱 Kullanım

### Başlatma

1. ESP32'yi güç kaynağına bağlayın
2. Seri monitörü açın (115200 baud)
3. WiFi bağlantısını bekleyin
4. Seri monitörde görünen IP adresini not alın

### LED Göstergeleri

| LED Durumu | Anlam |
|------------|-------|
| 3x Yanıp Sönme | Sistem başlatılıyor |
| Sürekli Yanık | WiFi bağlantısı başarılı |
| Hızlı Yanıp Sönme | WiFi bağlantı hatası |
| 2 sn'de bir Yanıp Sönme | Normal çalışma (heartbeat) |

---

## 🌐 Web Arayüzü

Tarayıcınızda ESP32'nin IP adresini açın (örn: `http://192.168.1.100`)

### Arayüz Özellikleri

- **Sıcaklık Kartı**: Anlık sıcaklık ve nem değerleri
- **Işık Kartı**: LDR sensör değeri (0-4095)
- **Fan Kontrolü**: 
  - Sıcaklık eşiği slider'ı (20-40°C)
  - Manuel açma/kapama butonları
- **Işık Kontrolü**:
  - Işık eşiği slider'ı (0-4095)
  - Manuel açma/kapama butonları

### Otomatik Kontrol Mantığı

```
📊 Fan: Sıcaklık >= Eşik Değeri → FAN AÇIK
📊 Işık: LDR Değeri < Eşik Değeri → IŞIK AÇIK (Karanlıkta aç)
```

---

## 🔗 API Endpoints

| Endpoint | Metod | Açıklama | Örnek |
|----------|-------|----------|-------|
| `/` | GET | Ana web arayüzü | - |
| `/data` | GET | JSON formatında sensör verileri | - |
| `/setTemp` | GET | Sıcaklık eşiğini ayarla | `/setTemp?v=25` |
| `/setLight` | GET | Işık eşiğini ayarla | `/setLight?v=1500` |
| `/manual` | GET | Manuel röle kontrolü | `/manual?dev=fan&state=1` |

### `/data` Yanıt Örneği

```json
{
  "temp": 25.5,
  "hum": 60,
  "light": 2345,
  "fanState": true,
  "lightState": false,
  "tempThr": 28.0,
  "lightThr": 2000
}
```

---

## 🔧 Sorun Giderme

### WiFi Bağlanmıyor

- [ ] SSID ve şifrenin doğruluğunu kontrol edin
- [ ] 2.4GHz ağa bağlandığınızdan emin olun (5GHz desteklenmez)
- [ ] Router'ın ESP32'ye izin verdiğinden emin olun
- [ ] ESP32'yi router'a yaklaştırın

### DHT22 Okuma Hatası

- [ ] Kablo bağlantılarını kontrol edin
- [ ] 3.3V güç verildiğinden emin olun
- [ ] Data pini ile VCC arasına 10K pull-up direnci ekleyin
- [ ] Sensörün arızalı olmadığından emin olun

### LDR Değeri Sabit

- [ ] Pull-down direnci (10K) bağlı mı kontrol edin
- [ ] GPIO 34 kullanıldığından emin olun (ADC1)
- [ ] LDR'nin ışığa tepki verdiğini test edin

### Röle Çalışmıyor

- [ ] Transistör bağlantısını kontrol edin
- [ ] Baz direncinin (1K) doğru olduğundan emin olun
- [ ] Röle modülüne 5V verildiğinden emin olun
- [ ] Transistörün doğru yönde takıldığından emin olun

---

## 📝 Seri Monitör Çıktısı

```
========================================
ESP32 IoT Sistem Baslatiliyor...
========================================

[OK] DHT22 sensoru baslatildi
[OK] Ayarlar hafizadan yuklendi
    Sicaklik esigi: 28.0 C
    Isik esigi: 2000

[WiFi] Baglaniyor........
[OK] WiFi baglandi!
    IP Adresi: 192.168.1.100
    Tarayicida ac: http://192.168.1.100
[OK] Web server baslatildi

========================================
Sistem hazir ve calisiyor!
========================================

Temp: 24.5C | Hum: 55% | LDR: 2456 | Fan: OFF | Light: OFF
```

---

## 📁 Proje Yapısı

```
ESP32-IoT-Ortam-Kontrol/
│
├── ESP32_IoT_Ortam_Kontrol.ino    # Ana Arduino kodu
├── README.md                       # Bu dosya
└── docs/                           # (Opsiyonel) Ek dökümanlar
    ├── devre_semasi.png
    └── web_arayuzu.png
```

---

## 🔮 Gelecek Geliştirmeler

- [ ] MQTT entegrasyonu
- [ ] Mobil uygulama desteği
- [ ] Veri kayıt ve grafik gösterimi
- [ ] Çoklu sensör desteği
- [ ] OTA (Over-The-Air) güncelleme
- [ ] Blynk / Home Assistant entegrasyonu

---

## 📄 Lisans

Bu proje MIT lisansı altında lisanslanmıştır. Detaylar için [LICENSE](LICENSE) dosyasına bakın.

---

## 🤝 Katkıda Bulunma

1. Bu projeyi fork edin
2. Feature branch oluşturun (`git checkout -b feature/YeniOzellik`)
3. Değişikliklerinizi commit edin (`git commit -m 'Yeni özellik eklendi'`)
4. Branch'inizi push edin (`git push origin feature/YeniOzellik`)
5. Pull Request oluşturun

---

## 📞 İletişim

Sorularınız veya önerileriniz için issue açabilirsiniz.

---

<p align="center">
  Made with ❤️ for IoT Community
</p>
