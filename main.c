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

#define  WS_VERSION_REQUIRED   0x0101
#define  MIN_SOCKETS_REQUIRED  (MAXUSERS*2+4)

int    newsocketmode=0;
uchar   versionbuf[64];
uchar   versionnamebuf[64];
uchar  *version=versionbuf;
uchar  *versionname=versionnamebuf;

SOCKADDR_IN proxy_serverip;
SOCKADDR_IN proxy_clientip;

#ifdef WIN32
int    useselect=0;
#else
int    useselect=1;
#endif

int    noautoexit=0;
int    litemode=0;
int    autotimeout=1;
int    loglevel=0;
int    lograwpackets=0;
int    originalloglevel=0;
int    lognpackets=0;
int    oldloglevel=0;
int    hpymode=0;

int    priority=0;
int    prioritytoggle=0;

FILE   *lograwc;
FILE   *lograws;

int    stopping=0;

DWORD  timeout=0x7fffffff;
DWORD  timeouttime=1000*60*240; // 240 minutes

FILE  *logfile;

SOCKET consocket;

int    usernum=0;
int    maxusernum=MAXPLAYERS;
User   user[MAXUSERS];

#ifdef EXPIRETIME
uchar  *versiontype="";
char   *betanotes=
"\n"
"release notes:\n"
"\n";
#else
uchar  *versiontype="";
#endif

void cleanup(int type);
void setversions(void);

#ifndef WIN32

void Sleep(int ms)
{
    struct timeval delay;
    delay.tv_sec=0;
    delay.tv_usec=ms*1000;
    select(0,NULL,NULL,NULL,&delay);
}

int memicmp(uchar *a,uchar *b,int len)
{
    while(len>0)
    {
        if(((*a)|0x20)!=((*b)|0x20)) return(1);
        a++; b++; len--;
    }
    return(0);
}

int stricmp(uchar *a,uchar *b)
{
    int al,bl;
    al=strlen(a);
    bl=strlen(b);
    if(al!=bl) return(1);
    return(memicmp(a,b,al));
}

int strlwr(uchar *a)
{
    while(*a)
    {
        *a=tolower(*a);
        a++;
    }
#if defined(__APPLE__) || defined(MACOSX)
    return (0);
#endif /* __APPLE__ || MACOSX */
}

#endif

int timeus(void)
{ // return microseconds (win95 only)
#ifdef PERFTIME
    LONGLONG t;
    LARGE_INTEGER pc;
    static int initdone;
    static LARGE_INTEGER perf_freq,perf_counter;
    if(!initdone)
    {
        if(!QueryPerformanceFrequency(&perf_freq))
        {
            printf("This computer does not support Pentium Performance Counter\n");
            exit(3);
        }
        QueryPerformanceCounter(&perf_counter);
        initdone=1;
    }
    QueryPerformanceCounter(&pc);
    t=1000000*(pc.QuadPart-perf_counter.QuadPart)/perf_freq.QuadPart;
    return((int)t);
#else
    return(0);
#endif
}

int timems(void)
{ // return milliseconds
#ifdef GETTIMEOFDAY
    struct timeval  tv;
    struct timezone tz;
    static int initdone;
    static int seconds0;
    gettimeofday(&tv,&tz);
    if(!initdone)
    {
        seconds0=tv.tv_sec;
        initdone=1;
    }
    return((tv.tv_sec-seconds0)*1000+tv.tv_usec/1000);
#endif
#ifdef PERFTIME
    LONGLONG t;
    LARGE_INTEGER pc;
    static int initdone;
    static LARGE_INTEGER perf_freq,perf_counter;
    if(!initdone)
    {
        if(!QueryPerformanceFrequency(&perf_freq))
        {
            printf("This computer does not support Pentium Performance Counter\n");
            exit(3);
        }
        QueryPerformanceCounter(&perf_counter);
        initdone=1;
    }
    QueryPerformanceCounter(&pc);
    t=1000*(pc.QuadPart-perf_counter.QuadPart)/perf_freq.QuadPart;
    return((int)t);
#endif
#ifdef HRTIME
    hrtime_t tim;
    static hrtime_t tim0;
    static int initdone;
    tim=gethrtime();
    if(!initdone)
    {
        tim0=tim;
        initdone=1;
    }
    tim-=tim0;
    return((int)(tim/1000000));
#endif
#ifdef FTIME
    struct timeb tim;
    static int   seconds0;
    static int   initdone;
    int ms;
    ftime(&tim);
    if(!initdone)
    {
        seconds0=tim.time;
        initdone=1;
    }
    ms=(tim.time-seconds0)*1000+tim.millitm;
    return(ms);
#endif
#ifdef WINTIME
    return(GetCurrentTime());
#endif
}

#ifdef EXPIRETIME
static int expires_time;
#endif

void expires_gettime(void)
{
#ifdef EXPIRETIME
    time(&expires_time);
#endif
}

void log_open(void)
{
    if(loglevel>0) logfile=fopen("proxy.log","wt");
}

void log_close(void)
{
    if(loglevel>0) fclose(logfile);
}

void log_flush(void)
{
    if(loglevel>0)
    {
        fflush(logfile);
    }
}

static int dontflush;

void logx(int level,uchar *txt,...)
{
    static uchar buf[BUFSIZE];
    va_list argp;

    if(loglevel<0) return;
    if(level>LOG_CONSOLE && level>loglevel) return;
    va_start(argp,txt);
    vsprintf(buf,txt,argp);
    if(level<=LOG_CONSOLE)
    {
        fputs(buf,stdout);
    }
    if(level<=loglevel && logfile)
    {
        fputs(buf,logfile);
        if(level==LOG_CONSOLE) fflush(logfile);
    }
    if(!dontflush && loglevel==LOG_FLUSH) fflush(logfile);
}

void *mymalloc(int size)
{
    static void *safe;
    void *p;

    if(!size) size=1;

    if(!safe)
    { // allocate some safety (get core for us)
        safe=malloc(256*1024); // 256K safety
        if(!safe)
        {
            logx(LOG_CONSOLE,"Out of memory!\n");
            printf("Out of memory!\n");
            exit(3);
        }
    }

    p=malloc(size);

    if(!p)
    {
        logx(LOG_CONSOLE,"Running low on memory!\n");
        // free safety and try to alloc again
        if(safe)
        {
            free(safe);
        }

        p=malloc(size);
        if(!p)
        {
            logx(LOG_CONSOLE,"Out of memory!\n");
            printf("Out of memory!\n");
            exit(3);
        }
    }

    return(p);
}

void myfree(void *p)
{
    if(p) free(p);
}

void expires_check(void)
{
#ifdef EXPIRETIME
    if(expires_time>EXPIRETIME+172800)
    {
        exit(3);
    }
#endif
}

void ws_init(void)
{
#ifdef WIN32
    WSADATA      wsaData;
    int          error;

    error=WSAStartup(WS_VERSION_REQUIRED,&wsaData);
    if (error !=0 ) {
          logx(LOG_CONSOLE,"error: Windows Sockets not responding or compatible");
          exit(3);
    }

    if (wsaData.wVersion < WS_VERSION_REQUIRED)
    {
          logx(LOG_CONSOLE,"error: Windows Sockets version %d.%d not supported",
                 LOBYTE (wsaData.wVersion), HIBYTE (wsaData.wVersion));
          exit(3);
    }

    if (wsaData.iMaxSockets < MIN_SOCKETS_REQUIRED ) {
          logx(LOG_CONSOLE,"error: Windows Sockets could not support %d sockets",
                 MIN_SOCKETS_REQUIRED);
          exit(3);
    }
#endif
}

void ws_deinit(void)
{
#ifdef WIN32
    int   error;
    error=WSACleanup();
    if (error !=0 ) {
          logx(LOG_CONSOLE,"error: Windows Sockets cleanup failed.");
          exit(3);
    }
#endif
}

void ws_showerror(User *u,uchar *when)
{
#ifdef WIN32
    int err=WSAGetLastError();
    if(err==WSAENOTSOCK)
    {
        static int reportonce=0;
        if(!reportonce)
        {
            logx(LOG_VERBOSE,"error: Windows Sockets error %d calling %s [once]\n",err,when);
            reportonce=1;
        }
        return;
    }
    logx(LOG_VERBOSE,"error: Windows Sockets error %d calling %s\n",err,when);
#endif
    if(u) u->errors++;
}

int ws_openserver(int port)
{
//    unsigned long flag;

    consocket=socket(PF_INET,SOCK_DGRAM,0);
    if(consocket==INVALID_SOCKET) ws_showerror(NULL,"socket");

    if(bind(consocket,(LPSOCKADDR)&proxy_serverip,sizeof(proxy_serverip))==SOCKET_ERROR)
    {
        ws_showerror(NULL,"bind");
        logx(LOG_CONSOLE,"Unable to bind to port %i\n",port);
        return(1);
    }

//    flag=1; // nonblocking
//    ioctlsocket(consocket,FIONBIO,&flag);
    return(0);
}

void c_hexdump(uchar *msg,uchar *buffer0,int bytes,User *u)
{
    uchar *buffer=(uchar *)buffer0;
    int i,j,n,s0,s1,a;

    dontflush=1;
    s0=intat(buffer+0);
    s1=intat(buffer+4);
    if(u)
    {
        logx(LOG_PACKETS,"\n%s [%s] (%i bytes): %i%c %i%c\n",
            msg,u->name,bytes,
            s0==0xffffffff?0:s0&0x7fffffff,s0&0x80000000?'x':'.',
            s1==0xffffffff?0:s1&0x7fffffff,s1&0x80000000?'x':'.');
    }
    else
    {
        logx(LOG_PACKETS,"\n%s [] (%i bytes): %i%c %i%c\n",
            msg,bytes,
            s0==0xffffffff?0:s0&0x7fffffff,s0&0x80000000?'x':'.',
            s1==0xffffffff?0:s1&0x7fffffff,s1&0x80000000?'x':'.');
    }

    n=(bytes+15)/16*16;
    for(i=0;i<n;i+=16)
    {
        for(j=i;j<i+16;j++)
        {
            if(j<bytes) logx(LOG_PACKETS,"%02X ",buffer[j]);
            else logx(LOG_PACKETS,"   ");
        }
        logx(LOG_PACKETS," ");
        for(j=i;j<i+16;j++)
        {
            if(j<bytes)
            {
                a=buffer[j]&127;
                if(a<32) logx(LOG_PACKETS,".");
                else logx(LOG_PACKETS,"%c",a);
            }
            else logx(LOG_PACKETS," ");
        }
        logx(LOG_PACKETS,"\n");
    }
    if(lognpackets>0)
    {
        lognpackets--;
        if(!lognpackets)
        {
            loglevel=oldloglevel;
        }
    }
    dontflush=0;
}

User *c_finduser(SOCKADDR_IN *from)
{
    int   i;

    for(i=0;i<usernum;i++)
    {
        if(c_sameaddress(from,&user[i].toclientaddr)) return(user+i);
    }
    return(NULL);
}

void c_reopenserver(User *u)
{
    SOCKET news;
    SOCKADDR_IN tmp;

    if(newsocketmode) return;

    news=socket(PF_INET,SOCK_DGRAM,0);
    if(news==INVALID_SOCKET) ws_showerror(NULL,"socket2");
    closesocket(u->toserver);
    u->toserver=news;

    memcpy(&tmp,&proxy_clientip,sizeof(SOCKADDR_IN));
    tmp.sin_port=htons(0);
    bind(u->toserver,(LPSOCKADDR)&tmp,sizeof(tmp));

    {
        char buf[64];
        SOCKADDR_IN addr;
        unsigned int addrlen=sizeof(SOCKADDR_IN);
        memset(&addr,0,sizeof(addr));
        getsockname(u->toserver,(void *)&addr,&addrlen);
        c_textaddress(buf,&addr);
        logx(LOG_PACKETS,"Toserver socket %s\n",buf);
    }
}

User *c_newuser(SOCKADDR_IN *from)
{
    User *u;

    if(usernum==MAXUSERS) return(NULL);

    u=user+usernum;
    memset(u,0,sizeof(User));

    if(newsocketmode) u->toserver=consocket;
    else
    {
        u->toserver=socket(PF_INET,SOCK_DGRAM,0);
        if(u->toserver==INVALID_SOCKET) ws_showerror(NULL,"socket3");
    }
    u->toclientaddr=*from;

    /*
    {
        SOCKADDR_IN tmp;
        tmp.sin_family=AF_INET;
        tmp.sin_addr.s_addr=htonl(INADDR_ANY);
        tmp.sin_port=htons(0);
        bind(u->toclient,(LPSOCKADDR)&tmp,sizeof(tmp));
    }

    {
        char buf[64];
        SOCKADDR_IN addr;
        int addrlen=sizeof(SOCKADDR_IN);
        memset(&addr,0,sizeof(addr));
        getsockname(u->toclient,(void *)&addr,&addrlen);
        c_textaddress(buf,&addr);
        logx(LOG_PACKETS,"Toclient socket %s\n",buf);
    }
    */

//    flag=1; // nonblocking io
//    ioctlsocket(u->toserver,FIONBIO,&flag);
//    ioctlsocket(u->toclient,FIONBIO,&flag);

    u->locations=0;
    u->locationsmax=256;
    u->loc=mymalloc(sizeof(Location)*u->locationsmax);
    memset(u->loc,0,sizeof(Location)*u->locationsmax);

    usernum++;
    return(u);
}

void c_killuser(User *u)
{
    int   i,uid;

    uid=u-user;
    if(uid<0 || uid>=MAXUSERS) return;

    if(!newsocketmode)
    {
        closesocket(u->toserver);
        //closesocket(u->toclient);
    }

    for(i=0;i<MAXMODELS;i++)
    {
        if(u->model[i]) free(u->model[i]);
    }
    for(i=0;i<MAXSOUNDS;i++)
    {
        if(u->sound[i]) free(u->sound[i]);
    }
    for(i=0;i<u->locationsmax;i++)
    {
        if(u->loc[i].name) free(u->loc[i].name);
    }
    free(u->loc);
    u->loc=NULL;

    memcpy(user+uid,user+usernum-1,sizeof(User));
    usernum--;
}

void c_killserver(User *u)
{
    SOCKET news;
//    int flag;

    if(!newsocketmode)
    {
        news=socket(PF_INET,SOCK_DGRAM,0);

        closesocket(u->toserver);
        u->toserver=news;
    }

    u->time_lastserver=0;

    u->servermainaddr.sin_port=0;

//    flag=1; // nonblocking io
//    ioctlsocket(u->toserver,FIONBIO,&flag);
}

void c_textaddress(uchar *text,SOCKADDR_IN *a)
{
    if(a->sin_port)
    {
        strcpy(text,inet_ntoa(a->sin_addr));
        sprintf(text+strlen(text),":%i",ntohs(a->sin_port));
    }
    else strcpy(text,"-");
}

int c_sameaddress(SOCKADDR_IN *a,SOCKADDR_IN *b)
{
   if(a->sin_addr.s_addr!=b->sin_addr.s_addr) return(0);
   if(a->sin_port!=b->sin_port) return(0);
   return(1);
}

int c_islocalconnect(SOCKADDR_IN *a)
{
    unsigned int lh,lhm;
    if(a->sin_port!=htons(27001)) return(0);
    lh =htonl(0x7f000000);
    lhm=htonl(0xff000000);
    if((a->sin_addr.s_addr&lhm)==lh) return(1);
    return(0);
}

void c_setaddress(SOCKADDR_IN *a,uchar *text)
{
    uchar addr[256];
    uchar port[8];
    uchar *p;
    struct hostent *he;

    logx(LOG_DEBUG,"Namelookup '%s'\n",text);

    p=addr;
    while(*text && *text>32 && *text!=':' && p<addr+255)
    {
        *p++=*text++;
    }
    if(*text==':') text++;
    *p=0;

    p=port;
    while(*text && p<port+7)
    {
        *p++=*text++;
    }
    *p=0;

    a->sin_family=AF_INET;
    a->sin_port=htons((unsigned short)atoi(port));
    if(!a->sin_port) a->sin_port=htons(27500);

        a->sin_addr.s_addr=inet_addr(addr);
    if(a->sin_addr.s_addr==-1)
    { // not xxx.xxx.xxx.xxx, do lookup
        he=gethostbyname(addr);
        if(!he)
        { // dns failed
            a->sin_addr.s_addr=0;
        }
        else
        {
            memcpy(&a->sin_addr,he->h_addr_list[0],sizeof(struct in_addr));
        }
    }

    if(a->sin_addr.s_addr==0xffffffff || a->sin_addr.s_addr==0x0)
    {
        a->sin_addr.s_addr=0;
        a->sin_port=0;
    }

    logx(LOG_DEBUG,"Namelookup result %08X:%i\n",a->sin_addr.s_addr,atoi(port));
}

void c_sendservermain(User *u,uchar *buffer,int bytes)
{
    int error;
    if(loglevel>=LOG_PACKETS) c_hexdump("Proxy->ServerConnect",buffer,bytes,u);
    error=sendto(u->toserver,buffer,bytes,0,(LPSOCKADDR)&u->servermainaddr,sizeof(SOCKADDR_IN));
    if(error==SOCKET_ERROR)
    {
        uchar tmp[64];
        c_textaddress(tmp,&u->servermainaddr);
        ws_showerror(u,"sendservermain");
        logx(LOG_CONSOLE,"servermainaddr: %s",tmp);
    }
}

void c_sendserver(User *u,uchar *buffer,int bytes)
{
    int error;
    if(lograwpackets && u->state==STATE_CONNECTED)
    {
        fwrite(&bytes,1,4,lograwc);
        fwrite(buffer,1,bytes,lograwc);
    }
    u->bytestoserversec1+=bytes;
    u->bytestoserver+=bytes;
    if(u->dumpsentpacket) if(loglevel>=LOG_PACKETS) c_hexdump("Proxy->Server",buffer,bytes,u);
    u->dumpsentpacket=0;
    error=sendto(u->toserver,buffer,bytes,0,(LPSOCKADDR)&u->toserveraddr,sizeof(SOCKADDR_IN));
    if(error==SOCKET_ERROR) ws_showerror(u,"sendserver");
}

void c_sendclient(User *u,uchar *buffer,int bytes)
{
    int error;
    if(lograwpackets && u->state==STATE_CONNECTED)
    {
        fwrite(&bytes,1,4,lograws);
        fwrite(buffer,1,bytes,lograws);
    }
    u->bytestoclientsec1+=bytes;
    u->bytestoclient+=bytes;
    if(u->dumpsentpacket) if(loglevel>=LOG_PACKETS) c_hexdump("Proxy->Client",buffer,bytes,u);
    u->dumpsentpacket=0;
    error=sendto(consocket,buffer,bytes,0,(LPSOCKADDR)&u->toclientaddr,sizeof(SOCKADDR_IN));
    if(error==SOCKET_ERROR) ws_showerror(u,"sendclient");
}

void c_sendto(SOCKADDR_IN *to,uchar *buffer,int bytes)
{
    int error;
    error=sendto(consocket,buffer,bytes,0,(LPSOCKADDR)to,sizeof(SOCKADDR_IN));
    if(error==SOCKET_ERROR) ws_showerror(NULL,"sendto");
}

int startqwcl_nochecks(char *cmd, char *cmdline)
{
#ifdef WIN32
	char execmdline[2048];
	char exename[1024];
	char exepath[1024];
	char exedir[1024];
    char *exepathname;
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    if(strlen(cmd)>256) return(5);
    if(strlen(cmdline)>1024) return(5);

    strcpy(exename,cmd);
    if(strlen(exename)<4 || stricmp(exename+strlen(exename)-4,".exe") )
    {
        strcat(exename,".EXE");
    }
	if(!GetFullPathName(exename, 1024, exepath, &exepathname))
    {
	    return(1);
    }
    strcpy(exedir,exepath);
    exedir[exepathname-exepath]=0;

    strcpy(execmdline,exepath);
    strcat(execmdline," ");
    strcat(execmdline,cmdline);

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if(!CreateProcess(exepath,execmdline,0,0,TRUE,0,0,exedir,&si,&pi))
	{
		return 4;
	}

    WaitForSingleObject(pi.hProcess,INFINITE);
#endif
	return 0;
}

int mainloop_startqw(int doonce)
{ // low priority
#ifdef WIN32
    int ret;
#endif /* WIN32 */

    logx(LOG_DEBUG,"mainloop_startqw started\n");
#ifdef WIN32
    if(!*quakepath) return(0);

    sprintf(quakeoptions+strlen(quakeoptions)," +connect 127.1:%i",proxy_port);

    if(gamespy)
    {
        //strcat(quakeoptions," +exec GameSpyC.cfg");
        printf("Cheapo started from GameSpy.\n");
    }

    printf("Starting QWCL (%s)\n",quakeoptions);
#ifdef PROTECT
    ret=startqwcl(quakepath,quakeoptions);
#else
    ret=startqwcl_nochecks(quakepath,quakeoptions);
#endif
    if(ret)
    {
        if(ret==1) printf("error: Could not find executable.\n");
        else if(ret==2) printf("error: The executable is modified.\n");
        else printf("error: Could not start executable\n",ret);
        cleanup(0);
        return(0);
    }
#ifdef PROTECT
    if(quakemodified)
    {
        printf("warning: Executable is modified (f_modified will report this).\n");
    }
#endif

    if(!noautoexit)
    {
        printf("QWCL has been closed: ");
        cleanup(0);
    }
#endif
    return(0);
}

void setpriorityx(int fast)
{
#ifdef WIN32
    HANDLE thread; // mainloop thread
    HANDLE process; // mainloop thread
    process=GetCurrentProcess();
    thread=GetCurrentThread();

    if(fast)
    {
        SetPriorityClass(process,HIGH_PRIORITY_CLASS);
        if(!priority)
        {
            SetThreadPriority(thread,THREAD_PRIORITY_HIGHEST);
        }
        else
        {
            SetThreadPriority(thread,priority-13);
        }
    }
    else
    {
        SetPriorityClass(process,NORMAL_PRIORITY_CLASS);
        SetThreadPriority(thread,THREAD_PRIORITY_BELOW_NORMAL);
    }
#endif
}

int mainloop_low(int doonce)
{ // low priority
    int     i;
    DWORD   time;
    User   *u;

    logx(LOG_DEBUG,"mainloop_low started\n");

#ifdef WIN32
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);
#endif

    while(!stopping)
    {
        // handle autotimeout
        time=timems();
        if(autotimeout && time>timeout) stopping=1;

        for(i=0;i<usernum;i++)
        {
            u=user+i;
            if(u->state==STATE_NAMELOOKUP)
            {
                logx(LOG_DEBUG,"main lookup\n");
                c_setaddress(&u->servermainaddr,u->connecthere);
                u->state=STATE_TRYING;
            }
        }
        if(doonce) return(0);

        Sleep(250); // 0.25 sec
    }
    return(0);
}

int mainloop_high(int value)
{ // high priority
#if defined(__APPLE__) || defined(MACOSX)
    User        *u = NULL;
#else
    User        *u;
#endif /* __APPLE__ || MACOSX */
    int          bytes;
    SOCKADDR_IN  from;
    unsigned int fromsize=sizeof(from);
    int          error,i;
#if !defined(__APPLE__) && !defined(MACOSX)
    int          cliopen=0;
#endif /* !__APPLE__ && !MACOSX */
    fd_set       readset;
    struct timeval delay;
    DWORD        time;
    static uchar  buffer[BUFSIZE];
    int          hh;
    // stats
#ifdef MAINPROFILE
    DWORD        time1,time2;
    DWORD        nextprinttime=0;
    DWORD        timesum=0,timecnt=0;
    DWORD        timemax=0;
#endif

    logx(LOG_DEBUG,"mainloop_high started\n");

    setpriorityx(2);

    timeout=timems()+timeouttime;

    while(!stopping)
    {
        hh=0;
        FD_ZERO(&readset);
        FD_SET(consocket,&readset);
#ifndef WIN32
        hh=consocket+1;
#endif
        for(i=0;i<usernum;i++)
        {
            if(user[i].state!=STATE_HERE)
            {
                FD_SET(user[i].toserver,&readset);
#ifndef WIN32
                if(user[i].toserver>=hh) hh=user[i].toserver+1;
#endif
            }
        }

        delay.tv_sec=1;
        delay.tv_usec=0;

        if(prioritytoggle) setpriorityx(0);
        error=select(hh,&readset,NULL,NULL,&delay);
        if(prioritytoggle) setpriorityx(1);
//        if(loglevel>=LOG_PACKETS) logx(LOG_PACKETS,"\n\n\nSelect returns with %i\n",error);
        if(error==SOCKET_ERROR)
        {
            ws_showerror(u,"select");
        }

        time=timems();  // milliseconds
#ifdef MAINPROFILE
        time1=timeus();
#endif
        if(FD_ISSET(consocket,&readset))
        {
            int i;
            timeout=time+timeouttime;
            // contact to server port
            bytes=recvfrom(consocket,buffer,sizeof(buffer)-1,0,
                           (LPSOCKADDR)&from,&fromsize);
            buffer[bytes]=0;

            if(bytes==SOCKET_ERROR)
            {
                ws_showerror(u,"connect");
            }
            else
            {
                // check if it is from a connected client
                for(i=0;i<usernum;i++)
                {
                    globalu=u=user+i;
                    u->time=time;

                    if(c_sameaddress(&u->toclientaddr,&from))
                    {
                        if(u->time>u->time_bytestosec)
                        {
                            u->bytestoserversec=u->bytestoserversec1;
                            u->bytestoclientsec=u->bytestoclientsec1;
                            u->bytestoserversec1=0;
                            u->bytestoclientsec1=0;
                            if(u->bytestoserversec>u->bytestoserversecmax) u->bytestoserversecmax=u->bytestoserversec;
                            if(u->bytestoclientsec>u->bytestoclientsecmax) u->bytestoclientsecmax=u->bytestoclientsec;
                            u->time_bytestosec=u->time+1000;
                        }

                        u->time_lastclient=time;

                        if(u->quiet>0) u->quiet--;
                        else u->quiet=0;

                        if(!memcmp(buffer,"\xff\xff\xff\xffgetcha",10))
                        {
                            u->state=STATE_DROPPING;
                            break;
                        }

                        if(u->state==STATE_DISCONNECTING)
                        {
                            q_command(u,"!sdrop");
                        }
                        else if(u->state==STATE_TRYING)
                        {
                            if(u->time>u->time_nexttry) q_command(u,"!try");
                        }
                        else if(u->state==STATE_INIT)
                        {
                            q_command(u,"!init");
                        }
                        else if(u->state==STATE_HERE)
                        {
                            if(u->doconnect)
                            {
                                        if(u->doconnect>1) u->doconnect--;
                                        else
                                        {
                                            q_command(u,"!connect");
                                            u->doconnect=0;
                                        }
                            }
                        }
                        else
                        {
                            if(u->donew)
                            {
                                if(u->donew>1) u->donew--;
                                else
                                {
                                    q_command(u,"!sendnew");
                                    //u->donew=25; // send again in 25 cycles if not cancelled by fullserverinfo
                                    u->donew=0;
                                }
                            }
                        }

                        if(loglevel>=LOG_PACKETS) c_hexdump("\n\nClient->Proxy",buffer,bytes,u);
                        if(u->state>=STATE_CONNECTED)
                        {
                            q_client(u,buffer,bytes);
                        }
                        else
                        {
                            q_proxy(u,buffer,bytes);
                        }

                        break;
                    }
                }

                if(i==usernum)
                { // user not found
                    if(buffer[0]==0xff && buffer[1]==0xff && buffer[2]==0xff && buffer[3]==0xff)
                    {
                        if(loglevel>=LOG_PACKETS) c_hexdump("Someone->ProxyConnect",buffer,bytes,NULL);
                        q_connect(buffer,bytes,&from);
                    }
                    else
                    { // unrecognized packet from someone
                        if(hpymode)
                        {
                            int besti,bestv,value;
                            bestv=0x10000000;
                            besti=-1;
                            // find best match
                            for(i=0;i<usernum;i++)
                            {
                                value=packetmatch(user+i,buffer);
                                if(value<bestv)
                                {
                                    bestv=value;
                                    besti=i;
                                }
                            }
                            if(besti>=0)
                            {
                                u=user+besti;

                                memcpy(&u->toclientaddr,&from,sizeof(SOCKADDR_IN));
                                //q_print(u,"proxy: dynamic ip change detected\n");
                                q_printf(u,"proxy: ip change for %s\n",u->name);
                            }
                        }
                    }
                }
            }
        }

        for(i=0;i<usernum;i++)
        {
            globalu=u=user+i;

            u->time=time;

            if(FD_ISSET(u->toserver,&readset))
            {
                timeout=time+timeouttime;
                bytes=recvfrom(u->toserver,buffer,sizeof(buffer)-1,0,(LPSOCKADDR)&from,&fromsize);
                u->time_lastserver=time;
                buffer[bytes]=0;
                if(bytes==SOCKET_ERROR)
                {
                    ws_showerror(u,"receiveserver");
                }
                else
                {
                    if(loglevel>=LOG_PACKETS) c_hexdump("\n\nServer->Proxy",buffer,bytes,u);
                    u->toserveraddr=from;

                    q_server(u,buffer,bytes);
                }
            }
        }

        // process dead users (also timeouts)
        // process dropping
        for(i=0;i<usernum;i++)
        {
#if !defined(__APPLE__) && !defined(MACOSX)
            static DWORD lasttime;
#endif /* !__APPLE__ && !MACOSX */
            globalu=u=user+i;

            if(u->errors>MAXERRORS)
            { // too many errors (handle as timeout)
                q_command(u,"!errors");
                i=0; // user order has changed
            }
            if(u->time_lastclient>0 && u->time>u->time_lastclient+TIMEOUT)
            { // client timeout
                q_command(u,"!timeout");
                i=0; // user order has changed
            }
            if(u->state==STATE_DROPPING)
            {
                q_command(u,"!kill");
                i=0; // user order has changed
            }
        }

#ifdef MAINPROFILE
        time2=timeus();
        time2-=time1;
        if(time2>timemax) timemax=time2;
        timesum+=time2;
        timecnt++;

        if(time>nextprinttime)
        {
            nextprinttime=time+1000;

            if(timecnt>0) timesum/=timecnt;
            printf("Process time: avg %2ius max %2ius\n",timesum,timemax);

            timemax=0;
            timecnt=0;
            timesum=0;
        }
#endif

        if(autotimeout && time>timeout) stopping=1;
    }
    return(0);
}

int startup(int port)
{
    log_open();
    logx(LOG_DEBUG,"startup %i\n",port);
    ws_init();
    if(ws_openserver(port)) return(1);
    return(0);
}

void cleanup(int type)
{
    static int cleanupdone=0;
    if(cleanupdone) return;
    cleanupdone=1;
    stopping=1;
    log_flush();
    printf("Terminating...\n");
    Sleep(500);
    ws_deinit();
    log_close();
    if(lograwpackets)
    {
       fclose(lograwc);
       fclose(lograws);
    }
#ifdef WIN32
    ExitProcess(0);
#endif
}

void expires_startup(void)
{
#ifdef EXPIRETIME
    int tim;

//    printf("\nTHIS IS A SPECIAL NONPUBLIC TEST VERSION (Sir-BETA).\n");
    printf("\nTHIS IS A SPECIAL NONPUBLIC TEST VERSION, DO NOT DISTRIBUTE.\n");

    time(&tim);
    if(tim>EXPIRETIME+0)
    {
        printf("This version has expired. A newer version should be available.\n");
        exit(3);
    }

#endif
}

void setversions(void)
{
    int    versionnamesrc[16];
    int    versionsrc[16];
    int i,j;
    i=0;
    j=120+0x40800000;
    versionnamesrc[i++]=(j++)+'C';
    versionnamesrc[i++]=(j++)+'h';
    versionnamesrc[i++]=(j++)+'e';
    versionnamesrc[i++]=(j++)+'a';
    versionnamesrc[i++]=(j++)+'p';
    versionnamesrc[i++]=(j++)+'o';
    versionnamesrc[i++]=0;
    i=0;
    j=120+0x40800000;
    versionsrc[i++]=(j++)+'2';
    versionsrc[i++]=(j++)+'.';
    versionsrc[i++]=(j++)+'6';
    versionsrc[i++]=(j++)+'1';
    versionsrc[i++]=0;
    {
        int i;
        char *d;

        d=version;
        for(i=0;i<16;i++)
        {
            if(!versionsrc[i]) break;
            *d++=(versionsrc[i]&255)-(120+i);
        }
        *d=0;

        d=versionname;
        for(i=0;i<16;i++)
        {
            if(!versionnamesrc[i]) break;
            *d++=(versionnamesrc[i]&255)-(120+i);
        }
        *d=0;
    }
}

void convertspyconfig(char *fin,char *fout)
{
    FILE *f1,*f2;
    char buf[256];

    f1=fopen(fin,"rb");
    if(!f1) return;
    f2=fopen(fout,"wb");
    if(!f2)
    {
        fclose(f1);
        return;
    }

    fprintf(f2,"wait\n");
    while(!feof(f1))
    {
        *buf=0;
        fgets(buf,255,f1);
        if(!memcmp(buf,"connect ",8))
        {
            putc('s',f2);
            memcpy(buf,"ay .con ",8);
        }
        fputs(buf,f2);
    }

    fclose(f2);
    fclose(f1);
}

int main2(int argc,uchar *argv[])
{
    int port;
    int error=0,a,showinternal=0;
    int mset=0,aset=0,csset=0,nset=0,pwset=0;
    uchar *p;

    proxy_serverip.sin_family=AF_INET;
    proxy_serverip.sin_addr.s_addr=INADDR_ANY;
    proxy_serverip.sin_port=27500;

    proxy_clientip.sin_family=AF_INET;
    proxy_clientip.sin_addr.s_addr=INADDR_ANY;
    proxy_clientip.sin_port=0;

    port=27500;
    loglevel=-1;

    setversions();
    for(a=1;a<argc;a++)
    {
        if(*(argv[a])=='/' || *(argv[a])=='-') switch(*(argv[a]+1))
        {
            case 'h' :
            case '?' : if(argv[a][2]=='p') error=2;
                       else error=1;
                       break;

            case '!' : showinternal=1; break;

            case 'p' : p=argv[++a];
                       if(!p) break;
                       port=atoi(p);
                       break;

            case 'm' : p=argv[++a];
                       if(!p) break;
                       maxusernum=atoi(p);
                       if(maxusernum>MAXPLAYERS) maxusernum=MAXPLAYERS;
                       if(maxusernum<1) maxusernum=1;
                       mset=1;
                       break;

            case 'a' : p=argv[++a];
                       if(!p || !*p) break;
                       strncpy(proxy_allowip,p,MAXSTR-1);
                       aset=1;
                       break;

            case 'd' : p=argv[++a];
                       if(!p || !*p) break;
                       if(proxy_denyipnum<16)
                       {
                            strncpy(proxy_denyip[proxy_denyipnum],p,MAXSTR-1);
                            proxy_denyipnum++;
                       }
                       aset=1;
                       break;

            case 'c' : p=argv[++a];
                       if(!p || !*p) break;
                       c_setaddress(&proxy_clientip,p);
                       csset=1;
                       break;

            case 'o' : p=argv[++a];
                       if(!p || !*p) break;
                       strcpy(proxy_name,p);
                       nset=1;
                       break;

            case 'w' : p=argv[++a];
                       if(!p || !*p) break;
                       strcpy(proxy_password,p);
                       pwset=1;
                       break;

            case 's' : p=argv[++a];
                       if(!p || !*p) break;
                       c_setaddress(&proxy_serverip,p);
                       csset=1;
                       break;

            case 'q' : loglevel=-1;
                       break;

            case 'x' : noautoexit=1;
                       break;

            case 'n':  autotimeout=0;
                       break;

    //        case 'z':  newsocketmode=0;
    //                   break;

            case 'y':  hpymode=1;
                       break;

            case 'j':  prioritytoggle=1;
                       priority=13;
            case 'i' : p=argv[++a];
                       if(!p) break;
                       priority=atoi(p);
                       if(priority<11 || priority>15)
                       {
                            printf("WARNING: -i parameter should be in range 11..15\n");
                       }
                       break;

    //    case 'i':  litemode=FPD_TIMERS|FPD_PERCENTE|FPD_SKIN;
    //           break;

            case 'l' : p=argv[++a];
                       if(!p) break;
                       loglevel=atoi(p);
    #ifdef PROMODE
                       if(loglevel==-1)
                       {
                           loglevel=2;
                           lograwpackets=1;
                           lograwc=fopen("client.raw","wb");
                           lograws=fopen("server.raw","wb");
                           printf("Logging raw packets to CLIENT.RAW, SERVER.RAW\n");
                           if(!lograwc) exit(1);
                           if(!lograws) exit(1);
                       }
    #endif
                       break;
            default :
                    error=1;
                    break;
        }
        else
        {
#ifdef WIN32
            char *p;
            strcpy(quakepath,argv[a]);
            strcpy(quakeoptions,strstr(GetCommandLine(), argv[a]));
            if(p=strstr(quakeoptions,"+exec GameSpy.cfg"))
            {
                *p=0;
                gamespy=1;
                convertspyconfig("id1\\GameSpy.cfg","id1\\GameSpyC.cfg");
            }
#else
            error=1;
#endif
            break;
        }
    }

    if(litemode)
    {
        strcat(version," lite");
    }

    proxy_serverip.sin_port=htons((unsigned short)port);

#ifdef PROMODE
    versiontype=" open-pro";
#else
    versiontype=" open";
#endif

    printf("\n"
           "%s V%s%s - Freeware QuakeWorld Proxy - (C) 1998-1999 Sami Tammilehto.\n"
           "Unsupported software. Absolutely no warranty. Use at your own risk.\n"
           ,versionname,version,versiontype);

    expires_startup();
    if(error)
    {
        printf("\n"
#ifdef WIN32
               "usage: cheapo [options...] [qwcl.exe path] [qwcl options]\n"
#else
               "usage: cheapo [options...]\n"
#endif
               "options:\n"
               "-p <port>   port to listen to (default 27500)\n"
               "-m <num>    set maxclients (1-16)\n"
               "-o <name>   server host name\n"
               "-w <pass>   password required to connect\n"
               "-s <ip>     server interface (which ip proxy listens)\n"
               "-c <ip>     client interface (which ip proxy uses to connect)\n"
               "-a <ip>     only accept connections with ip addresses starting like this\n"
               "            (so -a 192.168 would accept connections only from 192.168.x.x)\n"
               "-d <ip>     deny connections from ip addresses (like -a). Multiple allowed.\n"
#ifdef WIN32
               "-i <pri>    set windows priority (11-15)\n"
               "-j          drop priority for select\n"
#endif
               "-l <level>  log to file proxy.log, level is 0..2\n"
               "-q          quiet; no screen output after startup\n"
               "            (now default, use '-l 0' to enable connect/disconnect reports)\n"
               "-n          no timeout (otherwise timeouts after 4 hours of inactivity)\n"
               /*
               "-z          use older TCP/IP routines (like in V2.4) instead of the new ones\n"
               "            (if you get problems or more lag with this new version)\n"
               */
               "-x          no proxy exit when QW exits (and has been started by the Proxy)\n"
               "-y          allow client IP changes (so that a reconnect using dynamic\n"
               "            IP numbers doesn't disconnect you from the proxy)\n"
               "\n"
               "You can automatically start QuakeWorld by specifying the QWCL path and options\n"
               "like 'cheapo c:\\quake\\qwcl.exe'. This is required for f_modified to work.\n"
               "When you quit QWCL in this mode, the proxy also exits (unless -x is used).\n"
               "\n");

        return(0);
    }

    printf("\n");

    if(csset)
    {
        c_textaddress(tmpbuf,&proxy_serverip);
        if(*tmpbuf!='-') printf("Server Interface %s. ",tmpbuf);
        c_textaddress(tmpbuf,&proxy_clientip);
        if(*tmpbuf!='-') printf("Client Interface %s. ",tmpbuf);
        printf("\n");
    }
    if(nset) printf("Proxy hostname '%s'.\n",proxy_name);
    if(hpymode)
    {
        if(maxusernum>1) printf("Dynamic client IP changes allowed.\n");
        else printf("Dynamic client IP changes allowed.\n");
    }
    if(pwset) printf("Proxy password set.\n");
    if(mset) printf("Max %i users allowed to connect.\n",maxusernum);
    if(aset) printf("IP checking (-a/-d) enabled.\n");
    if(priority) printf("Using adjusted windows priority.\n");
    if(autotimeout) printf("Proxy exits after 4 hours of inactivity (-n disables autoexit)\n");
#ifndef PACKETLOG
    if(loglevel>2)
    {
        loglevel=2;
        printf("NOTE: maximum loglevel is 2\n");
    }
#endif
    if(loglevel>0) printf("Logging to \'proxy.log\' (loglevel %i).\n",loglevel);

#ifdef PROMODE
    printf("compiled with: PROMODE\n");
#endif
#ifdef COMPRESSION
    printf("compiled with: COMPRESSION\n");
#endif
#ifdef PACKETLOG
    printf("compiled with: PACKETLOG\n");
#endif
#ifdef IDENT
    printf("compiled with: IDENT\n");
#endif
#ifdef SPEED
    printf("compiled with: .SHOW\n");
#endif

#ifdef EXPIRETIME
    printf(betanotes);
#endif

    if(startup(port))
    {
        printf("Could not bind to port %i.\n\n",port);
        return(1);
    }

    printf("Listening to port %i (press CTRL-C to stop).\n",port);
    proxy_port=port;
    fflush(stdout);

    originalloglevel=loglevel;

    timeout=timems()+timeouttime;

#ifdef WIN32
    {
        HANDLE thread; // mainloop thread
        DWORD threadid;

        SetConsoleCtrlHandler((PHANDLER_ROUTINE)cleanup,TRUE);

        if(newsocketmode)
        {
/*
            thread=CreateThread(NULL,        // security settings
                                0,           // stack size
                                (LPTHREAD_START_ROUTINE)mainloop_high_newbug, // routine
                                NULL,        // parameter (not used)
                                0,           // creationflags
                                &threadid);  // thread id
*/
        }
        else
        {
            thread=CreateThread(NULL,        // security settings
                                0,           // stack size
                                (LPTHREAD_START_ROUTINE)mainloop_high, // routine
                                NULL,        // parameter (not used)
                                0,           // creationflags
                                &threadid);  // thread id
        }
//        SetThreadPriority(thread,THREAD_PRIORITY_HIGHEST);

        if(*quakepath)
        {
            thread=CreateThread(NULL,        // security settings
                                0,           // stack size
                                (LPTHREAD_START_ROUTINE)mainloop_startqw, // routine
                                NULL,        // parameter (not used)
                                0,           // creationflags
                                &threadid);  // thread id
        }

        mainloop_low(0);
    }
#else
    mainloop_high(0);
#endif

    Sleep(1000);
    cleanup(0);

    return(0);
}

/******************************************/

void statecrc(User *u,char *result)
{
#ifdef PROTECT
    // REMOVED: 3 letter version/connection checksum
#else
    strcpy(result,"<open>");
#endif
}

void codecrc(char *salt,char *result)
{
#ifdef PROTECT
    // REMOVED: code/fmodified checksum
#else
    strcpy(result,"<open>");
#endif
}

#if defined(__APPLE__) || defined(MACOSX)

int main(int argc,char **argv)

#else

int main(int argc,uchar *argv[])

#endif /* __APPLE__ || MACOSX */

{
#ifdef PROTECT
    // REMOVED: exe integrity check
#endif

#if defined(__APPLE__) || defined(MACOSX)

    return(main2(argc,(uchar **) argv));

#else

    return(main2(argc,argv));

#endif /* __APPLE__ || MACOSX */

}

