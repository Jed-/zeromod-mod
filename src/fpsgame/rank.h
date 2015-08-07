struct playerscore {
	string name;
	uint range; // /16
	int frags, flags, deaths, totalshots, totaldamage, suicides, teamkills, timeplayed, matches;
	playerscore() : frags(0), deaths(0), totalshots(0), totaldamage(0), suicides(0), teamkills(0), timeplayed(0), matches(0) {}
};
vector<playerscore *> playerscores;
bool playerscoresort(playerscore *a, playerscore *b) {
	if(a->frags > b->frags) return true;
	if(a->frags < b->frags) return false;
	if(a->flags > b->flags) return true;
	if(a->flags < b->flags) return false;
	if((float)((float)a->frags/max((float)a->deaths, 1.f)) > (float)((float)b->frags/max((float)b->deaths, 1.f))) return true;
	if((float)((float)a->frags/max((float)a->deaths, 1.f)) < (float)((float)b->frags/max((float)b->deaths, 1.f))) return false;
	if(a->deaths < b->deaths) return true;
	if(a->deaths > b->deaths) return false;
	if((float)((float)a->totaldamage/max((float)a->totalshots, 1.f)) > (float)((float)b->totaldamage/max((float)b->totalshots, 1.f))) return true;
	if((float)((float)a->totaldamage/max((float)a->totalshots, 1.f)) < (float)((float)b->totaldamage/max((float)b->totalshots, 1.f))) return false;
	if(a->teamkills < b->teamkills) return true;
	if(a->teamkills > b->teamkills) return false;
	if(a->suicides < b->suicides) return true;
	if(a->suicides > b->suicides) return false;
	if(a->matches > b->matches) return true;
	if(a->matches < b->matches) return false;
	if(a->timeplayed > b->timeplayed) return true;
	if(a->timeplayed < b->timeplayed) return false;
//	return strcmp(a->name, b->name) < 0;
	if(strcmp(a->name, b->name) < 0) return true;
	if(strcmp(a->name, b->name) > 0) return false;
	return a->range < b->range;
}
playerscore *getplayerscore(char *name, uint ip) {
	playerscores.sort(playerscoresort);
	uint range = (ip % 256) + 256 * ((ip / 256) % 256);
	string n;
	copystring(n, name, MAXNAMELEN+1);
	loopv(playerscores) {
		if(!strcmp(playerscores[i]->name, n) && playerscores[i]->range==range) return playerscores[i];
	}
	return NULL;
}
playerscore *getplayerscore(clientinfo *ci) {
	if(!ci) return NULL;
	return getplayerscore(ci->name, getclientip(ci->clientnum));
}
uint getrange_16(uint ip) {
	return (ip % 256) + 256 * ((ip / 256) % 256);
}
void addplayerscore(int cn) {
	clientinfo *ci = getinfo(cn);
	if(!ci) return;
	playerscore *ps = getplayerscore(ci);
	if(!ps) {
		ps = new playerscore;
		copystring(ps->name, ci->name, MAXNAMELEN+1);
		ps->range = getrange_16(getclientip(ci->clientnum));
		playerscores.add(ps);
	}
}
void setplayerscore(int cn) {
	clientinfo *ci = getinfo(cn);
	if(!ci) return;
	playerscore *ps = getplayerscore(ci);
	if(!ps) return;
	// frags flags deaths totalshots totaldamage teamkills suicides matches timeplayed
	ps->frags += ci->state.frags;
	ps->flags += ci->state.flags;
	ps->deaths += ci->state.deaths;
	ps->totalshots += ci->state.shotdamage;
	ps->totaldamage += ci->state.damage;
	ps->teamkills += ci->state.teamkills;
	ps->suicides += ci->state._suicides;
	ps->matches++;
	ps->timeplayed += (gamemillis - ci->connmillis);
}
void clearplayerscores() {
	playerscores.shrink(0);
}
COMMAND(clearplayerscores, "");
void addplayerscore(char *name, uint range) {
	if(!strcmp(name, "unnamed")) return;
	string n;
	copystring(n, name, MAXNAMELEN+1);
	playerscore *ps = getplayerscore(n, range);
	if(!ps) {
		ps = new playerscore;
		copystring(ps->name, name, MAXNAMELEN+1);
		ps->range = range;
		playerscores.add(ps);
	}
}
ICOMMAND(addplayerscore, "si", (char *name, int *range), addplayerscore(name, *(uint*)range));
#define PS_SET(field) \
void ps_set##field(int val) { \
	if(!playerscores.inrange(0)) return; \
	playerscore *ps = playerscores[playerscores.length()-1]; \
	if(!ps) return; \
	ps->field = val; \
}
PS_SET(frags);
PS_SET(flags);
PS_SET(deaths);
PS_SET(totalshots);
PS_SET(totaldamage);
PS_SET(teamkills);
PS_SET(suicides);
PS_SET(matches);
PS_SET(timeplayed);
#undef PS_SET
ICOMMAND(ps_frags, "i", (int *val), ps_setfrags(*val));
ICOMMAND(ps_flags, "i", (int *val), ps_setflags(*val));
ICOMMAND(ps_deaths, "i", (int *val), ps_setdeaths(*val));
ICOMMAND(ps_totalshots, "i", (int *val), ps_settotalshots(*val));
ICOMMAND(ps_totaldamage, "i", (int *val), ps_settotaldamage(*val));
ICOMMAND(ps_teamkills, "i", (int *val), ps_setteamkills(*val));
ICOMMAND(ps_suicides, "i", (int *val), ps_setsuicides(*val));
ICOMMAND(ps_matches, "i", (int *val), ps_setmatches(*val));
ICOMMAND(ps_timeplayed, "i", (int *val), ps_settimeplayed(*val));

int getrank(int cn) {
	clientinfo *ci = getinfo(cn);
	if(!ci) return -1;
	if(!scoreboard || !scoreboardxml) return 0;
	playerscore *ps = getplayerscore(ci);
	if(!ps) return 0;
	loopv(playerscores) {
		if(playerscores[i]==ps) return i+1;
	}
	return 0;
}
#define PS_GET(field) \
int ps_get##field(int cn) { \
	clientinfo *ci = getinfo(cn); \
	if(!ci) return -1; \
	if(!scoreboard || !scoreboardxml) return 0; \
	playerscore *ps = getplayerscore(ci); \
	if(!ps) return 0; \
	loopv(playerscores) { \
		if(playerscores[i]==ps) return ps->field; \
	} \
	return 0; \
}
PS_GET(frags);
PS_GET(flags);
PS_GET(deaths);
PS_GET(totalshots);
PS_GET(totaldamage);
PS_GET(teamkills);
PS_GET(suicides);
PS_GET(matches);
PS_GET(timeplayed);
#undef PS_GET
void savescorescfg() {
	playerscores.sort(playerscoresort);
	stream *f = openutf8file(path("scores.cfg", true), "w");
    if(f)
    {
        f->printf("// List of all the players with their scores\n");
        f->printf("clearplayerscores\n");
        loopv(playerscores) {
			playerscore *ps = playerscores[i];
			f->printf("\naddplayerscore %s %u", escapestring(ps->name), ps->range);
			f->printf("\nps_frags %d", ps->frags);
			f->printf("\nps_flags %d", ps->flags);
			f->printf("\nps_deaths %d", ps->deaths);
			f->printf("\nps_totalshots %d", ps->totalshots);
			f->printf("\nps_totaldamage %d", ps->totaldamage);
			f->printf("\nps_teamkills %d", ps->teamkills);
			f->printf("\nps_suicides %d", ps->suicides);
			f->printf("\nps_matches %d", ps->matches);
			f->printf("\nps_timeplayed %d", ps->timeplayed);
		}
        delete f;
    }
}
void savescoresxml() {
	playerscores.sort(playerscoresort);
	stream *f = openfile(path(scoreboardxml, true), "w");
	if(f) {
		f->printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		f->printf("<scoreboard>");
		loopv(playerscores) {
			f->printf("<player>");
//    			char *n = strreplace(playerscores[i]->name, "<", "&amp;lt;");
			uchar dst[MAXSTRLEN];
			encodeutf8(dst, MAXNAMELEN+1, (uchar*)playerscores[i]->name, MAXNAMELEN+1, NULL);
			char *n = strreplace(playerscores[i]->name, "&", "&amp;");
			char *m = strreplace(n, "<", "&amp;lt;");
			f->printf("<name>%s</name>", strreplace(m, ">", "&amp;gt;"));
			f->printf("<frags>%d</frags>", playerscores[i]->frags);
			f->printf("<flags>%d</flags>", playerscores[i]->flags);
			f->printf("<deaths>%d</deaths>", playerscores[i]->deaths);
			f->printf("<totalshots>%d</totalshots>", playerscores[i]->totalshots);
			f->printf("<totaldamage>%d</totaldamage>", playerscores[i]->totaldamage);
			f->printf("<suicides>%d</suicides>", playerscores[i]->suicides);
			f->printf("<teamkills>%d</teamkills>", playerscores[i]->teamkills);
			f->printf("<timeplayed>%d</timeplayed>", playerscores[i]->timeplayed);
			f->printf("<matches>%d</matches>", playerscores[i]->matches);
			f->printf("</player>");
		}
		f->printf("</scoreboard>");
		delete f;
	}
}
