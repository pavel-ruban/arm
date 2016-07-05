#include <binds.h>

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
