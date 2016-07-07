#pragma once
#pragma pack(1)

#include <string.h>

/*
 * Config
 */
#define WITH_ICMP

#define MAC_ADDR			{0x00,0x4e,0x42,0x4c,0x55,0x46}
// In case of external requests, the dest mac address should be the GATEWAY MAC ADDRESS.
#define DEST_MAC_ADDR			{0x54,0x04,0xa6,0x3f,0xaf,0xc1}
//#define DEST_MAC_ADDR			{0x00,0x24,0x1d,0xc6,0x90,0xc5}
//#define DEST_MAC_ADDR			{0xc8,0xd3,0xa3,0x4b,0x78,0x5c}
#define dest_ip_addr			inet_addr(192,168,1,113)
#define IP_ADDR				inet_addr(192,168,1,74)
#define IP_GATEWAY			inet_addr(192,168,1,1)
#define IP_MASK 			inet_addr(255,255,255,0);
#define APP_PORT			37208

#define IP_PACKET_TTL		64

/*
 * BE conversion
 */
#define htons(a)			((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define ntohs(a)			htons(a)

#define htonl(a)			( (((a)>>24)&0xff) | (((a)>>8)&0xff00) |\
								(((a)<<8)&0xff0000) | (((a)<<24)&0xff000000) )
#define ntohl(a)			htonl(a)

#define inet_addr(a,b,c,d)		( ((uint32_t)a) | ((uint32_t)b << 8) |\
								((uint32_t)c << 16) | ((uint32_t)d << 24) )

/*
 * Ethernet
 */
#define ETH_TYPE_ARP		htons(0x0806)
#define ETH_TYPE_IP		htons(0x0800)

typedef struct eth_frame {
	uint8_t to_addr[6];
	uint8_t from_addr[6];
	uint16_t type;
	uint8_t data[];
} eth_frame_t;

/*
 * ARP
 */
#define ARP_HW_TYPE_ETH		htons(0x0001)
#define ARP_PROTO_TYPE_IP	htons(0x0800)

#define ARP_TYPE_REQUEST	htons(1)
#define ARP_TYPE_RESPONSE	htons(2)

typedef struct arp_message {
	uint16_t hw_type;
	uint16_t proto_type;
	uint8_t hw_addr_len;
	uint8_t proto_addr_len;
	uint16_t type;
	uint8_t mac_addr_from[6];
	uint32_t ip_addr_from;
	uint8_t mac_addr_to[6];
	uint32_t ip_addr_to;
} arp_message_t;

/*
 * IP
 */
#define IP_PROTOCOL_ICMP	1
#define IP_PROTOCOL_TCP		6
#define IP_PROTOCOL_UDP		17

typedef struct ip_packet {
	uint8_t ver_head_len;
	uint8_t tos;
	uint16_t total_len;
	uint16_t fragment_id;
	uint16_t flags_framgent_offset;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t cksum;
	uint32_t from_addr;
	uint32_t to_addr;
	uint8_t data[];
} ip_packet_t;

/*
 * ICMP
 */
#define ICMP_TYPE_ECHO_RQ	8
#define ICMP_TYPE_ECHO_RPLY	0

typedef struct icmp_echo_packet {
	uint8_t type;
	uint8_t code;
	uint16_t cksum;
	uint16_t id;
	uint16_t seq;
	uint8_t data[];
} icmp_echo_packet_t;

/*
 * UDP
 */
typedef struct udp_packet {
	uint16_t from_port;
	uint16_t to_port;
	uint16_t len;
	uint16_t cksum;
	uint8_t data[];
} udp_packet_t;

/*
 * LAN
 */
void lan_init();
void lan_poll();

void udp_reply(eth_frame_t *frame, uint16_t len);
uint8_t ip_send(eth_frame_t *frame, uint16_t len);
void eth_send(eth_frame_t *frame, uint16_t len);
uint8_t udp_send(eth_frame_t *frame, uint16_t len);
uint8_t *arp_resolve(uint32_t node_ip_addr);
uint8_t *arp_search_cache(uint32_t node_ip_addr);

void eth_reply(eth_frame_t *frame, uint16_t len);
void eth_send_packet(uint8_t *data, uint16_t len);
uint16_t eth_recv_packet(uint8_t *buf, uint16_t buflen);
void eth_filter(eth_frame_t *frame, uint16_t len);

void arp_filter(eth_frame_t *frame, uint16_t len);
void ip_filter(eth_frame_t *frame, uint16_t len);
void icmp_filter(eth_frame_t *frame, uint16_t len);
void udp_filter(eth_frame_t *frame, uint16_t len);

void ip_reply(eth_frame_t *frame, uint16_t len);
uint16_t ip_cksum(uint32_t sum, uint8_t *buf, uint16_t len);

// Заголовок TCP-пакета
#define TCP_FLAG_URG        0x20
#define TCP_FLAG_ACK        0x10
#define TCP_FLAG_PSH        0x08
#define TCP_FLAG_RST        0x04
#define TCP_FLAG_SYN        0x02
#define TCP_FLAG_FIN        0x01

typedef struct tcp_packet {
    uint16_t from_port;        // порт отправителя
    uint16_t to_port;        // порт получателя
    uint32_t seq_num;        // указатель потока
    uint32_t ack_num;        // указатель подтверждения
    uint8_t data_offset;    // размер заголовка
    uint8_t flags;            // флаги
    uint16_t window;        // размер окна
    uint16_t cksum;            // контрольная сумма
    uint16_t urgent_ptr;    // указатель срочных данных
    uint8_t data[];
} tcp_packet_t;

// рассчитывает размер TCP-заголовка
#define tcp_head_size(tcp)    (((tcp)->data_offset & 0xf0) >> 2)

// вычисляет указатель на данные
#define tcp_get_data(tcp)    ((uint8_t*)(tcp) + tcp_head_size(tcp))

// Коды состояния TCP
typedef enum tcp_status_code {
    TCP_CLOSED,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT
} tcp_status_code_t;

// Состояние TCP
typedef struct tcp_state {
    tcp_status_code_t status;    // стейт
    uint16_t rx_time;            // время когда был получен последний пакет
    uint16_t local_port;        // локальный порт
    uint16_t remote_port;        // порт удалённого узла
    uint32_t remote_addr;        // IP-адрес удалённого узла
} tcp_state_t;

// Инициирует открытие нового соединения
// При ошибке возвращает 0xff.
// При успехе, возвращает идентификатор нового соединения
// Когда соединение будет создано (удалённый узел ответит),
//    будет вызван коллбэк tcp_opened
uint8_t tcp_open(uint32_t addr, uint16_t port, uint16_t local_port);

// Отправляет данные. Данные можно отправлять только в ответ на запрос
//    (т.е. из коллбэков tcp_opened и tcp_data )
// close - инициирует закрытие соединения. Когда соединение будет закрыто,
//    будет вызван коллбэк tcp_closed
void tcp_send(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t close);
//Вызывается при получении запроса на соединение
// Приложение возвращает ненулевое значение, если подтверждает соединение
uint8_t tcp_listen(uint8_t id, eth_frame_t *frame);

// Вызывается при успешном открытии нового соединения
void tcp_opened(uint8_t id, eth_frame_t *frame);

// Вызывается при закрытии соединения
//    reset - признак сброса соединения ("некорректного" закрытия)
void tcp_closed(uint8_t id, uint8_t reset);

// Вызывается, когда приложение должно забрать и/или отправить пакет
//    len - сколько получено данных (может быть 0)
//    closing - признак того, что соединение закрывается
//        (и удалённый узел отдаёт последние данные из буффера)
void tcp_data(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t closing);

