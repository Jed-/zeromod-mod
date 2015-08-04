 /* See LICENSE file for license details. */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static char *host = (char*)"irc.oftc.net";
static char *port = (char*)"6667";
static char *password;
static char nick[32];
static char bufin[4096];
static char bufout[4096];
static char channel[256];
static time_t trespond;
static FILE *srv;
static char *sauerhost = (char*)"127.0.0.1";
static int sauerport = 28786;

#include "util.c"

void initialize_enet();
void sendbuf(char *hostname, int port, char *channel, char *_buf);

static void
pout(char *channel, char *fmt, ...) {
	static char timestr[18];
	time_t t;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	t = time(NULL);
	strftime(timestr, sizeof timestr, "%D %R", localtime(&t));
	fprintf(stdout, "%-12s: %s %s\n", channel, timestr, bufout);
	sendbuf(sauerhost, sauerport, channel, bufout);
}

static void
sout(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	fprintf(srv, "%s\r\n", bufout);
}

static void
privmsg(char *channel, char *msg) {
	if(channel[0] == '\0') {
		pout((char*)"", (char*)"No channel to send to");
		return;
	}
	pout(channel, (char*)"<%s> %s", nick, msg);
	sout((char*)"PRIVMSG %s :%s", channel, msg);
}

static void
parsein(char *s) {
	char c, *p;

	if(s[0] == '\0')
		return;
	skip(s, '\n');
	if(s[0] != ':') {
		privmsg(channel, s);
		return;
	}
	c = *++s;
	if(c != '\0' && isspace(s[1])) {
		p = s + 2;
		switch(c) {
		case 'j':
			sout((char*)"JOIN %s", p);
			if(channel[0] == '\0')
				strlcpy(channel, p, sizeof channel);
			return;
		case 'l':
			s = eat(p, isspace, 1);
			p = eat(s, isspace, 0);
			if(!*s)
				s = channel;
			if(*p)
				*p++ = '\0';
			if(!*p)
				p = (char*)"sic - 250 LOC are too much!";
			sout((char*)"PART %s :%s", s, p);
			return;
		case 'm':
			s = eat(p, isspace, 1);
			p = eat(s, isspace, 0);
			if(*p)
				*p++ = '\0';
			privmsg(s, p);
			return;
		case 's':
			strlcpy(channel, p, sizeof channel);
			return;
		}
	}
	sout((char*)"%s", s);
}

static void
parsesrv(char *cmd) {
	char *usr, *par, *txt;

	usr = host;
	if(!cmd || !*cmd)
		return;
	if(cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if(cmd[0] == '\0')
			return;
		skip(usr, '!');
	}
	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');
	trim(par);
	if(!strcmp((char*)"PONG", cmd))
		return;
	if(!strcmp((char*)"PRIVMSG", cmd))
		pout(par, (char*)"<%s> %s", usr, txt);
	else if(!strcmp((char*)"PING", cmd))
		sout((char*)"PONG %s", txt);
	else {
		pout(usr, (char*)">< %s (%s): %s", cmd, par, txt);
		if(!strcmp((char*)"NICK", cmd) && !strcmp(usr, nick))
			strlcpy(nick, txt, sizeof nick);
	}
}

int
main(int argc, char *argv[]) {
	initialize_enet();
	int i, c;
	struct timeval tv;
	const char *user = getenv((char*)"USER");
	fd_set rd;

	strlcpy(nick, user ? user : "unknown", sizeof nick);
	for(i = 1; i < argc; i++) {
		c = argv[i][1];
		if(argv[i][0] != '-' || argv[i][2])
			c = -1;
		switch(c) {
		case 'h':
			if(++i < argc) host = argv[i];
			break;
		case 'p':
			if(++i < argc) port = argv[i];
			break;
		case 'n':
			if(++i < argc) strlcpy(nick, argv[i], sizeof nick);
			break;
		case 'k':
			if(++i < argc) password = argv[i];
			break;
		case 'v':
			eprint((char*)"sic-1.2, © 2005-2012 Kris Maglione, Anselm R. Garbe, Nico Golde\n");
			break;
		case 'i':
			if(++i < argc) sauerhost = argv[i];
			break;
		case 'q':
			if(++i < argc) sauerport = atoi(argv[i]);
			break;
		default:
			eprint((char*)"usage: sic [-h host] [-p port] [-n nick] [-k keyword] [-v]\n");
		}
	}
	/* init */
	i = dial(host, port);
	srv = fdopen(i, "r+");
	/* login */
	if(password)
		sout((char*)"PASS %s", password);
	sout((char*)"NICK %s", nick);
	sout((char*)"USER %s localhost %s :%s", nick, host, nick);
	fflush(srv);
	setbuf(stdout, NULL);
	setbuf(srv, NULL);
	for(;;) { /* main loop */
		FD_ZERO(&rd);
		FD_SET(0, &rd);
		FD_SET(fileno(srv), &rd);
		tv.tv_sec = 120;
		tv.tv_usec = 0;
		i = select(fileno(srv) + 1, &rd, 0, 0, &tv);
		if(i < 0) {
			if(errno == EINTR)
				continue;
			eprint((char*)"sic: error on select():");
		}
		else if(i == 0) {
			if(time(NULL) - trespond >= 300) {
				eprint((char*)"sic shutting down: parse timeout\n");
				break;
			}
			sout((char*)"PING %s", host);
			continue;
		}
		if(FD_ISSET(fileno(srv), &rd)) {
			if(fgets(bufin, sizeof bufin, srv) == NULL) {
				eprint((char*)"sic: remote host closed connection\n");
				break;
			}
			parsesrv(bufin);
			trespond = time(NULL);
		}
		if(FD_ISSET(0, &rd)) {
			if(fgets(bufin, sizeof bufin, stdin) == NULL) {
				eprint((char*)"sic: broken pipe\n");
				break;
			}
			parsein(bufin);
		}
	}
	return 0;
}
