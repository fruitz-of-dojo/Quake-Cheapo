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

//---------------- configuration -------------------------------

//#define PROMODE
// enable debug/development features:
// - .debug command (includes misc stuff and cheat tests)
// - enables logging levels 3-5 (4+=full protocol log to proxy.log)
// - timing for all items (not just powerups)
// note: compiling with promode results in a different f_modified checksum.

//#define RLPACK
// If you point to a pack left by a player that you saw fired
// a rocket just before death, the pack is tagged as 'rl-pack'.
// This proved way too inaccurate (like 1 pack in 20 with luck
// would be classified rl-pack) for practical use, and was also
// too cheatish, so it wasn't published or used.

//#define IDENT
// enable ident of teammates. Pointing a teammate and pressing '.ident'
// shows stats for that player (f_report) in your hud without usin
// messages. This feature didn't prove too useful, is complicated,
// and requires everyone uses Cheapo so it was never published.

//#define POWERDROP
// allows .rep power2 which reports separately when powerups are
// picked and when they time out. Decided not to make this public
// since at the time powerup timers were going away.

//#define COMPRESS
// enable compression. Also requires PROMODE (.debug com enables)
// Compress.c only contains compression stubs (compress_raw and
// uncompress_raw) that don't really compress anything. I didn't
// have time to develop a compression protocol, so only the
// framework is in place. I did find out that you need a very
// specialized algorithm (since the packets are so small and
// they may be dropped) like the excellent one in Qizmo to get
// any useful results.

//#define SPEED
// '.show speed' and '.show time' commands which allow for
// timing routes and checking movement speed (buggy)

//#define TIMERS
// enable powerup timers

//#define MAINPROFILE
// enable some performance profiling

//#define EXPIRETIME 920316146  // 1.3.1998
// expiration time, used for beta versions

//#define PROTECT
// enable f_modified, f_version, exe integrity check
// (code for these is not present in open version)

//#define UNIXALPHA
// use settings suitable for Alpha unix
// Windows/unix selection is based on define WIN32 which
// must be used in windows.

//---------------- other defines based on above ----------------

#ifdef PROMODE
#define PACKETLOG
// enable logging of qw protocol in decoded form
#endif

#ifndef WIN32

#ifdef UNIXALPHA
#define VERCODE "a"
#define ALPHA
#undef PROTECT
#else
#define VERCODE "l"
#endif

// what to use for real time clock

// UNIX settings
#define GETTIMEOFDAY // linux: use gettimeofday system call
//#define FTIME     // alpha: use ftime system call
//#define HRTIME    // sparc: use sparc high resolution timer
#define HILO        // all: unknown byte ordering

#else

#define VERCODE "w"

// what to use for real time clock

// WINDOWS settings
//#define FTIME       // use ftime system call
//#define WINTIME     // use windows getcurrenttime
#define PERFTIME      // use performance counter

#endif

//----------------------------------------------------------------

#ifdef WIN32

#include <windows.h>
#include <sys/timeb.h>

#else

/*#include <sys/timeb.h>*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>

#if defined(__APPLE__) || defined(MACOSX)

#include <unistd.h>

#endif /* __APPLE__ || MACOSX */

#define SOCKET        int
#define SOCKET_ERROR  -1
#define SOCKADDR_IN   struct sockaddr_in
#define DWORD         unsigned int
#define INVALID_SOCKET -1
#define LPSOCKADDR    struct sockaddr *
#define closesocket   close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <math.h>

typedef unsigned char	uchar;

#define LOG_CONSOLE  1    // messages also shown at console
#define LOG_VERBOSE  2    // more info
#define LOG_DEBUG    3    // debug messages
#define LOG_PACKETS  4    // packet traffic dump
#define LOG_FLUSH    5    // packet traffic dump with fflush
#define MAXPLAYERS     16
#define MAXUSERS       (MAXPLAYERS+2)

#define CLIENTINFOSIZE 512
#define APPENDSIZE     1024  // max data to append to packets
#define MAXSTR         256
#define BUFSIZE        16384 // network buffers
#define MAXPLAYERNAME  32
#define MAXSKINNAME    32
#define MAXPROXYNAME   32

#define MAXMODELS      256
#define MAXSOUNDS      256
#define MAXENTITIES    512
#define STATNUM        256 // pinghistory
#define MAXCAMP        256 // number of objects

#define TIMEOUT        60000 // 60 sec timeout
#define MAXERRORS      64 // if more than this net errors, autodisconnect

#define SEQUENCES      8 // impulse sequences

#define TOOKRADIUS     900
#define NEARDIST       6000 // %o,%e

#define CAMP_P         0x0001
#define CAMP_Q         0x0002
#define CAMP_R         0x0004
#define CAMP_LASTPOWER 0x0004
#define CAMP_MH        0x0008
#define CAMP_RA        0x0010
#define CAMP_YA        0x0020
#define CAMP_GA        0x0040
#define CAMP_SUIT      0x0080
#define CAMP_LASTARMOR 0x0080
#define CAMP_RL        0x0100
#define CAMP_LG        0x0200
#define CAMP_GL        0x0400
#define CAMP_LASTAUTOMARK 0x0400
#define CAMP_SNG       0x0800
#define CAMP_NG        0x1000
#define CAMP_SSG       0x2000
#define CAMP_RUNE      0x4000  // also key and flag
#define CAMP_HEALTH    0x8000
#define CAMP_CELLS     0x10000
#define CAMP_ROCKETS   0x20000
#define CAMP_NAILS     0x40000
#define CAMP_SHELLS    0x80000
#define CAMP_PLAYER    0x100000
#define CAMP_GLOW      0x100000 // glowing player
#define CAMP_PACK      0x200000
#define CAMP_POINTED   0x400000 // last pointed item
#define CAMP_ENEMY     0x800000
#define CAMP_LAST      0x1000000

#define CAMP_DEF     (CAMP_PLAYER|CAMP_RA|CAMP_GA|CAMP_YA|CAMP_MH|CAMP_Q|CAMP_R|CAMP_P|CAMP_PACK|CAMP_RL|CAMP_GL|CAMP_LG|CAMP_RUNE|CAMP_SNG|CAMP_ROCKETS)
#define CAMP_DEFTOOK (CAMP_RA|CAMP_YA|CAMP_RL|CAMP_LG|CAMP_RUNE)
#define CAMP_POWER   (CAMP_R|CAMP_P|CAMP_Q)

// quakeworld 32 bit items word
#define ITEMS_SG       0x00000001
#define ITEMS_SSG      0x00000002
#define ITEMS_NG       0x00000004
#define ITEMS_SNG      0x00000008
#define ITEMS_GL       0x00000010
#define ITEMS_RL       0x00000020
#define ITEMS_LG       0x00000040
#define ITEMS_GA       0x00002000
#define ITEMS_YA       0x00004000
#define ITEMS_RA       0x00008000
#define ITEMS_MEGA     0x00010000
#define ITEMS_SILVER   0x00020000
#define ITEMS_GOLD     0x00040000
#define ITEMS_R        0x00080000
#define ITEMS_P        0x00100000
#define ITEMS_SUIT     0x00200000
#define ITEMS_Q        0x00400000
#define ITEMS_RESIST   0x10000000
#define ITEMS_STRENGTH 0x20000000
#define ITEMS_HASTE    0x30000000
#define ITEMS_REGEN    0x40000000
#define ITEMS_FLAG     (ITEMS_SILVER|ITEMS_GOLD) // flag=keys

typedef struct Camp_t Camp;
struct Camp_t
{
    int    entity; // entity number
    int    pos[3];
    int    delay;  // re appear delay
    int    category;
    int    secondcamp;
    DWORD  appeartime; // 0=unknown
    uchar  name[4];
    uchar  genericname[8];
    uchar *pickupsound;
};
#define UNKNOWNTIME  0xffffffff
#define UNKNOWNTIME2 0xfffffffe

typedef struct Location Location;
struct Location
{
    int   x,y,z;
    char *name;
};

typedef struct User User;
struct User
{
    // communication
    char         proxyhostname[16]; // max 10 chars+port actually
    SOCKET       toserver;
    //SOCKET       toclient;
    SOCKADDR_IN  toserveraddr;
    SOCKADDR_IN  toclientaddr;
    SOCKADDR_IN  servermainaddr;     // server 27500 port
    SOCKADDR_IN  lastservermainaddr; // last server 27500 port
    int          safemode;       // just forward packets, no parsing
    int          sequence;       // last received from client
    int          connectstartsequence; // sequence when connect initiated
    int          server_lastthis;
    int          server_lastrecv;
    int          server_hiddenbit;
    int          server_sendsecure; // command flag
    int          client_lastthis;
    int          client_lastrecv;
    int          client_hiddenbit;
    int          client_sendsecure; // commnd fla
    int          server_ping;
    int          errors; // in net traffic;
    int          version; // 21 or 22
    int          dumpsentpacket;
    int          linkedserver; // linked to a serverside proxy
    int          linkedclient; // linked to a clientside proxy
    int          choke; // for last server packet

    int          noappendnow;
    int          quiet;

    int          challenge;
    int          havechallenge;
    int          response_a;  // version
    int          response_b;  // portid
    int          response_c;  // challenge
    int          clientident; // 16 bit client ident
    // append data to packets
    int          appendclientbytes;
    uchar        appendclient[APPENDSIZE];
    int          appendserverbytes;
    uchar        appendserver[APPENDSIZE];
    // msg buffers for centerprinting
    uchar        msgline[64];
    uchar        notice[64]; // near center of screen
    // timed events & state
    DWORD        time; // current time in milliseconds (set by main)
    DWORD        time_connect;
    DWORD        time_forcedisconnect;
    DWORD        time_lastserver; // last msg from server (timeout detect)
    DWORD        time_lastclient; // last msg from client (timeout detect)
    DWORD        time_nextcamp;   // next camping info on screen
    DWORD        time_nexttarget; // next target check
    DWORD        time_nextreport; // next report
    DWORD        time_reskinall;
    DWORD        time_lastfreport;
    DWORD        time_sendfreport;
    DWORD        time_namelookupmsg;
    DWORD        time_nexttry;    // try next connect at this time
    DWORD        time_lastenemymask;
    DWORD        time_lastoverlay;
    DWORD        time_lasttook;
    DWORD        time_lastpoint;
    DWORD        time_nextcpps;   // ok to send client packet at this time
    DWORD        time_cppsdelta;  // minimum timedelta between packets
    DWORD        time_died;
    DWORD        time_lastfversion;
    DWORD        time_lastsay[4];
    int          state;
// disconnecting/disconnected from proxy
#define STATE_DROPPING     -1
// connecting to proxy
#define STATE_INIT          0
// connected to proxy
#define STATE_HERE          1
// doing namelookup from connecthere=>servermainaddr
#define STATE_NAMELOOKUP    2
// trying to connect to servermainaddr
#define STATE_TRYING        3
// connected to a server
#define STATE_CONNECTED     4
// dropping connection
#define STATE_DISCONNECTING 5
    // connecting control variables
    int          doconnect;           // when in STATE_HERE, connect to addr below
    int          donew;
    int          isreconnect;         // this connect is a reconnect
    uchar        connecthere[MAXSTR]; // ip address for next connect
    int          connectingcount;     // try this many times
    int          challengecount;      // if 0 say server not responding
    int          dropcount;           // how long to wait for server disconnect after drop
    int          firsttry;
    int          servertimeoutmsg;    // flag
    int          parsecount;
    // operation modes
    uchar        proxysay[32]; // "say proxy:"
    uchar        proxysay2[32]; // "say <host>:"
    int          fps_explosion;
    int          fps_powerup;
    int          fps_muzzle;
    int          fps_gib;
    int          fps_rocket;
    int          fps_setinfo; // tbd
    int          fps_nails;
    int          fps_damage;
    int          usepercentn;
    int          campthese;
    int          pointshow;
    int          pointtell;
    int          remindcamp;
    int          secondcamp;
    int          cpps;
    int          linesdown;
    int          linesright;
    int          linecenter;
    int          tellteam;
    int          floodprot;
    int          specmode;
    int          specentity;
    int          bestweapon;
    int          autofreport;
    int          autoihave;
    int          autoitook;
    int          autoitookitems;
    int          autoqpicked;
    int          moveoverlay;
    int          soundtrigger;
    int          classifypacks;
    int          fmodified;
    int          forcedmm; // 0=no force (use server)
//    int          reportthese;
//    int          reporttime; // report this many seconds before item
//    int          pickupthese;
    char         impulseorder[SEQUENCES][16]; // modifiers for impulses (first number is impulse, 0=unused)
    int          notimers;

    // skin settings
    int          skinoverride;
    int          coloroverride;
    int          skinoverridepos; // for sending a few skins per packet
    int          rankoverridepos; // for sending a few skins per packet
    char         skinname[16];    // ours
    char         skinname2[16];   // others
    int          color;
    int          color2;

    // compress settings
    int          compress_client;
    int          compress_server; // set if incoming compressed client messages
    int          compress_test;

    // debug settings
    int          nofversion;
    int          fakeping;
    int          fakedisablecnt;
    int          zztest;
    int          showtime;
    int          showspeed;
    int          showhud;
    DWORD        showtimebase;
    int          showdist;
    int          showdistbase[3];
    int          hideaxemen;
    int          turbo;
    int          fastmove;
    int          shoot;
    int          stoptime;

    // infomessages
    DWORD        time_request;
    int          requestfrom; // 0=nothing now
    int          clearrequest;
    char         requestformat[64];

    // client info
    int          amconnected; // to a server
    int          amspectator;
    uchar        overlay[256]; // server text overlay
    int          overlayactive; // our overlay
    uchar        name[MAXSTR];
    int          team,zero2;
    int          tcolor,bcolor;
    int          lastbegin; // last begin x command
    uchar        clientinfo[CLIENTINFOSIZE];
    int          entity;
    int          health;
    int          armor;
    int          weapon;
    int          ammo;
    int          ammo_shells;
    int          ammo_nails;
    int          ammo_rockets;
    int          ammo_cells;
    int          items;
    int          pos[3]; // z=altitude (pos up)
    int          ang[3]; // 16bit; 0=tilt (negative up), 1=yaw (negative left)
    int          speed[4];
    int          speedlastthis;
    int          speedcnt;
    char         filter[16][16];
    int          filternum;

    int          diedatloc; // near what loc you died last time
    int          diedatown;
    int          diedatenemy;
    int          lastreportedloc;

    int          power; // powerups, smae bits as items
    int          lastpower; // same bits as items
    int          lastnonzeropower; // same bits as items

    // history of actions
    uchar        lasttarget[64];
    uchar        lastpoint[64];
    uchar        lasttook[64];
    int          lastentitypicked;
    int          lastentitypointed;

    // how many soundlists / modellists received
    int          cnt_sound;
    int          cnt_model;

    int          lastflag; //fire/jump
    int          impulsehistory[3];

    int          client_spectrack;
    // level info
    int          changing;
    int          fortress;
    int          fpdcopy;
    int          dmm;
    int          originaldmm;
    int          dmmitemmask;
    int          ctf;
    int          fpd;
#define FPD_PERCENT      0x01
#define FPD_TIMERS       0x02
#define FPD_SOUND        0x04
#define FPD_PERCENTE     0x20
#define FPD_POINT        0x80
#define FPD_SKIN         0x100
#define FPD_COLOR        0x200
#define FPD_ITEM         0x400  // not implemented right now
    int          wherecrc;
    int          models;
    uchar       *model[MAXMODELS];
    int          grenademodel;
    int          pack2model;
    int          sounds;
    uchar       *sound[MAXMODELS];
    int          explosionsound;
    int          camps;
    Camp         camp[MAXCAMP];
    int          campcnt[16]; // 3=ring, 4=pent, 5=quad
    char         gamedir[MAXSTR];
    // dynamically allocated locations
    int          locations;
    int          locationsmax; // this many allocated
    Location    *loc;
    uchar        locname[MAXSTR];
    // tracking smooth
    int          tr_pos[3];
    int          tr_ang[3];
    int          tr_postar[3];
    int          tr_angtar[3];
    // other players info (indexed by entity, not player number)
    uchar        playername[34][MAXPLAYERNAME];
    uchar        playerskin[34][MAXSKINNAME];
    uchar        playerproxy[34][MAXPROXYNAME];
    uchar        playerteamtext[34][8];
    int          playerfps[34];
    int          playerteam[34];
    int          playerbcolor[34];
    int          playertcolor[34];
    int          playerxcolor[34];
    int          playerrcolor[34]; // rank color
    int          playerfrags[34];
    int          playertime[34];
    int          playerfragsbest; // leader frags
    int          playernear[34];
    int          playerglow[34]; // low 4 bits=weapon!?
    int          playeractive[34]; // 0 for spec / unused players
    int          playerpos[34][3];
    int          playerang[34][3];

    int          trackloc;
    int          tracklocind;
    int          playerloc[34];
    int          playerlastloc[34];

    int          numplayers; // 1+last active player
    DWORD        time_playerrocket[34]; // last time rocket fired
    DWORD        time_playerdied[34];
    int          lastenemymask; // powerups
    // visible entities
    char         entitygib[MAXENTITIES];
    char         entityvisible[MAXENTITIES];
    uchar *      entityname[MAXENTITIES];
    int          entitypos[MAXENTITIES][3];
    int          entitymask[MAXENTITIES];

    // statistics
    int          packetsendtime[STATNUM];
    int          packetrecvtime[STATNUM];
    int          packetchoke[STATNUM];
    int          packetserversizecnt;
    int          packetserversize[16];
    int          packetclientsizecnt;
    int          packetclientsize[16];
    int          cppsdrop,cppspass;
    int          outoforder;
    int          outoforder30;

    // client sizes measured at output
    int          packetclientinsize[16];  // compressed
    int          packetclientoutsize[16]; // compressed
    // server sizes measured at input
    int          packetserverinsize[16];  // compressed
    int          packetserveroutsize[16]; // compressed

    int          lastpacketrecv;

    int          bytestoserver;
    int          bytestoclient;
    DWORD        time_bytestosec;
    int          bytestoserversec1; // tmp counters
    int          bytestoclientsec1; // tmp counters
    int          bytestoserversec; // val for last full second
    int          bytestoclientsec; // val for last full second
    int          bytestoserversecmax; // val for last full second
    int          bytestoclientsecmax; // val for last full second
};

extern int   litemode; // = FPD mask
extern int   hpymode;
extern int   usernum;
extern int   maxusernum;
extern int   newsocketmode;
extern User  user[MAXUSERS];

typedef struct
{
    int     type;
#define USERINFO_REPLY        0x00
#define USERINFO_REQUEST      0x01
#define USERINFO_PICKUP       0x02
#define USERINFO_EXTENDED     0x07
#define USERINFO_ERROR        0xff
    int     requestfrom;
    int     requestlong;
    // sent in short
    int     armor;
    int     armortype; // 0=none, 1=green, 2=yellow, 3=red
    int     health;
    int     bestweapon;
    int     bestammo;
    int     items;
    int     power;
    // sent in long
    int     x,y,z;
    // not sent right now
    int     weapon;
    int     ammo;

    int     own;
    char   *name;
} Userinfo;

void q_command(User *u,uchar *s);

void q_connect(uchar *inbuf,int inbytes,SOCKADDR_IN *from); // data to proxy connect port
void q_client(User *u,uchar *buffer,int bytes);   // data to client
void q_server(User *u,uchar *buffer,int bytes);   // data to server
void q_proxy(User *u,uchar *uffer,int bytes);     // data to proxy (server before connect)

int  packetmatch(User *u,uchar *buffer);

int   c_islocalconnect(SOCKADDR_IN *a);
void  c_hexdump(uchar *msg,uchar *buffer,int bytes,User *u);
User *c_finduser(SOCKADDR_IN *from);
User *c_newuser(SOCKADDR_IN *from);
void  c_killuser(User *u);
void  c_killserver(User *u);
void  c_reopenserver(User *u);
int   c_sameaddress(SOCKADDR_IN *a,SOCKADDR_IN *b);
void  c_setaddress(SOCKADDR_IN *a,uchar *text);
void  c_textaddress(uchar *text,SOCKADDR_IN *a);
void  c_sendservermain(User *u,uchar *data,int bytes);
void  c_sendserver(User *u,uchar *data,int bytes);
void  c_sendclient(User *u,uchar *data,int bytes);
void  c_sendto(SOCKADDR_IN *a,uchar *data,int bytes);

void  logx(int level,uchar *txt,...);

int   intat(uchar *a);

//-----------------------------------------------

char *_first_(void);
char *_last_(void);

//------------- stuff in main.c -----------------

extern User  *globalu;
extern int loglevel;
extern int lognpackets;
extern int oldloglevel;

extern uchar *version;
extern uchar *version2;
extern uchar *versionname;
extern uchar *versiontype;
extern int    filecrc[2];

void expires_gettime(void);
void expires_startup(void);
void expires_check(void);

void *mymalloc(int size);
void  myfree(void *p);

void codecrc(char *salt,char *result); // 16 byte result
void statecrc(User *u,char *result); // 16 byte result

//------------- stuff in proxy.c -----------------

extern int   proxy_port;
extern int   challenge;
extern uchar proxy_allowip[MAXSTR];
extern uchar proxy_denyip[16][MAXSTR];
extern int   proxy_denyipnum;
extern uchar proxy_name[MAXSTR];
extern uchar proxy_password[MAXSTR];
extern char  quakepath[MAXSTR];
extern int   quakehandle;
extern char  quakeoptions[MAXSTR];
extern int   quakemodified;
extern int   gamespy;
extern uchar tmpbuf[BUFSIZE];
extern uchar outbuf[BUFSIZE];
int handlesaying(User *u,Userinfo *ui,uchar **d0,uchar **s0);

void writeheader(uchar *inbuf,int *seqthis,int *seqrecv,int *bitthis,int *bitrecv);
void readheader(uchar *inbuf,int *seqthis,int *seqrecv,int *bitthis,int *bitrecv);

//------------- stuff in misc.c -----------------

void strxor80(char *t);
void stror80(char *t);
void strand80(char *t);
int  memcmp7f(char *a,char *b,int len);

void   synccamp(User *u,int sync,int num);

void   userinfofromuser(Userinfo *ui,User *u);
void   gotinfomessage(User *u,uchar *msg,int entity);
void   sendinfomessage(User *u,uchar *msg);
void   sendsetinfo(User *u,uchar *msg);
void   decryptinfo(Userinfo *ui,uchar *datain);
uchar *encryptinfo(Userinfo *ui);

int    checksum(char *packetstart,char *data,int size);
void   saveparseerror(char *text,int pos,uchar *buf,int bytes);
int    funnyconvert(uchar *d,uchar *s,int nocolor);
uchar *formatqstring(uchar *in);
uchar *extractfield(uchar *dest,uchar *src,uchar *field);
void   funnyhelp(User *u);

void   q_growlocmax(User *u,int num);
void   q_setlocname(User *u,int i,char *name);
void   q_loadloc(User *u,uchar *mapname);
void   q_savelocs(User *u);

void   q_print(User *u,uchar *text);
void   q_printf(User *u,uchar *text,...);
void   q_explode(User *u,int type,int x,int y,int z);
void   q_print1(User *u,uchar *text);
void   q_stufftext(User *u,uchar *text);
void   q_servercmd(User *u,uchar *text);
void   q_centerprint(User *u,uchar *text);

char *playerlocdirection(User *u,int pl);
int    teamint(uchar *a);
int    is_version(uchar *x);
int    is_timer(uchar *x);
int    is_modified(uchar *x);
void   createversiontext(User *u,uchar *txt);
int    modifyimpulse(User *u,int *impulse);
uchar *findnear(User *u,int x,int y,int z,int *ind);
uchar *findpoint(User *u,int x,int y,int z,int ax,int ay,int az,int targetmask,int flags,int *entitynum);
#define POINT_ONLYNAME 1
#define POINT_ONLYPOS  2
#define POINT_ADDSKIN  4 // fortress only
#define POINT_TEAMMATENUMBER 8
#define POINT_DIRECTION 16
#define POINT_SELF 32
uchar *findnearestitem(User *u,int x,int y,int z,int targetmask);
int text2color(char *p);
char *color2text(int c);

void   rankchange(User *u,int i,int dooverride);
void   skinoverride(User *u,int i);
void   spectatenext(User *u,uchar *findname);
void   cl_clearstats(User *u);
void   cl_updatestat(User *u,int stat,int value);
void   cl_setinfo(User *u,uchar *attr,uchar *value);
Camp  *campadd(User *u,int entity,int *pos,int modelindex,int skin);
char  *makeentityname(User *u,int modelindex,int skin,int *mask);
void   reportfpd(User *u,int fpd);
void setinfo(User *u,int i,char *attr,char *value);

int    floodprot(User *u);
void   dmmchange(User *u,int newdmm);
void showcrcerror(int showinternal,int crc8,int crc24);
//------------- stuff in cmd.c -----------------

void   q_help(User *u,uchar *param);
void   q_command(User *u,uchar *s);

//------------- misc macros ---------------------

#define SAMETEAM(x) ((!u->fortress && u->team==u->playerteam[x]) || (u->fortress && u->bcolor==u->playerbcolor[x]))
// you in same team as player x
#define NEWQW (u->version>=22)

//------------- compress.c ---------------------

void   compresstest(User *u,uchar *buf,int bytes);
void   compress(User *u,uchar *inbuf,int *inbytes,int server);
void   uncompress(User *u,uchar *inbuf,int *inbytes,int server);

void count_save(User *u);
void count_load(User *u);

#if defined(__APPLE__) || defined(MACOSX)

extern	int mainloop_low(int);
extern	int memicmp(uchar *,uchar *,int);
extern	int strlwr(uchar *);
extern	int stricmp(uchar *,uchar *);

#endif /* __APPLE__ || MACOSX */




