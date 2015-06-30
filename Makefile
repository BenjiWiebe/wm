CFLAGS=-Wall -Werror -Wextra -g -O0 -std=gnu99
PREFIX?=/usr
PROG=wm
GROUP=msg
SPOOLDIR=/var/spool/msg

all: $(PROG)

$(PROG): $(PROG).c
	gcc $(CFLAGS) $^ -o $@

perms: $(PROG)
	chown root:$(GROUP) $(PROG)
	chmod a-rwx,a+x,g+s $(PROG)

install: $(PROG)
	@getent group $(GROUP) >/dev/null || ( echo 'No such group: $(GROUP). Please create it and try again.' && exit 1 )
	install -g msg -o root -s -m 2111 ./$(PROG) $(PREFIX)/bin
	install -g root -o root -m 0755 ./wm-register $(PREFIX)/bin
	install -g root -o root -m 0755 ./wm-unregister $(PREFIX)/bin
	install -g root -o root -m 0644 ./$(PROG)-complete.sh /etc/bash_completion.d/$(PROG)
	install -g root -o root -m 0755 -d $(SPOOLDIR)
	install -g root -o root -m 0644 ./$(PROG)-prompt-command.sh /etc/profile.d/
	@echo
	@echo "Don't forget to run wm-register <user> for all users that will be using $(PROG)."

clean:
	rm -f $(PROG)

.PHONY: perms install clean
