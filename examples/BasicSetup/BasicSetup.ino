/**
 * Author: Michael G. Ploof
 * Date: 12-3-2020
 * 
 * This example shows the basic functionality of the SerialCommandHandler
 * library. The library is meant to simplify connecting serial string 
 * commands to underlying functions in your library without requiring
 * those functions to know anything about the serial interface. 
 * 
 * All serial monitor commands must stake the following form:
 * 
 * "commandName arg1 arg2 ... arg-n"
 * 
 * The SerialCommandHandler will parse the string from the serial terminal
 * and compare the commandName to a library of commands you have created. If
 * the string name matches an entry in your library, it will call the associated
 * "glue" function.
 * 
 * "Glue" functions are essentially an interface between your underlying functions
 * and the command parser that all follow the same basic format
 * 
 * CMD_ERROR myFunction(String args[], int arg_count);
 * 
 * CMD_ERROR is an enum value that allows the function to return one of
 * the error values defined in SerialCommandHandler.hpp. This allows for
 * upstream error handling.
 * 
 * args[] is an array of arguments passed as String objects that must
 * be converted to the appropriate type before being passed to your underlying function.
 * 
 * arg_count is the number of arguments that are being passed in the array.
 * This is used for error checking. The glue function could, for instance, return
 * an error code if the number of arguments passed does not match the number required
 * in the underlying function.
 * 
 * Within the glue function, you will do your error checking on the string argument,
 * covert them to the appropriate types, call the underlying function, and include
 * any other functionality that is useful specifically to a serial command.
 * 
 * 
 * At any time you may type the built in command "help" into the serial monitor
 * to print a list of all the commands in your currently configured command dictionary.
 **/

#include "SerialCommandHandler.h"

/****************************
 *   Underlying Functions   *
 ****************************/
void blinkLED(int blink_count, int blink_millis) {
  for(int i = 0; i < blink_count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(blink_millis / 2);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blink_millis / 2);
  }
}

void setLED(bool is_on) {
  digitalWrite(LED_BUILTIN, is_on);
}


/****************************
 *      Glue Functions      *
 ****************************/

CMD_ERROR blinkLedCmd(String args[], int arg_count) {

  // Make sure there's the correct number of arguments being passed
  const int PARAM_COUNT = 2;
  if (arg_count < PARAM_COUNT) {
    return ERR_TOO_FEW_ARGS;
  }
  else if (arg_count > PARAM_COUNT) {
    return ERR_TOO_MANY_ARGS;
  }

  // Convert the string args to values
  int count = args[0].toInt();
  int time = args[1].toInt();

  // Additional code useful for serial interfacing
  Serial.println(String("Blinking the LED ") + String(count) + String(" times over a total of ") + String(count * time) + String(" milliseconds."));

  // Call the underlying function
  blinkLED(count, time);

  // Remember to return an error code
  return ERR_NONE;
}


CMD_ERROR setLedCmd(String args[], int arg_count) {

  // Make sure there's the correct number of arguments being passed
  const int PARAM_COUNT = 1;
  if (arg_count < PARAM_COUNT) {
    return ERR_TOO_FEW_ARGS;
  }
  else if (arg_count > PARAM_COUNT) {
    return ERR_TOO_MANY_ARGS;
  }

  // In this case, we're not coverting the string arg directly, but comparing it to some known values.
  // This also allows us to generate an error code if there's a bad input
  args[0].toLowerCase();  // To ensure consistency, force the argument to lowercase
  if(args[0] != "on" && args[0] != "off") {
    return ERR_INVALID_ARG;
  }

  // Generate the argument to pass to the underlying function
  bool is_on;
  if(args[0] == "on") {
    is_on = true;
  }
  else {
    is_on = false;
  }

  // Additional code useful for serial interfacing
  Serial.println(String("Setting the LED ") + String(args[0]));

  // Call the underlying function
  setLED(is_on);

  // Remember to return an error code
  return ERR_NONE;
}


/******************************
 *     Command Dictionary     *
 ******************************/
const int command_count = 2;
const Command commandDictionary[] = {
  {String("blink"), blinkLedCmd},
  {String("set"), setLedCmd}
};


/****************************
 *      Core Functions      *
 ****************************/
void setup() {
  // Initialize the led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Connect the command dictionary to the command handler 
  Command::setCommandDictionary(commandDictionary, command_count);

  Serial.begin(115200);
  Serial.println("Ready for commands!");
}

void loop() {
  // Run the command handler. This automatically checks the serial port
  // for input, parses it, then runs the associated glue function if the
  // command exists in the dictionary.
  CMD_ERROR error = Command::runHandler();
  if(error != ERR_NONE) {
    Command::printError(error);
  }
}