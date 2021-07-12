#include "Arduino.h"

#include <EEPROM.h>
#include <Ethernet.h>

#include "time_eth.h"
#include "settings.h"
#include "time_clock.h"

#define FIRST_PORT 6
#define COUNT_PORTS 2

byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 177);

unsigned int localPort = 8888;

EthernetUDP Udp;


void setup() {
    rtc.begin();

    // Open serial communications and wait for port to open:
    Serial.begin(9600);
//    printSettings(s);

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


    Udp.begin(localPort);
}

uint64_t counter_of_time = 0;

uint16_t getTime() {
    uint16_t time = getTimeClock();
    if (counter_of_time >= 60000) {
        uint16_t ntp_time = getTimeFromNTP(Udp);
        if (time != (uint16_t) -1) {
            setTimeClock(ntp_time / 60, ntp_time % 60);
            counter_of_time = 0;
        }
    }
    counter_of_time++;
    return time;
}


bool states[COUNT_PORTS];

void stateMachine() {
    struct settings s = getSettings();
    bool changed[COUNT_PORTS];

    for (bool &i : changed) {
        i = false;
    }
    if (s.enabled) {
        uint16_t now_time = getTime();
        for (int i = 0; i < s.count; i++) {
            if (now_time >= s.schedule[i].time_i and s.schedule[i].port >= FIRST_PORT and
                s.schedule[i].port < FIRST_PORT + COUNT_PORTS) {
                if (states[s.schedule[i].port - FIRST_PORT] != s.schedule[i].enabled)
                    changed[s.schedule[i].port - FIRST_PORT] = 1;
                states[s.schedule[i].port - FIRST_PORT] = s.schedule[i].enabled;
            }
        }
    } else{
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
}

void loop() {
    stateMachine();
    getSettingsFromWeb(sendState);
    EthernetClass::maintain();
}