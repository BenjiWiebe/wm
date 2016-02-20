complete -W "$(echo /var/spool/msg/*|sed 's|/var/spool/msg/||g')" wm
