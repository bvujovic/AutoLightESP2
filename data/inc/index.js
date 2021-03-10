
const TEST = false;

const sepRows = '\n';
const sepProps = '=';

class Config {
    constructor(name, value) {
        this.name = name;
        this.value = value;
    }

    toString() {
        return this.name + '=' + this.value;
    }
}

var configs = [];

function GetConfig() {
    if (TEST) {
        const resp =
            `lightOn=6
lightLevel=255
backlightLimitLow=200
backlightLimitHigh=400
wifiOn=1234
lightOutDelay=200
minHighPIRs=3
msMainDelay=12`;
        ParseConfig(resp);
    }
    else {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200)
                ParseConfig(this.responseText);
        };
        xhttp.open('GET', 'config.ini', true); xhttp.send();
    }
}

function ParseConfig(resp) {
    var a = resp.split(sepRows);
    for (var s of a) {
        s = s.trim();
        if (s.length === 0) continue;
        const props = s.split(sepProps);
        configs.push(new Config(props[0], props[1]));
    }
    DisplayConfig();
}

//B function docel(id) { return document.getElementById(id); }

function DisplayConfig() {
    numLightOn.value = ConfValue('lightOn');
    numLightLevel.value = ConfValue('lightLevel');
    numBacklightLimitLow.value = ConfValue('backlightLimitLow');
    numBacklightLimitHigh.value = ConfValue('backlightLimitHigh');
    numWifiOn.value = ConfValue('wifiOn');
    numLightOutDelay.value = ConfValue('lightOutDelay');
    numMinHighPIRs.value = ConfValue('minHighPIRs');
    numMsMainDelay.value = ConfValue('msMainDelay');
}

function ConfValue(confName) {
    for (const c of configs)
        if (c.name == confName)
            return c.value;
    return '';
}

function SaveConfig() {
    const sepParams = '&';
    const confData
        = 'lightOn' + sepProps + numLightOn.value + sepParams
        + 'lightLevel' + sepProps + numLightLevel.value + sepParams
        + 'backlightLimitLow' + sepProps + numBacklightLimitLow.value + sepParams
        + 'backlightLimitHigh' + sepProps + numBacklightLimitHigh.value + sepParams
        + 'wifiOn' + sepProps + numWifiOn.value + sepParams
        + 'lightOutDelay' + sepProps + numLightOutDelay.value + sepParams
        + 'minHighPIRs' + sepProps + numMinHighPIRs.value + sepParams
        + 'msMainDelay' + sepProps + numMsMainDelay.value;

    if (TEST)
        console.log(confData);
    else {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200)
                alert("Your data has been saved.");
        };
        xhttp.open('GET', 'save_config?' + confData, true); xhttp.send();
    }
}