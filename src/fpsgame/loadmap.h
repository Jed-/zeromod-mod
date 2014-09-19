void checkloadmap() {
	if(_loaded_map && (!_wpmode || _wpchosen)) return;
	bool l = true;
	bool w = true;
	loopv(clients) {
		if(clients[i]->state.state==CS_SPECTATOR || clients[i]->state.aitype!=AI_NONE) continue;
		if(!clients[i]->loaded) {
			l = false;
			break;
		}
		if(_wpmode) {
			if(!clients[i]->wpchosen) {
				l = false;
				w = false;
				break;
			}
		}
	}
	_loaded_map = l;
	_wpchosen = w;
}
