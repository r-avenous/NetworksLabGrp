#include <stdio.h> 
#include <string.h> 
    
int main() { 
    char filename[100] = "./Hello.txt"; 
    FILE *fp; 
    char c; 
    
    // open the file 
    fp = fopen(filename, "r"); 
    if (fp == NULL) {
        printf("Error opening file\n"); 
        return 0;
    } 
    
    // read character by character and print them 
    while ((c = fgetc(fp)) != EOF) {
        printf("%c", c); 
    }
    
    fclose(fp); 
    
    return 0; 
} 
