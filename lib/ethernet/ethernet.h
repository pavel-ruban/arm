#pragma once
#pragma pack(1)

#include <string.h>

/*
 * Config
 */
#define WITH_ICMP

// Defaults
#define MAC_ADDR			{0x00,0x4e,0x42,0x4c,0x55,0x46}
#define DEST_IP_ADDR			inet_addr(192,168,0,43)
#define IP_ADDR				inet_addr(192,168,0,74)
#define IP_GATEWAY			inet_addr(192,168,0,1)
#define DNS_SERVER_IP			inet_addr(192,168,0,43)
#define IP_MASK 			inet_addr(255,255,255,0);
#define APP_PORT			37208

#define IP_PACKET_TTL		64

extern uint32_t dest_ip_addr;

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

#define ip_broadcast (ip_addr | ~ip_mask)

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
uint8_t udp_send(eth_frame_t *frame, uint16_t len);
uint8_t *arp_search_cache(uint32_t node_ip_addr);

void eth_send_packet(uint8_t *data, uint16_t len);
uint16_t eth_recv_packet(uint8_t *buf, uint16_t buflen);
void eth_filter(eth_frame_t *frame, uint16_t len);

#ifdef __cplusplus
extern "C" {
#endif
	void eth_reply(eth_frame_t *frame, uint16_t len);
	void eth_send(eth_frame_t *frame, uint16_t len);
	void arp_filter(eth_frame_t *frame, uint16_t len);
	uint8_t *arp_resolve(uint32_t node_ip_addr);
#ifdef __cplusplus
}
#endif

void ip_filter(eth_frame_t *frame, uint16_t len);

void ip_resend(eth_frame_t *frame, uint16_t len);
void icmp_filter(eth_frame_t *frame, uint16_t len);
void udp_filter(eth_frame_t *frame, uint16_t len);
void tcp_filter(eth_frame_t *frame, uint16_t len);

void ip_reply(eth_frame_t *frame, uint16_t len);
uint16_t ip_cksum(uint32_t sum, uint8_t *buf, uint16_t len);

// Заголовок TCP-пакета
#define TCP_FLAG_URG        0x20
#define TCP_FLAG_ACK        0x10
#define TCP_FLAG_PSH        0x08
#define TCP_FLAG_RST        0x04
#define TCP_FLAG_SYN        0x02
#define TCP_FLAG_FIN        0x01

#define TCP_REXMIT_TIMEOUT 100
#define TCP_REXMIT_LIMIT 30

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

    // Тут всё по-прежнему
    tcp_status_code_t status;
    uint32_t event_time;
    uint32_t seq_num;
    uint32_t ack_num;
    uint32_t remote_addr;
    uint16_t remote_port;
    uint16_t local_port;

    // Соединение закрывается?
    uint8_t is_closing;

    // Количество выполненных ретрансмиссий
    uint8_t rexmit_count;

    // Сохранённый указатель потока
    uint32_t seq_num_saved;
} tcp_state_t;

// Режим отправки пакета
//    используется в основном для оптимизации отправки
typedef enum tcp_sending_mode {
    // Отправляем новый пакет.
    // При этом резолвим MAC-адрес получателя,
    //    выставляем все поля пакета,
    //    залиыаем данные,
    //    считаем чексумму.
    // Самый медленный способ отправки.
    TCP_SENDING_SEND,

    // Только что получили пакет от удалённого узла и
    //    отправляем ему пакет в ответ.
    // При этом обмениваем в нём адреса получателя/отправителя,
    //    обновляем указатели seq/ack,
    //    заливаем новые данные,
    //    пересчитываем чексумму.
    //    Затем тупо пинаем пакет туда, откуда он пришёл
    TCP_SENDING_REPLY,

    // Только что отправляли пакет узлу и
    //    отправляем следом ещё один.
    // При этом заливаем новые данные,
    //    обновляем указатели seq/ack,
    //    пересчитываем чексумму,
    //    все адреса оставляем те же.
    // Самый быстрый способ отправки.
    // После отправки любого пакета, этот режим
    //    активируется автоматически.
    TCP_SENDING_RESEND
} tcp_sending_mode_t;

extern tcp_sending_mode_t tcp_send_mode;
extern uint8_t tcp_ack_sent;

// Максимальное количество одновременно открытых соединений
#define TCP_MAX_CONNECTIONS        2

// Размер окна, который будем указывать в отправляемых пакетах
#define TCP_WINDOW_SIZE            65535

// Таймаут, после которого неактивные соединения будут прибиваться
#define TCP_TIMEOUT                10 //s

// Максимальный размер пакета, передаваемый при открытии соединения
#define TCP_SYN_MSS                448

// Отправить блок с установленным флагом PSH
#define TCP_OPTION_PUSH            0x01
// Отправить блок и закрыть соединение
#define TCP_OPTION_CLOSE        0x02

// Пул TCP-соединений
extern tcp_state_t tcp_pool[TCP_MAX_CONNECTIONS];

// Режим отправки пакетов
// Устанавливается в 1 после отправки пакета
//  (следующие пакеты по умолчанию
//    будут отправляться на тот же адрес)
extern uint8_t tcp_use_resend;

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
void tcp_send(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t options);
uint8_t tcp_xmit(tcp_state_t *st, eth_frame_t *frame, uint16_t len);
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

void tcp_read(uint8_t id, eth_frame_t *frame, uint8_t re);

void tcp_poll();

// Порт DHCP-сервера
#define DHCP_SERVER_PORT        htons(0x0043)
// Порт DHCP-клиента
#define DHCP_CLIENT_PORT        htons(0x0044)

// Тип операции (запрос/ответ)
#define DHCP_OP_REQUEST            1
#define DHCP_OP_REPLY            2

// Тип аппаратного адреса (Ethernet)
#define DHCP_HW_ADDR_TYPE_ETH    1

// Флаг - широковещательный пакет
//  (если флаг установлен, ответ
//  тоже будет широковещательным)
#define DHCP_FLAG_BROADCAST        htons(0x8000)

// Волшебная кука
#define DHCP_MAGIC_COOKIE        htonl(0x63825363)

// Формат DHCP-сообщения
typedef struct dhcp_message {
    uint8_t operation; // Операция (запрос или ответ)
    uint8_t hw_addr_type; // Тип адреса = 1 (Ethernet)
    uint8_t hw_addr_len; // Длина адреса = 6 (MAC-адрес)
    uint8_t unused1; // Не юзаем
    uint32_t transaction_id; // Идентификатор транзакции
    uint16_t second_count; // Время с начала операции
    uint16_t flags; // Флаги
    uint32_t client_addr; // Адрес клиента (при обновлении)
    uint32_t offered_addr; // Предложенный адрес
    uint32_t server_addr; // Адрес сервера
    uint32_t unused2; // Не юзаем
    uint8_t hw_addr[16]; // Наш MAC-адрес
    uint8_t unused3[192]; // Не юзаем
    uint32_t magic_cookie; // Волшебная кука
    uint8_t options[]; // Опции
} dhcp_message_t;

// Коды опций
#define DHCP_CODE_PAD            0        // Выравнивание
#define DHCP_CODE_END            255        // Конец
#define DHCP_CODE_SUBNETMASK    1        // Маска подсети
#define DHCP_CODE_GATEWAY        3        // Гейт
#define DHCP_CODE_REQUESTEDADDR    50        // Выбранный IP-адрес
#define DHCP_CODE_LEASETIME        51        // Время аренды адреса
#define DHCP_CODE_MESSAGETYPE    53        // Тип сообщения
#define DHCP_CODE_DHCPSERVER    54        // DHCP-сервер
#define DHCP_CODE_RENEWTIME        58        // Время до обновления адреса
#define DHCP_CODE_REBINDTIME    59        // Время до выбора другого сервера

// DHCP-опция
typedef struct dhcp_option {
    uint8_t code;
    uint8_t len;
    uint8_t data[];
} dhcp_option_t;

// Тип сообщения
#define DHCP_MESSAGE_DISCOVER    1    // Поиск
#define DHCP_MESSAGE_OFFER        2    // Предложение адреса
#define DHCP_MESSAGE_REQUEST    3    // Запрос адреса
#define DHCP_MESSAGE_ACK        5    // Подтверждение

// Состояние DHCP
typedef enum dhcp_status_code {
    DHCP_INIT, // Адрес не выделен
    DHCP_ASSIGNED, // Адрес выделен
    DHCP_WAITING_OFFER, // Ждём ответа сервера
    DHCP_WAITING_ACK // Ждём подтверждение от сервера
} dhcp_status_code_t;

// Состояние DHCP
extern dhcp_status_code_t dhcp_status; // стейт
extern uint32_t dhcp_retry_time; // время повторной попытки получения адреса

// Эта штука добавляет опцию в пакет и
//  сдвигает указатель к концу опции
//   ptr - указатель на начало опции
#define dhcp_add_option(ptr, optcode, type, value) \
    ((dhcp_option_t*)ptr)->code = optcode; \
    ((dhcp_option_t*)ptr)->len = sizeof(type); \
    *((type *) (((dhcp_option_t *) ptr)->data)) = value; \
    ptr += sizeof(dhcp_option_t) + sizeof(type); \
    if (sizeof(type) & 1) *(ptr++) = 0; // добавляем пад

void dhcp_filter(eth_frame_t *frame, uint16_t len);
void dhcp_poll();

#define DNS_CLIENT_PORT		htons(58491)
#define DNS_SERVER_PORT		htons(53)

extern uint32_t dns_ip_addr;

void dns_filter(eth_frame_t *frame, uint16_t len);
void dns_query(char *domain);

typedef struct dns_packet {
	uint16_t id;
	uint16_t flags;
	uint16_t questions;
	uint16_t answer_rrs;
	uint16_t authority_rrs;
	uint16_t additional_rrs;
	uint8_t data[];
} dns_packet_t;

typedef struct dns_query {
	uint16_t type;
	uint16_t qclass;
	uint8_t *domain;
} dns_query_t;

typedef struct dns_rrecord {
	uint16_t name;
	uint16_t type;
	uint16_t rclass;
	uint16_t ttl;
	uint16_t rdlength;
	uint8_t data[];
} dns_rrecord_t;

typedef struct dns_state {
	uint16_t id;
} dns_state_t;

extern dns_state_t dns_state;

// Эта штука добавляет опцию в пакет и
//  сдвигает указатель к концу опции
//   ptr - указатель на начало опции
#define dns_add_question(ptr, domain, len, type, qclass)	\
	*ptr++ = len;						\
	memcpy((void *) ptr, domain, len);			\
	ptr += len;						\
	*ptr++ = 0x0;						\
	*((uint16_t *) ptr) = type;				\
	ptr += sizeof(uint16_t);				\
	*((uint16_t *) ptr) = qclass;				\
	ptr += sizeof(uint16_t);

// Эта штука добавляет resource record в пакет и
//  сдвигает указатель к концу resource record
//   ptr - указатель на начало опции
#define dns_add_rrecord(ptr, rr, len)				\
	*ptr++ = 0;						\
	*((uint16_t *) ptr) = htons(0x0029);			\
	ptr += sizeof(uint16_t);				\
	*((uint16_t *) ptr) = htons(512);			\
	ptr += sizeof(uint16_t);				\
	*ptr++ = 0;						\
	*ptr++ = 0;						\
	*((uint16_t *) ptr) = htons(0x8000);			\
	ptr += sizeof(uint16_t);				\
	*((uint16_t *) ptr) = htons(0);				\
	ptr += sizeof(uint16_t);

#define DNS_QUERY_TYPE_A	htons(0x0001)
#define DNS_QUERY_CLASS_IN	htons(0x0001)
#define DNS_FLAG_STD_QUERY	htons(0x0100)

typedef struct httpd_status {
	uint16_t statuscode;
	uint16_t statuscode_saved;
	uint16_t numbytes;
	uint16_t numbytes_saved;
	uint8_t *data;
	uint8_t *data_saved;
} httpd_status_t;

extern httpd_status_t httpd_status;
