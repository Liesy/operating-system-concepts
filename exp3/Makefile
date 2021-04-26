head=exp3.h
srcs=exp3.c
objs=exp3.o
opts=-g -c
all: exp3
pctl:	$(objs)
	gcc $(objs) -o exp3
pctl.o:	$(srcs)	$(head)
	gcc $(opts)	$(srcs)
clean:
	rm exp3 *.o
