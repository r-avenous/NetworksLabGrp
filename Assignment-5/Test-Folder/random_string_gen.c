#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char* generate_random_string(int length) {
    char* str = malloc((length + 1) * sizeof(char));
    strcpy(str, "\n\nSTART:-\n\n");
    if (str == NULL) {
        printf("Error: unable to allocate memory\n");
        exit(1);
    }
    
    // Set random seed
    srand(time(NULL));
    
    for (int i = 11; i < length-12; i++) {
        // Generate a random ASCII character
        str[i] = (char) ((rand() % 127)+1);
    }
    
    // Add null terminator
    strcpy(str+length-12, "\n\nEND\n\n");
    str[length] = '\0';
    
    return str;
}

int main() {
    char* big_random_string = generate_random_string(300);
    printf("%s", big_random_string);
    free(big_random_string);
    
    return 0;
}
