#include <binds.h>

// Режим отправки пакетов
// Устанавливается в 1 после отправки пакета
//  (следующие пакеты по умолчанию
//    будут отправляться на тот же адрес)
uint8_t tcp_use_resend;
tcp_sending_mode_t tcp_send_mode;

// Признак отправки подтверждения
//    устанавливается при отправке пакета с установленым флагом ACK.
uint8_t tcp_ack_sent;

// Пул TCP-соединений
tcp_state_t tcp_pool[TCP_MAX_CONNECTIONS];

uint8_t tcp_listen(uint8_t id, eth_frame_t *frame)
{
	ip_packet_t *ip = (void*)(frame->data);
	tcp_packet_t *tcp = (void*)(ip->data);

	// Если получили запрос на подключение на порт приложения,
	//    подтверждаем соединение
	if (tcp->to_port == htons(APP_PORT)) return 1;
	// Помимо основного протокола, также есть легкий http сервер
	// для настроек.
	if (tcp->to_port == htons(80)) return 1;

	return 0;
}

void tcp_opened(uint8_t id, eth_frame_t *frame)
{
	ip_packet_t *ip = (void*)(frame->data);
	tcp_packet_t *tcp = (void*)(ip->data);

	// Соединение открыто - отправляем "Hello!"
	//memcpy(tcp->data, (void*)"Hello!\n", 7);
	//tcp_send(id, frame, 7, 0);
}

void tcp_data(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t closing)
{
	ip_packet_t *ip = (void*)(frame->data);
	tcp_packet_t *tcp = (void*)(ip->data);
	char *in_data = (void*)tcp_get_data(tcp);

	//  {
	// получили команду "quit" -
	//    отправляем "Bye" и закрываем соединение
	//      if(memcmp(in_data, "quit", 4) == 0)
	//      {


	// Отправляем "OK"
	//  memcpy(tcp->data, "OK\n", 3);
	//tcp_send(id, frame, 3, 0);
	//}
}

void tcp_closed(uint8_t id, uint8_t hard)
{
    tcp_state_t *st = tcp_pool + id;
}

// Отправляет ответ(ы) на полученный пакет
void tcp_reply(eth_frame_t *frame, uint16_t len)
{
	uint16_t temp;

	ip_packet_t *ip = (void*)(frame->data);
	tcp_packet_t *tcp = (void*)(ip->data);

	// Если в буфере лежит полученый пакет, меняем местами
	//   порты отправителя/получателя
	// Если в буфере предыдущий отправленый нами пакет,
	//   оставляем порты как есть
	if(!tcp_use_resend)
	{
		temp = tcp->from_port;
		tcp->from_port = tcp->to_port;
		tcp->to_port = temp;
		tcp->window = htons(TCP_WINDOW_SIZE);
		tcp->urgent_ptr = 0;
	}

	// Если отправляем SYN, добавляем опцию MSS
	//    (максимальный размер пакета)
	//    чтобы пакеты не фрагментировались
	if(tcp->flags & TCP_FLAG_SYN)
	{
		tcp->data_offset = (sizeof(tcp_packet_t) + 4) << 2;
		tcp->data[0] = 2;//option: MSS
		tcp->data[1] = 4;//option len
		tcp->data[2] = TCP_SYN_MSS>>8;
		tcp->data[3] = TCP_SYN_MSS&0xff;
		len = 4;
	}

	else
	{
		tcp->data_offset = sizeof(tcp_packet_t) << 2;
	}

	// Рассчитываем контрольную сумму
	len += sizeof(tcp_packet_t);
	tcp->cksum = 0;
	tcp->cksum = ip_cksum(len + IP_PROTOCOL_TCP,
			(uint8_t*)tcp - 8, len + 8);

	// В буфере лежит принятый пакет?
	if(!tcp_use_resend)
	{
		// Меняем адрес отправителя/получателя
		//    и отправляем
		ip_reply(frame, len);
		tcp_use_resend = 1;
	}

	// Или предыдущий отправленый пакет?
	else
	{
		// Отправляем туда же
		ip_resend(frame, len);
	}
}

// Отправка пакета
//    Поле tcp.flags должно быть установлено вручную
uint8_t tcp_xmit(tcp_state_t *st, eth_frame_t *frame, uint16_t len)
{
	uint8_t status = 1;
	uint16_t temp, plen = len;

	ip_packet_t *ip = (void *) (frame->data);
	tcp_packet_t *tcp = (void *) (ip->data);

	// Отправляем новый пакет?
	if(tcp_send_mode == TCP_SENDING_SEND)
	{
		// Устанавливаем поля в заголовке
		ip->to_addr = st->remote_addr;
		ip->from_addr = ip_addr;
		ip->protocol = IP_PROTOCOL_TCP;

		tcp->to_port = st->remote_port;
		tcp->from_port = st->local_port;
	}
	// Отправляем пакет в ответ?
	else if(tcp_send_mode == TCP_SENDING_REPLY)
	{
		// Меняем местами порты отправителя/получателя
		temp = tcp->from_port;
		tcp->from_port = tcp->to_port;
		tcp->to_port = temp;
	}

	if (tcp_send_mode != TCP_SENDING_RESEND)
	{
		// Устанавливаем поля в заголовке
		tcp->window = htons(TCP_WINDOW_SIZE);
		tcp->urgent_ptr = 0;
	}

	// Если отправляем SYN, добавляем опцию MSS
	//        (максимальный размер сегмента)
	if(tcp->flags & TCP_FLAG_SYN)
	{
		tcp->data_offset = (sizeof(tcp_packet_t) + 4) << 2;
		tcp->data[0] = 2; //MSS
		tcp->data[1] = 4; //длина
		tcp->data[2] = TCP_SYN_MSS >> 8;
		tcp->data[3] = TCP_SYN_MSS & 0xff;
		plen = 4;
	}
	else
	{
		tcp->data_offset = sizeof(tcp_packet_t) << 2;
	}

	// Устанавливаем указатели потока
	tcp->seq_num = htonl(st->seq_num);
	tcp->ack_num = htonl(st->ack_num);

	// Вычисляем контрольную сумму
	plen += sizeof(tcp_packet_t);
	tcp->cksum = 0;
	tcp->cksum = ip_cksum(plen + IP_PROTOCOL_TCP,
			(uint8_t*)tcp - 8, plen + 8);

	// Отправляем всю эту фигню в виде IP-пакета
	switch(tcp_send_mode)
	{
		case TCP_SENDING_SEND:
			status = ip_send(frame, plen);
			tcp_send_mode = TCP_SENDING_RESEND;
			break;
		case TCP_SENDING_REPLY:
			ip_reply(frame, plen);
			tcp_send_mode = TCP_SENDING_RESEND;
			break;
		case TCP_SENDING_RESEND:
			ip_resend(frame, plen);
			break;
	}

	// Обновляем наш указатель потока
	st->seq_num += len;
	if( (tcp->flags & TCP_FLAG_SYN) || (tcp->flags & TCP_FLAG_FIN) )
		st->seq_num++;

	// Устанавливаем флаг отправки подтверждения
	if( (tcp->flags & TCP_FLAG_ACK) && (status) )
		tcp_ack_sent = 1;

	return status;
}

// Отправляет данные по TCP
void tcp_send(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t options)
{
	ip_packet_t *ip = (void *) (frame->data);
	tcp_packet_t *tcp = (void *) (ip->data);
	tcp_state_t *st = tcp_pool + id;
	uint8_t flags = TCP_FLAG_ACK;

	// Если соединение не устновлено, выходим - защита от дурака)
	if(st->status != TCP_ESTABLISHED)
		return;

	// Если нужно, добавляем флаг PUSH
	if(options & TCP_OPTION_PUSH)
		flags |= TCP_FLAG_PSH;

	// Если закрываем соединения, добавляем флаг FIN
	//    и начинаем ждать, пока удалённый
	//            узел тоже пришлёт FIN/ACK
	if(options & TCP_OPTION_CLOSE)
	{
		flags |= TCP_FLAG_FIN;
		st->status = TCP_FIN_WAIT;
	}

	// Пинаем пакет куда нужно
	tcp->flags = flags;
	tcp_xmit(st, frame, len);
}

// Подтверждает принятые данные и устанавливает указатель потока
//    (обменивает seq_num и ack_num, затем прибавляет к ack_num
//    количество принятых данных)
void tcp_step(tcp_packet_t *tcp, uint16_t num)
{
	uint32_t ack_num;
	ack_num = ntohl(tcp->seq_num) + num;
	tcp->seq_num = tcp->ack_num;
	tcp->ack_num = htonl(ack_num);
}

// Обработчик TCP-пакетов
void tcp_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void *) (frame->data);
	tcp_packet_t *tcp = (void *) (ip->data);
	tcp_state_t *st = 0, *pst;
	uint8_t id, tcpflags;

	// Пришло не на наш адрес (например, на широковещательный) - выбрасываем
	// Кто будет посылать TCP-пакеты на широковещательный адрес?
	// ХЗ, подстрахуемся на всякий случай))
	if(ip->to_addr != ip_addr)
		return;

	// Отнимаем размер заголовка и получаем количество данных в пакете
	len -= tcp_head_size(tcp);

	// Отрезаем для удобства флаги SYN, FIN, ACK и RST
	//    другие флаги нам не интересны
	tcpflags = tcp->flags & (TCP_FLAG_SYN | TCP_FLAG_ACK | TCP_FLAG_RST | TCP_FLAG_FIN);

	// Переключаем tcp_xmit в режим отправки пакетов "в ответ"
	tcp_send_mode = TCP_SENDING_REPLY;
	tcp_ack_sent = 0;

	// Ищем к какому из активных соединений относится
	//    полученный пакет
	for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
	{
		pst = tcp_pool + id;

		if((pst->status != TCP_CLOSED) &&
				(ip->from_addr == pst->remote_addr) &&
				(tcp->from_port == pst->remote_port) &&
				(tcp->to_port == pst->local_port))
		{
			st = pst;
			break;
		}
	}

	// Не нашли такого соединения - пакет является запросом
	//    на соединение или просто каким-то заблудившимся пакетом
	if (!st)
	{
		// Получили запрос на соединение?
		if(tcpflags == TCP_FLAG_SYN)
		{
			// Ищем свободный слот, куда добавить новое соединение
			for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
			{
				pst = tcp_pool + id;

				if(pst->status == TCP_CLOSED)
				{
					st = pst;
					break;
				}
			}

			// Нашли слот, и приложение подтверждает это соединение
			if(st && tcp_listen(id, frame))
			{
				// Добавляем новое соединение
				st->status = TCP_SYN_RECEIVED;
				st->event_time = ticks;
				st->seq_num = ticks;
				st->ack_num = ntohl(tcp->seq_num) + 1;
				st->remote_addr = ip->from_addr;
				st->remote_port = tcp->from_port;
				st->local_port = tcp->to_port;

				// И пинаем обратно SYN/ACK
				tcp->flags = TCP_FLAG_SYN|TCP_FLAG_ACK;
				tcp_xmit(st, frame, 0);
			}
		}
	}
	// Полученный пакет относится к известному соединению
	else
	{
		// Получили RST (сброс), значит прибиваем соединение
		if(tcpflags & TCP_FLAG_RST)
		{
			if((st->status == TCP_ESTABLISHED) ||
					(st->status == TCP_FIN_WAIT))
			{
				tcp_closed(id, 1);
			}
			st->status = TCP_CLOSED;
			return;
		}

		// Проверяем в пакете указатели потока
		//    и подтверждения.
		// Пакет должен находиться в потоке строго
		//    после предыдущего полученного,
		// и подтверждать последний отправленный
		//    нами пакет
		if( (ntohl(tcp->seq_num) != st->ack_num) ||
				(ntohl(tcp->ack_num) != st->seq_num) ||
				(!(tcpflags & TCP_FLAG_ACK)) )
		{
			return;
		}

		// Обновляем наш указатель подтверждения,
		//    чтобы следующим отправленным пакетом
		//    подтвердить только что полученный пакет
		st->ack_num += len;
		if( (tcpflags & TCP_FLAG_FIN) || (tcpflags & TCP_FLAG_SYN) )
			st->ack_num++;

		// Сбрасываем счётчик таймаута
		st->event_time = ticks;

		// Смотрим, в каком состоянии мы сейчас, и в зависимости
		//    от этого реагируем на полученный пакет
		switch(st->status)
		{

			// Состояние:
			//     Мы отправили SYN
			//     Теперь ждём SYN/ACK
			case TCP_SYN_SENT:
				// Получили что-то, но не SYN/ACK
				if(tcpflags != (TCP_FLAG_SYN | TCP_FLAG_ACK))
				{
					// Прибиваем соединение
					st->status = TCP_CLOSED;
					break;
				}

				// Отправляем ACK
				tcp->flags = TCP_FLAG_ACK;
				tcp_xmit(st, frame, 0);

				// Соединение установлено
				st->status = TCP_ESTABLISHED;

				// Дёргаем коллбэк, в котором
				//    прога может посылать свои данные
				tcp_read(id, frame, 0);

				break;

			// Состояние:
			//    Мы получили SYN
			//    И отправили SYN/ACK
			//    Теперь ждём ACK
			case TCP_SYN_RECEIVED:

				// Если получили не ACK,
				if(tcpflags != TCP_FLAG_ACK)
				{
					// то прибиваем соединение
					st->status = TCP_CLOSED;
					break;
				}

				// Соединение установлено
				st->status = TCP_ESTABLISHED;

				// Приложение может отправлять данные
				tcp_read(id, frame, 0);

				break;

			// Состояние:
			//    Соединение установлено
			//    Ждём ACK или FIN/ACK
			case TCP_ESTABLISHED:

				// Получили FIN/ACK
				if(tcpflags == (TCP_FLAG_FIN | TCP_FLAG_ACK))
				{
					// Отдаём данные приложению
					if(len) tcp_write(id, frame, len);

					// Отправляем FIN/ACK
					tcp->flags = TCP_FLAG_FIN|TCP_FLAG_ACK;
					tcp_xmit(st, frame, 0);

					// Соединение закрыто
					st->status = TCP_CLOSED;
					tcp_closed(id, 0);
				}
				// Получили просто ACK
				else if (tcpflags == TCP_FLAG_ACK)
				{
					// Отдаём данные приложению
					if(len) tcp_write(id, frame, len);

					// Приложение может отправлять данные
					tcp_read(id, frame, 0);

					// Если приложение не отправило ничего,
					//    но нам приходили данные,
					//    отправляем ACK
					if( (len) && (!tcp_ack_sent) )
					{
						tcp->flags = TCP_FLAG_ACK;
						tcp_xmit(st, frame, 0);
					}
				}

				break;

			// Состояние:
			//    Мы отправили FIN/ACK
			//    Ждём FIN/ACK или ACK
			case TCP_FIN_WAIT:

				// Получили FIN/ACK
				if(tcpflags == (TCP_FLAG_FIN | TCP_FLAG_ACK))
				{
					// Отдаём приложению данные
					if(len) tcp_write(id, frame, len);

					// Отправляем ACK
					tcp->flags = TCP_FLAG_ACK;
					tcp_xmit(st, frame, 0);

					// Соединение закрыто
					st->status = TCP_CLOSED;
					tcp_closed(id, 0);
				}
				// Получили данные и ACK
				// (типа удалённый узел отдаёт данные из буфера)
				else if((tcpflags == TCP_FLAG_ACK) && (len))
				{
					// Отдаём данные приложению
					tcp_write(id, frame, len);

					// Отправляем ACK
					tcp->flags = TCP_FLAG_ACK;
					tcp_xmit(st, frame, 0);
				}

				break;
		}
	}
}

// Периодическое событие
void tcp_poll()
{
	eth_frame_t *frame = (void*)net_buf;
	ip_packet_t *ip = (void*)(frame->data);
	tcp_packet_t *tcp = (void*)(ip->data);

	uint8_t id;
	tcp_state_t *st;

	// Проходимся по всем соединениям
	for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
	{
		st = tcp_pool + id;

		// Есть таймаут?
		if((st->status != TCP_CLOSED) &&
			(ticks - st->event_time > TCP_REXMIT_TIMEOUT))
		{
			// Если количество ретрансмиссий превысило
			//    допустимый предел, прибиваем соединение
			if(st->rexmit_count > TCP_REXMIT_LIMIT)
			{
				st->status = TCP_CLOSED;
				tcp_closed(id, 1);
			}
			// Иначе...
			else
			{
				// Сбрасываем счётчик таймаута
				st->event_time = ticks;

				// Увеличиваем счётчик ретрансмиссий
				st->rexmit_count++;

				// Загружаем предыдущее состояние
				st->seq_num = st->seq_num_saved;

				// Будем отпаврялт пакеты "с нуля",
				// принятых пакетов у нас в буфере нету
				tcp_send_mode = TCP_SENDING_SEND;
				tcp_ack_sent = 0;

				// В зависимсоти от состояния,
				//    производим ретрансмиссию
				switch(st->status)
				{
					// Состояние:
					//    Посылали SYN, но не
					//        получили в ответ ничего
					case TCP_SYN_SENT:
						// Посылаем SYN ещё раз
						tcp->flags = TCP_FLAG_SYN;
						tcp_xmit(st, frame, 0);
						break;

					// Состояние:
					//    Получили SYN,
					//    Отправили SYN/ACK,
					//    Но не получили ответа
					case TCP_SYN_RECEIVED:
						// Посылаем SYN/ACK ещё раз
						tcp->flags = TCP_FLAG_SYN | TCP_FLAG_ACK;
						tcp_xmit(st, frame, 0);
						break;

					// Состояние:
					//    Посылали FIN/ACK,
					//    либо посылали ACK в состоянии FIN_WAIT,
					//    но не получили ответа
					case TCP_FIN_WAIT:

						// Мы посылали ACK в состояниии FIN_WAIT
						// (st->is_closing означает, что уже получено
						//    подтверждение на отправленный нами FIN/ACK)
						if(st->is_closing)
						{
							// Посылаем ACK ещё раз
							tcp->flags = TCP_FLAG_ACK;
							tcp_xmit(st, frame, 0);
							break;
						}

						// Мы посылали FIN/ACK в состоянии ESTABLISHED,
						// после чего перешли в состояние FIN_WAIT,
						// но не получили ответа на FIN/ACK
						// Возвращаемся к состоянию ESTABLISHED
						st->status = TCP_ESTABLISHED;

					// Состояние:
					//    Мы посылали данные в сост. ESTABLISHED
					//    или ACK в состоянии ESTABLISHED
					//    или FIN/ACK в сост. ESTABLISHED
					case TCP_ESTABLISHED:
						// Просим приложение
						//    повторить последнее действие
						//    (последний параметр у tcp_read
						//        указывает на это)
						tcp_read(id, frame, 1);

						// Посылаем ACK, если нужно
						if(!tcp_ack_sent)
						{
							tcp->flags = TCP_FLAG_ACK;
							tcp_xmit(st, frame, 0);
						}
						break;
				}
			}
		}
	}
}

// обработчик получаемых данных
void tcp_write(uint8_t id, eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void *) (frame->data);
	tcp_packet_t *tcp = (void *) (ip->data);
	char *request = (void *) tcp_get_data(tcp);
}

char *css_settings_form =

"form {"
	"border: 1px solid #f7f4f4;"
	"width: 230px;"
	"margin: auto;"
	"padding: 35px;"
	"background: white;"
	"margin-top: 15px;"
"}"
"h1 {"
	"text-align: center;"
	"font-size: 23px;"
"}"
"body {"
	"background: #efefef;"
"}"
"label {"
	"margin-left: 38px;"
	"display: inline-block;"
"}"
"input {"
	"border: 1px #eaeaea solid;"
"}"
"input[type=\"checkbox\"] {"
	"margin-left: 38px;"
	"display: inline-block;"
	"margin-bottom: 12px;"
"}"
"input[type=\"text\"] {"
    "clear: right;"
    "display: block;"
    "margin: 0 auto 8px auto;"
"}"
"input[type=\"submit\"] {"
    "margin: auto;"
    "display: block;"
    "margin-top: 22px;"
    "width: 152px;"
"}";

char *html_settings_form =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"<html>"
		"<head>"
			"<title>SOMI internal MCU settings form</title>"
			"<link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\">"
		"</head>"
		"<body>"
			"<h1>MCU Network Settings</h1>"
			"<form>"
				"<label for=\"mac_address\">MAC Adress:</label>"
				"<input type=\"text\" name=\"mac_address\" />"
				"<label for=\"dhcp\">DHCP:</label>"
				"<input type=\"checkbox\" name=\"dhcp\" />"
				"<label for=\"ip_address\">IP Adress:</label>"
				"<input type=\"text\" name=\"ip_address\" />"
				"<label for=\"net_mask\">Net Mask:</label>"
				"<input type=\"text\" name=\"net_mask\" />"
				"<label for=\"port\">Port:</label>"
				"<input type=\"text\" name=\"port\" />"

				"<label for=\"gateway\">Gateway:</label>"
				"<input type=\"text\" name=\"gateway\" />"

				"<label for=\"server\">Server Domain Or Ip:</label>"
				"<input type=\"text\" name=\"server\" />"
				"<label for=\"dns_server\">DNS Server:</label>"
				"<input type=\"text\" name=\"server_addr\" />"
				"<input type=\"submit\" value=\"ok\"/>"
			"</form>"
		"</body>"
	"</html>\r\n";

// upstream callback
void tcp_read(uint8_t id, eth_frame_t *frame, uint8_t re)
{
	ip_packet_t *ip = (void *) (frame->data);
	tcp_packet_t *tcp = (void *) (ip->data);
	char *buf = (void *) (tcp->data);
	uint8_t options;

	httpd_status_t *st = &httpd_status;

	if (!st->data) {
		if (strncmp("GET ", buf, 4)) return;

		if (!strncmp("/ ", buf + 4, 2))
		{
			st->data = st->data_saved = html_settings_form;
		}
		else if (!strncmp("/styles.css", buf + 4, 11))
		{
			st->data = st->data_saved = css_settings_form;
		}
		else return;

		st->numbytes = st->numbytes_saved = strlen(st->data);
		st->statuscode = st->statuscode_saved = 200;
	}

	// нужен ретрансмит?
	if (re)
	{
		// загружаемся
		st->statuscode = st->statuscode_saved;
		st->numbytes = st->numbytes_saved;
		st->data = st->data_saved;
	}
	else
	{
		// сохраняемся перед отправкой пакета
		st->statuscode_saved = st->statuscode;
		st->numbytes_saved = st->numbytes;
		st->data_saved = st->data;
	}

	uint16_t segment_bytes = st->numbytes > TCP_SYN_MSS
		? TCP_SYN_MSS
		: st->numbytes;

	memcpy(tcp->data, st->data, segment_bytes);

	st->numbytes -= segment_bytes;
	st->data += segment_bytes;

	options = 0;
	// Если все данные отданы закрываем соединение.
	if (!st->numbytes) {
		st->data = 0;
		options |= TCP_OPTION_CLOSE;
	}

	tcp_send(id, frame, segment_bytes, options);
}

