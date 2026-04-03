#include "../h/const.h"
#include "../h/type.h"
#include "../h/com.h"
#include "const.h"
#include "type.h"
#include "proc.h"
#include "dp8390.h"
#include "assert.h"
#include "dp8390info.h"
#include "eplinfo.h"

#define  PIC_enable()	port_out(INT_CTL, ENABLE)

extern char get_byte();

struct eplinfo eplinfo = {0x280};

struct dp8390info dp8390info = {0x290, 6, 27, 0xC4000, 0xC4000};

inbyte(port)
    vir_bytes port;
{
    int value;

    port_in(port, &value);
    return value;
}

outbyte(port, value)
    vir_bytes port;
    int value;
{
    port_out(port, value);
}

getheader(paddr, pkthead)
    phys_bytes paddr;
    struct rcvdheader *pkthead;
{
    vir_bytes  seg;

    assert((paddr&0xFFF0000F)==0L);
    seg = paddr>>4;
    pkthead->rp_status = get_byte(seg,0);
    pkthead->rp_next = get_byte(seg,1);
    pkthead->rp_rbcl = get_byte(seg,2);
    pkthead->rp_rbch = get_byte(seg,3);
}


short
getbint(paddr)
    phys_bytes paddr;
{
    vir_bytes seg,offset;

    seg = paddr >> 4;
    offset = paddr & 0xF;
    return (((short)get_byte(seg, offset)&0xFF)<<8) + (short)(get_byte(seg, offset+1)&0xFF);
}

/*
getbyte(paddr)
phys_bytes paddr;
{
    vir_bytes	seg;
    vir_bytes	offset;

    seg = paddr >> 4;
    offset = paddr & 0xf;
    return get_byte(seg, offset);
}


vp_copy(procnr, seg, vir_addr, phys_addr, count)
    int		procnr;
    int 	seg;
    vir_bytes 	vir_addr;
    phys_bytes 	phys_addr;
    vir_bytes 	count;
{
    phys_bytes 	u_phys;
    register struct proc *rp;
    phys_bytes umap();

    rp = proc_addr(procnr);
    u_phys = umap(rp, seg, vir_addr, count);
    assert(u_phys!=0L);
    phys_copy(u_phys, phys_addr, (phys_bytes)count);
}

pv_copy(phys_addr, procnr, seg, vir_addr, count)
    phys_bytes 	phys_addr;
    int		procnr;
    int 	seg;
    vir_bytes 	vir_addr;
    vir_bytes 	count;
{
    phys_bytes 	vir_phys;
    register struct proc *rp;
    phys_bytes umap();

    rp = proc_addr(procnr);
    vir_phys = umap(rp, seg, vir_addr, count);
    assert(vir_phys!=0L);
    phys_copy(phys_addr, vir_phys, (phys_bytes)count);
}
*/
