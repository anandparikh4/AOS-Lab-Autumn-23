#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <linux/tcp.h>
#include "part2.bpf.h"



#define NUMSERVERS 3
#define BUFFER_SIZE 100

#define IP_ADDRESS(x) (unsigned int)(172 + (17 << 8) + (0 << 16) + (x << 24))


SEC("xdp_drop")
int xdp_drop_prog(struct xdp_md *ctx){

    unsigned int lb_addr = 0;
    unsigned int server_addr[NUMSERVERS] = {1,2,3};
    int threads_free[NUMSERVERS] = {5, 5, 5};
    int buf[BUFFER_SIZE];
    int st=0, end=0;

    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    __u16 h_proto;

    struct ethhdr *eth = data;
    struct iphdr *ip = data + sizeof(*eth);
    struct tcphdr *tcp = data + sizeof(*eth) + sizeof(*ip);

    if(data + sizeof(struct ethhdr) > data_end)
        return XDP_PASS;
    
    if(bpf_ntohs(eth->h_proto) != ETH_P_IP)
        return XDP_PASS;

    if (data + sizeof(struct ethhdr) + sizeof(struct iphdr) > data_end){
        bpf_trace_printk("Not a valid IP packet\n");
        return XDP_PASS;
    }

    if(ip->protocol != IPPROTO_TCP){
        bpf_trace_printk("Not a TCP packet\n");
        return XDP_PASS;
    }

    if (data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) > data_end){
        bpf_trace_printk("Not a valid TCP packet\n");
        return XDP_PASS;
    }

    

    if(tcp->dest != htons(20000))
        bpf_trace_printk("Not meant for the server");
        return XDP_PASS;

    // int flag = 0;
    for(int i=0;i<NUMSERVERS;i++){
        if(ip->saddr == htonl(IP_ADDRESS(server_addr[i]))){
            // flag = 1;
            bpf_trace_printk("Received from server %d\n", i);
            if(st!=end){
                ip->daddr = htonl(IP_ADDRESS(server_addr[i]));
                eth->h_dest[5] = server_addr[i];
                ip->saddr = htonl(IP_ADDRESS(lb_addr));
                eth->h_source[5] = lb_addr;
                int *data_ptr = data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr);
                *data_ptr = buf[st];
                st = (st+1)%BUFFER_SIZE;

                ip->check = iph_csum(ip);

                bpf_trace_printk("Sending packet from queue to server %d\n", i);
                return XDP_TX;
            }
            else{
                bpf_trace_printk("Queue empty, incrementing threads_free\n");
                threads_free[i]++;
                return XDP_DROP;
            }
        }
    }
    // if(!flag){
    for(int i=0;i<NUMSERVERS;i++){
        if(threads_free[i]>0){
            ip->daddr = htonl(IP_ADDRESS(server_addr[i]));
            eth->h_dest[5] = server_addr[i];
            ip->saddr = htonl(IP_ADDRESS(lb_addr));
            eth->h_source[5] = lb_addr;

            ip->check = iph_csum(ip);
            threads_free[i]--;

            bpf_trace_printk("Sending to server %d\n", i);
            return XDP_TX;
        }
    }
    if(end == (st-1+BUFFER_SIZE)%BUFFER_SIZE){
        bpf_trace_printk("Buffer full\n");
        return XDP_PASS;
    }
    buf[end] = *(int *)(data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr));
    end = (end+1)%BUFFER_SIZE;
    // }
    
    return XDP_DROP;
}

char _license[] SEC("license") = "GPL";

