#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// can be sent as single char
unsigned char int_to_hex(int n){
    unsigned char ch, hex[5];
    sprintf(hex, "%x", n);
    printf("%s\n", hex);
    sscanf(hex, "%hhx", &ch);
    return ch; 
}

int main(){
    while(1){
        int n;
        scanf("%d", &n);
        unsigned char ch = int_to_hex(n);
        printf("%c\n", ch);
        int m = ch;
        printf("%d\n-----\n\n", m);

    }
    return 0;
}