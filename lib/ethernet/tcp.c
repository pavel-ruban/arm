#include "lan.h"
//extern char *term_var_info;

uint8_t dest_mac_addr[6] = DEST_MAC_ADDR;
uint8_t mac_addr[6] = MAC_ADDR;
uint32_t ip_addr;
uint32_t ip_mask;
uint32_t ip_gateway;

uint8_t net_buf[ENC28J60_MAXFRAME];

/*
 * UDP
 */
void udp_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void*)(frame->data);
	udp_packet_t *udp = (void*)(ip->data);

	if(len >= sizeof(udp_packet_t))
	{
		udp_packet(frame, ntohs(udp->len) -
			sizeof(udp_packet_t));
	}
}

void udp_reply(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void*)(frame->data);
	udp_packet_t *udp = (void*)(ip->data);
	uint16_t temp;

	len += sizeof(udp_packet_t);

	temp = udp->from_port;
	udp->from_port = udp->to_port;
	udp->to_port = temp;

	udp->len = htons(len);

	udp->cksum = 0;
	udp->cksum = ip_cksum(len + IP_PROTOCOL_UDP,
		(uint8_t*)udp-8, len+8);

	ip_reply(frame, len);
}

/*
 * ICMP
 */

#ifdef WITH_ICMP

void icmp_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *packet = (void*)frame->data;
	icmp_echo_packet_t *icmp = (void*)packet->data;

	if(len >= sizeof(icmp_echo_packet_t) )
	{
		if(icmp->type == ICMP_TYPE_ECHO_RQ)
		{
			icmp->type = ICMP_TYPE_ECHO_RPLY;
			icmp->cksum += 8; // update cksum
			ip_reply(frame, len);
		}
	}
}

#endif


/*
 * IP
 */

uint16_t ip_cksum(uint32_t sum, uint8_t *buf, size_t len)
{
	while(len >= 2)
	{
		sum += ((uint16_t)*buf << 8) | *(buf+1);
		buf += 2;
		len -= 2;
	}

	if(len)
		sum += (uint16_t)*buf << 8;

	while(sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~htons((uint16_t)sum);
}

void ip_reply(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *packet = (void*)(frame->data);

	packet->total_len = htons(len + sizeof(ip_packet_t));
	packet->fragment_id = 0;
	packet->flags_framgent_offset = 0;
	packet->ttl = IP_PACKET_TTL;
	packet->cksum = 0;
	packet->to_addr = packet->from_addr;
	packet->from_addr = ip_addr;
	packet->cksum = ip_cksum(0, (void*)packet, sizeof(ip_packet_t));

	eth_reply((void*)frame, len + sizeof(ip_packet_t));
}

void ip_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *packet = (void*)(frame->data);

	//if(len >= sizeof(ip_packet_t))
	//{
		if( (packet->ver_head_len == 0x45) &&
			(packet->to_addr == ip_addr) )
		{
			len = ntohs(packet->total_len) -
				sizeof(ip_packet_t);

			switch(packet->protocol)
			{
#ifdef WITH_ICMP
			case IP_PROTOCOL_ICMP:
				icmp_filter(frame, len);
				break;
#endif
			case IP_PROTOCOL_UDP:
				udp_filter(frame, len);
				break;
			}
		}
	//}
}


/*
 * ARP
 */

void arp_filter(eth_frame_t *frame, uint16_t len)
{
	arp_message_t *msg = (void*)(frame->data);

	if(len >= sizeof(arp_message_t))
	{
		if( (msg->hw_type == ARP_HW_TYPE_ETH) &&
			(msg->proto_type == ARP_PROTO_TYPE_IP) )
		{
			if( (msg->type == ARP_TYPE_REQUEST) &&
				(msg->ip_addr_to == ip_addr) )
			{
				msg->type = ARP_TYPE_RESPONSE;
				memcpy(msg->mac_addr_to, msg->mac_addr_from, 6);
				memcpy(msg->mac_addr_from, mac_addr, 6);
				msg->ip_addr_to = msg->ip_addr_from;
				msg->ip_addr_from = ip_addr;

				eth_reply(frame, sizeof(arp_message_t));
			}
		}
	}
}


/*
 * Ethernet
 */

void eth_reply(eth_frame_t *frame, uint16_t len)
{
	memcpy(frame->to_addr, frame->from_addr, 6);
	memcpy(frame->from_addr, mac_addr, 6);
	enc28j60_send_packet((void*)frame, len +
		sizeof(eth_frame_t));
}
extern char *term_var_info;

void eth_filter(eth_frame_t *frame, uint16_t len)
{
//	*term_var_info = *((uint8_t *) (&len));
//	if (!*term_var_info) *term_var_info = 0xff;
//
//	*(term_var_info + 1) = *((uint8_t *) (&len + 1));
//	if (!*(term_var_info + 1)) *(term_var_info + 1)= 0xff;
//	*(term_var_info + 2) = 0;
	//if(len >= sizeof(eth_frame_t))
	//{
		switch(frame->type)
		{
		case ETH_TYPE_ARP:
			arp_filter(frame, len - sizeof(eth_frame_t));
			break;
		case ETH_TYPE_IP:
			ip_filter(frame, len - sizeof(eth_frame_t));
			break;
		}
//	}
}


/*
 * LAN
 */

void lan_init()
{
	enc28j60_init(mac_addr);
}

#include "spi.h"

void lan_poll()
{
	uint16_t len;
	eth_frame_t *frame = (void*)net_buf;

	while((len = enc28j60_recv_packet(net_buf, sizeof(net_buf))));
		eth_filter(frame, len);
}

// Отправка Ethernet-фрейма
// Должны быть установлены следующие поля:
//    - frame.to_addr - MAC-адрес получателя
//    - frame.type - протокол
// len - длина поля данных фрейма
//void eth_send(eth_frame_t *frame, uint16_t len)
//{
//    memcpy(frame->from_addr, mac_addr, 6);
//    enc28j60_send_packet((void*)frame, len +
//        sizeof(eth_frame_t));
//}

// Отправляет UDP-пакет
// Должны быть установлены следующие поля:
//    ip.to_addr - адрес получателя
//    udp.from_port - порт отрпавителя
//    udp.to_port - порт получателя
// len - длина поля данных пакета
// Если MAC-адрес узла/гейта ещё не определён, функция возвращает 0
/*
uint8_t udp_send(eth_frame_t *frame, uint16_t len)
{
    ip_packet_t *ip = (void*)(frame->data);
    udp_packet_t *udp = (void*)(ip->data);

    len += sizeof(udp_packet_t);

    ip->protocol = IP_PROTOCOL_UDP;
    ip->from_addr = ip_addr;

    udp->len = htons(len);
    udp->cksum = 0;
    udp->cksum = ip_cksum(len + IP_PROTOCOL_UDP,
        (uint8_t*)udp-8, len+8);

    return ip_send(frame, len);
}

// Отправка IP-пакета
// Следующие поля пакета должны быть установлены:
//    ip.to_addr - адрес получателя
//    ip.protocol - код протокола
// len - длина поля данных пакета
// Если MAC-адрес узла/гейта ещё не определён, функция возвращает 0
uint8_t ip_send(eth_frame_t *frame, uint16_t len)
{
    ip_packet_t *ip = (void*)(frame->data);
    uint32_t route_ip;
    uint8_t *mac_addr_to;

    // Если узел в локалке, отправляем пакет ему
    //    если нет, то гейту
    if( ((ip->to_addr ^ ip_addr) & ip_mask) == 0 )
        route_ip = ip->to_addr;
    else
        route_ip = ip_gateway;

    // Ресолвим MAC-адрес
    if(!(mac_addr_to = arp_resolve(route_ip)))
        return 0;

    // Отправляем пакет
    len += sizeof(ip_packet_t);

    memcpy(frame->to_addr, mac_addr_to, 6);
    frame->type = ETH_TYPE_IP;

    ip->ver_head_len = 0x45;
    ip->tos = 0;
    ip->total_len = htons(len);
    ip->fragment_id = 0;
    ip->flags_framgent_offset = 0;
    ip->ttl = IP_PACKET_TTL;
    ip->cksum = 0;
    ip->from_addr = ip_addr;
    ip->cksum = ip_cksum(0, (void*)ip, sizeof(ip_packet_t));

    eth_send(frame, len);
    return 1;
}
// Размер ARP-кэша
#define ARP_CACHE_SIZE      3

// ARP-кэш
typedef struct arp_cache_entry {
   uint32_t ip_addr;
   uint8_t mac_addr[6];
} arp_cache_entry_t;

uint8_t arp_cache_wr;
arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];

// Поиск в ARP-кэше
uint8_t *arp_search_cache(uint32_t node_ip_addr)
{
    uint8_t i;
    for(i = 0; i < ARP_CACHE_SIZE; ++i)
    {
        if(arp_cache[i].ip_addr == node_ip_addr)
            return arp_cache[i].mac_addr;
    }
    return 0;
}

// ARP-ресолвер
// Если MAC-адрес узла известен, возвращает его
// Неизвестен - посылает запрос и возвращает 0
uint8_t *arp_resolve(uint32_t node_ip_addr)
{
    eth_frame_t *frame = (void*)net_buf;
    arp_message_t *msg = (void*)(frame->data);
    uint8_t *mac;

    // Ищем узел в кэше
    if((mac = arp_search_cache(node_ip_addr)))
        return mac;

    // Отправляем запрос
    memset(frame->to_addr, 0xff, 6);
    frame->type = ETH_TYPE_ARP;

    msg->hw_type = ARP_HW_TYPE_ETH;
    msg->proto_type = ARP_PROTO_TYPE_IP;
    msg->hw_addr_len = 6;
    msg->proto_addr_len = 4;
    msg->type = ARP_TYPE_REQUEST;
    memcpy(msg->mac_addr_from, mac_addr, 6);
    msg->ip_addr_from = ip_addr;
    memset(msg->mac_addr_to, 0x00, 6);
    msg->ip_addr_to = node_ip_addr;

    eth_send(frame, sizeof(arp_message_t));
    return 0;
}
*/
