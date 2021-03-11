#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <EasyINI.h>

// Podesavanja sacuvana u config fajlu.
class Setts
{
private:
    EasyINI *ei;
    const char *fileName = "/config.ini";

public:
    int lightOn;            // koliko je sekundi svetlo upaljeno posle poslednjeg signala sa PIR-a
    int longLightOn;        // koliko je minuta dugo svetlo upaljeno posle poslednjeg signala sa PIR-a
    int lightLevel = 100;   // jacina LED svetla (0-100)
    int backlightLimitLow;  // granica pozadinskog osvetljenja iznad koje se svetlo ne pali
    int backlightLimitHigh; // granica pozadinskog osvetljenja ispod koje se svetlo ne gasi
    int wifiOn;             // broj minuta za koje wifi ostaje ukljucen; 0 -> wifi je uvek ukljucen
    int msMainDelay;        // broj milisekundi delay-a u loop-u
    int lightOutDelay;      // delay u ms posle gasenja svetla
    int minHighPIRs;        // minimalan broj uzastopnih PIR HIGH signala da bi se to prihvatilo kao detektovani pokret

    const int MAX_LEVEL = 100; // maksimalni nivo osvetljenja

    int photoInterval; // Cekanje (u sec) izmedju 2 slikanja.
    void setLightOn(int x) { lightOn = constrain(x, 1, 300); }
    void setLongLightOn(int x) { longLightOn = constrain(x, 1, 30); }
    void setLightLevel(int x) { lightLevel = constrain(x, 1, MAX_LEVEL); }
    void setBacklightLimitLow(int x) { backlightLimitLow = constrain(x, 1, MAX_LEVEL); }
    void setBacklightLimitHigh(int x) { backlightLimitHigh = constrain(x, 1, MAX_LEVEL); }
    void setWifiOn(int x) { wifiOn = constrain(x, 0, 30); }
    void setLightOutDelay(int x) { lightOutDelay = constrain(x, 0, 2000); }
    void setMinHighPIRs(int x) { minHighPIRs = constrain(x, 1, 100); }
    void setMsMainDelay(int x) { msMainDelay = constrain(x, 5, 500); }

    const char *getFileName() { return fileName; }

    // Ucitavanje podesavanja iz .ini fajla.
    bool loadSetts();
    // Cuvanje podesavanja u .ini fajl.
    void saveSetts(ESP8266WebServer &srv);
};
