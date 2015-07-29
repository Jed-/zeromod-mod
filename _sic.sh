#!/bin/bash
g_() {
	echo '' > chat.txt
	sleep 5
	echo ':j #channel' > chat.txt
}
f_() {
        while true;
        do
        g_ &
        tail -f chat.txt | sic/sic -h server.com -n BotName
        done;
}
f_
