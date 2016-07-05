#include <binds.h>

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
