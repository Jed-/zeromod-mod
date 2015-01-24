void doresume(int secs) {
	if(_resuming) {
		_resuming = false;
		sendf(-1, 1, "ris", N_SERVMSG, "\f1[\f7Resume\f1]\f7 game is \f3paused\f7!");
		return;
	}
	if(/*_resuming || */!gamepaused || (_arena && _startarena && _roundstart && (totalmillis - _roundstart) <= 5000) || (_match && _matchstart && (totalmillis - _matchstart) <= 5000)) return;
	secs = clamp(secs, 0, 10);
	_resuming = true;
	_startresume = totalmillis;
	_resumemillis = _startresume+secs*1000;
	_resumemsg = secs;
}
void checkresume() {
	if(!gamepaused || !_resuming || !_startresume || !_resumemillis || _resumemillis < _startresume) return;
	if(_resumemillis <= totalmillis) {
		pausegame(false);
		_resuming = false;
		return;
	}
	int rem = (_resumemillis - totalmillis) / 1000;
	if(_resumemsg > rem) {
		_resumemsg--;
		defformatstring(msg)("\f1[\f7Resume\f1]\f7 resuming in \f1%d\f7 sec%s", rem+1, rem==0 ? "" : "s");
		sendf(-1, 1, "ris", N_SERVMSG, msg);
	}
}
