#include <stdio.h>
#include <RestaurantTypes.h>

int createOrderTable(int type, int reservationID, int tableID, int guests, double distance){

    // so luong order muon tao
    int n;
    printf("Enter amount of order: ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("Amount of order is invalid.\n");
        return -1;
    }

    // tao order moi
    for(int i = 0; i < n; i++){

        printf("Enter customer type (DINE_IN / TAKEAWAY / DELIVERY): ");
        if(scanf("%d", &type) != 1 || type < DINE_IN || type > DELIVERY){
            printf("Customer type is invalid.\n");
            return -1;
        }

        // DINE IN
        if(type == DINE_IN){

            printf("Enter reservation ID (0 if no reservation): ");
            if(scanf("%d", &reservationID) != 1 || reservationID < 0){
                printf("Reservation ID is invalid.\n");
                return -1;
            }

            if(reservationID > 0){

                int reservationIndex = findReservationByID(&S, reservationID);

                if(reservationIndex == -1 ||
                   S.reservations[reservationIndex].reservationStatus != WAITING){
                    printf("Reservation is invalid.\n");
                    return -1;
                }

                tableID = S.reservations[reservationIndex].tableID;
                guests = S.reservations[reservationIndex].NumberCustomer;

                if(findOpenOrder(&S, 0, reservationID) != -1){
                    printf("There is already an open order for this reservation.\n");
                    return -1;
                }
            }
            else{

                printf("Enter table ID: ");
                if(scanf("%d", &tableID) != 1){
                    printf("Table ID is invalid.\n");
                    return -1;
                }

                printf("Enter number of guests: ");
                if(scanf("%d", &guests) != 1 || guests < 1 || guests > 8){
                    printf("Number of guests is invalid.\n");
                    return -1;
                }

                int tableIndex = findTableByID(&S, tableID);

                if(tableIndex == -1){
                    printf("Table is invalid.\n");
                    return -1;
                }

                if(S.tables[tableIndex].tableStatus == TABLE_SERVING){

                    int openOrderIndex = findOpenOrder(&S, tableID, 0);

                    if(openOrderIndex == -1 || S.orders[openOrderIndex].isPreOrder){
                        printf("Cannot use existing order.\n");
                        return -1;
                    }

                    printf("Use existing order %d.\n",
                           S.orders[openOrderIndex].orderID);

                    return S.orders[openOrderIndex].orderID;
                }

                if(S.tables[tableIndex].tableStatus != TABLE_AVAILABLE){
                    printf("Table is not available.\n");
                    return -1;
                }

                if(guests > S.tables[tableIndex].tableType){
                    printf("Number of guests exceeds table capacity.\n");
                    return -1;
                }
            }

            distance = 0;
        }

        // TAKEAWAY / DELIVERY
        else{

            reservationID = 0;
            tableID = 0;
            guests = 0;

            printf("Enter delivery distance: ");
            if(scanf("%lf", &distance) != 1 ||
               !isfinite(distance) ||
               distance < 0 ||
               distance > MAX_DISTANCE){
                printf("Distance is invalid.\n");
                return -1;
            }

            if(type == TAKEAWAY && distance != 0){
                printf("Takeaway distance must be 0.\n");
                return -1;
            }
        }

        // copy state
        State *c = copyState(&S);

        if(c == NULL){
            printf("Cannot copy state.\n");
            return -1;
        }

        // tao order trong state copy
        int orderIndex = appendEmptyOrder(
            c, type, reservationID, tableID, guests, distance
        );

        if(orderIndex == -1){
            free(c);
            printf("Cannot create order.\n");
            return -1;
        }

        // cap nhat trang thai ban
        if(type == DINE_IN && reservationID == 0){

            if(!updateStatusAutoTable(c, tableID, CHECK_IN)){
                free(c);
                printf("Cannot update table status.\n");
                return -1;
            }
        }

        // luu thay doi
        int newOrderID = c->orders[orderIndex].orderID;

        if(!commitChange(c, "CREATE_ORDER", newOrderID, "-", "OPEN")){
            printf("Cannot save order.\n");
            return -1;
        }

        printf("Create order successfully. Order ID: %d\n", newOrderID);
    }
    return 0;
}

int addOrderFood(int orderID, int productID, int quantity){

    // di tim order bang ID
    int orderIndex = findOrderByID(&S, orderID);
    if(orderIndex == -1 || orderIndex >= S.orderCount || S.orders[orderIndex].orderStatus != ORDER_OPEN){
        printf("Order is invalid.\n");
        return -1;
    }

    // neu tim thay thi in ra listProduct -> roi nhap ID mon an trong List
    printlistProduct(S.products, S.productCount);
    printf("Enter the product ID: ");
    if(scanf("%d", &productID) != 1 || productID <= 0){
        printf("The product ID is invalid.\n");
        return -1;
    }
    int productID = findProductByID(&S, productID);
    if(productID < 0 || productID >= S.productCount || S.products[productID].productStatus == PRODUCT_UNAVAILABLE){
        printf("The produc does not exist and is no longer in stock.\n");
        return -1;
    }

    // nhap so luong mon an muon them vao 
    printf("Enter the number of quantity: ");
    if(scanf("%d", &quantity) < 1 || quantity > MAX_QUANTITY){
        printf("Quantity is invaild.\n");
        return -1;
    }
    int itemIndex = findOrderItem(&S.orders[orderIndex], productID);
    int oldQuanity = 0;
    if (itemIndex >= 0 ) {
        int currentQuantity = S.orders[orderIndex].items[itemIndex].quantity;
    }
    else if(itemIndex == 1){
        printf("Invaild\n.");
    }
    

    printf("Add order food is successfull.\n");
    return 0;
}


