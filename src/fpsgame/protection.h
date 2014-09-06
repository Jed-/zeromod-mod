struct protectedname {
	string name;
	protectedname() {};
};
struct protecteduser {
	vector<protectedname> names;
	string authname;
	protecteduser() {};
};
vector<protecteduser *> protectedusers;
int getuseridx(char *name) {
	loopv(protectedusers) {
		if(!strcasecmp(protectedusers[i]->authname, name)) return i;
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
bool isreservedname(char *name) {
	loopv(protectedusers) {
		loopvj(protectedusers[i]->names)
			if(!strcasecmp(protectedusers[i]->names[j].name, name)) return true;
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
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "You are using a reserved name! You have 30 seconds to authenticate!");
	} else if(diff>=10000 && ci->namemessages==1) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "You are using a reserved name! You have 20 seconds to authenticate!");
	} else if(diff>=20000 && ci->namemessages==2) {
		ci->namemessages++;
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "You are using a reserved name! You have 10 seconds to authenticate!");
	} else if(diff>=30000 && ci->namemessages==3) {
		ci->namemessages++;
		defformatstring(args)("%d unnamed", ci->clientnum);
		_renamefunc("rename", args, NULL);
		sendf(ci->clientnum, 1, "ris", N_SERVMSG, "You have been renamed to 'unnamed' because you were using a reserved name!");
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
ICOMMAND(addreservedname, "ss", (char *nick, char *authname), addreservedname(nick, authname));
