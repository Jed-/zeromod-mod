/*	static bool _resuming = false;
	static int _startresume = 0;
	static int _resumemillis = 0; */
void doresume(int secs) {
	if(_resuming || !gamepaused) return;
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
		defformatstring(msg)("\f1[\f7Resume\f1]\f7 resuming in \f1%ds", rem+1);
		sendf(-1, 1, "ris", N_SERVMSG, msg);
	}
}
