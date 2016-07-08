#include <binds.h>

uint8_t dest_mac_addr[6] = DEST_MAC_ADDR;
uint8_t mac_addr[6] = MAC_ADDR;
uint32_t ip_addr = IP_ADDR;
uint32_t dest_ip_addr = DEST_IP_ADDR;
uint32_t ip_mask = IP_MASK;
uint32_t ip_gateway = IP_GATEWAY;

uint8_t net_buf[ETHERNET_MAXFRAME];

/*
 * Ethernet
 */

void eth_reply(eth_frame_t *frame, uint16_t len)
{
	memcpy(frame->to_addr, frame->from_addr, 6);
	memcpy(frame->from_addr, mac_addr, 6);
	eth_send_packet((void *) frame, len + sizeof(eth_frame_t));

}

void eth_filter(eth_frame_t *frame, uint16_t len)
{
	switch(frame->type)
	{
		case ETH_TYPE_ARP:
			arp_filter(frame, len - sizeof(eth_frame_t));
			break;
		case ETH_TYPE_IP:
			ip_filter(frame, len - sizeof(eth_frame_t));
			break;
	}
}

// Отправка Ethernet-фрейма
// Должны быть установлены следующие поля:
//    - frame.to_addr - MAC-адрес получателя
//    - frame.type - протокол
// len - длина поля данных фрейма
void eth_send(eth_frame_t *frame, uint16_t len)
{
    memcpy(frame->from_addr, mac_addr, 6);
    eth_send_packet((void*)frame, len +
        sizeof(eth_frame_t));
}
