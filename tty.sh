# This file should be placed at /etc/profile.d/, using that to disable Modem's escape sequence.

TTY=`tty`

if [[ $TTY == /dev/tty* ]]; then
        echo "You've login via a Serial/Modem Console."
        export TMOUT=300
        echo "The shell will automatically logout after "$TMOUT" secs."
        fflush
        stty -echo

        sleep 1.5
        echo -ne +++
        sleep 1.5
        echo ATS2=128
        read -t 0.1 -N 16 discard #Modem 开着回显，要读两次
        read -t 0.1 -N 16 discard
        sleep 0.3
        echo ATO0
        read -t 0.1 -N 16 discard
        read -t 0.1 -N 16 discard
        sleep 0.2
        read -t 0.1 -N 128 discard
        stty echo
        echo -ne '\b\b\b'
fi
