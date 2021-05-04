//
// Created by Hammad Khan Musakhel on 2/28/21.
//
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// Definitions
#define STDOUT 1
#define MAX_RALPHA 36

// Contant(s)
const char alphanumeric[MAX_RALPHA] = { 'a', 'b', 'c', 'd', 'e', 'f','g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u','v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

// Global Variables
int M;

// Main function
int main(int given, char* argv[]) {
    if (given > 1) {
        M = atoi(argv[1]); //the value of M to be used in the procedure
    }
    srand(time(NULL)); //for random generation

    int written = 0;
    for(int i = 0; i < M; i++){
        char character = alphanumeric[rand() % MAX_RALPHA];
        write(STDOUT, &character, 1); //to the screen
    }
    return 0;
}