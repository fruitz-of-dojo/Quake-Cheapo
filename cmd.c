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

//const char *_first_(void)
char *_first_(void)
{
    static char firststring[]="<<<<FFFFFFFF>>>>";
//    char *firststring="<<<<FFFFFFFF>>>>";
    return(firststring);
}

void q_help(User *u,uchar *param) //??
{
    if(*param=='.') param++;
    if(!memicmp(param,"fun",3))
    {
        funnyhelp(u);
    }
    else if(!memicmp(param,"ascii",5))
    {
        int i;
        for(i=0x01;i<0x20;i+=4)
        {
            q_printf(u,"#%02X:%c #%02X:%c #%02X:%c #%02X:%c\n",
                    i+0,i+0,
                    i+1,i+1,
                    i+2,i+2,
                    i+3,i+3);
        }
    }
    else if(!memicmp(param,"con",3))
        q_print(u,".connect <address>\n"
                  "Connects to a server or a proxy.\n"
                  "If linking to another proxy, the\n"
                  "first proxy in the chain will\n"
                  "respond to .command, and the\n"
                  "last proxy to proxy:command. All\n"
                  "respond to hostname:command.\n"
                  );
    else if(!memicmp(param,"dis",3))
        q_print(u,".disconnect\n"
                  "Disconnects from a server or\n"
                  "a proxy.\n"
                  );
    else if(!memicmp(param,"rec",3))
        q_print(u,".reconnect\n"
                  "Reconnects to the last address\n"
                  "connected to.\n"
                  );
    else if(!memicmp(param,"use",3))
        q_print(u,".users\n"
                  "Lists users connected to this\n"
                  "proxy and the address they are\n"
                  "connected to.\n"
                  );
    else if(!memicmp(param,"nam",3))
        q_print(u,".name <name>\n"
                  "This can be used to rename the\n"
                  "proxy, so that it responds to\n"
                  "commands which start <name>:\n"
                  "proxy: or host: is no longer\n"
                  "recognized after this command.\n"
                  );
    else if(!memicmp(param,"fil",3))
        q_print(u,".filter <group> <group> ...\n"
                  "This commands allows dividing a\n"
                  "team into groups and sending\n"
                  "messages only to a single group.\n"
                  "All groupnames must be short and\n"
                  "start with #. Here's an example:\n"
                  "  player1 .filter #att\n"
                  "  player2 .filter #def\n"
                  "After this a message ending with\n"
                  "' #att' like 'say_team go! #att'\n"
                  "is seen only by player 1. Texts\n"
                  "with no group are seen by all,\n"
                  "and people without no filters\n"
                  "see all messages as well. You\n"
                  "can belong to multiple groups,\n"
                  "but a message can only have one\n"
                  "group. All text after '#' is\n"
                  "removed from the message, and\n"
                  "there must be a space before '#'.\n"
                  );
    else if(!memicmp(param,"tra",3))
        q_print(u,".track <player>"
                  "Gives a 1st-person tracking view\n"
                  "of <player>. Must be a spectator\n"
                  "and in fly-mode. NOTE: QW 2.3 has\n"
                  "a cl_chasecam 1 option that gives\n"
                  "you tracking without a proxy.\n"
                  );
    else if(!memicmp(param,"cam",3))
#ifdef TIMERS
        q_print(u,".camp <items...>\n"
                  "Selects which items are timed:\n"
                  "  none, quad, penta, ring, all\n"
                  "See also .remind and .guess\n"
                  );
#else
        q_print(u,".camp <items...>\n"
                  "Timers are no longer available in\n"
                  "Cheapo, and this command is obsolete.\n"
                  );
#endif
    else if(!memicmp(param,"rem",3))
#ifdef TIMERS
        q_print(u,".remind <time>\n"
                  "Remind <time> seconds before a\n"
                  "powerup appears with a message\n"
                  "near center of the screen. Give\n"
                  "0 to disable remind.\n"
                  );
#else
        q_print(u,".remind <time>\n"
                  "Timers are no longer available in\n"
                  "Cheapo, and this command is obsolete.\n"
                  );
#endif
    else if(!memicmp(param,"gue",3))
#ifdef TIMERS
        q_print(u,".guess <1/0>\n"
                  "Normally camp timers trigger\n"
                  "when somebody picks up a powerup\n"
                  "and you hear it (guess 0). With\n"
                  "guess 1, if you don't hear a\n"
                  "powerup when last counter goes\n"
                  "to zero, the counter is reset\n"
                  "(assuming someone probably picked\n"
                  "the powerup immediately.) The new\n"
                  "counter has a different symbol to\n"
                  "show that the count value is a\n"
                  "guess. Additionally, when someone\n"
                  "says 'i have <powerup>' and you\n"
                  "don't have time for the given\n"
                  "powerup, the counter is reset\n"
                  "with a guess.\n"
                  );
#else
        q_print(u,".guess <1/0>\n"
                  "Timers are no longer available in\n"
                  "Cheapo, and this command is obsolete.\n"
                  );
#endif
    else if(!memicmp(param,"not",3))
#ifdef TIMERS
        q_print(u,".notimers < 1 | 0 >\n"
                  "Disables camp timers. Text 'notimers'\n"
                  "is shown in f_version and .who proxy\n"
                  "when this is active. The purpose is\n"
                  "to allow games where it's easy for\n"
                  "everyone to check that timers are not\n"
                  "used. Notimers mode must be set in the\n"
                  "first proxy if you have linked through\n"
                  "many proxies. You can enable notimers\n"
                  "when connected, but not disable them\n"
                  );
#else
        q_print(u,".notimers < 1 | 0 >\n"
                  "Timers are no longer available in\n"
                  "Cheapo, and this command is obsolete.\n"
                  "Notimers is always enabled.\n"
                  );
#endif
    else if(!memicmp(param,"flo",3))
        q_print(u,".flood <1/0>\n"
                  "Flood protect protect. If enabled\n"
                  "the proxy does not allow you to\n"
                  "trigger floodprotect on the server\n"
                  "(instead the proxy gives the flood\n"
                  "protect warning). The advantage is\n"
                  "that you won't get any flooding\n"
                  "penalty. Server limits are assumed\n"
                  "to be 4 messages in 4 sec. Default\n"
                  "enabled.\n"
                  );
    else if(!memicmp(param,"point2",6))
        q_print(u,"Pointing makes it possible to\n"
                  "report to teammates an item you\n"
                  "see/pick using following in say:\n"
//                  "  %T - item pointed to with move\n"
//                  "       direction for players\n"
                  "  %t - item pointed to\n"
                  "  %i - last item picked up\n"
                  "  %j - last item pointed to\n"
                  "  %k - last item picked/pointed\n"
                  "  %m - last item picked or nearest\n"
                  "  %x - %t but item name only\n"
                  "  %y - %t but location only\n"
                  "The nearest object near center\n"
                  "of the screen is picked for point.\n"
                  "Picking only tracks items you can\n"
                  "point.\n"
                  );
    else if(!memicmp(param,"poi",3))
        q_print(u,".point <items>\n"
                  "This command controls what objects\n"
                  "are taken into account for %tijk:\n"
                  " quad, penta, ring, player, enemy,\n"
                  " ra, ya, ga, mh, suit, pack,\n"
                  " rl, gl, lg, sng, ng, ssg\n"
                  " cell, rock, nail, shel, health\n"
                  " misc (=rune+key+flag+sentry-etc)\n"
                  "aliases:\n"
                  " none  - no items\n"
                  " power - quad penta ring\n"
                  " armor - ra ya ga\n"
                  " all   - everything\n"
                  " def   - default items:\n"
                  "   player, pack, armors, mega, rock\n"
                  "   powerups, lg, rl, ga, sng, misc\n"
                  "see .help point2 for more info.\n"
                  );
    else if(!memicmp(param,"too",3))
        q_print(u,".took <items>\n"
                  "Select what items are reported\n"
                  "with f_took when you collect them.\n"
                  "Items is a list of:\n"
                  " quad, penta, ring, player, enemy\n"
                  " ra, ya, ga, mh, suit, pack,\n"
                  " rl, gl, lg, sng, ng, ssg\n"
                  " cell, rock, nail, shel, health\n"
                  " misc (=rune+key+flag+sentry-etc)\n"
                  "Default:\n"
                  " armors, lg, rl, gl\n"
                  );
    else if(!memicmp(param,"rep",3))
        q_print(u,".report <events>\n"
                  "What events are reported to team\n"
                  "automatically. Possibilites:\n"
                  "none   - nothing \n"
                  "reply  - when someone says something\n"
                  "         containing ':' (as in f_report)\n"
                  "         reply with f_report\n"
                  "power  - when you pick a powerup\n"
                  "         execute f_powerup.\n"
/*
                  "power2 - allows you to specify separate\n"
                  "         commands for each powerup. You"
                  "         must define aliases:\n"
                  "           f_power_quad, f_power_noquad\n"
                  "           f_power_pent, f_power_nopent\n"
                  "           f_power_ring, f_power_noring\n"
*/
                  "took   - when you collect an item\n"
                  "         execute f_took (see .took)\n"
#ifdef TIMERS
                  "sync  - when you hear anyone pick a\n"
                  "        powerup a team message like\n"
                  "        'Quad #1 picked up' is sent.\n"
                  "        These messages are used to\n"
                  "        sync powerup timers, and are\n"
                  "        compatible with Qizmo. NOTE:\n"
                  "        Cheapo hides these messages\n"
                  "        when you play, but they do\n"
                  "        affect the timers.\n"
                  "show  - show&sync. Same as previous\n"
                  "        except that the picked up\n"
                  "        messages are shown. NOTE: if\n"
                  "        all players in a team use\n"
                  "        sync there will be multiple\n"
                  "        messages\n"
#endif
                  );
    else if(!memicmp(param,"lin",3))
        q_print(u,".line <lines | vga | 512 | 640> [center]\n"
                  "Set number of text lines to align\n"
                  "camp timers correctly. Larger\n"
                  "numbers move the texts down.\n"
                  "640 - alias for 640x480 (45)\n"
                  "512 - alias for 512x384 (33)\n"
                  "vga - alias for 320x200 (11)\n"
                  "If the server sends a statusline\n"
                  "(CTF etc) decrease the .line value\n"
                  "by one so the powerups are shown\n"
                  "above the statusline. The final\n"
                  "'center' option centers powerup\n"
                  "timers (default is left aligned)\n"
                  );
    else if(!memicmp(param,"mov",3))
        q_print(u,".move <lines>\n"
                  "Move small overlay texts near\n"
                  "center of screen <lines> lines\n"
                  "downward. Mostly useful to get\n"
                  "that Arena FIGHT! away from the\n"
                  "crosshair (try .move 3) or to\n"
                  "move the remainder (see .remind)\n"
                  );
    else if(!memicmp(param,"ite",3))
        q_print(u,".items\n"
                  "List important items in current\n"
                  "level and their nearest mark.\n"
                  "Useful for checking how many\n"
                  "given items are in a level and\n"
                  "where for example quad2 is.\n"
                  );
    else if(!memicmp(param,"ski",3))
        q_print(u,".skin <myskin> <other>\n"
                  "Overrides skins for all players\n"
                  "so that your team has <myskin>\n"
                  "and other players have <other>.\n"
                  "Default for <other> is base.\n"
                  "This change is not visible to\n"
                  "other players in the server.\n"
                  "Without parameters .skin restores\n"
                  "skins to normal. Skin can also\n"
                  "be specified as * to leave it\n"
                  "unchanged: '.skin * base' forces\n"
                  "base to enemies, but leaves\n"
                  "teammate skins normal.\n"
                  );
    else if(!memicmp(param,"col",3))
        q_print(u,".color <mycolor> <othercolor>\n"
                  "Overrides colors for all players\n"
                  "so that your team has <mycolor>\n"
                  "and other players have <other>.\n"
                  "Default for <other> is 0 (white).\n"
                  "This change is not visible to\n"
                  "other players in the server.\n"
                  "Without parameters .color restores\n"
                  "colors to normal. The same color\n"
                  "is used for top and bottom.\n"
                  "Color can also be specified as *\n"
                  "to leave it unchanged: '.color * 0'\n"
                  "forces white to enemies, but leaves\n"
                  "teammate colors normal. Color can\n"
                  "also have two numbers like 3,4 in\n"
                  "which case the first is topcolor\n"
                  "and the second is bottomcolor.\n"
                  );
    else if(!memicmp(param,"fps",3))
        q_print(u,".fps a b c d e f g h\n"
                  "Gives more fps by removing or\n"
                  "modifying visible events. There\n"
                  "are 8 options each specified by\n"
                  "a value. If less than 8 values\n"
                  "given others are assumed to be 0\n"
                  "[x]            [values of x]\n"
                  "<a> = explosions       [0-5]\n"
                  "<b> = powerup glow     [0-2]\n"
                  "<c> = muzzle flash     [0-2]\n"
                  "<d> = gib/body filter  [0-2]\n"
                  "<e> = rocket change    [0-1]\n"
                  "<f> = setinfo filter   [0-0]\n"
                  "<g> = nail filter      [0-4]\n"
                  "<h> = damage filter    [0-1]\n"
                  "Try different options to see\n"
                  "what each number means. 0 is\n"
                  "normal in all cases. Here is\n"
                  "an example: .fps 5 1 2 2 1\n"
                  "The f parameter is ignored.\n"
                  );
    else if(!memicmp(param,"mar",3))
        q_print(u,".mark [name|delete|reload|auto]\n"
                  "modifies marks that can be used\n"
                  "to tell player location with\n"
                  "say_team %l.\n"
                  "to show nearest:      .mark\n"
                  "to create a new mark: .mark <name>\n"
                  "to delete nearest:    .mark delete\n"
                  "to reload marks:      .mark reload\n"
                  "to autogenerate:      .mark auto\n"
                  "Marks are stored in .loc files in\n"
                  "the current directory (files are\n"
                  "compatible with FAQ-proxy). All\n"
                  "commands save the loc file except\n"
                  "autogenerate which replaces current\n"
                  "marks with marks based on important\n"
                  "items (the automarks are not good,\n"
                  "but better than nothing if you are\n"
                  "in a hurry).\n"
                  );
    else if(!memicmp(param,"wea",3))
        q_print(u,".weapon <best | none | #: # # # ...>\n"
                  "controls impulse modification\n"
                  "best | 1\n"
                  "  For each impulse command, if that\n"
                  "  weapon doesn't have ammo or you\n"
                  "  don't have the weapon, select the\n"
                  "  next best weapon with ammo. For\n"
                  "  default impulses the last working\n"
                  "  weapon in this list is selected:\n"
                  "  8: 1 2 4 3 5 8\n"
                  "  7: 1 2 4 3 5 7\n"
                  "  6: 1 2 4 3 5 6\n"
                  "  5: 1 2 4 3 5\n"
                  "  4: 1 2 4\n"
                  "  3: 1 2 3\n"
                  "  2: 1 2\n"
                  "none | 0\n"
                  "  disable the above impulses\n"
                  "  modifications (default)\n"
                  "10: 3 2\n"
                  "  define your own impulse sequence\n"
                  "  (numbers above are just examples)\n"
                  "  This would define impulse 10 to\n"
                  "  be the equivalent of an alias\n"
                  "  'impulse 3; wait; impulse 2'.\n"
                  "10:\n"
                  "  undefine the above sequences\n"
                  "You can define up to 8 sequences. Also\n"
                  "note that these only work with default\n"
                  "Quake weapons, not for example in TF.\n"
                  );
    else if(!memicmp(param,"who",3))
        q_print(u,".who [proxy | fps]\n"
                  "Lists players in the server along\n"
                  "with their teams and skins (like\n"
                  "Kombat Teams who-command). Options:\n"
                  ".who proxy  displays what proxies\n"
                  "            all players are using\n"
                  ".who fps    displays fps rates of\n"
                  "            visible players. Last\n"
                  "            known rates for far\n"
                  "            away players in ( ).\n"
                  );
    else if(!memicmp(param,"sou",3))
        q_print(u,".sound [triggerchar]\n"
                  "Selects the soundtrigger character and\n"
                  "enables soundtriggers. If the character\n"
                  "is $ (for example) and a team message\n"
                  "ends like '$items/damage.wav' that sound\n"
                  "will be played. Start of the message\n"
                  "is displayed, so 'help $items/damage.wav'\n"
                  "would print 'help' and play that sound.\n"
                  "Use 'none' to disable (default disabled).\n"
                  );
#ifdef IDENT
    else if(!memicmp(param,"ide",3))
        q_print(u,".ident [format formattext]\n"
                  "Prints who you are looking at above\n"
                  "the poweruptimers. The format can be\n"
                  "changed with for example:\n"
                  ".ident format %N %Aa:%a h:%h\n"
                  "Format needs only be set once, the\n"
                  "default is like f_report. You can\n"
                  "only get status of teammates using\n"
                  "Cheapo 2.2 or later. Otherwise only\n"
                  "the name is shown.\n"
                  );
#endif
    else if(!memicmp(param,"sta",3))
        q_print(u,".stat < what to show ... >\n"
                  "Display communications statistics\n"
                  "between proxy and server. Multiple\n"
                  "options can be specified:\n"
                  "all   - show all\n"
                  "ser   - server side stuff\n"
                  "ext   - extended statistics\n"
                  "ping  - ping for 8 last packets\n"
                  "net   - netgraph for 30 packets\n"
                  "proxy - server/clientside proxies\n"
                  "reset - reset extended stats\n"
                  );
    else if(!memicmp(param,"cpp",3))
        q_print(u,".cpps <packets/sec>\n"
                  "Limit the number of client packets\n"
                  "sent to the server per second. Most\n"
                  "useful when you set cl_maxfps 72 to\n"
                  "get smooth play and then want to cut\n"
                  "the number of packets (with .cpps\n"
                  "50 for example) so your connection\n"
                  "won't overload. Dropping looks like\n"
                  "packet loss in netgraph, but if the\n"
                  ".cpps limit is greater than half\n"
                  "your framerate the loss shouldn't\n"
                  "matter in practice.\n"
                  );
    else if(!memicmp(param,"fmo",3))
        q_print(u,".fmodified < 1 | 0 >\n"
                  "Enable or disable f_modified which is\n"
                  "used to verify that no models have\n"
                  "been modified in official games. QWCL\n"
                  "must be started from Cheapo command\n"
                  "line for this to work. (enabled by\n"
                  "default).\n"
                  );
    else if(!memicmp(param,"dmm",3))
        q_print(u,".dmm 1 | 2 | 3 | auto\n"
                  "Controls deathmatch mode detection.\n"
                  "In mode 1 weapons are reported in\n"
                  "point/took messages. In other modes\n"
                  "they are not (even when enabled with\n"
                  "with .point or .took). Default value\n"
                  "auto reads dmm from server. Values\n"
                  "1, 2 and 3 can be used to force a\n"
                  "mode. Previous Cheapo versions acted\n"
                  "as if mode 1 was always active.\n"
                  );
    else if(!memicmp(param,"qui",3))
        q_print(u,".quiet < 0 | 1 >\n"
                  "Just '.quiet' silences all proxy\n"
                  "messages for 0.5 sec, so appending\n"
                  "it before a list of commands removes\n"
                  "the normal output they generate.\n"
                  "Server messages are always printed.\n"
                  "If you have a longer script, you can\n"
                  "put '.quiet 1' at start and '.quiet 0'\n"
                  "at end. This command doesn't print\n"
                  "anything itself.\n"
                  );
    else if(!memicmp(param,"mor",3))
        q_print(u,
                  "- for all commands, the first three\n"
                  "  letters are enough\n"
                  "- when first connecting to proxy\n"
                  "  alias proxyautoexec is executed.\n"
                  "  (you can define it yourself)\n"
                  "- proxy removes all unrecognized\n"
                  "  messages starting with '!' or '.'\n"
                  "- you can talk to other players in\n"
                  "  the proxy when not connected\n"
                  "- see .help funny for funny names\n"
                  "- proxy supports serverside feature\n"
                  "  disables (fpd=xx, as in Qizmo)\n"
                  "- proxy defines f_report, f_powerup\n"
                  "- proxy responds to f_version\n"
                  "- proxy shows Cheapo in userinfo\n"
                  );
    else if(!memicmp(param,"say2",4))
    {
        q_print(u,
                  "additional %-commands supported:\n"
                  "these five are uppercase:\n"
                  "  %S   = your skin (for tf)\n"
                  "  %C   = your bottom color\n"
                  "  %L   = these work like %l,%e,%o\n"
//                  "  %U   = your loc & dir (see %T)\n"
                  "  %P   = last powerup that ended\n"
                  "  %E   =   except after death they\n"
                  "  %O   =   are frozen for 5 seconds\n"
                  "  %r   = last reported location\n"
                  "see .point for these:\n"
                  "  %t   = target pointed to\n"
//                  "  %T   = target pointed to with\n"
//                  "         move direction for players\n"
                  "  %x   = like %t but item name only\n"
                  "  %y   = like %t but location only\n"
                  "  %i   = last item picked up\n"
                  "  %j   = last target pointed to\n"
                  "  %k   = last item picked/pointed\n"
                  "  %m   = last item picked or nearest\n"
                  "powerup reporting:\n"
                  "  %q   = powerups of last seen enemy\n"
                  "         (or quad if none seen)\n"
                  "  %g   = soon appearing powerups\n"
                  "         (or quad if none appearing)\n"
                  "aliases (redefine if you want):\n"
                  "f_report  = %[a] %[h] %[b] %p %n\n"
                  "f_powerup = i have %p\n"
                  "f_took    = took %i\n"
                  );
    }
    else if(!memicmp(param,"say",3))
    {
        q_print(u,
                  "following %-commands supported:\n"
                  "status reporting:\n"
                  "  %A   = armortype\n"
                  "  %a   = armor\n"
                  "  %[a] = %Aa:%a (with [] if under 50)\n"
                  "  %h   = health\n"
                  "  %[h] = h:%h (with [] if under 50)\n"
                  "  %w   = current weapon\n"
                  "  %[w] = %w (with [] if not good)\n"
                  "  %b   = best weapon\n"
                  "  %[b] = %b (with [] if not good)\n"
                  "  %p   = powerups you have\n"
                  "  %[p] = %p (with [])\n"
                  "  %s   = report only what is low\n"
                  "  %l   = nearest location\n"
                  "  %d   = location you died at\n"
                  "  %e   = enemies\n"
                  "  %o   = teammates\n"
                  "  %n   = send to visible players only\n"
                  "  %N   = send to ALL but yourself\n"
                  "         (only use one of %n or %N)\n"
                  "see .help say2 for more %-commands.\n"
                  );
    }
    else
    {
       q_print(u,
            "for nearest proxy use:  .cmd\n"
            "for specific proxy use: host:cmd\n"
            "for final proxy use:    proxy:cmd\n"
            ".help <command> for more help\n"
            ".help more      for misc info\n"
            ".help say       for %-commands\n"
            "\n"
#ifdef TIMERS
            "[connecting]    [camp/hud]\n"
            "connect         line      \n"
            "disconnect      camp      \n"
            "reconnect       remind    \n"
            "users           guess     \n"
            "name            move      \n"
            "                notimers  \n"
            "[teamplay]                \n"
            "who             [misc]    \n"
            "mark            fps       \n"
            "skin            weapon    \n"
            "color           items     \n"
            "point           track     \n"
            "took            cpps      \n"
            "report          flood     \n"
            "sound           stat      \n"
            "                fmodified \n"
#else
            "[connecting]   [visibles]\n"
            "connect        fps       \n"
            "disconnect     skin      \n"
            "reconnect      color     \n"
            "users                    \n"
            "name           [misc]    \n"
            "               who       \n"
            "[messages]     items     \n"
            "mark           track     \n"
            "report         cpps      \n"
            "point          move      \n"
            "took           stat      \n"
            "sound          fmodified \n"
            "filter         flood     \n"
            "dmm            weapon    \n"
            "               quiet     \n"
#endif
#ifdef IDENT
            "ident                     \n"
#endif
            );
    }
}

void modifycampflags(uchar *s,int *campflags,uchar *resulttxt,int limitmask,int def)
{
    int flags=*campflags;

    while(*s)
    {
        int set=1,a;

        if(0)
        {
        }
        else if(!memicmp(s,"all",3))
        {
            flags=-1;
        }
        else if(!memicmp(s,"none",4))
        {
            flags=0;
        }
        else if(!memicmp(s,"def",3))
        {
            flags|=def;
        }
        else if(!memicmp(s,"powe",4))
        {
            flags|=CAMP_P|CAMP_Q|CAMP_R;
        }
        else if(!memicmp(s,"poin",4))
        {
            flags|=CAMP_POINTED;
        }
        else if(!memicmp(s,"pack",4))
        {
            flags|=CAMP_PACK;
        }
        else if(!memicmp(s,"rune",4) || !memicmp(s,"flag",4) || !memicmp(s,"misc",4))
        {
            flags|=CAMP_RUNE;
        }
        else if(!memicmp(s,"armo",4))
        {
            flags|=CAMP_RA|CAMP_YA|CAMP_GA;
        }
        else if(!memicmp(s,"play",4))
        {
            flags|=CAMP_PLAYER;
        }
        else if(!memicmp(s,"enem",4))
        {
            flags|=CAMP_ENEMY;
        }
        else if(!memicmp(s,"heal",4))
        {
            flags|=CAMP_HEALTH;
        }
        else if(!memicmp(s,"suit",4))
        {
            flags|=CAMP_SUIT;
        }
        else if(!memicmp(s,"she",3))
        {
            flags|=CAMP_SHELLS;
        }
        else if(!memicmp(s,"roc",3))
        {
            flags|=CAMP_ROCKETS;
        }
        else if(!memicmp(s,"nai",3))
        {
            flags|=CAMP_NAILS;
        }
        else if(!memicmp(s,"cel",3))
        {
            flags|=CAMP_CELLS;
        }
        else if(!memicmp(s,"ra",2))
        {
            a=CAMP_RA;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"ya",2))
        {
            a=CAMP_YA;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"ga",2))
        {
            a=CAMP_GA;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"me",2) || !memicmp(s,"mh",2))
        {
            a=CAMP_MH;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"rl",2))
        {
            a=CAMP_RL;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"gl",2))
        {
            a=CAMP_GL;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"lg",2))
        {
            a=CAMP_LG;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"ng",2))
        {
            a=CAMP_NG;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"sn",2))
        {
            a=CAMP_SNG;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"ss",2))
        {
            a=CAMP_SSG;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"r",1))
        {
            a=CAMP_R;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"p",1))
        {
            a=CAMP_P;
            if(!set) flags&=~a;
            else flags|=a;
        }
        else if(!memicmp(s,"q",1))
        {
            a=CAMP_Q;
            if(!set) flags&=~a;
            else flags|=a;
        }

        while(*s>32) s++;
        while(*s<=32 && *s) s++;
    }

    *resulttxt=0;
    flags&=limitmask;
    if(flags&CAMP_RA)       strcat(resulttxt,"ra ");
    if(flags&CAMP_YA)       strcat(resulttxt,"ya ");
    if(flags&CAMP_GA)       strcat(resulttxt,"ga ");
    if(flags&CAMP_MH)       strcat(resulttxt,"mh ");
    if(flags&CAMP_LG)       strcat(resulttxt,"lg ");
    if(flags&CAMP_RL)       strcat(resulttxt,"rl ");
    if(flags&CAMP_GL)       strcat(resulttxt,"gl ");
    if(flags&CAMP_SNG)      strcat(resulttxt,"sng ");
    if(flags&CAMP_NG)       strcat(resulttxt,"ng ");
    if(flags&CAMP_SSG)      strcat(resulttxt,"ssg ");
    if(flags&CAMP_HEALTH)   strcat(resulttxt,"heal ");
    if(flags&CAMP_NAILS)    strcat(resulttxt,"nail ");
    if(flags&CAMP_CELLS)    strcat(resulttxt,"cell ");
    if(flags&CAMP_SHELLS)   strcat(resulttxt,"shel ");
    if(flags&CAMP_ROCKETS)  strcat(resulttxt,"rock ");
    if(flags&CAMP_SUIT)     strcat(resulttxt,"suit ");
    if(flags&CAMP_R)        strcat(resulttxt,"ring ");
    if(flags&CAMP_P)        strcat(resulttxt,"pent ");
    if(flags&CAMP_Q)        strcat(resulttxt,"quad ");
    if(flags&CAMP_PLAYER)   strcat(resulttxt,"player ");
    if(flags&CAMP_ENEMY)    strcat(resulttxt,"enemy ");
    if(flags&CAMP_RUNE)     strcat(resulttxt,"misc ");
    if(flags&CAMP_PACK)     strcat(resulttxt,"pack ");
    if(flags&CAMP_POINTED)  strcat(resulttxt,"pointed ");

    *campflags=flags;
}

char *nextparam(uchar **sp)
{
    static uchar buf[64];
    uchar *d=buf,*s=*sp;

    while(*s<=32 && *s) s++;

    while(d<buf+60 && *s>32)
    {
        *d++=*s++;
    }
    *d++=0;

    while(*s<=32 && *s) s++;

    *sp=s;
    return(buf);
}

void q_command(User *u,uchar *s)
{
    uchar tmp[MAXSTR];
    uchar buf[MAXSTR];

    logx(LOG_DEBUG,"%s command: %s \n",u->name,formatqstring(s));

#ifndef TIMERS
    u->campthese=0;
    u->notimers=1;
#endif

    if(*s=='!')
    { // internal commands
        if(!memicmp(s,"!accept",7))
        { // accept from server (generated message)
            u->state=STATE_CONNECTED;
            //q_print(u,"Connected\n");
            if(loglevel>=LOG_VERBOSE) logx(LOG_CONSOLE,"Join: %s connected to %s\n",u->name,u->connecthere);
            u->appendserverbytes=0;
            u->appendclientbytes=0;
            q_servercmd(u,"new");
        }
        else if(!memicmp(s,"!sendnew",8))
        {
            q_servercmd(u,"new");
        }
        else if(!memicmp(s,"!reject",7))
        { // reject from server (generated message)
            c_killserver(u);
            u->state=STATE_HERE;
            q_print(u,"Server rejected connection\n");
            u->doconnect=0;
        }
        else if(!memicmp(s,"!init",5))
        { // init after connect to proxy
            //if(u->sequence>0)
            {
                extern uchar *version;
                uchar *p;
                char buf[16];
                int i;
#if !defined(__APPLE__) && !defined(MACOSX)
                static int crcchecked;
#endif /* !__APPLE__ && !MACOSX */
                memset(tmp,0x9e,36);
                tmp[0]=0x9d;
                tmp[35]=0x9f;
                tmp[36]='\n';
                tmp[37]=0;

                if(!*proxy_name)
                {
                    gethostname(proxy_name,255);
                    strlwr(proxy_name);
                }
                strncpy(buf,proxy_name,10);
                for(i=0;i<10;i++)
                {
                    if(buf[i]=='.') buf[i]=0;
                }
                buf[10]=0;

                if(u->version==24)      p="protocol QuakeWorld 2.3";
                else if(u->version==23) p="protocol QuakeWorld 2.29";
                else if(u->version==22) p="protocol QuakeWorld 2.2";
                else if(u->version==21) p="protocol QuakeWorld 2.1";
                else                    p="protocol unrecognized";

                expires_gettime();

                q_print(u,tmp);
                q_printf(u,
                    "Welcome to %s %s%s\n"
                    "%s\n"
                    "type \'%s:help\' for commands\n"
                    "type \'.help\' for nearest proxy\n"
                    ,versionname,version,versiontype,p,buf);
                q_print(u,tmp);

                strcpy(u->proxysay,"say proxy:");

                sprintf(u->proxyhostname,"%s:%i",buf,proxy_port);

                sprintf(u->proxysay2,"say %s:",buf);

                strcpy(u->requestformat,"%N %[a] %[h] %[b] %p");

                sprintf(tmpbuf,"name \"%s\"\n",u->name);
                q_stufftext(u,tmpbuf);

                u->fmodified=1;

                u->state=STATE_HERE;
                u->usepercentn=1;
                u->nofversion=0;
                u->secondcamp=1;
                u->floodprot=1;
                u->pointtell=CAMP_DEF;
                u->autoitookitems=CAMP_DEFTOOK;
                u->time_connect=u->time;
                u->fpd=litemode;

                if(u->version>24)
                {
                    q_print(u,"Since the protocol is unrecognized, the proxy will probably not work!\n");
                }

                {
                    char ver[16];
                    sprintf(ver,"%s",version);
                    cl_setinfo(u,"Cheapo",ver);
                }

                q_stufftext(u,"alias f_report \"say_team %[a] %[h] %[b] %p%n\"\n");
                q_stufftext(u,"alias f_powerup \"say_team i have %p\"\n");
                q_stufftext(u,"alias f_took \"say_team took %i\"\n");
                q_stufftext(u,"proxyautoexec\n");

                if(gamespy)
                {
                    q_stufftext(u,"exec GameSpyC.cfg");
                }
            }
        }
        else if(!memicmp(s,"!drop",5))
        { // drop from proxy
            if(u->state!=STATE_DROPPING)
            {
                u->state=STATE_DROPPING;
                q_printf(u,"Thanks for using %s.\n",versionname);
                // disconnect server command
                u->appendserver[u->appendserverbytes++]=0x02;
            }
        }
        else if(!memicmp(s,"!sdisconnect",6))
        { // server sends disconnect
            if(loglevel>=LOG_VERBOSE) logx(LOG_CONSOLE,"Drop: %s dropped from %s\n",u->name,u->connecthere);
            if(u->state!=STATE_DISCONNECTING)
            {
                u->state=STATE_DISCONNECTING;
                u->dropcount=3;
            }
        }
        else if(!memicmp(s,"!sdrop",6))
        { // drop from server (called when mode is disconnecting)
            if(u->dropcount>0)
            {
                q_servercmd(u,"drop");
                u->dropcount--;
            }
            else
            {
                c_killserver(u);

                if(u->servertimeoutmsg)
                {
                    u->servertimeoutmsg=0;
                    q_print(u,"Server timeout.");
                }

                q_command(u,"!sdisconnect");
                u->dropcount=0;
                u->appendserverbytes=0;
                u->appendclientbytes=0;
                memset(&u->toserveraddr,0,sizeof(u->toserveraddr));
                u->state=STATE_HERE;

                q_stufftext(u,"changing\n");
                q_print(u,"Server disconnected.");

                u->time_forcedisconnect=0;
            }
        }
        else if(!memicmp(s,"!kill",5))
        { // real drop
            c_textaddress(tmp,&u->toclientaddr);
            if(strcmp(u->name,"<challenge>") && strcmp(u->name,"<status>"))
            {
                logx(LOG_CONSOLE,"Disconnect: %s from %s (users %i)\n",u->name,tmp,usernum-1);
            }
            c_killuser(u);
        }
        else if(!memicmp(s,"!errors",5))
        { // real drop
            c_textaddress(tmp,&u->toclientaddr);
            logx(LOG_CONSOLE,"Net errors (disconnected): %s from %s\n",u->name,tmp);
            c_killuser(u);
        }
        else if(!memicmp(s,"!timeout",5))
        { // real drop
            c_textaddress(tmp,&u->toclientaddr);
            logx(LOG_CONSOLE,"Timeout (disconnected): %s from %s\n",u->name,tmp);
            c_killuser(u);
        }
        else if(!memicmp(s,"!challenge",10))
        { // received a challenge from server
            u->havechallenge=1;
            logx(LOG_DEBUG,"Got challenge %i\n",u->challenge);
            u->time_nexttry=u->time+100;
        }
        else if(!memicmp(s,"!try",4))
        { // try connecting to a server
            if(u->firsttry==1)
            {
                *tmpbuf=0;
                extractfield(tmpbuf,u->clientinfo,"spectator\\");
                if(*tmpbuf=='0' || *tmpbuf==0) u->amspectator=0;
                else u->amspectator=1;

                if(u->servermainaddr.sin_port)
                {
                    c_textaddress(tmp,&u->servermainaddr);
                    if(newsocketmode) memcpy(&u->toserveraddr,&u->servermainaddr,sizeof(SOCKADDR_IN));
                }
                else
                {
                    q_printf(u,"Illegal address: %s\n",u->connecthere);
                    u->state=STATE_HERE;
                    return;
                }

                if(u->response_b)
                {
                    u->response_b++;
                    if(!u->response_b) u->response_b++;
                }

                if(u->isreconnect || c_sameaddress(&u->servermainaddr,&u->lastservermainaddr))
                {
                    u->time_nexttry=u->time+1600; // 1.6 sec delay
                }
                else
                {
                    u->time_nexttry=u->time+300;  // 0.3 sec delay
                }
                u->lastservermainaddr=u->servermainaddr;

                u->firsttry=2;
                return;
            }
            else if(u->firsttry==2)
            { // flip client hidden bit to 0
                if(u->client_hiddenbit)
                {
                    u->server_sendsecure=1;
                }
                else
                {
                    q_print(u,"Challenging...\n");
                    u->firsttry=0;
                }
                u->time_nexttry=u->time+100; // 0.1 sec delay
                return;
            }
            u->isreconnect=0;
            u->connectingcount--;
            if(u->challengecount<=0)
            {
                q_print(u,"Server not responding.\n");
                u->state=STATE_HERE;
            }
            else if(u->connectingcount<=0)
            {
                q_print(u,"Server not responding.\n");
                u->state=STATE_HERE;
            }
            else
            {
                if(!u->havechallenge && u->version>=22)
                { // ask for challenge
                    memset(tmpbuf,255,4);
                    strcpy(tmpbuf+4,"getchallenge\n");
                    if(loglevel>=LOG_DEBUG)
                    {
                        c_hexdump("Connect attempt",tmpbuf,5+strlen(tmpbuf+4),u);
                    }
                    c_sendservermain(u,tmpbuf,4+strlen(tmpbuf+4));
                    u->time_nexttry=u->time+1000;
                    u->challengecount--;
                }
                else
                { // normal connect after that
                    if(u->challenge==challenge)
                    {
                        q_print(u,"Cannot connect to self.\n");
                        u->state=STATE_HERE;
                        return;
                    }
                    q_print(u,"Connecting...\n");
                    memset(tmpbuf,255,4);
                    if(u->version>=22)
                    {
                        sprintf(tmpbuf+4,"connect %i %i %i %s",
                                u->response_a,
                                u->response_b,
                                u->challenge,
                                u->clientinfo);
                    }
                    else
                    {
                        sprintf(tmpbuf+4,"connect %s",
                                u->clientinfo);
                    }

                    if(loglevel>=LOG_DEBUG)
                    {
                        c_hexdump("Connect attempt",tmpbuf,5+strlen(tmpbuf+4)+1,u);
                    }
                    c_sendservermain(u,tmpbuf,4+strlen(tmpbuf+4)+1);
                    u->time_nexttry=u->time+3000;
                }
            }
        }
        else if(!memicmp(s,"!connect",7))
        {
            /*
            lognpackets=200;
            oldloglevel=loglevel;
            loglevel=LOG_PACKETS;
            logx(LOG_CONSOLE,"--------packetdump %i packets--------",lognpackets);
            */

            c_reopenserver(u);

            u->connectstartsequence=u->sequence;
            u->havechallenge=0;
            u->connectingcount=1000000; // be persistent :)
            u->challengecount=5;
            u->time_nexttry=0;
            u->servermainaddr.sin_port=0;
            u->firsttry=1;
            u->state=STATE_NAMELOOKUP;
            u->time_namelookupmsg=u->time+3000;
            u->time_reskinall=0;

            if(u->isreconnect) q_printf(u,"Reconnecting to %s\n",u->connecthere);
            else               q_printf(u,"Connecting to %s\n",u->connecthere);

#ifndef WIN32
            mainloop_low(1); // do namelookup now
#endif
        }
        else if(!memicmp(s,"!disconnect",11))
        {
            if(u->servermainaddr.sin_port)
            {
                u->time_forcedisconnect=u->time+3000; // force a disconnect in 3 sec
                u->state=STATE_DISCONNECTING;
                u->linkedserver=0;
                u->dropcount=3;
                if(u->compress_client) u->compress_client=0;
            }
        }
        else if(!memicmp(s,"!servertimeout",13))
        {
            q_command(u,"!disconnect");
            u->servertimeoutmsg=1;
        }
    }
    else
    { // user commands
        if(!memicmp(s,".teleport",9))
        { // FAQ proxy sends
            // do nothing
        }
        else if(!memicmp(s,".hel",4))
        { // help
            uchar *p;
            nextparam(&s); // skip command
            p=nextparam(&s);
            q_help(u,p);
        }
        else if(!memicmp(s,".nam",4))
        { // proxy name
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                p=nextparam(&s);

                strncpy(buf,p,10);
                buf[10]=0;
                sprintf(u->proxysay,"say %s:",buf);
                sprintf(u->proxysay2,"say %s:",buf);
            }

            if(strcmp(u->proxysay,u->proxysay2))
            {
                q_printf(u,"Proxy %s responds to:\n"
                           "%scommand\n"
                           "%scommand\n"
                           ".command\n",
                           u->proxyhostname,
                           u->proxysay+4,
                           u->proxysay2+4);
            }
            else
            {
                q_printf(u,"Proxy %s responds to:\n"
                           "%scommand\n"
                           ".command\n",
                           u->proxyhostname,
                           u->proxysay+4);
            }
        }
        else if(!memicmp(s,".use",4))
        { // users/clients
            int i;
            for(i=0;i<usernum;i++)
            {
                c_textaddress(tmp,&user[i].servermainaddr);
                if(user[i].state==STATE_HERE) strcpy(tmp,"idle");
                q_printf(u,"%i: %-10s %s\n",i,user[i].name,tmp);
            }
        }
        else if(!memicmp(s,".con",4))
        { // connect
            uchar *p;

            if(u->state==STATE_NAMELOOKUP)
            {
                q_print(u,"Namelookup still in progress.\n");
            }
            else if(u->state>=STATE_CONNECTED)
            {
                q_command(u,"!disconnect");
            }
            else u->state=STATE_HERE;

            nextparam(&s);
            p=nextparam(&s);
            if(strlen(p)>1)
            {
                strcpy(u->connecthere,p);
                u->doconnect=10;
                u->isreconnect=0;
            }
            u->specmode=0;
            u->specentity=0;
        }
        else if(!memicmp(s,".dis",4))
        { // disconnect
            if(u->state==STATE_NAMELOOKUP)
            {
                q_print(u,"Namelookup still in progress.\n");
            }
            else if(u->state==STATE_CONNECTED)
            {
                c_textaddress(tmp,&u->servermainaddr);
                q_printf(u,"Disconnecting from %s\n",tmp);
                q_command(u,"!disconnect");
            }
            else u->state=STATE_HERE;
            memset(&u->lastservermainaddr,0,sizeof(u->lastservermainaddr));
        }
        else if(!memicmp(s,".rec",4))
        { // reconnect
            if(u->state==STATE_NAMELOOKUP)
            {
                q_print(u,"Namelookup still in progress.\n");
            }
            else
            {
                if(u->state==STATE_CONNECTED)
                {
                    q_command(u,"!disconnect");
                }
                u->doconnect=10;
                u->isreconnect=1;
            }
        }
        else if(!memicmp(s,".tra",4))
        { // start tracking
            uchar *p;
            if(u->state<STATE_CONNECTED)
            {
                q_print(u,"Cannot track, not connected.\n");
                return;
            }

            if(!u->amspectator)
            {
                q_print(u,"Cannot track, not spectator.\n");
                return;
            }

            if(u->client_spectrack)
            {
                q_print(u,"Cannot track, must be in fly mode.\n");
                return;
            }

            nextparam(&s);
            p=nextparam(&s);
            if(*p)
            {
                u->specmode=1;
                u->specentity=0;
                spectatenext(u,p);
                if(u->specentity)
                {
                    q_printf(u,"Tracking \'%s\' enabled.\n",p);
                    q_print(u,"Jump to change player. Fire to stop tracking.\n");
                }
                else
                {
                    spectatenext(u,NULL);
                    q_printf(u,"Tracking enabled but \'%s\' not found.\n",s);
                    q_print(u,"Jump to change player. Fire to stop tracking.\n");
                }
            }
            else
            {
                q_print(u,"Tracking enabled.\n");
                q_print(u,"Jump to change player. Fire to stop tracking.\n");
                u->specmode=1;
                u->specentity=0;
                spectatenext(u,NULL);
            }
        }
        else if(!memicmp(s,".mar",4))
        { // mark
            uchar *place;
            int  a;
            if(u->state!=STATE_CONNECTED)
            {
                q_print(u,"Mark unusable, not connected.\n");
                return;
            }

            nextparam(&s);

            if(!memicmp(s,"reload",6))
            {
                q_loadloc(u,NULL);
            }
            else if(!memicmp(s,"auto",4))
            {
                static int ordertable[]=
                    {CAMP_RUNE,CAMP_Q,CAMP_P,CAMP_R,CAMP_RA,CAMP_RL,CAMP_YA,
                     CAMP_GA,CAMP_MH,CAMP_GL,CAMP_LG,CAMP_SNG,0};
                int i,j,c,d,pass,ord;
                int redflag=-1,bluflag=-1;
                char markname[64];
                char markname2[64];
                Camp *ca,*ca2;
                u->locations=0;

                // do we have flags?
                for(i=0;i<u->camps;i++)
                {
                    ca=&u->camp[i];
                    if(ca->category==CAMP_RUNE)
                    {
                        if(!strcmp(ca->name,"!r")) redflag=i;
                        if(!strcmp(ca->name,"!b")) bluflag=i;
                    }
                }
                if(redflag<0 || bluflag<0) redflag=bluflag=-1;
                else
                {
                    q_print(u,"Marking based on red/blue flags\n");
                }

                q_print(u,"Marks replaced with automarks(tm):\n");

                for(pass=0;pass<=2;pass++)
                { // pass0=single items, 1=dual items, 2=rest of items
                    for(ord=0;ordertable[ord];ord++)
                    {
                        int num,lowerz,upperz,lowercnt,uppercnt;

                        c=ordertable[ord];

                        num=0;
                        lowerz= 999999;
                        upperz=-999999;
                        // find number of items, upper & lower
                        for(i=0;i<u->camps;i++)
                        {
                            ca=&u->camp[i];
                            if(ca->category&c)
                            {
                                if(ca->pos[2]<lowerz) lowerz=ca->pos[2];
                                if(ca->pos[2]>upperz) upperz=ca->pos[2];
                                num++;
                            }
                        }
                        lowercnt=0;
                        uppercnt=0;
                        // find number of low / high items
                        for(i=0;i<u->camps;i++)
                        {
                            ca=&u->camp[i];
                            if(ca->category&c)
                            {
                                if(abs(ca->pos[2]-lowerz)<64) lowercnt++;
                                if(abs(ca->pos[2]-upperz)<64) uppercnt++;
                            }
                        }

                        if(!num) continue;
                        if(pass==0 && num>1) continue;
                        if(pass==1 && num>2) continue;

                        // set texts
                        for(i=0;i<u->camps;i++)
                        {
                            ca=&u->camp[i];
                            if(ca->category&c)
                            {
                                for(j=0;j<u->locations;j++)
                                {
                                    d=abs(u->loc[j].x-ca->pos[0])*2+
                                      abs(u->loc[j].y-ca->pos[1])*2+
                                      abs(u->loc[j].z-ca->pos[2]);
                                    if(d<7000) break;
                                }
                                if(j==u->locations)
                                {
                                    u->loc[u->locations].x=ca->pos[0];
                                    u->loc[u->locations].y=ca->pos[1];
                                    u->loc[u->locations].z=ca->pos[2];

                                    *markname=0;
                                    if(num==1)
                                    {
                                        sprintf(markname,"%s",ca->genericname);
                                    }
                                    else
                                    {
                                        if(lowercnt==1 && abs(ca->pos[2]-lowerz)<64)
                                            sprintf(markname,"lower-%s",ca->genericname);

                                        if(uppercnt==1 && abs(ca->pos[2]-upperz)<64)
                                            sprintf(markname,"upper-%s",ca->genericname);

                                        if(!markname[0] && *ca->name=='!')
                                            sprintf(markname,"%s",ca->genericname);

                                        if(!markname[0])
                                            sprintf(markname,"%s%c",ca->genericname,ca->name[strlen(ca->name)-1]);
                                    }

                                    if(*markname)
                                    {
                                        *markname2=0;
                                        if(redflag>=0)
                                        {
                                            int dred,dblu;
                                            ca2=&u->camp[redflag];
                                            dred=abs(ca2->pos[0]-ca->pos[0])*2+
                                                 abs(ca2->pos[1]-ca->pos[1])*2+
                                                 abs(ca2->pos[2]-ca->pos[2]);
                                            ca2=&u->camp[bluflag];
                                            dblu=abs(ca2->pos[0]-ca->pos[0])*2+
                                                 abs(ca2->pos[1]-ca->pos[1])*2+
                                                 abs(ca2->pos[2]-ca->pos[2]);
                                            if(dblu*3<dred*2)
                                            {
                                                strcpy(markname2,"blue ");
                                            }
                                            else if(dred*3<dblu*2)
                                            {
                                                strcpy(markname2,"red ");
                                            }
                                        }

                                        strcat(markname2,markname);

                                        sprintf(buf,"- %s\n",markname2);
                                        q_print(u,buf);

                                        q_setlocname(u,u->locations,markname2);
                                        u->locations++;
                                        if(u->locations>=u->locationsmax-16)
                                        {
                                            q_growlocmax(u,u->locationsmax*2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if(!memicmp(s,"delete",6))
            { // delete mark
                int b;
                place=findnear(u,u->pos[0],u->pos[1],u->pos[2],&a);
                if(a!=-1)
                {
                    q_explode(u,0x04,u->loc[a].x,u->loc[a].y,u->loc[a].z);
                    q_printf(u,"%s deleted\n",place);
#if 0
                    q_setlocname(u,a,NULL);
                    if(a!=u->locations-1) memmove(u->loc+a,u->loc+a+1,(u->locations-a-1)*sizeof(Location));
                    u->locations--;
                    memset(u->loc+u->locations,0,sizeof(Location));
#else
                    q_setlocname(u,a,NULL);
                    for(b=a;b<u->locations-1;b++)
                    {
                        memcpy(u->loc+b,u->loc+b+1,sizeof(Location));
                    }
                    u->locations--;
                    memset(u->loc+u->locations,0,sizeof(Location));
#endif

                    q_savelocs(u);
                }
                else
                { // mark not found
                    q_printf(u,"no marks\n",place);
                }
            }
            else if(!*s) // report
            {
                place=findnear(u,u->pos[0],u->pos[1],u->pos[2],&a);
                q_printf(u,"near %s\n",place);
                u->appendserver[u->appendserverbytes++]=0x17;
                u->appendserver[u->appendserverbytes++]=0x04;
                u->appendserver[u->appendserverbytes++]=u->loc[a].x;
                u->appendserver[u->appendserverbytes++]=u->loc[a].x>>8;
                u->appendserver[u->appendserverbytes++]=u->loc[a].y;
                u->appendserver[u->appendserverbytes++]=u->loc[a].y>>8;
                u->appendserver[u->appendserverbytes++]=u->loc[a].z;
                u->appendserver[u->appendserverbytes++]=u->loc[a].z>>8;
            }
            else // new mark
            {
                if(strlen(s)>1)
                {
                    q_setlocname(u,u->locations,s);
                    u->loc[u->locations].x = u->pos[0];
                    u->loc[u->locations].y = u->pos[1];
                    u->loc[u->locations].z = u->pos[2];
                    u->locations++;
                }

                q_printf(u,"%s marked\n",s);
                q_savelocs(u);

                u->appendserver[u->appendserverbytes++]=0x17;
                u->appendserver[u->appendserverbytes++]=0x04;
                u->appendserver[u->appendserverbytes++]=u->pos[0];
                u->appendserver[u->appendserverbytes++]=u->pos[0]>>8;
                u->appendserver[u->appendserverbytes++]=u->pos[1];
                u->appendserver[u->appendserverbytes++]=u->pos[1]>>8;
                u->appendserver[u->appendserverbytes++]=u->pos[2];
                u->appendserver[u->appendserverbytes++]=u->pos[2]>>8;
            }
        }
#ifdef TIMERS
        else if(!memicmp(s,".cam",4))
        { // camp these
            nextparam(&s);
            if(*s) u->campthese=0;

            if(u->notimers)
            {
                sprintf(buf,"Timers disabled with .notimers.\n");
            }
            else if(u->fpd&FPD_TIMERS)
            {
                sprintf(buf,"Timers disabled.\n");
            }
            else
            {
                strcpy(buf,"Timing: ");
                modifycampflags(s,&u->campthese,buf+strlen(buf),CAMP_P|CAMP_Q|CAMP_R,CAMP_DEF);
                strcat(buf,"\n");
            }

            q_print(u,buf);
        }
        else if(!memicmp(s,".rem",4))
        { // remind
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->remindcamp=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->remindcamp=atoi(p);
                if(u->remindcamp<0) u->remindcamp=0;
                break;
            }

            if(u->remindcamp) sprintf(buf,"Remind: %i seconds before item\n",u->remindcamp);
            else strcpy(buf,"Remind: disabled\n");
            q_print(u,buf);
        }
        else if(!memicmp(s,".gue",4))
        { // guess
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->secondcamp=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->secondcamp=atoi(p);
                if(u->secondcamp<=0) u->secondcamp=0;
                if(u->secondcamp>1) u->secondcamp=1;
                break;
            }

            if(u->secondcamp) strcpy(buf,"Guess: estimate if not known\n");
            else strcpy(buf,"Guess: disabled\n");
            q_print(u,buf);
        }
        else if(!memicmp(s,".notim",6))
        {
            int check=1;
            nextparam(&s);
            if(*s<=32) check=0;
            if(*s==0x7f)
            {
                check=0;
                s++;
            }

            if(check && u->amconnected && *s!='1')
            {
                q_print(u,"Notimers mode can only be cleared\n"
                          "when not connected to a server.\n");
            }
            else if(check && u->linkedclient)
            {
                q_print(u,"Notimers mode has to be set in the\n"
                          "first proxy. Use .notimers instead\n"
                          "of proxy:notimers.\n");
            }
            else
            {
                char buf[64];

                if(*s=='1')
                {
                    u->campthese=0;
                    u->notimers=1;
                    u->fpd|=FPD_TIMERS;

                    sprintf(buf,"%s notimers",version);
                    cl_setinfo(u,"Cheapo",buf);
                    if(u->amconnected)
                    {
                        sprintf(buf,"Cheapo \"%s notimers\"",version);
                        sendsetinfo(u,buf);
                    }

                    if(u->linkedserver)
                    {
                        sprintf(buf,"say .notimers \x7f%i",u->notimers);
                        q_servercmd(u,buf);
                    }
                }
                else if(*s=='0')
                {
                    u->campthese=0;
                    u->notimers=0;
                    u->fpd=0;

                    sprintf(buf,"%s",version);
                    cl_setinfo(u,"Cheapo",buf);

                    if(u->linkedserver)
                    {
                        sprintf(buf,"say .notimers \x7f%i",u->notimers);
                        q_servercmd(u,buf);
                    }
                }

                if(u->notimers)
                {
                    q_print(u,"Notimers: powerup timers disabled\n");
                }
                else
                {
                    q_print(u,"Notimers: powerup timers enabled\n");
                }
            }
        }
#else
        else if(!memicmp(s,".rem",4))
        {
            //ignore
        }
        else if(!memicmp(s,".gue",4))
        {
            //ignore
        }
        else if(!memicmp(s,".cam",4))
        {
            //ignore
        }
        else if(!memicmp(s,".not",4))
        {
            //ignore
        }
#endif
        else if(!memicmp(s,".poi",4))
        { // camp these
            nextparam(&s);

            {
                if(*s) u->pointtell=0;
                strcpy(buf,"%t pointing: ");
                modifycampflags(s,&u->pointtell,buf+strlen(buf),CAMP_LAST-1,CAMP_DEF);
                strcat(buf,"\n");
            }
            *u->msgline=0;

            q_print(u,buf);
        }
        else if(!memicmp(s,".too",4))
        { // took messages
            nextparam(&s);

            {
                if(*s) u->autoitookitems=0;
                strcpy(buf,"auto f_took items: ");
                modifycampflags(s,&u->autoitookitems,buf+strlen(buf),CAMP_LAST-1,CAMP_DEFTOOK);
                strcat(buf,"\n");
            }
            *u->msgline=0;

            q_print(u,buf);
            if(!u->autoitook)
            {
                q_print(u,"f_took disabled (see .report)\n");
            }
        }
/*
        else if(!memicmp(s,".spo",4))
        { // camp these
            nextparam(&s);
            if(*s) u->pointshow=0;

            {
                strcpy(buf,"screen pointing: ");
                modifycampflags(s,&u->pointshow,buf+strlen(buf),CAMP_LAST-1,CAMP_DEF);
                strcat(buf,"\n");
            }
            *u->msgline=0;

            q_print(u,buf);
        }
*/
        else if(!memicmp(s,".fmo",4))
        { // fmodified
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->fmodified=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->fmodified=atoi(p);
                break;
            }

            if(u->fmodified) sprintf(buf,"F_modified: enabled\n");
            else strcpy(buf,"F_modified: disabled\n");
            q_print(u,buf);
            if(u->fmodified)
            {
                if(!*quakepath)
                {
                    q_print(u,"QWCL has not been started from\n"
                              "Cheapo; f_modified will not work.\n");
                }
                else if(quakemodified)
                {
                    q_print(u,"QWCL executable has been modified;\n"
                              "f_modified will not work.\n");
                }
                else if(!c_islocalconnect(&u->toclientaddr))
                {
                    q_print(u,"QWCL client is not local;\n"
                              "f_modified will not work.\n");
                }
            }
        }
        else if(!memicmp(s,".mov",4))
        { // move
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->moveoverlay=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->moveoverlay=atoi(p);
                if(u->moveoverlay<=0) u->moveoverlay=0;
                if(u->moveoverlay>9) u->moveoverlay=9;
                break;
            }

            if(u->secondcamp) sprintf(buf,"Move texts: %i lines down\n",u->moveoverlay);
            else strcpy(buf,"Move texts: disabled\n");
            q_print(u,buf);
        }
        else if(!memicmp(s,".flo",4))
        { // flood
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->floodprot=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->floodprot=atoi(p);
                if(u->floodprot<=0) u->floodprot=0;
                if(u->floodprot>1) u->floodprot=1;
                break;
            }

            if(u->floodprot) strcpy(buf,"Flood protect protect: enabled\n");
            else strcpy(buf,"Flood protect protect: disabled\n");
            q_print(u,buf);
        }
#ifdef RLPACK
        else if(!memicmp(s,".pac",4))
        { // pack
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->classifypacks=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->classifypacks=atoi(p);
                if(u->classifypacks<=0) u->classifypacks=0;
                if(u->classifypacks>1) u->classifypacks=1;
                break;
            }

            if(u->classifypacks) strcpy(buf,"Packs: detect rl\n");
            else strcpy(buf,"Packs: normal\n");
            q_print(u,buf);
        }
#endif
        else if(!memicmp(s,".cpp",4))
        { // cpps
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->cpps=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                u->cpps=atoi(p);
                if(u->cpps<=0) u->cpps=0;
                if(u->cpps>100) u->cpps=100;

                break;
            }

            u->time_nextcpps=0;
            if(u->cpps) u->time_cppsdelta=1000/u->cpps;
            else u->time_cppsdelta=0;
            u->cppsdrop=1;
            u->cppspass=1;

            if(u->cpps) sprintf(buf,"Client to server: %i packets/sec\n",u->cpps);
            else strcpy(buf,"Client to server: no limit\n");
            q_print(u,buf);
        }
#ifdef COMPRESS
        else if(!memicmp(s,".com",4))
        { // compress
            uchar *p;
            int compwas=u->compress_client;

            nextparam(&s);
            if(*s)
            {
                u->compress_client=0;

                for(;;)
                {
                    p=nextparam(&s);
                    if(!*p) break;

                    u->compress_client=atoi(p);
                    break;
                }

                if(u->compress_client && !u->linkedserver)
                {
                    u->compress_client=0;
                    q_print(u,"Cannot compress: not connected\n"
                              "to a compatible serverside proxy.\n");
                }

                if(compwas!=u->compress_client)
                {
                    if(u->compress_client) sprintf(buf,"Client compress: %s\n",u->proxyhostname);
                    else                   sprintf(buf,"Client compress: disabled\n",u->proxyhostname);
                    q_print(u,buf);
                }

                if(u->compress_client) q_print(u,"Compression: enabled\n");
                else                   q_print(u,"Compression: disabled\n");
            }
            else
            {
                int i,c1=0,c2=0,s1=0,s2=0,cr,sr;
                for(i=0;i<16;i++)
                {
                    c1+=u->packetclientinsize[i];
                    c2+=u->packetclientoutsize[i];
                    s1+=u->packetserverinsize[i];
                    s2+=u->packetserveroutsize[i];
                }

                if(!c1) cr=0;
                else cr=100*c2/c1;
                c1>>=4;
                c2>>=4;

                if(!s1) sr=0;
                else sr=100*s2/s1;
                s1>>=4;
                s2>>=4;

                if(u->compress_client) q_print(u,"Compression: enabled\n");
                else                   q_print(u,"Compression: disabled\n");

                /*
                q_print(u,"Delta counts:\n");
                for(i=0;i<8;i++)
                {
                    sprintf(buf,"e%i: %i\n",i,expcount[i]);
                    q_print(u,buf);
                }
                */

                sprintf(buf,"Client ratio: %i%% (%i to %i)\n",cr,c1,c2);
                q_print(u,buf);
                sprintf(buf,"Server ratio: %i%% (%i to %i)\n",sr,s1,s2);
                q_print(u,buf);
            }
        }
#endif
        else if(!memicmp(s,".rep",4))
        { // autoreport
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->autofreport=0;
                u->autoihave=0;
                u->autoitook=0;
                u->autoqpicked=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if(!memicmp(p,"rep",3))
                {
                    u->autofreport=1;
                }
#ifdef POWERDROP
                if(!memicmp(p,"power2",6))
                {
                    u->autoihave|=2;
                }
#endif
                if(!memicmp(p,"pow",3))
                {
                    u->autoihave|=1;
                }
                if(!memicmp(p,"too",3))
                {
                    u->autoitook=1;
                }
#ifdef TIMERS
                if(!memicmp(p,"syn",3))
                {
                    if(!u->autoqpicked) u->autoqpicked=1;
                }
                if(!memicmp(p,"sho",3))
                {
                    u->autoqpicked=2;
                }
#endif
            }

            {
                int x=0;
                sprintf(buf,"Reporting: ");
                if(u->autofreport) { x=1; strcat(buf,"reply_report "); }
                if(u->autoihave&2) { x=1; strcat(buf,"powerup_2 "); }
                if(u->autoihave&1) { x=1; strcat(buf,"powerup_have "); }
                if(u->autoitook) { x=1; strcat(buf,"took_item "); }
                if(u->autoqpicked==2) { x=1; strcat(buf,"show&sync_powerup "); }
                else if(u->autoqpicked) { x=1; strcat(buf,"sync_powerup "); }
                if(!x) strcat(buf,"nothing");
                strcat(buf,"\n");
                q_print(u,buf);
            }
        }
#if 0
        else if(!memicmp(s,".tim",4))
        { // timer settings
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->secondcamp=0;
                u->remindcamp=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if(!memicmp(p,"gue",3))
                { // guess
                    u->secondcamp=1;
                }
                else if(!memicmp(p,"syn",3))
                { // sync
                    u->internalsync=1;
                }
                else if(!memicmp(p,"rem",3))
                { // remind
                    p=nextparam(&s);
                    u->remindcamp=atoi(p);
                    if(u->remindcamp<=0) u->remindcamp=10;
                }
            }

            {
                int x=0;
                sprintf(buf,"Active timer options:\n");
                if(u->remindcamp>0) { x=1; sprintf(buf+strlen(buf),"- remind at %i seconds\n",u->remindcamp);   }
                if(u->secondcamp)   { x=1; strcat(buf,"- guess collected at timeout\n");            }
                if(u->internalsync) { x=1; strcat(buf,"- syncronize with players in same proxy\n"); }
                if(!x) strcat(buf,"none\n");
                q_print(u,buf);
            }
        }
#endif
#ifdef SPEED
        else if(!memicmp(s,".sho",4))
        { // show
            int a=0;
            char *p;

            nextparam(&s);
            if(*s>32) a=atoi(s);

            u->showspeed=0;
            u->showtime=0;
            u->showdist=0;

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if(*p=='s')
                {
                    u->showspeed=1;
                }
                if(*p=='t')
                {
                    u->showtime=1;
                }
                if(*p=='h')
                {
                    u->showhud=1;
                }
                if(*p=='d') { u->showdist=1; } }
                u->showtimebase=u->time; u->showdistbase[0]=u->pos[0];
                u->showdistbase[1]=u->pos[1];
                u->showdistbase[2]=u->pos[2];

            if(!u->showspeed && !u->showtime && !u->showdist && !u->showhud)
            {
                q_print(u,"Show: nothing\n(options: speed dist time hug)\n");
            }
            else
            {
                strcpy(buf,"Show: ");
                if(u->showspeed) strcat(buf,"speed ");
                if(u->showtime) strcat(buf,"time ");
                if(u->showdist) strcat(buf,"distance ");
                if(u->showhud) strcat(buf,"hud ");
                strcat(buf,"\n");
                q_print(u,buf);
            }
            u->overlayactive=0;
        }
#endif
#ifdef PROMODE
        else if(!memicmp(s,".deb",4))
        {
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                *u->msgline=0;
                u->showtime=0;
                u->showspeed=0;
                u->hideaxemen=0;
            }

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if(!memicmp(p,"tim",3))
                { // time
                    u->showtime=1;
                    q_print(u,"Debug: show time\n");
                }
                else if(!memicmp(p,"spe",3))
                { // speed
                    u->showspeed=1;
                    q_print(u,"Debug: show time\n");
                }
                else if(!memicmp(p,"dip",3))
                { // dynamic ip test
                    int i;
                    for(i=0;i<MAXUSERS;i++)
                    {
                        memset(&user[i].toclientaddr,0,sizeof(SOCKADDR_IN));
                    }
                }
                else if(!memicmp(p,"mov",3))
                { // speed
                    u->moveoverlay=1;
                    q_print(u,"Debug: move overlay\n");
                }
                else if(!memicmp(p,"com",3))
                { // compress
                    int a;
                    p=nextparam(&s);
                    a=atoi(p);
                    u->compress_client=a&1;
                    u->compress_server=(a&2)>>1;

                    q_printf(u,"Debug: compress cli %i ser %i\n",
                        u->compress_client,u->compress_server);
                }
                else if(!memicmp(p,"cte",3))
                { // compress test
                    int a;

                    count_load(u);
                    count_save(u);

                    p=nextparam(&s);
                    a=atoi(p);
                    u->compress_test=a;

                    q_printf(u,"Debug: compress test %i\n",
                        u->compress_test);
                }
                else if(!memicmp(p,"ser",3))
                {
                    sprintf(buf,"Debug: servercmd %s sent\n",s);
                    q_print(u,buf);
                    q_servercmd(u,s);
                }
                else if(!memicmp(p,"axe",3))
                { // speed
                    u->hideaxemen=1;
                    q_print(u,"Debug: hide axe men (rocket arena)\n");
                }
                else if(!memicmp(p,"fpd",3))
                { // fpd clear
                    u->fpd=0;
                    u->fpdcopy=0;
                    q_print(u,"Debug: fpd cleared\n");
                }
                else if(!memicmp(p,"beg",3))
                { // restart
                    q_print(u,"Debug: restarting with begin\n");
                    q_printf(u,"begin %i\n",u->lastbegin);
                    q_servercmd(u,tmpbuf);
                }
                else if(!memicmp(p,"pin",3))
                { // ping fake
                    p=nextparam(&s);
                    u->fakeping=atoi(p);
                    q_printf(u,"Debug: ping test %i\n",u->fakeping);
                }
                else if(!memicmp(p,"fv1",3))
                { // yes f version
                    if(u->state==STATE_HERE)
                    {
                        char ver[16];
                        sprintf(ver,"%s",version);
                        cl_setinfo(u,"Cheapo",ver);
                    }
                    else
                    {
                        sprintf(tmpbuf,"setinfo Cheapo \"%s\"",version);
                        q_stufftext(u,tmpbuf);
                    }
                    u->nofversion=0;
                    q_print(u,"Debug: f_version enabled\n");
                }
                else if(!memicmp(p,"fv",2)) // also fv0
                { // no f version
                    if(u->state==STATE_HERE)
                    {
                        cl_setinfo(u,"Cheapo","");
                    }
                    else
                    {
                        sprintf(tmpbuf,"setinfo Cheapo \"\"");
                        q_stufftext(u,tmpbuf);
                    }
                    u->nofversion=1;
                    q_print(u,"Debug: f_version disabled\n");
                }
                else if(!memicmp(p,"poi",3))
                { // point
                    findpoint(u,u->pos[0],u->pos[1],u->pos[2],u->ang[0],u->ang[1],u->ang[2],-1,0,NULL);
                }
            }
        }
#endif
        else if(!memicmp(s,".sou",4))
        {
            uchar *p;
            nextparam(&s);
            if(u->fpd&FPD_SKIN)
            {
                *s=0;
                q_print(u,"Soundtrigger disabled.\n");
            }
            if(*s)
            {
                u->soundtrigger=0;
            }
            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if((*p>='A' && *p<='Z') || (*p>='a' && *p<='z'))
                { // do not allow letters
                    ;
                }
                else
                { // best
                    u->soundtrigger=*p;
                }
            }

            {
                if(u->soundtrigger) sprintf(buf,"Soundtrigger: %c",u->soundtrigger);
                else strcpy(buf,"Soundtrigger: disabled\n");
                strcat(buf,"\n");

                q_print(u,buf);
            }
        }
        else if(!memicmp(s,".wea",4))
        {
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                u->bestweapon=0;
            }
            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if(!memicmp(p,"bes",3))
                { // best
                    u->bestweapon=1;
                }
                else if(p[strlen(p)-1]==':')
                { // define/undefine own
                    int i,seq,imp,imp2;

                    imp=atoi(p);
                    if(imp<9)
                    {
                        q_print(u,"Cannot redefine sequences\n"
                                  "for impulses 1..8\n");
                        break;
                    }

                    // find if already defined
                    for(seq=0;seq<SEQUENCES;seq++)
                    {
                        if(u->impulseorder[seq][0]==imp) break;
                    }
                    if(seq==SEQUENCES)
                    {
                        for(seq=0;seq<SEQUENCES;seq++)
                        {
                            if(u->impulseorder[seq][0]==0)
                            {
                                u->impulseorder[seq][0]=imp;
                                break;
                            }
                        }
                    }
                    if(seq==SEQUENCES)
                    {
                        q_printf(u,"All %i sequences are defined.\n"
                                  "You have to delete an existing\n"
                                  "one with '.weapon #:' where #\n"
                                  "is the impulse to delete\n",SEQUENCES);
                        break;
                    }

                    for(i=1;i<15;i++) u->impulseorder[seq][i]=0;

                    for(i=1;i<10;i++) // limit to 10 impulses
                    {
                        p=nextparam(&s);
                        if(!*p) break;
                        imp2=atoi(p);
                        if(!p) break;

                        u->impulseorder[seq][i]=imp2;
                    }
                    if(i==1) u->impulseorder[seq][0]=0;
                }
                else if(atoi(p)==1)
                { // enable
                    u->bestweapon=1;
                }

                break;
            }

            {
                int seq,i,def=0;
                strcpy(buf,"Weapon: ");

                if(u->bestweapon) strcat(buf,"select best with ammo");
                else strcat(buf,"normal impulses");
                strcat(buf,"\n");
                q_print(u,buf);

                for(seq=0;seq<SEQUENCES;seq++)
                {
                    if(u->impulseorder[seq][0])
                    {
                        q_printf(u,"Sequence %i: ",u->impulseorder[seq][0]);
                        for(i=1;i<14;i++)
                        {
                            if(!u->impulseorder[seq][i]) break;
                            q_printf(u,"%i ",u->impulseorder[seq][i]);
                        }
                        q_print(u,"\n");
                        def=1;
                    }
                }
            }
        }
        else if(!memicmp(s,".ski",4))
        {
            uchar *p;
            nextparam(&s);
            if(u->fpd&FPD_SKIN)
            {
                q_print(u,"Skin override disabled.\n");
                u->skinoverride=0;
            }
            else if(u->version<22)
            {
                q_print(u,"Skin command requires QW 2.21 or newer\n");
                u->skinoverride=0;
            }
            else if(u->fortress)
            {
                q_print(u,"Skin override disabled in TF.\n");
                u->skinoverride=0;
            }
            else
            {
                if(*s)
                {
                    u->skinoverride=1;

                    p=nextparam(&s);
                    strncpy(u->skinname,p,15);

                    p=nextparam(&s);
                    if(*p>=32) strncpy(u->skinname2,p,15);
                    else       strcpy(u->skinname2,"base");

                    sprintf(buf,"Skin override: us \'%s\' them \'%s\'\n",u->skinname,u->skinname2);
                    q_print(u,buf);

                    u->skinoverridepos=0;
                }
                else
                {
                    if(!(u->fpd&FPD_SKIN)) q_print(u,"Skin override: off\n");

                    u->skinoverride=-1; // clear
                    u->skinoverridepos=0;
                }

                //u->time_reskinall=u->time;
                if(u->time_reskinall==0) u->time_reskinall=u->time;
            }
        }
        else if(!memicmp(s,".col",4))
        {
            uchar *p;
            nextparam(&s);
            if(u->fpd&FPD_COLOR)
            {
                q_print(u,"Color override disabled.\n");
                u->coloroverride=0;
            }
            else if(u->version<22)
            {
                q_print(u,"Color command requires QW 2.21 or newer\n");
                u->coloroverride=0;
            }
            else if(u->fortress)
            {
                q_print(u,"Color override disabled in TF.\n");
                u->coloroverride=0;
            }
            else
            {
                if(*s)
                {
                    char buf[64];

                    u->coloroverride=1;

                    p=nextparam(&s);
                    u->color=text2color(p);

                    p=nextparam(&s);
                    u->color2=text2color(p);

                    if(u->color2==-2) u->color=-1;

                    strcpy(buf,"Color override: us ");
                    strcat(buf,color2text(u->color));
                    strcat(buf,"them ");
                    strcat(buf,color2text(u->color2));
                    strcat(buf,"\n");
                    q_print(u,buf);

                    u->skinoverridepos=0;
                }
                else
                {
                    if(!(u->fpd&FPD_COLOR)) q_print(u,"Color override: off\n");

                    u->coloroverride=-1; // clear
                    u->skinoverridepos=0;
                }

                //u->time_reskinall=u->time;
                if(u->time_reskinall==0) u->time_reskinall=u->time;
            }
        }
        else if(!memicmp(s,".fps",4))
        { // fps enhancement
            {
                while(*s>32) s++;
                while(*s<=32 && *s) s++;
                if(*s)
                {
                    u->fps_explosion=0;
                    u->fps_powerup=0;
                    u->fps_muzzle=0;
                    u->fps_gib=0;
                    u->fps_rocket=0;
                    u->fps_setinfo=0;
                    u->fps_damage=0;
                    u->fps_nails=0;
                }

                {
                    if(*s>='0' && *s<='9') u->fps_explosion=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_powerup=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_muzzle=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_gib=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_rocket=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_setinfo=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_nails=atoi(s);
                    while(*s>32) s++;
                    while(*s<=32 && *s) s++;
                    if(*s>='0' && *s<='9') u->fps_damage=atoi(s);
                }
            }
            sprintf(buf,"FPS enhancement: %i %i %i %i %i %i %i %i\n",
                u->fps_explosion,u->fps_powerup,u->fps_muzzle,
                u->fps_gib,u->fps_rocket,
                u->fps_setinfo,u->fps_nails,u->fps_damage);
            q_print(u,buf);
            switch(u->fps_explosion) // explosion
            {
                default:
                    u->fps_explosion=0;
                case 0: // normal
                    q_print(u,"- Explosions:    normal\n");
                    break;
                case 1: // tarbaby
                    q_print(u,"- Explosions:    tarbaby\n");
                    break;
                case 2: // teleport
                    q_print(u,"- Explosions:    teleport\n");
                    break;
                case 3: // blood
                    q_print(u,"- Explosions:    blood\n");
                    break;
                case 4: // nothing
                    q_print(u,"- Explosions:    nothing\n");
                    break;
                case 5: // shotgun
                    q_print(u,"- Explosions:    shotgun\n");
                    break;
            }
            switch(u->fps_powerup) // powerupp glow
            {
                default:
                    u->fps_powerup=0;
                case 0: // normal
                    q_print(u,"- Powerup glow:  normal\n");
                    break;
                case 1: // oma pois
                    q_print(u,"- Powerup glow:  own off\n");
                    break;
                case 2: // kaikki pois
                    q_print(u,"- Powerup glow:  all off\n");
                    break;
            }
            switch(u->fps_muzzle) // muzzle
            {
                default:
                    u->fps_muzzle=0;
                case 0: // normal
                    q_print(u,"- Muzzle flash:  normal\n");
                    break;
                case 1: // oma pois
                    q_print(u,"- Muzzle flash:  own off\n");
                    break;
                case 2: // kaikki pois
                    q_print(u,"- Muzzle flash:  all off\n");
                    break;
            }
            switch(u->fps_gib) // gib filter
            {
                default:
                    u->fps_gib=0;
                case 0: // normal (off)
                    q_print(u,"- Gibs/bodies:   normal\n");
                    break;
                case 1: // on
                    q_print(u,"- Gibs/bodies:   removed\n");
                    break;
                case 2: // on
                    q_print(u,"- Gibs/bodies:   only bodies\n");
                    break;
            }
            switch(u->fps_rocket) // rocket cchange
            {
                default:
                    u->fps_muzzle=0;
                case 0: // normal
                    q_print(u,"- Rockets:       normal\n");
                    break;
                case 1: // no glow
                    q_print(u,"- Rockets:       grenade (no glow)\n");
                    break;
            }
            switch(u->fps_setinfo) // setinfoo filter
            {
                default:
                    u->fps_setinfo=0;
                case 0: // normal
                    q_print(u,"- Setinfo:       not supported\n");
                    break;
            }
            switch(u->fps_nails) // nails fillter
            {
                default:
                    u->fps_nails=0;
                case 0:
                    q_print(u,"- Nails:         normal\n");
                    break;
                case 1:
                    q_print(u,"- Nails:         normal\n");
                    break;
                case 2:
                    q_print(u,"- Nails:         1/2\n");
                    break;
                case 3:
                    q_print(u,"- Nails:         2/3\n");
                    break;
                case 4:
                    q_print(u,"- Nails:         3/4\n");
                    break;
            }
            switch(u->fps_damage) // damage ffilter
            {
                default:
                    u->fps_damage=0;
                case 0: // normal
                    q_print(u,"- Damage flash:  normal\n");
                    break;
                case 1: // normal
                    q_print(u,"- Damage flash:  filtered out\n");
                    break;
            }
            // reset entity table
            memset(u->entitygib,0,MAXENTITIES);
        }
        else if(!memicmp(s,".lin",4))
        { // line
            while(*s>32) s++;
            while(*s<=32 && *s) s++;
            u->linesright=36;
            if(!memicmp(s,"640",3) || !memicmp(s,"gl",2))
            {
                u->linesdown=45;
            }
            else if(!memicmp(s,"512",3))
            {
                u->linesdown=33;
            }
            else if(!memicmp(s,"320",3) || !memicmp(s,"vg",2))
            {
                u->linesdown=11;
            }
            else if(*s>='0' && *s<'9')
            {
                u->linesdown=atoi(s);
            }
            while(*s>32) s++;
            while(*s<=32 && *s) s++;

            if(!memicmp(s,"cen",3)) u->linecenter=1;
            else u->linecenter=0;

            if(u->linesdown>50) u->linesdown=50;
            if(u->linesright>50) u->linesright=50;

            if(u->linecenter) sprintf(buf,"Screen lines: %i, center timers\n",u->linesdown);
            else sprintf(buf,"Screen lines: %i\n",u->linesdown);
            q_print(u,buf);
        }
        else if(!memicmp(s,".ite",4))
        { // items
            int i,c;
            int intlist;

            nextparam(&s);
            intlist=atoi(s);
#ifndef PROMODE
            intlist=0;
#endif
            if(intlist<=0) intlist=0;

            if(intlist==2)
            {
                q_print(u,"[visible item]\n");
                for(i=0;i<MAXENTITIES;i++) if(u->entityvisible[i])
                {
                    sprintf(buf,"%-8s (%i,%i,%i)\n",
                                u->entityname[i],
                                u->entitypos[i][0],
                                u->entitypos[i][1],
                                u->entitypos[i][2]);
                    q_print(u,buf);
                }
            }
            else
            {
                q_print(u,"[item]  [nearest mark]\n");
                for(c=CAMP_LASTAUTOMARK;c>=1;c>>=1)
                {
                    if(c==CAMP_LASTPOWER ||
                       c==CAMP_LASTARMOR ||
                       c==CAMP_LAST)
                    {
                        uchar tmp[36];
                        memset(tmp,0x9e,31);
                        tmp[0]=0x9d;
                        tmp[30]=0x9f;
                        tmp[31]='\n';
                        tmp[32]=0;
                        q_print(u,tmp);
                    }
                    for(i=0;i<u->camps;i++)
                    {
                        Camp  *ca;
                        uchar *text;
                        ca=&u->camp[i];
                        if(ca->category&c)
                        {
                            char name[16];

                            strcpy(name,ca->genericname);
                            name[strlen(name)+1]=0;
                            name[strlen(name)]=ca->name[strlen(ca->name)-1];

                            text=findnear(u,ca->pos[0],ca->pos[1],ca->pos[2],NULL);
                            if(!intlist) sprintf(buf,"%-8s near %s\n",name,text);
                            else         sprintf(buf,"%-8s %s (%i,%i,%i)\n",
                                           name,text,ca->pos[0],ca->pos[1],ca->pos[2]);
                            q_print(u,buf);
                        }
                    }
                }
            }

            if(intlist)
            {
                sprintf(buf,"you (%i,%i,%i) -> (%i,%i,%i)\n",
                    u->pos[0],u->pos[1],u->pos[2],
                    u->ang[0],u->ang[1],u->ang[2]);
                q_print(u,buf);
            }
        }
        else if(!memicmp(s,".sta",4))
        {
#if !defined(__APPLE__) && !defined(MACOSX)
            int firstround=1;
            int pingcnt=0;
#endif /* !__APPLE__ && !MACOSX */
            int i,n,loss=0,ping=0,avgs=0,avgc=0,ratio;
            uchar *cmd,*p;

            if(u->state==STATE_HERE)
            {
                q_print(u,"not connected\n");
                return;
            }

            { // init stuff
                for(n=0;n<STATNUM/2;n++)
                {
                    i=(u->lastpacketrecv+4+n)&(STATNUM-1);
                    if(!u->packetrecvtime[i]) loss++;
                }
                loss=200*loss/STATNUM;

                for(n=0;n<16;n++)
                {
                    avgs+=u->packetserversize[n];
                    avgc+=u->packetclientsize[n];
                }
                avgs=(avgs+8)/16;
                avgc=(avgc+8)/16;

                ping=u->server_ping;

                if(u->cpps) ratio=100*u->cppsdrop/(u->cppsdrop+u->cppspass);
                else ratio=0;

            }

            cmd=s;
            p=nextparam(&s);
            if(*s<=32) s="pro ser net";
            else s=cmd;

            for(;;)
            {
                p=nextparam(&s);
                if(!*p) break;

                if(!memicmp(p,"all",3)) p=NULL;

                if(!p || !memicmp(p,"pro",3))
                { // proxies
                    if(u->linkedclient) q_print(u,"Client side proxy present.\n");
                    if(u->linkedserver) q_print(u,"Server side proxy present.\n");
                }

                if(!p || !memicmp(p,"ser",3))
                { // default stats
                    q_printf(u,"Serverside statistics:\n"
                               "data to server  : %i bytes/sec\n"
                               "data to client  : %i bytes/sec\n"
                               ".cpps dropping  : %i%%\n"
                               "packet loss     : %i%%\n"
                               "average ping    : %i ms\n",
                               u->bytestoserversec,
                               u->bytestoclientsec,
                               ratio,loss,ping);
                }

                if(!p || !memicmp(p,"ext",3))
                { // extended stats
                    q_printf(u,
                           "Extended statistics:\n"
                           "out of order-pkt: %i\n"
                           "data to server  : %i KB\n"
                           "data to client  : %i KB\n"
                           "aver server-pkt : %i bytes\n"
                           "aver client-pkt : %i bytes\n"
                           "peak server-pkt : %i bytes\n"
                           "peak client-pkt : %i bytes\n",
                           u->outoforder,
                           u->bytestoserver/1024,
                           u->bytestoclient/1024,
                           avgs,
                           avgc,
                           u->bytestoserversecmax,
                           u->bytestoclientsecmax);
                }

                if(!p || !memicmp(p,"pin",3))
                { // packet pings
                    strcpy(buf,"Pings for last 8 packets:\n");
                    for(n=0;n<8;n++)
                    {
                        i=(STATNUM+u->lastpacketrecv-1-n)&(STATNUM-1);
                        if(!u->packetrecvtime[i]) sprintf(buf+strlen(buf),"  L ");
                        else                      sprintf(buf+strlen(buf),"%3i ",u->packetrecvtime[i]-u->packetsendtime[i]);
                    }
                    strcat(buf,"\n");
                    q_print(u,buf);
                }

                if(!p || !memicmp(p,"net",3))
                { // netgraph
                    strcpy(buf,"Netgraph for last 30 packets:\n");
                    q_print(u,buf);

                    for(n=0;n<30;n++)
                    {
                        i=(STATNUM+u->lastpacketrecv-1-n)&(STATNUM-1);
                        if(!u->packetrecvtime[i])  buf[n]='L';
                        else if(u->packetchoke[i]) buf[n]='=';
                        else                       buf[n]='-';
                    }
                    buf[n]='\n';
                    buf[n+1]=0;
                    q_print(u,buf);
                }

                if(p && !memicmp(p,"res",3))
                { // reset
                    u->bytestoserver=0;
                    u->bytestoclient=0;
                    u->bytestoserversecmax=0;
                    u->bytestoclientsecmax=0;
                    u->outoforder=0;
                }
            }
        }
        else if(!memicmp(s,".dmm",4))
        {
            nextparam(&s);
            if(*s<=32) ;
            else u->forcedmm=atoi(s);
            if(u->forcedmm<0) u->forcedmm=0;
            if(u->forcedmm>3) u->forcedmm=3;

            if(u->forcedmm) q_printf(u,"Dm-mode: assume %i\n",u->forcedmm);
            else q_printf(u,"Dm-mode: autodetect\n");

            dmmchange(u,u->originaldmm);
        }
        else if(!memicmp(s,".qui",4))
        {
            nextparam(&s);
            if(*s<=32)
            {
                u->quiet=10; // for 10 packets
            }
            else
            {
                if(atoi(s)) u->quiet=0x7fffffff;
                else u->quiet=0;
            }
        }
        else if(!memicmp(s,".who",4))
        {
            int  i;
            int  mode;

            nextparam(&s);
            if(*s=='f') mode=2;
            else if(*s=='p') mode=1;
            else mode=0;

            if(mode==2)      sprintf(buf,"[name]            [fps]\n");
            else if(mode==1) sprintf(buf,"[name]            [proxy]\n");
            else             sprintf(buf,"[name]            [team]  [skin]\n");
            q_print(u,buf);

            for(i=0;i<34;i++) if(u->playeractive[i])
            {
                if(mode==2)
                {
                    if(u->playernear[i])
                    {
                        sprintf(buf,"%-17s  %3i\n",
                                    u->playername[i],u->playerfps[i]);
                    }
                    else
                    {
                        if(u->playerfps[i])
                        {
                            sprintf(buf,"%-17s (%3i)\n",
                                        u->playername[i],u->playerfps[i]);
                        }
                        else
                        {
                            sprintf(buf,"%-17s ( - )\n",
                                        u->playername[i]);
                        }
                    }
                }
                else if(mode==1)
                {
                    sprintf(buf,"%-17s %s\n",u->playername[i],u->playerproxy[i]);
                }
                else
                {
                    sprintf(buf,"%-17s %-7s %-7s\n",
                                u->playername[i],u->playerteamtext[i],u->playerskin[i]);
                }
                q_print(u,buf);
            }
        }
        else if(!memicmp(s,".fil",4))
        { // message filter
            uchar *p;
            nextparam(&s);
            if(*s)
            {
                int i,j=0;
                for(i=0;i<15;i++)
                {
                    p=nextparam(&s);
                    if(!*p) break;
                    if(*p!='#')
                    {
                        q_print(u,"group must start with '#'\n");
                        i=0;
                        break;
                    }
                    else
                    {
                        strncpy(u->filter[j],p,8);
                        u->filter[j][8]=0;
                        j++;
                    }
                }
                u->filternum=i;
            }

            q_print(u,"Your message filter groups:\n");
            if(!u->filternum) q_print(u,"none\n");
            else
            {
                int i;
                for(i=0;i<u->filternum;i++)
                {
                    q_printf(u,"%-10s",u->filter[i]);
                    if((i%3)==2) q_print(u,"\n");
                }
                if((i%3)!=0) q_print(u,"\n");
            }
        }
#ifdef IDENT
        else if(!memicmp(s,".ide",4))
        {
            Userinfo ui;
            int      num;

            nextparam(&s);
            if(*s=='0')
            {
                num=u->entity;
            }
            else if(*s=='f')
            {
                nextparam(&s);
                strncpy(u->requestformat,s,63);
                q_printf(u,"Ident format: %s\n",u->requestformat);
                return;
            }
            else
            {
                num=atoi(findpoint(u,u->pos[0],u->pos[1],u->pos[2],u->ang[0],u->ang[1],u->ang[2],CAMP_PLAYER,POINT_TEAMMATENUMBER));
            }

//            sprintf(tmpbuf,"num=%i act=%i team:%08X==%08X\n",
//                num,u->playeractive[num],u->playerteam[num],u->team);

            if(num && u->playeractive[num] && u->playerteam[num]==u->team)
            {
                if(u->playerproxy[num]>=10000 || u->playerproxy[num]<211)
                {
                    // user doesn't have a proxy with this feature
                    strcpy(u->msgline,u->playername[num]);
                    stror80(u->msgline);
                    u->time_nextcamp=u->time;
                }
                else
                {
                    ui.type=USERINFO_REQUEST;
                    ui.requestfrom=num;
                    ui.requestlong=0;
                    u->time_request=u->time;
                    u->requestfrom=ui.requestfrom;

                    // create a setinfo command
                    sendinfomessage(u,encryptinfo(&ui));
                }
            }
        }
#endif
        else
        {
            q_printf(u,"Unknown proxy command: %s\n",s);
        }
    }
}


