// Cheapo - a QuakeWorld Proxy
// Copyright (C) 1998 Sami Tammilehto
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


//----------------------------- compress.c ---------------------------

#include "proxy.h"

#ifdef COMPRESS

static uchar *rw_bitp;
static int    rw_bit;

void writebit(int bit)
{ // bit must be 0 / 1
    rw_bit|=bit;
    if(rw_bit<0)
    {
        *rw_bitp++=rw_bit;
        rw_bit=0x01000000;
    }
    else
    {
        rw_bit=rw_bit+rw_bit;
    }
}

void writebits(int value,int bits)
{
    while(bits--)
    {
        writebit((value>>bits)&1);
    }
}

void writebit_begin(uchar *d)
{
    rw_bitp=d;
    rw_bit=0x01000000;
}

uchar *writebit_end(void)
{ // returns first unused byte in stream
    while(rw_bit!=0x01000000) writebit(0);
    return(rw_bitp);
}

int readbit(void)
{
    if(rw_bit<0)
    {
        rw_bit=0x00800000|(*rw_bitp++);
    }
    rw_bit=rw_bit+rw_bit;
    return( (rw_bit>>8) & 1);
}

int readbits(int bits)
{
    int value=0;
    while(bits--)
    {
        value<<=1;
        value|=readbit();
    }
    return(value);
}

void readbit_begin(uchar *d)
{
    rw_bitp=d;
    rw_bit=-1;
}

uchar *readbit_pos(void)
{ // returns first unused byte in stream
    return(rw_bitp);
}

int compress_raw(uchar *d,uchar *s,int inbytes)
{
    memcpy(d,s,inbytes);
    return(inbytes);
}

int uncompress_raw(uchar *d,uchar *s,int inbytes)
{
    memcpy(d,s,inbytes);
    return(inbytes);
}

/**/

int compress_header(User *u,uchar *d,uchar *s,int server)
{
    int seqthis,seqrecv,bitthis,bitrecv;

    readheader(s,&seqthis,&seqrecv,&bitthis,&bitrecv);
    *d++=(seqthis&0xff);
    *d++=((seqthis>>8)&0x3f)|0x40|(bitthis?0x80:0);
    *d++=(seqrecv&0xff);
    *d++=((seqrecv>>8)&0x3f)|0x40|(bitrecv?0x80:0);

    return(4);
}

int uncompress_header(User *u,uchar *d,uchar *s,int server)
{
    int seqthis,seqrecv,bitthis,bitrecv;
    int flip=0;

    seqthis = s[0];
    seqthis|=(s[1]&0x3f)<<8;
    bitthis = s[1]&0x80;

    seqrecv = s[2];
    seqrecv|=(s[3]&0x3f)<<8;
    bitrecv = s[3]&0x80;

    if(server)
    {
        seqthis|=u->server_lastthis&0xffffc000;
        if((seqthis&0x3fff)<(u->server_lastthis&0x3fff))
        {
            flip=1;
            seqthis+=0x4000;
        }
        seqrecv|=u->server_lastrecv&0xffffc000;
        if((seqrecv&0x3fff)<(u->server_lastrecv&0x3fff))
        {
            flip=1;
            seqrecv+=0x4000;
        }
    }
    else
    {
        seqthis|=u->client_lastthis&0xffffc000;
        if((seqthis&0x3fff)<(u->client_lastthis&0x3fff))
        {
            flip=1;
            seqthis+=0x4000;
        }
        seqrecv|=u->client_lastrecv&0xffffc000;
        if((seqrecv&0x3fff)<(u->client_lastrecv&0x3fff))
        {
            flip=1;
            seqrecv+=0x4000;
        }
    }

    writeheader(d,&seqthis,&seqrecv,&bitthis,&bitrecv);

    return(8);
}

int compress_client(User *u,uchar *d,uchar *s,int inbytes)
{
    inbytes=compress_raw(d,s+2,inbytes-2);
    return(inbytes);
}

int uncompress_client(User *u,uchar *d,uchar *s,int inbytes)
{
    inbytes=uncompress_raw(d+2,s,inbytes);
    d[0]=u->response_b&255;
    d[1]=u->response_b>>8;
    inbytes+=2;
    return(inbytes);
}

int compress_server(User *u,uchar *d,uchar *s,int inbytes)
{
    compress_count(s,inbytes);
    inbytes=compress_raw(d,s,inbytes);
    return(inbytes);
}

int uncompress_server(User *u,uchar *d,uchar *s,int inbytes)
{
    inbytes=uncompress_raw(d,s,inbytes);
    return(inbytes);
}

void uncompress(User *u,uchar *inbuf,int *inbytes,int server)
{
    int bytes=0,n;
    int showcompressstatus=0;

    if(intat(inbuf)==-1) return;

    if(!(inbuf[3]&0x40))
    {
        if(!server && u->compress_server)
        { // received an uncompressed client packet, stop packing
            showcompressstatus=1;
            u->compress_server=0;
        }
    }
    else
    {
        // uncompress
        if(server)
        { // server packet
            bytes+=uncompress_header(u,tmpbuf,inbuf,1);
            n=uncompress_server(u,tmpbuf+bytes,inbuf+4,*inbytes-4);
            if(n<0)
            {
                n=0;
                u->compress_server=0;
                u->compress_client=0;
                q_print(u,"Server packet uncompress error. Compression disabled.\n");
            }
            bytes+=n;
        }
        else
        { // client packet
            // received a compressed client packet, start packing
            if(!u->compress_server && !u->compress_test)
            {
                showcompressstatus=1;
                u->compress_server=1;
            }
            bytes+=uncompress_header(u,tmpbuf,inbuf,0);
            n=uncompress_client(u,tmpbuf+bytes,inbuf+4,*inbytes-4);
            if(n<0)
            {
                n=0;
                u->compress_server=0;
                u->compress_client=0;
                q_print(u,"Client packet uncompress error. Compression disabled.\n");
            }
            bytes+=n;
        }

        if(loglevel>=LOG_PACKETS)
        {
            if(server) logx(LOG_PACKETS,"uncompress-server");
            else       logx(LOG_PACKETS,"uncompress-client");
            logx(LOG_PACKETS," %i -> %i bytes\n",
                *inbytes,bytes);
        }

        memcpy(inbuf,tmpbuf,bytes);
        *inbytes=bytes;
        u->dumpsentpacket=1;
    }

    if(showcompressstatus)
    {
        if(u->compress_server) sprintf(tmpbuf,"Server compress: %s\n",u->proxyhostname);
        else                   sprintf(tmpbuf,"Server compress: disabled\n",u->proxyhostname);
        q_print(u,tmpbuf);
    }
}

void compress(User *u,uchar *inbuf,int *inbytes,int server)
{ // uses tmpbuf
    int bytes=0;

    if(intat(inbuf)==-1) return;

    if(server)
    { // server packet
        if(!u->compress_server) return;
        bytes+=compress_header(u,tmpbuf,inbuf,1);
        bytes+=compress_server(u,tmpbuf+bytes,inbuf+8,*inbytes-8);
    }
    else
    { // client packet
        if(!u->compress_client) return;
        bytes+=compress_header(u,tmpbuf,inbuf,0);
        bytes+=compress_client(u,tmpbuf+bytes,inbuf+8,*inbytes-8);
    }

    if(loglevel>=LOG_PACKETS)
    {
        if(server) logx(LOG_PACKETS,"compress-server");
        else       logx(LOG_PACKETS,"compress-client");
        logx(LOG_PACKETS," %i -> %i bytes\n",
            *inbytes,bytes);
    }

    memcpy(inbuf,tmpbuf,bytes);
    *inbytes=bytes;
    u->dumpsentpacket=1;
}

#else

void compress(User *u,uchar *inbuf,int *inbytes,int server)
{
}

void uncompress(User *u,uchar *inbuf,int *inbytes,int server)
{
}

#endif

