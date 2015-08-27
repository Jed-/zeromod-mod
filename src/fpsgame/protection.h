struct protectedname {
	string name;
	protectedname() {};
};
struct protecteduser {
	vector<protectedname *> names;
	string authname;
	protecteduser() {};
};
struct protectedclan {
	string tag;
	protectedclan() {};
};
struct protectedclanuser {
	vector<protectedclan *> clans;
	string authdesc;
	protectedclanuser() {};
};
vector<protecteduser *> protectedusers;
vector<protectedclanuser *> protectedclanusers;
int getuseridx(const char *name) {
	loopv(protectedusers) {
		if(!strcasecmp(protectedusers[i]->authname, name)) return i;
	}
	return -1;
}
int getclanuseridx(const char *tag) {
	loopv(protectedclanusers) {
		if(!strcasecmp(protectedclanusers[i]->authdesc, tag)) return i;
	}
	return -1;
}
bool isreservedname(char *name) {
	loopv(protectedusers) {
		loopvj(protectedusers[i]->names)
			if(!strcasecmp(protectedusers[i]->names[j]->name, name)) return true;
	}
	return false;
}
char *reservedauthname(char *name) {
	loopv(protectedusers) {
		loopvj(protectedusers[i]->names) {
			if(!strcasecmp(protectedusers[i]->names[j]->name, name)) return protectedusers[i]->authname;
		}
	}
	return NULL;
}
int _strcasestr(char *a, char *b) {
	char *_a = lowerstring(a);
	char *_b = lowerstring(b);
	char *s = strstr(_a, _b);
	return s ? s-_a : -1;
}
bool isreservedclan(char *name) {
	loopv(protectedclanusers) {
		loopvj(protectedclanusers[i]->clans)
			if(_strcasestr(name, protectedclanusers[i]->clans[j]->tag) >= 0) return true;
	}
	return false;
}
bool isreservedauthdesc(char *desc) {
	loopv(protectedclanusers) {
		loopvj(protectedclanusers[i]->clans)
			if(!strcasecmp(desc, protectedclanusers[i]->clans[j]->tag)) return true;
	}
	return false;
}
char *reserveddomain(char *name) {
	loopv(protectedclanusers) {
		loopvj(protectedclanusers[i]->clans)
			if(_strcasestr(name, protectedclanusers[i]->clans[j]->tag) >= 0) return protectedclanusers[i]->authdesc;
	}
	return NULL;
}
bool protecteduserlogin(int cn, const char *authname, const char *authdesc) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE) return false;
	if(!isreservedname(ci->name)) return true;
	bool tolog = !ci->logged;
	bool cond = false;
	if(getuseridx(authname) > -1 && protectedusers.inrange(getuseridx(authname))) {
		loopv(protectedusers[getuseridx(authname)]->names) {
			if(!strcasecmp(protectedusers[getuseridx(authname)]->names[i]->name, ci->name)) {
				cond = true;
				break;
			}
		}
	}
	ci->logged = cond;/* (getuseridx(authname) > -1 && protectedusers.inrange(getuseridx(authname))) ? true : false; */
	if(ci->logged && tolog) {
		defformatstring(msg)("\f0[\f7Protection\f0]\f1 %s \f7is \f0verified\f7 as '\f6%s\f7' \f0[\f7%s\f0]", colorname(ci), authname, authdesc);
		if(ci->_xi.spy) {
			loopv(clients) {
				if(clients[i]->state.state==AI_NONE && clients[i]->privilege >= PRIV_ADMIN)
					sendf(clients[i]->clientnum, 1, "ris", N_SERVMSG, msg);
			}
		} else {
			sendf(-1, 1, "ris", N_SERVMSG, msg);
		}
	}
	if(ci->logged) {
		copystring(ci->loginname, authname, MAXSTRLEN);
		copystring(ci->logindesc, authdesc, MAXSTRLEN);
	}
	return ci->logged;
}
bool protectedclanuserlogin(int cn, const char *authname, const char *authdesc) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE) return false;
	if(!isreservedclan(ci->name)) return true;
	bool tolog = !ci->logged;
	bool cond = false;
	if(getclanuseridx(authdesc) > -1 && protectedclanusers.inrange(getclanuseridx(authdesc))) {
		loopv(protectedclanusers[getclanuseridx(authdesc)]->clans) {
			if(!_strcasestr(protectedclanusers[getclanuseridx(authdesc)]->clans[i]->tag, ci->name)) {
				cond = true;
				break;
			}
		}
	}
	ci->logged = cond;/* getclanuseridx(authdesc) > -1 ? true : false; */
	if(ci->logged && tolog) {
		defformatstring(msg)("\f0[\f7Protection\f0]\f1 %s \f7is \f0verified\f7 as '\f6%s\f7' \f0[\f7%s\f0]", colorname(ci), authname, authdesc);
		if(ci->_xi.spy) {
			loopv(clients) {
				if(clients[i]->state.state==AI_NONE && clients[i]->privilege >= PRIV_ADMIN)
					sendf(clients[i]->clientnum, 1, "ris", N_SERVMSG, msg);
			}
		} else {
			sendf(-1, 1, "ris", N_SERVMSG, msg);
		}
	}
	if(ci->logged) {
		copystring(ci->loginname, authname, MAXSTRLEN);
		copystring(ci->logindesc, authdesc, MAXSTRLEN);
	}
	return ci->logged;
}
void checkprotection(int cn) {
	clientinfo *ci = getinfo(cn);
	if(!ci || ci->state.aitype!=AI_NONE || (totalmillis-ci->lastnamechange) < 3000 || !ci->loaded || !ci->lastloaded || (totalmillis-ci->lastloaded) < 3000 || ci->logged || !(isreservedname(ci->name) || isreservedclan(ci->name))) return;
	// player is not logged in
	// player has either registered name or clantag
	if(isreservedname(ci->name) && ((totalmillis-ci->lastnamechange) < 10000 || (totalmillis-ci->lastloaded) < 10000)) { // give reserved name users 10 seconds to authenticate
		if(!ci->namemessages && reservedauthname(ci->name)) {
			defformatstring(msg)("\f3[\f7Warning\f3]\f7 you are using a reserved name. You have 10 seconds to authenticate as \f6'\f7%s\f6'\f7!", reservedauthname(ci->name));
			sendf(ci->clientnum, 1, "ris", N_SERVMSG, msg);
			ci->namemessages++;
		}
		return;
	}
//	if(isreservedclan(ci->name)) protectedclanuserlogin(ci->clientnum, ci->authdesc);
//	if(!isreservedname(ci->name) || protecteduserlogin(ci->clientnum, ci->authname)) return;
	logoutf("Renaming %s because:", ci->name);
	logoutf("\tis bot: %s", ci->state.aitype!=AI_NONE ? "true" : "false");
	logoutf("\tchanged name for %.3fs", (float)(totalmillis-ci->lastnamechange)/1000.f);
	logoutf("\tloaded map: %s", ci->loaded ? "true" : "false");
	logoutf("\t\tloaded map for %.3fs", (float)(totalmillis-ci->lastloaded)/1000.f);
	logoutf("\t\tloaded map at %.3fs", (float)(ci->lastloaded)/1000.f);
	logoutf("\tname registered: %s", isreservedname(ci->name) ? "yes" : "no");
	if(isreservedname(ci->name)) logoutf("\t\tregistered authname: %s", reservedauthname(ci->name));
	logoutf("\tclantag registered: %s", isreservedclan(ci->name) ? "yes" : "no");
	if(isreservedclan(ci->name)) logoutf("\t\tregistered authdesc: %s", reserveddomain(ci->name));
	logoutf("\tlogged: %s", ci->logged ? "true" : "false");
	copystring(ci->name, "unnamed", MAXNAMELEN+1);
	_rename(ci, ci->name, true);
}
void addreservedname(char *nick, const char *authname) {
	if(isreservedname(nick)) return;
	int idx = getuseridx(authname);
	if(idx <= -1) {
		protecteduser *pu = new protecteduser();
		copystring(pu->authname, authname);
		protectedname *pn = new protectedname;
		copystring(pn->name, nick, MAXSTRLEN);
		pu->names.add(pn);
		protectedusers.add(pu);
	} else {
		protecteduser *pu = protectedusers[idx];
		protectedname *pn = new protectedname;
		copystring(pn->name, nick, MAXSTRLEN);
		pu->names.add(pn);
	}
}
void addreservedclan(char *tag, const char *authdesc) {
	if(isreservedclan(tag)) return;
	int idx = getclanuseridx(authdesc);
	if(idx <= -1) {
		protectedclanuser *pcu = new protectedclanuser();
		copystring(pcu->authdesc, authdesc);
		protectedclan *pc = new protectedclan;
		copystring(pc->tag, tag, MAXSTRLEN);
		pcu->clans.add(pc);
		protectedclanusers.add(pcu);
	} else {
		protectedclanuser *pcu = protectedclanusers[idx];
		protectedclan *pc = new protectedclan;
		copystring(pc->tag, tag, MAXSTRLEN);
		pcu->clans.add(pc);
	}
}
void clearprotection() {
	protectedusers.shrink(0);
	protectedclanusers.shrink(0);
}
void storeprotection() {
	stream *f = openutf8file(path("protection.cfg", true), "w");
	if(f) {
		f->printf("// Reserved users and clans list\nclearprotection");
		loopv(protectedusers) {
			loopvj(protectedusers[i]->names) {
				f->printf("\naddreservedname %s %s", protectedusers[i]->names[j]->name, protectedusers[i]->authname);
			}
		}
		loopv(protectedclanusers) {
			loopvj(protectedclanusers[i]->clans) {
				f->printf("\naddreservedclan %s %s", protectedclanusers[i]->clans[j]->tag, protectedclanusers[i]->authdesc);
			}
		}
		delete f;
	}
}
ICOMMAND(clearprotection, "", (), clearprotection());
ICOMMAND(addreservedname, "ss", (char *nick, char *authname), addreservedname(nick, authname));
ICOMMAND(addreservedclan, "ss", (char *tag, char *authdesc), addreservedclan(tag, authdesc));
