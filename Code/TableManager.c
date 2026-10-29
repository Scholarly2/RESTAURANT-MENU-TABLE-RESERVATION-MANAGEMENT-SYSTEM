#include <stdio.h>
#include "common.h"
#include "Input.h"
#include "TableManager.h"
#include "ReservationManager.h"
#include "OrderManager.h"
#include "BillManager.h"

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
        printf("%-8d | %-6d | %s\n",
               tables[i].tableNumber,
               tables[i].tableType,
               statusToString(tables[i].tableStatus));
    }
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
    int i, expected;

    if (s == NULL) return FALSE;

    i = findTableByID(s, tableID);
    if (i == -1) return FALSE;

    if (event != BOOK && event != CANCEL && event != CHECK_IN
        && event != PAY_BILL && event != CANCEL_ORDER) {
        return FALSE;
    }

    expected = getExpectedTableStatus(s, tableID);

    if (event == BOOK && expected != TABLE_BOOKED) return FALSE;
    if (event == CHECK_IN && expected != TABLE_SERVING) return FALSE;
    if ((event == CANCEL || event == PAY_BILL || event == CANCEL_ORDER)
        && expected != TABLE_AVAILABLE) {
        return FALSE;
    }

    s->tables[i].tableStatus = expected;
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

    if (orderIndex < 0 || orderIndex >= S.orderCount) return FALSE;

    if (!readDouble("Discount percent: ", 0, 100, &discount)) return FALSE;
    if (!readDouble("Tax percent: ", 0, 100, &tax))           return FALSE;

    if (!calculateBill(&S.orders[orderIndex], S.policy, discount, tax, &b)) {
        return FALSE;
    }
    displayBillReport(&b);

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
            return checkInReservation(S.reservations[r].reservationID);
        }
        if (!readInt("Cancel booking 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
        if (confirm == 0) return FALSE;
        return cancelReservation(S.reservations[r].reservationID);
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