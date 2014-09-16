struct histr {
	string str;
	histr() {};
};
struct drink {
	string name;
	drink() {};
};
vector<histr> histrings;
vector<drink> drinks;
SVAR(drinkcmd, "gimme");
int _strcasestr(char *a, char *b);
void parsebar(const char *m, int cn) {
	if(!histrings.inrange(0)) return;
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype==AI_BOT) return;
	int botcn = -1;
	loopv(bots) {
		if(bots[i] && bots[i]->state.state==CS_SPECTATOR) {
			botcn = bots[i]->clientnum;
			break;
		}
	}
	if(botcn==-1) return;
	clientinfo *b = getinfo(botcn);
	if(!b) return;
	defformatstring(bname)("%s", b->name);
	for(int j=0; bname[j]; j++) bname[j] = tolower(bname[j]);
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
	
	if(!drinks.inrange(0)) return;
	int idx = _strcasestr((char *)m, bname);
	if(idx <= -1) return;
	string barcmd;
	copystring(barcmd, (char *)&m[idx]);
	char *argv[260];
	int argc = _argsep(barcmd, 260, argv);
	if(argc < 2) return;
	if(strcasecmp(argv[0], bname) || strcasecmp(argv[1], drinkcmd)) return;
	char list[MAXTRANS];
	list[0] = '\0';
	loopv(drinks) {
		concatstring(list, drinks[i].name);
		if(i < (drinks.length()-1)) concatstring(list, " ");
	}
	if(argc < 3 || !strcasecmp(argv[2], "list") || !strcasecmp(argv[2], "menu")) {
		defformatstring(_cmd)("barlist %d %d \"%s\"", botcn, cn, list);
		if(identexists("barlist")) execute(_cmd);
		return;
	}
	bool listed = false;
	loopv(drinks) {
		if(!strcasecmp(drinks[i].name, argv[2])) {
			listed = true;
			break;
		}
	}
	if(!listed) {
		defformatstring(_cmd)("barunlisted %d %d %s", botcn, cn, argv[2]);
		if(identexists("barunlisted")) execute(_cmd);
		return;
	}
	defformatstring(_cmd)("bardrink %d %d %s", botcn, cn, argv[2]);
	if(identexists("bardrink")) execute(_cmd);
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
void adddrink(char *name) {
	drink d;
	copystring(d.name, name);
	drinks.add(d);
}
ICOMMAND(adddrink, "s", (char *name), adddrink(name));
void resetdrinks() {
	drinks.shrink(0);
}
ICOMMAND(resetdrinks, "", (), resetdrinks());
