const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

const int TIMEZONE_OFFSET = 3600 * 3;


void sendNTPpacket(const char *address, EthernetUDP &Udp) {
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    Udp.beginPacket(address, 123); // NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

uint16_t getTimeFromNTP(EthernetUDP &Udp) {
    sendNTPpacket(timeServer, Udp);
    delay(300);
    if (Udp.parsePacket()) {
        if (Udp.available() >= NTP_PACKET_SIZE) {
            Udp.read(packetBuffer, NTP_PACKET_SIZE);

            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
            unsigned long secsSince1900 = highWord << 16 | lowWord;

            const unsigned long seventyYears = 2208988800UL;
            unsigned long epoch = secsSince1900 - seventyYears;
            epoch += TIMEZONE_OFFSET;
            epoch /= 60;
            byte minute = epoch % 60;
            epoch /= 60;
            byte hour = epoch % 24;
            return hour * 60 + minute;
        }
    }
    return -1;
}

