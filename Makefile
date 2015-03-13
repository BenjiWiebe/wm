CFLAGS=-Wall -Werror -Wextra -g -O0 -std=gnu99

all: wm

wm: wm.c
	gcc $(CFLAGS) $^ -o $@

perms: wm
	chown root:msg wm
	chmod a-rwx,a+x,g+s wm

install: wm
	install -g msg -o root -s -m 2111 ./wm /usr/misc/bin/wm

clean:
	rm -f wm

.PHONY: perms install clean
