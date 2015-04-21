

#ifndef PX_HEART
#define PX_HEART        0x0202
#endif

#ifndef PX_NEWCON
#define PX_NEWCON       0x0404
#endif

#ifndef PX_DAT
#define PX_DAT          0x0303
#endif

struct px
{
        uint32_t px_ses;        // session number for extra credit
        uint32_t px_newaddr;    // the new connection initiation
        uint16_t px_tos;         // type of service
        uint16_t px_len;        // total length of packet
        uint32_t px_ack;        // acknowlegdement numbers
        uint32_t px_seq;        // sequence number 
};

