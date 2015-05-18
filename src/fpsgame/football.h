void sendpart(int index, float x, float y, float z, int attr1, int attr2, int attr3, int attr4, int attr5) {
	clientinfo *ci;
	if(firstbar() > -1) ci = (clientinfo *)getclientinfo(firstbar());
	else if(clients[0]) ci = clients[0];
	if(!ci) return;
	flushserver(true);
	uchar buf[MAXTRANS];
	ucharbuf b(buf, sizeof(buf));
	putint(b, N_EDITENT);
	putint(b, index);
	putint(b, int(x*DMF)); putint(b, int(y*DMF)); putint(b, int(z*DMF));
	putint(b, 5);
	putint(b, attr1); putint(b, attr2); putint(b, attr3); putint(b, attr4); putint(b, attr5);
	packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
	putint(p, N_CLIENT);
	putint(p, ci->clientnum);
	putint(p, b.len);
	p.put(buf, b.len);
	sendpacket(-1, 1, p.finalize());
}
vec ballspawn(0, 0, 0), areamin(0, 0, 0), areamax(0, 0, 0), goalgoodmin(0, 0, 0), goalgoodmax(0, 0, 0), goalevilmin(0, 0, 0), goalevilmax(0, 0, 0);
void setballspawn(int x, int y, int z) {
	ballspawn.x = x; ballspawn.y = y; ballspawn.z = z;
}
void setareamin(int x, int y, int z) {
	areamin.x = x; areamin.y = y; areamin.z = z;
}
void setareamax(int x, int y, int z) {
	areamax.x = x; areamax.y = y; areamax.z = z;
}
void setgoalgoodmin(int x, int y, int z) {
	goalgoodmin.x = x; goalgoodmin.y = y; goalgoodmin.z = z;
}
void setgoalgoodmax(int x, int y, int z) {
	goalgoodmax.x = x; goalgoodmax.y = y; goalgoodmax.z = z;
}
void setgoalevilmin(int x, int y, int z) {
	goalevilmin.x = x; goalevilmin.y = y; goalevilmin.z = z;
}
void setgoalevilmax(int x, int y, int z) {
	goalevilmax.x = x; goalevilmax.y = y; goalevilmax.z = z;
}
ICOMMAND(ballspawn, "iii", (int *x, int *y, int *z), setballspawn(*x, *y, *z));
ICOMMAND(areamin, "iii", (int *x, int *y, int *z), setareamin(*x, *y, *z));
ICOMMAND(areamax, "iii", (int *x, int *y, int *z), setareamax(*x, *y, *z));
ICOMMAND(goalgoodmin, "iii", (int *x, int *y, int *z), setgoalgoodmin(*x, *y, *z));
ICOMMAND(goalgoodmax, "iii", (int *x, int *y, int *z), setgoalgoodmax(*x, *y, *z));
ICOMMAND(goalevilmin, "iii", (int *x, int *y, int *z), setgoalevilmin(*x, *y, *z));
ICOMMAND(goalevilmax, "iii", (int *x, int *y, int *z), setgoalevilmax(*x, *y, *z));

VAR(ballidx, 0, 0, 10000);
int ballattrneutral[5] = {3, 2, 4000, 0, 0};
int ballattrgood[5] = {3, 2, 175, 0, 0};
int ballattrevil[5] = {3, 2, 3840, 0, 0};
void setballattrneutral(int a1, int a2, int a3, int a4, int a5) {
	ballattrneutral[0] = a1; ballattrneutral[1] = a2; ballattrneutral[2] = a3; ballattrneutral[3] = a4; ballattrneutral[4] = a5;
}
void setballattrgood(int a1, int a2, int a3, int a4, int a5) {
	ballattrgood[0] = a1; ballattrgood[1] = a2; ballattrgood[2] = a3; ballattrgood[3] = a4; ballattrgood[4] = a5;
}
void setballattrevil(int a1, int a2, int a3, int a4, int a5) {
	ballattrevil[0] = a1; ballattrevil[1] = a2; ballattrevil[2] = a3; ballattrevil[3] = a4; ballattrevil[4] = a5;
}
ICOMMAND(ballattrneutral, "iiiii", (int *a1, int *a2, int *a3, int *a4, int *a5), setballattrneutral(*a1, *a2, *a3, *a4, *a5));
ICOMMAND(ballattrgood, "iiiii", (int *a1, int *a2, int *a3, int *a4, int *a5), setballattrgood(*a1, *a2, *a3, *a4, *a5));
ICOMMAND(ballattrevil, "iiiii", (int *a1, int *a2, int *a3, int *a4, int *a5), setballattrevil(*a1, *a2, *a3, *a4, *a5));
int score[2] = {0, 0};
int lasthit = -1;
int footballmillis = 0;
SVAR(footballmap, "");

VAR(ballplayerdist, 0, 16, 64);
FVAR(ballshootdiv, 1.f, 2.25f, 20.f);
FVAR(ballcontroldiv, 1.f, 5.5f, 20.f);
FVAR(ballfloorbounce, 0.f, 0.6f, 1.f);
FVAR(ballceilingbounce, 0.f, 0.1f, 1.f);
FVAR(ballwallbounce, 0.f, 0.35f, 1.f);
FVAR(ballgravity, 0.f, 1.f/5000.f, 1.f);
FVAR(ballfriction, 0.f, 0.000075f, 1.f);
FVAR(ballairresistance, 0.f, 0.0002f, 1.f);

VAR(ballout, 0, 0, 1000000);

struct bouncer {
	vec min, max;
	bouncer() : min(0, 0, 0), max(0, 0, 0) {
	}
	void setmin(int x, int y, int z) {
		min = vec(x, y, z);
	}
	void setmax(int x, int y, int z) {
		max = vec(x, y, z);
	}
};
vector<bouncer> bouncers;
void ballbouncer(int x1, int y1, int z1, int x2, int y2, int z2) {
	bouncer b;
	b.setmin(x1, y1, z1);
	b.setmax(x2, y2, z2);
	bouncers.add(b);
}
ICOMMAND(ballbouncer, "iiiiii", (int *x1, int *y1, int *z1, int *x2, int *y2, int *z2), ballbouncer(*x1, *y1, *z1, *x2, *y2, *z2));

vec ball(0, 0, 0), ballvel(0, 0, 0);

void sendball() {
	int t = -1;
	clientinfo *ci = (clientinfo *)getclientinfo(lasthit);
	t = ci ? isteam(ci->team, "good") ? 0 : 1 : -1;
	sendpart(ballidx, ball.x, ball.y, ball.z, t==-1 ? ballattrneutral[0] : t==0 ? ballattrgood[0] : ballattrevil[0], t==-1 ? ballattrneutral[1] : t==0 ? ballattrgood[1] : ballattrevil[1], t==-1 ? ballattrneutral[2] : t==0 ? ballattrgood[2] : ballattrevil[2], t==-1 ? ballattrneutral[3] : t==0 ? ballattrgood[3] : ballattrevil[3], t==-1 ? ballattrneutral[4] : t==0 ? ballattrgood[4] : ballattrevil[4]);
}

void moveball() {
	int elapsedtime = lastmillis - footballmillis;
	ball.x = ball.x + (ballvel.x * elapsedtime);
	ball.y = ball.y + (ballvel.y * elapsedtime);
	ball.z = ball.z + (ballvel.z * elapsedtime);
	sendball();
}

void startfootball() {
	startmatch(_football==1 ? 1 : 11, footballmap);
	ball = ballspawn;
	ballvel = vec(0, 0, 0);
	lasthit = -1;
	footballmillis = lastmillis;
	sendball();
}
void endfootball() { // delete ball
	clientinfo *ci;
	if(firstbar() > -1) ci = (clientinfo *)getclientinfo(firstbar());
	else if(clients[0]) ci = clients[0];
	if(!ci) return;
	flushserver(true);
	uchar buf[MAXTRANS];
	ucharbuf b(buf, sizeof(buf));
	putint(b, N_EDITENT);
	putint(b, ballidx);
	putint(b, 0); putint(b, 0); putint(b, 0);
	putint(b, 0);
	putint(b, 0); putint(b, 0); putint(b, 0); putint(b, 0); putint(b, 0);
	packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
	putint(p, N_CLIENT);
	putint(p, ci->clientnum);
	putint(p, b.len);
	p.put(buf, b.len);
	sendpacket(-1, 1, p.finalize());
}
void updatefootball() {
	if(_football) {
		// A PLAYER HITS THE BALL
		loopv(clients) {
			if(clients[i]->state.state!=CS_ALIVE) continue;
			if(clients[i]->state.o.dist(ball) <= ballplayerdist) {
				lasthit = clients[i]->clientnum;
				int yaw = clients[i]->yaw;
				int pitch = clients[i]->pitch;
				ballvel.x = sin(yaw/(180/PI)) * -1 / (clients[i]->state.gunselect!=GUN_PISTOL ? ballshootdiv : ballcontroldiv) * cos(abs(pitch)/(180/PI));
				ballvel.y = cos(yaw/(180/PI)) / (clients[i]->state.gunselect!=GUN_PISTOL ? ballshootdiv : ballcontroldiv) * cos(abs(pitch)/(180/PI));
				ballvel.z = (sin((pitch >= 0 ? pitch : (pitch/2.f))/(180/PI))) / (clients[i]->state.gunselect!=GUN_PISTOL ? ballshootdiv : ballcontroldiv);
				break;
			}
		}
		if(ballvel.z && ball.z < (areamin.z + 8)) { // FLOOR
			clientinfo *ci = (clientinfo *)getclientinfo(lasthit);
			if(!ci || ci->state.o.dist(ball) > ballplayerdist || ci->state.pitch <= 0)
			if(ballvel.z < 0) ballvel.z *= -ballfloorbounce;
		} else
		if(ball.z > (areamax.z - 8)) { // CEILING
			if(ballvel.z > 0) ballvel.z *= -ballceilingbounce;
		}
		else { // FALLING
			ballvel.z -= ballgravity * (lastmillis - footballmillis);
		}
		if(ball.x > (areamax.x - 8)) { // WALL RIGHT
			if(ball.z > ballout) {
				ball = ballspawn;
				ballvel = vec(0, 0, 0);
				sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Football\f1]\f7 ball \f3reset\f7!");
			} else
			if(ballvel.x > 0) {ballvel.x *= -ballwallbounce; ballvel.y *= ballwallbounce; ballvel.z *= ballwallbounce;}
		} else
		if(ball.x < (areamin.x + 8)) { // WALL LEFT
			if(ball.z > ballout) {
				ball = ballspawn;
				ballvel = vec(0, 0, 0);
				sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Football\f1]\f7 ball \f3reset\f7!");
			} else
			if(ballvel.x < 0) {ballvel.x *= -ballwallbounce; ballvel.y *= ballwallbounce; ballvel.z *= ballwallbounce;}
		} else
		if(ball.y > (areamax.y - 8)) { // WALL FORWARDS
			if(ball.z > ballout) {
				ball = ballspawn;
				ballvel = vec(0, 0, 0);
				sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Football\f1]\f7 ball \f3reset\f7!");
			} else
			if(ballvel.y > 0) {ballvel.y *= -ballwallbounce; ballvel.x *= ballwallbounce; ballvel.z *= ballwallbounce;}
		} else
		if(ball.y < (areamin.y + 8)) { // WALL BACKWARDS
			if(ball.z > ballout) {
				ball = ballspawn;
				ballvel = vec(0, 0, 0);
				sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Football\f1]\f7 ball \f3reset\f7!");
			} else
			if(ballvel.y < 0) {ballvel.y *= -ballwallbounce; ballvel.x *= ballwallbounce; ballvel.z *= ballwallbounce;}
		}
		if((ball.x < (goalgoodmax.x + 4) && ball.x > (goalgoodmin.x - 4) && ball.y < (goalgoodmax.y + 4) && ball.y > (goalgoodmin.y - 4) && ball.z > (goalgoodmax.z + 4) && ball.z < (goalgoodmax.z + 12)) || (ball.x < (goalgoodmax.x + 4) && ball.x > (goalgoodmin.x - 4) && ball.y < (goalgoodmax.y + 4) && ball.y > (goalgoodmin.y - 4) && ball.z > (goalgoodmax.z + 4) && ball.z < (goalgoodmax.z + 12))) {
			ball = ballspawn;
			ballvel = vec(0, 0, 0);
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Football\f1]\f7 ball \f3reset\f7!");
		}
		loopv(bouncers) {
			bouncer b = bouncers[i];
			if((ballvel.x > 0 && (ball.x > (b.min.x - 4) && ball.x < (b.min.x + 4)) && (ball.y > (b.min.y - 4) && ball.y < (b.max.y + 4)) && (ball.z > (b.min.z - 4) && ball.z < (b.max.z + 4))) || (ballvel.x < 0 && (ball.x < (b.max.x + 4) && ball.x > (b.max.x - 4)) && (ball.y > (b.min.y - 4) && ball.y < (b.max.y + 4)) && (ball.z > (b.min.z - 4) && ball.z < (b.max.z + 4)))) {
				ballvel.x *= -ballwallbounce;
				ballvel.y *= ballwallbounce;
				ballvel.z *= ballwallbounce;
			} else if((ballvel.y > 0 && (ball.y > (b.min.y - 4) && ball.y < (b.min.y + 4)) && (ball.x > (b.min.x - 4) && ball.x < (b.max.x + 4)) && (ball.z > (b.min.z - 4) && ball.z < (b.max.z + 4))) || (ballvel.y < 0 && (ball.y < (b.max.y + 4) && ball.y > (b.max.y - 4)) && (ball.x > (b.min.x - 4) && ball.x < (b.max.x + 4)) && (ball.z > (b.min.z - 4) && ball.z < (b.max.z + 4)))) {
				ballvel.x *= ballwallbounce;
				ballvel.y *= -ballwallbounce;
				ballvel.z *= ballwallbounce;
			} else if((ballvel.z > 0 && (ball.z > (b.min.z - 4) && ball.z < (b.min.z + 4)) && (ball.y > (b.min.y - 4) && ball.y < (b.max.y + 4)) && (ball.x > (b.min.x - 4) && ball.x < (b.max.x + 4))) || (ballvel.z < 0 && (ball.z < (b.max.z + 4) && ball.z > (b.max.z - 4)) && (ball.y > (b.min.y - 4) && ball.y < (b.max.y + 4)) && (ball.x > (b.min.x - 4) && ball.x < (b.max.x + 4)))) {
				ballvel.x *= ballwallbounce;
				ballvel.y *= ballwallbounce;
				ballvel.z *= -ballwallbounce;
			}
		}
		// FRICTION
		float balltotalvxy = sqrt(ballvel.x * ballvel.x + ballvel.y * ballvel.y);
		float friction = ballfriction; if(ball.z > (areamin.z + 8)) friction = 0;
		float balldivx = ballvel.x / balltotalvxy;
		float balldivy = ballvel.y / balltotalvxy;
		float ballfinalvxy = max(0.f, (balltotalvxy - friction * (lastmillis - footballmillis)));
		if(balltotalvxy) {
			if(ballvel.x) ballvel.x = ballfinalvxy * balldivx;
			if(ballvel.y) ballvel.y = ballfinalvxy * balldivy;
		}
		// AIR RESISTANCE
		float C = ballairresistance;
		float _balltotalvxy = sqrt(ballvel.x * ballvel.x + ballvel.y * ballvel.y);
		float _balltotalvxyz = sqrt(_balltotalvxy * _balltotalvxy + ballvel.z * ballvel.z);
		float _balldivx = ballvel.x / _balltotalvxyz;
		float _balldivy = ballvel.y / _balltotalvxyz;
		float _balldivz = ballvel.z / _balltotalvxyz;
		if(_balltotalvxyz) {
			float airres = C * (_balltotalvxy * _balltotalvxy + ballvel.z * ballvel.z);
			_balltotalvxyz -= airres * (lastmillis - footballmillis);
			ballvel.x = _balltotalvxyz * _balldivx;
			ballvel.y = _balltotalvxyz * _balldivy;
			ballvel.z = _balltotalvxyz * _balldivz;
		}

		if(ball.x > areamax.x || ball.x < areamin.x || ball.y > areamax.y || ball.y < areamin.y || ball.z > areamax.z) {
			ball = ballspawn;
			ballvel = vec(0, 0, 0);
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Football\f1]\f7 ball \f3reset\f7!");
		}
		if(ball.z < (areamin.z)) ball.z = (areamin.z + 8);
		// GOAL
		clientinfo *ci = (clientinfo *)getclientinfo(lasthit);
		if(ci && ball.x < (goalgoodmax.x + 4) && ball.x > (goalgoodmin.x - 4) && ball.y < (goalgoodmax.y + 4) && ball.y > (goalgoodmin.y - 4) && ball.z < (goalgoodmax.z + 4) && ball.z > (goalgoodmin.z - 4)) {
			if(m_teammode) {
				// GOAL FOR GOOD
				score[0]++;
				defformatstring(GOALSTR)("\f1[\f7Football\f1]\f7 %soal\f7 for \f1GOOD\f7 by \f6%s\f7 [\f1GOOD\f7 %d - \f3EVIL\f7 %d]", isteam(ci->team, "good") ? "\f6g" : "\f3own g", colorname(ci), score[0], score[1]);
				sendf(-1, 1, "ris", N_SERVMSG, GOALSTR);
			}
			ballvel = vec(0, 0, 0);
			ball = ballspawn;
			}
		if(ci && ball.x < goalevilmax.x && ball.x > goalevilmin.x && ball.y < goalevilmax.y && ball.y > goalevilmin.y && ball.z < goalevilmax.z && ball.z > goalevilmin.z) {
			if(m_teammode) {
				// GOAL FOR EVIL
				score[1]++;
				defformatstring(GOALSTR)("\f1[\f7Football\f1]\f7 %soal\f7 for \f3EVIL\f7 by \f6%s\f7 [\f1GOOD\f7 %d - \f3EVIL\f7 %d]", isteam(ci->team, "evil") ? "\f6g" : "\f3own g", colorname(ci), score[0], score[1]);
				sendf(-1, 1, "ris", N_SERVMSG, GOALSTR);
			}
			ballvel = vec(0, 0, 0);
			ball = ballspawn;
		}
		moveball();
		footballmillis = lastmillis;
	}
}
