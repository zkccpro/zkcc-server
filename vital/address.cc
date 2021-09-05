#include"address.h"

Address::Address(in_addr_t addr,in_port_t port,int protocol){
    setAddr(addr,port,protocol);
}

void Address::setSrcAddr(in_addr_t addr,in_port_t port,int protocol){
    setAddr(addr,port,protocol);
}

void Address::setDstAddr(in_addr_t addr,in_port_t port,int protocol){
    setAddr(addr,port,protocol);
}

void Address::setAddr(in_addr_t addr,in_port_t port,int protocol){
    bzero(&addr_,sizeof(addr_));
	addr_.sin_addr.s_addr=addr;
	addr_.sin_port=htons(port);
    addr_.sin_family=protocol;
}