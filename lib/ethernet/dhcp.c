#include <binds.h>

dhcp_status_code_t dhcp_status; // стейт

uint32_t dhcp_retry_time; // время повторной попытки получения адреса

static uint32_t dhcp_server; // адрес выбранного сервера
static uint32_t dhcp_renew_time; // время обновления
static uint32_t dhcp_transaction_id; // текущая транзакция

// Обработчик DHCP-пакета
void dhcp_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void*)(frame->data);
	udp_packet_t *udp = (void*)(ip->data);
	dhcp_message_t *dhcp = (void*)(udp->data);
	dhcp_option_t *option;
	uint8_t *op, optlen;
	uint32_t offered_net_mask = 0, offered_gateway = 0;
	uint32_t lease_time = 0, renew_time = 0, renew_server = 0;
	uint8_t type = 0;
	uint32_t temp;

	// Проверяем, что пакет отправлен нам
	if( (len >= sizeof(dhcp_message_t)) &&
			(dhcp->operation == DHCP_OP_REPLY) &&
			(dhcp->transaction_id == dhcp_transaction_id) &&
			(dhcp->magic_cookie == DHCP_MAGIC_COOKIE))
	{
		len -= sizeof(dhcp_message_t);

		// Парсим опции в пакете
		op = dhcp->options;
		while(len >= sizeof(dhcp_option_t))
		{
			option = (void*)op;

			// Пад - пропускаем
			if(option->code == DHCP_CODE_PAD)
			{
				op++;
				len--;
			}

			// Конец - заканчиваем парсить
			else if(option->code == DHCP_CODE_END)
			{
				break;
			}

			// Обычная опция
			else
			{
				switch(option->code)
				{
					case DHCP_CODE_MESSAGETYPE:
						type = *(option->data);
						break;
					case DHCP_CODE_SUBNETMASK:
						offered_net_mask = *(uint32_t*)(option->data);
						break;
					case DHCP_CODE_GATEWAY:
						offered_gateway = *(uint32_t*)(option->data);
						break;
					case DHCP_CODE_DHCPSERVER:
						renew_server = *(uint32_t*)(option->data);
						break;
					case DHCP_CODE_LEASETIME:
						temp = *(uint32_t*)(option->data);
						lease_time = ntohl(temp);
						break;
					case DHCP_CODE_RENEWTIME:
						temp = *(uint32_t*)(option->data);
						renew_time = ntohl(temp);
						break;
				}

				optlen = sizeof(dhcp_option_t) + option->len;
				op += optlen;
				len -= optlen;
			}
		}

		switch(type)
		{
			// Получили OFFER (предложение IP-адреса)?
			case DHCP_MESSAGE_OFFER:
				if(dhcp_status == DHCP_WAITING_OFFER)
				{
					// Отправляем запрос на выделение адреса
					ip->to_addr = inet_addr(255,255,255,255);

					udp->to_port = DHCP_SERVER_PORT;
					udp->from_port = DHCP_CLIENT_PORT;

					op = dhcp->options;
					dhcp_add_option(op, DHCP_CODE_MESSAGETYPE,
							uint8_t, DHCP_MESSAGE_REQUEST);
					dhcp_add_option(op, DHCP_CODE_REQUESTEDADDR,
							uint32_t, dhcp->offered_addr);
					dhcp_add_option(op, DHCP_CODE_DHCPSERVER,
							uint32_t, renew_server);
					*(op++) = DHCP_CODE_END;

					dhcp->operation = DHCP_OP_REQUEST;
					dhcp->offered_addr = 0;
					dhcp->server_addr = 0;
					dhcp->flags = DHCP_FLAG_BROADCAST;

					udp_send(frame, (uint8_t*) op - (uint8_t*) dhcp);

					// Ждём подтверждения
					dhcp_status = DHCP_WAITING_ACK;
				}
				break;

				// Получили подтверждение?
			case DHCP_MESSAGE_ACK:
				if (dhcp_status == DHCP_WAITING_ACK)
				{
					// Устанавливаем время и сервер обновления
					if (!renew_time)
						renew_time = lease_time / 2;

					dhcp_server = renew_server;
					dhcp_renew_time = RTC_GetCounter() + renew_time;
					dhcp_retry_time = RTC_GetCounter() + lease_time;

					// Конфигурируем наш хост
					ip_addr = dhcp->offered_addr;
					ip_mask = offered_net_mask;
					ip_gateway = offered_gateway;

					// Всё готово
					dhcp_status = DHCP_ASSIGNED;
				}
				break;
		}
	}
}

// Вызывается периодически
void dhcp_poll()
{
	eth_frame_t *frame = (void*)net_buf;
	ip_packet_t *ip = (void*)(frame->data);
	udp_packet_t *udp = (void*)(ip->data);
	dhcp_message_t *dhcp = (void*)(udp->data);
	uint8_t *op;

	// Истечение аренды адреса или
	//   время для новой попытки получения адреса
	if(RTC_GetCounter() >= dhcp_retry_time)
	{
		// Придумываем идентификатор транзакции
		dhcp_transaction_id = ticks;

		__disable_enc28j60_irq();

		// Опускаем интерфейс (если истекла аренда)
		ip_addr = 0;
		ip_mask = 0;
		ip_gateway = 0;

		// Отправляем DISCOVER (поиск сервера)
		ip->to_addr = inet_addr(255,255,255,255);

		udp->to_port = DHCP_SERVER_PORT;
		udp->from_port = DHCP_CLIENT_PORT;

		memset(dhcp, 0, sizeof(dhcp_message_t));
		dhcp->operation = DHCP_OP_REQUEST;
		dhcp->hw_addr_type = DHCP_HW_ADDR_TYPE_ETH;
		dhcp->hw_addr_len = 6;
		dhcp->transaction_id = dhcp_transaction_id;
		dhcp->flags = DHCP_FLAG_BROADCAST;
		memcpy(dhcp->hw_addr, mac_addr, 6);
		dhcp->magic_cookie = DHCP_MAGIC_COOKIE;

		op = dhcp->options;
		dhcp_add_option(op, DHCP_CODE_MESSAGETYPE,
				uint8_t, DHCP_MESSAGE_DISCOVER);
		*(op++) = DHCP_CODE_END;

		udp_send(frame, (uint8_t *) op - (uint8_t *) dhcp);

		__enable_enc28j60_irq();

		// Ждём ответа (если не будет, снова
		//  попробуем через 15 секунд)
		dhcp_status = DHCP_WAITING_OFFER;
		dhcp_retry_time = RTC_GetCounter() + 2;
	}

	// Обновление адреса
	if((RTC_GetCounter() >= dhcp_renew_time) &&
			(dhcp_status == DHCP_ASSIGNED))
	{
		// Придумываем идентификатор транзакции
		dhcp_transaction_id = ticks;

		__disable_enc28j60_irq();

		// Отправляем запрос
		ip->to_addr = dhcp_server;

		udp->to_port = DHCP_SERVER_PORT;
		udp->from_port = DHCP_CLIENT_PORT;

		memset(dhcp, 0, sizeof(dhcp_message_t));
		dhcp->operation = DHCP_OP_REQUEST;
		dhcp->hw_addr_type = DHCP_HW_ADDR_TYPE_ETH;
		dhcp->hw_addr_len = 6;
		dhcp->transaction_id = dhcp_transaction_id;
		dhcp->client_addr = ip_addr;
		memcpy(dhcp->hw_addr, mac_addr, 6);
		dhcp->magic_cookie = DHCP_MAGIC_COOKIE;

		op = dhcp->options;
		dhcp_add_option(op, DHCP_CODE_MESSAGETYPE,
				uint8_t, DHCP_MESSAGE_REQUEST);
		dhcp_add_option(op, DHCP_CODE_REQUESTEDADDR,
				uint32_t, ip_addr);
		dhcp_add_option(op, DHCP_CODE_DHCPSERVER,
				uint32_t, dhcp_server);
		*(op++) = DHCP_CODE_END;

		if(!udp_send(frame, (uint8_t*) op - (uint8_t*) dhcp))
		{
			
			__enable_enc28j60_irq();

			// Пакет не отправлен (видимо, адреса
			//  сервера ещё нет в арп-кэше)
			// попробуем снова через 5 секунд
			dhcp_renew_time = RTC_GetCounter() + 1;
			return;
		}

		__enable_enc28j60_irq();

		// Ждём ответа
		dhcp_status = DHCP_WAITING_ACK;
	}
}
