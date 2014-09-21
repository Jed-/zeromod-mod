void startarena(const char *map, int mode) {
	changemap(map, mode);
	_arena = true;
	_rounds = 0;
	_startarena = 0;
	_arenamsg = 0;
	_roundsgood = 0;
	_roundsevil = 0;
	_scored = 0;
	defformatstring(msg)("\f1[\f7Arena\f1]\f7 waiting for all the players to load the map%s...", _wpmode ? " and choose \f32\f7 weapons with \f3#setwp weapon1 weapon2\f7" : "");
	sendf(-1, 1, "ris", N_SERVMSG, msg);
	if(_wpmode) {
		loopv(clients) {
			if(clients[i]->state.state!=CS_SPECTATOR) {
				const char *_weapons[7] = {"chainsaw", "shotgun", "chaingun", "rocket launcher", "rifle", "grenade launcher", "pistol"};
            	int _w1 = rnd(5)+1;
            	int _w2 = 0;
            	do _w2 = rnd(5)+1; while(_w2 == _w1);
            	const char *wpcodes[7] = {"cs", "sg", "cg", "rl", "ri", "gl", "pi"};
            	defformatstring(_msg)("\f1[\f7Wp\f1]\f7 do \f3#setwp %s %s\f7 to get \f3%s\f7 and \f3%s\f7!", wpcodes[_w1], wpcodes[_w2], _weapons[_w1], _weapons[_w2]);
            	sendf(clients[i]->clientnum, 1, "ris", N_SERVMSG, _msg);
			}
		}
	}
	_timefunc("time", "30", NULL);
}
void checkstartround() {
	if(!_loaded_map) {
		if(!gamepaused) pausegame(true);
		checkloadmap();
	}
	if(!_loaded_map) return;
	if(!_startarena) {
		_startarena = totalmillis;
		startround();
	}
}
void _respawn() {
	loopv(clients) {
		if(clients[i]->state.state==CS_SPECTATOR) continue;
		clientinfo *ci = clients[i];
		clients[i]->state.respawn();
		clients[i]->state.state=CS_ALIVE;
		sendspawn(ci);
	}
}
void startround() {
	if(_rounds >= 10 || _roundsgood >= 6 || _roundsevil >= 6) {
		startintermission();
		defformatstring(msg)("\f1[\f7Arena\f1]\f7 match finished \f%d%d\f7 - \f%d%d\f7%s!", (_roundsgood==_roundsevil) ? 6 : (_roundsgood>_roundsevil) ? 0 : 3, _roundsgood, (_roundsgood==_roundsevil) ? 6 : (_roundsgood>_roundsevil) ? 3 : 0, _roundsevil,
		(_roundsgood==_roundsevil) ? "" : (_roundsgood>_roundsevil) ? " for team \f1good" : " for team \f3evil");
		sendf(-1, 1, "ris", N_SERVMSG, msg);
	} else {
		_respawn();
		_rounds++;
		_roundstart = totalmillis;
		_arenamsg = 0;
		_scored = 0;
		pausegame(true);
		defformatstring(msg)("\f1[\f7Arena\f1]\f7 starting round \f0%d", _rounds);
		sendf(-1, 1, "ris", N_SERVMSG, msg);
	}
}
void checkround() {
	if(!_startarena || !_roundstart) return;
	if((totalmillis - _roundstart) <= 5000) {
		switch((totalmillis - _roundstart) / 1000) {
			case 0: {
				if(_arenamsg == 0) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Arena\f1]\f7 round starts in \f05\f7 seconds");
					_arenamsg++;
				}
				break;
			}
			case 1: {
				if(_arenamsg == 1) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Arena\f1]\f7 round starts in \f04\f7 seconds");
					_arenamsg++;
				}
				break;
			}
			case 2: {
				if(_arenamsg == 2) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Arena\f1]\f7 round starts in \f03\f7 seconds");
					_arenamsg++;
				}
				break;
			}
			case 3: {
				if(_arenamsg == 3) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Arena\f1]\f7 round starts in \f02\f7 seconds");
					_arenamsg++;
				}
				break;
			}
			case 4: {
				if(_arenamsg == 4) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Arena\f1]\f7 round starts in \f01\f7 second");
					_arenamsg++;
				}
				break;
			}
		}
	} else {
		if(_arenamsg == 5) {
			pausegame(false);
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Arena\f1]\f7 round \f0started\f7 - kill all the enemies to win the round!");
			_arenamsg++;
			updatematchclients();
		}
		_arenamsg = -1;
		// let's check if everybody is died in a team
		int _num_good = 0;
		int _num_evil = 0;
		int _dead_good = 0;
		int _dead_evil = 0;
		int _n = 0;
		loopv(clients) _n++;
		bool hasmaster = false;
		loopv(clients) {
			if(clients[i]->privilege) hasmaster = true;
			break;
		}
		if(!gamepaused && mastermode>= 2 && hasmaster) loopv(_matchclients) {
			clientinfo *_ci = getinfo(_matchclients[i]);
			if(!_ci || _ci->state.state==CS_SPECTATOR) {
				pausegame(true);
				defformatstring(msg)("\f1[\f7Arena\f1]\f7 game paused: client \f0%d\f7 left!", _matchclients[i]);
				sendf(-1, 1, "ris", N_SERVMSG, msg);
				break;
			}
		}
		updatematchclients();
		loopv(clients) {
			clientinfo *ci = clients[i];
			if(ci->clientnum< 0 || ci->clientnum >= 256) continue;
			if(ci->state.state==CS_SPECTATOR) continue;
			if(isteam(ci->team, "good")) {
				_num_good++;
				if(ci->state.state==CS_DEAD) _dead_good++;
			} else
			if(isteam(ci->team, "evil")) {
				_num_evil++;
				if(ci->state.state==CS_DEAD) _dead_evil++;
			}
		}
		if((_dead_good == _num_good && _num_good) || _scored==2) {
			// all the goods died, score for evil
			int _f = serverflagruns;
			serverflagruns = 0;
			if(_scored!=2) loopv(clients) {
				if(isteam(clients[i]->team, "evil") && clients[i]->state.state==CS_ALIVE) {
					ctfmode.scoreflag(clients[i], 0);
					break;
				}
			}
			serverflagruns = _f;
			_roundsevil++;
			loopv(ctfmode.flags) ctfmode.dropflag(i, vec(0, 0, 0), 11000);
			defformatstring(msg)("\f1[\f7Arena\f1]\f7 team \f3evil\f7 won round \f0%d\f7 (\f1%d\f7 - \f3%d\f7)!", _rounds, _roundsgood, _roundsevil);
			sendf(-1, 1, "ris", N_SERVMSG, msg);
			startround();
		} else if((_dead_evil == _num_evil && _num_evil) || _scored==1) {
			// all the evils died, score for good
			int _f = serverflagruns;
			serverflagruns = 0;
			if(_scored!=1) loopv(clients) {
				if(isteam(clients[i]->team, "good") && clients[i]->state.state==CS_ALIVE) {
					ctfmode.scoreflag(clients[i], 0);
					break;
				}
			}
			serverflagruns = _f;
			_roundsgood++;
			loopv(ctfmode.flags) loopv(ctfmode.flags) ctfmode.dropflag(i, vec(0, 0, 0), 11000);
			defformatstring(msg)("\f1[\f7Arena\f1]\f7 team \f1good\f7 won round \f0%d\f7 (\f1%d\f7 - \f3%d\f7)!", _rounds, _roundsgood, _roundsevil);
			sendf(-1, 1, "ris", N_SERVMSG, msg);
			startround();
		}
	}
}
void checkarena() {
	checkstartround();
	checkround();
	return;
}
