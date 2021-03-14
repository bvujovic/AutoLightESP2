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
    uint lightOn;           // koliko je sekundi svetlo upaljeno posle poslednjeg signala sa PIR-a
    uint longLightOn;       // koliko je minuta dugo svetlo upaljeno posle poslednjeg signala sa PIR-a
    uint lightLevel = 100;   // jacina LED svetla (0-100)
    uint lightLevel2 = 100;  // jacina drugog/sekundarnog LED svetla (0-100)
    uint backlightLimitLow;  // granica pozadinskog osvetljenja iznad koje se svetlo ne pali
    uint backlightLimitHigh; // granica pozadinskog osvetljenja ispod koje se svetlo ne gasi
    uint wifiOn;             // broj minuta za koje wifi ostaje ukljucen; 0 -> wifi je uvek ukljucen
    uint msMainDelay;        // broj milisekundi delay-a u loop-u
    uint lightOutDelay;      // delay u ms posle gasenja svetla
    uint minHighPIRs;        // minimalan broj uzastopnih PIR HIGH signala da bi se to prihvatilo kao detektovani pokret

    const uint MAX_LEVEL = 100; // maksimalni nivo osvetljenja

    int photoInterval; // Cekanje (u sec) izmedju 2 slikanja.
    void setLightOn(uint x) { lightOn = constrain(x, 1, 300); }
    void setLongLightOn(uint x) { longLightOn = constrain(x, 1, 30); }
    void setLightLevel(uint x) { lightLevel = constrain(x, 1, MAX_LEVEL); }
    void setLightLevel2(uint x) { lightLevel2 = constrain(x, 1, MAX_LEVEL); }
    void setBacklightLimitLow(uint x) { backlightLimitLow = constrain(x, 1, MAX_LEVEL); }
    void setBacklightLimitHigh(uint x) { backlightLimitHigh = constrain(x, 1, MAX_LEVEL); }
    void setWifiOn(uint x) { wifiOn = constrain(x, 0, 30); }
    void setLightOutDelay(uint x) { lightOutDelay = constrain(x, 0, 2000); }
    void setMinHighPIRs(uint x) { minHighPIRs = constrain(x, 1, 100); }
    void setMsMainDelay(uint x) { msMainDelay = constrain(x, 5, 500); }

    const char *getFileName() { return fileName; }

    // Ucitavanje podesavanja iz .ini fajla.
    bool loadSetts();
    // Cuvanje podesavanja u .ini fajl.
    void saveSetts(ESP8266WebServer &srv);
};
