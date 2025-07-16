#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <linux/tcp.h>

SEC("xdp_drop")
int xdp_drop_prog(struct xdp_md *ctx){
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
    // Attach a ebpf map to the program
    // int id = 1;
    // int *port = bpf_map_lookup_elem(&port_map, &id);


    

    if(tcp->dest != htons(20000)){
        bpf_trace_printk("Not meant for the server");
        return XDP_PASS;
    }

    if(tcp->seq % 2 == 0){
        bpf_trace_printk("DROPPING PACKET - EVEN SEQ NO. %d\n", tcp->seq);
        return XDP_DROP;
    }
    
    bpf_trace_printk("ACCEPTING PACKET - ODD SEQ NO. %d\n", tcp->seq);
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";

