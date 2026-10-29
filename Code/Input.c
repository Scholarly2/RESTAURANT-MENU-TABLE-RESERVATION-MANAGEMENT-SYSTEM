void getCurrentDateTime(DateTime*reasult){
    time_t now = time(NULL);
	struct tm*t=localtime(&now);
	if (reasult==NULL|| t== NULL){
		printf("Failed to get current time or result pointer is NULL.\n");
        return;
	}
	(*reasult).year=t->tm_year+1900;
	(*reasult).month=t->tm_mon +1;
	(*reasult).day=t->tm_mday;
	(*reasult).hour=t->tm_hour;
	(*reasult).minute=t->tm_min;
}
      