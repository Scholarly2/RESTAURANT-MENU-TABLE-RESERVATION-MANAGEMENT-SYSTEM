#include <stdio.h>
#include <limits.h>
#include "../Include_type_struct/Input.h"
#include "../Include_type_struct/RestaurantTypes.h"
#include "../Include_type_struct/menu.h"
#include "../Include_type_struct/Login.h"
#include "../Include_type_struct/FileHelper.h"

/* ===================== HELPERS ===================== */

const char *statusToString(int status)
{
    switch (status) {
        case TABLE_AVAILABLE: return "Available";
        case TABLE_BOOKED:    return "Booked";
        case TABLE_SERVING:   return "Serving";
        default:              return "Unknown";
    }
}

void showTables(const Table *tables, int tableCount)
{
    int i;

    printf("%-8s | %-6s | %s\n", "Table", "Seats", "Status");
    for (i = 0; i < tableCount; i++) {
        printf("| %-8d | %-7d | %-12s |\n", tables[i].tableNumber,
               tables[i].tableType, statusToString(tables[i].tableStatus));
    }
    puts("+----------+---------+--------------+");
    if (tableCount == 0) puts("(no tables)");
}

int findTableByID(const State *s, int tableID)
{
    int i;

    if (s == NULL) return -1;
    for (i = 0; i < s->tableCount; i++) {
        if (s->tables[i].tableID == tableID) return i;
    }
    return -1;
}

int findTableByNumber(const State *s, int tableNumber)
{
    int i;

    if (s == NULL) return -1;
    for (i = 0; i < s->tableCount; i++) {
        if (s->tables[i].tableNumber == tableNumber) return i;
    }
    return -1;
}

int tableTypeForGuests(int guests)
{
    if (guests < 1 || guests > 8) return -1;
    if (guests <= 2) return 2;
    if (guests <= 4) return 4;
    return 8;
}

int getExpectedTableStatus(const State *s, int tableID)
{
    int i;

    for (i = 0; i < s->orderCount; i++) {
        if (s->orders[i].tableID == tableID
            && s->orders[i].orderStatus == ORDER_OPEN
            && !s->orders[i].isPreOrder) {
            return TABLE_SERVING;
        }
    }
    for (i = 0; i < s->resCount; i++) {
        if (s->reservations[i].tableID == tableID
            && s->reservations[i].reservationStatus == WAITING) {
            return TABLE_BOOKED;
        }
    }
    return TABLE_AVAILABLE;
}

static int findWaitingReservation(const State *s, int tableID)
{
    int i, found = -1, n = 0;

    for (i = 0; i < s->resCount; i++) {
        if (s->reservations[i].tableID == tableID
            && s->reservations[i].reservationStatus == WAITING) {
            found = i;
            n++;
        }
    }
    return (n == 1) ? found : -1;
}

/* ===================== 3.24 updateStatusAutoTable ===================== */

    int updateStatusAutoTable(State *s, int tableID, int event)
    {
        int i;

        if (s == NULL) return FALSE;

        i = findTableByID(s, tableID);
        if (i == -1) return FALSE;

        if (event != CHECK_IN && event != BOOK &&
            event != CANCEL && event != PAY_BILL &&
            event != CANCEL_ORDER) {
            return FALSE;
        }

        //thay doi neu tim thay
        s->tables[i].tableStatus = getExpectedTableStatus(s, tableID);

        return TRUE;

    }

/* ===================== shared interactive flows ===================== */

int promptCreateReservation(int tableID)
{
    char     name[CUSTOMER_NAME_SIZE];
    char     phone[PHONE_SIZE];
    DateTime when;
    int      guests, maxGuests = 8, t;

    if (tableID > 0) {
        t = findTableByID(&S, tableID);
        if (t == -1) return FALSE;
        maxGuests = S.tables[t].tableType;
    }

    if (!readInt("Guests: ", 1, maxGuests, &guests)) return FALSE;
    if (!readName("Customer: ", TRUE, name))         return FALSE;
    if (!readPhone(phone))                           return FALSE;
    if (!readBookingTime(&when))                     return FALSE;

    return createReservation(name, phone, guests, when,
                             tableTypeForGuests(guests), tableID);
}

int promptPayOrder(int orderIndex)
{
    double discount, tax;
    int    confirm;
    Bill   b;
    int orderID;

    if (orderIndex < 0 || orderIndex >= S.orderCount) return FALSE;

    if (!readDouble("Discount percent: ", 0, 100, &discount)) return FALSE;
    if (!readDouble("Tax percent: ", 0, 100, &tax))           return FALSE;

    if (!calculateBill(&S.orders[orderIndex], S.policy, discount, tax, &b)) {
        return FALSE;
    }
  	printBillReport(orderID, discount,tax);

    if (!readInt("Confirm payment 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
    if (confirm == 0) return FALSE;

    return printBillReport(S.orders[orderIndex].orderID, discount, tax);
}

/* ===================== 3.25 updateStatusTable ===================== */

/*
 *   Available -> Serving : create a dine-in order (walk-in)
 *   Available -> Booked  : create a reservation
 *   Booked    -> Serving : check the reservation in
 *   Booked    -> Available : cancel the reservation
 *   Serving   -> Available : pay or cancel the open order
 *   Serving   -> Booked  : not allowed
 */
int updateStatusTable(int tableID, int newStatus)
{
    int t, oldStatus;

    t = findTableByID(&S, tableID);
    if (t == -1) return FALSE;

    if (newStatus != TABLE_AVAILABLE && newStatus != TABLE_BOOKED
        && newStatus != TABLE_SERVING) {
        return FALSE;
    }

    oldStatus = S.tables[t].tableStatus;
    if (oldStatus == newStatus) return TRUE;

    if (oldStatus == TABLE_AVAILABLE) {
        if (newStatus == TABLE_SERVING) {
            int guests;

            if (!readInt("Guests: ", 1, S.tables[t].tableType, &guests)) return FALSE;
            return createOrderTable(DINE_IN, 0, tableID, guests, 0);
        }
        return promptCreateReservation(tableID);            /* -> BOOKED */
    }

    if (oldStatus == TABLE_BOOKED) {
        int r = findWaitingReservation(&S, tableID);
        int confirm;

        if (r == -1) return FALSE;
        if (newStatus == TABLE_SERVING) {
            return CheckInReservation(S.reservations[r].phoneNumber);
        }
        if (!readInt("Cancel booking 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
        if (confirm == 0) return FALSE;
        return cancelReservation(S.reservations[r].phoneNumber);
    }

    /* oldStatus == TABLE_SERVING */
    {
        int o, action, confirm;

        if (newStatus == TABLE_BOOKED) {
            printf("Close the current order before a new booking\n");
            return FALSE;
        }
        o = findOpenOrder(&S, tableID, 0);
        if (o == -1) return FALSE;

        if (!readInt("1 Pay 2 Cancel order 0 Back: ", 0, 2, &action)) return FALSE;
        if (action == 0) return FALSE;
        if (action == 1) return promptPayOrder(o);

        if (!readInt("Confirm cancellation 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
        if (confirm == 0) return FALSE;
        return cancelOrder(S.orders[o].orderID);
    }
}



/* ===================== 3.26 findAvailableTable ===================== */

int findAvailableTable(const State *s, int numberCustomer)
{
    int best = -1;
    int i;

    if (s == NULL || numberCustomer < 1 || numberCustomer > 8) return -1;

    for (i = 0; i < s->tableCount; i++) {
        if (s->tables[i].tableStatus != TABLE_AVAILABLE) continue;
        if (s->tables[i].tableType < numberCustomer) continue;

        if (best == -1
            || s->tables[i].tableType < s->tables[best].tableType
            || (s->tables[i].tableType == s->tables[best].tableType
                && s->tables[i].tableNumber < s->tables[best].tableNumber)) {
            best = i;
        }
    }
    return best;
}

/* ===================== 3.27 sortTableByStatus ===================== */

void sortTableByStatus(void)
{
    Table view[MAX_TABLES];
    Table temp;
    int   i, j;

    for (i = 0; i < S.tableCount; i++) view[i] = S.tables[i];   /* work on a copy */

    for (i = 0; i < S.tableCount - 1; i++) {
        for (j = 0; j < S.tableCount - i - 1; j++) {
            if (view[j].tableStatus > view[j + 1].tableStatus
                || (view[j].tableStatus == view[j + 1].tableStatus
                    && view[j].tableNumber > view[j + 1].tableNumber)) {
                temp         = view[j];
                view[j]      = view[j + 1];
                view[j + 1]  = temp;
            }
        }
    }
    showTables(view, S.tableCount);
}

/* ======= MENU FOR TABLE MANAGER ========*/

void TableMenu(void)
{
    int choice, tableID, option, guests, index, newStatus;

    for (;;) {
        puts("\n========== TABLE MANAGEMENT ==========");
        puts("1. Show tables");
        puts("2. Change table status");
        puts("3. Find available table");
        puts("4. Sort tables by status");
        puts("0. Back");
        if (!readInt("Choice: ", 0, 4, &choice) || choice == 0) return;

        switch (choice) {
        case 1:
            showTables(S.tables, S.tableCount);
            break;
        case 2:
            showTables(S.tables, S.tableCount);
            if (!readInt("Table ID (0 Back): ", 0, INT_MAX, &tableID)) return;
            if (tableID == 0) break;
            if (findTableByID(&S, tableID) == -1) { puts("Table not found."); break; }
            if (!readInt("New status (1 Available, 2 Booked, 3 Serving): ", 1, 3, &option)) return;
            newStatus = option == 1 ? TABLE_AVAILABLE : (option == 2 ? TABLE_BOOKED : TABLE_SERVING);
            puts(updateStatusTable(tableID, newStatus) ? "Table status updated."
                                                       : "Table status was not changed.");
            break;
        case 3:
            if (!readInt("Number of customers (1-8): ", 1, 8, &guests)) return;
            index = findAvailableTable(&S, guests);
            if (index < 0 || index >= S.tableCount) puts("No suitable table available.");
            else showTables(&S.tables[index], 1);
            break;
        case 4:
            sortTableByStatus();
            break;
        }
    }
}