// Online C compiler to run C program online
#include <stdio.h>

#define LIMIT 99
#define DOOR_DELAY 3

int main() {
  int millis, pin_second;
  while (1) {
    printf("-------------------------------------------------\n");
    printf("number: ");
    scanf("%d", &millis);
    if (((millis % 100) + DOOR_DELAY) > LIMIT) {
      pin_second = (DOOR_DELAY - 1) - (LIMIT - (millis % 100));
    } else {
      pin_second = (millis % 100) + DOOR_DELAY;
    }
    printf("pin set to : %d\n", pin_second);
    printf("-------------------------------------------------\n");
    while (1) {
      int millies_2 = 0;
      printf("input to test : ");
      scanf("%d", &millies_2);
      if ((millies_2 >= pin_second) &&
          ((millies_2 + (DOOR_DELAY + 2)) < LIMIT)) {
        printf("correct\n");
        break;
      } else {
        printf("wrong\n");
      }
    }
  }
  return 0;
}