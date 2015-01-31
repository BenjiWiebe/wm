CFLAGS=-Wall -Werror -Wextra -g -O0 -std=gnu99

all: wm

wm: wm.c
	gcc $(CFLAGS) $^ -o $@

perms:
	sudo chown root:msg wm
	sudo chmod a-rwx,a+x,g+s wm

.PHONY: perms
