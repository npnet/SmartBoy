/*
 *   File name: systool.h
 *
 *  Created on: 2018年7月9日
 *      Author: Cary
 *     Version: 1.0
 */

#ifndef __SYSTOOL_H__
#define __SYSTOOL_H__

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <unistd.h>
#include <netdb.h>

#include <string>
using namespace std;

#define SYS_ERR -1
#define SYS_SUCC 0
#define BUF_SIZE 1024
#define GOOGLE_DNS 8.8.8.8
#define GOOGLE_PROT 53

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

/*
 Function Name:GetIPAddr
 Parameters:
 In:网卡名称，wlan0
 Out:地址参数
 In/Out:None.
 Return Values:成功或者失败码
 Comments:获取ip地址
 */
int GetIPAddr(char *name, char *ip)
{
    int sock_fd;
    struct ifconf conf;
    struct ifreq *ifr;
    char buff[BUF_SIZE] = {0};
    int num;
    int i;

    sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
        return -1;
    conf.ifc_len = BUF_SIZE;
    conf.ifc_buf = buff;
    if (ioctl(sock_fd, SIOCGIFCONF, &conf) < 0)
    {
        close(sock_fd);
        return -1;
    }
    num = conf.ifc_len / sizeof(struct ifreq);
    ifr = conf.ifc_req;

    for (i = 0; i < num; i++)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);
        if (ioctl(sock_fd, SIOCGIFFLAGS, ifr) < 0)
        {
            close(sock_fd);
            return -1;
        }
        if ((ifr->ifr_flags & IFF_UP) && strcmp(name, ifr->ifr_name) == 0)
        {
            //ip = inet_ntoa(sin->sin_addr);
            strcpy(ip, inet_ntoa(sin->sin_addr));
            close(sock_fd);
            return SYS_SUCC;
        }
        ifr++;
    }

    close(sock_fd);
    return -1;
}

/*
 Function Name:GetMac
 Parameters:
 In:网卡名称，wlan0
 Out:Mac参数
 In/Out:None.
 Return Values:成功或者失败码
 Comments:获取Mac地址
 */
int GetMac(char *mac, char *net)
{
    struct ifreq ifreq;
    //  char mac_origin[20];
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return SYS_ERR;
    }
    strcpy(ifreq.ifr_name, net);
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        close(sock);
        return SYS_ERR;
    }

    close(sock);

    snprintf(mac, 20, "%02X:%02X:%02X:%02X:%02X:%02X",
             (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
             (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
             (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
             (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
             (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
             (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
    //mac = mac_origin;
    return SYS_SUCC;
}

/*
 Function Name:CheckNetworkConnectedByUrl
 Parameters:
 In:url和端口
 Out:Mac参数
 In/Out:None.
 Return Values:0成功-1失败
 Comments:根据URL判断是否连上网络
 */
int CheckNetworkConnectedByUrl(char *url, int port)
{
    int fd;
    int in_len = 0;
    char ipaddr[16];
    struct sockaddr_in servaddr;
    struct hostent *host;

    host = gethostbyname(url);
    if (host == NULL)
    {
        return SYS_ERR;
    }
    for (int i = 0; (host->h_addr_list)[i] != NULL; i++)
    {
        inet_ntop(AF_INET, (host->h_addr_list)[i], ipaddr, 16);
    }
    in_len = sizeof(struct sockaddr_in);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        return SYS_ERR;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ipaddr);
    memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
    if (connect(fd, (struct sockaddr *)&servaddr, in_len) < 0)
    {
        return SYS_ERR;
    }
    else
    {
        return SYS_SUCC;
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* C++ */
#endif