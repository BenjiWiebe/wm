CFLAGS=-Wall -Werror -Wextra -g -O0 -std=gnu99

all: wm.unicode

wm.unicode: wm.unicode.c
	gcc $(CFLAGS) $^ -o $@
	sudo chown root:msg $@
	sudo chmod a-rwx,a+x,g+s $@

#edit2: edit2.c tty.c
#	gcc $(CFLAGS) $^ -o $@
