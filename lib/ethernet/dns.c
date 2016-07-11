#include <binds.h>

dns_state_t dns_state;

void dns_query(char *domain)
{
	eth_frame_t *frame = (eth_frame_t *) net_buf;
	ip_packet_t *ip = (ip_packet_t *) frame->data;
	udp_packet_t *udp = (udp_packet_t *) ip->data;
	dns_packet_t *dns = (dns_packet_t *) udp->data;

	// Otherwise received packets would override our generated one.
	__disable_enc28j60_irq();

	ip->to_addr = dns_ip_addr;

	udp->from_port = DNS_CLIENT_PORT;
	udp->to_port = DNS_SERVER_PORT;

	dns->id = dns_state.id = htons(ticks);
	dns->flags = DNS_FLAG_STD_QUERY;
	dns->questions = htons(1);
	dns->answer_rrs = 0;
	dns->authority_rrs = 0;
	dns->additional_rrs = htons(1);

	uint8_t *question_ptr = dns->data;
	dns_add_question(question_ptr, domain, strlen(domain), DNS_QUERY_TYPE_A, DNS_QUERY_CLASS_IN);
	dns_add_rrecord(question_ptr, 0, 0);

	udp_send(frame, question_ptr - udp->data);

	__enable_enc28j60_irq();
}

void dns_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (ip_packet_t *) frame->data;
	udp_packet_t *udp = (udp_packet_t *) ip->data;
	dns_packet_t *dns = (dns_packet_t *) udp->data;

	// Если это DNS ответ на наш прошлый запрос, пакет имеет флаг ответа.
	// А ошибки отсутствуют.
	if ((dns_state.id == dns->id) && htons(dns->flags) & 0x8000)
	{
		// @todo записать полученный адрес в dst_ip_addr.
	}

}
