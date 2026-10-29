#ifndef RESTAURANT_TYPES_H
#define RESTAURANT_TYPES_H

//hang so can dung
#define TRUE 1
#define FALSE 0
#define MAX_PRODUCT 100
#define MAX_TABLES 100
#define MAX_RESERVATIONS 1000
#define MAX_ORDERS 1000
#define MAX_BILLS 1000
#define MAX_ITEMS 100
#define NAME_SIZE 100
#define CUSTOMER_NAME_SIZE 50
#define PHONE_SIZE 15
#define MAX_QUANTITY 1000
#define MAX_PRICE 1000000.0
#define MAX_MONEY 1000000000000.0
#define MAX_DISTANCE 1000.0
#define CHECKIN_EARLY_MIN 30

//enum
enum ProductUPDATE
{
    UP_Name = 1,
    UP_PRICE,
    UP_TYPE,
    UP_QUANTITY,
    UP_STATUS
};
enum ProductType 
{
    STARTER = 1, MAINDISH, SIDEDISH, DESSERT, DRINK
};
enum ProductStatus { PRODUCT_AVAILABLE = 0, PRODUCT_UNAVAILABLE };
enum TableStatus { TABLE_AVAILABLE = 0, TABLE_BOOKED, TABLE_SERVING };
enum OrderType { DINE_IN = 1, TAKEAWAY, DELIVERY };
enum ReservationStatus { WAITING = 0, COMPLETED, CANCELLED };
enum OrderStatus { ORDER_OPEN = 0, ORDER_PAID, ORDER_CANCELLED };
enum TableEvent { BOOK = 1, CANCEL, CHECK_IN, PAY_BILL, CANCEL_ORDER };

//time booking and bill
typedef struct {
    int year, month, day, hour, minute;
} DateTime;

//struct product
typedef struct {
    int productID;
    char productName[NAME_SIZE];
    double productPrice;
    enum ProductType productType;
    enum ProductStatus productStatus;
    int quantity;
} MenuItem;

//struct table
typedef struct {
    int tableID;
    int tableNumber;
    int tableType;
    int tableTypeID;
    enum TableStatus tableStatus;
} Table;

//struct reservation
typedef struct {
    int reservationID;
    char nameCustomer[CUSTOMER_NAME_SIZE];
    char phoneNumber[PHONE_SIZE];
    int NumberCustomer;
    DateTime timeBooking;
    int tableType;
    int tableID;
    enum ReservationStatus reservationStatus;
} Reservation;

//struct order product
typedef struct {
    int productID;
    char productName[NAME_SIZE];
    enum ProductType productType;
    double productPrice;
    int quantity;
} OrderItem;

//struct order
typedef struct {
    int orderID, tableID, reservationID;
    enum OrderType orderType;
    int isPreOrder;
    int NumberCustomer;
    double deliveryDistance;
    enum OrderStatus orderStatus;
    int itemCount;
    OrderItem items[MAX_ITEMS];
} Order;

//struct phu phi
typedef struct {
    double dineInRate, takeawayRate;
    double deliveryBase, deliveryPerKm;
} ChargePolicy;

//struct bill
typedef struct {
    int billID, orderID, tableID, staffID;
    enum OrderType orderType;
    DateTime timeBill;
    int itemCount;
    OrderItem items[MAX_ITEMS];
    double totalMoney;
    double discountRate, discount;
    double taxRate, tax;
    double surcharge;
    double amountDue;
    ChargePolicy policySnapshot;
    double deliveryDistance;
} Bill;

//bo nho
typedef struct {
    MenuItem products[MAX_PRODUCT];
    Table tables[MAX_TABLES];
    Reservation reservations[MAX_RESERVATIONS];
    Order orders[MAX_ORDERS];
    Bill bills[MAX_BILLS];
    int productCount, tableCount, resCount;
    int orderCount, billCount;
    int lastProductID, lastResID, lastOrderID, lastBillID;
    ChargePolicy policy;
} State;


extern State S;

//phụ
int nextID(int lastID);
double roundMoney(double value);
int initializeState(State *s);
State *copyState(const State *source);
int validateState(const State *s);

int commitChange(State *candidate, const char *actionType, int targetID,
                 const char *oldValue, const char *newValue);


//function pproduct
int findProductByID(const State *s, int id); // Done
int generateProductID(const State *s); //Done
int addProduct(void); //Done
int updatePrice(MenuItem *item, double value); //Done
int updateName(MenuItem *item, const char *value); //Done
int updateType(MenuItem *item, int value); //done
int updateQuantity(MenuItem *item, int value); //done
int updateStatus(MenuItem *item, int value); //done
int updateProduct(void); //change input function //done
int deleteProduct(void); //done
void searchProductByName(const char *keyword); //done 
void searchProductByType(int type); //done
void sortByPrice(void); //done
void sortByName(void); //done
void printListProduct(const MenuItem *list, int count); //done
void productMenu(void); //done
void sortByID(void); //done
void searchProductByID(int id); //done

//function table
int findTableByID(const State *s, int id); //done
int getExpectedTableStatus(const State *s, int tableID); //done
int updateStatusAutoTable(State *s, int tableID, int event); //done
int updateStatusTable(int tableID, int newStatus); //done
int findAvailableTable(const State *s, int numberCustomer); //done  
void showTables(const Table *list, int count); //done
void sortTableByStatus(void); //done

//function reservation
int findReservationByID(const State *s, int id); //done
int createReservation(const char *nameCustomer, const char *phoneNumber,
                      int NumberCustomer, DateTime timeBooking,
                      int tableType, int requestedTableID); //done
int cancelReservation(const char* phoneNumber); //done 
int checkInReservation(const char* phoneNumber); //change reservationID -> phoneNumber //done
void sortBookingByTime(void); //done

//function order va quan li ton kho
int findOrderByID(const State *s, int id); // not done
int findOrderItem(const Order *order, int productID); // not done
int findOpenOrder(const State *s, int tableID, int reservationID); // not done
int createOrderTable(int type, int reservationID, int tableID, int guests, double distance); //done
int changeOrderQuantity(State *s, int orderIndex, int productID, int newQuantity); // done
int addOrderFood(int orderID, int productID, int quantity); // done
int updateOrderFood(int orderID, int productID, int newQuantity); //done
int deleteOrderFood(int orderID, int productID); //done
void showAllOrderFood(const Order *order); //not done
void showAllOrderTable(void); // not done
void listPreOrderedDishes(int tableID); //done
int releaseOrderStock(State *s, int orderIndex); // not done
int cancelOrder(int orderID); // not done

//bill
int calculateOrderTotal(const Order *order, double *total); // not done
int calculateServiceCharge(int orderType, double subtotal,
                           ChargePolicy policy, double distance, double *fee); //done
int applyDineInCharge(double subtotal, double rate, double *fee); //done
int applyTakeawayCharge(double subtotal, double rate, double *fee); //done
int applyDeliveryCharge(double distance, double baseFee, double perKm, double *fee); //done
int configureCharges(ChargePolicy policy); //not done
int calculateBill(const Order *order, ChargePolicy policy,
                  double discountRate, double taxRate, Bill *bill); //not done
int validateBill(const Bill *bill); //not done
int printBillReport(int orderID, double discountRate, double taxRate); // in processing
int checkoutOrder(int orderID, double discountRate, double taxRate); //not done

#endif