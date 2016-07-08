#include <binds.h>

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

/*
 * IP
 */
uint16_t ip_cksum(uint32_t sum, uint8_t *buf, uint16_t len)
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

	if (len >= sizeof(ip_packet_t))
	{
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
			case IP_PROTOCOL_TCP:
				//tcp_filter(frame, len);
				break;
			}
		}
	}
}
