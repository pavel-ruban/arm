#include <binds.h>
// Режим отправки пакетов
// Устанавливается в 1 после отправки пакета
//  (следующие пакеты по умолчанию
//    будут отправляться на тот же адрес)
uint8_t tcp_use_resend;

// Пул TCP-соединений
tcp_state_t tcp_pool[TCP_MAX_CONNECTIONS];

uint8_t tcp_listen(uint8_t id, eth_frame_t *frame)
{
    ip_packet_t *ip = (void*)(frame->data);
    tcp_packet_t *tcp = (void*)(ip->data);

    // Если получили запрос на подключение на порт 1212,
    //    подтверждаем соединение
    if(tcp->to_port == htons(1212))
        return 1;

    return 0;
}

void tcp_opened(uint8_t id, eth_frame_t *frame)
{
    ip_packet_t *ip = (void*)(frame->data);
    tcp_packet_t *tcp = (void*)(ip->data);

    // Соединение открыто - отправляем "Hello!"
    memcpy(tcp->data, (void*)"Hello!\n", 7);
    tcp_send(id, frame, 7, 0);
}

void tcp_data(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t closing)
{
    ip_packet_t *ip = (void*)(frame->data);
    tcp_packet_t *tcp = (void*)(ip->data);
    char *in_data = (void*)tcp_get_data(tcp);

    // Получены данные
    if( (!closing) && (len) )
    {
        // получили команду "quit" -
        //    отправляем "Bye" и закрываем соединение
        if(memcmp(in_data, "quit", 4) == 0)
        {
            memcpy(tcp->data, "Bye!\n", 5);
            tcp_send(id, frame, 5, 1);
            return;
        }

        // Отправляем "OK"
        memcpy(tcp->data, "OK\n", 3);
        tcp_send(id, frame, 3, 0);
    }
}

void tcp_closed(uint8_t id, uint8_t reset)
{
}

// Создаёт TCP-соединение
uint8_t tcp_open(uint32_t addr, uint16_t port, uint16_t local_port)
{
    eth_frame_t *frame = (void*)net_buf;
    ip_packet_t *ip = (void*)(frame->data);
    tcp_packet_t *tcp = (void*)(ip->data);
    tcp_state_t *st = 0, *pst;
    uint8_t id;
    uint16_t len;

    // Ищем свободный слот для соединения
    for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
    {
        pst = tcp_pool + id;

        if(pst->status == TCP_CLOSED)
        {
            st = pst;
            break;
        }
    }

    if(st)
    {
        // Отправляем SYN (активное открытие, шаг 1)
        ip->to_addr = addr;
        ip->from_addr = ip_addr;
        ip->protocol = IP_PROTOCOL_TCP;

        tcp->to_port = port;
        tcp->from_port = local_port;
        tcp->seq_num = 0;
        tcp->ack_num = 0;
        tcp->data_offset = (sizeof(tcp_packet_t) + 4) << 2;
        tcp->flags = TCP_FLAG_SYN;
        tcp->window = htons(TCP_WINDOW_SIZE);
        tcp->cksum = 0;
        tcp->urgent_ptr = 0;
        tcp->data[0] = 2; // MSS option
        tcp->data[1] = 4; // MSS option length = 4 bytes
        tcp->data[2] = TCP_SYN_MSS>>8;
        tcp->data[3] = TCP_SYN_MSS&0xff;

        len = sizeof(tcp_packet_t) + 4;

        tcp->cksum = ip_cksum(len + IP_PROTOCOL_TCP,
            (uint8_t*)tcp - 8, len + 8);

        if(ip_send(frame, len))
        {
            // Пакет отправлен, добавляем соединение в пул
            st->status = TCP_SYN_SENT;
            st->rx_time = rtime();
            st->remote_port = port;
            st->local_port = local_port;
            st->remote_addr = addr;

            return id;
        }
    }

    return 0xff;
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

// Отправка TCP-данных
void tcp_send(uint8_t id, eth_frame_t *frame, uint16_t len, uint8_t close)
{
    ip_packet_t *ip = (void*)(frame->data);
    tcp_packet_t *tcp = (void*)(ip->data);
    tcp_state_t *st = tcp_pool + id;

    // Соединение установлена и нам есть что отправить (данные или FIN)
    if( (st->status == TCP_ESTABLISHED) && (len || close) )
    {
        // Отправляем FIN?
        if(close)
        {
            // Ждём пока удалённый узел тоже отправит FIN
            st->status = TCP_FIN_WAIT;
            tcp->flags = TCP_FLAG_FIN|TCP_FLAG_ACK;
        }

        // Обычные данные
        else
        {
            tcp->flags = TCP_FLAG_ACK;
        }

        // Отправляем пакет
        tcp_reply(frame, len);
    }
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

// Обработка TCP-пакета
void tcp_filter(eth_frame_t *frame, uint16_t len)
{
    ip_packet_t *ip = (void*)(frame->data);
    tcp_packet_t *tcp = (void*)(ip->data);
    tcp_state_t *st = 0, *pst;
    uint8_t id, tcpflags;

    // Пакет не на наш адрес - выбрасываем
    if(ip->to_addr != ip_addr)
        return;

    // Получен пакет, отправлять будем с обменом адресов
    tcp_use_resend = 0;

    // Рассчитываем количество полученных данных
    len -= tcp_head_size(tcp);

    // Выбираем интересные нам флаги (SYN/ACK/FIN/RST)
    tcpflags = tcp->flags & (TCP_FLAG_SYN|TCP_FLAG_ACK|
        TCP_FLAG_RST|TCP_FLAG_FIN);

    // Смотрим к какому соединению относится данный пакет
    for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
    {
        pst = tcp_pool + id;

        if( (pst->status != TCP_CLOSED) &&
            (ip->from_addr == pst->remote_addr) &&
            (tcp->from_port == pst->remote_port) &&
            (tcp->to_port == pst->local_port) )
        {
            st = pst;
            break;
        }
    }

    // Нет такого соединения
    if(!st)
    {
        // Получили SYN - запрос открытия соединения
        //  (пассивное открытие, шаг 1)
        if( tcpflags == TCP_FLAG_SYN )
        {
            // Ищем свободный слот для соединения
            for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
            {
                pst = tcp_pool + id;

                if(pst->status == TCP_CLOSED)
                {
                    st = pst;
                    break;
                }
            }

            // Слот найден и приложение подтверждает открытие соединения?
            if(st && tcp_listen(id, frame))
            {
                // Добавляем новое соединение
                st->status = TCP_SYN_RECEIVED;
                st->rx_time = rtime();
                st->remote_port = tcp->from_port;
                st->local_port = tcp->to_port;
                st->remote_addr = ip->from_addr;

                // Отправляем SYN/ACK
                //    (пассивное открытие, шаг 2)
                tcp->flags = TCP_FLAG_SYN|TCP_FLAG_ACK;
                tcp_step(tcp, 1);
                tcp->seq_num = gettc() + (gettc() << 16);
                tcp_reply(frame, 0);
            }
        }
    }

    else
    {
        st->rx_time = rtime();

        switch(st->status)
        {

        // Был отправлен SYN (активное открытие, пройден шаг 1)
        case TCP_SYN_SENT:

            // Если полученный пакет не SYN/ACK (RST, etc.)
            // прибиваем соединение
            if(tcpflags != (TCP_FLAG_SYN|TCP_FLAG_ACK))
            {
                st->status = TCP_CLOSED;
                break;
            }

            // Подтверждаем принятые данные,
            //    устанавливаем указатель потока
            tcp_step(tcp, len+1);

            // Получили SYN/ACK (шаг 2), отправляем ACK (шаг 3)
            tcp->flags = TCP_FLAG_ACK;
            tcp_reply(frame, 0);

            // Всё, соединение установлено
            st->status = TCP_ESTABLISHED;

            // Оповещаем приложение что открыто соединение
            tcp_opened(id, frame);

            break;

        // Был получен SYN (пассивное открытие, шаг 1),
        //    отправлен SYN/ACK (шаг 2)
        case TCP_SYN_RECEIVED:

            // Если полученный пакет - не ACK,
            //    прибиваем соединение
            if(tcpflags != TCP_FLAG_ACK)
            {
                st->status = TCP_CLOSED;
                break;
            }

            // Устанавливаем указатель потока и
            //    указатель подтверждения
            tcp_step(tcp, 0);

            // Получен ACK (шаг 3), соединение открыто
            st->status = TCP_ESTABLISHED;

            // Оповещаем приложение
            tcp_opened(id, frame);

            break;

        // Соединение установлено
        case TCP_ESTABLISHED:

            // Получили не ACK и не FIN/ACK
            if( (tcpflags & ~TCP_FLAG_FIN) != TCP_FLAG_ACK )
            {
                // Прибиваем соединение
                st->status = TCP_CLOSED;

                // Оповещаем приложение
                tcp_closed(id, 1);

                break;
            }

            // Получили FIN/ACK (пассивное закрытие, шаг 1)
            if(tcpflags == (TCP_FLAG_FIN|TCP_FLAG_ACK))
            {
                // Устанавливаем указатель потока и
                //    указатель подтверждения
                tcp_step(tcp, len+1);

                // Отвечаем FIN/ACK (пассивное закрытие, шаг 2)
                tcp->flags = TCP_FLAG_FIN|TCP_FLAG_ACK;
                tcp_reply(frame, 0);

                // Соединение закрыто
                st->status = TCP_CLOSED;

                if(len)
                {
                    // Если вместе с FIN/ACK приходили данные
                    //    (такое может быть), отдаём их приложению
                    tcp_data(id, frame, len, 1);
                }

                // Оповещаем приложение, что соединение закрыто
                tcp_closed(id, 0);

                break;
            }

            // Устанавливаем указатель потока и
            //    указатель подтверждения
            tcp_step(tcp, len);

            if(len)
            {
                // Отправляем подтверждение
                tcp->flags = TCP_FLAG_ACK;
                tcp_reply(frame, 0);
            }

            // Отдаём данные приложению,
            //    забираем данные у приложения
            tcp_data(id, frame, len, 0);

            break;

        // Был отправлен FIN/ACK (активное закрытие, шаг 1)
        case TCP_FIN_WAIT:

            // Получено что-то, отличающееся от ACK или FIN/ACK?
            if( (tcpflags & ~TCP_FLAG_FIN) != TCP_FLAG_ACK )
            {
                // Прибиваем соединение
                st->status = TCP_CLOSED;

                // Оповещаем приложение
                tcp_closed(id, 1);
                break;
            }

            // Получен FIN/ACK (активное закрытие, шаг 2 или 3)
            if(tcpflags == (TCP_FLAG_FIN|TCP_FLAG_ACK))
            {
                // Устанавливаем указатели потока и подтверждения
                tcp_step(tcp, len+1);

                // Отправлям ACK (активное закрытие, шаг 3 или 4)
                tcp->flags = TCP_FLAG_ACK;
                tcp_reply(frame, 0);

                // Соединение закрыто
                st->status = TCP_CLOSED;

                if(len)
                {
                    // Отдаём данные приложению (если были)
                    tcp_data(id, frame, len, 1);
                }

                // Оповещаем приложение
                tcp_closed(id, 0);

                break;
            }

            // Соединение ещё не закрыто,
            //    у удалённого узла остались данные в буфере
            if(len)
            {
                // Устанавливаем указатель подтверждения
                tcp_step(tcp, len);

                // Отправляем подтверждение
                tcp->flags = TCP_FLAG_ACK;
                tcp_reply(frame, 0);

                // Отдаём данные приложению
                tcp_data(id, frame, len, 1);
            }

            break;

        default:
            break;
        }
    }
}

// Вызывается периодически
void tcp_poll()
{
    uint8_t id;
    tcp_state_t *st;

    tcp_use_resend = 0;

    // Пробегаемся по соединениям
    for(id = 0; id < TCP_MAX_CONNECTIONS; ++id)
    {
        st = tcp_pool + id;

        // Если у соединения долго не было активности
        if( (st->status != TCP_CLOSED) &&
            (rtime() - st->rx_time > TCP_TIMEOUT) )
        {
            // Прибиваем его
            st->status = TCP_CLOSED;

            // И оповещаем приложение
            tcp_closed(id, 1);
        }
    }
}

