int inputReservation(void) {
    char nameCustomer[CUSTOMER_NAME_SIZE];
    char phoneNumber[PHONE_SIZE];
    int NumberCustomer;
    int tableType;
    DateTime timeBooking;
    if (!readName("Customer name: ", TRUE, nameCustomer)) {
        return FALSE;
    }
    if (!readPhone(phoneNumber)) {
        return FALSE;
    }
    if (!readInt("Number of customers: ", 1, 8,
                 &NumberCustomer)) {
        return FALSE;
    }
    if (!readBookingTime(&timeBooking)) {
        return FALSE;
    }
    if (NumberCustomer <= 2) {
        tableType = 2;
    }
    else if (NumberCustomer <= 4) {
        tableType = 4;
    }
    else {
        tableType = 8;
    }
    return createReservation(
        nameCustomer,
        phoneNumber,
        NumberCustomer,
        timeBooking,
        tableType,
        0
    );
}
