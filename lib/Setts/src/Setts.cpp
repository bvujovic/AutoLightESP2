#include "Setts.h"

bool Setts::loadSetts()
{
    ei = new EasyINI(fileName);
    if (ei->open(FMOD_READ))
    {
        lightOn = ei->getInt("lightOn");
        longLightOn = ei->getInt("longLightOn");
        lightLevel = ei->getInt("lightLevel");
        lightLevel2 = ei->getInt("lightLevel2");
        backlightLimitLow = ei->getInt("backlightLimitLow");
        backlightLimitHigh = ei->getInt("backlightLimitHigh");
        wifiOn = ei->getInt("wifiOn");
        msMainDelay = ei->getInt("msMainDelay");
        lightOutDelay = ei->getInt("lightOutDelay");
        minHighPIRs = ei->getInt("minHighPIRs");
        ei->close();
        return true;
    }
    else
        return false;
}

void Setts::saveSetts(ESP8266WebServer &server)
{
    if (ei->open(FMOD_WRITE))
    {
        // lightOn=41&lightLevel=100&backlightLimitLow=21&backlightLimitHigh=51&wifiOn=121&lightOutDelay=500&minHighPIRs=10&msMainDelay=10
        setLightOn(server.arg("lightOn").toInt());
        setLongLightOn(server.arg("longLightOn").toInt());
        setLightLevel(server.arg("lightLevel").toInt());
        setLightLevel2(server.arg("lightLevel2").toInt());
        setBacklightLimitLow(server.arg("backlightLimitLow").toInt());
        setBacklightLimitHigh(server.arg("backlightLimitHigh").toInt());
        setWifiOn(server.arg("wifiOn").toInt());
        setMsMainDelay(server.arg("msMainDelay").toInt());
        setLightOutDelay(server.arg("lightOutDelay").toInt());
        setMinHighPIRs(server.arg("minHighPIRs").toInt());

        ei->setInt("lightOn", lightOn);
        ei->setInt("longLightOn", longLightOn);
        ei->setInt("lightLevel", lightLevel);
        ei->setInt("lightLevel2", lightLevel2);
        ei->setInt("backlightLimitLow", backlightLimitLow);
        ei->setInt("backlightLimitHigh", backlightLimitHigh);
        ei->setInt("wifiOn", wifiOn);
        ei->setInt("msMainDelay", msMainDelay);
        ei->setInt("lightOutDelay", lightOutDelay);
        ei->setInt("minHighPIRs", minHighPIRs);
        ei->close();
    }
}