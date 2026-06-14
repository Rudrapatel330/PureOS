#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include "../../drivers/pcnet.h"
#include "../../drivers/virtio/virtio_net.h"

#define IFNAME0 'e'
#define IFNAME1 'n'

static void low_level_init(struct netif *netif) {
    if (virtio_net_initialized) {
        for (int i = 0; i < 6; i++) {
            netif->hwaddr[i] = virtio_net_mac[i];
        }
    } else {
        for (int i = 0; i < 6; i++) {
            netif->hwaddr[i] = pcnet_dev.mac[i];
        }
    }
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    (void)netif;
    struct pbuf *q;
    uint8_t buffer[1514];
    uint16_t pos = 0;

    for (q = p; q != NULL; q = q->next) {
        if (pos + q->len > sizeof(buffer)) break;
        memcpy(&buffer[pos], q->payload, q->len);
        pos += q->len;
    }

    if (virtio_net_initialized) {
        virtio_net_send(buffer, pos);
    } else {
        pcnet_send(buffer, pos);
    }
    return ERR_OK;
}

void ethernetif_input(struct netif *netif, const uint8_t *packet, uint16_t len) {
    struct pbuf *p, *q;
    uint16_t pos = 0;

    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (p != NULL) {
        for (q = p; q != NULL; q = q->next) {
            memcpy(q->payload, &packet[pos], q->len);
            pos += q->len;
        }
        if (netif->input(p, netif) != ERR_OK) {
            pbuf_free(p);
        }
    }
}

err_t ethernetif_init(struct netif *netif) {
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    low_level_init(netif);
    return ERR_OK;
}
