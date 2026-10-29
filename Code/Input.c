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
   int findReservationByID(const State*s,int id){
    if (s==NULL||id <=0){
        return -1;
    }
    for (int i=0;i<s->resCount;i++){
        if (s->reservations[i].reservationID==id){
        printf("Found reservation at index: %d\n", i);
        return i;
            printf("Found reservation at index: %d\n", i);
        }
    }
        printf("Reservation with ID %d not found.\n", id);
    return FALSE;
}
int isValidPhone(const char*phoneNumber){
    const char*p=phoneNumber;
	if(phoneNumber==NULL){
		return FALSE;
	}
	if (*p=='+'){
		p=p+1;
        printf("Phone number starts with '+', skipping to: %s\n", p);
	}
	else if (*p!='0'){
        printf("Phone number does not start with '0' or '+'.\n");
        return FALSE;
	}
	int digits=strlen(p);
    printf("Phone number after processing: %s, digits count: %d\n", p, digits);
	if (digits<MIN_PHONE_DIGITS||digits>MAX_PHONE_DIGITS){
		printf("Phone number has invalid number of digits: %d\n", digits);
        return FALSE;
	}
    printf("Validating each character in phone number: %s\n", p);
	while (*p!='\0'){
		if (!isdigit((unsigned char)*p)){
		printf("Invalid character in phone number: %c\n", *p);
        return FALSE;
		}
		p=p+1;
        printf("Next character to validate: %c\n", *p);
	}
	    printf("Phone number is valid: %s\n", phoneNumber);
    return TRUE;
}
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
    printf("Current DateTime set to: %04d-%02d-%02d %02d:%02d\n", reasult->year, reasult->month, reasult->day, reasult->hour, reasult->minute);	
}
int compareDateTime(DateTime a, DateTime b) {
long long ka = (((a.year * 100LL + a.month)
                * 100 + a.day)
                * 100 + a.hour)
                * 100 + a.minute;
printf("Comparing DateTime A: %04d-%02d-%02d %02d:%02d (ka=%lld) with DateTime B: %04d-%02d-%02d %02d:%02d\n",
           a.year, a.month, a.day, a.hour, a.minute, ka,
           b.year, b.month, b.day, b.hour, b.minute);
    long long kb = (((b.year * 100LL + b.month) * 100 + b.day)
                    * 100 + b.hour) * 100 + b.minute;
                    printf("Computed kb=%lld for DateTime B\n", kb);
    if (ka < kb) {
        printf("DateTime A is earlier than DateTime B\n");
        return -1;
    }
    if (ka > kb) {
        printf("DateTime A is later than DateTime B\n");
        return TRUE;
    }
    printf("DateTime A is equal to DateTime B\n");
    return FALSE;
}
int isValidBookingTime(DateTime t){
    DateTime now;
    getCurrentDateTime(&now);
    if (compareDateTime(t, now) < 0) {
        return FALSE;
    }
    time_t bookingTimestamp, nowTimestamp;
    if (!toTimestamp(t, &bookingTimestamp) || !toTimestamp(now, &nowTimestamp)) {
        return FALSE;
    }
    double diffMinutes = difftime(bookingTimestamp, nowTimestamp) / 60.0;
    if (diffMinutes < CHECKIN_EARLY_MIN) {
        return FALSE;
    }
    return TRUE;
}   