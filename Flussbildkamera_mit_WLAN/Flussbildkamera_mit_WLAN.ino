#include <esp_camera.h>        // Wird für die Kamera gebraucht
#include <soc/soc.h>           // Deaktivieren Sie Probleme mit dem Brownout
#include <soc/rtc_cntl_reg.h>  // Deaktivieren Sie Probleme mit dem Brownout
#include <driver/rtc_io.h>     // Deaktivieren Sie Probleme mit dem Brownout
#include <HTTPClient.h>        // Wird für den Http-Post gebraucht
#include <WiFi.h>              // Wird für die Wi-Fi Verbindung gebraucht

// Replace with your network credentials
const char* ssid = "***";       // eigene ssid angeben
const char* password = "***";   // eigenes passwort angeben

// Pins des OV2640 Kameramoduls (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// An diese URL wird das Bild geschickt
const char* server_url = "***";     // eigene Url angeben

// nimmt Foto auf und schickt es direkt über http Post an Url
void FotoAufnehmenUndHttpPost( void ) {
  // Kamera-Bild erfassen
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Fehler beim Erfassen des Kamerabildes");
    return;
  }

  // HTTP POST-Anfrage erstellen
  HTTPClient http;
  http.begin(server_url);
  http.addHeader("Content-Type", "image/jpeg");

  // Bilddaten senden
  int httpResponseCode = http.POST((uint8_t *)fb->buf, fb->len);
  if (httpResponseCode > 0) {
    Serial.printf("HTTP-POST-Anfrage erfolgreich, Antwortcode: %d\n", httpResponseCode);
  } else {
    Serial.printf("HTTP-POST-Anfrage fehlgeschlagen, Fehlercode: %d\n", httpResponseCode);
  }

  // Verbindung schließen und freigegebene Ressourcen freigeben
  http.end();
  esp_camera_fb_return(fb);
}

void setup() {
  // Setze die Baudrate des seriellen Monitors auf 115200
  Serial.begin(115200);

  // Verbindet mit Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }  

  // gibt die ESP32 Lokale IP Addresse aus
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Deaktiviert den 'Brownout-Detektor'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // OV2640-Kameramodul
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Kamera initialisieren
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamerainitialisierung fehlgeschlagen mit Fehler 0x%x \n", err);
    ESP.restart();
  }
}

void loop() {
  // Holen Sie sich die aktuelle Zeit
  time_t now = time(nullptr);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  int hour = timeinfo.tm_hour;

  // Überprüfen, ob die aktuelle Stunde zwischen 18 und 6 Uhr liegt
  if (hour >= 18 || hour < 6) {
    Serial.println("Geht in den langen Tiefschlaf...");
    // Deaktivieren Sie die Kamera, um Strom zu sparen
    esp_camera_deinit();
    // Konfigurieren Sie Deep Sleep
    esp_sleep_enable_timer_wakeup(12 * 60 * 60 * 1000000); // 12 Stunden Schlafzeit
    esp_deep_sleep_start();
  }
  
  // Nach dem Aufwachen ein Bild aufnehmen
  FotoAufnehmenUndHttpPost();
  
  // Überprüfen, ob die aktuelle Stunde zwischen 6 und 18 Uhr liegt
  if (hour >= 6 && hour < 18) {
    Serial.println("Geht in den Tiefschlaf für 60 Minuten...");
    // Deaktivieren Sie die Kamera, um Strom zu sparen
    esp_camera_deinit();
    // Konfigurieren Sie Deep Sleep für 60 Minuten
    esp_sleep_enable_timer_wakeup(60 * 60 * 1000000);
    esp_deep_sleep_start();
  }  
}