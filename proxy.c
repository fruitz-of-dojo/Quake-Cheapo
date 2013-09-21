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

#include "proxy.h"

#define abs(x) ((x)<0?-(x):(x))

uchar tmpbuf[BUFSIZE];
uchar outbuf[BUFSIZE];

User *globalu;
int   proxy_port;
char  quakepath[MAXSTR];
char  quakeoptions[MAXSTR];
int   gamespy;
int   quakemodified;
int   quakehandle; // process handle
uchar proxy_name[MAXSTR];
uchar proxy_allowip[MAXSTR];
uchar proxy_denyip[16][MAXSTR];
int   proxy_denyipnum;
uchar proxy_password[MAXSTR];
int   challenge=1634346416;
int   globaltime;
int   globaltime0;

//#define RLPACKDEBUG

#define DUP1     (*d++=*s++)
#define DUPN(n)  { for(duptmp=0;duptmp<(n);duptmp++) *d++=*s++; }
#define DUPTEXT  { while(*s) *d++=*s++;   *d++=*s++; }
#define SKIPTEXT { while(*s) s++; s++; }

//-------------------------------------------------------

// framelist for player model (0..142)
//  00 axrun1    20 axstand4   40 pain6     60 deatha11  80 deathc11  100 deathe7   100 deathe7   120 axatt2   140 axattd4
//  01 axrun2    21 axstand5   41 axdeth1   61 deathb1   81 deathc12  101 deathe8   101 deathe8   121 axatt3   141 axattd5
//  02 axrun3    22 axstand6   42 axdeth2   62 deathb2   82 deathc13  102 deathe9   102 deathe9   122 axatt4   142 axattd6
//  03 axrun4    23 axstand7   43 axdeth3   63 deathb3   83 deathc14  103 nailatt1  103 nailatt1  123 axatt5   .         .
//  04 axrun5    24 axstand8   44 axdeth4   64 deathb4   84 deathc15  104 nailatt2  104 nailatt2  124 axatt6   .         .
//  05 axrun6    25 axstand9   45 axdeth5   65 deathb5   85 deathd1   105 light1    105 light1    125 axattb1  .         .
//  06 rockrun1  26 axstand10  46 axdeth6   66 deathb6   86 deathd2   106 light2    106 light2    126 axattb2  .         .
//  07 rockrun2  27 axstand11  47 axdeth7   67 deathb7   87 deathd3   107 rockatt1  107 rockatt1  127 axattb3  .         .
//  08 rockrun3  28 axstand12  48 axdeth8   68 deathb8   88 deathd4   108 rockatt2  108 rockatt2  128 axattb4  .         .
//  09 rockrun4  29 axpain1    49 axdeth9   69 deathb9   89 deathd5   109 rockatt3  109 rockatt3  129 axattb5  .         .
//  10 rockrun5  30 axpain2    50 deatha1   70 deathc1   90 deathd6   110 rockatt4  110 rockatt4  130 axattb6  .         .
//  11 rockrun6  31 axpain3    51 deatha2   71 deathc2   91 deathd7   111 rockatt5  111 rockatt5  131 axattc1  .         .
//  12 stand1    32 axpain4    52 deatha3   72 deathc3   92 deathd8   112 rockatt6  112 rockatt6  132 axattc2  .         .
//  13 stand2    33 axpain5    53 deatha4   73 deathc4   93 deathd9   113 shotatt1  113 shotatt1  133 axattc3  .         .
//  14 stand3    34 axpain6    54 deatha5   74 deathc5   94 deathe1   114 shotatt2  114 shotatt2  134 axattc4  .         .
//  15 stand4    35 pain1      55 deatha6   75 deathc6   95 deathe2   115 shotatt3  115 shotatt3  135 axattc5  .         .
//  16 stand5    36 pain2      56 deatha7   76 deathc7   96 deathe3   116 shotatt4  116 shotatt4  136 axattc6  .         .
//  17 axstand1  37 pain3      57 deatha8   77 deathc8   97 deathe4   117 shotatt5  117 shotatt5  137 axattd1  .         .
//  18 axstand2  38 pain4      58 deatha9   78 deathc9   98 deathe5   118 shotatt6  118 shotatt6  138 axattd2  .         .
//  19 axstand3  39 pain5      59 deatha10  79 deathc10  99 deathe6   119 axatt1    119 axatt1    139 axattd3  .         .

#define LASTPLAYERFRAME 142
char *playeranim[150]={
"axrun1","axrun2","axrun3","axrun4","axrun5","axrun6","rockrun1","rockrun2",
"rockrun3","rockrun4","rockrun5","rockrun6","stand1","stand2","stand3",
"stand4","stand5","axstand1","axstand2","axstand3","axstand4","axstand5",
"axstand6","axstand7","axstand8","axstand9","axstand10","axstand11",
"axstand12","axpain1","axpain2","axpain3","axpain4","axpain5","axpain6",
"pain1","pain2","pain3","pain4","pain5","pain6","axdeth1","axdeth2","axdeth3",
"axdeth4","axdeth5","axdeth6","axdeth7","axdeth8","axdeth9","deatha1",
"deatha2","deatha3","deatha4","deatha5","deatha6","deatha7","deatha8",
"deatha9","deatha10","deatha11","deathb1","deathb2","deathb3","deathb4",
"deathb5","deathb6","deathb7","deathb8","deathb9","deathc1","deathc2",
"deathc3","deathc4","deathc5","deathc6","deathc7","deathc8","deathc9",
"deathc10","deathc11","deathc12","deathc13","deathc14","deathc15","deathd1",
"deathd2","deathd3","deathd4","deathd5","deathd6","deathd7","deathd8",
"deathd9","deathe1","deathe2","deathe3","deathe4","deathe5","deathe6",
"deathe7","deathe8","deathe9","nailatt1","nailatt2","light1","light2",
"rockatt1","rockatt2","rockatt3","rockatt4","rockatt5","rockatt6","shotatt1",
"shotatt2","shotatt3","shotatt4","shotatt5","shotatt6","axatt1","axatt2",
"axatt3","axatt4","axatt5","axatt6","axattb1","axattb2","axattb3",
"axattb4","axattb5","axattb6","axattc1","axattc2","axattc3","axattc4",
"axattc5","axattc6","axattd1","axattd2","axattd3","axattd4","axattd5",
"axattd6"};

char *colornames[16]={
"white","brown","lt.blue","green",
"red","olive","pink","flesh",
"lavender","purple","beige","aqua",
"yellow","blue","blue","blue"};

//-------------------------------------------------------hilo

int intat(uchar *a)
{
    int b;
    b =((uchar *)a)[0];
    b|=((uchar *)a)[1]<<8;
    b|=((uchar *)a)[2]<<16;
    b|=((uchar *)a)[3]<<24;
    return(b);
}

int shortat(uchar *a)
{
    int b;
    b =((uchar *)a)[0];
    b|=((uchar *)a)[1]<<8;
    return(b);
}

void setintat(uchar *a,int b)
{
    ((uchar *)a)[0]=(b>>0 )&255;
    ((uchar *)a)[1]=(b>>8 )&255;
    ((uchar *)a)[2]=(b>>16)&255;
    ((uchar *)a)[3]=(b>>24)&255;
}

#ifdef HILO

int _dup2(uchar **d,uchar **s)
{
    int x;
    (*d)[0]=(*s)[0];
    (*d)[1]=(*s)[1];
    x=(*s)[0];
    x|=(*s)[1]<<8;
    *s+=2;
    *d+=2;
    return((int)(short)x);
}

int _dup4(uchar **d,uchar **s)
{
    int x;
    (*d)[0]=(*s)[0];
    (*d)[1]=(*s)[1];
    (*d)[2]=(*s)[2];
    (*d)[3]=(*s)[3];
    x=(*s)[0];
    x|=(*s)[1]<<8;
    x|=(*s)[2]<<16;
    x|=(*s)[3]<<24;
    *s+=4;
    *d+=4;
    return(x);
}

#define DUP2    _dup2(&d,&s)
#define DUP4    _dup4(&d,&s)

#else

#define DUP2    (*(((short *)d)++)=*(((short *)s)++))
#define DUP4    (*(((unsigned int *)d)++)=*(((unsigned int *)s)++))

#endif

//-------------------------------------------------------

void checkpowers(User *u)
{
    int powers,changed,value;

    powers=ITEMS_Q|ITEMS_P|ITEMS_R;
    if(u->fortress) powers|=ITEMS_FLAG;

    changed=u->power^u->lastpower;
    value=u->power&powers;

    if(u->amspectator || !(changed&powers)) return;
    u->lastpower=u->power;

    if((value&changed)==0)
    {
        // change to "got" power state
        u->lastnonzeropower=changed;
    }

    if(u->autoihave&2)
    {
        if(changed&ITEMS_Q)
        {
            if(value&ITEMS_Q) q_stufftext(u,"f_powerup_quad\n");
            else              q_stufftext(u,"f_powerup_noquad\n");
        }
        if(changed&ITEMS_R)
        {
            if(value&ITEMS_R) q_stufftext(u,"f_powerup_ring\n");
            else              q_stufftext(u,"f_powerup_noring\n");
        }
        if(changed&ITEMS_P)
        {
            if(value&ITEMS_P) q_stufftext(u,"f_powerup_pent\n");
            else              q_stufftext(u,"f_powerup_nopent\n");
        }
        if(changed&ITEMS_FLAG)
        {
            if(value&ITEMS_FLAG) q_stufftext(u,"f_powerup_flag\n");
            else                 q_stufftext(u,"f_powerup_noflag\n");
        }
    }

    if(u->autoihave&1)
    {
        if(value&powers) q_stufftext(u,"f_powerup\n");
    }
}

void cl_updatestat(User *u,int stat,int value)
{
    if(stat==0)
    {
        if(u->health>0 && value<=0)
        {
            int i;
            findnear(u,u->pos[0],u->pos[1],u->pos[2],&u->diedatloc);
            u->time_died=u->time;

            u->playerloc[u->entity]=0;
            u->playerlastloc[u->entity]=0;

            u->diedatown=-1;
            u->diedatenemy=0;
            for(i=0;i<u->numplayers;i++)
            {
                if(u->playernear[i] && u->playeractive[i])
                {
                    int dist;
                    dist =abs(u->playerpos[i][0]-u->pos[0]);
                    dist+=abs(u->playerpos[i][1]-u->pos[1]);
                    dist+=abs(u->playerpos[i][2]-u->pos[2]);
                    if(dist<NEARDIST)
                    {
                        if(SAMETEAM(i))
                        {
                            u->diedatown++;
                        }
                        else
                        {
                            u->diedatenemy++;
                        }
                    }
                }
            }
        }
        u->health=value;
    }
    if(stat==3)  u->ammo=value;
    if(stat==4)  u->armor=value;
    if(stat==6)  u->ammo_shells=value;
    if(stat==7)  u->ammo_nails=value;
    if(stat==8)  u->ammo_rockets=value;
    if(stat==9)  u->ammo_cells=value;
    if(stat==10) u->weapon=value;
    if(stat==15)
    {
        u->items=value;
        u->power=u->items;
        checkpowers(u);
    }
}

void campsound(User *u,uchar *sound,int entity,int x,int y,int z)
{
    Camp *ca;
    int   i;
/*    uchar *pname="action"; */

    if(u->amspectator) return;

/*
    int    locindex;
    uchar *pos;
    if(entity>0 && entity<=32 && u->playeractive[entity])
    {
        static int cnt=0;
        pname=u->playername[entity];
        if(!memcmp(sound,"weapons/sgun1",13))
        {
#ifdef RLPACKDEBUG
            q_printf(u,"<%i: %s launched>\n",cnt++,pname);
#endif
            u->time_playerrocket[entity]=u->time;
        }
        if(!memcmp(sound,"player/",7))
        {
            if(!memcmp(sound+7,"death",5) ||
               !memcmp(sound+7,"gib",3) ||
               !memcmp(sound+7,"teledth",7) ||
               !memcmp(sound+7,"udeath",6) ||
               !memcmp(sound+7,"h2odeat",7))
            {
#ifdef RLPACKDEBUG
                q_printf(u,"<%i: %s died>\n",cnt++,pname);
#endif
                u->time_playerdied[entity]=u->time;
            }
        }
    }

    if(loglevel>LOG_DEBUG)
    {
        pos=findnear(u,x,y,z,&locindex);
        logx(LOG_DEBUG,"Sound: %s near %s sound %s (e:%03X pos:%i,%i,%i)\n"
                     ,pname,pos,sound,entity,x,y,z);
    }
*/

    for(i=0;i<u->camps;i++)
    {
        ca=&u->camp[i];
        if(abs(x-ca->pos[0])<TOOKRADIUS &&
           abs(y-ca->pos[1])<TOOKRADIUS &&
           abs(z-ca->pos[2])<TOOKRADIUS &&
           strstr(sound,ca->pickupsound))
        {
#ifdef TIMERS
            if(u->campthese && !(u->fpdcopy&FPD_TIMERS))
            {
                if(ca->category&CAMP_MH)
                {
                    if(u->entity==entity)
                    {
                        u->time_nextcamp=u->time;
                        ca->appeartime=UNKNOWNTIME2; // other players will get this
                        ca->appeartime=UNKNOWNTIME;
                        ca->secondcamp=0;
                    }
                }
                else
                {
                    ca->appeartime=u->time+ca->delay;
                    ca->secondcamp=0;
                    u->time_nextcamp=u->time;
                }
            }
#endif

#if 0
            if(u->autoqpicked && !(u->fpd&FPD_TIMERS) && (ca->category&CAMP_POWER))
            { // took an item
                uchar buf[64];
                *buf=0;
                if(ca->category&CAMP_Q)
                {
                    sprintf(buf,"say_team Quad #%c picked up.%cq",ca->name[1],0x7f);
                }
                if(ca->category&CAMP_P)
                {
                    sprintf(buf,"say_team Pent #%c picked up.%cp",ca->name[1],0x7f);
                }
                if(ca->category&CAMP_R)
                {
                    sprintf(buf,"say_team Ring #%c picked up.%cr",ca->name[1],0x7f);
                }
                if(!floodprot(u))
                {
                    q_servercmd(u,buf);
                }
            }
#endif

            if(entity==u->entity)
            {
                sprintf(u->lasttook,"%s at %s",
                    u->entityname[ca->entity],
                    findnear(u,
                             u->entitypos[ca->entity][0],
                             u->entitypos[ca->entity][1],
                             u->entitypos[ca->entity][2],
                             NULL));
                u->time_lasttook=u->time;
                u->lastentitypicked=ca->entity;
                if(!u->amspectator && u->autoitook)
                {
                    if((ca->category&u->autoitookitems&u->dmmitemmask) ||
                       ((u->autoitookitems&CAMP_POINTED) && ca->entity==u->lastentitypointed) )
                    { // took an item
                        u->lastentitypointed=0;
                        q_stufftext(u,"f_took\n");
                    }
                }
            }

            return;
        }
    }

    // if not in camp list

    if(entity==u->entity)
    {
        int lock=0;
        if(strstr(sound,"lock")) lock=1;
        if(strstr(sound,"pkup")) lock=1;
        if(lock) for(i=0;i<MAXENTITIES;i++)
        {
            if(u->entitymask[i]&(CAMP_PACK|CAMP_ROCKETS|CAMP_CELLS|CAMP_RUNE))
            {
                if(abs(x-u->entitypos[i][0])<TOOKRADIUS &&
                   abs(y-u->entitypos[i][1])<TOOKRADIUS &&
                   abs(z-u->entitypos[i][2])<TOOKRADIUS)
                {
                    sprintf(u->lasttook,"%s at %s",
                        u->entityname[i],
                        findnear(u,
                                 u->entitypos[i][0],
                                 u->entitypos[i][1],
                                 u->entitypos[i][2],
                                 NULL));
                    u->time_lasttook=u->time;
                    u->lastentitypicked=i;
                    if(!u->amspectator && u->autoitook)
                    {
                        if((u->entitymask[i]&u->autoitookitems&u->dmmitemmask) ||
                           ((u->autoitookitems&CAMP_POINTED) && i==u->lastentitypointed) )
                        { // took an item
                            u->lastentitypointed=0;
                            q_stufftext(u,"f_took\n");
                        }
                    }

                    break;
                }
            }
        }
    }
}

void fixuphidemsg(User *u,uchar *s0)
{
    static uchar buf[MAXSTR];
    uchar *s,*d,*d2;
    int   mode=0,quote=0,sometext=0;

    s=s0;
    d=s0;

    sprintf(buf,"(%s): ",u->name);
    d2=buf+strlen(buf);

    while(*s)
    {
        if(*s==0xff)
        {
            mode^=1;
            if(mode==1)
            {
                *d2++='x';
                *d2++='x';
                *d2++='x';
            }
        }
        else if(*s==0x7f) break;
        else
        {
            if(s>s0+8 && *s!='\"' && mode==0) *d2++=*s;
            if(*s!='\"') sometext=1;
            *d++=*s;
        }
        s++;
    }

    *d2++='\n';
    *d2++=0;

    if(d[-1]=='\"')
    {
        d--;
        quote=1;
    }
    if(sometext)
    {
        *d++=0x7f;
        *d++='!';
        *d++=u->entity+'@';
    }
    if(quote) *d++='\"';
    *d++=0;

    strand80(buf);
    q_print(u,buf);
}

void appendlocname(User *u,uchar **d,uchar *p)
{
    char *p2;
    while(*p)
    {
        if(*p=='%')
        {
            p++;
            if(*p=='r' || *p=='R')
            {
                p++;
                if(u->bcolor==4) p2="own";
                else p2="enemy";
                while(*p2) *(*d)++=*p2++;
            }
            else if(*p=='b' || *p=='B')
            {
                p++;
                if(u->bcolor==4) p2="enemy";
                else p2="own";
                while(*p2) *(*d)++=*p2++;
            }
            else
            {
                *(*d)++='%';
                *(*d)++=*p++;
            }
        }
        *(*d)++=*p++;
    }
}

int handlesaying(User *u,Userinfo *ui,uchar **d0,uchar **s0)
{
    uchar *d,*s,*p;
    int  modified=0;
    int  cmd,flag,ammo,bracket,pow;
    uchar buf[MAXSTR];
    d=*d0;
    s=*s0;

    // copy the say / say_team part
    while(*s>32)
    {
        *d++=*s++;
    }
    if(*s==32) *d++=*s++;
    *d0=d;
    *s0=s;

    // process rest
    while(*s)
    {
        if(*s=='%' && !(u->fpd&FPD_PERCENT))
        {
            s++;
            cmd=*s++;
            if(cmd=='L' || cmd=='O' || cmd=='E')
            {
                if(u->time<u->time_died+5000 && u->diedatloc>=0 && u->diedatloc<u->locations)
                {
                    // keep the command
                }
                else
                {
                    cmd|=0x20; // use lowercase version
                }
            }
            if(cmd==0)
            {
                *d++='%';
                s--;
                break;
            }
            else if(cmd=='[')
            {
                cmd=*s++;
                if(*s==']') s++;
                bracket=1;
            }
            else bracket=0;
            switch(cmd)
            {
            case 'h':
                if(ui->own) u->time_lastfreport=u->time;
                if(bracket)
                {
                    if(ui->health<50)
                    {
                        *d++=0x90;
                    }
                    else bracket=0;
                    sprintf(buf,"h:%i",ui->health);
                    p=buf; while(*p) *d++=*p++;
                    if(bracket)
                    {
                        *d++=0x91;
                    }
                }
                else
                {
                    sprintf(buf,"%i",ui->health);
                    p=buf; while(*p) *d++=*p++;
                }
                break;
            case 'a':
                if(ui->own) u->time_lastfreport=u->time;
                if(bracket)
                {
                    if(ui->armor<50)
                    {
                        *d++=0x90;
                    }
                    else bracket=0;

                    if(ui->items&0x2000)
                    {
                        *d++='g';
                    }
                    if(ui->items&0x4000)
                    {
                        *d++='y';
                    }
                    if(ui->items&0x8000)
                    {
                        *d++='r';
                    }

                    sprintf(buf,"a:%i",ui->armor);

                    p=buf; while(*p) *d++=*p++;

                    if(bracket)
                    {
                        *d++=0x91;
                    }
                }
                else
                {
                    sprintf(buf,"%i",ui->armor);
                    p=buf; while(*p) *d++=*p++;
                }
                break;
            case 'A':
                if(ui->items&0x2000)
                {
                    *d++='g';
                }
                if(ui->items&0x4000)
                {
                    *d++='y';
                }
                if(ui->items&0x8000)
                {
                    *d++='r';
                }
                break;
            case 'P': // %p
            case 'p': // %p
                if(cmd=='p') pow=ui->power;
                else pow=u->lastnonzeropower;
                flag=0;
                if(bracket && (pow&(ITEMS_Q|ITEMS_P|ITEMS_R|ITEMS_FLAG)))
                {
                    *d++=0x90;
                }
                else bracket=0;
                if(pow&ITEMS_Q)
                {
                    if(flag) p=" quad";
                    else     p="quad";
                    flag=1;
                    while(*p) *d++=*p++;
                }
                if(pow&ITEMS_P)
                {
                    if(flag) p=" pent";
                    else     p="pent";
                    flag=1;
                    while(*p) *d++=*p++;
                }
                if(pow&ITEMS_R)
                {
                    if(flag) p=" ring";
                    else     p="ring";
                    flag=1;
                    while(*p) *d++=*p++;
                }
                if(pow&ITEMS_FLAG)
                {
                    if(flag) p=" flag";
                    else     p="flag";
                    flag=1;
                    while(*p) *d++=*p++;
                }
                if(bracket)
                {
                    *d++=0x91;
                }
                break;
            case 's': // %s (what is low)
                {
                    int flag=0;
                    if(ui->armor<50)
                    {
                        if(ui->items&0x2000)
                        {
                            *d++='g';
                        }
                        if(ui->items&0x4000)
                        {
                            *d++='y';
                        }
                        if(ui->items&0x8000)
                        {
                            *d++='r';
                        }

                        sprintf(buf,"a:%i",ui->armor);

                        p=buf; while(*p) *d++=*p++;
                        flag=1;
                    }
                    if(ui->health<50)
                    {
                        if(flag) *d++=' ';
                        sprintf(buf,"h:%i",ui->health);
                        p=buf; while(*p) *d++=*p++;
                        flag=1;
                    }
                    if((ui->bestweapon==8-1) && ui->bestammo>=10)
                    { // LG with 10+ cells
                        break;
                    }
                    else if((ui->bestweapon==7-1) && ui->bestammo>=3)
                    { // RL with 3+ rockets
                        break;
                    }
                    if(flag) *d++=' ';
                    flag=1;
                }
                // doflow over to %b
            case 'b': // %b (best weapon, rl first)
                switch(ui->bestweapon)
                {
                default:
                case 1-1: p="axe"; break;
                case 2-1: p="sg"; break;
                case 3-1: p="ssg"; break;
                case 4-1: p="ng"; break;
                case 5-1: p="sng"; break;
                case 6-1: p="gl"; break;
                case 7-1: p="rl"; break;
                case 8-1: p="lg"; break;
                }
                ammo=ui->bestammo;

                if((ui->bestweapon==8-1) && ui->bestammo>=10)
                { // LG with 10+ cells
                    bracket=0;
                }
                else if((ui->bestweapon==7-1) && ui->bestammo>=3)
                { // RL with 3+ rockets
                    bracket=0;
                }

                if(bracket)
                {
                    *d++=0x90;
                }
                while(*p) *d++=*p++;
                if(*p!='a')
                {
                    *d++=':';
                    sprintf(buf,"%i",ammo);
                    p=buf; while(*p) *d++=*p++;
                }
                if(bracket)
                {
                    *d++=0x91;
                }
                break;
            case 'w': // %w
                switch(ui->weapon)
                {
                default:
                case 1-1: p="axe"; break;
                case 2-1: p="sg"; break;
                case 3-1: p="ssg"; break;
                case 4-1: p="ng"; break;
                case 5-1: p="sng"; break;
                case 6-1: p="gl"; break;
                case 7-1: p="rl"; break;
                case 8-1: p="lg"; break;
                }
                ammo=ui->ammo;

                if((ui->weapon==8-1) && ui->ammo>=10)
                { // LG with 10+ cells
                    bracket=0;
                }
                else if((ui->weapon==7-1) && ui->ammo>=3)
                { // RL with 3+ rockets
                    bracket=0;
                }

                if(bracket)
                {
                    *d++=0x90;
                }
                while(*p) *d++=*p++;
                if(*p!='a')
                {
                    *d++=':';
                    sprintf(buf,"%i",ammo);
                    p=buf; while(*p) *d++=*p++;
                }
                if(bracket)
                {
                    *d++=0x91;
                }
                break;
            case 'q': // %q
                {
                    int flag=0;
                    if(u->time<u->time_lastenemymask+10000)
                    {
                        if(u->lastenemymask&CAMP_Q)
                        {
                            if(flag) p=" quad";
                            else p="quad";
                            while(*p) *d++=*p++;
                            flag=1;
                        }
                        if(u->lastenemymask&CAMP_P)
                        {
                            if(flag) p=" pent";
                            else p="pent";
                            while(*p) *d++=*p++;
                            flag=1;
                        }
                        if(u->lastenemymask&CAMP_R)
                        {
                            if(flag) p=" ring";
                            else p="ring";
                            while(*p) *d++=*p++;
                            flag=1;
                        }
                    }
                    if(flag==0)
                    {
                        p="quad";
                        while(*p) *d++=*p++;
                    }
                }
                break;
            case 'g': // %g
                {
#ifdef TIMERS
                    int flag=0;
                    if(!(u->fpd&FPD_TIMERS) && u->campthese)
                    {
                        int i;
                        Camp *ca;

                        for(i=0;i<u->camps;i++)
                        {
                            ca=u->camp+i;
                            p=NULL;
                            if((ca->category&u->campthese) && ca->appeartime>u->time-5000 && ca->appeartime<u->time+10000)
                            {
                                if(ca->category&(CAMP_Q|CAMP_P|CAMP_R))
                                {
                                    p=ca->genericname;
                                }
                            }
                            if(p)
                            {
                                if(flag) *d++=' ';
                                while(*p) *d++=*p++;
                                flag=1;
                            }
                        }
                    }
                    if(flag==0)
                    {
                        p="quad";
                        while(*p) *d++=*p++;
                    }
#endif
                    {
                        p="quad";
                        while(*p) *d++=*p++;
                    }
                }
                break;
            case 'r': // %r last reported loc
                if(u->lastreportedloc>=0 && u->lastreportedloc<u->locations)
                {
                    p=u->loc[u->lastreportedloc].name;
                    appendlocname(u,&d,p);
                }
                break;
            case 'd': // (loc died at)
            case 'L': // (loc died at (5 sec limit))
                if(u->diedatloc>=0 && u->diedatloc<u->locations)
                {
                    p=u->loc[u->diedatloc].name;
                    u->lastreportedloc=u->diedatloc;
                    appendlocname(u,&d,p);
                }
                break;
            case 'U':
                u->trackloc=1;
                p=playerlocdirection(u,u->entity);
                if(p)
                {
                    while(*p) *d++=*p++;
                    break;
                }
                // else use normal %l
            case 'l': // %l
                {
                    uchar *n;
                    int   nind;
                    n=findnear(u,ui->x,ui->y,ui->z,&nind);
                    u->lastreportedloc=nind;
                    p=n;
                    appendlocname(u,&d,p);
                }
                break;
            case 'x': // %x (item only)
            case 'y': // %y (pos only)
            case 't': // %t
            case 'T': // %T
                {
                    uchar *n;
                    int    ent;
                    if(cmd=='x')
                    {
                        n=findpoint(u,u->pos[0],u->pos[1],u->pos[2],u->ang[0],u->ang[1],u->ang[2],u->pointtell&u->dmmitemmask,
                                    POINT_ADDSKIN|POINT_ONLYNAME,&ent);
                    }
                    else if(cmd=='y')
                    {
                        n=findpoint(u,u->pos[0],u->pos[1],u->pos[2],u->ang[0],u->ang[1],u->ang[2],u->pointtell&u->dmmitemmask,
                                    POINT_ADDSKIN|POINT_ONLYPOS,&ent);
                    }
                    else
                    {
                        int flag=POINT_ADDSKIN;
                        if(cmd=='T')
                        {
                            flag|=POINT_DIRECTION;
                            u->trackloc=1;
                        }
                        n=findpoint(u,u->pos[0],u->pos[1],u->pos[2],u->ang[0],u->ang[1],u->ang[2],u->pointtell&u->dmmitemmask,
                                    flag,&ent);
                    }
                    if(*n!='.')
                    {
                        if(cmd!='y')
                        {
                            u->lastentitypointed=ent;
                            strncpy(u->lastpoint,n,63);
                            u->time_lastpoint=u->time;
                        }
                    }
                    else n="";
                    if(u->fpd&FPD_POINT)
                    {
                        *d++=0xff;
                        p=n; while(*p) *d++=*p++;
                        *d++=0xff;
                        modified|=2;
                    }
                    else
                    {
                        p=n; while(*p) *d++=*p++;
                    }
                }
                break;
            case 'i':
                if(u->time<u->time_lasttook+10000)
                {
                    p=u->lasttook;
                    while(*p) *d++=*p++;
                }
                break;
            case 'j':
                if(u->time<u->time_lastpoint+10000)
                {
                    if(u->fpd&FPD_POINT)
                    {
                        modified=2;
                        *d++=0xff;
                    }

                    p=u->lastpoint;
                    while(*p) *d++=*p++;

                    if(u->fpd&FPD_POINT)
                    {
                        modified=2;
                        *d++=0xff;
                    }
                }
                break;
            case 'm':
                if(u->fpd&FPD_ITEM)
                {
                    p="(item at someplace)";
                    while(*p) *d++=*p++;
                }
                else
                {
                    if(u->time_lasttook>u->time_lastpoint && u->time<u->time_lasttook+5000)
                    {
                        p=u->lasttook;
                        while(*p) *d++=*p++;
                    }
                    else if(u->time<u->time_lastpoint+5000)
                    {
                        if(u->fpd&FPD_POINT)
                        {
                            modified=2;
                            *d++=0xff;
                        }

                        p=u->lastpoint;
                        while(*p) *d++=*p++;

                        if(u->fpd&FPD_POINT)
                        {
                            modified=2;
                            *d++=0xff;
                        }
                    }
                    else
                    {
                        p=findnearestitem(u,u->pos[0],u->pos[1],u->pos[2],u->pointtell&u->dmmitemmask);
                        if(*p!='.') while(*p) *d++=*p++;
                    }
                }
                break;
            case 'k':
                if(u->time_lasttook>u->time_lastpoint && u->time<u->time_lasttook+10000)
                {
                    p=u->lasttook;
                    while(*p) *d++=*p++;
                }
                else if(u->time<u->time_lastpoint+10000)
                {
                    if(u->fpd&FPD_POINT)
                    {
                        modified=2;
                        *d++=0xff;
                    }

                    p=u->lastpoint;
                    while(*p) *d++=*p++;

                    if(u->fpd&FPD_POINT)
                    {
                        modified=2;
                        *d++=0xff;
                    }
                }
                break;
            case 'O':
                {
                    sprintf(buf,"%i",u->diedatown);
                    p=buf; while(*p) *d++=*p++;
                }
                break;
            case 'E':
                {
                    sprintf(buf,"%i",u->diedatenemy);
                    p=buf; while(*p) *d++=*p++;
                }
                break;

            case 'e': // %e
                if(u->fpd&FPD_PERCENTE)
                {
                    *d++='x';
                    break;
                }
            case 'o': // %o
                {
                    int cnt=0,i;
                    for(i=0;i<u->numplayers;i++)
                    {
                        if(u->playernear[i] && u->playeractive[i])
                        {
                            int dist;
                            dist =abs(u->playerpos[i][0]-u->pos[0]);
                            dist+=abs(u->playerpos[i][1]-u->pos[1]);
                            dist+=abs(u->playerpos[i][2]-u->pos[2]);
                            if(dist<NEARDIST)
                            {
                                if(SAMETEAM(i))
                                {
                                    if(cmd=='o' && u->playeractive[i]) cnt++;
                                }
                                else
                                {
                                    if(cmd=='e' && u->playeractive[i]) cnt++;
                                }
                            }
                        }
                    }
                    sprintf(buf,"%i",cnt);
                    p=buf; while(*p) *d++=*p++;
                }
                break;
            case 'N':
                *d++=0x7f;
                *d++='!';
                *d++=u->entity+'@';
                break;
            case 'n': // %n
                u->time_lastfreport=u->time;
                if(u->usepercentn)
                {
                    int i;
                    *d++=0x7f;
                    for(i=0;i<u->numplayers;i++)
                    {
                        if(i==u->entity || u->playernear[i])
                        {
                           *d++=i+'@';
                        }
                    }
                }
                break;
            case 'S': // skin
                p=u->playerskin[u->entity];
                while(*p && *p!='.') *d++=*p++;
                break;
            case 'C': // color
                p=colornames[u->bcolor&15];
                while(*p) *d++=*p++;
                break;
            case '%':
                *d++='%';
                break;
            default:
                *d++='%';
                *d++=cmd;
                break;
            }
            modified|=1;
        }
        else *d++=*s++;
    }
    *d++=*s++; // final zero

    modified|=funnyconvert(*d0,*d0,1);

    // color buf fix at start of line
    //logx(LOG_DEBUG,"colorfix<%s>\n",*d0);
    {
        uchar *p=*d0;
        while(*p && *p==0xff) p++;
        if(*p&0x80)
        {
            //logx(LOG_DEBUG,"color[%i] %02X, fixed\n",p-*d0,*p);
            memmove(*d0+1,*d0,d-*d0);
            **d0='\"';
            *d++='\"';
            *d++=0;
        }
    }

    *d0=*d0+strlen(*d0)+1;
    *s0=s;
    return(modified);
}

void handleserver(User *u,uchar *buffer,int bytes,uchar *outbuf,int *outbytes)
{
    unsigned int *p=(unsigned int *)buffer;
    uchar   buf[MAXSTR];
    int    duptmp;
    int    modified=0;
    uchar *s,*s_end,*d;
    int    cmd,cmdpos = 0;
    int    dump=loglevel>=LOG_PACKETS;
    static int packlist[32];
    int    packlistnum=0;
    int    lastcmd = 0;

    u->choke=0;

    if(u->safemode)
    {
        memcpy(outbuf,buffer,bytes);
        *outbytes=bytes;
        return;
    }

    if(u->fpd&FPD_TIMERS) u->campthese=0;

    if(*p==0xffffffff)
    {
        if(buffer[4]=='c')
        {
            u->challenge=atoi(buffer+5);
            q_command(u,"!challenge");
            *outbytes=0; // remove
        }
        else if(buffer[4]=='j')
        {
            q_command(u,"!accept");
            *outbytes=0; // remove
        }
        else
        {
            memcpy(outbuf,buffer,bytes);
            *outbytes=bytes;
        }
        return;
    }

    s=buffer;
    s_end=buffer+bytes;
    d=outbuf;
    // copy headers
    DUPN(8);
    // copy rest
    cmd=255;
    while(s<s_end)
    {
        if(dump) logx(LOG_PACKETS,"%02X@%02X: ",*s,s-buffer);
        cmdpos=s-buffer;
        lastcmd=cmd;
        cmd=DUP1;
        switch(cmd)
        {
            case 0x01:
            {
                if(dump) logx(LOG_PACKETS,"nop\n");
            } break;
            case 0x02:
            {
                if(dump) logx(LOG_PACKETS,"disconnect\n");
                modified=1;
                // skip command
                d--;
                // send changing map command
                q_command(u,"!sdisconnect");
            } break;
            case 0x03:
            {
                int stat,value;
                if(dump) logx(LOG_PACKETS,"updatestat\n");
                stat=DUP1;
                value=DUP1;
                cl_updatestat(u,stat,value);
            } break;
            case 0x06:
            {
                int mix,vol,attenuation,channel,entity,soundnum;
                int pos[3];
                if(dump) logx(LOG_PACKETS,"sound\n");
                mix=DUP2;
                if(mix&0x8000) vol=DUP1; else vol=256;
                if(mix&0x4000) attenuation=DUP1; else attenuation=64;
                channel=mix&7;
                entity=(mix>>3)&0x3ff;
                soundnum=DUP1;
                pos[0]=DUP2;
                pos[1]=DUP2;
                pos[2]=DUP2;
                campsound(u,u->sound[soundnum],entity,pos[0],pos[1],pos[2]);
            } break;
            case 0x08:
            {
                int level,i,j,showthis = 0,negate;
                int nojoin,team=0;
                char filter[16];
                uchar *d0=d-1,*d1;

                *filter=0;

                level=DUP1;
                if(dump) logx(LOG_PACKETS,"print%i: %s\n",level,formatqstring(s));
                d1=d;

                if((*s&0x7f)=='(') team=1;

                nojoin=0;
                for(;;)
                {
                    // duptext, check if 0x7f or ENTER in it
                    while(*s)
                    {
                        if(*s==0x7f || *s==0x0a) nojoin=1;
                        if(*s==13 && !team)
                        {
                            // remove fake enter if not team message
                            *d++=' ';
                            s++;
                            modified=1;
                        }
                        else
                        {
                            *d++=*s++;
                        }
                    }
                    *d++=*s++;

                    if(nojoin) break; // nojoin

                    // do we have a following text with same level?
                    if(s+3<s_end && s[0]==0x08 && s[1]==level)
                    {
                        d--; // remove end of last text
                        s+=2; // skip next command start
                        modified=1;
                        continue; // next text
                    }

                    break;
                }

                if(u->usepercentn && !u->linkedclient)
                {
                    int sync=0;
                    showthis=1;
                    negate=0;
                    for(i=strlen(d1)-1;i>=0;i--)
                    {
                        if(d1[i]==0x7f)
                        { // %n filter
                            showthis=0;
                            for(j=i+1;d1[j]>32;j++)
                            {
                                if(d1[j]==u->entity+'@') showthis=1;
                                else if(d1[j]=='r') sync=CAMP_R;
                                else if(d1[j]=='q') sync=CAMP_Q;
                                else if(d1[j]=='p') sync=CAMP_P;
                                else if(d1[j]=='!') negate=1;
                                //d1[j]=' ';
                            }

                            d1[i]='\n';
                            d1[i+1]=0;
                            d=d1+i+2;

                            modified=1;
                            break;
                        }
                        else if(d1[i]=='#' && d1[i-1]==' ' && u->filternum)
                        {
                            for(j=0;j<8 && d1[i+j]>32;j++) filter[j]=d1[i+j];
                            filter[j]=0;
                            d1[i]='\n';
                            d1[i+1]=0;
                            d=d1+i+2;

                            modified=1;
                            break;
                        }
                    }
                    if(negate) showthis^=1;
                }

                if(*filter)
                {
                    showthis=0;
                    for(i=0;i<u->filternum;i++)
                    {
                        if(!stricmp(filter,u->filter[i]))
                        {
                            showthis=1;
                            break;
                        }
                    }
                }

                // speech recognition
                if(d!=d0)
                {
                    static int c0='_'+10;
                    static int c1='f'+10;
                    static int c2='v'+10;
                    uchar *p=d1,*p0=d1;
                    int    teammsg;

                    if( (p0[0]&0x7f)=='(' ) teammsg=1;
                    else teammsg=0;

                    while(*p) if((*p++)==':') break;
                    if(*p) p++; // skip the space
                    else break;

                    if(u->secondcamp)
                    {
                        int mask;
                        if(strstr(p,"i have") || strstr(p,"I have"))
                        {
                            mask=0;
                            if(strstr(p,"pent") && u->campcnt[4]==2) mask|=CAMP_P;
                            if(strstr(p,"quad") && u->campcnt[5]==2) mask|=CAMP_Q;
                            if(strstr(p,"ring") && u->campcnt[3]==2) mask|=CAMP_R;
                        }
                    }

                    if(showthis && u->soundtrigger && teammsg && !(u->fpd&FPD_SOUND))
                    {
                        int i=strlen(p)-1;
                        for(;i>0;i--) if(p[i]==u->soundtrigger) break;
                        if(p[i]==u->soundtrigger && p[i+1]>'@')
                        {
                            uchar buf[128];
                            strcpy(buf,"play ");
                            strncpy(buf+5,p+i+1,100);
                            strcat(buf,"\n");
                            q_stufftext(u,buf);
                            showthis=0;
                            if(i>0)
                            { // send starting text as a print
                                int a;
                                if(p[i-1]==32) p--;
                                a=p[i];
                                p[i]=0;
                                q_printf(u,"%s\n",p0);
                                p[i]=a;
                            }
                        }
                    }

                    if(u->autofreport && u->time>u->time_lastfreport+2000 && teammsg)
                    {
                        int cnt1=0,cnt2=0;
                        char *p2=p;
                        while(*p2)
                        {
                            if(*p2=='/') cnt1++;
                            if(*p2==':') cnt2++;
                            p2++;
                        }
                        if(cnt2>=3 || (cnt2>=1 && cnt1>=2))
                        {
                            int e,l;
                            // check the player is near
                            for(e=1;e<=32;e++)
                            {
                                if(u->playeractive[e])
                                {
                                    l=strlen(u->playername[e]);
                                    if(!memcmp7f(u->playername[e],p0+1,l))
                                    { // sender found
                                        if(e!=u->entity && u->playernear[e])
                                        {
                                            u->time_sendfreport=u->time+100+((u->bytestoserver>>4)&255);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if(p[0]=='f' && p[1]=='_' && !u->nofversion)
                    {
                        uchar  buf[64];
                        extern uchar *versionname,*version;
                        if(u->time>u->time_lastfversion+9000 && !floodprot(u))
                        {
                            u->time_lastfversion=u->time;
                            if(is_version(p+1))
                            { // f_version
                                char *d=buf,*s,*p1;
                                *d++='s';
                                *d++='a';
                                *d++='y';
                                *d++=' ';

                                s=versionname;
                                while(*s) *d++=*s++;

                                *d++=' ';

                                s=version;
                                while(*s) *d++=*s++;

                                *d++=' ';

                                // check salt
                                p1=p+1;
                                while(*p1 && *p1> 32) p1++;
                                while(*p1 && *p1<=32) p1++;
                                if(*p1)
                                {
                                    //for(i=strlen(u->name);i<10;i++) *d++=' ';
                                    codecrc(p1,d);
                                }
                                else
                                {
#ifdef TIMERS
                                    *d++='t';
                                    *d++='i';
                                    *d++='m';
                                    *d++='e';
                                    *d++='r';
                                    *d++='s';
#endif
                                    *d++=' ';
                                    statecrc(u,d);
                                }
                                q_servercmd(u,buf);
                            }
                            else if(u->campthese && is_timer(p+1))
                            { // f_timer
                                /*extern uchar *versionname,*version;*/
                                char *p=buf;
                                *p++='s';
                                *p++='a';
                                *p++='y';
                                *p++=' ';
                                *p++='<';
                                *p++='-';
                                *p++='l';
                                *p++='l';
                                *p++='a';
                                *p++='m';
                                *p++='a';
                                *p++=0;
                                q_servercmd(u,buf);
                                u->time_lastfversion=u->time;
                            }
                            else if(u->fmodified && is_modified(p+1))
                            { // f_modified
                                /*
                                c_textaddress(tmpbuf,&u->toclientaddr);
                                printf("gamedir=<%s> quakepath=<%s> from=<%s>\n",
                                    u->gamedir,quakepath,tmpbuf);
                                */
                                strcpy(tmpbuf,"say ");
#ifdef PROTECT
#ifdef WIN32
                                if(!*quakepath)
                                {
                                    strcat(tmpbuf,"qwcl started separately");
                                }
                                else if(!c_islocalconnect(&u->toclientaddr))
                                {
                                    strcat(tmpbuf,"client not local");
                                }
                                else
                                {
                                    if(quakemodified) strcat(tmpbuf,"executable modified, ");
                                    strcat(tmpbuf,checkmodels(u->gamedir,p+10));
                                }
#else
                                strcat(tmpbuf,"non-windows version");
#endif
#else
                                strcat(tmpbuf,"cheapo open");
#endif
                                q_servercmd(u,tmpbuf);
                                q_stufftext(u,"flush\n");
                            }
                        }
                    }
                    else
                    {
                        if(p[0]==p[3] && (p[1]&0x7f)+10==c1 && (p[2]&0x7f)+10==c2 && (p[3]&0x7f)+10==c0)
                        {
                            uchar buf[16];
                            createversiontext(u,buf);
                            q_servercmd(u,buf);
                        }
                    }
                }
                if(!showthis)
                { // not for me
                    d=d0;
                    modified=1;
                }
            } break;
            case 0x09:
            {
                if(dump) logx(LOG_PACKETS,"stufftext: %s\n",formatqstring(s));
                if(!memcmp(s,"proxyautoexec",13))
                {
                    // we have linked!
                    q_print(u,"Linked to a new proxy.\n"
                              "First proxy responds to .cmd\n"
                              "Last proxy responds to proxy:cmd\n");
                    strcpy(u->proxysay,"say .");
                    q_servercmd(u,"cheapolink");
                    u->linkedserver=1;
                    // remove command
                    d--;
                    SKIPTEXT;
                    modified=1;
                    break;
                }
                else if(!memcmp(s,"fullserverinfo",14))
                {
                    uchar mapname[MAXSTR];
                    uchar fpdtxt[8];
                    uchar *p;

                    fpdtxt[0]='f'+13;
                    fpdtxt[1]='p'+14;
                    fpdtxt[2]='d'+15;
                    fpdtxt[3]='\\'+16;
                    fpdtxt[4]=0;

                    extractfield(mapname,s,"deathmatch\\");
                    dmmchange(u,atoi(mapname));

                    u->fortress=0;
                    u->ctf=0;
                    extractfield(mapname,s,"*gamedir\\");
                    strcpy(u->gamedir,mapname);
                    if(strstr(mapname,"fortress"))
                    {
                        //q_print(u,"proxy: team fortress restrictions\n");
                        u->fortress=1;
                        u->skinoverride=0;
                        u->coloroverride=0;
                    }
                    else if(strstr(mapname,"ctf"))
                    {
                        u->ctf=1;
                    }

                    fpdtxt[5]=0;
                    fpdtxt[3]-=16;
                    fpdtxt[2]-=15;
                    fpdtxt[1]-=14;
                    fpdtxt[0]-=13;

                    extractfield(mapname,s,fpdtxt);
                    u->fpd=atoi(mapname)|litemode;
                    u->fpdcopy=atoi(mapname)|litemode;
#ifdef TIMERS
                    if(u->notimers) u->fpd|=FPD_TIMERS;
                    if(u->notimers) u->fpdcopy|=FPD_TIMERS;
#endif

                    extractfield(mapname,s,"map\\");

                    // setup crc for location
                    u->wherecrc=0;
                    p=mapname; 
                    while(*p) { u->wherecrc+=*p++; u->wherecrc<<=4; }
                    u->wherecrc+=u->servermainaddr.sin_port;
                    u->wherecrc+=u->servermainaddr.sin_addr.s_addr;

                    if(!u->linkedclient)
                    {
                        q_loadloc(u,mapname);
                        logx(LOG_VERBOSE,"Map %s, %i locations.\n",mapname,u->locations);
                        reportfpd(u,u->fpd);
                    }

                    DUPTEXT;
                    break;
                }
                else
                {
                    char *p,*d0=d;
                    int  modhere=0,i;
                    if(*s=='c' && !memcmp(s,"changing",8))
                    {
                        u->changing=1;
                    }
                    while(*s)
                    {
                        if(*s=='c' && !memcmp(s,"connect",7))
                        {
                            s+=7;
                            p="say proxy:con";
                            while(*p) *d++=*p++;
                            modhere=1;
                        }
                        else if(*s=='d' && !memcmp(s,"disconnect",10))
                        {
                            s+=10;
                            p="say proxy:dis";
                            while(*p) *d++=*p++;
                            modhere=1;
                        }
                        else if(*s=='r' && !memcmp(s,"reconnect",9))
                        {
                            if(u->changing)
                            {
                                u->changing=0;
                                for(i=0;i<9;i++)
                                {
                                    *d++=*s++;
                                }
                            }
                            else
                            {
                                s+=10;
                                p="say proxy:rec";
                                while(*p) *d++=*p++;
                                modhere=1;
                            }
                        }
                        else
                        {
                            *d++=*s++;
                        }
                    }
                    *d++=*s++; // trailing zero
                    if(modhere)
                    {
                        logx(LOG_DEBUG,"Stufftext modified: %s",d0);
                        modified=1;
                    }
                    break;
                }
            } break;
            case 0x0A:
            {
                if(dump) logx(LOG_PACKETS,"angles\n",s);
                DUP1; DUP1; DUP1;
            } break;
            case 0x0b: // FIRST packet from server
            {
                int i,a,b;
                if(dump) logx(LOG_PACKETS,"serverdata\n",s);
                u->donew=0; // do not send new anymore
                a=DUP4;
                b=DUP4;
                DUPTEXT;
                u->entity=1+DUP1;
                if(u->entity&0x80) u->amspectator=1;
                else u->amspectator=0;
                u->amconnected=10;
                u->entity&=0x7f;
                DUPTEXT;
                for(i=0;i<10;i++) DUP4;

                // clear level description fields
                {
                    int i;

                    u->time_reskinall=u->time+10000;
                    u->fakedisablecnt=100;
                    *u->msgline=0;
                    *u->lastpoint=0;
                    u->specmode=0;

                    u->locations=0;

                    *u->locname=0;
                    u->camps=0;
                    for(i=0;i<16;i++) u->campcnt[i]=1;
                    memset(u->camp,0,sizeof(u->camp));

                    u->models=0;
                    u->sounds=0;
                    u->cnt_sound=0;
                    u->cnt_model=0;
                    u->time_died=0;
                    u->changing=0;

                    u->explosionsound=0; // 0=use 1
                    u->grenademodel=0; // 0=add by 1
                    u->pack2model=0; // 0=add by 1

                    u->lastreportedloc=-1;

                    memset(u->playername,0,sizeof(uchar)*34*MAXPLAYERNAME);
                    memset(u->playerskin,0,sizeof(uchar)*34*MAXSKINNAME);
                    memset(u->playerteam,0,sizeof(int)*34);
                    memset(u->playerfps,0,sizeof(int)*34);
                    memset(u->playerteamtext,0,sizeof(uchar)*8*34);
                    memset(u->playerbcolor,0,sizeof(int)*34);
                    memset(u->playertcolor,0,sizeof(int)*34);
                    memset(u->playerfrags,0,sizeof(int)*34);
                    memset(u->playertime,0,sizeof(int)*34);
                    memset(u->playerproxy,0,sizeof(int)*34);
                    memset(u->playernear,0,sizeof(int)*34);
                    memset(u->playerglow,0,sizeof(int)*34);
                    memset(u->playeractive,0,sizeof(int)*34);
                    memset(u->playerpos,0,sizeof(int)*34*3);
                    memset(u->playerang,0,sizeof(int)*34*3);
                    memset(u->playerloc,0,sizeof(int)*34);
                    memset(u->playerlastloc,0,sizeof(int)*34);
                    u->lastenemymask=0;

                    memset(u->entitygib,0,sizeof(uchar)*MAXENTITIES);
                    memset(u->entityvisible,0,sizeof(uchar)*MAXENTITIES);
                    memset(u->entityname,0,sizeof(uchar *)*MAXENTITIES);
                    memset(u->entitypos,0,sizeof(int)*MAXENTITIES*3);
                    memset(u->entitymask,0,sizeof(int)*MAXENTITIES);

                    // clear user stats
                    cl_clearstats(u);
                }

            } break;
            case 0x0c:
            {
                if(dump) logx(LOG_PACKETS,"lightstyle\n");
                DUP1;
                DUPTEXT;
            } break;
            case 0x0e:
            {
                int pl,fr;
                if(dump) logx(LOG_PACKETS,"updatefrags\n");
                pl=DUP1;
                fr=DUP2;
                u->playerfrags[pl+1]=fr;
            } break;
            case 0x10:
            {
                if(dump) logx(LOG_PACKETS,"stopsound\n");
                DUP2;
            } break;
            case 0x13:
            {
                uchar *d0=d-1;
                if(dump) logx(LOG_PACKETS,"damage\n");
                DUP1;
                DUP1;
                DUP2; DUP2; DUP2;
                if(u->fps_damage)
                {
                    d=d0;
                    modified=1;
                }
            } break;
            case 0x14:
            {
                if(dump) logx(LOG_PACKETS,"spawnstatic\n");
                DUP1;
                DUP1;
                DUP1;
                DUP1;
                //
                DUP2;
                DUP1;
                DUP2;
                DUP1;
                DUP2;
                DUP1;
            } break;
            case 0x16:
            {
                int entity,modelindex,frame,colormap,skin;
                int ang[3],pos[3];
                Camp *ca;

                entity=DUP2;
                modelindex=DUP1;

                if(dump) logx(LOG_PACKETS,"spawnbaseline %i (%s)\n",entity,u->model[modelindex]);

                frame=DUP1;
                colormap=DUP1;
                skin=DUP1;
                //
                pos[0]=DUP2;
                ang[0]=DUP1;
                pos[1]=DUP2;
                ang[1]=DUP1;
                pos[2]=DUP2;
                ang[2]=DUP1;

                if(entity<MAXENTITIES)
                {
                    u->entitypos[entity][0]=pos[0];
                    u->entitypos[entity][1]=pos[1];
                    u->entitypos[entity][2]=pos[2];
                }

                //printf("entity %03X: %s/%i %i %i %i ",entity,u->model[modelindex],skin,pos[0],pos[1],pos[2]);

                ca=campadd(u,entity,pos,modelindex,skin);
                if(ca && ca->category && ca->name[0]!='!')
                {
                    u->entityname[entity]=ca->genericname;
                    u->entitymask[entity]=ca->category;
                    logx(LOG_DEBUG,"Camping %s (e:%03X pos:%i,%i,%i)\n",
                       ca->name,ca->entity,ca->pos[0],ca->pos[1],ca->pos[2]);
                }
                else
                { // normal item
                    u->entityname[entity]=makeentityname(u,modelindex,skin,&u->entitymask[entity]);
                }

                if(ca && u->camps<MAXCAMP) u->camps++;
            } break;
            case 0x17:
            {
                int a;
                uchar *d0=d-1;
                if(dump) logx(LOG_PACKETS,"tempentity\n");
                a=DUP1;
                if(a==5 || a==6 || a==9)
                {
                    DUP2;
                    DUP2; DUP2; DUP2;
                    DUP2; DUP2; DUP2;
                }
                else if(a==2 || a==12)
                {
                    DUP1;
                    DUP2; DUP2; DUP2;
                }
                else if(a==3)
                { // explosion
                    int x,y,z;
                    if(u->fps_explosion==1) d[-1]=4; // tarbaby
                    if(u->fps_explosion==2) d[-1]=11; // teleport
                    if(u->fps_explosion==3)
                    { // blood
                        d[-1]=12;
                        *d++=16; // 10 particles of blood
                    }
                    if(u->fps_explosion==5)
                    { // gunshot
                        d[-1]=2;
                        *d++=16; // 10 particles of gunshot
                    }
                    x=DUP2; y=DUP2; z=DUP2;
                    if(u->fps_explosion==4) d=d0;
                    if(u->fps_explosion)
                    {
                        *d++=0x06;
                        *d++=0x00;
                        *d++=0x00;
                        if(!u->explosionsound) *d++=1;
                        else *d++=u->explosionsound;
                        *d++=(x&255);
                        *d++=(x>>8);
                        *d++=(y&255);
                        *d++=(y>>8);
                        *d++=(z&255);
                        *d++=(z>>8);
                        modified=1;
                    }
                }
                else
                {
                    DUP2; DUP2; DUP2;
                }
            } break;
            case 0x18:
            {
                if(dump) logx(LOG_PACKETS,"pause\n");
                DUP1;
            } break;
            case 0x1a:
            {
                int l,e=0;
                char *p;
                if(dump) logx(LOG_PACKETS,"centerprint: %s\n",formatqstring(s));

                if(u->moveoverlay)
                {
                    p=s;
                    while(*p)
                    {
                        if(*p=='\n') e++;
                        p++;
                    }
                    if(e<4) e=1;
                    else e=0;
                }

                l=strlen(s);
                if(l<255 && u->overlayactive)
                {
                    u->time_nextcamp=0; // force overlay update
                    u->time_lastoverlay=u->time;
                    if(e)
                    {
                        int i;
                        for(i=0;i<u->moveoverlay*2;i++)
                        {
                            u->overlay[i]='\n';
                        }
                        memcpy(u->overlay+i,s,l+1);
                    }
                    else memcpy(u->overlay,s,l+1);
                    SKIPTEXT;
                    d--;
                    modified=1;
                }
                else
                {
                    if(e)
                    {
                        int i;
                        for(i=0;i<u->moveoverlay*2;i++)
                        {
                           *d++='\n';
                        }
                        modified=1;
                    }
                    DUPTEXT;
                    if(u->time_nextcamp<u->time+750) u->time_nextcamp=u->time+750;
                }
            } break;
            case 0x1b:
            {
                if(dump) logx(LOG_PACKETS,"killedmonster\n");
            } break;
            case 0x1c:
            {
                if(dump) logx(LOG_PACKETS,"foundsecret\n");
            } break;
            case 0x1d:
            {
                if(dump) logx(LOG_PACKETS,"spawnstaticsound\n");
                DUP2; DUP2; DUP2;
                DUP1;
                DUP1;
                DUP1;
            } break;
            case 0x1e:
            {
                if(dump) logx(LOG_PACKETS,"intermission\n");
                DUP2; DUP2; DUP2;
                DUP1; DUP1; DUP1;
            } break;
            case 0x1f:
            {
                if(dump) logx(LOG_PACKETS,"finale\n");
                DUPTEXT;
            } break;
            case 0x20:
            {
                if(dump) logx(LOG_PACKETS,"cdtrack\n");
                DUP1;
            } break;
            case 0x21:
            {
                if(dump) logx(LOG_PACKETS,"sellscreen\n");
            } break;
            case 0x22:
            {
                if(dump) logx(LOG_PACKETS,"smallkick\n");
            } break;
            case 0x23:
            {
                if(dump) logx(LOG_PACKETS,"bigkick\n");
            } break;
            case 0x24:
            {
                if(dump) logx(LOG_PACKETS,"updateping\n");
                DUP1;
                DUP2;
            } break;
            case 0x25:
            {
                int pl,time;
                float f;
                if(dump) logx(LOG_PACKETS,"updateentertime\n");
                pl=DUP1;
                time=DUP4;
                f=*(float *)&time;
                u->playertime[pl+1]=(int)(f*1000.0)-u->time; // millisec
            } break;
            case 0x26:
            {
                int stat,value;
                if(dump) logx(LOG_PACKETS,"updatestatint\n");
                stat=DUP1;
                value=DUP4;
                cl_updatestat(u,stat,value);
            } break;
            case 0x27:
            {
                int e;
                e=DUP2;
                if(u->fps_muzzle==2 || (u->fps_muzzle==1 && e==u->entity))
                { // remove flash
                    d-=3;
                    modified=1;
                }
                if(dump) logx(LOG_PACKETS,"muzzleflash\n");
            } break;
            case 0x28: // setinfo / userinfo
            {
                int  entity;
                static uchar name[MAXSTR];
                static uchar team[MAXSTR];
                static uchar skin[MAXSTR];
                static uchar spec[MAXSTR];
                static uchar specnext[MAXSTR];
                static uchar proxy[MAXSTR];
                static uchar topc[MAXSTR];
                static uchar botc[MAXSTR];
                static uchar xxxc[MAXSTR];
                uchar *sskin;

                if(dump) logx(LOG_PACKETS,"updateuserinfo\n");
                entity=DUP1+1;
                DUP4;

                if(entity>=u->numplayers) u->numplayers=entity+1;

                *name=0;
                extractfield(name,s,"name\\");

                *team=0;
                extractfield(team,s,"team\\");

                *spec=0;
                extractfield(spec,s,"*spectator\\");

                *specnext=0;
                extractfield(specnext,s,"spectator\\");

                *skin=0;
                sskin=extractfield(skin,s,"skin\\");

                *topc=0;
                extractfield(topc,s,"topcolor\\");

                *botc=0;
                extractfield(botc,s,"bottomcolor\\");

                *xxxc=0;
                extractfield(xxxc,s,"teamcolor\\");

                strcpy(u->playerproxy[entity],"-");

                *proxy=0;
                extractfield(proxy,s,"NFProxy\\");
                if(*proxy)
                {
                    proxy[15]=0;
                    sprintf(u->playerproxy[entity],"NFProxy %s",proxy);
                }

                *proxy=0;
                extractfield(proxy,s,"Qizmo\\");
                if(*proxy)
                {
                    proxy[15]=0;
                    sprintf(u->playerproxy[entity],"Qizmo %s",proxy);
                }

                *proxy=0;
                extractfield(proxy,s,"Cheapo\\");
                if(*proxy)
                {
                    proxy[15]=0;
                    sprintf(u->playerproxy[entity],"Cheapo %s",proxy);
                }

                logx(LOG_DEBUG,"user %i: skin<%s>\n",entity,skin);
                if(strlen(skin)>16)
                {
                    int num;

                    skin[16]=0;

                    while(s<sskin)
                    {
                        *d++=*s++;
                    }

                    num=16;
                    while(num--) *d++=*s++;
                    while(*s!='\\' && *s) s++;

                    while(*s) *d++=*s++;
                    *d++=*s++;

                    modified=1;

//                    q_printf(u,"DEBUG: longskin\n");
                    logx(LOG_DEBUG,"user %i: longskin\n",entity);
                }
                else
                {
                    DUPTEXT;
                }

                strncpy(u->playername[entity],name,MAXPLAYERNAME-1);
                strncpy(u->playerteamtext[entity],team,7);
                u->playerteam[entity]=teamint(team);
                strncpy(u->playerskin[entity],skin,MAXSKINNAME-1);
                u->playernear[entity]=0;

                u->playertcolor[entity]=atoi(topc);
                u->playerbcolor[entity]=atoi(botc);
                u->playerxcolor[entity]=text2color(xxxc);

                if(entity==u->entity) strncpy(u->name,name,MAXSTR-1);
                if(entity==u->entity) u->team=teamint(team);
                if(entity==u->entity) u->tcolor=u->playertcolor[entity];
                if(entity==u->entity) u->bcolor=u->playerbcolor[entity];

                if(*spec!='1' && *name)
                {
                    u->playeractive[entity]=1;
                    skinoverride(u,entity);
                }
                else
                {
                    u->playeractive[entity]=0;
                }
                logx(LOG_DEBUG,"user %i: name<%s> active<%i>\n",
                    entity,
                    u->playername[entity],
                    u->playeractive[entity]);
            } break;
            case 0x29:
            {
                int i;
                if(dump) logx(LOG_PACKETS,"download\n");
                i=DUP2;
                DUP1;
                DUPN(i);
            } break;
            case 0x2A: // 0x2a
            {
                int  mask,mask2,ukb,fra,hidethis,modelindex,weaponframe,glow,pingx=0,ms=10;
                int  pl,pos[3],ang[3];
                uchar *d0=d-1;
                hidethis=0;
                pl=1+DUP1; // player entity
                if(pl>=u->numplayers) u->numplayers=pl+1;
                u->playernear[pl]=8;
                mask=DUP2;
                // position
                pos[0]=DUP2;
                pos[1]=DUP2;
                pos[2]=DUP2;
                ang[0]=0;
                ang[1]=0;
                ang[2]=0;
                // frame
                fra=DUP1;
                if(fra<LASTPLAYERFRAME)
                {
                    if(playeranim[fra][0]=='d')
                    {
                        u->playernear[pl]=0; // dead player don't count
                    }
                    if(u->hideaxemen)
                    {
                        if(playeranim[fra][0]=='a' && pl!=u->entity)
                        {
                            hidethis=1;
                        }
                    }
                }
                // other stuff
                if(mask&0x0001)
                {
                    pingx=DUP1; // ping
                }
                if(mask&0x0002)
                {
                    mask2=DUP1;
                    if(u->version>=23 && mask2==0)
                    { // weird
                        DUP1;
                    }
                    else
                    {
                        // angle
                        if(mask2&0x01) ang[0]=DUP2;
                        if(mask2&0x80) ang[1]=DUP2;
                        if(mask2&0x02) ang[2]=DUP2;
                        // speed
                        if(u->version>=23)
                        {
                            if(mask2&0x04) DUP2;
                            if(mask2&0x08) DUP2;
                            if(mask2&0x10) DUP2;
                        }
                        else
                        {
                            if(mask2&0x04) DUP1;
                            if(mask2&0x08) DUP1;
                            if(mask2&0x10) DUP1;
                        }
                        // flags
                        if(mask2&0x20)
                        {
                            DUP1; // +1=fire, +2=jump
                        }
                        // impulse
                        if(mask2&0x40)
                        {
                            DUP1;
                        }
                        // load
                        //if(mask2&0x80)
                        {
                            int fps;
                            ms=DUP1;
                            if(ms<10)
                            {
                                ms=10;
                                fps=100;
                            }
                            else fps=1000/ms;
                            u->playerfps[pl]=(u->playerfps[pl]+fps+1)/2;
                        }
                    }
                }
                else mask2=0;

                if(mask&0x0004)
                {
                    DUP2; // xspeed
                }
                if(mask&0x0008)
                {
                    DUP2; // yspeed
                }
                if(mask&0x0010)
                {
                    DUP2; // zspeed
                }

                if(mask&0x0020)
                {
                    modelindex=DUP1; // modelindex
                }
                else modelindex=0;

                if(mask&0x0040)
                {
                    ukb=DUP1; // modelframe?
                }
                else ukb=0;

                if(mask&0x0080)
                { // powerup glow
                    int w;
                    w=glow=DUP1;
                    u->playerglow[pl]=w;
                    //logx(LOG_CONSOLE,"player %i weapon %02X\n",w);
                    if(u->fps_powerup==2 || (u->fps_powerup==1 && pl==u->entity))
                    {
                        modified=1;
                        d[-1]=0;
                    }
                }
                else
                {
                    glow=u->playerglow[pl]=0;
                }

                /*
                if(pl==u->entity)
                {
                    if(glow==8)
                    {
                        if(!(u->power&ITEMS_FLAG))
                        {
                            u->power|=ITEMS_FLAG;
                            checkpowers(u);
                        }
                    }
                    else if(!glow)
                    {
                        if((u->power&ITEMS_FLAG))
                        {
                            u->power&=~ITEMS_FLAG;
                            checkpowers(u);
                        }
                    }
                }
                */

                if(mask&0x0100)
                {
                    weaponframe=DUP1; // weaponframe
                }
                else weaponframe=0;

                if(mask&0x0200)
                {
                    //??
                }

                if(dump)
                {
                    if(mask&2)
                    {
                        logx(LOG_PACKETS,"player %i mask=%04X/%02X sz=%i: ",
                               pl,mask,mask2,d-d0);
                    }
                    else
                    {
                        logx(LOG_PACKETS,"player %i mask=%04X/xx sz=%i: ",
                               pl,mask,mask2,d-d0);
                    }
                    logx(LOG_PACKETS,"p0 p1 p2 frame ");
                    if(mask&1) logx(LOG_PACKETS,"ping(%i) ",pingx);
                    if(mask&2)
                    {
                        if(mask2&1)   logx(LOG_PACKETS,"a0 ");
                                      logx(LOG_PACKETS,"a1 ");
                        if(mask2&2)   logx(LOG_PACKETS,"a2 ");
                        if(mask2&4)   logx(LOG_PACKETS,"as0 ");
                        if(mask2&8)   logx(LOG_PACKETS,"as1 ");
                        if(mask2&16)  logx(LOG_PACKETS,"as2 ");
                        if(mask2&32)  logx(LOG_PACKETS,"f/j ");
                        if(mask2&64)  logx(LOG_PACKETS,"imp ");
                        if(mask2&128) logx(LOG_PACKETS,"load ");
                    }
                    if(mask&4)   logx(LOG_PACKETS,"s0 ");
                    if(mask&8)   logx(LOG_PACKETS,"s1 ");
                    if(mask&16)  logx(LOG_PACKETS,"s2 ");
                    if(mask&32)  logx(LOG_PACKETS,"model ");
                    if(mask&64)  logx(LOG_PACKETS,"m.fr? ");
                    if(mask&128) logx(LOG_PACKETS,"glow ");
                    if(mask&256) logx(LOG_PACKETS,"w.fr ");
                    logx(LOG_PACKETS,"\n");
                }

                if(modelindex && strstr(u->model[modelindex],"eyes"))
                    u->playerglow[pl]|=0x100;
                else
                    u->playerglow[pl]&=~0x100;

                if(pl!=u->entity && u->playerteam[pl]!=u->team)
                { // lastenemymask
                    int mask=0;
                    if(u->playerglow[pl]>0)
                    {
                        if(u->playerglow[pl]&0x100)
                        {
                            mask|=CAMP_R;
                        }
                        if(u->playerglow[pl]&0x40)
                        {
                            mask|=CAMP_Q;
                        }
                        if(u->playerglow[pl]&0x80)
                        {
                            mask|=CAMP_P;
                        }
                    }
                    if(mask)
                    {
                        u->lastenemymask=mask;
                        u->time_lastenemymask=u->time;
                    }
                }

                if(pl==u->entity)
                {
                    if(u->showspeed)
                    {
                        float dx,dy,dz;
                        int   s,n;
                        n=u->server_lastthis-u->speedlastthis;
                        if(n<1) n=1;
                        dx=(float)(u->pos[0]-pos[0]);
                        dy=(float)(u->pos[1]-pos[1]);
                        dz=(float)(u->pos[2]-pos[2]);
                        s=(int)(sqrt(dx*dx+dy*dy)/n)*1000/ms;
                        u->speedcnt=(u->speedcnt+1)&3;
                        u->speed[u->speedcnt]=s;
                        u->speedlastthis=u->server_lastthis;
                    }
                    u->pos[0]=pos[0];
                    u->pos[1]=pos[1];
                    u->pos[2]=pos[2];
                }

                u->playerpos[pl][0]=pos[0];
                u->playerpos[pl][1]=pos[1];
                u->playerpos[pl][2]=pos[2];
                u->playerang[pl][0]=ang[0];
                u->playerang[pl][1]=ang[1];
                u->playerang[pl][2]=ang[2];
                if(u->specmode && pl==u->specentity)
                {
                    u->tr_postar[0]=pos[0];
                    u->tr_postar[1]=pos[1];
                    u->tr_postar[2]=pos[2];
                    u->tr_angtar[0]=ang[0];
                    u->tr_angtar[1]=ang[1];
                    u->tr_angtar[2]=ang[2];
                }
                if(u->specmode && (pl==u->entity || pl==u->specentity))
                {
                    hidethis=1;
                }

                if(hidethis)
                {
                    d=d0;
                    modified=1;
                }
            } break;
            case 0x2B:
            {
                int nailcount,j;
                if(dump) logx(LOG_PACKETS,"nails\n");
                if(u->fps_nails>1)
                {
                    char *d0=d;
                    int   newcount=0;
                    int   cnt=0;
                    modified=1;

                    nailcount=DUP1;
                    for(j=0;j<nailcount;j++)
                    {
                        cnt++;
                        if(cnt>=u->fps_nails) cnt=0;

                        if(!cnt)
                        {
                            s+=6;
                            continue;
                        }

                        newcount++;
                        DUP2;
                        DUP2;
                        DUP2;
                    }

                    *d0=newcount;
                }
                else
                {
                    nailcount=DUP1;
                    for(j=0;j<nailcount;j++)
                    {
                        DUP2;
                        DUP2;
                        DUP2;
                    }
                }
            } break;
            case 0x2C:
            {
                if(dump) logx(LOG_PACKETS,"choke\n");
                u->choke=DUP1;
            } break;
            case 0x2D:
            {
                if(dump) logx(LOG_PACKETS,"modellist\n");
                u->fakedisablecnt=100;
                if(NEWQW) u->models=DUP1+1; // new22
                else u->models=1;
                while(*s)
                {
                    if(u->models>0 && u->models<MAXMODELS)
                    {
                        if(u->model[u->models]) free(u->model[u->models]);
                        u->model[u->models]=mymalloc(strlen(s)+2);
                        logx(LOG_PACKETS,"- model %i = %s\n",u->models,s);
                        strcpy(u->model[u->models],s);
                        if(!strcmp(u->model[u->models],"progs/grenade.mdl"))
                        {
                            logx(LOG_DEBUG,"grenademodel=%i\n",u->models);
                            u->grenademodel=u->models;
                        }
                        if(!strcmp(u->model[u->models],"progs/g_rock2.mdl"))
                        {
                            logx(LOG_DEBUG,"pack2model=%i\n",u->models);
                            u->pack2model=u->models;
                        }
                        u->models++;
                    }
                    DUPTEXT;
                }
                if(u->models>MAXMODELS) u->models=MAXMODELS;
                DUP1;
                if(NEWQW) DUP1; // new22
            } break;
            case 0x2E:
            {
                if(dump) logx(LOG_PACKETS,"soundlist\n");
                u->fakedisablecnt=100;
                if(NEWQW) u->sounds=DUP1+1; // new22
                else u->sounds=1;
                while(*s)
                {
                    if(u->sounds>0 && u->sounds<MAXSOUNDS)
                    {
                        if(u->sound[u->sounds]) free(u->sound[u->sounds]);
                        u->sound[u->sounds]=mymalloc(strlen(s)+2);
                        strcpy(u->sound[u->sounds],s);
                        logx(LOG_PACKETS,"- sound %i = %s\n",u->sounds,s);
                        if(!strcmp(u->sound[u->sounds],"weapons/r_exp3.wav"))
                        {
                            logx(LOG_DEBUG,"explosionsound=%i\n",u->sounds);
                            u->explosionsound=u->sounds;
                        }
                        u->sounds++;
                    }
                    DUPTEXT;
                }
                if(u->sounds>MAXSOUNDS) u->sounds=MAXSOUNDS;
                DUP1;
                if(NEWQW) DUP1; // new22
            } break;
            case 0x2F:
            case 0x30:
            {
                int mask,entity,ef,index,modelindex,modelframe,modelskin = 0;
                uchar *d0;
                u->amconnected=10;
                if(dump && cmd==0x2f) logx(LOG_PACKETS,"packetentities\n");
                if(dump && cmd==0x30) logx(LOG_PACKETS,"packetentities(delta)\n");
                if(cmd==0x30)
                {
                    index=DUP1;
                }
                else
                {
                    memset(u->entityvisible,0,sizeof(u->entityvisible));
                }
                for(;;)
                {
                    int wasvisible,isvisible;
                    int oldpos[3];
                    int newobject=1;
                    d0=d;

                    mask=DUP2;
                    modelindex=0;
                    modelframe=0;
                    ef=0;
                    if(!mask) break;
                    entity=mask&0x1ff;
                    mask&=0xfe00;

                    if(mask&0x8000) mask|=DUP1;

                    wasvisible=u->entityvisible[entity];
                    if(mask&0x4000)
                    { // hide
                        u->entityvisible[entity]=0;
                    }
                    else
                    {
                        u->entityvisible[entity]=1;
                    }
                    isvisible=u->entityvisible[entity];

                    if(mask&0x0004)
                    {
                        modelindex=DUP1; // modelindex
                        if(u->fps_rocket && !strcmp(u->model[modelindex],"progs/missile.mdl"))
                        {
                            if(u->grenademodel<=0) d[-1]++;
                            else d[-1]=u->grenademodel;
                            modified=1;
                        }
                    }
                    if(mask&0x2000) modelframe=DUP1; // frame
                    if(mask&0x0008) DUP1; // colormap
                    if(mask&0x0010) modelskin=DUP1; // skin
                    if(mask&0x0020)
                    {
                        ef=DUP1; // effects
                    }

                    if(u->fps_gib && entity<MAXENTITIES && modelindex && cmd==0x30)
                    {
                        u->entitygib[entity]=0;

                        if(u->fps_gib==1)
                        { // also remove gibs
                            if(strstr(u->model[modelindex],"progs/gib")) u->entitygib[entity]=1;
                        }

                        { // remove bodies
                            if(strstr(u->model[modelindex],"progs/h_player")) u->entitygib[entity]=1;
                            if(strstr(u->model[modelindex],"progs/player"))   u->entitygib[entity]=1;
                        }
                    }

#if 0
                    if(isvisible && u->entityname[entity] && !strcmp(u->entityname[entity],"pack"))
                    {
                        modified=1;

                        if(mask&0x0020) d--; // remove old effect if present
                        // make sure the secondary mask is there
                        if(!(mask&0x8000))
                        {
                            uchar *p=d;
                            while(p>d0+2)
                            {
                                p[1]=p[0];
                                p--;
                            }
                            p[0]=0x20;
                            d0[1]|=0x80;
                            d++;
                        }
                        // add the mask
                        d0[2]|=0x20;

                        *d++=0x10; // add grenade effect

                        if(!wasvisible) q_print(u,"<pack appeared>\n");
                    }
#endif

                    if(!wasvisible &&
                       (mask&(0x0004+0x0200+0x0400+0x0800))==(0x0004+0x0200+0x0400+0x0800))
                    {
                        newobject=1;
                        oldpos[0]=u->entitypos[entity][0];
                        oldpos[1]=u->entitypos[entity][1];
                        oldpos[2]=u->entitypos[entity][2];
                    }
                    else newobject=0;

                    if(mask&0x0200) u->entitypos[entity][0]=DUP2; // origin x
                    if(mask&0x0001) DUP1; // angle x
                    if(mask&0x0400) u->entitypos[entity][1]=DUP2; // origin y
                    if(mask&0x1000) DUP1; // angle y
                    if(mask&0x0800) u->entitypos[entity][2]=DUP2; // origin z
                    if(mask&0x0002) DUP1; // angle z

                    if(oldpos[0]==u->entitypos[entity][0] &&
                       oldpos[1]==u->entitypos[entity][1] &&
                       oldpos[2]==u->entitypos[entity][2]) newobject=0;

                    if(newobject)
                    {
                        u->entityname[entity]=makeentityname(u,modelindex,modelskin,&u->entitymask[entity]);

                        // determine pack owner (if model & pos given so pack is new)
                        if(u->classifypacks && !strcmp(u->entityname[entity],"pack"))
                        {
                            /*
                            if(oldpos[0]!=u->entitypos[entity] ||
                               oldpos[0]!=u->entitypos[entity] ||
                               oldpos[0]!=u->entitypos[entity])
                            */
                            { // pack is not in same pos as last time, good
                                if(packlistnum<u->numplayers)
                                {
                                    packlist[packlistnum++]=entity;
                                }
                            }
                        }
                    }

                    if(dump)
                    {
                        logx(LOG_PACKETS,"entity %03X mask=%04X sz=%i: ",entity,mask,d-d0);
                        if(mask&0x8000) logx(LOG_PACKETS,"mask2 ");
                        if(mask&0x4000) logx(LOG_PACKETS,"(hide) ");
                        if(mask&0x0004) logx(LOG_PACKETS,"model ");
                        if(mask&0x2000) logx(LOG_PACKETS,"m.fr ");
                        if(mask&0x0008) logx(LOG_PACKETS,"cmap ");
                        if(mask&0x0010) logx(LOG_PACKETS,"skin ");
                        if(mask&0x0020) logx(LOG_PACKETS,"eff ");
                        if(mask&0x0200) logx(LOG_PACKETS,"p0 ");
                        if(mask&0x0001) logx(LOG_PACKETS,"a0 ");
                        if(mask&0x0400) logx(LOG_PACKETS,"p1 ");
                        if(mask&0x1000) logx(LOG_PACKETS,"a1 ");
                        if(mask&0x0800) logx(LOG_PACKETS,"p2 ");
                        if(mask&0x0002) logx(LOG_PACKETS,"a2 ");
                        logx(LOG_PACKETS,"\n");
                    }

                    if(u->fps_gib && u->entitygib[entity] && cmd==0x30)
                    {
                        int mask;
                        mask=entity|0x4000;
                        d=d0;
                        *d++=mask&255;
                        *d++=mask>>8;
                        modified=1;
                    }
                }
            } break;
            case 0x31: // new22
            { // maxspeed
                if(dump) logx(LOG_PACKETS,"new maxspeed\n");
                DUP4;
            } break;
            case 0x32: // new22
            { // ?
                int a;
                a=DUP4;
                if(dump) logx(LOG_PACKETS,"new unknown (ctf) %08X\n",a);
            } break;
            case 0x33: // new22
            {
                int entity;
                static char attr[80];
                static char value[80];
                uchar *d0;
                entity=1+DUP1; // player?
                strcpy(attr,s);
                DUPTEXT;
                d0=d;
                strcpy(value,s);
                DUPTEXT;
                if(dump) logx(LOG_PACKETS,"new setinfo %s=%s\n",attr,value);
                if(!strcmp(attr,"team"))
                {
                    strncpy(u->playerteamtext[entity],value,7);
                    u->playerteam[entity]=teamint(value);
                    if(entity==u->entity)
                    {
                        u->time_reskinall=u->time+500;
                    }
                    else skinoverride(u,entity);
                }
                else if(!strcmp(attr,"skin"))
                {
                    if(strlen(value)>16)
                    {
                        value[16]=0;
                        d0[16]=0;
                        d=d0+17;
                        modified=1;
                        logx(LOG_DEBUG,"user %i: longskin1\n",entity);
                    }
                    strncpy(u->playerskin[entity],value,MAXSKINNAME-1);
                    skinoverride(u,entity);
                }
                else if(!strcmp(attr,"topcolor"))
                {
                    u->playertcolor[entity]=atoi(value);
                    if(entity==u->entity)
                    {
                        u->tcolor=atoi(value);
                    }
                    skinoverride(u,entity);
                }
                else if(!strcmp(attr,"bottomcolor"))
                {
                    u->playerbcolor[entity]=atoi(value);
                    if(entity==u->entity)
                    {
                        u->bcolor=atoi(value);
                    }
                    skinoverride(u,entity);
                }
                else if(!strcmp(attr,"teamcolor"))
                {
                    u->playerxcolor[entity]=text2color(value);
                    skinoverride(u,entity);
                }
                else if(!strcmp(attr,"spectator"))
                {
                }
                else if(!strcmp(attr,"Cheapo"))
                {
                    if(!*value)
                    {
                        strcpy(u->playerproxy[entity],"-");
                    }
                    else
                    {
                        value[15]=0;
                        sprintf(u->playerproxy[entity],"Cheapo %s",value);
                    }
                }
                else if(!strcmp(attr,"name"))
                {
                    strncpy(u->playername[entity],value,MAXPLAYERNAME-1);
                }
#ifdef IDENT
                else if(!strcmp(attr,"#"))
                {
                    gotinfomessage(u,value,entity);
                }
#endif
            } break;
            case 0x34: // new22
            {
                char *attr,*value;
                if(dump) logx(LOG_PACKETS,"new serverinfo/userinfo?\n");

                tmpbuf[0]='f'+10;
                tmpbuf[1]='p'+11;
                tmpbuf[2]='d'+12;
                tmpbuf[3]=0;

                attr=s;
                DUPTEXT;
                value=s;
                DUPTEXT;

                if(!strcmp(attr,"deathmatch"))
                {
                    dmmchange(u,atoi(value));
                }

                tmpbuf[3]=0;
                tmpbuf[2]-=12;
                tmpbuf[1]-=11;
                tmpbuf[0]-=10;

                if(!strcmp(attr,tmpbuf))
                {
                    u->fpd=atoi(value)|litemode;
                    u->fpdcopy=atoi(value)|litemode;
#ifdef TIMERS
                    if(u->notimers) u->fpd|=FPD_TIMERS;
                    if(u->notimers) u->fpdcopy|=FPD_TIMERS;
#endif
                    reportfpd(u,u->fpd);
                }
            } break;
            case 0x35: // new23
            {
                DUP1; // player?
                DUP1; // ploss
            } break;
            default:
            { // unrecognized data, bailout (will cause parse error message)
                s=s_end;
                modified=0;
            } break;
        }
    }

#ifdef RLPACK
    if(packlistnum)
    {
        int i,entity;
        for(i=0;i<packlistnum;i++)
        {
            int owner=0;
            entity=packlist[i];
#ifdef RLPACKDEBUG
            q_print(u,"<checkrlpack>\n");
#endif
            {
                // find out whos pack
                int ownerd=999999999;
                int i,d;

                for(i=0;i<=32;i++) if(u->playeractive[i] && u->time<u->time_playerdied[i]+1000)
                {
                    d =abs(u->entitypos[entity][0]-u->playerpos[i][0]);
                    d+=abs(u->entitypos[entity][1]-u->playerpos[i][1]);
                    d+=abs(u->entitypos[entity][2]-u->playerpos[i][2]);
                    if(d<ownerd)
                    {
                        owner=i;
                        ownerd=d;
                    }
                }
            }
            if(owner)
            {
#ifdef RLPACKDEBUG
                char msg[80];
                sprintf(msg,"<%s's pack, rtime %i>\n",
                    u->playername[owner],u->time-u->time_playerrocket[owner]);
                q_print(u,msg);
#endif
                if(u->time-u->time_playerrocket[owner]<700)
                {
                    char msg[64];
                    sprintf(msg,"proxy: rl-pack (%s)\n",u->playername[owner]);
                    q_print(u,msg);
                    u->entityname[entity]="rl-pack";
                }
            }
        }
    }
#endif

    if(u->specmode && u->specentity)
    { // track
        int a,i;
        if(dump)
        {
            logx(LOG_DEBUG,"obs: %02X pos: %i %i %i angles: %i %i %i \n",
                u->entity,
                u->tr_postar[0],
                u->tr_postar[1],
                u->tr_postar[2],
                u->tr_angtar[0],
                u->tr_angtar[1],
                u->tr_angtar[2]);
        }

        // smoothing
        for(i=0;i<3;i++)
        {
            a=u->tr_pos[i]-u->tr_postar[i]; if(a<0) a=-a;
            if(a>512 ) u->tr_pos[i]=u->tr_postar[i];
            else       u->tr_pos[i]=(u->tr_pos[i]+u->tr_postar[i])/2;
            a=u->tr_ang[i]-u->tr_angtar[i]; if(a<0) a=-a;
            if(a>8000) u->tr_ang[i]=u->tr_angtar[i];
            else       u->tr_ang[i]=(u->tr_ang[i]+u->tr_angtar[i])/2;
        }

        modified=1;
        // create angles command from server
        *d++=0x0a;
        *d++=u->tr_ang[0]>>8;
        *d++=u->tr_ang[1]>>8;
        *d++=u->tr_ang[2]>>8;
        // create player pos command from server
        *d++=0x2a;
        *d++=u->entity-1;
        // mask
        *d++=0;
        *d++=0;
        // position
        *d++=u->tr_pos[0]&255;
        *d++=u->tr_pos[0]>>8;
        *d++=u->tr_pos[1]&255;
        *d++=u->tr_pos[1]>>8;
        *d++=u->tr_pos[2]&255;
        *d++=u->tr_pos[2]>>8;
        // frametime
        *d++=0x1f;
        // create a specating command to client
        u->appendclient[u->appendclientbytes++]=0x06;
        u->appendclient[u->appendclientbytes++]=u->tr_postar[0]&255;
        u->appendclient[u->appendclientbytes++]=u->tr_postar[0]>>8;
        u->appendclient[u->appendclientbytes++]=u->tr_postar[1]&255;
        u->appendclient[u->appendclientbytes++]=u->tr_postar[1]>>8;
        u->appendclient[u->appendclientbytes++]=u->tr_postar[2]&255;
        u->appendclient[u->appendclientbytes++]=u->tr_postar[2]>>8;
    }

    if(modified)
    {
        logx(LOG_PACKETS,"Packet MODIFIED!\n");
        u->dumpsentpacket=1;
    }
    if(!modified && (bytes!=d-outbuf || memicmp(buffer+8,outbuf+8,bytes-8)))
    {
        u->noappendnow=2;
        logx(LOG_CONSOLE,"Packet parse error (saved to parse.log)\n",cmd);
        logx(LOG_DEBUG,"last command %02X at %03X\n",cmd,cmdpos);
        saveparseerror("<%02X@server.%04X>",cmdpos,buffer,bytes);
        memcpy(outbuf,buffer,bytes);
        *outbytes=bytes;
        sprintf(buf,"proxy: parsing error [s%02X]\n",lastcmd);
        q_print(u,buf);
        if(u->parsecount>100)
        {
            u->safemode=1;
#if defined(__APPLE__) || defined(MACOSX)
            sprintf(buf,"proxy: packet parsing disabled\n");
#else
            sprintf(buf,"proxy: packet parsing disabled\n",cmd);
#endif /* __APPLE__ ||ÊMACOSX */
            q_print(u,buf);
            q_print(u,buf);
            q_print(u,buf);
        }
        else
            u->parsecount+=10;
        return;
    }
    else
    {
        // set size
        *outbytes=d-outbuf;
        if(u->parsecount<0)
            u->parsecount=0;
        else
            u->parsecount--;
    }
}

int handleclient(User *u,uchar *buffer,int bytes,uchar *outbuf,int *outbytes)
{
    unsigned int *p=(unsigned int *)buffer;
    int   duptmp;
    uchar  buf[MAXSTR];
    uchar *s,*s_end,*d;
    uchar *s_beg,*d_beg;
    int   cmd,cmdpos = 0;
    int   modified=0;
    int   dump=loglevel>=LOG_PACKETS;
    int   simplepacket=1;
    int   lastcmd = 0;

    if(u->safemode)
    {
        memcpy(outbuf,buffer,bytes);
        *outbytes=bytes;
        return(0);
    }

    if(*p==0xffffffff)
    {
        memcpy(outbuf,buffer,bytes);
        *outbytes=bytes;
        return(0);
    }

    s=buffer;
    s_end=buffer+bytes;
    d=outbuf;
    s_beg=s;
    d_beg=d;
    // copy headers
    DUPN(8);
    // new22
    if(NEWQW)
    {
        *d++=u->response_b&255;
        *d++=u->response_b>>8;
        u->clientident=shortat(s);
        s+=2;
    }
    else u->clientident=-1;
    // copy rest
    cmd=0xff;
    while(s<s_end)
    {
        if(dump) logx(LOG_PACKETS,"%02X@%02X: ",*s,s-buffer);
        lastcmd=cmd;
        cmd=DUP1;
        cmdpos=s-buffer;
        if(cmd!=0x03 && cmd!=0x05) simplepacket=0;
        switch(cmd)
        {
            case 0x04:
            {
                if(dump) logx(LOG_PACKETS,"servercmd: %s\n",formatqstring(s));

                /*
                {
                    sprintf(tmpbuf,"servercmd: %s\n",formatqstring(s));
                    q_print(u,tmpbuf);
                }
                */

                if(!memcmp(s,"cheapolink",5))
                {
                    u->linkedclient=1;
                    // remove command from server stream
                    modified=1;
                    d--;
                    // skip from source
                    SKIPTEXT;
                }
                else if(!memcmp(s,"say .",5))
                {
                    q_command(u,s+4);
                    // remove command from server stream
                    modified=1;
                    d--;
                    // skip from source
                    SKIPTEXT;
                }
                else if(!memcmp(s,"say \".",6))
                {
                    q_command(u,s+5);
                    // remove command from server stream
                    modified=1;
                    d--;
                    // skip from source
                    SKIPTEXT;
                }
                else if(!memcmp(s,"say !",5))
                {
                    // remove command from server stream
                    modified=1;
                    d--;
                    // skip from source
                    SKIPTEXT;
                    q_print(u,"Ignored !-command\n");
                }
                else if(!memcmp(s,u->proxysay,strlen(u->proxysay)))
                {
                    int l=strlen(u->proxysay);
                    s[l-1]='.';
                    q_command(u,s+l-1);
                    // remove command from server stream
                    modified=1;
                    d--;
                    // skip from source
                    SKIPTEXT;
                }
                else if(!memcmp(s,u->proxysay2,strlen(u->proxysay2)))
                {
                    int l=strlen(u->proxysay2);
                    s[l-1]='.';
                    q_command(u,s+l-1);
                    // remove command from server stream
                    modified=1;
                    d--;
                    // skip from source
                    SKIPTEXT;
                }
                else if(!memcmp(s,"say ",4))
                {
                    if(floodprot(u))
                    {
                        d--;
                        SKIPTEXT;
                        modified=1;
                        break;
                    }

                    if(u->state==STATE_HERE && !u->linkedserver)
                    {
                        int i;
                        // send saying to all other users in proxy
                        sprintf(tmpbuf,"[[%s]]: %s\n",u->name,formatqstring(s+4));
                        //strxor80(tmpbuf);
                        for(i=0;i<usernum;i++) if(user[i].state==STATE_HERE)
                        {
                            q_print(user+i,tmpbuf);
                        }
                        DUPTEXT;
                    }
                    else
                    {
                        /*
                        Userinfo ui;
                        userinfofromuser(&ui,u);
                        modified|=handlesaying(u,&ui,&d,&s);
                        */
                        modified|=funnyconvert(d,s,3);
                        d+=strlen(d)+1;
                        s+=strlen(s)+1;
                    }
                }
                else if(!memcmp(s,"say_team ",9))
                {
                    if(floodprot(u))
                    {
                        d--;
                        SKIPTEXT;
                        modified=1;
                        break;
                    }

                    {
                        char *d0=d;
                        int m;
                        Userinfo ui;
                        userinfofromuser(&ui,u);
                        m=handlesaying(u,&ui,&d,&s);
                        if(m) modified|=1;
                        if(m&2)
                        {
                            fixuphidemsg(u,d0);
                            d=d0+strlen(d0)+1;
                        }
                    }
                }
                else if(!memcmp(s,"drop",4))
                {
                    q_command(u,"!drop");
                    DUPTEXT;
                }
                else if(!memcmp(s,"ptrack",6))
                { // spectator change
                    if(s[7]>32) u->client_spectrack=1;
                    else u->client_spectrack=0;
                    DUPTEXT;
                }
                else if(!memcmp(s,"begin",5))
                { // game begin
                    int a;
                    u->time_reskinall=u->time+3000;
                    sscanf(s,"begin %i",&a);
                    u->lastbegin=a;
                    DUPTEXT;
                }
                else if(!memcmp(s,"soundlist",9))
                {
                    int a,b;
                    sscanf(s,"soundlist %i %i",&a,&b);
                    if(b==0) u->cnt_sound++;
                    logx(LOG_DEBUG,"client requests soundlist %i %i\n",a,b);
                    DUPTEXT;
                }
                else if(!memcmp(s,"modellist",9))
                {
                    int a,b;
                    sscanf(s,"modellist %i %i",&a,&b);
                    if(b==0) u->cnt_model++;
                    logx(LOG_DEBUG,"client requests modellist %i %i\n",a,b);
                    DUPTEXT;
                }
                else if(!memcmp(s,"setinfo",5))
                {
                    uchar attr[MAXSTR],value[MAXSTR];
                    uchar *p,*sorg;
                    int  endmark,change=0,remove=0;

                    sorg=s;
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;

                    *attr=0;
                    *value=0;

                    // attribute
                    p=attr;
                    if(*s=='\"')
                    {
                        s++;
                        endmark='\"';
                    }
                    else endmark=' ';
                    while(*s && *s!=endmark && p<attr+60)
                    {
                        *p++=*s++;
                    }
                    if(*s) s++;
                    *p=0;

                    while(*s<=32 && *s) s++;

                    // value
                    p=value;
                    if(*s=='\"')
                    {
                        s++;
                        endmark='\"';
                    }
                    else endmark=' ';
                    while(*s && *s!=endmark && p<value+60)
                    {
                        *p++=*s++;
                    }
                    if(*s) s++;
                    *p=0;

                    logx(LOG_DEBUG,"Setinfo <%s> <%s> s=%02X d=%02X\n",
                        attr,value,s-s_beg,d-d_beg);

                    if(!stricmp(attr,"Cheapo"))
                    { // do not allow changing of cheapo setinfo
#ifndef PROMODE
                        remove=1;
#endif
                    }
                    else if(!stricmp(attr,"name"))
                    { // fake name helper
                        if(funnyconvert(value,value,0))
                        {
                            q_print(u,"Name containing $ or # modified.\n");
                            change=1;
                        }
                    }
                    else if(!stricmp(attr,"teamcolor"))
                    {
                        u->playerxcolor[u->entity]=text2color(value);
                        skinoverride(u,u->entity);
                    }

                    if(!remove) cl_setinfo(u,attr,value);

                    if(remove)
                    {
                        d--;

                        s=sorg;
                        SKIPTEXT;

                        modified=1;
                    }
                    else if(change)
                    {
                        p="setinfo \"";
                        while(*p) *d++=*p++;
                        p=attr;
                        while(*p) *d++=*p++;
                        p="\" \"";
                        while(*p) *d++=*p++;
                        p=value;
                        while(*p) *d++=*p++;
                        p="\"";
                        while(*p) *d++=*p++;
                        *d++=0xA;
                        *d++=0;

                        s=sorg;
                        SKIPTEXT;

                        modified=1;
                    }
                    else
                    {
                        s=sorg;
                        DUPTEXT;
                    }
                }
                else
                {
                    DUPTEXT;
                }
            } break;
            case 0x05:
            {
                if(dump) logx(LOG_PACKETS,"sequence(?)\n");
                DUP1;
            } break;
            case 0x06:
            {
                if(dump) logx(LOG_PACKETS,"spectator(?)\n");
                DUP2; DUP2; DUP2;
            } break;
            case 0x03:
            {
                int i,sx,sy,sz,sp;
                uchar *d0,*d1;
                d0=d;
                if(NEWQW) DUP1; // new22, crc
                u->impulsehistory[0]=u->impulsehistory[1];
                u->impulsehistory[1]=u->impulsehistory[2];
                u->impulsehistory[2]=0;
                if(u->version<=22)
                {
                    for(i=0;i<3;i++)
                    {
                        int command,tilt=0,yaw=0,roll=0,flag,impulse;
                        impulse=0;
                        command=DUP1;
                        if(dump)
                        {
                            if(i!=0) logx(LOG_PACKETS,"--@--: ");
                            logx(LOG_PACKETS,"movement(%i): mask:%02X ",i,command);
                        }
                        sx=sy=sz=sp=0;
                        if(command&0x01) tilt=DUP2;
                        yaw=DUP2;
                        if(command&0x02) roll=DUP2;
                        if(i==0)
                        {
                            u->ang[0]=tilt;
                            u->ang[1]=yaw;
                            u->ang[2]=roll;
                        }
                        if(command&0x04) sx=DUP1; // speed x
                        if(command&0x08) sy=DUP1; // speed y
                        if(command&0x10) sz=DUP1; // speed z

                        if(command&0x20)
                        {
                            flag=DUP1; // flag
                            if(dump) logx(LOG_PACKETS,"flag:%02X ",flag);
                            if(!i)
                            {
                                if((flag&2) && !(u->lastflag&2) && u->specmode)
                                { // jump
                                    spectatenext(u,NULL);
                                }
                                if((flag&1) && u->specmode)
                                { // fire
                                    u->specmode=0;
                                    q_print(u,"Tracking disabled.\n");
                                }
                                u->lastflag=flag;
                            }
                            if(u->specmode)
                            {
                                if(flag&2)
                                {
                                    d[-1]&=~2;
                                    modified=1;
                                }
                            }
                        }
                        else
                        {
                            if(!i) u->lastflag=0;
                        }
                        if(command&0x40)
                        {
                            impulse=DUP1; // impulse
                            if(dump) logx(LOG_PACKETS,"impulse:%02X ",impulse);
                            if(i==2 && impulse)
                            {
                                if(modifyimpulse(u,&impulse))
                                { // modified
                                    if(dump) logx(LOG_PACKETS,"=>%02X ",impulse);
                                    d[-1]=impulse;
                                    modified=1;
                                    u->impulsehistory[2]=impulse;
                                }
                                else
                                { // not modified
                                    u->impulsehistory[2]=0;
                                }
                            }
                            else if(u->impulsehistory[i])
                            {
                                if(dump) logx(LOG_PACKETS,"=>%02X ",u->impulsehistory[i]);
                                d[-1]=u->impulsehistory[i];
                                modified=1;
                            }
                        }
                        if(command&0x80)
                        {
                            sp=DUP1;
                            {
                                int ms=sp;
                                int fps;
                                if(ms<10) fps=100;
                                else fps=1000/ms;
                                u->playerfps[u->entity]=(u->playerfps[u->entity]+fps+1)/2;
                            }
                            if(dump) logx(LOG_PACKETS,"time:%i ",sp);
                            /* speed cheat
                            modified=1;
                            d[-1]*=2;
                            */
                        }
                        if(dump) logx(LOG_PACKETS,"\n");
                    }
                }
                else
                {
                    if(u->version>=24)
                    {
                        DUP1; // ploss
                    }
                    for(i=0;i<3;i++)
                    {
                        int command,tilt=0,yaw=0,roll=0,flag,impulse;
                        char *cmd0;
                        impulse=sx=sy=sz=sp=0;
                        cmd0=d;
                        command=DUP1;
                        if(dump)
                        {
                            if(i!=0) logx(LOG_PACKETS,"--@--: ");
                            logx(LOG_PACKETS,"movement(%i): mask:%02X ",i,command);
                        }
                        if(command&0x01) tilt=DUP2;
                        if(command&0x80) yaw =DUP2;
                        if(command&0x02) roll=DUP2;
                        if(i==0)
                        {
                            u->ang[0]=tilt;
                            u->ang[1]=yaw;
                            u->ang[2]=roll;
                        }

                        if(command&0x04)
                        { // forward
                            sx=DUP2;
                        }
                        if(command&0x08)
                        { // strafe
                            sy=DUP2;
                        }
                        if(command&0x10)
                        { // jump
                            sz=DUP2;
                        }

                        if(command&0x20)
                        {
                            flag=DUP1; // flag
                            if(dump) logx(LOG_PACKETS,"flag:%02X ",flag);
                            if(!i)
                            {
                                if((flag&2) && !(u->lastflag&2) && u->specmode)
                                { // jump
                                    spectatenext(u,NULL);
                                }
                                if((flag&1) && u->specmode)
                                { // fire
                                    u->specmode=0;
                                    q_print(u,"Tracking disabled.\n");
                                }
                                u->lastflag=flag;
                            }
                            if(u->specmode)
                            {
                                if(flag&2)
                                {
                                    d[-1]&=~2;
                                    modified=1;
                                }
                            }
                        }
                        else
                        {
                            if(!i) u->lastflag=0;
                        }

                        if(command&0x40)
                        {
                            impulse=DUP1; // impulse
                            if(dump) logx(LOG_PACKETS,"impulse:%02X ",impulse);
                            if(i==2 && impulse)
                            {
                                if(modifyimpulse(u,&impulse))
                                { // modified
                                    if(dump) logx(LOG_PACKETS,"=>%02X ",impulse);
                                    d[-1]=impulse;
                                    modified=1;
                                    u->impulsehistory[2]=impulse;
                                }
                                else
                                { // not modified
                                    u->impulsehistory[2]=0;
                                }
                            }
                            else if(u->impulsehistory[i])
                            {
                                if(dump) logx(LOG_PACKETS,"=>%02X ",u->impulsehistory[i]);
                                d[-1]=u->impulsehistory[i];
                                modified=1;
                            }
                        }
                        {
                            sp=DUP1;
                            {
                                int ms=sp;
                                int fps;
                                if(ms<10) fps=100;
                                else fps=1000/ms;
                                u->playerfps[u->entity]=(u->playerfps[u->entity]+fps+1)/2;
                            }
                            if(dump) logx(LOG_PACKETS,"time:%i ",sp);
                        }

                        if(dump) logx(LOG_PACKETS,"\n");
                    }
                }

                d1=d;

#if 0
                logx(LOG_VERBOSE,"Client movedata check: packet %i/%i real %02X calculated %02X\n",
                     *((int *)(buffer+0)) & 0x7fffffff,
                     *((int *)(buffer+4)) & 0x7fffffff,
                     d0[0],checksum(buffer,d0+1,d1-d0-1));
#endif

                if(NEWQW && modified)
                { // recalculate packet checksum
                    *d0=checksum(buffer,d0+1,d1-d0-1);
                }
            } break;
            default:
            { // unrecognized data, bailout (will cause parse error message)
                s=s_end;
                modified=0;
            } break;
        }
    }

    if(modified)
    {
        logx(LOG_PACKETS,"Packet MODIFIED!\n");
        u->dumpsentpacket=1;
    }
    if(!modified && (bytes!=d-outbuf || memicmp(buffer+10,outbuf+10,bytes-10)))
    {
        lognpackets=1;
        oldloglevel=loglevel;
        loglevel=LOG_PACKETS;

        u->noappendnow=2;
        logx(LOG_CONSOLE,"Packet parse error (saved to parse.log)\n",cmd);
        logx(LOG_DEBUG,"last command %02X at %03X\n",cmd,cmdpos);
        saveparseerror("<%02X@client.%04X>",cmdpos,buffer,bytes);
        memcpy(outbuf,buffer,bytes);
        *outbytes=bytes;
        sprintf(buf,"proxy: parsing error [c%02X]\n",lastcmd);
        q_print(u,buf);
        if(u->parsecount>100)
        {
            u->safemode=1;
#if defined(__APPLE__) || defined(MACOSX)
            sprintf(buf,"proxy: packet parsing disabled\n");
#else
            sprintf(buf,"proxy: packet parsing disabled\n",cmd);
#endif /* __APPLE__ ||ÊMACOSX */
            q_print(u,buf);
            q_print(u,buf);
            q_print(u,buf);
        }
        else
            u->parsecount+=10;
        return(0);
    }
    else
    {
        // set size
        *outbytes=d-outbuf;
        if(u->parsecount<0)
            u->parsecount=0;
        else
            u->parsecount--;
    }
    return(simplepacket);
}

void appendpacket(uchar *buf,int *bytes,uchar *adata,int *abytes)
{
    if(*bytes+*abytes>APPENDSIZE) return;
    memcpy(buf+*bytes,adata,*abytes);
    *bytes+=*abytes;
    *abytes=0;
}

/********************************/

typedef int (*qsortfunc)(const void *a,const void *b);

int cmpnum(const uchar **txt1,const uchar **txt2)
{
    const uchar *a,*b;
    uchar at[3],bt[3];
    int  ai,bi;

    a=*txt1;
    b=*txt2;

    a+=strlen(a)-3;
    b+=strlen(b)-3;

    at[0]=a[0]&0x7f;
    at[1]=a[1]&0x7f;
    at[2]=0;

    bt[0]=b[0]&0x7f;
    bt[1]=b[1]&0x7f;
    bt[2]=0;

    if(at[0]=='?') ai=999;
    else if(at[1]=='m') ai=100+at[0];
    else ai=atoi(at);

    if(bt[0]=='?') bi=999;
    else if(bt[1]=='m') bi=100+bt[0];
    else bi=atoi(bt);

    return(ai-bi);
}

void mixoverlays(char *d,char *s1,char *s2)
{
    char *dend=d+1000,*d0=d,*s0;
    int   lines=0;
//    logx(LOG_CONSOLE,"mix<%s><%s>\n",s1,s2);
    while((*s1 || *s2) && d<dend)
    {
        while(*s1=='\r') s1++;
        while(*s2=='\r') s2++;

        if( ((*s1) && (*s1)!='\n') )
        { // line from s1
            s0=s1;
        }
        else
        {
            s0=s2;
        }

        while(*s0 && *s0!='\n') *d++=*s0++;
        while(*s1 && *s1!='\n') s1++;
        while(*s2 && *s2!='\n') s2++;
        if(*s1 || *s2) *d++='\n';
        if(*s1=='\n') s1++;
#if defined(__APPLE__) || defined(MACOSX)
        // BUG?
        if(*s2=='\n') s2++;
#else
        if(*s2=='\n') *s2++;
#endif /* __APPLE__ ||ÊMACOSX */
        lines++;
    }
    if(lines>500) *d0=0;
    *d++=0;
//    logx(LOG_CONSOLE,"mixresult<%s>\n",d0);
}

void readheader(uchar *inbuf,int *seqthis,int *seqrecv,int *bitthis,int *bitrecv)
{
    *seqthis=intat(inbuf+0);
    *seqrecv=intat(inbuf+4);
    if(*seqthis==-1) *bitthis=0x80;
    else
    {
        *bitthis=(*seqthis&0x80000000)?0x80:0x00;
        *seqthis&=0x7fffffff;
    }
    if(*seqrecv==-1) *bitrecv=0x80;
    else
    {
        *bitrecv=(*seqrecv&0x80000000)?0x80:0x00;
        *seqrecv&=0x7fffffff;
    }
}

void writeheader(uchar *inbuf,int *seqthis,int *seqrecv,int *bitthis,int *bitrecv)
{
    int s1,s2;
    if(*seqthis==-1) s1=-1;
    else
    {
        s1=*seqthis&0x7fffffff;
        if(*bitthis) s1|=0x80000000;
    }
    if(*seqrecv==-1) s2=-1;
    else
    {
        s2=*seqrecv&0x7fffffff;
        if(*bitrecv) s2|=0x80000000;
    }
    setintat(inbuf+0,s1);
    setintat(inbuf+4,s2);
}

int packetmatch(User *u,uchar *buffer)
{ // 0=exact match, smaller better
    int nthis,nrecv,value,dummy;
    int timedelay,ident;

    timedelay=500;

    readheader(buffer,&nthis,&nrecv,&dummy,&dummy);
    ident=shortat(buffer+8);

    if(u->time<u->time_lastclient+timedelay) return(0x7fffffff);
    if(nthis<u->client_lastthis) return(0x7fffffff);
    if(nrecv>u->server_lastthis) return(0x7fffffff);
    if(u->clientident!=-1 && ident!=u->clientident) return(0x7fffffff);
    value =abs(nrecv-u->server_lastthis);
    value+=abs(nthis-u->client_lastthis);
    return(value);
}

void q_serverextras(User *u)
{
    int i;

    if(u->time>u->time_lastoverlay+1100)
    {
        *u->overlay=0;
    }

    if(u->trackloc)
    {
        int i,n;
        // update one visible player / packet
        for(i=0;i<32;i++)
        {
            u->tracklocind++;
            if(u->tracklocind>32) u->tracklocind=0;
            n=u->tracklocind;
            if(u->playeractive[n])
            {
                int l;
                int old1,old2;
                findnear(u,u->playerpos[n][0],u->playerpos[n][1],u->playerpos[n][2],&l);

                old1=u->playerlastloc[n];
                old2=u->playerloc[n];

                u->playerlastloc[n]=u->playerloc[n];
                u->playerloc[n]=l;

                if(!playerlocdirection(u,n))
                {
                    u->playerlastloc[n]=old1;
                    u->playerloc[n]=old2;
                }
                break;
            }
            else
            {
                u->playerloc[n]=u->playerlastloc[n]=0;
            }
        }
    }

    if(u->time_reskinall && u->time>u->time_reskinall)
    {
        if(u->skinoverride || u->coloroverride)
        {
            int limit=2; // 2 updates per packet

            logx(LOG_DEBUG,"reskinall (pos=%i)\n",u->skinoverridepos);

            // reset change counter to start
            if(u->skinoverridepos>=34)
            {
                int i;
                u->playerfragsbest=0;
                for(i=0;i<32;i++)
                {
                    if(u->playerfrags[i]>u->playerfragsbest)
                    {
                        u->playerfragsbest=u->playerfrags[i];
                    }
                }
                u->skinoverridepos=0;
            }

            // change limit skins
            while(limit>0)
            {
                if(u->playeractive[u->skinoverridepos])
                {
                    limit--;
                    u->time_reskinall=0;
                    if(u->color==-3 || u->color2==-3)
                    {
                        rankchange(u,u->skinoverridepos,0);
                    }
                    skinoverride(u,u->skinoverridepos);
                    u->time_reskinall=u->time;
                }

                u->skinoverridepos++;
                if(u->skinoverridepos>=34)
                {
                    if(u->skinoverride<0) u->skinoverride=0;
                    q_stufftext(u,"skins\n");
                    u->time_reskinall=0;
                    break;
                }
            }
        }
        else u->time_reskinall=0;
    }
    else
    {
        if(u->coloroverride && u->color==-3 || u->color2==-3)
        {
            // increment counter (all players updated every 32 packets)
            u->rankoverridepos+=7;
            if(u->rankoverridepos>=33) u->rankoverridepos-=32;
            if(u->rankoverridepos>=33) u->rankoverridepos=1;

            if(u->playeractive[u->rankoverridepos])
            {
                // rankupdate that player
                rankchange(u,u->rankoverridepos,1);
            }
        }
    }

    // player nearness check
    for(i=0;i<u->numplayers;i++)
    {
        u->playernear[i]--;
        if(u->playernear[i]<0) u->playernear[i]=0;
    }

    if(!u->amspectator && u->time_sendfreport && u->time>u->time_sendfreport)
    {
        u->time_sendfreport=0;
        u->time_lastfreport=u->time;
        q_stufftext(u,"f_report\n");
    }

#if 0
    if(u->pointshow && u->time>u->time_nexttarget)
    {
        uchar *n;

        n=findpoint(u,u->pos[0],u->pos[1],u->pos[2],u->ang[0],u->ang[1],u->ang[2],u->pointshow,0,NULL);
        strcpy(u->msgline,n);
        strxor80(u->msgline);

        u->time_nexttarget=u->time+100; // 0.1 sec
        if(strcmp(n,u->lasttarget)) u->time_nextcamp=0;
        strcpy(u->lasttarget,n);
    }
#endif

    if(u->showhud)
    {
        static uchar buf[MAXSTR];
        static uchar buf2[MAXSTR];

        u->overlayactive=1;

        if(u->time>u->time_nextcamp || u->time<u->time_nextcamp-1000)
        {
            int a;

            u->time_nextcamp=u->time+100;

            memset(buf,'\n',u->moveoverlay);
            buf[u->moveoverlay]=0;

            strcat(buf,"\n\n\n\n");


            {
                     if(u->items&ITEMS_RA) buf2[0]=0x80|'r';
                else if(u->items&ITEMS_YA) buf2[0]=0x80|'y';
                else if(u->items&ITEMS_GA) buf2[0]=0x80|'g';
                else                       buf2[0]=0x80|'a';
                buf2[1]=0x9d;
                buf2[2]=0x9e;
                buf2[3]=0x9e;
                buf2[4]=0x9e;
                buf2[5]=0x9e;
                buf2[6]=0x9f;
                buf2[7]='\n';
                buf2[8]=0;

                a=6*u->armor/195;
                if(a<1) a=1;
                if(a>6) a=6;
                     if(u->armor<50)  buf2[a]=0x87;
                else if(u->armor<101) buf2[a]=0x88;
                else                  buf2[a]=0x8d;

                strcat(buf,buf2);
            }

            {
                if(u->health>100) buf2[0]=0x80|'m';
                else              buf2[0]=0x80|'h';
                buf2[1]=0x9d;
                buf2[2]=0x9e;
                buf2[3]=0x9e;
                buf2[4]=0x9e;
                buf2[5]=0x9e;
                buf2[6]=0x9f;
                buf2[7]='\n';
                buf2[8]=0;

                a=6*u->health/95;
                if(a<1) a=1;
                if(a>6) a=6;
                     if(u->health<50) buf2[a]=0x87;
                else if(u->health<75) buf2[a]=0x88;
                else                  buf2[a]=0x8d;

                strcat(buf,buf2);
            }

            if(u->ammo_rockets<5 && (u->items&ITEMS_RL))
            {
                sprintf(buf2,"ammo:%i\n",u->ammo_rockets);
                strcat(buf,buf2);
            }

            q_centerprint(u,buf);
            return;
        }
    }

    if(u->time>u->time_nextcamp)
    {
        u->time_nextcamp=u->time+1000;

        // determine last active player
        for(i=33;i>1;i--) if(u->playeractive[i]) break;
        u->numplayers=i;

        // megahealth check
        for(i=0;i<u->camps;i++)
        {
            Camp *ca;
            ca=&u->camp[i];
            if(ca->category&CAMP_MH)
            {
                if(u->health<=100 && ca->appeartime==UNKNOWNTIME)
                {
                    ca->appeartime=20500+u->time;
                    ca->secondcamp=0;
                }
            }
        }

        if(u->showspeed || u->showtime || u->showdist)
        {
            *u->msgline=0;
            u->time_nextcamp=u->time+200;
            if(u->showtime)
            {
                sprintf(u->msgline+strlen(u->msgline),"time:%03i ",((u->time-u->showtimebase)/1000)%1000);
            }
            if(u->showspeed)
            {
                int s,i;
                s=0;
                for(i=0;i<4;i++) s+=u->speed[i];
                s=s/300;
                sprintf(u->msgline+strlen(u->msgline),"speed:%03i ",s);
            }
            if(u->showdist)
            {
                float d,f;
                f=(float)(u->showdistbase[0]-u->pos[0]);
                d=f*f;
                f=(float)(u->showdistbase[1]-u->pos[1]);
                d+=f*f;
                f=(float)(u->showdistbase[2]-u->pos[2]);
                d=(float)sqrt(d);

                sprintf(u->msgline+strlen(u->msgline),"dist:%04i (x:%04i y:%04i z:%04i) ",
                    (int)(d/10),
                    (u->showdistbase[0]-u->pos[0])/10,
                    (u->showdistbase[1]-u->pos[1])/10,
                    (u->showdistbase[2]-u->pos[2])/10);
            }
        }

        if(u->campthese||u->specmode||*u->msgline||*u->overlay||*u->notice)
        { // show to self
            static uchar buf[MAXSTR];
            static uchar buf1[MAXSTR]; // first camp line
            static uchar buf2[MAXSTR]; // second camp line
#ifdef TIMERS
            static uchar reports[64][16];
#endif /* TIMERS */
            int          i;

            memset(buf,0,MAXSTR);
            memset(buf,'\n',u->linesdown/2+u->moveoverlay);
            if(*u->notice)
            {
                strcat(buf,u->notice);
                *u->notice=0;
            }
            memset(buf+strlen(buf),'\n',u->linesdown-u->linesdown/2-u->moveoverlay);

            // addc lock
            if(u->specmode)
            {
                if(u->specentity==u->entity)
                {
#if defined(__APPLE__) || defined(MACOSX)
                    buf[strlen(buf)] = 0x00;
#else
                    sprintf(buf+strlen(buf),"");
#endif /* __APPLE__ || MACOSX */
                }
                else if(u->specentity==0)
                {
                    sprintf(buf+strlen(buf),"nobody to track\n");
                }
                else
                {
                    uchar *armor=" ";
                    if(u->items&0x2000)
                    {
                        armor="g";
                    }
                    if(u->items&0x4000)
                    {
                        armor="y";
                    }
                    if(u->items&0x8000)
                    {
                        armor="r";
                    }
                    sprintf(buf+strlen(buf),"%s \x9c \xE8\xE5\xE1\xEC\xF4\xE8 %3i \x9c \xe1\xf2\xed\xef\xf2 %s %3i\n",
                        u->playername[u->specentity],
                        u->health,
                        armor,
                        u->armor);
                }
            }
            else
            {
                sprintf(buf+strlen(buf),"%s\n",u->msgline);
                *u->msgline=0;
            }

#ifdef TIMERS
            // generate list of camps
            if(u->campthese && !(u->fpd&FPD_TIMERS))
            {
                uchar       *rep[64];
                int          r;
                r=0;
                for(i=0;i<u->camps;i++)
                {
                    Camp *ca;
                    int dtime;
                    char middlechar;
                    ca=&u->camp[i];
                    if((ca->category&u->campthese) && ca->appeartime)
                    {
                        int style=0,tim;
                        dtime=(int)ca->appeartime-(int)u->time;

                        if(u->remindcamp && (ca->category&(CAMP_R|CAMP_P|CAMP_Q)) && dtime>=u->remindcamp*1000-1000 && dtime<u->remindcamp*1000+1000 )
                        {
                            sprintf(u->notice,"\x10%s\x11",ca->name);
                            strxor80(u->notice);
                        }

                        if(ca->appeartime==UNKNOWNTIME || ca->appeartime==UNKNOWNTIME2)
                        {
                            style=0;
                        }
                        else if(dtime<-3000)
                        {
                            if(u->secondcamp && ca->secondcamp==0 && (ca->category&(CAMP_R|CAMP_P|CAMP_Q)))
                            {
                                ca->appeartime+=ca->delay;
                                dtime+=ca->delay;
                                ca->secondcamp++;
                                style=1;
                            }
                            else
                            {
                                ca->appeartime=0;
                                continue; // don't display
                            }
                        }
                        else
                        {
                            style=1;
                        }

                        if(ca->secondcamp) middlechar=0x0d;
                        else middlechar=0x1c;

                        tim=(dtime-123)/1000;
                        if(tim<0) tim=0;

                        if(!style)
                        {
                            sprintf(reports[r++],"%s%c?? ",ca->name,middlechar);
                        }
                        else
                        {
                            if(tim>99)
                            {
                                tim=(tim+30)/60;
                                sprintf(reports[r++],"%s%c%cm ",ca->name,middlechar,tim%10+'0');
                            }
                            else
                            {
                                if(tim<10) sprintf(reports[r++],"%s%c%c%c ",ca->name,middlechar,tim/10+'0'+0x80,tim%10+'0'+0x80);
                                else sprintf(reports[r++],"%s%c%c%c ",ca->name,middlechar,tim/10+'0',tim%10+'0');
                            }
                        }
                    }
                }

                // sort list of camps
                if(r)
                {
                    for(i=0;i<r;i++) rep[i]=reports[i];
                    qsort(rep,r,sizeof(uchar *),(qsortfunc)cmpnum);
                }

                // add camps to two camp rows
                buf1[0]=' '; buf1[1]=0;
                buf2[0]=' '; buf2[1]=0;
                {
                    int r1=0,r2=0;
                    for(i=0;i<r;i++)
                    {
                        if(rep[i][1]>='0' && rep[i][1]<='9' &&
                          (rep[i][0]=='p' || rep[i][0]=='r' || rep[i][0]=='q'))
                        {
                            if(r1<5)
                            {
                                strcat(buf1,rep[i]);
                                r1++;
                            }
                        }
                    }
                    for(i=0;i<r;i++)
                    {
                        if(rep[i][1]>='0' && rep[i][1]<='9' &&
                          (rep[i][0]=='p' || rep[i][0]=='r' || rep[i][0]=='q'))
                        {
                        }
                        else
                        {
                            if(r1+r2<5)
                            {
                                strcat(buf2,rep[i]);
                                r2++;
                            }
                        }
                    }
                }
            }
            else
#endif
            {
                *buf2=0;
                *buf1=0;
            }

            {
                uchar *d,*s;
                int   l;

                d=buf+strlen(buf);

                s=buf1;
                while(*s)
                {
                    *d++=(*s++)^0x80;
                }

                if(!u->linecenter)
                {
                    l=u->linesright-strlen(buf1)-strlen(buf2);
                    if(l<0) l=0;
                    memset(d,' ',l); d+=l;
                }

                s=buf2;
                while(*s)
                {
                    *d++=(*s++)^0x80;
                }
            }

            // check emptiness, print if not empty
            for(i=strlen(buf)-1;i>=0;i--)
            {
                if((buf[i]&127)>32) break;
            }
            if(i<=0)
            {
                buf[0]=0;
                u->overlayactive=0;
            }
            else u->overlayactive=1;

            if(*u->overlay)
            {
                mixoverlays(tmpbuf,buf,u->overlay);
                if(*tmpbuf) q_centerprint(u,tmpbuf); // something to show
            }
            else
            {
                if(*buf) q_centerprint(u,buf); // something to show
            }
        }
        else u->overlayactive=0;
    }
}

void q_server(User *u,uchar *inbuf,int inbytes)
{
    int seqthis,seqrecv;
    int bitthis,bitrecv;
    int outbytes;

    if(u->amconnected>0) u->amconnected--;
//    if((rand()&15)<9) return; // throw away packet

    u->packetserveroutsize[u->packetserversizecnt]=inbytes;
#ifdef COMPRESS
    uncompress(u,inbuf,&inbytes,1);
#endif
    u->packetserverinsize[u->packetserversizecnt]=inbytes;

    readheader(inbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);

    if(seqthis!=-1)
    {
        if(seqthis<u->server_lastthis-30)
        {
            u->outoforder30++;
        }
        if(seqthis<u->server_lastthis)
        {
            u->outoforder++;
        }
        if(seqthis<u->connectstartsequence)
        {
            logx(LOG_DEBUG,"packet out of sequence, dropped.\n");
            return;
        }
        u->server_hiddenbit=bitthis;
        u->server_lastthis=seqthis;
        u->server_lastrecv=seqrecv;
    }

    if(u->packetserversizecnt==0)
    { // every 16 packets, calc ping
        int n,i,ping=0,pingcnt=0;
        for(n=0;n<16;n++)
        {
            i=(STATNUM+u->lastpacketrecv-1-n)&(STATNUM-1);
            if(u->packetrecvtime[i])
            {
                ping+=u->packetrecvtime[i]-u->packetsendtime[i];
                pingcnt++;
            }
        }
        if(!pingcnt) ping=999;
        else ping=ping/pingcnt+1; // raw round up
        if(ping<=0) ping=1;
        u->server_ping=ping;
    }

    if(loglevel>=LOG_DEBUG) logx(LOG_DEBUG,"receive %02X %02X time %i\n",inbuf[0],inbuf[4],u->time);

    handleserver(u,inbuf,inbytes,outbuf,&outbytes);

    if(u->cnt_model>1 || u->cnt_sound>1)
    {
        q_print(u,"Error connecting to server.\n");
        outbytes=8; // remove all but header
    }
    expires_check();

    q_serverextras(u);

    if(!outbytes) return;

    if(u->noappendnow>0) u->noappendnow--;
    else
    {
        if(u->appendserverbytes)
        {
            appendpacket(outbuf,&outbytes,u->appendserver,&u->appendserverbytes);
            logx(LOG_PACKETS,"Packet APPENDED!\n");
            u->dumpsentpacket=1;
        }
    }

    writeheader(outbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);

    if(u->compress_test)
    {
#ifdef COMPRESS
        static char tmp[4096];
        int         tmpbytes;

        u->compress_server=1;

        tmpbytes=outbytes;
        memcpy(tmp,outbuf,tmpbytes);

        compress(u,tmp,&tmpbytes,1);
        c_hexdump("CTEST server compressed:",tmp,tmpbytes,u);

        u->packetserveroutsize[u->packetserversizecnt]=tmpbytes;

        uncompress(u,tmp,&tmpbytes,1);
        c_hexdump("CTEST server uncompressed:",tmp,tmpbytes,u);

        u->packetserverinsize[u->packetserversizecnt]=tmpbytes;

        u->compress_server=0;

        if(outbytes!=tmpbytes)
        {
            logx(LOG_PACKETS,"\nCTEST COMPRESS MISMATCH!\n");
        }
        logx(LOG_PACKETS,"\n");

        if(u->compress_test>1)
        {
            outbytes=tmpbytes;
            memcpy(outbuf,tmp,outbytes);
        }
#endif
    }
    else
    {
#ifdef COMPRESS
        compress(u,outbuf,&outbytes,1);
#endif
    }

    if(seqrecv!=-1)
    {
        int i;
        u->packetrecvtime[seqrecv&(STATNUM-1)]=u->time;
        u->lastpacketrecv=seqrecv&(STATNUM-1);
        u->packetchoke   [seqrecv&(STATNUM-1)]=u->choke;
        for(i=0;i<u->choke;i++)
        {
            u->packetrecvtime[(seqrecv-1-i)&(STATNUM-1)]=u->time;
            u->packetchoke   [(seqrecv-1-i)&(STATNUM-1)]=-1;
        }

        u->packetserversize[u->packetserversizecnt]=inbytes;
        u->packetserversizecnt=(u->packetserversizecnt+1)&15;
    }

    c_sendclient(u,outbuf,outbytes);

    if(u->cnt_model>1 || u->cnt_sound>1)
    {
        u->cnt_model=u->cnt_sound=0;
        q_command(u,"!disconnect");
    }
}

void q_client(User *u,uchar *inbuf,int inbytes)
{
    int seqthis,seqrecv;
    int bitthis,bitrecv;
    int outbytes,simple;
#ifdef COMPRESS
    int inbytes0=inbytes;
#endif /* COMPRESS */

//    if((rand()&15)<9) return; // throw away packet

#ifdef COMPRESS
    uncompress(u,inbuf,&inbytes,0);
#endif

    readheader(inbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);

    if(u->cpps)
    {
        if(u->time<u->time_nextcpps && !bitthis)
        { // drop packet
            u->cppsdrop++;
            return;
        }
        u->cppspass++;
        // accept packet, calc ok arrival for next packet
        u->time_nextcpps+=u->time_cppsdelta;
        if(u->time_nextcpps<u->time)
        {
            u->time_nextcpps=u->time+u->time_cppsdelta;
        }
    }

    if(seqthis!=-1)
    {
        u->sequence=seqthis+1;
        u->client_lastthis=seqthis;
        u->client_lastrecv=seqrecv;

        u->packetsendtime[seqthis&(STATNUM-1)]=u->time;
        u->packetrecvtime[seqthis&(STATNUM-1)]=0;
        u->packetchoke   [seqthis&(STATNUM-1)]=0;

        u->packetclientsize[u->packetclientsizecnt]=inbytes;
        u->packetclientsizecnt=(u->packetclientsizecnt+1)&15;
    }

#ifdef IDENT
    if(u->clearrequest)
    {
        sendinfomessage(u,"\"\"");
        u->clearrequest=0;
    }
#endif

    simple=handleclient(u,inbuf,inbytes,outbuf,&outbytes);
    if(u->appendclientbytes>0) simple=0;
    // simple=only movement & sequence in packet

    if(u->noappendnow>0) u->noappendnow--;
    else
    {
        if(u->appendclientbytes)
        {
            appendpacket(outbuf,&outbytes,u->appendclient,&u->appendclientbytes);
            logx(LOG_PACKETS,"Packet APPENDED!\n");
            u->dumpsentpacket=1;
        }
    }

    if(u->fakedisablecnt>0) u->fakedisablecnt--;
    else if(u->fakeping)
    {
        u->fakedisablecnt=0;
        if(u->fakeping<=-50)
        {
            seqrecv=u->server_lastthis-(u->fakeping+100);
        }
        else
        {
            seqrecv-=u->fakeping;
        }
    }

    writeheader(outbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);

    if(u->compress_test)
    {
#ifdef COMPRESS
        static char tmp[4096];
        int         tmpbytes;

        u->compress_client=1;

        tmpbytes=outbytes;
        memcpy(tmp,outbuf,tmpbytes);

        compress(u,tmp,&tmpbytes,0);
        c_hexdump("CTEST client compressed:",tmp,tmpbytes,u);

        u->packetclientoutsize[u->packetclientsizecnt]=tmpbytes;

        uncompress(u,tmp,&tmpbytes,0);
        c_hexdump("CTEST client uncompressed:",tmp,tmpbytes,u);

        u->packetclientinsize[u->packetclientsizecnt]=tmpbytes;

        u->compress_client=0;

        if(outbytes!=tmpbytes)
        {
            logx(LOG_PACKETS,"\nCTEST COMPRESS MISMATCH!\n");
        }
        logx(LOG_PACKETS,"\n");

        if(u->compress_test>1)
        {
            outbytes=tmpbytes;
            memcpy(outbuf,tmp,outbytes);
        }
#endif
    }
    else
    {
        u->packetclientinsize[u->packetclientsizecnt]=outbytes;
#ifdef COMPRESS
        compress(u,outbuf,&outbytes,0);
#endif
        u->packetclientoutsize[u->packetclientsizecnt]=outbytes;
    }

    c_sendserver(u,outbuf,outbytes);

    logx(LOG_DEBUG,"send    %02X %02X time %i\n",outbuf[0],outbuf[4],u->time);

    if(u->state==STATE_DROPPING)
    {
        c_sendserver(u,outbuf,outbytes);
        c_sendserver(u,outbuf,outbytes);
        // send thanks
        outbytes=8;
        seqthis=u->sequence+1;
        bitthis=0;
        writeheader(outbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);
        if(u->noappendnow>0) u->noappendnow--;
        else
        {
            if(u->appendserverbytes)
            {
                appendpacket(outbuf,&outbytes,u->appendserver,&u->appendserverbytes);
                logx(LOG_PACKETS,"Packet APPENDED!\n");
                u->dumpsentpacket=1;
            }
        }
        c_sendclient(u,outbuf,outbytes);
    }
}

void q_proxy(User *u,uchar *inbuf,int inbytes)
{
    int seqthis,seqrecv;
    int bitthis,bitrecv;
    int outbytes;

    // handle input client packet

    u->amconnected=0;
    u->server_ping=0;

    uncompress(u,inbuf,&inbytes,0);

    readheader(inbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);

    if(seqthis!=-1)
    {
        u->sequence=seqthis+1;

        u->client_lastthis=seqthis;
        u->client_lastrecv=seqrecv;

        if(bitthis) u->server_hiddenbit^=0x80;
        u->client_hiddenbit=bitrecv;
    }

    if(u->state==STATE_NAMELOOKUP && u->time>=u->time_namelookupmsg)
    {
        q_print(u,"performing namelookup...\n");
        u->time_namelookupmsg=u->time+2000;
    }

    handleclient(u,inbuf,inbytes,outbuf,&outbytes);

    // create output server packet

    outbytes=8;
    appendpacket(outbuf,&outbytes,u->appendserver,&u->appendserverbytes);

    seqthis=u->sequence;
    bitrecv=u->server_hiddenbit;
    bitthis=0;
    if(u->server_sendsecure)
    {
        bitthis=0x80;
        u->server_sendsecure=0;
    }

    if(seqthis!=-1)
    {
        u->server_lastthis=seqthis;
        u->server_lastrecv=seqrecv;
    }

    writeheader(outbuf,&seqthis,&seqrecv,&bitthis,&bitrecv);

    compress(u,outbuf,&outbytes,1);

    u->dumpsentpacket=1;
    c_sendclient(u,outbuf,outbytes);
}

void q_connect(uchar *inbuf,int inbytes,SOCKADDR_IN *from)
{
    uchar  buf[MAXSTR];
    User  *u;
    if(!memicmp(inbuf+4,"getchal",7) )
    { // challenge to proxy
        u=c_finduser(from);
        if(!u)
        { // new user
            u=c_newuser(from);
            if(!u)
            { // server full
                c_textaddress(buf,from);
                logx(LOG_CONSOLE,"Proxy full (connect attempt from %s)\n",buf);
                // send connect reply
                buf[0]=255;
                buf[1]=255;
                buf[2]=255;
                buf[3]=255;
                strcpy(buf+4,"nproxy is full\n");
                c_sendto(from,buf,strlen(buf));
                return;
            }
            else
            {
                strcpy(u->name,"<challenge>");
                c_textaddress(buf,from);
                logx(LOG_VERBOSE,"Challenge: from %s (total %i users)\n",buf,usernum);
            }
        }
        else
        {
            //c_textaddress(buf,from);
            //logx(LOG_VERBOSE,"Dup connect ignored: %s from %s\n",u->name,buf);
            return;
        }
        challenge+=7;
        // send connect reply
        buf[0]=255;
        buf[1]=255;
        buf[2]=255;
        buf[3]=255;
        sprintf(buf+4,"c%i",challenge);
        c_sendclient(u,buf,strlen(buf));
        u->state=STATE_DROPPING; // this connection no inter needed
    }
    else if(!memicmp(inbuf+4,"status",6) || !memicmp(inbuf+4,"ping",4) || inbuf[4]=='k')
    { // status / ping
        int i,isping=memicmp(inbuf+4,"status",6);
        u=c_finduser(from);
        if(!u)
        { // new user
            u=c_newuser(from);
            if(!u)
            {
                return;
            }
            else
            {
                strcpy(u->name,"<status>");
                c_textaddress(buf,from);
                logx(LOG_VERBOSE,"Status: from %s\n",buf);
            }
        }
        // generate hostname
        if(!*proxy_name)
        {
            gethostname(proxy_name,255);
            strlwr(proxy_name);
        }
        // send connect reply
        buf[0]=255;
        buf[1]=255;
        buf[2]=255;
        buf[3]=255;
        if(isping)
        {
            c_sendclient(u,buf,4);
        }
        else
        {
            int min,ping;
            sprintf(buf+4,"n\\support\\QW 2.1,2.2,2.3\\info\\QuakeWorld proxy\\maxclients\\%i\\*version\\%s\\hostname\\%s - %s\n",
                    maxusernum,version,versionname,proxy_name);
            for(i=0;i<usernum;i++) if(u!=user+i)
            {
                ping=user[i].server_ping;
                ping=0;
                min=(user[i].time-user[i].time_connect)/1000/60;
                c_textaddress(tmpbuf,&user[i].servermainaddr);
                if(user[i].state==STATE_HERE) strcpy(tmpbuf,"idle");
                sprintf(buf+strlen(buf),"0 0 %i %i \"%s\" \"%s\" %i %i\n",
                    min,ping,user[i].name,tmpbuf,user[i].tcolor,user[i].bcolor);
            }
            c_sendclient(u,buf,strlen(buf)+1);
        }
        u->state=STATE_DROPPING; // this connection no inter needed
    }
    else if(!memicmp(inbuf+4,"connect ",8))
    { // connect to proxy
        c_textaddress(buf,from);
        u=c_finduser(from);

        if(!u)
        { // new user
            if(usernum>=maxusernum) u=NULL;
            else u=c_newuser(from);
            if(!u)
            { // server full
                logx(LOG_CONSOLE,"Proxy full (connect attempt from %s)\n",buf);
                // send connect reply
                buf[0]=255;
                buf[1]=255;
                buf[2]=255;
                buf[3]=255;
                strcpy(buf+4,"nproxy is full\n");
                c_sendto(from,buf,strlen(buf));
                u->state=STATE_DROPPING; // this connection no inter needed
                return;
            }
            else
            {
                int deny=0;
                uchar *p;
                p=inbuf+4+8;
                if(*p=='\"')
                {
                    u->version=21;
                    strcpy(u->clientinfo,p);
                }
                else
                {
                    while(*p<=32 && *p) p++;
                    u->response_a=atoi(p);
                    while(*p>32) p++;
                    while(*p<=32 && *p) p++;
                    u->response_b=atoi(p);
                    while(*p>32) p++;
                    while(*p<=32 && *p) p++;
                    u->response_c=atoi(p);
                    while(*p>32) p++;
                    while(*p<=32 && *p) p++;

                    if(u->response_a<=26) u->version=22;
                    else if(u->response_a<=27) u->version=23;
                    else u->version=24;

                    strcpy(u->clientinfo,p);
                }

                extractfield(tmpbuf,u->clientinfo,"topcolor\\");
                u->tcolor=atoi(tmpbuf);
                extractfield(tmpbuf,u->clientinfo,"bottomcolor\\");
                u->bcolor=atoi(tmpbuf);
                extractfield(u->name,u->clientinfo,"name\\");

                extractfield(tmpbuf,u->clientinfo,"password\\");

                if(*proxy_password && strcmp(tmpbuf,proxy_password))
                {
                    logx(LOG_CONSOLE,"Connection not allowed (wrong password from %s)\n",buf);
                    // send connect reply
                    buf[0]=255;
                    buf[1]=255;
                    buf[2]=255;
                    buf[3]=255;
                    strcpy(buf+4,"nincorrect password. connection not allowed.\n");
                    c_sendto(from,buf,strlen(buf));
                    u->state=STATE_DROPPING; // this connection no inter needed
                    return;
                }

                deny=0;
                if(*proxy_allowip && memicmp(buf,proxy_allowip,strlen(proxy_allowip))) deny=1;
                if(proxy_denyipnum)
                {
                    int i;
                    for(i=0;i<proxy_denyipnum;i++)
                    {
                        if(memicmp(buf,proxy_denyip[i],strlen(proxy_denyip[i])))
                        {
                            deny=1;
                            break;
                        }
                    }
                }
                if(deny)
                {
                    logx(LOG_CONSOLE,"Connection not allowed (connect attempt from %s)\n",buf);
                    // send connect reply
                    buf[0]=255;
                    buf[1]=255;
                    buf[2]=255;
                    buf[3]=255;
                    strcpy(buf+4,"nconnection not allowed from your ip address\n");
                    c_sendto(from,buf,strlen(buf));
                    u->state=STATE_DROPPING; // this connection no inter needed
                    return;
                }

                u->showtime=0;
                u->campthese=0;
//                u->reportthese=0;
//                u->pickupthese=0;
                u->time_nextcamp=0;
                u->linesdown=11;
                u->linesright=36;
                u->time_namelookupmsg=0;
                u->sequence=0;

                logx(LOG_CONSOLE,"Connect: %s from %s (ver %i, users %i)\n",u->name,buf,u->version,usernum);
            }
        }
        else
        {
            //logx(LOG_VERBOSE,"Dup connect ignored: %s from %s\n",u->name,buf);
            return;
        }
        // send connect reply
        buf[0]=255;
        buf[1]=255;
        buf[2]=255;
        buf[3]=255;
        buf[4]='j';
        c_sendclient(u,buf,5);
        u->state=STATE_INIT;
    }
    else
    {
        // skip
        if(loglevel>=LOG_DEBUG)
        {
            c_textaddress(buf,from);
            logx(LOG_DEBUG,"Unrecognized connect attempt from %s.\n",buf);
        }
    }
}

