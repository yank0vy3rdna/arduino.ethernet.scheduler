#include "Arduino.h"

#include <Ethernet.h>

#include "time_eth.h"
#include "settings.h"
#include "time_clock.h"
#include <avr/wdt.h>

#define FIRST_PORT 6
#define COUNT_PORTS 2

byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 177);

unsigned int localPort = 8888;



void setup() {
    rtc.begin();

    Serial.begin(9600);

    for (int i = FIRST_PORT; i < FIRST_PORT + COUNT_PORTS; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, HIGH);
    }

    EthernetClass::begin(mac, ip);

    if (EthernetClass::linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
    }

    Serial.print("server is at ");
    Serial.println(EthernetClass::localIP());

    Serial.println("Settings on start: ");
    printSettings(getSettings());

    EthernetUDP Udp;

    Udp.begin(localPort);

    uint16_t ntp_time = getTimeFromNTP(Udp);
    if (ntp_time != (uint16_t) -1) {
        setTimeClock(ntp_time / 60, ntp_time % 60);
    }
    Udp.stop();

    Serial.print("Current ntp time: ");
    Serial.println(ntp_time);
    Serial.print("Current clock time: ");
    Serial.println(getTimeClock());
    Serial.println("Wait 5 sec..");
    delay(5000);
    wdt_enable(WDTO_8S);
}

uint16_t getTime() {
    uint16_t time = getTimeClock();
    return time;
}


bool states[COUNT_PORTS];

void stateMachine() {
    struct settings s = getSettings();

    if (s.enabled) {
        uint16_t now_time = getTime();
        for (int i = 0; i < s.count; i++) {
            if (now_time >= s.schedule[i].time_i and s.schedule[i].port >= FIRST_PORT and
                s.schedule[i].port < FIRST_PORT + COUNT_PORTS) {
                states[s.schedule[i].port - FIRST_PORT] = s.schedule[i].enabled;
            }
        }
    } else {
        for (bool &i : states) {
            i = false;
        }
    }
    for (int i = FIRST_PORT; i < FIRST_PORT + COUNT_PORTS; i++) {
        if (states[i - FIRST_PORT]) {
            digitalWrite(i, LOW);
        } else {
            digitalWrite(i, HIGH);
        }
    }
}

void sendState(EthernetClient &client) {
    for (int i = FIRST_PORT; i < FIRST_PORT + COUNT_PORTS; i++) {
        client.print('p');
        client.print(i);
        client.print('=');
        client.print(states[i - FIRST_PORT] ? "yes" : "no");
        client.print('&');
    }
    client.print("arduinotime=");
    client.print(getTime());
    client.print('&');
}

void loop() {
    stateMachine();
    getSettingsFromWeb(sendState);
    wdt_reset();
    delay(5000);
    wdt_reset();
    delay(5000);
    wdt_reset();
    delay(5000);
    wdt_reset();
    delay(5000);
}