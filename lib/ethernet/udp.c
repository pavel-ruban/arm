#include <binds.h>

/*
 * UDP
 */
void udp_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void *) (frame->data);
	udp_packet_t *udp = (void *) (ip->data);

	if(len >= sizeof(udp_packet_t))
	{
		switch (udp->to_port)
		{
			case DHCP_CLIENT_PORT:
				dhcp_filter(frame, ntohs(udp->len) - sizeof(udp_packet_t));
				break;


			case DNS_CLIENT_PORT:
				dns_filter(frame, ntohs(udp->len) - sizeof(udp_packet_t));
				break;

			default:
				udp_reply(frame, ntohs(udp->len) - sizeof(udp_packet_t));
				break;
		}
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

// Отправляет UDP-пакет
// Должны быть установлены следующие поля:
//    ip.to_addr - адрес получателя
//    udp.from_port - порт отрпавителя
//    udp.to_port - порт получателя
// len - длина поля данных пакета
// Если MAC-адрес узла/гейта ещё не определён, функция возвращает 0
uint8_t udp_send(eth_frame_t *frame, uint16_t len)
{
    ip_packet_t *ip = (void *) (frame->data);
    udp_packet_t *udp = (void *) (ip->data);

    len += sizeof(udp_packet_t);

    ip->protocol = IP_PROTOCOL_UDP;
    ip->from_addr = ip_addr;

    udp->len = htons(len);
    udp->cksum = 0;
    udp->cksum = ip_cksum(len + IP_PROTOCOL_UDP,
        (uint8_t *) udp - 8, len + 8);

    return ip_send(frame, len);
}
