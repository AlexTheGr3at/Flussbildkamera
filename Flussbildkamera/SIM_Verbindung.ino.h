// GPRS-Anmeldedaten (leer lassen, wenn nicht benötigt)
const char apn[]      = "TM"; // APN 
const char gprsUser[] = "";   // GPRS-Benutzer
const char gprsPass[] = "";   // GPRS-Passwort

// SIM-Karten-PIN (leer lassen, wenn nicht definiert)
const char simPIN[]   = "1503"; 

// Pins für SIM-Modul          
#define MODEM_TX             5
#define MODEM_RX             4

// Serielle Schnittstelle für Debug-Ausgaben (für Serial Monitor, Standardgeschwindigkeit 115200)
#define SerialMon Serial
// Serielle Schnittstelle für AT-Befehle (für SIM800-Modul)
#define SerialAT Serial1

// Konfiguration der TinyGSM-Bibliothek
#define TINY_GSM_MODEM_SIM800      // Modem ist SIM800
#define TINY_GSM_RX_BUFFER   1024  // RX-Puffer auf 1 KB setzen

#include <TinyGsmClient.h> // Bibliothek zur Internetverbindung über GSM

// TinyGSM Kommunikation
TinyGsm modem(SerialAT);

// TinyGSM-Client für die Internetverbindung        
TinyGsmClient client(modem);
