#include <esp_camera.h>        // Wird für die Kamera gebraucht
#include <soc/soc.h>           // Deaktivieren Sie Probleme mit dem Brownout
#include <soc/rtc_cntl_reg.h>  // Deaktivieren Sie Probleme mit dem Brownout
#include <driver/rtc_io.h>     // Deaktivieren Sie Probleme mit dem Brownout
#include <HTTPClient.h>        // WIrd für den Http-Post gebraucht

#include "SIM_Verbindung.ino.h"   // Headerdatei für die SIM-Verbindung

// serielle Datenübertragung der ESP32-Cam
// #define ESP32_CAM_RX      3
// #define ESP32_CAM_TX      1

// Pins des OV2640 Kamerasensors zum Kameramodul (CAMERA_MODEL_AI_THINKER)
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
const char* server_url = "***";   // eigene Url angeben

// HardwareSerial Serial2(2);

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
  //pinMode(ONBOARD_LED_PIN, OUTPUT);
  
  // Setze die Baudrate des seriellen Monitors auf 115200
  SerialMon.begin(115200);

  // Serial2.begin (115200, SERIAL_8N1, ESP_RX, ESP_TX);

  // Setze die Baudrate und die UART-Pins des GSM-Moduls
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

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
  
  // Starte das SIM800-Modul neu, es dauert eine Weile
  // Um es zu überspringen, ruft man init() anstelle von restart() auf
  SerialMon.println("Modem initialisieren...");
  modem.init();
  // Verwende modem.restart() wenn ein vollständiger Neustart benötigt wird
  // Verwende modem.init(), wenn der vollständige Neustart nicht benötigen wird

  // Entsperren Sie Ihre SIM-Karte mit einer PIN, falls erforderlich
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }

  //digitalWrite(ONBOARD_LED_PIN, HIGH);

  //SIM-Verbindung
  SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
  }
  else {
    SerialMon.println(" OK");
  }
}

void loop() {
  // Holt die aktuelle Zeit
  time_t now = time(nullptr);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  int hour = timeinfo.tm_hour;

  // Überprüfen, ob die aktuelle Stunde zwischen 18 oder 6 Uhr liegt
  if (hour >= 18 || hour < 6) {
    Serial.println("Geht in den langen Tiefschlaf...");
    // Deaktiviert die Kamera, um Strom zu sparen
    esp_camera_deinit();
    // Konfiguriert den Deep Sleep
    esp_sleep_enable_timer_wakeup(12 * 60 * 60 * 1000000); // 12 Stunden Schlafzeit
    esp_deep_sleep_start();
  }
  
  // Nach dem Aufwachen wird ein Bild aufgenommen und an den Server geschickt
  FotoAufnehmenUndHttpPost();

  // Überprüft, ob die aktuelle Stunde zwischen 6 und 18 Uhr liegt
  if (hour >= 6 && hour < 18) {
    Serial.println("Geht in den Tiefschlaf für 60 Minuten...");
    // Deaktiviert die Kamera, um Strom zu sparen
    esp_camera_deinit();
    // Konfiguriert den Deep Sleep für 60 Minuten
    esp_sleep_enable_timer_wakeup(60 * 60 * 1000000);
    esp_deep_sleep_start();
  }  
}
