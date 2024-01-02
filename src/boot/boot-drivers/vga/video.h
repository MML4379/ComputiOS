#ifndef VIDEO_H
#define VIDEO_H

// Function to initialize the video driver
void initializeVideoDriver();

// Function to clear the screen
void clearScreen();

// Function to write a character to the screen
void writeCharacter(char character);

// Print a string to the display
void writeString(const char* string);

// Print an integer to the display
void writeInteger(int integer);

#endif // VIDEO_H