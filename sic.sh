#!/bin/bash
echo '' > chat.txt
f_() {
	tail -f chat.txt | sic/sic -h irc.andrius4669.org -n Bud
}
f_ >> /dev/null &
sleep 5
echo ':j #bud' > chat.txt
