#ifndef ETHERNETIF_H
#define ETHERNETIF_H

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif, const uint8_t *packet, uint16_t len);

#endif /* ETHERNETIF_H */
