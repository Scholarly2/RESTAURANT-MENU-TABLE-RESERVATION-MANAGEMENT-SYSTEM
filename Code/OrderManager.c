#include <stdio.h>
#include <RestaurantTypes.h>

int createOrderTable(int type, int reservationID, int tableID,int guests, double distance){
    //so luong ban se dat
    int n;
    printf("Enter amount of table: ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("Amount of table is invalid.\n");
        return -1;
    };
    //tao order moi
    for(int i = 0; i < n; i++){
        printf("Enter table ID: ");
        if(scanf("%d", &tableID) != 1 || tableID <= 0){
            printf("table ID is invalid.\n");
            return -1;
        }
        printf("Enter the reservation ID: ");
        if(scanf("%d", &reservationID) != 1 || reservationID <= 0){
            printf("Reservation ID is invalid.\n");
            return -1;
        }
        printf("Enter amount of quests: ");
        if(scanf("%d", &guests) != 1 || guests <= 0){
            printf("So luong khach khong hop le.\n");
            return -1;
        }
        printf("Enter the type of the table (2, 4, 6): ");
        if(scanf("%d", &type) != 1 || (type != 2 && type != 4 && type != 6)){
            printf("Type of table is invalid.\n");
            return -1;
        }
        printf("Enter the distance to the restaurant (km): ");
        if(scanf("%lf", &distance) != 1 || distance < 0){
            printf("The distance is invalid.\n");
            return -1;
        }
    }

    // bat dau tham order moi
    Order *newOrder = &S.orders[S.orderCount];
    newOrder->orderID = nextID(S.lastOrderID);
    newOrder->tableID = tableID;
    newOrder->reservationID = reservationID;
    newOrder->orderType = type;
    newOrder->isPreOrder = 0;
    newOrder->NumberCustomer = guests;
    newOrder->deliveryDistance = distance;
    newOrder->orderStatus = ORDER_OPEN;
    newOrder->itemCount = 0;
    S.orderCount++;
    S.lastOrderID = newOrder->orderID;
    printf("Tao order thanh cong. ID order: %d\n", newOrder->orderID);
    return newOrder->orderID;
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


