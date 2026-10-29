#ifndef RESTAURANT_MENU_H
#define RESTAURANT_MENU_H

#include <stdio.h>
#include "Input.h"

//thieu ham nao bao voi tao de bo sung
//dung cac ham trong nay de code, dung ten tranh conflict
int runAction(int module, int choice);
int runProductAction(int choice);
int runTableAction(int choice);
int runReservationAction(int choice);
int runOrderAction(int choice);
static inline void showModuleMenu(int module)
{
    int choice;
    int maxChoice;

    if (module < 1 || module > 4) {
        return;
    }

    for (;;) {
        switch (module) {
        case 1:
            puts("\n=== QUAN LY SAN PHAM ===");
            puts("1. Them mon");
            puts("2. Sua mon (ten, gia, loai, so luong, trang thai)");
            puts("3. Xoa mon theo ten");
            puts("4. Tim mon theo ten");
            puts("5. Tim mon theo loai");
            puts("6. Sap xep theo gia");
            puts("7. Sap xep theo ten");
            puts("8. Hien thi thuc don");
            maxChoice = 8;
            break;
        case 2:
            puts("\n=== QUAN LY BAN ===");
            puts("1. Hien thi danh sach ban");
            puts("2. Thay doi trang thai ban");
            puts("3. Tim ban trong theo so khach");
            puts("4. Sap xep ban theo trang thai");
            maxChoice = 4;
            break;
        case 3:
            puts("\n=== QUAN LY DAT BAN ===");
            puts("1. Tao dat ban");
            puts("2. Huy dat ban");
            puts("3. Hien thi dat ban theo thoi gian");
            puts("4. Xac nhan khach nhan ban");
            maxChoice = 4;
            break;
        default: // module == 4
            puts("\n=== DON HANG VA HOA DON ===");
            puts("1. Tao hoac chon don hang");
            puts("2. Them mon vao don");
            puts("3. Sua so luong mon trong don");
            puts("4. Xoa mon khoi don");
            puts("5. Xem mon dat truoc theo ban");
            puts("6. Xem hoa don tam tinh");
            puts("7. Thanh toan va xuat hoa don");
            puts("8. Huy don hang");
            puts("9. Xem danh sach/lich su don hang");
            puts("10. Cai dat phu phi");
            puts("11. Xem hoa don da thanh toan");
            maxChoice = 11;
            break;
        }

        puts("0. Quay lai");
        if (!readInt("Lua chon: ", 0, maxChoice, &choice)) {
            return;
        }
        if (choice == 0) {
            return;
        }
        if (!runAction(module, choice)) {
            return;
        }
    }
}

static inline void showMainMenu(void)
{
    int choice;

    for (;;) {
        puts("\n=== QUAN LY NHA HANG ===");
        puts("1. Quan ly san pham");
        puts("2. Quan ly ban");
        puts("3. Quan ly dat ban");
        puts("4. Don hang va hoa don");
        puts("0. Thoat");

        if (!readInt("Lua chon: ", 0, 4, &choice)) {
            return;
        }
        if (choice == 0) {
            puts("Tam biet!");
            return;
        }
        showModuleMenu(choice);
    }
}

#endif

