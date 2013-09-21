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

//-------------------------------------------------------
void rankchange(User *u,int pl,int dooverride)
{
    int newcol;
    int frags,ratio,time;
    int best,besti,bestratio,besttime;
    int i;

    if(pl<1 || pl>32) return;

    // find leader
    best=0;
    besti=0;
    for(i=1;i<=32;i++)
    {
        if(u->playerfrags[i]>best)
        {
            besti=i;
            best=u->playerfrags[i];
        }
    }
    u->playerfragsbest=best;

    if(!besti)
    {
        besttime=0;
        bestratio=1;
    }
    else
    {
        i=u->playertime[besti]+u->time;
        if(i<1000) i=1000;
        besttime=i;
        bestratio=1000000*best/i;
    }

    frags=u->playerfrags[pl];

    i=u->playertime[pl]+u->time;
    if(i<1000) i=1000;
    time=i;
    ratio=1000000*frags/i;

//    printf("pl %2i: time %6i frags %3i ratio %6i  (best %3i ratio %6i time %6i)\n",
//        pl,i,frags,ratio,best,bestratio,besttime);

    if(frags<=0 || time<60000)    newcol=0;  // <=0 frags or <=1 min (WHITE)
    else if(frags>=best || ratio>=bestratio) newcol=4 ; // leader(s)      RED
    else if(ratio>=bestratio*3/4) newcol=12; // ratio over 75% YELLOW
    else if(ratio>=bestratio*2/4) newcol=13; // ratio over 50% BLUE
    else if(ratio>=bestratio*1/4) newcol=3 ; // ratio over 25% GREEN
    else                          newcol=0 ; // the rest       WHITE

    if(newcol!=u->playerrcolor[pl])
    {
        u->playerrcolor[pl]=newcol;
        if(dooverride) skinoverride(u,pl);
    }
}

void skinoverride(User *u,int i)  // also coloroverride
{
    char *skin = NULL;
    char *color = NULL,*color2 = NULL;
    char  colortext[4];
    char  colortext2[4];
    //char buf2[32],*p2;
    //char buf[80],*p;

    skin=NULL;

/*
    // check and cut long skin names
    if(strlen(u->playerskin[i])>19)
    {
        u->playerskin[i][19]=0;
        skin=u->playerskin[i];
        q_printf(u,"DEBUG: %s longskin\n",u->playername[i]);
    }
*/

    for(;;)
    {
        if(u->fortress) break;
        if(u->fpd&FPD_SKIN) break;
        if(!u->skinoverride) break;
        if(!u->playeractive[i]) break;
        if(u->time_reskinall) break;

        skin=(SAMETEAM(i))?u->skinname:u->skinname2;
        if(*skin=='*' || u->skinoverride==-1) skin=u->playerskin[i];

        /*
        p=u->playerskin[i];
        p2=buf2;
        while(*p)
        {
            if(*p=='.') break;
            *p2++=*p++;
            if(p2>buf2+15) break;
        }
        *p2=0;
        */

        if(u->skinoverride==1)
        {
//            q_printf(u,"DEBUG: %15s %s\n",u->playername[i],skin);
        }

        break;
    }

    if(skin)
    {
        setinfo(u,i,"skin",skin);
    }

    //-------- color ------------

    color=NULL;

    for(;;)
    {
        if(u->fortress) break;
        if(u->fpd&FPD_COLOR) break;
        if(!u->coloroverride) break;
        if(!u->playeractive[i]) break;
        if(u->time_reskinall) break;

        {
            int c;
            if(SAMETEAM(i)) c=u->color;
            else            c=u->color2;

            if(c==-3)
            {
                color=color2=colortext;
                if(u->playerrcolor[i]<0) *color='-';
                else sprintf(colortext,"%i",u->playerrcolor[i]);
            }
            else if(c==-2)
            {
                color=color2=colortext;
                if(u->playerxcolor[i]<0) *color='-';
                else *color='+';
            }
            else if(c<0)
            {
                color=color2=colortext;
                *color='-';
            }
            else if(c<256)
            {
                color=color2=colortext;
                sprintf(color,"%i",c&15);
            }
            else
            {
                color=colortext;
                color2=colortext2;
                sprintf(colortext,"%i",(c>>4)&15);
                sprintf(colortext2,"%i",c&15);
            }
        }

        if(u->coloroverride==-1 || *color=='-')
        {
            color=colortext;
            color2=colortext2;
            sprintf(colortext,"%i",u->playertcolor[i]);
            sprintf(colortext2,"%i",u->playerbcolor[i]);
        }
        else if(*color=='+')
        {
            color=colortext;
            color2=colortext2;
            sprintf(colortext,"%i",(u->playerxcolor[i]>>4)&15);
            sprintf(colortext2,"%i",(u->playerxcolor[i])&15);
        }

        break;
    }

    if(color)
    {
        setinfo(u,i,"topcolor",color);
        setinfo(u,i,"bottomcolor",color2);
    }
}

//-------------------------------------------------------

uchar *findnear(User *u,int x,int y,int z,int *ind)
{
    int i,ibest=-1;
    int d,dbest=9999999;
    for(i=0;i<u->locations;i++)
    {
        d=abs(x-u->loc[i].x)+
          abs(y-u->loc[i].y)+
          abs(z-u->loc[i].z);
        if(d<dbest)
        {
            dbest=d;
            ibest=i;
        }
    }
    if(ind) *ind=ibest;
    if(ibest<0) return("someplace");
    else return(u->loc[ibest].name);
}

char *playerlocdirection(User *u,int pl)
{
    char *name1,*name2;
    static char buf[512];
    int i;

    i=u->playerlastloc[pl];
    if(i<0 || i>=u->locations) name1="";
    else name1=u->loc[i].name;

    i=u->playerloc[pl];
    if(i<0 || i>=u->locations) name2="";
    else name2=u->loc[i].name;

    if(strcmp(name1,name2))
    {
        strcpy(buf,name1);
        strcat(buf,"-\x8d");
        strcat(buf,name2);
        return(buf);
    }
    else return(NULL);
}

// pointing globals
static float px,py,pz; // player
static float dx,dy,dz; // lookdir
static float best;
static int   showdebug;

int pointcheck(User *u,int x,int y,int z,char *name)
{ // returns 1 if this object should replace last
    float jx,jy,jz;
    float kx,ky,kz;
    float zdist; // depth distance
    float pdist; // perpendicular distance
    float dist;  // dist used for evaluation

    jx=(float)x-px;
    jy=(float)y-py;
    jz=(float)z-pz;
    zdist=(jx*dx+jy*dy+jz*dz);

    if(zdist<100) return(0);

    kx=dx*zdist-jx;
    ky=dy*zdist-jy;
    kz=dz*zdist-jz;
    pdist=(float)sqrt(kx*kx+ky*ky+kz*kz);

    if(pdist>2000) return(0);

    if(pdist>zdist) return(0); // aim over 45 degrees off

    if(zdist<3000)
    {
        dist=pdist*(zdist*0.0002f+0.3f);
    }
    else dist=pdist;

    if(showdebug)
    {
        char buf[64];
        sprintf(buf,"- z:%5.0f p:%5.0f d:%5.0f %s\n",zdist,pdist,dist,name);
        q_print(u,buf);
    }

    if(dist<best)
    {
        best=dist;
        return(1);
    }
    return(0);
}

uchar *findpoint(User *u,int x,int y,int z,int ax,int ay,int az,int targetmask,int flags,int *entnum)
{
    static char buf[256];
    uchar *name;
    int    namex = 0,namey = 0,namez = 0;
    int    gotplayer=0; // 1=teammate
    int    gotplayernum=0;
    int    i;

    if(entnum) *entnum=0;

    // parameters for pointcheck
    dx=(float)(cos((double)ay/32768.0*3.141592)* cos((double)ax/32768.0*3.141592));
    dy=(float)(sin((double)ay/32768.0*3.141592)* cos((double)ax/32768.0*3.141592));
    dz=(float)(                                 -sin((double)ax/32768.0*3.141592));
    px=(float)x;
    py=(float)y;
    pz=(float)z;
    best=99999999.0f;
    showdebug=(targetmask==-1);

    // result clear
    *buf=0;
    name=NULL;

    if(flags&POINT_SELF)
    {
        gotplayernum=u->entity;
        gotplayer=1;
        name="self";
        namex=u->playerpos[gotplayernum][0];
        namey=u->playerpos[gotplayernum][1];
        namez=u->playerpos[gotplayernum][2];
    }
    else
    {
        if(targetmask&(CAMP_PLAYER|CAMP_ENEMY))
        { // players
            for(i=1;i<=32;i++) if(u->playeractive[i] && u->playernear[i] && i!=u->entity)
            {
                if(pointcheck(u,u->playerpos[i][0],u->playerpos[i][1],u->playerpos[i][2],"player"))
                {
                    int lo,hi;

                    *buf=0;

                    namex=u->playerpos[i][0];
                    namey=u->playerpos[i][1];
                    namez=u->playerpos[i][2];

                    lo=u->playerglow[i]&0x0f;
                    hi=u->playerglow[i]&0xf0;

                    if(u->playerglow[i]&0x100)
                    {
                        if(hi&0x40)
                        {
                            strcat(buf,"quaded ");
                        }
                        if(hi&0x80)
                        {
                            strcat(buf,"penta ");
                        }

                        strcat(buf,"eyes");
                        gotplayer=3;
                    }
                    else
                    {
                        if(hi&0x10) strcat(buf,"red FLAG on ");
                        else if(hi&0x20) strcat(buf,"blue FLAG on ");

                        if(hi&0x40)
                        {
                            strcat(buf,"quaded ");
                        }
                        if(hi&0x80)
                        {
                            strcat(buf,"penta ");
                        }

                        if(SAMETEAM(i))
                        {
                            strcat(buf,u->playername[i]);
                            gotplayer=1;
                        }
                        else
                        {
                            strcat(buf,"enemy");
                            if((flags&POINT_ADDSKIN) && u->fortress)
                            {
                                char *s;
                                s=u->playerskin[i];
                                if(s[0]=='t' && s[1]=='f' && s[2]=='_')
                                {
                                    s+=3;
                                    if(!strcmp(s,"demo.pcx")) strcat(buf," demoman");
                                    if(!strcmp(s,"eng.pcx")) strcat(buf," engineer");
                                    if(!strcmp(s,"hwguy.pcx")) strcat(buf," hw-guy");
                                    if(!strcmp(s,"medic.pcx")) strcat(buf," medic");
                                    if(!strcmp(s,"pyro.pcx")) strcat(buf," pyro");
                                    if(!strcmp(s,"scout.pcx")) strcat(buf," scout");
                                    if(!strcmp(s,"snipe.pcx")) strcat(buf," sniper");
                                    if(!strcmp(s,"sold.pcx")) strcat(buf," soldier");
                                    if(!strcmp(s,"spy.pcx")) strcat(buf," spy");
                                }
                            }
                            gotplayer=2;
                        }
                    }

                    if(entnum) *entnum=i;

                    name=buf;
                    gotplayernum=i;
                }
            }
        }

        if(gotplayer==1 && !(targetmask&CAMP_PLAYER))
        { // got own player, and only CAMP_ENEMY set
            gotplayer=0;
            name=NULL;
            best=99999999.0f;
        }

        if(targetmask&(~(CAMP_PLAYER|CAMP_ENEMY)))
        {
            for(i=0;i<MAXENTITIES;i++) if(u->entityvisible[i] && (u->entitymask[i]&targetmask))
            {
                if(pointcheck(u,u->entitypos[i][0],u->entitypos[i][1],u->entitypos[i][2],u->entityname[i]))
                {
                    gotplayer=0;
                    namex=u->entitypos[i][0];
                    namey=u->entitypos[i][1];
                    namez=u->entitypos[i][2];
                    name=u->entityname[i];
                    if(entnum) *entnum=i;
                }
            }
        }
    }

    if(flags&POINT_TEAMMATENUMBER)
    {
        sprintf(buf,"%i",gotplayernum);
        return(buf);
    }
    else if(name==NULL)
    {
        // do nothing
        return("."); // nothing found
    }
    else
    {
        // add location
        if(flags&POINT_ONLYNAME)
        {
            strcpy(buf,name);
        }
        else if(flags&POINT_ONLYPOS)
        {
            strcpy(buf,findnear(u,namex,namey,namez,NULL));
        }
        else
        {
            strcpy(buf,name);
            strcat(buf," at ");
            if(gotplayer && (flags&POINT_DIRECTION))
            {
                char *p;
                p=playerlocdirection(u,gotplayernum);
                if(p) strcat(buf,p);
                else strcat(buf,findnear(u,namex,namey,namez,NULL));
            }
            else
            {
                strcat(buf,findnear(u,namex,namey,namez,NULL));
            }
        }

        return(buf);
    }
}

uchar *findnearestitem(User *u,int x,int y,int z,int targetmask)
{ // onlynamepos: 1=name, 2=pos
    static char buf[256];
    char *name;
    int   namex,namey,namez;
    int   ix,iy,iz,i;
    float jx,jy,jz;
    float dist;
    float namedist=50000.0f;
#if !defined(__APPLE__) && !defined(MACOSX)
    int   gotplayer=0; // 1=teammate
    int   gotplayernum=0;
    float distlimit=10000.0f,zdistlimit=3500.0f;
#endif /* !__APPLE__ && !MACOSX */

    *buf=0;
    name=NULL;
    namex=0;
    namey=0;
    namez=0;

    for(i=0;i<MAXENTITIES;i++) if(u->entityvisible[i] && (u->entitymask[i]&targetmask))
    {
        ix=u->entitypos[i][0]; jx=(float)(ix-x);
        iy=u->entitypos[i][1]; jy=(float)(iy-y);
        iz=u->entitypos[i][2]; jz=(float)(iz-z);
        dist=(float)sqrt(jx*jx+jy*jy+jz*jz);

        if(dist<namedist)
        {
            namedist=dist;
            namex=ix;
            namey=iy;
            namez=iz;
            name=u->entityname[i];
        }
    }

    if(name==NULL)
    {
        strcpy(buf,".");
        return(buf);
    }
    else
    {
        strcat(buf,name);
        strcat(buf," at ");
        strcat(buf,findnear(u,namex,namey,namez,NULL));

        return(buf);
    }
}


//-------------------------------------------------------

#define SHOWINFOMESSAGES

void userinfofromuser(Userinfo *ui,User *u)
{
    ui->own=1;
    ui->name=u->name;

    ui->armor=u->armor;
    ui->health=u->health;

    if(0) { }
    else if(u->weapon&1) {
        ui->weapon=2-1;
        ui->ammo=u->ammo_shells;
    }
    else if(u->weapon&2) {
        ui->weapon=3-1;
        ui->ammo=u->ammo_shells;
    }
    else if(u->weapon&4) {
        ui->weapon=4-1;
        ui->ammo=u->ammo_nails;
    }
    else if(u->weapon&8) {
        ui->weapon=5-1;
        ui->ammo=u->ammo_nails;
    }
    else if(u->weapon&16) {
        ui->weapon=6-1;
        ui->ammo=u->ammo_rockets;
    }
    else if(u->weapon&32) {
        ui->weapon=7-1;
        ui->ammo=u->ammo_rockets;
    }
    else if(u->weapon&64) {
        ui->weapon=8-1;
        ui->ammo=u->ammo_cells;
    }
    else
    {
        ui->weapon=1-1;
        ui->ammo=0;
    }

    if(0) { }
    else if((u->items&32) && u->ammo_rockets>=0) {
        ui->bestweapon=7-1;
        ui->bestammo=u->ammo_rockets;
    }
    else if((u->items&64) && u->ammo_cells>=0) {
        ui->bestweapon=8-1;
        ui->bestammo=u->ammo_cells;
    }
    else if((u->items&16) && u->ammo_rockets>=0) {
        ui->bestweapon=6-1;
        ui->bestammo=u->ammo_rockets;
    }
    else if((u->items&8) && u->ammo_nails>=0) {
        ui->bestweapon=5-1;
        ui->bestammo=u->ammo_nails;
    }
    else if((u->items&4) && u->ammo_nails>=0) {
        ui->bestweapon=4-1;
        ui->bestammo=u->ammo_nails;
    }
    else if((u->items&2) && u->ammo_shells>=0) {
        ui->bestweapon=3-1;
        ui->bestammo=u->ammo_shells;
    }
    else if((u->items&1) && u->ammo_shells>=0) {
        ui->bestweapon=2-1;
        ui->bestammo=u->ammo_shells;
    }
    else
    {
        ui->bestweapon=1-1;
        ui->bestammo=0;
    }

    ui->items=u->items;
    ui->power=u->power;

    ui->x=u->pos[0];
    ui->y=u->pos[1];
    ui->z=u->pos[2];
}

void sendsetinfo(User *u,uchar *msg)
{
    uchar *p;

    sprintf(tmpbuf,"setinfo %s",msg);
    u->appendclient[u->appendclientbytes++]=0x04;
    p=tmpbuf; while(*p) u->appendclient[u->appendclientbytes++]=*p++;
    u->appendclient[u->appendclientbytes++]=0;
}

void strxor80(char *t)
{
    while(*t)
    {
        if((*t&0x7f)>=0x20)
        {
            *t^=0x80;
        }
        t++;
    }
}

void stror80(char *t)
{
    while(*t)
    {
        if((*t&0x7f)>=0x20)
        {
            *t|=0x80;
        }
        t++;
    }
}

int memcmp7f(char *a,char *b,int len)
{
    while(len--)
    {
        if((*a&0x7f)!=(*b&0x7f)) return(1);
        a++;
        b++;
    }
    return(0);
}

void strand80(char *t)
{
    while(*t)
    {
        if((*t&0x7f)>=0x20)
        {
            *t&=~0x80;
        }
        t++;
    }
}

int teamint(uchar *a)
{
    unsigned int b,x,i;
    b=0;
    for(i=0;i<16;i++)
    {
        x=*a++;
        if(!x) return(b);

        b=(b<<8)|(b>>24);
        b^=x;
    }
    return(b);
}

int is_version(uchar *x)
{
    x--;
    if(x[1]=='_' && x[3]=='e' && x[2]=='v' &&
       x[4]=='r' && x[5]=='s' && x[6]=='i' && x[7]=='o' && x[8]=='n')
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

int is_timer(uchar *x)
{
    x--;
    if(x[1]=='_' && x[3]=='i' && x[2]=='t' &&
       x[4]=='m' && x[5]=='e' && x[6]=='r')
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

int is_modified(uchar *x)
{
    x--;
    if(x[1]=='_' && x[3]=='o' && x[2]=='m' &&
       x[4]=='d' && x[5]=='i' && x[6]=='f' && x[7]=='i' && x[8]=='e' && x[9]=='d')
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

void createversiontext(User *u,uchar *txt)
{
    int flags=0;
#ifdef PROMODE
    flags|=0x01;
#endif
    if(u->nofversion) flags|=2;
    if(u->campthese) flags|=0x10;
    if(u->campthese&(~CAMP_POWER)) flags|=0x20;
    sprintf(txt+4,"c%s (%02X)",version,flags);
    txt[0]='s';
    txt[1]='a';
    txt[2]='y';
    txt[3]=' ';
}

int weaponok(User *u,int weapon)
{
    switch(weapon)
    {
    case 1: return(1);
    case 2: return( (u->items & 1 ) && u->ammo_shells >=1 );
    case 3: return( (u->items & 2 ) && u->ammo_shells >=2 );
    case 4: return( (u->items & 4 ) && u->ammo_nails  >=1 );
    case 5: return( (u->items & 8 ) && u->ammo_nails  >=1 );
    case 6: return( (u->items & 16) && u->ammo_rockets>=1 );
    case 7: return( (u->items & 32) && u->ammo_rockets>=1 );
    case 8: return( (u->items & 64) && u->ammo_cells  >=1 );
    default: return(1);
    }
}

int modifyimpulse(User *u,int *impulse)
{
    int i,i0,seq,new,j;
    i=i0=*impulse;

    for(seq=0;seq<SEQUENCES;seq++)
    {
        if(i0==u->impulseorder[seq][0])
        {
            for(j=1;j<14;j++)
            {
                new=u->impulseorder[seq][j];
                if(!new) break;
                if(weaponok(u,new)) i=new;
            }

            if(i!=i0)
            {
                *impulse=i;
                return(1);
            }
            else return(0);
        }
    }

    if(u->bestweapon)
    {
        if(i0==4 && (u->items & 4 ) && u->ammo_nails  >=1) i=4;
        else if(i0>8) i=i0;
        else if(i0==8 && (u->items & 64) && u->ammo_cells  >=1) i=8;
        else if(i0==7 && (u->items & 32) && u->ammo_rockets>=1) i=7;
        else if(i0==6 && (u->items & 16) && u->ammo_rockets>=1) i=6;
        else if(i0>=5 && (u->items & 8 ) && u->ammo_nails  >=1) i=5;
        else if(i0>=3 && (u->items & 2 ) && u->ammo_shells >=2) i=3;
        else if(i0>=4 && (u->items & 4 ) && u->ammo_nails  >=1) i=4;
        else if(i0>=2 && (u->items & 1 ) && u->ammo_shells >=1) i=2;
        else i=i0; // if axe just use the original impulse

        if(i!=i0)
        {
            *impulse=i;
            return(1);
        }
        else return(0);
    }

    return(0);
}

void setinfo(User *u,int i,char *attr,char *value)
{
    char *p;
    u->appendserver[u->appendserverbytes++]=0x33;
    u->appendserver[u->appendserverbytes++]=i-1;
    p=attr;
    for(;;)
    {
        u->appendserver[u->appendserverbytes++]=*p;
        if(!*p) break;
        p++;
    }
    p=value;
    for(;;)
    {
        u->appendserver[u->appendserverbytes++]=*p;
        if(!*p) break;
        p++;
    }
}

//-------------------------------------------------------

void spectatenext(User *u,uchar *findname)
{ // spectator change
    int i,j;

    if(findname)
    {
        uchar buf1[32],buf2[32];
        memset(buf1,0,32);
        memset(buf2,0,32);
        strncpy(buf1,findname,31);
        strlwr(buf1);

        for(i=0;i<u->numplayers;i++)
        {
            u->specentity++;
            if(u->specentity>31) u->specentity=1;

            if(u->playeractive[u->specentity] && u->specentity!=u->entity)
            { // player with this index exists!
                strncpy(buf2,u->playername[u->specentity],31);
                for(j=strlen(buf2)-1;j>=0;j--) buf2[j]&=0x7f;
                strlwr(buf2);

                if(strstr(buf2,buf1))
                {
                    break;
                }
            }
        }
        if(i>=33)
        {
            u->specentity=0;
            return;
        }
    }
    else
    {
        for(i=0;i<u->numplayers;i++)
        {
            u->specentity++;

            if(u->specentity>31) u->specentity=1;
            if(u->playeractive[u->specentity] && u->specentity!=u->entity)
            { // player with this index exists!
                break;
            }
        }
        if(i>=33)
        {
            u->specentity=0;
            sprintf(tmpbuf,"Nobody to track!\n");
            q_print(u,tmpbuf);
            return;
        }
    }

    sprintf(tmpbuf,"ptrack %i",u->specentity-1);
    q_servercmd(u,tmpbuf);

    u->time_nextcamp=0; // update text
}

void cl_clearstats(User *u)
{
    u->health=100;
    u->armor=0;
    u->ammo=0;
    u->ammo_shells=0;
    u->ammo_nails=0;
    u->ammo_rockets=0;
    u->ammo_cells=0;
    u->weapon=1;
    u->items=1;
}

void cl_setinfo(User *u,uchar *attr,uchar *value)
{
    uchar *p,*s2,*d2;
    int   found=0;
    static char tmpbuf[CLIENTINFOSIZE];

    logx(LOG_DEBUG,"setinfo %s to %s\n",attr,value);

    if(!strcmp(attr,"name"))
    {
        uchar buf[80];
        if(strcmp(u->name,value))
        {
            sprintf(buf,"%s is now %s\n",u->name,value);
            strncpy(u->name,value,MAXSTR-1);
            q_print(u,buf);
        }
    }
    if(!strcmp(attr,"team"))
    {
        u->team=teamint(value);
    }
    if(!strcmp(attr,"topcolor"))
    {
        u->tcolor=atoi(value);
    }
    if(!strcmp(attr,"bottomcolor"))
    {
        u->bcolor=atoi(value);
    }

    memcpy(tmpbuf,u->clientinfo,CLIENTINFOSIZE);
    d2=u->clientinfo;
    s2=tmpbuf;
    if(*s2) *d2++=*s2++;
    if(*s2) *d2++=*s2++;
    while(*s2)
    {
//        logx(LOG_DEBUG,"Compare <%s> to %s",attr,s2);
        if(*s2=='\"')
        { // end of list
            if(d2[-1]=='\\') d2--;
            if(!found && *value)
            {
                sprintf(d2,"\\%s\\%s\"\n",attr,value);
                d2+=strlen(d2);
            }
            else
            {
                *d2++='"';
                *d2++='\n';
                *d2++=0;
            }
            break;
        }
        else if(!memicmp(attr,s2,strlen(attr)))
        { // matching attribute
            if(*value==0)
            { // remove
                // skip name
                while(*s2!='\\' && *s2) s2++;
                // skip slash
                if(*s2=='\\') s2++;
                // skip value
                while(*s2!='\\' && *s2!='\"' && *s2) s2++;
                // skip slash
                if(*s2=='\\') s2++;
            }
            else
            { // change
                // copy name
                while(*s2!='\\' && *s2) *d2++=*s2++;
                // copy slash
                if(*s2=='\\') *d2++=*s2++;

                // skip original value
                while(*s2!='\\' && *s2!='\"' && *s2) s2++;
                // insert new value
                p=value;
                while(*p) *d2++=*p++;
                // copy slash
                if(*s2=='\\') *d2++=*s2++;
            }

            found=1;
        }
        else
        { // other attribute, just copy
            while(*s2!='\\' && *s2) *d2++=*s2++;
            if(*s2=='\\') *d2++=*s2++;
            while(*s2!='\\' && *s2!='\"' && *s2) *d2++=*s2++;
            if(*s2=='\\') *d2++=*s2++;
        }
    }

    logx(LOG_DEBUG,"clientinfo=%s",u->clientinfo);
}

Camp *campadd(User *u,int entity,int *pos,int modelindex,int skin)
{
    Camp *ca;

    ca=&u->camp[u->camps];
    ca->entity=entity;
    ca->pos[0]=pos[0];
    ca->pos[1]=pos[1];
    ca->pos[2]=pos[2];

    if(strstr(u->model[modelindex],"armor"))
    {
        if(skin==2) ca->name[0]='r';
        else if(skin==1) ca->name[0]='y';
        else ca->name[0]='g';
        ca->name[1]='a';
        ca->name[2]='0'+(u->campcnt[15-skin]++);
        ca->name[3]=0;
        ca->delay=20000;
        ca->pickupsound="armor1.wav";
        if(skin==2)      ca->category=CAMP_RA;
        else if(skin==1) ca->category=CAMP_YA;
        else             ca->category=CAMP_GA;
        ca->genericname[0]=ca->name[0];
        ca->genericname[1]=ca->name[1];
        ca->genericname[2]=0;
    }
    else if(strstr(u->model[modelindex],"b_bh100"))
    {
        ca->name[0]='m';
        ca->name[1]='h';
        ca->name[2]='0'+(u->campcnt[1]++);
        ca->name[3]=0;
        ca->delay=125000;
        ca->category=CAMP_MH;
        ca->pickupsound="r_item2.wav";
        ca->genericname[0]='m';
        ca->genericname[1]='e';
        ca->genericname[2]='g';
        ca->genericname[3]='a';
        ca->genericname[4]=0;
    }
    else if(strstr(u->model[modelindex],"suit"))
    {
        ca->name[0]='s';
        ca->name[1]='u';
        ca->name[2]='0'+(u->campcnt[2]++);
        ca->name[3]=0;
        ca->delay=125000;
        ca->category=CAMP_SUIT;
        ca->pickupsound="?.wav";
        ca->genericname[0]='s';
        ca->genericname[1]='u';
        ca->genericname[2]='i';
        ca->genericname[3]='t';
        ca->genericname[4]=0;
    }
    else if(strstr(u->model[modelindex],"invisib"))
    {
        ca->name[0]='r';
        ca->name[1]='0'+(u->campcnt[3]++);
        ca->name[2]=0;
        ca->delay=5*60000;
        ca->category=CAMP_R;
        ca->pickupsound="inv1.wav";
        ca->genericname[0]='r';
        ca->genericname[1]='i';
        ca->genericname[2]='n';
        ca->genericname[3]='g';
        ca->genericname[4]=0;
    }
    else if(strstr(u->model[modelindex],"invulne"))
    {
        ca->name[0]='p';
        ca->name[1]='0'+(u->campcnt[4]++);
        ca->name[2]=0;
        ca->delay=5*60000;
        ca->category=CAMP_P;
        ca->pickupsound="protect.wav";
        ca->genericname[0]='p';
        ca->genericname[1]='e';
        ca->genericname[2]='n';
        ca->genericname[3]='t';
        ca->genericname[4]=0;
    }
    else if(strstr(u->model[modelindex],"quaddama"))
    {
        ca->name[0]='q';
        ca->name[1]='0'+(u->campcnt[5]++);
        ca->name[2]=0;
        ca->delay=60000;
        ca->category=CAMP_Q;
        ca->pickupsound="damage.wav";
        ca->genericname[0]='q';
        ca->genericname[1]='u';
        ca->genericname[2]='a';
        ca->genericname[3]='d';
        ca->genericname[4]=0;
    }
    else if(strstr(u->model[modelindex],"g_rock2"))
    {
        ca->name[0]='r';
        ca->name[1]='l';
        ca->name[2]='0'+(u->campcnt[6]++);
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_RL;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]=ca->name[0];
        ca->genericname[1]=ca->name[1];
        ca->genericname[2]=0;
    }
    else if(strstr(u->model[modelindex],"g_rock"))
    {
        ca->name[0]='g';
        ca->name[1]='l';
        ca->name[2]='0'+(u->campcnt[7]++);
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_GL;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]=ca->name[0];
        ca->genericname[1]=ca->name[1];
        ca->genericname[2]=0;
    }
    else if(strstr(u->model[modelindex],"g_light"))
    {
        ca->name[0]='l';
        ca->name[1]='g';
        ca->name[2]='0'+(u->campcnt[8]++);
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_LG;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]=ca->name[0];
        ca->genericname[1]=ca->name[1];
        ca->genericname[2]=0;
    }
#if 0
    else if(strstr(u->model[modelindex],"g_nail2"))
    {
        ca->name[0]='s';
        ca->name[1]='n';
        ca->name[2]='0'+(u->campcnt[9]++);
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_SNG;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]='s';
        ca->genericname[1]='n';
        ca->genericname[2]='g';
        ca->genericname[3]=0;
    }
    else if(strstr(u->model[modelindex],"g_nail"))
    {
        ca->name[0]='n';
        ca->name[1]='g';
        ca->name[2]='0'+(u->campcnt[10]++);
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_NG;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]='n';
        ca->genericname[1]='g';
        ca->genericname[2]=0;
    }
    else if(strstr(u->model[modelindex],"g_shot"))
    {
        ca->name[0]='s';
        ca->name[1]='s';
        ca->name[2]='0'+(u->campcnt[12]++);
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_SSG;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]='s';
        ca->genericname[1]='s';
        ca->genericname[2]='g';
        ca->genericname[3]=0;
    }
#endif
    else if(strstr(u->model[modelindex],"flag") || strstr(u->model[modelindex],"tf_stan"))
    {
        int color='?';
        // find color
        if(strstr(u->model[modelindex],"tf_"))
        {
            // in tf:
            // 0=?
            // 1=blue
            // 2=red
            // 3,4=black?,yellow?
            if(skin==1) color='b';
            else if(skin==2) color='r';
        }
        else //if(u->ctf)
        {
            // in ctf:
            // 0=red
            // 1=blue
            if(skin==1) color='b';
            else if(skin==0) color='r';
        }

        ca->name[0]='!';
        ca->name[1]=ca->name[2]=color;
        ca->name[2]=0;
        ca->name[3]=0;
        ca->delay=30000;
        ca->category=CAMP_RUNE;
        ca->pickupsound="pkup.wav";
        ca->genericname[0]='f';
        ca->genericname[1]='l';
        ca->genericname[2]='a';
        ca->genericname[3]='g';
        ca->genericname[4]=0;
    }
    else ca=NULL;

    return(ca);
}

char *makeentityname(User *u,int modelindex,int skin,int *mask)
{
    if(strstr(u->model[modelindex],"backpack"))
    {
        *mask=CAMP_PACK;
        return("pack");
    }
    else if(strstr(u->model[modelindex],"_key"))
    { // also b_ key
        *mask=CAMP_RUNE;
        return("key");
    }
    else if(strstr(u->model[modelindex],"g_nail2"))
    {
        *mask=CAMP_SNG;
        return("sng");
    }
    else if(strstr(u->model[modelindex],"g_nail"))
    {
        *mask=CAMP_NG;
        return("ng");
    }
    else if(strstr(u->model[modelindex],"g_shot"))
    {
        *mask=CAMP_SSG;
        return("ssg");
    }
    else if(strstr(u->model[modelindex],"b_"))
    {
        if(strstr(u->model[modelindex],"b_bh"))
        {
            *mask=CAMP_HEALTH;
            return("health");
        }
        else if(strstr(u->model[modelindex],"b_nail"))
        {
            *mask=CAMP_NAILS;
            return("nails");
        }
        else if(strstr(u->model[modelindex],"b_rock"))
        {
            *mask=CAMP_ROCKETS;
            return("rockets");
        }
        else if(strstr(u->model[modelindex],"b_shell"))
        {
            *mask=CAMP_SHELLS;
            return("shells");
        }
        else if(strstr(u->model[modelindex],"b_batt"))
        {
            *mask=CAMP_CELLS;
            return("cells");
        }
    }
    else if(strstr(u->model[modelindex],"tf_flag") || strstr(u->model[modelindex],"tf_stan"))
    {
        *mask=CAMP_RUNE;
        // in tf:
        // 0=?
        // 1=blue
        // 2=red
        // 3,4=black?,yellow?
        if(skin==1) return("blue flag");
        else if(skin==2) return("red flag");
        else return("flag");
    }
    else if(strstr(u->model[modelindex],"flag"))
    { // after TF_FLAG has been checked
        *mask=CAMP_RUNE;
        // in ctf:
        // 0=red
        // 1=blue
        if(skin==1) return("blue flag");
        else if(skin==0) return("red flag");
        else return("flag");
    }
    else if(strstr(u->model[modelindex],"detpack"))
    {
        *mask=CAMP_RUNE;
        return("detpack");
    }
    else if(strstr(u->model[modelindex],"turrgun"))
    {
        *mask=CAMP_RUNE;
        return("sentrygun");
    }
    else if(strstr(u->model[modelindex],"disp"))
    {
        *mask=CAMP_RUNE;
        return("dispenser");
    }
    else if(strstr(u->model[modelindex],"turrbase"))
    {
        *mask=CAMP_RUNE;
        return("sentrygun");
    }
    else if(strstr(u->model[modelindex],"end"))
    {
        if(strstr(u->model[modelindex],"end1"))
        {
            *mask=CAMP_RUNE;
            return("resist");
        }
        if(strstr(u->model[modelindex],"end2"))
        {
            *mask=CAMP_RUNE;
            return("strength");
        }
        if(strstr(u->model[modelindex],"end3"))
        {
            *mask=CAMP_RUNE;
            return("haste");
        }
        if(strstr(u->model[modelindex],"end4"))
        {
            *mask=CAMP_RUNE;
            return("regen");
        }
    }
    *mask=0;
    return("item");
}

void reportfpd(User *u,int fpd)
{
    int  f;

    f=litemode;
    if(f)
    {
        q_printf(u,"proxy: lite mode disables:\n");
        if(f&1)    q_print(u,"proxy: - all % reporting\n");
        if(f&2)    q_print(u,"proxy: - poweruptimers\n");
        if(f&4)    q_print(u,"proxy: - soundtrigger\n");
        if(f&8)    q_print(u,"proxy: - (lagfeature)\n");
        if(f&16)   q_print(u,"proxy: - (report lagchange)\n");
        if(f&32)   q_print(u,"proxy: - %e reporting\n");
        if(f&64)   q_print(u,"proxy: - (spectator/player talk)\n");
        if(f&128)  q_print(u,"proxy: - hide pointing (%txyjk)\n");
        if(f&256)  q_print(u,"proxy: - skin forcing\n");
        if(f&512)  q_print(u,"proxy: - color forcing\n");
    }

    f=fpd&~litemode;
    if(f)
    {
        if(litemode) q_printf(u,"proxy: server disables (fpd=%i):\n",f);
        else q_printf(u,"proxy: server disables (fpd=%i):\n",f);
        if(f&1)    q_print(u,"proxy: - all % reporting\n");
        if(u->notimers) q_print(u,"proxy: - poweruptimers (.notimers)\n");
        else if(f&2)    q_print(u,"proxy: - poweruptimers\n");
        if(f&4)    q_print(u,"proxy: - soundtrigger\n");
        if(f&8)    q_print(u,"proxy: - (lagfeature)\n");
        if(f&16)   q_print(u,"proxy: - (report lagchange)\n");
        if(f&32)   q_print(u,"proxy: - %e reporting\n");
        if(f&64)   q_print(u,"proxy: - (spectator/player talk)\n");
        if(f&128)  q_print(u,"proxy: - hide pointing (%txyjk)\n");
        if(f&256)  q_print(u,"proxy: - skin forcing\n");
        if(f&512)  q_print(u,"proxy: - color forcing\n");
    }
    else q_print(u,"proxy: no features disabled\n");
}

//------------------------------

void q_servercmd(User *u,uchar *text)
{
    uchar *p;
    p=u->appendclient+u->appendclientbytes;
    *p++=0x4;
    while(*text)
    {
        *p++=*text++;
    }
    *p++=0;
    u->appendclientbytes=p-u->appendclient;
}

void q_explode(User *u,int type,int x,int y,int z)
{
    u->appendserver[u->appendserverbytes++]=0x17;
    u->appendserver[u->appendserverbytes++]=type;
    if(type==12 || type==2) u->appendserver[u->appendserverbytes++]=16; // 10 particles
    u->appendserver[u->appendserverbytes++]=x;
    u->appendserver[u->appendserverbytes++]=x>>8;
    u->appendserver[u->appendserverbytes++]=y;
    u->appendserver[u->appendserverbytes++]=y>>8;
    u->appendserver[u->appendserverbytes++]=z;
    u->appendserver[u->appendserverbytes++]=z>>8;
}

void q_print1(User *u,uchar *text)
{ // print text in white
    uchar *p;
    if(u->quiet) return;
    p=u->appendserver+u->appendserverbytes;
    *p++=0x8;
    *p++=2;
    while(*text)
    {
        *p++=*text++;
    }
    *p++=0;
    u->appendserverbytes=p-u->appendserver;
}

void q_print(User *u,uchar *text)
{ // print text in red
    uchar *p;
    int  a;
    if(u->quiet) return;
    p=u->appendserver+u->appendserverbytes;
    *p++=0x8;
    *p++=2;
    while(*text)
    {
        a=*text++;
        if((a&0x7f)>32) a^=0x80;
        *p++=a;
    }
    *p++=0;
    u->appendserverbytes=p-u->appendserver;
}

void q_printf(User *u,uchar *text,...)
{ // print text in red
    static uchar buf[BUFSIZE];
    va_list argp;
    va_start(argp,text);
    vsprintf(buf,text,argp);
    q_print(u,buf);
}

void q_centerprint(User *u,uchar *text)
{
    uchar *p;
    p=u->appendserver+u->appendserverbytes;
    *p++=0x1a;
    while(*text)
    {
        *p++=*text++;
    }
    *p++=0;
    u->appendserverbytes=p-u->appendserver;
}

void q_stufftext(User *u,uchar *text)
{
    uchar *p;
    p=u->appendserver+u->appendserverbytes;
    *p++=0x9;
    while(*text)
    {
        *p++=*text++;
    }
    *p++=0;
    u->appendserverbytes=p-u->appendserver;
}

//------------------------------

void q_growlocmax(User *u,int num)
{ // num<0 frees
    Location *old;
    if(num<u->locationsmax) return;
    if(num<u->locationsmax+256) num=u->locationsmax+256;

    old=u->loc;

    u->loc=mymalloc(num*sizeof(Location));
    memset(u->loc,0,num*sizeof(Location));
    memcpy(u->loc,old,u->locations*sizeof(Location));
    myfree(old);

    logx(LOG_DEBUG,"max locations grown to %i\n",num);
    u->locationsmax=num;
}

void q_setlocname(User *u,int i,char *name)
{
    Location *loc;
    int len;

    while(i>=u->locationsmax-16)
    {
        q_growlocmax(u,u->locationsmax*2);
    }

    loc=u->loc+i;

    if(loc->name)
    {
        myfree(loc->name);
    }

    if(name==NULL)
    {
        loc->name=NULL;
    }
    else
    {
        len=strlen(name);
        loc->name=mymalloc(len+1);
        strcpy(loc->name,name);
    }
}

void q_loadloc(User *u,uchar *mapname)
{
    FILE *f1;
    uchar tmp[MAXSTR];
    if(mapname==NULL)
    {
        strcpy(tmp,u->locname);
    }
    else
    {
        strcpy(tmp,mapname);
        strcat(tmp,".loc");
        strcpy(u->locname,tmp);
    }
    f1=fopen(tmp,"rb");
    u->locations=0;
    if(f1)
    {
        while(!feof(f1))
        {
            *tmp=0;
            fgets(tmp,255,f1);
            if(*tmp>32)
            {
                uchar *p,*p2;
                p=tmp;

                while(*p && *p<=32) p++;
                u->loc[u->locations].x=atoi(p);
                while(*p && *p>32) p++;

                while(*p && *p<=32) p++;
                u->loc[u->locations].y=atoi(p);
                while(*p && *p>32) p++;

                while(*p && *p<=32) p++;
                u->loc[u->locations].z=atoi(p);
                while(*p && *p>32) p++;

                while(*p && *p<=32) p++;

                p2=p;
                while(*p2 && *p2!='\n' && *p2!='\r') p2++;
                *p2=0;

                q_setlocname(u,u->locations,p);

                logx(LOG_DEBUG,"Location %s (%i,%i,%i)\n",
                              u->loc[u->locations].name,
                              u->loc[u->locations].x,
                              u->loc[u->locations].y,
                              u->loc[u->locations].z);

                u->locations++;
                if(u->locations>=u->locationsmax-16)
                {
                    q_growlocmax(u,u->locationsmax*2);
                }
            }
        }
        fclose(f1);
        if(u->locations>0)
        {
            sprintf(tmp,"proxy: %i locations from %s\n",u->locations,u->locname);
            q_print(u,tmp);
        }
    }
}

void q_savelocs(User *u)
{
   FILE *f1;
   int i=0;
   f1=fopen(u->locname,"wb");
   if(f1)
   {
       while(i<u->locations)
       {
           fprintf(f1,"%i %i %i %s\n",
               u->loc[i].x,
               u->loc[i].y,
               u->loc[i].z,
               u->loc[i].name);
           i++;
       }
       fclose(f1);
   }
}

//------------------------------

uchar *formatqstring(uchar *in)
{
    static uchar buf[MAXSTR];
    uchar *p=buf;
    while(*in && p<buf+MAXSTR/2)
    {
        if(*in=='\n')
        {
            //*p++='\\';
            //*p++='n';
        }
        else *p++=*in;
        in++;
    }
    *p++=0;
    return(buf);
}

uchar *extractfield(uchar *dest,uchar *src,uchar *field)
{
    uchar *p=src,*p0;
    uchar *dend=dest+MAXSTR-2;
    int    fieldlen=strlen(field);
    // skip first /
    while(*p!='\\' && *p) p++; // skip end of value
    if(*p) p++;
    // find key
    while(*p)
    {
        if(!memcmp(p,field,fieldlen))
        {
            p+=fieldlen;
            p0=p;
            while(*p && *p!='\\' && *p!='"' && dest<dend)
            {
                *dest++=*p++;
            }
            *dest=0;
            return(p0);
        }
        while(*p!='\\' && *p) p++; // skip end of key
        if(*p) p++;
        while(*p!='\\' && *p) p++; // skip end of value
        if(*p) p++;
    }
    *dest=0;
    return(p);
}

void saveparseerror(char *text,int pos,uchar *buf,int bytes)
{
    FILE *f1;
    int   i;
    f1=fopen("parse.log","ab");
    if(f1)
    {
        fprintf(f1,text,buf[pos],pos);
        fwrite(buf,1,bytes,f1);
        for(i=0;i<16;i++) putc(0xff,f1);
        for(i=bytes&15;i<16;i++) putc(0x00,f1);
        fclose(f1);
    }
}

//------------------------------

uchar funny[64]=
//0123456789ABCDEF0123456789ABCDEF
//aaaaaaaabbbbbbbbccccccccdddddddd
//01234567012345670123456701234567
 "(=)a  ghij_b c  []0123456789.<->"
 "                            ,   ";
int funny2[64]={
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f};

void funnyhelp(User *u)
{
    int i,n=0;
    uchar *p;
    q_print(u,"Funny names helper:\n");
    q_print(u,"names and messages containg\n"
              "following character sequences\n"
              "are converted.\n");

    strcpy(tmpbuf,"#X:red version of X\n");
    tmpbuf[0]^=0x80;
    tmpbuf[1]^=0x80;
    q_print(u,tmpbuf);

    strcpy(tmpbuf,"$r:cr, $n:lf (team only)\n");
    q_print(u,tmpbuf);

    strcpy(tmpbuf,"$0..9:yellow number\n");
    tmpbuf[0]^=0x80;
    tmpbuf[1]^=0x80;
    tmpbuf[4]^=0x80;
    q_print(u,tmpbuf);
    p=tmpbuf;
    for(i=0;i<64;i++) if((funny[i]<'0' || funny[i]>'9') && funny[i]< 'A' && funny[i]!=' ')
    {
        sprintf(p,"$%c:%c ",funny[i],funny2[i]);
        p[0]^=0x80;
        p[1]^=0x80;
        p+=strlen(p);
        n++;
        if(n>=7)
        {
            *p++='\n';
            n=0;
        }
    }
    for(i=0;i<64;i++) if((funny[i]<'0' || funny[i]>'9') && funny[i]>='A' && funny[i]!=' ')
    {
        sprintf(p,"$%c:%c ",funny[i],funny2[i]);
        p[0]^=0x80;
        p[1]^=0x80;
        p+=strlen(p);
        n++;
        if(n>=7)
        {
            *p++='\n';
            n=0;
        }
    }
    *p++='\n';
    n=0;
    for(i=0;i<32;i++)
    {
        sprintf(p,"$x%02X:%c ",i+0x80,i+0x80);
        p[0]^=0x80;
        p[1]^=0x80;
        p[2]^=0x80;
        p[3]^=0x80;
        p+=strlen(p);
        n++;
        if(n>=5)
        {
            *p++='\n';
            n=0;
        }
    }
    *p++='\n';
    *p++=0;
    q_print(u,tmpbuf);
}

int funnyconvert(uchar *d,uchar *s,int nobits)
{
    int chg=0;
    int nocolor=(nobits&1);
    int noenter=(nobits&2);
    while(*s)
    { // ##, $$
        if(*s=='#' && !nocolor)
        {
            s++;
            if(*s=='#')
            {
                *d++='#';
                s++;
            }
            else if(!*s) *d++=' ';
            else *d++=*s++^0x80;
            chg=1;
        }
        else if(*s=='$')
        {
            int j,x;
            s++;
            if(*s=='$')
            {
                *d++='$';
                s++;
            }
            else if(*s=='\\' && !noenter)
            {
                *d++='x';
                *d++='\r';
                s++;
            }
            else if(*s=='r' && !noenter)
            {
                *d++='\r';
                s++;
            }
            else if(*s=='n' && !noenter)
            {
                *d++='\n';
                s++;
            }
            else if(*s=='t' && !noenter)
            {
                *d++='\t';
                s++;
            }
#ifdef PROMODE
            else if(*s=='e')
            {
                *d++=0x7f;
                s++;
            }
            else if(*s=='z')
            {
                *d++=0x80;
                s++;
            }
#endif
            else if(*s=='x' && !noenter)
            {
                int a=0;
                s++;
                sscanf(s,"%02X",&a);
                *d++=a;
                if(*s) s++;
                if(*s) s++;
            }
            else if(*s=='\'')
            {
                *d++='\"';
                s++;
            }
            else if(!*s) *d++=' ';
            else
            {
                x=tolower(*s);
                for(j=0;j<64;j++) if(funny[j]==x)
                {
                    *d++=funny2[j];
                    break;
                }
                if(j==64)
                {
                    *d++='$';
                    *d++=*s;
                }
                s++;
            }
            chg=1;
        }
        else
        {
            *d++=*s++;
        }
    }
    *d++=0;
    return(chg);
}

//-------------------------------------------------------

unsigned char inittable[1024]={
0x78,0xD2,0x94,0xE3,0x41,0xEC,0xD6,0xD5,0xCB,0xFC,0xDB,0x8A,0x4B,0xCC,0x85,0x01,
0x23,0xD2,0xE5,0xF2,0x29,0xA7,0x45,0x94,0x4A,0x62,0xE3,0xA5,0x6F,0x3F,0xE1,0x7A,
0x64,0xED,0x5C,0x99,0x29,0x87,0xA8,0x78,0x59,0x0D,0xAA,0x0F,0x25,0x0A,0x5C,0x58,
0xFB,0x00,0xA7,0xA8,0x8A,0x1D,0x86,0x80,0xC5,0x1F,0xD2,0x28,0x69,0x71,0x58,0xC3,
0x51,0x90,0xE1,0xF8,0x6A,0xF3,0x8F,0xB0,0x68,0xDF,0x95,0x40,0x5C,0xE4,0x24,0x6B,
0x29,0x19,0x71,0x3F,0x42,0x63,0x6C,0x48,0xE7,0xAD,0xA8,0x4B,0x91,0x8F,0x42,0x36,
0x34,0xE7,0x32,0x55,0x59,0x2D,0x36,0x38,0x38,0x59,0x9B,0x08,0x16,0x4D,0x8D,0xF8,
0x0A,0xA4,0x52,0x01,0xBB,0x52,0xA9,0xFD,0x40,0x18,0x97,0x37,0xFF,0xC9,0x82,0x27,
0xB2,0x64,0x60,0xCE,0x00,0xD9,0x04,0xF0,0x9E,0x99,0xBD,0xCE,0x8F,0x90,0x4A,0xDD,
0xE1,0xEC,0x19,0x14,0xB1,0xFB,0xCA,0x1E,0x98,0x0F,0xD4,0xCB,0x80,0xD6,0x05,0x63,
0xFD,0xA0,0x74,0xA6,0x86,0xF6,0x19,0x98,0x76,0x27,0x68,0xF7,0xE9,0x09,0x9A,0xF2,
0x2E,0x42,0xE1,0xBE,0x64,0x48,0x2A,0x74,0x30,0xBB,0x07,0xCC,0x1F,0xD4,0x91,0x9D,
0xAC,0x55,0x53,0x25,0xB9,0x64,0xF7,0x58,0x4C,0x34,0x16,0xBC,0xF6,0x12,0x2B,0x65,
0x68,0x25,0x2E,0x29,0x1F,0xBB,0xB9,0xEE,0x6D,0x0C,0x8E,0xBB,0xD2,0x5F,0x1D,0x8F,
0xC1,0x39,0xF9,0x8D,0xC0,0x39,0x75,0xCF,0x25,0x17,0xBE,0x96,0xAF,0x98,0x9F,0x5F,
0x65,0x15,0xC4,0x62,0xF8,0x55,0xFC,0xAB,0x54,0xCF,0xDC,0x14,0x06,0xC8,0xFC,0x42,
0xD3,0xF0,0xAD,0x10,0x08,0xCD,0xD4,0x11,0xBB,0xCA,0x67,0xC6,0x48,0x5F,0x9D,0x59,
0xE3,0xE8,0x53,0x67,0x27,0x2D,0x34,0x9E,0x9E,0x24,0x29,0xDB,0x69,0x99,0x86,0xF9,
0x20,0xB5,0xBB,0x5B,0xB0,0xF9,0xC3,0x67,0xAD,0x1C,0x9C,0xF7,0xCC,0xEF,0xCE,0x69,
0xE0,0x26,0x8F,0x79,0xBD,0xCA,0x10,0x17,0xDA,0xA9,0x88,0x57,0x9B,0x15,0x24,0xBA,
0x84,0xD0,0xEB,0x4D,0x14,0xF5,0xFC,0xE6,0x51,0x6C,0x6F,0x64,0x6B,0x73,0xEC,0x85,
0xF1,0x6F,0xE1,0x67,0x25,0x10,0x77,0x32,0x9E,0x85,0x6E,0x69,0xB1,0x83,0x00,0xE4,
0x13,0xA4,0x45,0x34,0x3B,0x40,0xFF,0x41,0x82,0x89,0x79,0x57,0xFD,0xD2,0x8E,0xE8,
0xFC,0x1D,0x19,0x21,0x12,0x00,0xD7,0x66,0xE5,0xC7,0x10,0x1D,0xCB,0x75,0xE8,0xFA,
0xB6,0xEE,0x7B,0x2F,0x1A,0x25,0x24,0xB9,0x9F,0x1D,0x78,0xFB,0x84,0xD0,0x17,0x05,
0x71,0xB3,0xC8,0x18,0xFF,0x62,0xEE,0xED,0x53,0xAB,0x78,0xD3,0x65,0x2D,0xBB,0xC7,
0xC1,0xE7,0x70,0xA2,0x43,0x2C,0x7C,0xC7,0x16,0x04,0xD2,0x45,0xD5,0x6B,0x6C,0x7A,
0x5E,0xA1,0x50,0x2E,0x31,0x5B,0xCC,0xE8,0x65,0x8B,0x16,0x85,0xBF,0x82,0x83,0xFB,
0xDE,0x9F,0x36,0x48,0x32,0x79,0xD6,0x9B,0xFB,0x52,0x45,0xBF,0x43,0xF7,0x0B,0x0B,
0x19,0x19,0x31,0xC3,0x85,0xEC,0x1D,0x8C,0x20,0xF0,0x3A,0xFA,0x80,0x4D,0x2C,0x7D,
0xAC,0x60,0x09,0xC0,0x40,0xEE,0xB9,0xEB,0x13,0x5B,0xE8,0x2B,0xB1,0x20,0xF0,0xCE,
0x4C,0xBD,0xC6,0x04,0x86,0x70,0xC6,0x33,0xC3,0x15,0x0F,0x65,0x19,0xFD,0xC2,0xD3};

unsigned int checktable[256]={
0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,
0x1231,0x0210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,
0x9339,0x8318,0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,
0x2462,0x3443,0x0420,0x1401,0x64E6,0x74C7,0x44A4,0x5485,
0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,
0x3653,0x2672,0x1611,0x0630,0x76D7,0x66F6,0x5695,0x46B4,
0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,
0x48C4,0x58E5,0x6886,0x78A7,0x0840,0x1861,0x2802,0x3823,
0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,0xA90A,0xB92B,
0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0x0A50,0x3A33,0x2A12,
0xDBFD,0xCBDC,0xFBBF,0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,
0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0x0C60,0x1C41,
0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,
0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,0x2E32,0x1E51,0x0E70,
0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,
0x9188,0x81A9,0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,
0x1080,0x00A1,0x30C2,0x20E3,0x5004,0x4025,0x7046,0x6067,
0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,
0x02B1,0x1290,0x22F3,0x32D2,0x4235,0x5214,0x6277,0x7256,
0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,
0x34E2,0x24C3,0x14A0,0x0481,0x7466,0x6447,0x5424,0x4405,
0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,0xC71D,0xD73C,
0x26D3,0x36F2,0x0691,0x16B0,0x6657,0x7676,0x4615,0x5634,
0xD94C,0xC96D,0xF90E,0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,
0x5844,0x4865,0x7806,0x6827,0x18C0,0x08E1,0x3882,0x28A3,
0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,
0x4A75,0x5A54,0x6A37,0x7A16,0x0AF1,0x1AD0,0x2AB3,0x3A92,
0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,
0x7C26,0x6C07,0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0x0CC1,
0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,0x9FF8,
0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0x0ED1,0x1EF0};

int checksum(char *packetstart,char *data,int size)
{
    char buf[64];
    int  sum,i,a,b;

    if(size>0x3c) size=0x3c;
    memcpy(buf,data,size);

    b =(packetstart[0]&255);
    b|=(packetstart[1]&255)<<8;
    b|=(packetstart[2]&255)<<16;
    b|=(packetstart[3]&255)<<24;
    b&=0x7fffffff;
    a=b%1020;

    buf[size+0]=inittable[a+0]^(b);
    buf[size+1]=inittable[a+1];
    buf[size+2]=inittable[a+2]^(b>>8);
    buf[size+3]=inittable[a+3];
    size+=4;

//    printf("b=%02X ebp=%02X %02X %02X %02X\n",b,buf[size-4],buf[size-3],buf[size-2],buf[size-1]);

    sum=0xffff;
    for(i=0;i<size;i++)
    {
        a=(sum&0xffff) >> 8;
        a^=buf[i]&255;
        b=(sum&0xff) << 8;
        sum=checktable[a]^b;
    }

    return(sum&255);
}

int floodprot(User *u)
{
    if(u->floodprot && u->time<u->time_lastsay[0]+4100)
    {
        q_print(u,"proxy: flood protect protect\n");
        return(1);
    }

    u->time_lastsay[0]=u->time_lastsay[1];
    u->time_lastsay[1]=u->time_lastsay[2];
    u->time_lastsay[2]=u->time_lastsay[3];
    u->time_lastsay[3]=u->time;

    return(0);
}

void dmmchange(User *u,int newdmm)
{
    u->originaldmm=newdmm;
    u->dmm=newdmm;
    if(u->dmm<1) u->dmm=1;
    if(u->dmm>3) u->dmm=3;
    if(u->forcedmm) u->dmm=u->forcedmm;

    if(u->dmm==1) u->dmmitemmask=~0;
    else u->dmmitemmask=~(CAMP_RL|CAMP_LG|CAMP_GL|CAMP_SNG|CAMP_NG|CAMP_SSG);

//    q_printf(u,"dmm=%i, mask=%08X\n",u->dmm,u->dmmitemmask);
}

int text2color(char *p)
{
    int   c;
    char *p2;

    if(p[0]=='t' || p[0]=='T') return(-2);
    else if(p[0]=='r' || p[0]=='R') return(-3);
    else if(*p=='*' || !*p || *p<'0' || *p>'9') return(-1);

    p2=p;
    while(*p2 && *p2>32)
    {
        if(*p2==',') break;
        p2++;
    }

    if(*p2==',')
    {
        c=(atoi(p2+1)&15)+16*(atoi(p)&15)+256;
    }
    else
    {
        c=(atoi(p)&15)+16*(atoi(p)&15)+256;
    }

    return(c);
}

char *color2text(int c)
{
    static char buf[64];
    if(c==-1)      sprintf(buf,"* ");
    else if(c==-2) sprintf(buf,"Team ");
    else if(c==-3) sprintf(buf,"Rank ");
    else if(c<255) sprintf(buf,"%i ",c);
    else           sprintf(buf,"%i,%i ",(c>>4)&15,c&15);
    return(buf);
}

