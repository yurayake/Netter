#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "lib.c"

#include "asm/layer1_asm.c"
#include "asm/layer2_layer3_asm.c"
#include "asm/layer1_layer2_layer3_asm.c"

#include "option/s_start.c"
#include "option/s_save_start.c"
#include "option/key_send_start.c"
#include "option/key_rm_start.c"
#include "option/key_ls_start.c"

//オプション解析関数
int option_analysis(char *argv1_p, char *argv2_p, char *argv3_p, int argc)
{
    //オプション-sの場合の処理
    if (strcmp("-s", argv1_p) == 0 && argc == 3)
    {
        option_s_start(argv2_p);
    }
    //オプション-s_saveの場合の処理
    else if(strcmp("-s_save", argv1_p) == 0 && argc == 4)
    {
        option_s_save_start(argv2_p, argv3_p);
    }
    //オプション-key_sendの場合の処理
    else if(strcmp("-key_send", argv1_p) == 0 && argc == 3)
    {
        option_key_send_start(argv2_p);
    }
    //オプション-key_rmの場合の処理
    else if(strcmp("-key_rm", argv1_p) == 0 && argc == 3)
    {
        option_key_rm_start(argv2_p);
    }
    //オプション-key_lsの場合の処理
    else if(strcmp("-key_ls", argv1_p) == 0 && argc == 2)
    {
        option_key_ls_start();
    }
    //上記以外の場合
    else
    {
        printf("Netterコマンドの使い方を間違えています。\n");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    //argcが1または5以上の場合の処理
    if (argc == 1 || argc >= 5)
    {
        printf("netterコマンドの使い方を間違えています。\n");
        return 0;
    }

    //オプション解析
    option_analysis(argv[1], argv[2], argv[3], argc);
    return 0;
}
