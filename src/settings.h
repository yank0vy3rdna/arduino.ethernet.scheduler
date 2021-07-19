#include <avr/eeprom.h>

EthernetClient httpRequests;


struct time_port {
    uint16_t time_i;
    unsigned char port;
    unsigned char enabled;
};

struct settings {
    unsigned char enabled;
    unsigned char count;
    struct time_port schedule[40];
};

void setSettings(struct settings s) {
    eeprom_write_block((void*)&s, (void*)20, sizeof(s));
}

struct settings getSettings() {
    struct settings s{};
    eeprom_read_block((void*)&s, (void*)20, sizeof(s));
    return s;
}

void printSettings(struct settings s) {
    Serial.print("If poliv enabled: ");
    Serial.println((int) s.enabled);
    Serial.print("Count of rules: ");
    Serial.println(s.count);
    Serial.println();
    for (int i = 0; i < s.count; ++i) {
        Serial.print("Port: ");
        Serial.println(s.schedule[i].port);
        Serial.print("Time: ");
        Serial.println(s.schedule[i].time_i);
        Serial.print("if enabled: ");
        Serial.println((int) s.schedule[i].enabled);

    }
}

void getSettingsFromWeb(void (*printState)(EthernetClient &)) {
    uint8_t buffer[sizeof(struct settings)];
    if (httpRequests.connect("yank0vy3rdna.ru", 80)) {
        httpRequests.print("GET /poliv/get_settings/");
        httpRequests.print("asdfgerjgh1kjbisgudfhgkujsg3df");
        httpRequests.print('/');
        printState(httpRequests);
        httpRequests.print(' ');
        httpRequests.println("HTTP/1.1");
        httpRequests.println("Host: 192.168.0.115");
        httpRequests.println("Connection: close");
        httpRequests.println();
        int counter = 0;
        while (!httpRequests.available()) {
            counter++;
            if (counter > 2000) break;
        }
        while (httpRequests.readStringUntil('\n').length() != 1) {
            counter++;
            if (counter > 2000) break;
        }
        if (httpRequests.available() >= sizeof(struct settings)) {
            httpRequests.readBytes(buffer, sizeof(struct settings));
            auto *eh = (struct settings *) buffer;
            setSettings(*eh);
        }
        httpRequests.stop();
    }
}