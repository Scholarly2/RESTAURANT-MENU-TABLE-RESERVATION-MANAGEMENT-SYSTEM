void showMenuReservation(void) {
    int choice;
    char phoneNumber[PHONE_SIZE];
    while (TRUE) {
        printf("\n===== RESERVATION MENU =====\n");
        printf("1. Create Reservation\n");
        printf("2. Cancel Reservation\n");
        printf("3. Print Reservations\n");
        printf("0. Exit\n");

        if (!readInt("Enter choice: ", 0, 3, &choice)) {
            return;
        }

        switch (choice) {
            case 1:
                if (inputReservation()) {
                    printf("Reservation created successfully!\n");

                    printReservation(
                        &S.reservations[S.resCount - 1], 1
                    );
                }
                else {
                    printf("Failed to create reservation!\n");
                }
                break;

            case 2:
                if (!readPhone(phoneNumber)) {
                    return;
                }

                if (cancelReservation(phoneNumber)) {
                    printf("Reservation cancelled successfully!\n");
                }
                else {
                    printf("Failed to cancel reservation!\n");
                }
                break;

            case 3:
                printReservation(S.reservations, S.resCount);
                break;

            case 0:
                printf("Exit Reservation Menu.\n");
                return;
        }
    }
}
