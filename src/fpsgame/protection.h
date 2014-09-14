struct protectedname {
	string name;
	protectedname() {};
};
struct protecteduser {
	vector<protectedname> names;
	string authname;
	protecteduser() {};
};
struct protectedclan {
	string tag;
	protectedclan() {};
};
struct protectedclanuser {
	vector<protectedclan> clans;
	string authdesc;
	protectedclanuser() {};
};
vector<protecteduser *> protectedusers;
vector<protectedclanuser *> protectedclanusers;
int getuseridx(char *name) {
	loopv(protectedusers) {
		if(!strcasecmp(protectedusers[i]->authname, name)) return i;
	}
	return -1;
}
int getclanuseridx(char *tag) {
	loopv(protectedclanusers) {
		if(!strcasecmp(protectedclanusers[i]->authdesc, tag)) return i;
	}
	return -1;
}
bool protecteduserlogin(int cn, char *authname) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE) return false;
	if(!isreservedname(ci->name)) return true;
	if(!strcasecmp(ci->authname, authname)) {
		ci->logged = getuseridx(authname) > -1 ? true : false;
		return ci->logged;
	}
	return false;
}
bool protectedclanuserlogin(int cn, char *authdesc) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE) return false;
	if(!isreservedclan(ci->name)) return true;
	if(!strcasecmp(ci->authdesc, authdesc)) {
		ci->logged = getclanuseridx(authdesc) > -1 ? true : false;
	}
	return false;
}
bool isreservedname(char *name) {
	loopv(protectedusers) {
		loopvj(protectedusers[i]->names)
			if(!strcasecmp(protectedusers[i]->names[j].name, name)) return true;
	}
	return false;
}
int _strcasestr(char *a, char *b) {
	char *s = strcasestr(a, b);
	return s ? s-a : -1;
}
bool isreservedclan(char *name) {
	loopv(protectedclanusers) {
		loopvj(protectedclanusers[i]->clans)
			if(_strcasestr(name, protectedclanusers[i]->clans[j].tag) >= 0) return true;
	}
	return false;
}
void checkreservedname(int cn) {
// lastnamechange
// namemessages
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE || !isreservedname(ci->name) || ci->logged) return;
	int diff = totalmillis - ci->lastnamechange;
	if(ci->namemessages==0) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3[\f7Warning\f3]\f7 you are using a \f3reserved\f7 name! You have \f030\f7 seconds to authenticate!");
	} else if(diff>=10000 && ci->namemessages==1) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3[\f7Warning\f3]\f7 you are using a \f3reserved\f7 name! You have \f020\f7 seconds to authenticate!");
	} else if(diff>=20000 && ci->namemessages==2) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3[\f7Warning\f3]\f7 you are using a \f3reserved\f7 name! You have \f010\f7 seconds to authenticate!");
	} else if(diff>=30000 && ci->namemessages==3) {
		ci->namemessages++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f3[\f7Kick\f3]\f0 server\f3 kicked \f6%s\f7 because: \f3use of reserved name", colorname(ci));
		kickclients(getclientip(ci->clientnum));
	}
}
void checkreservedclan(int cn) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE || !isreservedclan(ci->name) || ci->logged) return;
	int diff = totalmillis - ci->lastnamechange;
	if(ci->namemessages==0) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3[\f7Warning\f3]\f7 you are using a \f3reserved\f7 clan-tag! You have \f030\f7 seconds to authenticate!");
	} else if(diff>=10000 && ci->namemessages==1) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3[\f7Warning\f3]\f7 you are using a \f3reserved\f7 clan-tag! You have \f020\f7 seconds to authenticate!");
	} else if(diff>=20000 && ci->namemessages==2) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3[\f7Warning\f3]\f7 you are using a \f3reserved\f7 clan-tag! You have \f010\f7 seconds to authenticate!");
	} else if(diff>=30000 && ci->namemessages==3) {
		ci->namemessages++;
		sendf(-1, 1, "ris", N_SERVMSG, "\f3[\f7Kick\f3]\f0 server\f3 kicked \f6%s\f7 because: \f3use of reserved clan tag", colorname(ci));
		kickclients(getclientip(ci->clientnum));
	}
}
void addreservedname(char *nick, char *authname) {
	if(isreservedname(nick)) return;
	int idx = getuseridx(authname);
	if(idx <= -1) {
		protecteduser *pu = new protecteduser();
		copystring(pu->authname, authname);
		protectedname pn;
		copystring(pn.name, nick);
		pu->names.add(pn);
		protectedusers.add(pu);
	} else {
		protecteduser *pu = protectedusers[idx];
		protectedname pn;
		copystring(pn.name, nick);
		pu->names.add(pn);
	}
}
void addreservedclan(char *tag, char *authdesc) {
	if(isreservedclan(tag)) return;
	int idx = getclanuseridx(authdesc);
	if(idx <= -1) {
		protectedclanuser *pcu = new protectedclanuser();
		copystring(pcu->authdesc, authdesc);
		protectedclan pc;
		copystring(pc.tag, tag);
		pcu->clans.add(pc);
		protectedclanusers.add(pcu);
	} else {
		protectedclanuser *pcu = protectedclanusers[idx];
		protectedclan pc;
		copystring(pc.tag, tag);
		pcu->clans.add(pc);
	}
}
ICOMMAND(addreservedname, "ss", (char *nick, char *authname), addreservedname(nick, authname));
ICOMMAND(addreservedclan, "ss", (char *tag, char *authdesc), addreservedclan(tag, authdesc));
