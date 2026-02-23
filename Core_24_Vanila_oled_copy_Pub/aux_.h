/*
Board Label	GPIO Number	Function	Boot State	Recommendation
D0	GPIO 16	Wake / User	High	    OK (No PWM/Interrupts)
D1	GPIO 5	SCL (I2C)	Float	    BEST (General Purpose)
D2	GPIO 4	SDA (I2C)	Float	    BEST (General Purpose)
D3	GPIO 0	Flash Mode	Pull-UP	    CAUTION (Must be HIGH on boot)
D4	GPIO 2	TXD1 / LED	Pull-UP	    CAUTION (Must be HIGH on boot)
D5	GPIO 14	SCK (SPI)	Float	    GOOD (General Purpose)
D6	GPIO 12	MISO (SPI)	Float	    GOOD (General Purpose)
D7	GPIO 13	MOSI (SPI)	Float	    GOOD (General Purpose)
D8	GPIO 15	CS (SPI)	Pull-DOWN	CAUTION (Must be LOW on boot)
RX	GPIO 3	Serial RX	High	    Avoid (Used for USB Serial)
TX	GPIO 1	Serial TX	High	    Avoid (Used for USB Serial)
A0	ADC0	Analog Input	        N/A	OK (Max 3.3V on NodeMCU)
*/

/* Developed with Gemini
           /|\
          --+--
           \|/
2026-02-22 13:41:00
*/

#ifndef AUX__H
#define AUX__H
#define sgn(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#include <Arduino.h>
#include <string>
#include <iostream>
#include <cstdio>   // Required for sscanf
#include <cstring>  // Required for strstr
#include <vector>
#include <set>
#include <cctype>
using std::string;

namespace aux {
  // Structure to hold a single command********************************************
struct Command {
    char letter;
    int value;

    Command(char l, int v) : letter(l), value(v) {}
};
// Structure to hold all parsed commands*****************************************
struct CommandList {
    std::vector<Command> commands;
    void addCommand(char letter, int value) {
        commands.push_back(Command(letter, value));
    }
    bool isEmpty() const {
        return commands.empty();
    }
    void print() const {
        for (const auto& cmd : commands) {
            std::cout << "Command: *" << cmd.letter << " Value: " << cmd.value << std::endl;
        }
    }
};

extern const std::set<char> VALID_COMMANDS;
extern char** commandArray;
string removeSubstring(const string& s, const string& sub);
CommandList parseCommands(const std::string& input);
char** commandsToStringArray(const std::string& input);
void freeStringArray(char** array);
char** parseCommandsToArray(const std::string& input);
int getCmdVal(const std::string& input);

// Logic to extract filename from a path string
string extractFileName(const char* path);

// MACROS: These must be macros so __FILE__ is evaluated in the calling file (.ino)
#define getFileName() aux::extractFileName(__FILE__)
#define getBuildInfo() (aux::extractFileName(__FILE__) + " (" + __DATE__ + " " + __TIME__ + ")")

}

void buildMessage(String& output, const char* name, int value, bool isOk);
#endif  // AUX__H