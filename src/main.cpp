#include <Arduino.h>
#include <EEPROM.h>
#include <LinkedList.h>

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

#include <ArduinoOTA.h>
bool isOtaOn = false; // da li je OTA update u toku

#include "LdrVals.h"
LdrVals ldrs;

#include "EasyINI.h"
EasyINI ei("/config.ini");

#define DEBUG true

// pins
// INPUT
const int pinPhotoRes = A0; // pin za LDR i taster za WiFi
const int pinPIR = D0;
// OUTPUT
const int pinLight = D8; // LED svetlo za osvetljavanje prostorije
const int pinLed = 2;    // ugradjena LED dioda na ESPu - upaljena kada radi wifi

const int eepromPos = 88; // pozicija u EEPROMu na kojoj ce se cuvati podatak da li se wifi pali ili ne pri sledecm resetu

// variables
bool isLightOn;          // da li je svetlo upaljeno ili ne
int backlightLimit;      // granica pozadinskog osvetljenja ...
bool isWiFiOn = false;   // da li wifi i veb server treba da budu ukljuceni
long msBtnStart = -1;    // millis za pocetak pritiska na taster
long msLastServerAction; // millis za poslednju akciju sa veb serverom (pokretanje ili ucitavanje neke stranice)
long msLastPir = -1;     // poslednji put kada je signal sa PIRa bio HIGH
long msStartPir = -1;    // pocetak HIGH PIR signala pri dovoljno jakom pozadinskom osvetljenju
int consecPirs = 0;      // broj uzastopnih pinPIR HIGH vrednosti
int i = 0;

#include "Setts.h"
Setts setts;

LinkedList<String> statuses = LinkedList<String>(); // lista statusa aparata
int idStatus = 0;
long msLastStatus;

// Dodavanje novog status stringa u listu statusa
void AddStatusString(int _id, int _ldr, int _secFromPIR, bool _isLightOn)
{
    // brisanje liste ako njena velicina predje neku zadatu vrednost
    if (statuses.size() > 100)
        statuses.clear();

    String s;
    s += _id;
    s += ';';
    s += _ldr;
    s += ';';
    s += _secFromPIR;
    s += ';';
    s += _isLightOn ? 1 : 0;
    statuses.add(s);
}

void SetLight(bool isOn)
{
    int lvl = map(setts.lightLevel, 0, setts.MAX_LEVEL, 0, PWMRANGE);
    analogWrite(pinLight, isOn ? lvl : 0);
    if (isLightOn && !isOn) // svetlo se upravo gasi
        delay(setts.lightOutDelay);
    isLightOn = isOn;
}

// Pamcenje informacije o tome da li ce WiFi biti ukljucen ili ne posle sledeceg paljenja/budjenja aparata i reset
void RestartForWiFi(bool nextWiFiOn)
{
    EEPROM.write(eepromPos, nextWiFiOn);
    EEPROM.commit();
    delay(10);
    ESP.restart();
}

void ReadConfigFile()
{
    msLastServerAction = millis();
    setts.loadSetts();
    backlightLimit = setts.backlightLimitLow;
}

void HandleSaveConfig()
{
    // lightOn=6&backlightLimitLow=200&backlightLimitHigh=400&wifiOn=2200...
    msLastServerAction = millis();
    setts.saveSetts(server);
    backlightLimit = setts.backlightLimitLow;
    SendEmptyText(server);

    if (setts.wifiOn == 0 && !isWiFiOn)
        RestartForWiFi(true);
}

void HandleTest()
{
    server.send(200, "text/html", "<h1>Test: You are connected</h1>");
}

void HandleGetStatus()
{
    msLastServerAction = millis();
    if (statuses.size() > 0)
    {
        String last = statuses.get(statuses.size() - 1);
        server.send(200, "text/x-csv", last);
    }
    else
        server.send(204, "text/x-csv", "");
}

void HandleLdrVals()
{
    server.send(200, "text/html", ldrs.PrintAll());
}

void HandleNotFound()
{
    String s = "Page Not Found\n\n";
    s += "URI: ";
    s += server.uri();
    s += "\nMethod: ";
    s += (server.method() == HTTP_GET) ? "GET" : "POST";
    s += "\nArguments: ";
    s += server.args();
    s += "\n";
    for (int i = 0; i < server.args(); i++)
        s += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    server.send(404, "text/plain", s);
}

void setup()
{
    pinMode(pinPIR, INPUT);
    pinMode(pinPhotoRes, INPUT);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, true);
    pinMode(pinLight, OUTPUT);
    SetLight(false);

    Serial.begin(115200);
    LittleFS.begin();
    ReadConfigFile();

    EEPROM.begin(512);
    isWiFiOn = setts.wifiOn == 0 ? true : EEPROM.read(eepromPos);

    SetLight(isWiFiOn); // ako se pali WiFi, svetlo je upaljeno
    backlightLimit = isWiFiOn ? setts.backlightLimitHigh : setts.backlightLimitLow;

    if (isWiFiOn)
    {
        ConnectToWiFi();
        SetupIPAddress(40);

        server.on("/inc/wifi_light_128.png", []() { HandleDataFile(server, "/inc/wifi_light_128.png", "image/png"); });
        server.on("/test", HandleTest);
        server.on("/", []() { HandleDataFile(server, "/index.html", "text/html"); });
        server.on("/inc/index.js", []() { HandleDataFile(server, "/inc/index.js", "text/javascript"); });
        server.on("/inc/style.css", []() { HandleDataFile(server, "/inc/style.css", "text/css"); });
        server.on(ei.getFileName(), []() { HandleDataFile(server, ei.getFileName(), "text/x-csv"); });
        server.on("/save_config", HandleSaveConfig);
        server.on("/current_data.html", []() { HandleDataFile(server, "/current_data.html", "text/html"); });
        server.on("/inc/current_data.js", []() { HandleDataFile(server, "/inc/current_data.js", "text/javascript"); });
        server.on("/get_status", HandleGetStatus);
        server.on("/get_ldr_vals", HandleLdrVals);
        server.on("/otaUpdate", []() { server.send(200, "text/plain", "ESP is waiting for OTA updates..."); isOtaOn = true; ArduinoOTA.begin(); });
        server.onNotFound(HandleNotFound);
        server.begin();
        msLastStatus = msLastServerAction = millis();
        if (DEBUG)
            Serial.println("HTTP server started");
    }
    else
    {
        WiFi.forceSleepBegin();
        delay(10); // give RF section time to shutdown
    }
}

void loop()
{
    delay(setts.msMainDelay);

    if (isOtaOn)
        ArduinoOTA.handle();
    else
        server.handleClient();

    long ms = millis();
    int valPir = digitalRead(pinPIR);
    consecPirs = valPir ? consecPirs + 1 : 0;

    if (consecPirs >= setts.minHighPIRs) // HIGH na PIR-u se prihvata samo ako je ta vrednost x puta zaredom ocitana
        msLastPir = ms;
    if (!valPir && msStartPir != -1)
    {
        msStartPir = -1;
        msLastPir = -1;
    }

    int valPhotoRes = map(analogRead(pinPhotoRes), 0, 1023, 0, setts.MAX_LEVEL);
    valPhotoRes = ldrs.Add(valPhotoRes);

    if (valPhotoRes > backlightLimit) // prostorija je dovoljno osvetljena
    {
        if (consecPirs == setts.minHighPIRs) // pokret zapocet kada je prostorija dovoljno osvetljena
        {
            msStartPir = ms;
            //T Serial.println("msStartPir 1");
        }
        SetLight(false);
        backlightLimit = setts.backlightLimitLow;
    }
    else // prostorija nije dovoljno osvetljena
    {
        if (msLastPir != -1 && ms - msLastPir < 1000 * setts.lightOn && msStartPir == -1)
        {
            backlightLimit = setts.backlightLimitHigh;
            SetLight(true);
        }
        else
            SetLight(false);
    }

    if (valPhotoRes > 0.95 * setts.MAX_LEVEL) // ako je pritisnut taster nabudzen sa LDRom
    {
        if (msBtnStart == -1)
            msBtnStart = ms;                          // pamti se pocetak pritiska na taster
        else if (ms - msBtnStart > 1000 && !isWiFiOn) // ako se taster drzi vise od sekunde
            RestartForWiFi(true);
    }
    else
        msBtnStart = -1;

    digitalWrite(pinLed, !isWiFiOn);
    if (isWiFiOn)
    {
        if (ms - msLastStatus > 1000)
        {
            AddStatusString(idStatus++, valPhotoRes, (ms - msLastPir) / 1000, isLightOn);
            msLastStatus = ms;
        }

        // da li je vreme da se iskljuci WiFi tj. veb server
        if (setts.wifiOn != 0 && ms - msLastServerAction > 60 * 1000 * setts.wifiOn)
            RestartForWiFi(false);

        server.handleClient();
    }
}
