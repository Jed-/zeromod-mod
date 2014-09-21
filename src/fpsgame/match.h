void startmatch(int mode, const char *map) {
	changemap(map, mode);
	_match = true;
	_matchstart = 0;
	_matchmsg = 0;
	updatematchclients();
	pausegame(true);
	defformatstring(msg)("\f1[\f7Match\f1]\f7 waiting for all the players to load the map%s...", _wpmode ? " and choose \f32\f7 weapons with \f3#setwp weapon1 weapon2\f7" : "");
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
}
void checkmatch() {
	if(!_loaded_map) checkloadmap();
	if(!_loaded_map) return;
	if(!_matchstart) _matchstart = totalmillis;
	if((totalmillis - _matchstart) <= 5000) {
		switch((totalmillis - _matchstart) / 1000) {
			case 0: {
				if(_matchmsg==0) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Match\f1]\f7 game starts in \f05\f7 seconds");
					_matchmsg++;
				}
				break;
			}
			case 1: {
				if(_matchmsg==1) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Match\f1]\f7 game starts in \f04\f7 seconds");
					_matchmsg++;
				}
				break;
			}
			case 2: {
				if(_matchmsg==2) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Match\f1]\f7 game starts in \f03\f7 seconds");
					_matchmsg++;
				}
				break;
			}
			case 3: {
				if(_matchmsg==3) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Match\f1]\f7 game starts in \f02\f7 seconds");
					_matchmsg++;
				}
				break;
			}
			case 4: {
				if(_matchmsg==4) {
					sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Match\f1]\f7 game starts in \f01\f7 second");
					_matchmsg++;
				}
				break;
			}
		}
	} else {
		if(_matchmsg==5) {
			pausegame(false);
			sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Match\f1]\f7 game started!");
			_matchmsg++;
			updatematchclients();
		}
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
				defformatstring(msg)("\f1[\f7Match\f1]\f7 game paused: client \f0%d\f7 left!", _matchclients[i]);
				sendf(-1, 1, "ris", N_SERVMSG, msg);
				break;
			}
		}
		updatematchclients();
	}
}
void updatematchclients() {
	_matchclients.shrink(0);
	loopv(clients) {
		if(clients[i]->state.state!=CS_SPECTATOR && clients[i]->state.aitype==AI_NONE) _matchclients.add(clients[i]->clientnum);
	}
}
