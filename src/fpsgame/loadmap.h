void checkloadmap() {
	if(_loaded_map) return;
	bool l = true;
	loopv(clients) {
		if(clients[i]->state.state==CS_SPECTATOR || clients[i]->state.aitype!=AI_NONE) continue;
		if(!clients[i]->loaded) {
			l = false;
			break;
		}
	}
	_loaded_map = l;
}
