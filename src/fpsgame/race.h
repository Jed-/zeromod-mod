struct racemap {
	string name;
	int mintime, besttime;
	vec win;
	string bestname;
	racemap() : mintime(0), besttime(0), win(0, 0, 0) {copystring(bestname, "[no best]", MAXNAMELEN+1);}
};
vector<racemap *> racemaps;
void loadrace(const char *name, int mintime, int x, int y, int z, const char *bestname = "", int besttime = 0) {
	int _exists = -1;
	loopv(racemaps) if(!strcmp(racemaps[i]->name, name)) _exists = i;
	if(_exists > -1) {
		racemap *r = racemaps[_exists];
		r->mintime = mintime;
		if(besttime && besttime>=mintime) r->besttime = besttime; else r->besttime = 0;
		if(bestname && bestname[0]) copystring(r->bestname, bestname, MAXNAMELEN+1);
		else copystring(r->bestname, "[no best]", MAXNAMELEN+1);
		r->win = vec(x, y, z);
	} else {
		racemap *r = new racemap();
		copystring(r->name, name);
		r->mintime = max(0, mintime);
		r->win = vec(x, y, z);
		if(bestname && bestname[0]) copystring(r->bestname, bestname, MAXNAMELEN+1);
		if(besttime && besttime>=mintime) r->besttime = besttime;
		racemaps.add(r);
	}
}
ICOMMAND(loadrace, "siiiisi", (const char *name, int *mintime, int *x, int *y, int *z, const char *bestname, int *besttime), loadrace(name, *mintime, *x, *y, *z, bestname, *besttime));
void _storeraces() {
	stream *f = openutf8file(path("racemaps.cfg", true), "w");
	if(f) {
		f->printf("// List of the default racemaps");
		loopv(racemaps) {
			f->printf("\nloadrace %s %d %d %d %d \"%s\" %d", racemaps[i]->name, racemaps[i]->mintime, (int)racemaps[i]->win.x, (int)racemaps[i]->win.y, (int)racemaps[i]->win.z, racemaps[i]->bestname, racemaps[i]->besttime);
		}
		delete f;
	}
}
bool loaded() {
	loopv(clients) if(clients[i]->state.state!=CS_SPECTATOR && (clients[i]->clientmap[0] == '\0' || !clients[i]->clientmap[0])) return false;
	return true;
}
void sendrace(char *map) {
	defformatstring(path)("races/%s.ogz", map);
	if(mapdata) DELETEP(mapdata);
	mapdata = openrawfile(path, "r+b");
	loopv(clients) clients[i]->getmap = sendfile(clients[i]->clientnum, 2, mapdata, "ri", N_SENDMAP);
}
void sendracecn(int cn) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE) return;
	defformatstring(path)("races/%s.ogz", smapname);
	if(mapdata) DELETEP(mapdata);
	mapdata = openrawfile(path, "r+b");
	sendfile(cn, 2, mapdata, "ri", N_SENDMAP);
}
void startracemap(char *map) {
	_racemode = true;
	_racewon = false;
	_racemsg = 0;
	_racewonmsg = 0;
	_racestart = 0;
	_raceend = 0;
	_raceloaded = false;
	changemap(map, 1);
	pausegame(true);
	loopv(clients) clients[i]->clientmap[0] = '\0';
	sendrace(map);
}
void startrace() {
	_racemode = true;
	_racewon = false;
	_racemsg = 0;
	_racewonmsg = 0;
	_racestart = 0;
	_raceend = 0;
	_raceloaded = false;
	_raceidx = 0;
	if(!racemaps.inrange(0)) {
		sendf(-1, 1, "ris", N_SERVMSG, "\f3[\f7Error\f3]\f7 could not start race mode: no race maps specified!");
		_racemode = false;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 racemode \f4off");
		return;
	}
	startracemap(racemaps[0]->name);
}
racemap *getracemap(char *name) {
	loopv(racemaps) if(!strcmp(racemaps[i]->name, name)) return racemaps[i];
	return NULL;
}
void checkrace() {
	if(!_racemode) return;
	if(!getracemap(smapname)) return;
	_raceloaded = loaded();
	if(!_raceloaded) return;
	if(!_racestart) _racestart = totalmillis;
	int time = totalmillis - _racestart;
	if(_racemsg==0) {
		_racemsg++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race starts in \f010\f7...");
	} else if(_racemsg==1 && time >= 5000) {
		_racemsg++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race starts in \f05\f7...");
	} else if(_racemsg==2 && time >= 6000) {
		_racemsg++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race starts in \f04\f7...");
	} else if(_racemsg==3 && time >= 7000) {
		_racemsg++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race starts in \f03\f7...");
	} else if(_racemsg==4 && time >= 8000) {
		_racemsg++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race starts in \f02\f7...");
	} else if(_racemsg==5 && time >= 9000) {
		_racemsg++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race starts in \f01\f7...");
	} else if(_racemsg==6 && time >= 10000) {
		_racemsg++;
		pausegame(false);
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 race \f0started\f7!");
	}
	loopv(clients) {
		if(clients[i]->state.state==CS_SPECTATOR) continue;
		if(clients[i]->state.state==CS_EDITING) {
			_forcespeccn(clients[i]->clientnum, 1);
			defformatstring(msg)("\f3[\f7Warning\f3]\f6 %s\f7 has been \f3caugth\f7 in \f0editmode\f7 and has been \f3spectated\f7 for this race!", colorname(clients[i]));
			sendf(-1, 1, "ris", N_SERVMSG, msg);
			continue;
		}
		if(clients[i]->state.state==CS_ALIVE && !gamepaused) {
			racemap *r = getracemap(smapname);
			if(clients[i]->state.o.dist(r->win) <= 32) {
				// race won?
				int _racetime = totalmillis - _racestart;
				_raceend = totalmillis;
				if((_racetime-10000) < r->mintime) {
					kickclients(getclientip(clients[i]->clientnum));
					defformatstring(msg)("\f3[\f7Kick\f3]\f0 server\f3 kicked \f6%s\f7 because: \f3cheating in a race", colorname(clients[i]));
					sendf(-1, 1, "ris", N_SERVMSG, msg);
					continue;
				}
				if((_racetime-10000) >= r->mintime && !_racewon) {
					_racewon = 1;
					if(_racetime-10000 < r->besttime || !r->besttime || !strcmp(r->bestname, "[no best]")) {
						if(!strcmp(r->bestname, "[no best]") && !r->besttime) {
							defformatstring(msg)("\f1[\f7Race\f1]\f0 %s\f6 has won\f7 the race in \f0%.3f\f7 seconds \f0(\f7new best\f0)\f7!", colorname(clients[i]), ((float)_racetime-10000.f)/1000.f);
							sendf(-1, 1, "ris", N_SERVMSG, msg);
						} else {
							defformatstring(msg)("\f1[\f7Race\f1]\f0 %s\f6 has won\f7 the race in \f0%.3f\f7 seconds \f0(\f7new best - old best: %s %.3fs\f0)\f7!", colorname(clients[i]), ((float)_racetime-10000.f)/1000.f, r->bestname, r->besttime/1000.f);
							sendf(-1, 1, "ris", N_SERVMSG, msg);
						}
						r->besttime = _racetime-10000;
						copystring(r->bestname, clients[i]->name, MAXNAMELEN+1);
					} else {
						defformatstring(msg)("\f1[\f7Race\f1]\f0 %s\f6 has won\f7 the race in \f6%.3f\f7 seconds \f0(\f7best: %s %.3f\f0)\f7!", colorname(clients[i]), ((float)_racetime-10000.f)/1000.f, r->bestname, r->besttime/1000.f);
						sendf(-1, 1, "ris", N_SERVMSG, msg);
					}
				}
			}
		}
	}
	if(_racewon) {
		int _time_ = totalmillis - _raceend;
		if(_racemsg==7) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race in \f010\f7...");
		} else if(_racemsg==8 && _time_ >= 5000) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race in \f05\f7...");
		} else if(_racemsg==9 && _time_ >= 6000) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race in \f04\f7...");
		} else if(_racemsg==10 && _time_ >= 7000) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race in \f03\f7...");
		} else if(_racemsg==11 && _time_ >= 8000) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race in \f02\f7...");
		} else if(_racemsg==12 && _time_ >= 9000) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race in \f01\f7...");
		} else if(_racemsg==13 && _time_ >= 10000) {
			_racemsg++;
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Race\f1]\f7 starting a new race...");
			_raceidx++;
			_raceidx = _raceidx % racemaps.length();
			startracemap(racemaps[_raceidx]->name);
		}
	}
}
