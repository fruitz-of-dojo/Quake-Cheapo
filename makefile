NODEBUG=1

!include <win32.mak>

clflags = -Ox -W3 -nologo -DWIN32
ldflags = $(lflags) $(ldebug)
ldlibs  = $(conlibs) wsock32.lib user32.lib

objs = main.obj compress.obj cmd.obj proxy.obj misc.obj

all: cheapo.exe

cheapo.exe: $(objs)
    $(link) $(ldflags) -out:$@ $(objs) $(ldlibs)

.c.obj:
    $(cc) $(clflags) -c $*.c

main.c  : proxy.h
proxy.c : proxy.h
cmd.c   : proxy.h
misc.c  : proxy.h

