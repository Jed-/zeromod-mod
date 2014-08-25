void parsebar(const char *m, int cn) {
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
	defformatstring(str0)("hi, %s", bname);
	defformatstring(str1)("hi %s", bname);
	defformatstring(str2)("hey, %s", bname);
	defformatstring(str3)("hey %s", bname);
	defformatstring(str4)("hello, %s", bname);
	defformatstring(str5)("hello %s", bname);
	char *text = (char*)m;
	for(int j=0; text[j]; j++) text[j] = tolower(text[j]);
	defformatstring(cmd)("barhi %d %d", botcn, cn);
	if(strstr(text, str0) || strstr(text, str1) || strstr(text, str2) || strstr(text, str3) || strstr(text, str4) || strstr(text, str5)) execute(cmd);
}
