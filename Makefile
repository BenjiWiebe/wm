CFLAGS=-Wall -Werror -Wextra -g -O0 -std=gnu99

all: wm

wm: wm.c
	gcc $(CFLAGS) $^ -o $@
	sudo chown root:msg $@
	sudo chmod a-rwx,a+x,g+s $@
