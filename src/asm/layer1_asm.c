int Ethernet_ARP_send(char *filename)
{
    char dummy1[30];
    char dummy2[30];

    char nic_name[30];
    char default_mac[30];

    char count[30];

    char ether_dhost[30];
    char ether_shost[30];

    char arp_ope[30];
    char arp_smac[30];
    char arp_sip[30];
    char arp_tmac[30];
    char arp_tip[30];

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen");
    }

    fscanf(fp, "%s%s%s%s%s%s%s%s%s%s%s%s", dummy1, dummy2, nic_name, 
    default_mac, count, ether_dhost, ether_shost, arp_ope, arp_smac, arp_sip, arp_tmac, arp_tip);

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(0x0806));
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

    char buf[sizeof(struct ether_header) + sizeof(struct ether_arp)];
    memset(buf, 0, sizeof(struct ether_header) + sizeof(struct ether_arp));

    // Etherヘッダ設定
    struct ether_header *ether_p = (struct ether_header *)buf;
    //宛先MACアドレス設定
    set_mac(ether_dhost, (unsigned char *)ether_p);
    ether_p += 6;
    //送信元MACアドレス設定
    set_mac(ether_shost, (unsigned char *)ether_p);
    ether_p -= 6;
    //イーサタイプ設定
    ether_p->ether_type = htons(0x0806);

    // ARPヘッダ設定
    struct ether_arp *arp_p = (struct ether_arp *)(buf + sizeof(struct ether_header));
    arp_p->ea_hdr.ar_hrd = htons(0x0001);
    arp_p->ea_hdr.ar_pro = htons(0x0800);
    arp_p->ea_hdr.ar_hln = 6;
    arp_p->ea_hdr.ar_pln = 4;
    arp_p->ea_hdr.ar_op = htons(atoi(arp_ope));
    set_mac(arp_smac, arp_p->arp_sha);
    set_ip(arp_sip, arp_p->arp_spa);
    set_mac(arp_tmac, arp_p->arp_tha);
    set_ip(arp_tip, arp_p->arp_tpa);

    int len = sizeof(struct ether_header) + sizeof(struct ether_arp);

    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(0x0806);
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

int layer1_asm(char *filename)
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
    
    if (strcmp("using_Ethernet_ARP", line_2) == 0)
    {
        Ethernet_ARP_send(filename);
    }

    return 0;
}