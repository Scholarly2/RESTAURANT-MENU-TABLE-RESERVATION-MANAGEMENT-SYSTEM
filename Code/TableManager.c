#include <stdio.h>
#include "common.h"
#include "Input.h"
#include "TableManager.h"

const char *statusToString(int status)
{
    switch (status) {
        case AVAILABLE: return "Available";
        case BOOKED:    return "Booked";
        case SERVING:   return "Serving";
        default:        return "Unknown";
    }
}

void printTables(const Table *tables, int tableCount)
{
    int i;

    printf("%-8s | %-6s | %s\n", "Table", "Type", "Status");
    for (i = 0; i < tableCount; i++) {
        printf("%-8d | %-6d | %s\n",
               tables[i].tableNumber,
               tables[i].tableType,
               statusToString(tables[i].tableStatus));
    }
}

int findTableIndexByID(const Table *tables, int tableCount, int tableID)
{
    int i;

    for (i = 0; i < tableCount; i++) {
        if (tables[i].tableID == tableID) return i;
    }
    return -1;
}

void sortTableByStatus(Table *tables, int tableCount)
{
    int i, j;
    Table temp;

    for (i = 0; i < tableCount - 1; i++) {
        for (j = 0; j < tableCount - i - 1; j++) {
            if (tables[j].tableStatus > tables[j + 1].tableStatus
                || (tables[j].tableStatus == tables[j + 1].tableStatus
                    && tables[j].tableNumber > tables[j + 1].tableNumber)) {
                temp          = tables[j];
                tables[j]     = tables[j + 1];
                tables[j + 1] = temp;
            }
        }
    }
}

void updateStatusTable(Table *tables, int tableCount)
{
    int tableNumber = 0;
    int newStatus   = 0;
    int index       = -1;
    int i, r;

    if (tableCount == 0) {
        printf("No table in the system\n");
        return;
    }

    sortTableByStatus(tables, tableCount);  
    printTables(tables, tableCount);

    do {
        r = readInt("Enter table number (0 to quit): ", &tableNumber);
        if (r == -1) return FALSE;
        if (r == 0 || tableNumber < 0) {
            printf("Error: please enter a valid number!\n");
            r = 0;
        }
    } while (r == 0);

    if (tableNumber == 0) return;

    for (i = 0; i < tableCount; i++) {
        if (tables[i].tableNumber == tableNumber) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Error: table not found\n");
        return;
    }

    printf("Current status: %d (%s)\n",
           tables[index].tableStatus, statusToString(tables[index].tableStatus));

    do {
        r = readInt("Enter new status (1: Available, 2: Booked, 3: Serving): ", &newStatus);
        if (r == -1) return;
        if (r == 0 || newStatus < 1 || newStatus > 3) {
            printf("Please enter 1 -> 3!\n");
            r = 0;
        }
    } while (r == 0);

    tables[index].tableStatus = newStatus;
    printf("Update table status successfully!\n");
}

int updateStatusAutoTable(Table *tables, int tableCount, int tableID, int event)
{
    if (tables == NULL) {
        return FALSE;
    }

    int index = findTableIndexByID(tables, tableCount, tableID);

    if (index == -1) return FALSE;              

    switch (event) {
        case BOOK:
            if (tables[index].tableStatus == AVAILABLE)
                tables[index].tableStatus = BOOKED;
            else
                return FALSE;
            break;
        case CANCEL:
            if (tables[index].tableStatus == BOOKED)
                tables[index].tableStatus = AVAILABLE;
            else
                return FALSE;
            break;
        case CHECK_IN:
            if (tables[index].tableStatus == BOOKED
                || tables[index].tableStatus == AVAILABLE)
                tables[index].tableStatus = SERVING;
            else
                return FALSE;
            break;
        case PAY_BILL:
            if (tables[index].tableStatus == SERVING)
                tables[index].tableStatus = AVAILABLE;
            else
                return FALSE;
            break;
        default:
            return FALSE;
    }
    if (!allowed){
        return FALSE;
    }
    tables[index].tableStatus = expected;
    return TRUE;                              
}

int findAvailableTable(const Table *tables, int tableCount, int numberCustomer)
{
    int needType;
    int bestIndex = -1;
    int i;

    if (numberCustomer <= 0){
        return -1;
    }     
    else if (numberCustomer <= 2){
        needType = 2;
    }
    else if (numberCustomer <= 4){
        needType = 4;
    } 
    else if (numberCustomer <= 8){
        needType = 8;
    } 
    else{
        return -1; 
    }                          

    for (i = 0; i < tableCount; i++) {
        if (tables[i].tableStatus == AVAILABLE && tables[i].tableType >= needType) {
            if (bestIndex == -1 || tables[i].tableType < tables[bestIndex].tableType) {
                bestIndex = i;
            }
        }
    }

    if (bestIndex == -1){
         return -1; 
    }        
    return tables[bestIndex].tableID;
}