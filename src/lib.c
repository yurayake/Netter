unsigned short checksum(unsigned short *data, int data_size)
{
  unsigned long sum = 0;

  while (data_size > 1)
  {
    sum += *data;
    data++;
    data_size -= 2;
  }

  if (data_size == 1)
  {
    sum += *(unsigned char *)data;
  }

  sum = (sum & 0xffff) + (sum >> 16);
  sum = (sum & 0xffff) + (sum >> 16);

  return ~sum;
}

int set_mac(const char *mac_addr, unsigned char *p)
{
  sscanf(mac_addr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]);
  return 0;
}

int set_ip(const char *ip_addr, u_int8_t *p)
{
  sscanf(ip_addr, "%hhd.%hhd.%hhd.%hhd", &p[0], &p[1], &p[2], &p[3]);
  return 0;
}