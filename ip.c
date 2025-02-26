#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "util.h"
#include "net.h"
#include "ip.h"

struct ip_hdr {
	uint8_t vhl;  //バージョンとヘッダ長
	uint8_t tos;
	uint16_t total;
	uint16_t id;
	uint16_t offset; //フラグとフラグメントオフセット長
	uint8_t ttl;
	uint8_t protocol;
	uint16_t sum;
	ip_addr_t src;
	ip_addr_t dst;
	uint8_t options[0];
};

const ip_addr_t IP_ADDR_ANY		  = 0x00000000; /* 0.0.0.0 */
const ip_addr_t IP_ADDR_BROADCAST = 0xffffffff; /* 255.255.255.255 */

int
ip_addr_pton(const char *p, ip_addr_t *n)
{
	char *sp, *ep;
	int idx;
	long ret;

	sp = (char *)p;
	for (idx = 0; idx < 4; idx++) {
		ret = strtol(sp, &ep, 10);
		if (ret < 0 || ret > 255) {
			return -1;
		}
		if (ep == sp) {
			return -1;
		}
		if ((idx == 3 && *ep != '\0') || (idx != 3 && *ep != '.')) {
			return -1;
		}
		((uint8_t *)n)[idx] = ret;
		sp = ep + 1;
 	}
	return 0;
}

char *
ip_addr_ntop(const ip_addr_t n, char *p, size_t size)
{
	uint8_t *u8;

	u8 = (uint8_t *)&n;
	snprintf(p, size, "%d.%d.%d.%d", u8[0], u8[1], u8[2], u8[3]);
	return p;
}

void
ip_dump(const uint8_t *data, size_t len)
{
	struct ip_hdr *hdr;
	uint8_t v, hl, hlen;
	uint16_t total, offset;
	char addr[IP_ADDR_STR_LEN];

	flockfile(stderr);
	hdr = (struct ip_hdr *)data;
	v = (hdr->vhl & 0xf0) >> 4;
	hl = hdr->vhl & 0x0f;
	hlen = hl << 2; //IPヘッダ長 … 32bit（4byte）単位の値が格納されているので4倍して 8bit（1byte）単位の値にする
	fprintf(stderr, "	vhl: 0x%02x [v: %u, hl: %u (%u)]\n", hdr->vhl, v, hl, hlen);
	fprintf(stderr, "	tos: 0x%02x\n", hdr->tos);
	total = ntoh16(hdr->total);
	fprintf(stderr, "	total: %u (payload: %u)\n", total, total - hlen);
	fprintf(stderr, "	id: %u\n", ntoh16(hdr->id));
	offset = ntoh16(hdr->offset);
	fprintf(stderr, "	offset: 0x%04x [flags=%x, offset=%u]\n", offset, (offset & 0xe000) >> 13, offset & 0x1fff);
	fprintf(stderr, );
	fprintf(stderr, "	ttl: %u\n", hdr->ttl);
	fprintf(stderr, "	protocol: %u\n", hdr->protocol);
	fprintf(stderr, "	sum: 0x%04x\n", ntoh16(hdr->sum));
	fprintf(stderr, "	src: %s\n", ip_addr_ntop(hdr->src, addr, sizeof(addr))); //network binary to printable text
	fprintf(stderr, "	dst: %s\n", ip_addr_ntop(hdr->dst, addr, sizeof(addr))); //printable text to network binary
#ifdef HEXDUMP
	hexdump(stderr, data, len);
#endif
	funlo0ckfile(stderr);
}

static void
ip_input(const uint8_t *data, size_t len, struct net_device *dev)
{
	struct ip_hdr *hdr;
	uint16_t offset;

	if (len < IP_HDR_SIZE_MIN) {
		errorf("too short");
		return ;
	}
	hdr = (struct ip_hdr *)data;
	if ()

	offset = ntoh16(hdr->offset);
	if (offset & 0x2000 || offset & 0x1fff) {
		errorf("fragments are not supported");
		return ;
	}
	debugf("dev=%s, protocol=%u, total=%u", dev->name, hdr->protocol, total);
	ip_dump(data, total);
}

int
ip_init(void)
{
	if (net_protocol_register(NET_PROTOCOL_TYPE_IP, ip_input) == -1) {
		errorf("net_protocol_register() failure");
		return -1;
	}
	return 0;
}
