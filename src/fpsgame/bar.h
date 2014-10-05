struct histr {
	string str;
	histr() {};
};
struct drink {
	string name;
	string name2;
	drink() {};
};
vector<histr> histrings;
vector<drink> drinks;
SVAR(drinkcmd, "give");
int _strcasestr(char *a, char *b);
void offer(int whocn, int tocn, char *what = (char*)"beer", int howmuch = 1) {
	if(!howmuch) return;
	clientinfo *who = getinfo(whocn);
	if(!who) return;
	clientinfo *to = getinfo(tocn);
	if(!to) return;
	defformatstring(args)("%d %d %s", to->clientnum, howmuch, what);
	_beerfunc("beer", args, who);
}
ICOMMAND(offer, "iisi", (int *whocn, int *tocn, char *what, int *howmuch), offer(*whocn, *tocn, what, *howmuch));
void parsebar(const char *m, int cn) {
//	if(!histrings.inrange(0)) return;
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype==AI_BOT) return;
	int botcn = -1;
	loopv(bots) {
		if(bots[i] && bots[i]->state.state==CS_SPECTATOR) {
			botcn = bots[i]->clientnum;
			if(botcn < 0 || botcn > 255) continue;
			clientinfo *b = getinfo(botcn);
			if(!b) continue;
			defformatstring(bname)("%s", b->name);
			for(int j=0; bname[j]; j++) bname[j] = tolower(bname[j]);
			if(histrings.inrange(0)) {
				bool ishi = false;
				loopv(histrings) {
					defformatstring(str1)("%s %s", histrings[i].str, bname);
					defformatstring(str2)("%s, %s", histrings[i].str, bname);
					if(strcasestr(m, str1) || strcasestr(m, str2)) {
						ishi = true;
						break;
					}
				}
				defformatstring(cmd)("barhi %d %d", botcn, cn);
				if(ishi && identexists("barhi")) execute(cmd);
			}
	
//			if(!drinks.inrange(0)) return;
			if(drinks.inrange(0)) {
				int idx = _strcasestr((char *)m, bname);
				if(idx <= -1) continue;
				string barcmd; // name command who how_many what
				copystring(barcmd, (char *)&m[idx]);
				char *argv[260];
				int argc = _argsep(barcmd, 260, argv);
				if(argc < 2) continue;
				if(strcasecmp(argv[0], bname)) continue;
				char list[MAXTRANS];
				list[0] = '\0';
				loopv(drinks) {
					concatstring(list, drinks[i].name);
					if(i < (drinks.length()-1)) concatstring(list, " ");
				}
				if(!strcasecmp(argv[1], "list") || !strcasecmp(argv[1], "menu")) { // name menu
					defformatstring(_cmd)("barlist %d %d \"%s\"", botcn, cn, list);
					if(identexists("barlist")) execute(_cmd);
					continue;
				}
				if(argc < 5) continue;
				if(strcasecmp(argv[1], drinkcmd)) continue;
				int tocn = -1;
				if(!strcasecmp(argv[2], "me")) tocn = cn; // 1: 'me'
				else { // 2: nick
					loopv(clients) {
						if(!strcasecmp(argv[2], clients[i]->name)) {
							tocn = clients[i]->clientnum;
							break;
						}
					}
					if(tocn <= -1) { // 3: nick containing string
						loopv(clients) {
							if(_strcasestr(clients[i]->name, argv[2]) >= 0) {
								tocn = clients[i]->clientnum;
								break;
							}
						}
					}
					if(tocn <= -1) { // 4: direct cn
						tocn = atoi(argv[2]);
						if(atoi(argv[2])==0 && strcmp(argv[2], "0")) tocn = -1;
					}
				}
				if(!getinfo(tocn) && identexists("barwho")) {
					defformatstring(_cmd)("barwho %d %d %s", botcn, cn, argv[2]);
					execute(_cmd);
					continue;
				}
				int num = -1;
				if(!strcasecmp(argv[3], "a") || !strcasecmp(argv[3], "an")) num = 1;
				else if(atoi(argv[3]) > 0) num = atoi(argv[3]);
				if(num <= 0 && identexists("barnum")) {
					defformatstring(_cmd)("barnum %d %d", botcn, cn);
					execute(_cmd);
					continue;
				}
				int didx = -1;
				loopv(drinks) {
					if(!strcasecmp(drinks[i].name, argv[4]) || !strcasecmp(drinks[i].name2, argv[4])) {
						didx = i;
						break;
					}
				}
				if(didx <= -1) {
					defformatstring(_cmd)("barunlisted %d %d %s", botcn, cn, argv[4]);
					if(identexists("barunlisted")) execute(_cmd);
					continue;
				}
				if(identexists("bardrink")) {
					defformatstring(_cmd)("bardrink %d %d %s %d", botcn, tocn, num==1 ? drinks[didx].name : drinks[didx].name2, num);
					execute(_cmd);
				}
			}
		}
	}
}
void histring(char *s) {
	histr h;
	copystring(h.str, s);
	histrings.add(h);
}
ICOMMAND(histring, "s", (char *s), histring(s));
void resethistrings() {
	histrings.shrink(0);
}
ICOMMAND(resethistrings, "", (), resethistrings());
void adddrink(char *name, char *name2) {
	drink d;
	copystring(d.name, name);
	copystring(d.name2, name2);
	drinks.add(d);
}
ICOMMAND(adddrink, "ss", (char *name, char *name2), adddrink(name, name2));
void resetdrinks() {
	drinks.shrink(0);
}
ICOMMAND(resetdrinks, "", (), resetdrinks());
