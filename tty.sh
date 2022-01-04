# This file should be placed at /etc/profile.d/, using that to disable Modem's escape sequence.

TTY=`tty`

if [[ $TTY == /dev/tty* ]]; then
        echo "You've login via a Serial/Modem Console."
        export TMOUT=300
        echo "The shell will automatically logout after "$TMOUT" secs."
fi
