
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
    uint8_t buffer[sizeof(struct settings)];
    memcpy(&buffer, &s, sizeof(s));
    for (int i = 0; i < sizeof(s); i++) {
        EEPROM[i] = buffer[i];
    }
}

struct settings getSettings() {
    uint8_t buffer[sizeof(struct settings)];
    struct settings read_settings{};
    for (int i = 0; i < sizeof(read_settings); i++) {
        buffer[i] = EEPROM[i];
    }
    memcpy(&read_settings, &buffer, sizeof(read_settings));
    return read_settings;
}

void printSettings(struct settings s){
    Serial.print("If poliv enabled: ");
    Serial.println((int)s.enabled);
    Serial.print("Count of rules: ");
    Serial.println(s.count);
    Serial.println();
    for (int i = 0; i < s.count; ++i) {
        Serial.print("Port: ");
        Serial.println(s.schedule[i].port);
        Serial.print("Time: ");
        Serial.println(s.schedule[i].time_i);
        Serial.print("if enabled: ");
        Serial.println((int)s.schedule[i].enabled);

    }
}

void getSettingsFromWeb() {
    uint8_t buffer[sizeof(struct settings)];
    if (httpRequests.connect("yank0vy3rdna.ru", 80)) { // todo send data to server
        httpRequests.println("GET /poliv/settings_poliv.bin HTTP/1.1");
        httpRequests.println("Host: yank0vy3rdna.ru");
        httpRequests.println("Connection: close");
        httpRequests.println();

        while (!httpRequests.available()) {}
        while (httpRequests.readStringUntil('\n').length() != 1) {}
        if (httpRequests.available() >= sizeof(struct settings)){
            httpRequests.readBytes(buffer, sizeof(struct settings));
            auto *eh = (struct settings *) buffer;
            setSettings(*eh);
        }

        httpRequests.stop();
    }
}