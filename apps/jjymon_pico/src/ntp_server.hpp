#pragma once

#ifdef BOARD_PICO_W

#include <stdint.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>

#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/timeouts.h>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <lwip/dhcp.h>

namespace shapoco::jjymon {

// todo: メンバ変数化
static uint64_t lastTsMs = 0;
static jjy::JjyDateTime lastJjyDateTime;

struct __attribute__ ((__packed__)) ntp_packet {
    uint8_t li_vn_mode;       // Eight bits. li, vn, and mode.
    uint8_t stratum;          // Eight bits. Stratum level of the local clock.
    uint8_t poll;             // Eight bits. Maximum interval between successive messages.
    int8_t precision;         // Eight bits. Precision of the local clock.
    uint32_t rootDelay;       // 32 bits. Total round trip delay time.
    uint32_t rootDispersion;  // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;           // 32 bits. Reference clock identifier.
    uint32_t refTm_s;         // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;         // 32 bits. Reference time-stamp fraction of a second.
    uint32_t origTm_s;        // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;        // 32 bits. Originate time-stamp fraction of a second.
    uint32_t rxTm_s;          // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;          // 32 bits. Received time-stamp fraction of a second.
    uint32_t txTm_s;          // 32 bits. Transmit time-stamp seconds.
    uint32_t txTm_f;          // 32 bits. Transmit time-stamp fraction of a second.
};

static uint32_t swapBytes(uint32_t w) {
    return
        ((w >> 24) & 0xfful) |
        ((w >> 8) & 0xff00ul) |
        ((w << 8) & 0xff0000ul) |
        ((w << 24) & 0xff000000ul);
}

class NtpServer {
public:
    int port = 123;
    bool cyw43InitOk = false;
    bool wifiConnected = false;

    struct udp_pcb *ntp_pcb;

    void init(int port = 123) {
        this->port = port;

        if (cyw43_arch_init()) {
            printf("Wi-Fi init failed");
            return;
        }
        cyw43InitOk = true;

        cyw43_arch_enable_sta_mode();

        printf("Connecting to Wi-Fi...\n");
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            printf("failed to connect.\n");
            return;
        }
        wifiConnected = true;
        printf("Connected.\n");

        ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
        if (!ntp_pcb) {
            printf("Failed to create UDP PCB\n");
            return;
        }

        err_t err = udp_bind(ntp_pcb, IP_ANY_TYPE, port);
        if (err != ERR_OK) {
            printf("Failed to bind UDP PCB: %d\n", err);
            udp_remove(ntp_pcb);
            return;
        }

        udp_recv(ntp_pcb, ntp_receive, nullptr);
    }

    void service(uint64_t nowMs, jjy::JjyDateTime &now) {
        lastTsMs = nowMs;
        lastJjyDateTime = now;
        if (!cyw43InitOk) return;
        cyw43_arch_poll();
    }

    static void ntp_receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
        if (p->len != sizeof(ntp_packet)) {
            pbuf_free(p);
            return;
        }
        
        ntp_packet *ntp_data = (ntp_packet *)p->payload;
        ntp_data->origTm_s = ntp_data->txTm_s;
        ntp_data->origTm_f = ntp_data->txTm_f;

        uint32_t leapIndicator = 0;
        uint32_t versionNumber = 4;
        uint32_t mode = 4;
        ntp_data->li_vn_mode = (leapIndicator << 6) | (versionNumber << 3) | mode;
        ntp_data->stratum = 1;
        ntp_data->poll = 0;
        ntp_data->precision = -9;
        ntp_data->rootDelay = 0;
        ntp_data->rootDispersion = 0;
        ntp_data->refId = 0x4a4a4920;
        
        uint64_t jjyNow = lastJjyDateTime.toNtpTime();
        jjyNow += 0x100000000ull * (time_us_64() / 1000 - lastTsMs) / 1000;
        uint32_t jjyNowS = swapBytes((jjyNow >> 32) & 0xfffffffful);
        uint32_t jjyNowF = swapBytes(jjyNow & 0xfffffffful);

        ntp_data->refTm_s = jjyNowS;
        ntp_data->refTm_f = jjyNowF;
        ntp_data->rxTm_s = jjyNowS;
        ntp_data->rxTm_f = jjyNowF;
        ntp_data->txTm_s = jjyNowS;
        ntp_data->txTm_f = jjyNowF;
        
        udp_sendto(pcb, p, addr, port);
        pbuf_free(p);
    }

};

}

#endif
