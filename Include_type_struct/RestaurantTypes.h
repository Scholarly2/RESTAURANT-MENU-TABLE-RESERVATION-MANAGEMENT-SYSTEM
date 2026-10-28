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
int findProductByID(const State *s, int id);
int generateProductID(const State *s);
int addProduct(void);
int updatePrice(MenuItem *item, double value);
int updateName(MenuItem *item, const char *value);
int updateType(MenuItem *item, int value);
int updateQuantity(MenuItem *item, int value);
int updateStatus(MenuItem *item, int value);
int updateProduct(void); //change input function
int deleteProduct(void);
void searchProductByName(const char *keyword);
void searchProductByType(int type);
void sortByPrice(void);
void sortByName(void);
void printListProduct(const MenuItem *list, int count);
void productMenu(void);
void sortByID(void);
void searchProductByID(int id);

//function table
int findTableByID(const State *s, int id);
int getExpectedTableStatus(const State *s, int tableID);
int updateStatusAutoTable(State *s, int tableID, int event);
int updateStatusTable(int tableID, int newStatus);
int findAvailableTable(const State *s, int numberCustomer);
void showTables(const Table *list, int count);
void sortTableByStatus(void);

//function reservation
int findReservationByID(const State *s, int id);
int createReservation(const char *nameCustomer, const char *phoneNumber,
                      int NumberCustomer, DateTime timeBooking,
                      int tableType, int requestedTableID);
int cancelReservation(const char* phoneNumber);
int checkInReservation(const char* phoneNumber); //change reservationID -> phoneNumber
void sortBookingByTime(void);

//function order va quan li ton kho
int findOrderByID(const State *s, int id);
int findOrderItem(const Order *order, int productID);
int findOpenOrder(const State *s, int tableID, int reservationID);
int appendEmptyOrder(State *s, int type, int reservationID, int tableID,
                     int guests, double distance);
int createOrderTable(int type, int reservationID, int tableID,
                     int guests, double distance);
int heldQuantity(const State *s, int productID);
int changeOrderQuantity(State *s, int orderIndex, int productID, int newQuantity);
int addOrderFood(int orderID, int productID, int quantity);
int updateOrderFood(int orderID, int productID, int newQuantity);
int deleteOrderFood(int orderID, int productID);
void showAllOrderFood(const Order *order);
void showAllOrderTable(void);
void listPreOrderedDishes(int tableID);
int releaseOrderStock(State *s, int orderIndex);
int cancelOrder(int orderID);

//bill
int calculateOrderTotal(const Order *order, double *total);
int calculateServiceCharge(int orderType, double subtotal,
                           ChargePolicy policy, double distance, double *fee);
int applyDineInCharge(double subtotal, double rate, double *fee);
int applyTakeawayCharge(double subtotal, double rate, double *fee);
int applyDeliveryCharge(double distance, double baseFee, double perKm, double *fee);
int configureCharges(ChargePolicy policy);
int calculateBill(const Order *order, ChargePolicy policy,
                  double discountRate, double taxRate, Bill *bill);
int validateBill(const Bill *bill);
int printBillReport(int orderID, double discountRate, double taxRate);
void displayBillReport(const Bill *bill);
int checkoutOrder(int orderID, double discountRate, double taxRate);

#endif

