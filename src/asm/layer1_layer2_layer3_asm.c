int Ethernet_IPv4_TCP_send(char *filename)
{
    char dummy1[30];
    char dummy2[30];

    char nic_name[30];
    char default_mac[30];

    char count[30];

    char ether_dhost[30];
    char ether_shost[30];

    char ip_tos[30];
    char ip_id[30];
    char ip_ttl[30];
    char ip_saddr[30];
    char ip_daddr[30];

    char tcp_source[30];
    char tcp_dest[30];
    char tcp_seq[30];
    char tcp_ack_seq[30];
    char tcp_fin[30];
    char tcp_syn[30];
    char tcp_rst[30];
    char tcp_psh[30];
    char tcp_ack[30];
    char tcp_urg[30];
    char tcp_window[30];
    char tcp_urg_ptr[30];

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen");
    }

    fscanf(fp, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", dummy1, dummy2, nic_name, 
    default_mac, count, ether_dhost, ether_shost, ip_tos, ip_id, ip_ttl, ip_saddr, ip_daddr, 
    tcp_source, tcp_dest, tcp_seq, tcp_ack_seq, tcp_fin, tcp_syn, tcp_rst, tcp_psh, tcp_ack,
    tcp_urg, tcp_window, tcp_urg_ptr);

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(0x0800));
    if (sockfd == -1)
    {
        perror("socket");

    }
    struct ifreq ifreQ;
    memset(&ifreQ, 0, sizeof(struct ifreq));
    strncpy(ifreQ.ifr_name, nic_name, strlen(nic_name) + 1);
    int tmp = socket(AF_INET, SOCK_STREAM, 0);
    if (tmp == -1)
    {
        perror("socket");
    }

    if (ioctl(tmp, SIOCGIFINDEX, &ifreQ) == -1)
    {
        perror("ioctl");
    }

    char buf[sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(buf, 0, sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct tcphdr));

    // Etherヘッダ設定
    struct ether_header *ether_p = (struct ether_header *)buf;
    //宛先MACアドレス設定
    set_mac(ether_dhost, (unsigned char *)ether_p);
    ether_p += 6;
    //送信元MACアドレス設定
    set_mac(ether_shost, (unsigned char *)ether_p);
    ether_p -= 6;
    //イーサタイプ設定
    ether_p->ether_type = htons(0x0800);

    struct iphdr *ip_p = (struct iphdr *)(buf + sizeof(struct ether_header));
    ip_p->ihl = 5;
    ip_p->version = 4;
    ip_p->tos = atoi(ip_tos);
    // tot_lenは飛ばす
    ip_p->id = htons(atoi(ip_id));
    ip_p->frag_off = 0;
    ip_p->ttl = atoi(ip_ttl);
    ip_p->protocol = 6;
    // checkは飛ばす
    ip_p->saddr = inet_addr(ip_saddr);
    ip_p->daddr = inet_addr(ip_daddr);

    struct tcphdr *tcp_p = (struct tcphdr *)(buf + sizeof(struct ether_header) + sizeof(struct iphdr));
    tcp_p->source = htons(atoi(tcp_source));
    tcp_p->dest = htons(atoi(tcp_dest));
    tcp_p->seq = atoi(tcp_seq);
    tcp_p->ack_seq = atoi(tcp_ack_seq);
    tcp_p->res1 = 0;
    tcp_p->doff = 5;
    tcp_p->fin = atoi(tcp_fin);
    tcp_p->syn = atoi(tcp_syn);
    tcp_p->rst = atoi(tcp_rst);
    tcp_p->psh = atoi(tcp_psh);
    tcp_p->ack = atoi(tcp_ack);
    tcp_p->urg = atoi(tcp_urg);
    tcp_p->res2 = 0;
    tcp_p->window = htons(atoi(tcp_window));
    // checkは一旦0にしておく
    tcp_p->check = 0;
    tcp_p->urg_ptr = atoi(tcp_urg_ptr);

    // TCPチェックサムを計算
    //疑似ヘッダ構造体
    struct pseudohdr
    {
        unsigned int saddr;
        unsigned int daddr;
        unsigned char zero;
        unsigned char protocol;
        unsigned short tcp_len;
    };

    char pseudohdr_buf[sizeof(struct pseudohdr)];
    struct pseudohdr *pseudohdr_p = (struct pseudohdr *)pseudohdr_buf;

    pseudohdr_p->saddr = inet_addr(ip_saddr);
    pseudohdr_p->daddr = inet_addr(ip_daddr);
    pseudohdr_p->zero = 0;
    pseudohdr_p->protocol = 6;
    //とりあえずtcpヘッダのサイズだけ
    pseudohdr_p->tcp_len = htons(sizeof(struct tcphdr));

    //疑似ヘッダ + TCPヘッダ配列
    char tcp_check_data[sizeof(struct pseudohdr) + sizeof(struct tcphdr)];
    char *tcp_check_data_p = tcp_check_data;

    memcpy(tcp_check_data_p, pseudohdr_p, sizeof(struct pseudohdr));
    tcp_check_data_p += sizeof(struct pseudohdr);
    memcpy(tcp_check_data_p, tcp_p, sizeof(struct tcphdr));
    // tcp_check_dataにテキスト(データ)がないため、0によるパディングは不要

    tcp_p->check = checksum((unsigned short *)tcp_check_data, sizeof(struct pseudohdr) + sizeof(struct tcphdr));

    int len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct tcphdr);
    ip_p->tot_len = htons(len - sizeof(struct ether_header));
    ip_p->check = checksum((unsigned short *)ip_p, 20);

    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(0x0800);
    socket_address.sll_ifindex = ifreQ.ifr_ifindex;
    socket_address.sll_halen = 6;
    set_mac(ether_dhost, socket_address.sll_addr);

    //パケット連射
    int i;
    for (i = 0; i < atoi(count); i++)
    {
        if (sendto(sockfd, buf, len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) == -1)
        {
            perror("sendto");
        }
    }

    if (close(sockfd) == -1)
    {
        perror("close");
    }

    return 0;
}

int Ethernet_IPv4_UDP_send(char *filename)
{
    char dummy1[30];
    char dummy2[30];

    char nic_name[30];
    char default_mac[30];

    char count[30];

    char ether_dhost[30];
    char ether_shost[30];

    char ip_tos[30];
    char ip_id[30];
    char ip_ttl[30];
    char ip_saddr[30];
    char ip_daddr[30];

    char udp_sport[30];
    char udp_dport[30];

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen");
    }

    fscanf(fp, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s", dummy1, dummy2, nic_name, 
    default_mac, count, ether_dhost, ether_shost, ip_tos, ip_id, ip_ttl, ip_saddr, ip_daddr, 
    udp_sport, udp_dport);

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(0x0800));
    if (sockfd == -1)
    {
        perror("socket");
    }

    struct ifreq ifreQ;
    memset(&ifreQ, 0, sizeof(struct ifreq));
    strncpy(ifreQ.ifr_name, nic_name, strlen(nic_name) + 1);
    int tmp = socket(AF_INET, SOCK_STREAM, 0);
    if (tmp == -1)
    {
        perror("socket");
    }

    if(ioctl(tmp, SIOCGIFINDEX, &ifreQ) == -1)
    {
        perror("ioctl");
    }

    char buf[sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr)];
    memset(buf, 0, sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr));

    // Etherヘッダ設定
    struct ether_header *ether_p = (struct ether_header *)buf;
    //宛先MACアドレス設定
    set_mac(ether_dhost, (unsigned char *)ether_p);
    ether_p += 6;
    //送信元MACアドレス設定
    set_mac(ether_shost, (unsigned char *)ether_p);
    ether_p -= 6;
    //イーサタイプ設定
    ether_p->ether_type = htons(0x0800);

    // IPヘッダ設定
    struct iphdr *ip_p = (struct iphdr *)(buf + sizeof(struct ether_header));
    ip_p->ihl = 5;
    ip_p->version = 4;
    ip_p->tos = atoi(ip_tos);
    // tot_lenは飛ばす
    ip_p->id = htons(atoi(ip_id));
    ip_p->frag_off = 0;
    ip_p->ttl = atoi(ip_ttl);
    ip_p->protocol = 17;
    // checkは飛ばす
    ip_p->saddr = inet_addr(ip_saddr);
    ip_p->daddr = inet_addr(ip_daddr);

    // UDPヘッダ設定
    struct udphdr *udp_p = (struct udphdr *)(buf + sizeof(struct ether_header) + sizeof(struct iphdr));
    udp_p->uh_sport = htons(atoi(udp_sport));
    udp_p->uh_dport = htons(atoi(udp_dport));
    // uh_ulenは飛ばす
    udp_p->uh_sum = 0;

    int len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr);
    udp_p->uh_ulen = htons(sizeof(struct udphdr));
    ip_p->tot_len = htons(len - sizeof(struct ether_header));
    ip_p->check = checksum((unsigned short *)ip_p, 20);

    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(0x0800);
    socket_address.sll_ifindex = ifreQ.ifr_ifindex;
    socket_address.sll_halen = 6;
    set_mac(ether_dhost, socket_address.sll_addr);

    //パケット連射
    int i;
    for (i = 0; i < atoi(count); i++)
    {
        if(sendto(sockfd, buf, len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) == -1)
        {
            perror("sendto");
        }
    }

    if(close(sockfd) == -1)
    {
        perror("close");
    }
    return 0;
}

int layer1_layer2_layer3_asm(char *filename)
{
    //二行目の構文解析
    char dummy[30];
    char line_2[30];
    FILE *fp = fopen(filename, "r");
    if(fp == NULL)
    {
        perror("fopen");
    }

    fscanf(fp, "%s%s", dummy, line_2);
    
    if (strcmp("using_Ethernet_IPv4_UDP", line_2) == 0)
    {
        Ethernet_IPv4_UDP_send(filename);
    }
    else if (strcmp("using_Ethernet_IPv4_TCP", line_2) == 0)
    {
        Ethernet_IPv4_TCP_send(filename);
    }

    return 0;
}