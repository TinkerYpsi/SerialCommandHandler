#ifndef B7C23FA6_CEE3_4D88_AD94_916FFCFAE4A2
#define B7C23FA6_CEE3_4D88_AD94_916FFCFAE4A2

#include <Arduino.h>

typedef enum _cmd_error {
  ERR_NONE,
  ERR_MISSING_START_CHAR,
  ERR_MISSING_STOP_CHAR,
  ERR_MISSING_DELIMITER,  
  ERR_INVALID_CMD,
  ERR_NO_MATCHING_CMD,  
  ERR_TOO_FEW_ARGS,
  ERR_TOO_MANY_ARGS,  
  ERR_NO_ARGS,
  ERR_INVALID_ARG
} CMD_ERROR;

class Command {

  public:

    // Parses raw command strings, checks against the command dictionary, and executes if possible
    static void setCommandDictionary(const Command* dictionary, int command_count);
    static CMD_ERROR process(String cmd_str);
    static CMD_ERROR runHandler();
    

    static void setStartChar(char start_char);
    static void setStopChar(char stop_char);
    static void setDelimiter(char delimiter);

    static void printError(CMD_ERROR error);

    // Constructor
    Command(String cmd_name, CMD_ERROR (*callback)(String* args, int arg_count));

  private:
  
    static const int MAX_ARGS = 10;
    static char start_char;
    static char stop_char;
    static char delimiter;
    static const Command* command_dictionary;
    static int command_count;

    static CMD_ERROR splitArgString(String arg_buff[], String arg_str, int* arg_count_buff);

    String cmd_name;
    CMD_ERROR (*callback)(String* args, int arg_count);
};

#endif /* B7C23FA6_CEE3_4D88_AD94_916FFCFAE4A2 */
