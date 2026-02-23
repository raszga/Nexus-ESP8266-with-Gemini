/* Developed with Gemini
           /|\
          --+--
           \|/
2026-02-22 13:41:00
*/

#include "aux_.h"

namespace aux {
const std::set<char> VALID_COMMANDS = {
  'A',  // alarm from the sensor
  'F',  // main fan command
  'D',  // buzzer
  'I',  // internal fan
  'P',  // send pulsar reset
  'R',  // Relay
  'Q',  // freq
  'T',  // time delay
};

char** commandArray = parseCommandsToArray("This is a test *H200*G50*A100");

/*remove chip id for the local sensors****************************************/
string removeSubstring(const string& s, const string& sub) {
  if (sub.empty()) return s;
  string out = s;
  size_t pos = 0;
  while ((pos = out.find(sub, pos)) != string::npos) {
    out.replace(pos, sub.length(), "");
  }
  return out;
}

// Function to parse commands from a string************************************
CommandList parseCommands(const std::string& input) {
  CommandList result;
  for (size_t i = 0; i < input.length(); i++) {
    // Look for '*' character
    if (input[i] == '*' && i + 1 < input.length()) {
      char letter = input[i + 1];
      // Check if letter is valid
      if (VALID_COMMANDS.find(letter) != VALID_COMMANDS.end()) {
        // Extract all consecutive digits after the letter
        std::string numStr = "";
        size_t j = i + 2;
        while (j < input.length() && isdigit(input[j])) {
          numStr += input[j];
          j++;
        }
        // If we found at least one digit, parse it
        if (!numStr.empty()) {
          int value = std::stoi(numStr);
          result.addCommand(letter, value);
          i = j - 1;  // Skip past this command
        }
      }
    }
  }
  return result;
}

// NEW FUNCTION: Convert commands to array of strings ending with nullptr*******
char** commandsToStringArray(const std::string& input) {
  CommandList cmdList = parseCommands(input);
  // Allocate array with space for all commands + nullptr terminator
  char** array = new char*[cmdList.commands.size() + 1];
  // Convert each command to string format "*Xxx"
  for (size_t i = 0; i < cmdList.commands.size(); i++) {
    std::string cmdStr = "*" + std::string(1, cmdList.commands[i].letter) + std::to_string(cmdList.commands[i].value);

    // Allocate and copy string
    array[i] = new char[cmdStr.length() + 1];
    strcpy(array[i], cmdStr.c_str());
  }
  // Null terminator
  array[cmdList.commands.size()] = nullptr;
  return array;
}
// Function to free the string array*******************************************
void freeStringArray(char** array) {
  if (array == nullptr) return;
  for (int i = 0; array[i] != nullptr; i++) {
    delete[] array[i];
  }
  delete[] array;
}

// INTEGRATED FUNCTION: Parse input and return array of strings ending with nullptr
char** parseCommandsToArray(const std::string& input) {
  // Temporary vector to store command strings
  std::vector<std::string> commandStrings;
  // Parse commands directly
  for (size_t i = 0; i < input.length(); i++) {
    // Look for '*' character
    if (input[i] == '*' && i + 1 < input.length()) {
      char letter = input[i + 1];
      // Check if letter is valid
      if (VALID_COMMANDS.find(letter) != VALID_COMMANDS.end()) {
        // Extract all consecutive digits after the letter
        std::string numStr = "";
        size_t j = i + 2;
        while (j < input.length() && isdigit(input[j])) {
          numStr += input[j];
          j++;
        }
        // If we found at least one digit, create command string
        if (!numStr.empty()) {
          std::string cmdStr = "*" + std::string(1, letter) + numStr;
          commandStrings.push_back(cmdStr);
          i = j - 1;  // Skip past this command
        }
      }
    }
  }
  // Allocate array with space for all commands + nullptr terminator
  char** array = new char*[commandStrings.size() + 1];
  // Copy strings to array
  for (size_t i = 0; i < commandStrings.size(); i++) {
    array[i] = new char[commandStrings[i].length() + 1];
    strcpy(array[i], commandStrings[i].c_str());
  }
  // Null terminator
  array[commandStrings.size()] = nullptr;
  return array;
}

// Legacy function that returns just the first command's integer (or -1 if not found)
int getCmdVal(const std::string& input) {
  CommandList commands = parseCommands(input);
  return commands.isEmpty() ? -1 : commands.commands[0].value;
}

// Helper to strip path from whatever string is passed to it*******************
string extractFileName(const char* path) {
  const char* file = strrchr(path, '\\');
  if (!file) {
    file = strrchr(path, '/');
  }
  return (file) ? string(file + 1) : string(path);
}

}  // namespace aux
/******************************************************************************/
/* Developed with Gemini
           /|\
          --+--
           \|/
2026-02-22 13:41:00
********************************************************************************/
void buildMessage(String& output, const char* name, int value, bool isOk) {
  char buffer[64];

  // Determine the suffix based on your enforced V=-1 logic
  const char* alarmStatus = (value < 0) ? "alarmNA" : (isOk ? "alarmOFF" : "alarmON");
  // Single formatting call for cleaner maintenance
  snprintf(buffer, sizeof(buffer), "%s %d %s",
           name,
           value,
           alarmStatus);

  output = buffer;
}