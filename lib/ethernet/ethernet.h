#pragma once

#include <string.h>
#include <binds.h>

/*
 * Config
 */
#define WITH_ICMP

#define MAC_ADDR			{0x00,0x4e,0x42,0x4c,0x55,0x45}
// In case of external requests, the dest mac address should be the GATEWAY MAC ADDRESS.
#define DEST_MAC_ADDR			{0x5c,0xd9,0x98,0xae,0x30,0x99}
#define dest_ip_addr			inet_addr(195,211,7,205)
#define IP_ADDR				inet_addr(192,168,1,75)
#define IP_GATEWAY			inet_addr(192,168,1,1)
#define IP_MASK 			inet_addr(255,255,255,0);
#define APP_PORT			36208

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

void udp_packet(eth_frame_t *frame, uint16_t len);
void udp_reply(eth_frame_t *frame, uint16_t len);
uint8_t ip_send(eth_frame_t *frame, uint16_t len);
void eth_send(eth_frame_t *frame, uint16_t len);
uint8_t udp_send(eth_frame_t *frame, uint16_t len);
uint8_t *arp_resolve(uint32_t node_ip_addr);
uint8_t *arp_search_cache(uint32_t node_ip_addr);

void eth_reply(eth_frame_t *frame, uint16_t len);
void eth_send_packet(uint8_t *data, uint16_t len);
uint16_t eth_recv_packet(uint8_t *buf, uint16_t buflen);

void ip_reply(eth_frame_t *frame, uint16_t len);
uint16_t ip_cksum(uint32_t sum, uint8_t *buf, uint16_t len);
