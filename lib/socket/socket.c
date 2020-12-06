#include <fudge.h>
#include <abi.h>
#include <net.h>
#include "socket.h"

static void send(unsigned int descriptor, void *buffer, unsigned int count)
{

    file_open(descriptor);
    file_writeall(descriptor, buffer, count);
    file_close(descriptor);

}

unsigned int socket_arp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short operation, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char sip[IPV4_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE])
{

    unsigned char *data = output;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_ARP, local->haddress, router->haddress);
    struct arp_header *aheader = arp_putheader(data + ethernet_hlen(eheader), 1, ETHERNET_ADDRSIZE, ETHERNET_TYPE_IPV4, IPV4_ADDRSIZE, operation);

    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader), sha, ETHERNET_ADDRSIZE); 
    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE, sip, IPV4_ADDRSIZE); 
    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE, tha, ETHERNET_ADDRSIZE); 
    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE, tip, IPV4_ADDRSIZE); 

    return ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE;

}

unsigned int socket_icmp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned char type, unsigned char code, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct icmp_header) + count;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_IPV4, local->haddress, router->haddress);
    struct ipv4_header *iheader = ipv4_putheader(data + ethernet_hlen(eheader), local->paddress, remote->paddress, IPV4_PROTOCOL_ICMP, length);
    struct icmp_header *icmpheader = icmp_putheader(data + ethernet_hlen(eheader) + ipv4_hlen(iheader), type, code);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + icmp_hlen(icmpheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = icmp_calculatechecksum(icmpheader, icmp_hlen(icmpheader) + count);

    buffer_copy(icmpheader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + icmp_hlen(icmpheader) + count;

}

unsigned int socket_tcp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short flags, unsigned int seq, unsigned int ack, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct tcp_header) + count;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_IPV4, local->haddress, router->haddress);
    struct ipv4_header *iheader = ipv4_putheader(data + ethernet_hlen(eheader), local->paddress, remote->paddress, IPV4_PROTOCOL_TCP, length);
    struct tcp_header *theader = tcp_putheader(data + ethernet_hlen(eheader) + ipv4_hlen(iheader), local->info.tcp.port, remote->info.tcp.port, flags, seq, ack);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = tcp_checksum(theader, local->paddress, remote->paddress, tcp_hlen(theader) + count);

    buffer_copy(theader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader) + count;

}

unsigned int socket_udp_build(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct udp_header) + count;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_IPV4, local->haddress, router->haddress);
    struct ipv4_header *iheader = ipv4_putheader(data + ethernet_hlen(eheader), local->paddress, remote->paddress, IPV4_PROTOCOL_UDP, length);
    struct udp_header *uheader = udp_putheader(data + ethernet_hlen(eheader) + ipv4_hlen(iheader), local->info.udp.port, remote->info.udp.port, count);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = udp_checksum(uheader, local->paddress, remote->paddress, udp_hlen(uheader) + count);

    buffer_copy(uheader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader) + count;

}

unsigned int socket_ethernet_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);

    return buffer_write(output, outputcount, eheader, ethernet_hlen(eheader), 0);
    
}

unsigned int socket_arp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_ARP)
    {

        struct arp_header *aheader = (struct arp_header *)(data + elen);

        return buffer_write(output, outputcount, aheader, arp_len(aheader), 0);

    }

    return 0;

}

unsigned int socket_ipv4_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);

        return buffer_write(output, outputcount, iheader, ipv4_len(iheader), 0);

    }

    return 0;

}

unsigned int socket_icmp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct icmp_header *icmpheader = (struct icmp_header *)(data + elen + ilen);

            return buffer_write(output, outputcount, icmpheader, itot - ilen, 0);

        }

    }

    return 0;

}

unsigned int socket_tcp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);

            return buffer_write(output, outputcount, theader, itot - ilen, 0);

        }

    }

    return 0;

}

unsigned int socket_udp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct tcp_header *uheader = (struct tcp_header *)(data + elen + ilen);

            return buffer_write(output, outputcount, uheader, itot - ilen, 0);

        }

    }

    return 0;

}

unsigned int socket_icmp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct icmp_header *header, void *pdata, unsigned int psize)
{

    struct message_data data;

    switch (header->type)
    {

    case ICMP_ECHOREQUEST:
        send(descriptor, &data, socket_icmp_build(local, remote, router, &data, ICMP_ECHOREPLY, 0, psize, pdata));

        break;

    }

    return 0;

}

unsigned int socket_tcp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct tcp_header *header, void *pdata, unsigned int psize)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_LISTEN:
        if (header->flags[1] == TCP_FLAGS1_SYN)
        {

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_ACK | TCP_FLAGS1_SYN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        break;

    case TCP_STATE_SYNSENT:
        if (header->flags[1] == (TCP_FLAGS1_SYN | TCP_FLAGS1_ACK))
        {

            local->info.tcp.state = TCP_STATE_ESTABLISHED;
            local->info.tcp.seq = net_load32(header->ack);
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        else if (header->flags[1] == TCP_FLAGS1_SYN)
        {

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        break;

    case TCP_STATE_SYNRECEIVED:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_ESTABLISHED;
            local->info.tcp.seq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_ESTABLISHED:
        if (header->flags[1] == (TCP_FLAGS1_PSH | TCP_FLAGS1_ACK))
        {

            local->info.tcp.seq = net_load32(header->ack);
            remote->info.tcp.seq = net_load32(header->seq) + psize;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            return psize;

        }

        else if (header->flags[1] == (TCP_FLAGS1_FIN | TCP_FLAGS1_ACK))
        {

            local->info.tcp.state = TCP_STATE_CLOSEWAIT;
            local->info.tcp.seq = net_load32(header->ack);
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            /* Wait for application to close down */

            local->info.tcp.state = TCP_STATE_LASTACK;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_FIN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        else if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.seq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_FINWAIT1:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_FINWAIT2;
            local->info.tcp.seq = net_load32(header->ack);

        }

        else if (header->flags[1] == TCP_FLAGS1_FIN)
        {

            local->info.tcp.state = TCP_STATE_CLOSING;
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        break;

    case TCP_STATE_FINWAIT2:
        if (header->flags[1] == TCP_FLAGS1_FIN)
        {

            local->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_CLOSEWAIT:
        break;

    case TCP_STATE_CLOSING:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_TIMEWAIT;
            local->info.tcp.seq = net_load32(header->ack);

            /* Sleep some time */

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_LASTACK:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_CLOSED;
            local->info.tcp.seq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_TIMEWAIT:
        break;

    case TCP_STATE_CLOSED:
        break;

    }

    return 0;

}

unsigned int socket_udp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct udp_header *header, void *pdata, unsigned int psize)
{

    return psize;

}

static unsigned int receiveicmp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_ICMP)
        {

            struct icmp_header *icmpheader = (struct icmp_header *)(data + elen + ilen);
            unsigned short icmplen = icmp_hlen(icmpheader);
            void *pdata = data + elen + ilen + icmplen;
            unsigned int psize = itot - (ilen + icmplen);

            socket_icmp_respond(descriptor, local, remote, router, icmpheader, pdata, psize);

        }

    }

    return 0;

}

static unsigned int receivetcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);
            unsigned short tlen = tcp_hlen(theader);

            if (net_load16(theader->tp) == net_load16(local->info.tcp.port))
            {

                void *pdata = data + elen + ilen + tlen;
                unsigned int psize = itot - (ilen + tlen);

                if (socket_tcp_respond(descriptor, local, remote, router, theader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

unsigned int socket_tcp_receive2(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);
            unsigned short tlen = tcp_hlen(theader);

            if (net_load16(theader->tp) == net_load16(local->info.tcp.port))
            {

                void *pdata = data + elen + ilen + tlen;
                unsigned int psize = itot - (ilen + tlen);

                if (!remote->resolved)
                {

                    socket_tcp_bind(remote, iheader->sip, theader->sp, net_load32(theader->seq));

                    remote->resolved = 1;

                }

                if (socket_tcp_respond(descriptor, local, remote, router, theader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

static unsigned int receiveudp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);
            unsigned short ulen = udp_hlen(uheader);

            if (net_load16(uheader->tp) == net_load16(local->info.udp.port))
            {

                void *pdata = data + elen + ilen + ulen;
                unsigned int psize = itot - (ilen + ulen);

                if (socket_udp_respond(descriptor, local, remote, router, uheader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

unsigned int socket_udp_receive2(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);
            unsigned short ulen = udp_hlen(uheader);

            if (net_load16(uheader->tp) == net_load16(local->info.udp.port))
            {

                void *pdata = data + elen + ilen + ulen;
                unsigned int psize = itot - (ilen + ulen);

                if (!remote->resolved)
                {

                    socket_udp_bind(remote, iheader->sip, uheader->sp);

                    remote->resolved = 1;

                }

                if (socket_udp_respond(descriptor, local, remote, router, uheader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

unsigned int socket_receive(unsigned int descriptor, unsigned int protocol, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    switch (protocol)
    {

    case IPV4_PROTOCOL_ICMP:
        return receiveicmp(descriptor, local, remote, router, count, buffer, outputcount, output);

    case IPV4_PROTOCOL_TCP:
        return receivetcp(descriptor, local, remote, router, count, buffer, outputcount, output);

    case IPV4_PROTOCOL_UDP:
        return receiveudp(descriptor, local, remote, router, count, buffer, outputcount, output);

    }

    return 0;

}

static unsigned int sendtcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_ESTABLISHED:
        send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_PSH | TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, psize, pdata));

        return psize;

    }

    return 0;

}

static unsigned int sendudp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata)
{

    struct message_data data;

    send(descriptor, &data, socket_udp_build(local, remote, router, &data, psize, pdata));

    return psize;

}

unsigned int socket_send(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata)
{

    switch (protocol)
    {

    case IPV4_PROTOCOL_TCP:
        return sendtcp(descriptor, local, remote, router, psize, pdata);

    case IPV4_PROTOCOL_UDP:
        return sendudp(descriptor, local, remote, router, psize, pdata);

    }

    return 0;

}

void socket_listen(unsigned int descriptor, unsigned char protocol, struct socket *local)
{

    switch (protocol)
    {

    case IPV4_PROTOCOL_TCP:
        local->info.tcp.state = TCP_STATE_LISTEN;

        break;

    case IPV4_PROTOCOL_UDP:
        break;

    }

}

void socket_connect(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, struct socket *router)
{

    struct message_data data;

    switch (protocol)
    {

    case IPV4_PROTOCOL_TCP:
        local->info.tcp.state = TCP_STATE_SYNSENT;

        send(descriptor, &data, socket_tcp_build(local, remote, router, &data, TCP_FLAGS1_SYN, local->info.tcp.seq, 0, 0, 0));

        break;

    case IPV4_PROTOCOL_UDP:
        break;

    }

}

void socket_resolveremote(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router)
{

    struct socket multicast;
    struct message_data data;
    unsigned char haddress[ETHERNET_ADDRSIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    buffer_copy(&multicast, router, sizeof (struct socket));
    buffer_copy(multicast.haddress, haddress, ETHERNET_ADDRSIZE); 
    send(descriptor, &data, socket_arp_build(local, remote, &multicast, &data, ARP_REQUEST, local->haddress, local->paddress, remote->haddress, remote->paddress));

}

void socket_resolvelocal(unsigned int descriptor, struct socket *socket)
{

    file_open(descriptor);
    file_readall(descriptor, socket->haddress, ETHERNET_ADDRSIZE);
    file_close(descriptor);

    socket->resolved = 1;

}

void socket_bind(struct socket *socket, unsigned char address[IPV4_ADDRSIZE])
{

    buffer_copy(&socket->paddress, address, IPV4_ADDRSIZE);

}

void socket_tcp_bind(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq)
{

    socket_bind(socket, address);
    buffer_copy(&socket->info.tcp.port, port, TCP_PORTSIZE);

    socket->info.tcp.state = TCP_STATE_CLOSED;
    socket->info.tcp.seq = seq;

}

void socket_udp_bind(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE])
{

    socket_bind(socket, address);
    buffer_copy(&socket->info.udp.port, port, UDP_PORTSIZE);

}

void socket_init(struct socket *socket)
{

    socket->resolved = 0;

}

