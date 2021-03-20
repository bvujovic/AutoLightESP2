#include <Arduino.h>
#include <LinkedList.h>

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

#include <ArduinoOTA.h>
bool isOtaOn = false; // da li je OTA update u toku

#include "LdrVals.h"
LdrVals ldrs;

#include "EasyINI.h"
EasyINI ei("/config.ini");

#include "EasyFS.h"

// pins
// INPUT
const byte pinPhotoRes = A0; // pin za LDR i taster za WiFi
const byte pinPIR = D0;
// OUTPUT
const byte pinLight = D8; // LED svetlo za osvetljavanje prostorije
const byte pinLed = 2;    // ugradjena LED dioda na ESPu - upaljena kada radi wifi

const int eepromPos = 88; // pozicija u EEPROMu na kojoj ce se cuvati podatak da li se wifi pali ili ne pri sledecm resetu

const ulong itvOtaTime = 1 * 60 * 1000; // vreme (u ms) koje ce aparat provesti u cekanju da pocne OTA update
const ulong itvModWait = 2 * 60 * 1000; // vreme cekanja na ponovnu upotrebu moda (dugo i/ili slabo svetlo).
// posle toga se aparat vraca na standardna podesavanja - kratko (npr 45sec) svetlo, podrazumevanog intenziteta.

// variables
bool isLightOn;              // da li je svetlo upaljeno ili ne
int backlightLimit;          // granica pozadinskog osvetljenja ...
bool isWiFiOn = false;       // da li wifi i veb server treba da budu ukljuceni
ulong msBtnStart = 0;        // millis za pocetak pritiska na taster
ulong msLastServerAction;    // millis za poslednju akciju sa veb serverom (pokretanje ili ucitavanje neke stranice)
ulong msLastShortClick = 0;  // vreme poslednjeg kratkog klika
ulong msModLastSet = 0;      // vreme pokretanja moda (dugo i/ili slabo svetlo) ili njegovog poslednjeg koriscenja
ulong msLastPir = 0;         // poslednji put kada je signal sa PIRa bio HIGH
ulong msStartPir = 0;        // pocetak HIGH PIR signala pri dovoljno jakom pozadinskom osvetljenju
ulong msLastLightChange = 0; // poslednja promena svetla: on->off ili off->on
ulong cntFastOnOff = 0;      // broj uzastopnih brzih promena svetla
uint consecPirs = 0;         // broj uzastopnih pinPIR HIGH vrednosti
int cntShortClicks = 0;      // broj kratkih klikova u seriji
bool isLongLight = false;    // da li se je trajanje svetla dugo (npr 5min) ili kratko/obicno (npr 45sec)
bool isLight2 = false;       // da li se koristi drugo svetlo (drugi intenzitet, slabije npr)
int i = 0;

void newMsg(const String &s) { EasyFS::addf(String(millis()) + " - " + s); }
void newMsg(const String &s, int x) { EasyFS::addf(String(millis()) + " - " + s + "=" + String(x)); }

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

// Postavljanje svetla na ON(true) ili OFF(false)
void SetLight(bool isOn)
{
    int lvl = isLight2 ? setts.lightLevel2 : setts.lightLevel;
    lvl = map(lvl, 0, setts.MAX_LEVEL, 0, PWMRANGE);
    analogWrite(pinLight, isOn ? lvl : 0);
    backlightLimit = isOn ? setts.backlightLimitHigh : setts.backlightLimitLow;
    ulong ms = millis();
    if (isLightOn && !isOn && (isLongLight || isLight2)) // svetlo se upravo gasi
        msModLastSet = ms;

    if (isLightOn != isOn)
    {
        isLightOn = isOn;
        // kôd koji brani brzo pali-gasenje svetla zbog preblizu postavljenih granica setts.backlight...
        if (ms < msLastLightChange + 1000)
        {
            cntFastOnOff++;
            if (cntFastOnOff >= 2) // setts.backlight... granice se razmicu ako se dese 2 brza ON<->OFF prelaza
            {
                const ulong step = 10;
                if (setts.backlightLimitHigh + step < setts.MAX_LEVEL * 0.9)
                    setts.backlightLimitHigh += step;
                else if (setts.backlightLimitLow - step >= 0)
                    setts.backlightLimitLow -= step;
                cntFastOnOff = 0;
                setts.saveToFile();
            }
        }
        else
            cntFastOnOff = 0;
        msLastLightChange = ms;
    }
}

void WebServerStart();

// Konektovanje na WiFi i postavljanje IP adrese.
void WiFiOn()
{
    SetLight(true);
    WiFi.mode(WIFI_STA);
    ConnectToWiFi();
    SetupIPAddress(40);
    WebServerStart();
    isWiFiOn = true;
}

// Diskonektovanje sa WiFi-a.
void WiFiOff()
{
    isWiFiOn = false;
    server.stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(100);
}

void ReadConfigFile()
{
    msLastServerAction = millis();
    setts.load();
    backlightLimit = setts.backlightLimitLow;
}

void HandleSaveConfig()
{
    // lightOn=6&backlightLimitLow=200&backlightLimitHigh=400&wifiOn=2200...
    msLastServerAction = millis();
    setts.saveToMem(server);
    setts.saveToFile();
    backlightLimit = setts.backlightLimitLow;
    SendEmptyText(server);
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
        SendEmptyText(server);
}

void HandleLdrVals()
{
    msLastServerAction = millis();
    server.send(200, "text/html", ldrs.PrintAll());
}

// prikaz poruka za debagovanje sistema
void HandleMsgs()
{
    msLastServerAction = millis();
    server.send(200, "text/plain", EasyFS::readf());
}

void HandleOTA()
{
    server.send(200, "text/plain", "ESP is waiting for OTA updates...");
    msLastServerAction = millis();
    isOtaOn = true;
    ArduinoOTA.begin();
}

void HandleNotFound()
{
    String s = "Page Not Found\n\n";
    s += "URI: " + server.uri();
    s += "\nMethod: " + String((server.method() == HTTP_GET) ? "GET" : "POST");
    s += "\nArguments:\n";
    for (int i = 0; i < server.args(); i++)
        s += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    server.send(404, "text/plain", s);
}

// Konfiguracija veb servera i njegovo pokretanje.
void WebServerStart()
{
    server.on("/inc/wifi_light_128.png", []() { HandleDataFile(server, "/inc/wifi_light_128.png", "image/png"); });
    server.on("/test", []() { server.send(200, "text/html", "<h1>Test: You are connected</h1>"); });
    server.on("/", []() { HandleDataFile(server, "/index.html", "text/html"); });
    server.on("/inc/index.js", []() { HandleDataFile(server, "/inc/index.js", "text/javascript"); });
    server.on("/inc/style.css", []() { HandleDataFile(server, "/inc/style.css", "text/css"); });
    server.on(ei.getFileName(), []() { HandleDataFile(server, ei.getFileName(), "text/x-csv"); });
    server.on("/save_config", HandleSaveConfig);
    server.on("/current_data.html", []() { HandleDataFile(server, "/current_data.html", "text/html"); });
    server.on("/inc/current_data.js", []() { HandleDataFile(server, "/inc/current_data.js", "text/javascript"); });
    server.on("/get_status", HandleGetStatus);
    server.on("/get_ldr_vals", HandleLdrVals);
    server.on("/msgs", HandleMsgs);
    server.on("/otaUpdate", HandleOTA);
    server.onNotFound(HandleNotFound);
    server.begin();
    msLastStatus = msLastServerAction = millis();
}

void setup()
{
    pinMode(pinPIR, INPUT);
    pinMode(pinPhotoRes, INPUT);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, true);
    pinMode(pinLight, OUTPUT);

    Serial.begin(115200);
    LittleFS.begin();
    ReadConfigFile();
    EasyFS::setFileName("/msgs.log", true);

    WiFiOn();
}

void loop()
{
    delay(setts.msMainDelay);
    ulong ms = millis();

    if (isOtaOn)
    {
        if (ms > msLastServerAction + itvOtaTime)
            isOtaOn = false;
        ArduinoOTA.handle();
        return;
    }

    int valPir = digitalRead(pinPIR);
    consecPirs = valPir ? consecPirs + 1 : 0;

    if (consecPirs >= setts.minHighPIRs) // HIGH na PIR-u se prihvata samo ako je ta vrednost x puta zaredom ocitana
        msLastPir = ms;
    if (!valPir && msStartPir != 0)
        msStartPir = msLastPir = 0;

    int valPhotoRes = map(analogRead(pinPhotoRes), 0, 1023, 0, setts.MAX_LEVEL);
    valPhotoRes = ldrs.Add(valPhotoRes);

    if (valPhotoRes > backlightLimit) // prostorija je dovoljno osvetljena
    {
        if (consecPirs == setts.minHighPIRs) // pokret zapocet kada je prostorija dovoljno osvetljena
            msStartPir = ms;
        SetLight(false);
    }
    else // prostorija nije dovoljno osvetljena
    {
        bool x = (!isLongLight && ms < msLastPir + 1000 * setts.lightOn) || (isLongLight && ms < msLastPir + 60 * 1000 * setts.longLightOn);
        if (msLastPir != 0 && msStartPir == 0 && x)
            SetLight(true);
        else
            SetLight(false);
    }
    ms = millis();
    if (valPhotoRes > 0.95 * setts.MAX_LEVEL) // ako je pritisnut taster nabudzen sa LDRom
    {
        if (msBtnStart == 0)
            msBtnStart = ms;                          // pamti se pocetak pritiska na taster
        else if (ms > msBtnStart + 1000 && !isWiFiOn) // ako se taster drzi vise od sekunde
            WiFiOn();
    }
    else if (msBtnStart != 0) // ako je upravo otpusten taster nabudzen sa LDRom
    {
        if (ms - msBtnStart < 1000) // ako je kratak klik
        {
            cntShortClicks++;
            msLastShortClick = ms;
        }
        msBtnStart = 0;
    }
    // ako taster ostaje otpusten, a bilo je skorasnjih klikova
    else if (msLastShortClick != 0 && ms > msLastShortClick + 1000)
    {
        if (cntShortClicks == 1)
            isLongLight = isLight2 = false;
        if (cntShortClicks == 2)
            isLongLight = true;
        if (cntShortClicks == 3)
            isLight2 = true;
        msModLastSet = (isLongLight || isLight2) ? ms : 0;
        msLastShortClick = cntShortClicks = 0;
    }

    if (!isLightOn && msModLastSet != 0 && ms > msModLastSet && ms - msModLastSet > itvModWait)
    {
        isLongLight = isLight2 = false;
        msModLastSet = 0;
    }

    digitalWrite(pinLed, !isWiFiOn);
    if (isWiFiOn)
    {
        if (ms - msLastStatus > 1000)
        {
            AddStatusString(idStatus++, valPhotoRes, (ms - msLastPir) / 1000, isLightOn);
            msLastStatus = ms;
        }

        // da li je vreme da se iskljuci WiFi tj. veb server
        if (setts.wifiOn != 0 && ms > msLastServerAction + setts.wifiOn * 60 * 1000)
            WiFiOff();

        server.handleClient();
    }
}
