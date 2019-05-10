#include <stdio.h>
#include <stdlib.h>

int main() {
    char phoneNum[11];
    int i = 0;

    scanf("%s", phoneNum);
    scanf("%d", &i);

    if (i == -1) {
        printf("%s\n", phoneNum);
    } 
    else if (0 <= i && i <= 9){
        printf("%c\n", phoneNum[i]);
    }
    else if (i < -1 || i > 9)
    {
        printf("ERROR\n");
        return 1;
    }
    return 0;
}