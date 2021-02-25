
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/priv/tcp_priv.h"
#include "netif/xadapter.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xstatus.h"

#include "udp_1ge_client.h"

#define INIT_ATON_OK 1

static struct udp_pcb *pcb = NULL;

static err_t send_udp_packet(void);
static void recv_udp_packet(void *arg, struct udp_pcb *tpcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

int32_t init_udp(struct netif *server_netif) {

    unsigned char mac_ethernet_address[6] = {0x00, 0x0a, 0x35, 0x00, 0x01, 0x02};

    if (NULL == server_netif) {
        return XST_FAILURE;
    }

    lwip_init();

    if (NULL == xemac_add(server_netif, NULL, NULL, NULL,
                          mac_ethernet_address, XPAR_ETHERNET_BASEADDR)) {
        xil_printf("Udp client: error adding N/W interface\r\n");
        return XST_FAILURE;
    }

    netif_set_default(server_netif);
    netif_set_up(server_netif);

    if (INIT_ATON_OK != inet_aton(UDP_CLIENT_IP_ADDRESS, &server_netif->ip_addr)) {
		xil_printf("Udp client ERROR: invalid IP address: %s\r\n", UDP_CLIENT_IP_ADDRESS);
		return XST_FAILURE;
	}

    if (INIT_ATON_OK != inet_aton(UDP_IP_MASK, &server_netif->netmask)) {
   		xil_printf("Udp client ERROR: invalid IP mask: %s\r\n", UDP_IP_MASK);
   		return XST_FAILURE;
   	}

    if (INIT_ATON_OK != inet_aton(UDP_CLIENT_GW_IP_ADDRESS, &server_netif->gw)) {
		xil_printf("Udp client ERROR: invalid gw IP address: %s\r\n", UDP_IP_MASK);
		return XST_FAILURE;
	}

    xil_printf("Udp client: board IP  : %d.%d.%d.%d\r\n", ip4_addr1(&(server_netif->ip_addr)),
                                                          ip4_addr2(&(server_netif->ip_addr)),
                                                          ip4_addr3(&(server_netif->ip_addr)),
                                                          ip4_addr4(&(server_netif->ip_addr)));
    xil_printf("Udp client: netmask   : %d.%d.%d.%d\r\n", ip4_addr1(&(server_netif->netmask)),
                                                          ip4_addr2(&(server_netif->netmask)),
                                                          ip4_addr3(&(server_netif->netmask)),
                                                          ip4_addr4(&(server_netif->netmask)));
    xil_printf("Udp client: gateway IP: %d.%d.%d.%d\r\n", ip4_addr1(&(server_netif->gw)),
                                                          ip4_addr2(&(server_netif->gw)),
                                                          ip4_addr3(&(server_netif->gw)),
                                                          ip4_addr4(&(server_netif->gw)));

    xil_printf("Udp client: connecting to %s on port %s\r\n", UDP_SERVER_IP_ADDRESS, UDP_PORT);
    return XST_SUCCESS;
}

err_t transfer_udp_packet(void) {

    return send_udp_packet();
}

static err_t send_udp_packet(void) {

    const int8_t *send_buf = "ready"; //TODO:
	err_t error = ERR_OK;
	size_t send_buf_size = (strlen(send_buf) + 1);//TODO:
	struct pbuf *packet = NULL;

    packet = pbuf_alloc(PBUF_TRANSPORT, send_buf_size, PBUF_POOL);
    if ((NULL == packet) || (send_buf_size > packet->len)) {
        xil_printf("Udp client: error allocating pbuf to send\r\n");
        return ERR_BUF;
    }

    memcpy(packet->payload, send_buf, send_buf_size);

    error = udp_send(pcb, packet);
    if (ERR_OK != error) {
        xil_printf("Udp client: error on udp_send\r\n");
        xil_printf("Udp client: error: %d\r\n", error);
        usleep(100);
    }

    pbuf_free(packet);

    return error;
}

static void recv_udp_packet(void *arg, struct udp_pcb *tpcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
	xil_printf("%s\n\r",(char *)(p->payload));
}

err_t init_udp_transfer(void) {

    err_t error = ERR_OK;
    ip_addr_t remote_addr;
    memset(&remote_addr, 0, sizeof(ip_addr_t));

    if (INIT_ATON_OK != inet_aton(UDP_SERVER_IP_ADDRESS, &remote_addr)) {
        xil_printf("Udp client ERROR: invalid server IP address: %s\r\n", UDP_SERVER_IP_ADDRESS);
        return XST_FAILURE;
    }

    pcb = udp_new();
	if (NULL == pcb) {
		xil_printf("Udp client: error in PCB creation: out of memory\r\n");
		return ERR_BUF;
	}


	if (ERR_OK != udp_connect(pcb, &remote_addr, strtoul(UDP_PORT,NULL,10))) {
	    xil_printf("Udp client ERROR: the udp_connect ERROR: %s\r\n", UDP_SERVER_IP_ADDRESS);
	    return XST_FAILURE;
	}

    udp_recv(pcb, recv_udp_packet, NULL);

	usleep(10);
	return error;
}
