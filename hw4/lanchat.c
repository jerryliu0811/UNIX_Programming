#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_link.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 1024

struct interface_struct{
    char name[BUFSIZE];
    char ip[NI_MAXHOST];
    char Mask[NI_MAXHOST];
    char Bcast[NI_MAXHOST];
    int hex_Mask;
    int index;
    unsigned char mac_addr[6];
    int sockfd;
};

struct interface_struct interface[50];
int interface_cnt = 0;
fd_set rfds, afds;

int get_interface_index(char *name){
    int i;
    //find interface
    for(i = 0; i < interface_cnt; i++){
        if(strcmp(name, interface[i].name) == 0){
            //old interface, return index
            return i;
        }
    }

    //new interface, add
    interface_cnt = interface_cnt + 1;
    return i;
}

void get_interface_info(void){
    struct ifaddrs *ifaddr, *ifa;
    int family, s, size, hex_Mask, swapped_hex_Mask, interface_index, isLookback, isRunning;
    size = sizeof(struct sockaddr_in);

    if(getifaddrs(&ifaddr) == -1){
        perror("getifaddrs"); exit(EXIT_FAILURE);
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if(ifa->ifa_addr == NULL){
            continue;
        }

        isLookback = (0 != (ifa->ifa_flags & IFF_LOOPBACK));
        if(isLookback){
            //printf("%s is Lookback\n", ifa->ifa_name);
            continue;
        }

        //check interface is running
        isRunning = (0 != (ifa->ifa_flags & IFF_RUNNING));
        if(!isRunning){
            //printf("%s is not running\n", ifa->ifa_name);
            continue;
        }

        //get my_interface_struct index
        interface_index = get_interface_index(ifa->ifa_name);

        family = ifa->ifa_addr->sa_family;
        if(family == AF_INET){
            //get name
            strcpy(interface[interface_index].name, ifa->ifa_name);
            
            //get ipv4
            s = getnameinfo(ifa->ifa_addr, size, interface[interface_index].ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if(s != 0){
                perror("getnameinfo1()"); exit(EXIT_FAILURE);
            }

            //get mask
            s = getnameinfo(ifa->ifa_netmask, size, interface[interface_index].Mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if(s != 0){
                perror("getnameinfo2()"); exit(EXIT_FAILURE);
            }

            //get broadcast ip
            s = getnameinfo(ifa->ifa_broadaddr, size, interface[interface_index].Bcast, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if(s != 0){
                perror("getnameinfo3()"); exit(EXIT_FAILURE);
            }

            //conver Mask to hex
            hex_Mask = inet_addr(interface[interface_index].Mask);
            swapped_hex_Mask = ((hex_Mask>>24) & 0xff)     | 
                                ((hex_Mask<<8)  & 0xff0000) | 
                                ((hex_Mask>>8)  & 0xff00)   | 
                                ((hex_Mask<<24) & 0xff000000);
            interface[interface_index].hex_Mask = swapped_hex_Mask;
        }
        else if(family == AF_PACKET){
            struct sockaddr_ll* sockaddr = (struct sockaddr_ll*)ifa->ifa_addr;

            strcpy(interface[interface_index].name, ifa->ifa_name);
            interface[interface_index].index = sockaddr->sll_ifindex;
            interface[interface_index].mac_addr[0] = sockaddr->sll_addr[0];
            interface[interface_index].mac_addr[1] = sockaddr->sll_addr[1];
            interface[interface_index].mac_addr[2] = sockaddr->sll_addr[2];
            interface[interface_index].mac_addr[3] = sockaddr->sll_addr[3];
            interface[interface_index].mac_addr[4] = sockaddr->sll_addr[4];
            interface[interface_index].mac_addr[5] = sockaddr->sll_addr[5];
        }
    }

    freeifaddrs(ifaddr);
}

void create_interface_socket(){
    int i, sockfd;
    for(i = 0; i < interface_cnt; i++){
        sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP+1));
        if(sockfd == -1){
            perror("create socket fail\n"); exit(EXIT_FAILURE);
        }
        else{
            interface[i].sockfd = sockfd;
            FD_SET(sockfd, &afds);
        }
    }
}

void print_interface(void){
    char *pch1, *pch2, *pch3, *pch4;
    int ip1, ip2, ip3, ip4;

    printf("Enumerated network interfaces:\n");
    for(int i = 0; i < interface_cnt; i++){
        //spilt ip to thr format
        pch1 = strtok(interface[i].ip, ".");
        pch2 = strtok (NULL, ".");
        pch3 = strtok (NULL, ".");
        pch4 = strtok (NULL, ".");
        ip1 = atoi(pch1);
        ip2 = atoi(pch2);
        ip3 = atoi(pch3);
        ip4 = atoi(pch4);

        //printf
        printf("%d - %s     %03d.%03d.%03d.%03d 0x%08x (%s) %02x:%02x:%02x:%02x:%02x:%02x\n", 
            interface[i].index, 
            interface[i].name, 
            ip1, ip2, ip3, ip4, 
            interface[i].hex_Mask, 
            interface[i].Bcast, 
            interface[i].mac_addr[0], 
            interface[i].mac_addr[1], 
            interface[i].mac_addr[2], 
            interface[i].mac_addr[3], 
            interface[i].mac_addr[4], 
            interface[i].mac_addr[5]
        );
    }
}

void send_broadcast_msg(char username[BUFSIZE], char msg[BUFSIZE]){
    char sendbuf[BUFSIZE];
    int tx_len, i, sockfd;
    struct ether_header *eh = (struct ether_header *) sendbuf;
    struct sockaddr_ll socket_address;

    for(i = 0; i < interface_cnt; i++){
        sockfd = interface[i].sockfd;

        //set interface index
        struct ifreq if_idx, if_mac;
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, interface[i].name, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0){
            perror("SIOCGIFINDEX\n"); exit(EXIT_FAILURE);
        }

        //set interface mac
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, interface[i].name, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0){
            perror("SIOCGIFHWADDR\n"); exit(EXIT_FAILURE);
        }

        //contruct header
        memset(sendbuf, 0, BUFSIZE);
        eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
        eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
        eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
        eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
        eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
        eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
        eh->ether_dhost[0] = 0xff;
        eh->ether_dhost[1] = 0xff;
        eh->ether_dhost[2] = 0xff;
        eh->ether_dhost[3] = 0xff;
        eh->ether_dhost[4] = 0xff;
        eh->ether_dhost[5] = 0xff;
        eh->ether_type = htons(ETH_P_IP+1);

        tx_len = 0;
        tx_len += sizeof(struct ether_header);

        //packet data
        sendbuf[tx_len++] = '[';
        for(i = 0; i < strlen(username); i++)
            sendbuf[tx_len++] = username[i];
        sendbuf[tx_len++] = ']';
        sendbuf[tx_len++] = ':';
        sendbuf[tx_len++] = ' ';
        for(i = 0; i < strlen(msg); i++)
            sendbuf[tx_len++] = msg[i];
        sendbuf[tx_len++] = 0;

        // construct dest_addr index/mac
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        socket_address.sll_halen = ETH_ALEN;
        socket_address.sll_addr[0] = 0xff;
        socket_address.sll_addr[1] = 0xff;
        socket_address.sll_addr[2] = 0xff;
        socket_address.sll_addr[3] = 0xff;
        socket_address.sll_addr[4] = 0xff;
        socket_address.sll_addr[5] = 0xff;

        // send packet
        if(sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0){
            perror("Send failed\n"); exit(EXIT_FAILURE);
        }
    }
}

void recv_broadcast_msg(int sockfd){
    char recvbuf[BUFSIZE];
    int n;

    n = recvfrom(sockfd, recvbuf, BUFSIZE, 0, NULL, NULL);
    if(n == -1){
        perror("Recvfrom failed"); exit(EXIT_FAILURE);
    }
    
    printf("<%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx> %s\n", 
        recvbuf[6], recvbuf[7], recvbuf[8], recvbuf[9], recvbuf[10], recvbuf[11],
        recvbuf+14
        );
}

int check_fd_in_interface(int fd){
    for(int i = 0; i < interface_cnt; i++){
        if(fd == interface[i].sockfd)
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[]){
    char username[BUFSIZE], msg[BUFSIZE];
    int n, nfds, fd;

    //get interface
    get_interface_info();

    //print all interface
    print_interface();

    nfds = 1024;
    FD_ZERO(&rfds);
    FD_ZERO(&afds);

    //set socket for all interface
    create_interface_socket();

    //input user name
    printf("Enter your name: ");
    n = scanf("%s", username);
    printf("Welcome, '%s'!\n", username);
    n = write(1, ">>> ", 4);

    //set stdin into fd_set
    FD_SET(0, &afds);

    while(1){
        memcpy(&rfds, &afds, sizeof(rfds));
        if(select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0){
            printf("select error\n");
        }

        if(FD_ISSET(0, &rfds)){
            //stdin is selected
            n = read(0, msg, sizeof(msg));
            if(n < 0){
                perror("read error"); exit(EXIT_FAILURE);
            }
            /*else if(n == 1){ //user no input
                continue;
            }*/
            else{
                msg[n-1] = 0;
                send_broadcast_msg(username, msg);
            }
            n = write(1, ">>> ", 4);
        }

        for(fd = 3; fd < nfds; ++fd){
            if(FD_ISSET(fd, &rfds) && check_fd_in_interface(fd)){
                recv_broadcast_msg(fd);
            }
        }
    }

    return 0;
}