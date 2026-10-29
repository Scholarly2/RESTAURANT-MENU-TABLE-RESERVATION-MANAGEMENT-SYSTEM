#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <RestaurantTypes.h>
#include <string.h>

int createReservation(const char *nameCustomer, const char *phoneNumber, int NumberCustomer, DateTime timeBooking, int tableType, int requestedTableID) {
        // check array Maxreservation max already?
    if (S.resCount >= MAX_RESERVATIONS) {
        return FALSE;
    }
    // check phonenumber and name
    if (!isValidCustomerName(nameCustomer) ||
        !isValidPhone(phoneNumber)) {
        printf("Invalid customer name or phone number.\n");
        return FALSE;
    }
    if (NumberCustomer < 1 || NumberCustomer > 8) {
        return FALSE;
    }
    // Check Tabletype by customer
    int neededType;
printf("Number of customers: %d\n", NumberCustomer);
    if (NumberCustomer <= 2) {
        neededType = 2;
        printf("Needed table type: %d\n", neededType);
    }
    else if (NumberCustomer <= 4) {
        neededType = 4;
        printf("Needed table type: %d\n", neededType);
    }
    else {
        neededType = 8;
        printf("Needed table type: %d\n", neededType);
    }
    // check table type and requestion 
    if (tableType != neededType ||
        requestedTableID < 0) {
        printf("Invalid table type or requested table ID. Table type: %d, Requested table ID: %d\n", tableType, requestedTableID);
        return FALSE;
    }
    // check is valid time 
    if (!isValidBookingTime(timeBooking)) {
        printf("Invalid booking time. Booking time: %04d-%02d-%02d %02d:%02d\n", timeBooking.year, timeBooking.month, timeBooking.day, timeBooking.hour, timeBooking.minute);
        return FALSE;
    }
    // find table phu hop
    int t;
    if (requestedTableID == 0) {
        t = findAvailableTable(&S, NumberCustomer);
        printf("Found available table index: %d\n", t);
    }
    else {
        t = findTableByID(&S, requestedTableID);
printf("Found requested table index: %d\n", t);
        if (t != -1 &&
            (S.tables[t].tableStatus != TABLE_AVAILABLE ||
             S.tables[t].tableType < tableType)) {
            printf("Requested table is not available or does not meet the required type. Table status: %d, Table type: %d\n", S.tables[t].tableStatus, S.tables[t].tableType);
            return FALSE;
        }
    }
    // check co ban trong hay con suc chua không
printf("Next reservation ID: %d\n", nextID(S.lastResID));
    if (t == -1) {
        printf("No available table found for the reservation.\n");
        return FALSE;
    }
    // tạo ID new for Reservation
    int id = nextID(S.lastResID);
    if (id == -1) {
        return FALSE;
    }
    // tạo bản sao
    State *c = copyState(&S);
    if (c == NULL) {
        printf("Failed to copy state for reservation creation.\n");
        return FALSE;
    }
    // khởi tạo bàn mới
    printf("Creating reservation with ID: %d\n", id);
    Reservation r = {0};
    // gan thong tin dat ban 
    r.reservationID = id;
    strcpy(r.nameCustomer, nameCustomer);
    strcpy(r.phoneNumber, phoneNumber);
    r.NumberCustomer = NumberCustomer;
    r.timeBooking = timeBooking;
    r.tableType = tableType;
    // lay thong tin ban tim thay
    r.tableID = c->tables[t].tableID;
    r.reservationStatus = WAITING;
    // add table to array into bản sao
    c->reservations[c->resCount++] = r;
    c->lastResID = id;
    printf("Reservation created successfully. Reservation ID: %d, Table ID: %d\n", r.reservationID, r.tableID);
    // cap nhat du lieu 
    if (!updateStatusAutoTable(c, r.tableID, BOOK)) {
        // giải phóng bộ nhớ
        free(c);
        printf("Failed to update table status after reservation creation.\n");
        return FALSE;
    }
    // update to data
    S = *c;
    free(c);
    return TRUE;
}
int CheckInReservation(const char*phoneNumber){
    int index=-1;
    // Check phone number
    if (!isValidPhone(phoneNumber)) {
        printf("Invalid phone number provided for check-in: %s\n", phoneNumber);
        return FALSE;
    }
    // find waiting reservation
    for (int i=0;i<S.resCount;i++){
        if (strcmp(S.reservations[i].phoneNumber, phoneNumber)==0 && S.reservations[i].reservationStatus==WAITING){
            if (index!=-1){
                printf("Multiple waiting reservations found!\n");
                return FALSE;
            }
            index=i;
        }
    }
    if (index==-1){
        printf("Reservation not found for phone number: %s\n", phoneNumber);
        return FALSE;
    }
    // Check booking time
    time_t bookingTime;
    time_t now=time(NULL);
    if (now == (time_t)-1 ||
        !toTimestamp(S.reservations[index].timeBooking,
                     &bookingTime)) {
        return FALSE;
    }
    if (difftime(bookingTime, now) > CHECKIN_EARLY_MIN * 60.0) {
        printf("Too early to check in!\n");
        return FALSE;
    }
    // check table status
    int tableID = S.reservations[index].tableID;
    int t = findTableByID(&S, tableID);

    if (t == -1 || S.tables[t].tableStatus != TABLE_BOOKED) {
        printf("Table is not booked!\n");
        return FALSE;
    }
    // Copy state
    State *c = copyState(&S);
    if (c == NULL) {
        return FALSE;
    }
    Reservation *r = &c->reservations[index];
    int orderIndex = -1;
    // find existing open order
    for (int i = 0; i < c->orderCount; i++) {
        if (c->orders[i].reservationID == r->reservationID &&
            c->orders[i].orderStatus == ORDER_OPEN) {
            orderIndex = i;
            break;
        }
    }
    // create order if none exitst
    if (orderIndex == -1) {
        if (c->orderCount >= MAX_ORDERS) {
            free(c);
            return FALSE;
        }

        int id = nextID(c->lastOrderID);

        if (id == -1) {
            free(c);
            return FALSE;
        }
        Order o = {0};
        o.orderID = id;
        o.tableID = r->tableID;
        o.reservationID = r->reservationID;
        o.orderType = DINE_IN;
        o.NumberCustomer = r->NumberCustomer;
        o.isPreOrder = FALSE;
        o.orderStatus = ORDER_OPEN;
        c->orders[c->orderCount++] = o;
        c->lastOrderID = id;
    }
    else {
        if (c->orders[orderIndex].tableID != r->tableID ||
            c->orders[orderIndex].orderType != DINE_IN) {
            free(c);
            return FALSE;
        }

        c->orders[orderIndex].isPreOrder = FALSE;
    }
    // Update reservation status
    r->reservationStatus = COMPLETED;
    if (!updateStatusAutoTable(c, tableID, CHECK_IN)) {
        printf("Failed to update table status!\n");
        free(c);
        return FALSE;
    }
    // Update global state
    S = *c;
    free(c);
    printf("Check-in successful!\n");
    printf("Table ID: %d\n", tableID);
    return TRUE;
}
int cancelReservation(const char *phoneNumber) {
    //Check phone number
    if (!isValidPhone(phoneNumber)) {
        printf("Invalid phone number!\n");
        return FALSE;
    }
    int index = -1;
    //Find waiting reservation bang phone
    for (int i = 0; i < S.resCount; i++) {
        if (strcmp(S.reservations[i].phoneNumber,
                   phoneNumber) == 0 &&
            S.reservations[i].reservationStatus == WAITING) {
            // Avoid cancelling the wrong reservation
            if (index != -1) {
                printf("Multiple reservations found!\n");
                return FALSE;
            }
            index = i;
        }
    }
    //No matching reservation
    if (index == -1) {
        printf("Reservation not found!\n");
        return FALSE;
    }
    int reservationID = S.reservations[index].reservationID;
    //Check thhoat open orders
    for (int i = 0; i < S.orderCount; i++) {
        if (S.orders[i].reservationID == reservationID &&
            S.orders[i].orderStatus == ORDER_OPEN) {
            printf("Cancel the related order first!\n");
            return FALSE;
        }
    }
    //Copy tranh loi
    State *c = copyState(&S);
    if (c == NULL)
        return FALSE;
    int tableID = c->reservations[index].tableID;
    //thay doi trang thai
    c->reservations[index].reservationStatus = CANCELLED;

    //Update table status
    if (!updateStatusAutoTable(c, tableID, CANCEL)) {
        free(c);
        printf("Failed to update table status!\n");
        return FALSE;
    }
    //thay doi du lieu
    S = *c;
    free(c);
    printf("Reservation cancelled successfully!\n");
    return TRUE;
}

int findReservationByID(const State *s, int id){
    //tim kiem bang ID
    for(int i = 0; i < S.resCount; i++){
        if(s->reservations[i].reservationID == id){
            return i;
        }
    }
    return -1;
}

void printReservation(void){
    int i;
    if (S.resCount <= 0)
    {
        printf("No reservations found!\n");
        return;
    }
    // Print table header//
    printf("\n%-5s %-20s %-15s %-6s %-16s %-7s %-10s\n",
           "ID", "Customer", "Phone", "Guests",
           "Booking Time", "Table", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    for (i = 0; i < S.resCount; i++)
    {
        const Reservation *r = &S.reservations[i];
        const char *status;
        //Check reservation status//
        if (r->reservationStatus == WAITING)
            status = "WAITING";
        else if (r->reservationStatus == COMPLETED)
            status = "COMPLETED";
        else if (r->reservationStatus == CANCELLED)
            status = "CANCELLED";
        else
            status = "UNKNOWN";

        // Print reservation information
        printf("%-5d %-20.20s %-15.15s %-6d %02d/%02d/%04d %02d:%02d %-7d %-10s\n",
               r->reservationID,
               r->nameCustomer,
               r->phoneNumber,
               r->NumberCustomer,
               r->timeBooking.day,
               r->timeBooking.month,
               r->timeBooking.year,
               r->timeBooking.hour,
               r->timeBooking.minute,
               r->tableID,
               status);
    }
}