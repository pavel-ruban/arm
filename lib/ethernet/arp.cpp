#include <binds.h>
#include <queue.h>

// Размер ARP-кэша
#define ARP_CACHE_SIZE      10

// ARP-кэш
typedef struct arp_cache_entry {
	uint32_t ip_addr;
	uint8_t mac_addr[6];
} arp_cache_entry_t;

Queue<arp_cache_entry_t, ARP_CACHE_SIZE> arp_cache;

/*
 * ARP
 */
extern "C" void arp_filter(eth_frame_t *frame, uint16_t len)
{
	arp_message_t *msg = (arp_message_t *)(frame->data);

	if(len >= sizeof(arp_message_t))
	{
		if( (msg->hw_type == ARP_HW_TYPE_ETH) &&
			(msg->proto_type == ARP_PROTO_TYPE_IP) &&
			(msg->ip_addr_to == ip_addr) )
		{
			switch (msg->type) {
				case ARP_TYPE_REQUEST:
					msg->type = ARP_TYPE_RESPONSE;
					memcpy(msg->mac_addr_to, msg->mac_addr_from, 6);
					memcpy(msg->mac_addr_from, mac_addr, 6);
					msg->ip_addr_to = msg->ip_addr_from;
					msg->ip_addr_from = ip_addr;

					eth_reply(frame, sizeof(arp_message_t));

					break;

				// ARP-ответ, добавляем узел в кэш
				case ARP_TYPE_RESPONSE:
					if (!arp_search_cache(msg->ip_addr_from))
					{
						arp_cache_entry_t cached;
						cached.ip_addr = msg->ip_addr_from;
						memcpy(cached.mac_addr, msg->mac_addr_from, 6);
						arp_cache.push_back(cached);
					}
					break;
			}
		}
	}
}

// Поиск в ARP-кэше
uint8_t *arp_search_cache(uint32_t node_ip_addr)
{
	for (auto it = arp_cache.begin(); it != arp_cache.end(); ++it)
	{
		if (it->ip_addr == node_ip_addr)
			return it->mac_addr;
	}
	return 0;
}

// ARP-ресолвер
// Если MAC-адрес узла известен, возвращает его
// Неизвестен - посылает запрос и возвращает 0
extern "C" uint8_t *arp_resolve(uint32_t node_ip_addr)
{
	eth_frame_t *frame = (eth_frame_t *) net_buf;
	arp_message_t *msg = (arp_message_t *) (frame->data);
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

