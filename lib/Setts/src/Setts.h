#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <EasyINI.h>

// Podesavanja sacuvana u config fajlu.
class Setts
{
private:
    EasyINI *ei;
    // const int minPhotoInterval = 5;
    // const int maxImageResolution = 10;
    // const int minBrightness = -2;
    // const int maxBrightness = 2;
    // const int maxGain = 6;
    // const int maxPhotoWait = 10;
    const char *fileName = "/config.ini";

public:
    int lightOn;            // koliko je sekundi svetlo upaljeno posle poslednjeg signala sa PIR-a
    int lightLevel = 100;   // jacina LED svetla (0-100)
    int backlightLimitLow;  // granica pozadinskog osvetljenja iznad koje se svetlo ne pali
    int backlightLimitHigh; // granica pozadinskog osvetljenja ispod koje se svetlo ne gasi
    int wifiOn;             // broj sekundi za koje wifi ostaje ukljucen; 0 -> wifi je uvek ukljucen
    int msMainDelay;        // broj milisekundi delay-a u loop-u
    int lightOutDelay;      // delay u ms posle gasenja svetla
    int minHighPIRs;        // minimalan broj uzastopnih PIR HIGH signala da bi se to prihvatilo kao detektovani pokret

    const int MAX_LEVEL = 100; // maksimalni nivo osvetljenja

    int photoInterval; // Cekanje (u sec) izmedju 2 slikanja.
    void setLightOn(int x) { lightOn = constrain(x, 1, 3600); }

    void setLightLevel(int x) { lightLevel = constrain(lightLevel, 1, MAX_LEVEL); }
    void setBacklightLimitLow(int x) { backlightLimitLow = constrain(backlightLimitLow, 1, MAX_LEVEL); }
    void setBacklightLimitHigh(int x) { backlightLimitHigh = constrain(backlightLimitHigh, 1, MAX_LEVEL); }
    void setWifiOn(int x) { wifiOn = constrain(wifiOn, 0, 3600); }
    void setLightOutDelay(int x) { lightOutDelay = constrain(lightOutDelay, 0, 2000); }
    void setMinHighPIRs(int x) { minHighPIRs = constrain(minHighPIRs, 1, 100); }
    void setMsMainDelay(int x) { msMainDelay = constrain(msMainDelay, 5, 500); }

    const char *getFileName() { return fileName; }

    // Ucitavanje podesavanja iz .ini fajla.
    bool loadSetts();
    // Cuvanje podesavanja u .ini fajl.
    void saveSetts(ESP8266WebServer &srv);
};
