#include <stdio.h>

int main(void)
{
  int n, i, b[32],dif;
  printf("数を入力してください->");
  scanf("%d", &n);
  for (i = 0; n > 0; i++) {
    b[i] = n % 2;
    n = n / 2;
  }
  dif=32-i;
  while (i > 0){
    while(dif>0){
      printf("0");
      dif--;
    }
    printf("%d",  b[--i]);
  }
  putchar('\n');
}
