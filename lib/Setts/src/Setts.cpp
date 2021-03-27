#include "Setts.h"

bool Setts::load()
{
    ei = new EasyINI(fileName);
    if (ei->open(FMOD_READ))
    {
        //B lightOn = ei->getInt("lightOn");
        setLightOn(ei->getInt("lightOn"));
        setRememberMode(ei->getInt("rememberMode"));
        setLongLightOn(ei->getInt("longLightOn"));
        setLightLevel(ei->getInt("lightLevel"));
        setLightLevel2(ei->getInt("lightLevel2"));
        setBacklightLimitLow(ei->getInt("backlightLimitLow"));
        setBacklightLimitHigh(ei->getInt("backlightLimitHigh"));
        setWifiOn(ei->getInt("wifiOn"));
        setMsMainDelay(ei->getInt("msMainDelay"));
        setMinHighPIRs(ei->getInt("minHighPIRs"));
        setHttpServerUpdate(ei->getInt("httpServerUpdate"));
        ei->close();
        return true;
    }
    else
        return false;
}

void Setts::saveToMem(ESP8266WebServer &server)
{
    // lightOn=41&lightLevel=100&backlightLimitLow=21&backlightLimitHigh=51&wifiOn=121&lightOutDelay=500&minHighPIRs=10&msMainDelay=10
    setLightOn(server.arg("lightOn").toInt());
    setRememberMode(server.arg("rememberMode").toInt());
    setLongLightOn(server.arg("longLightOn").toInt());
    setLightLevel(server.arg("lightLevel").toInt());
    setLightLevel2(server.arg("lightLevel2").toInt());
    setBacklightLimitLow(server.arg("backlightLimitLow").toInt());
    setBacklightLimitHigh(server.arg("backlightLimitHigh").toInt());
    setWifiOn(server.arg("wifiOn").toInt());
    setMsMainDelay(server.arg("msMainDelay").toInt());
    setMinHighPIRs(server.arg("minHighPIRs").toInt());
    // setHttpServerUpdate() ovde nije potreban
}

void Setts::saveToFile()
{
    if (ei->open(FMOD_WRITE))
    {
        ei->setInt("lightOn", lightOn);
        ei->setInt("rememberMode", rememberMode);
        ei->setInt("longLightOn", longLightOn);
        ei->setInt("lightLevel", lightLevel);
        ei->setInt("lightLevel2", lightLevel2);
        ei->setInt("backlightLimitLow", backlightLimitLow);
        ei->setInt("backlightLimitHigh", backlightLimitHigh);
        ei->setInt("wifiOn", wifiOn);
        ei->setInt("msMainDelay", msMainDelay);
        ei->setInt("minHighPIRs", minHighPIRs);
        ei->setInt("httpServerUpdate", httpServerUpdate);
        ei->close();
    }
}