# SerialCommandHandler

## What is it for?

This library is meant to simplify connecting serial string 
commands to underlying functions in your library without requiring
those functions to know anything about the serial interface. 

Given commands in the serial monitor of the form
 
`commandName arg1 arg2 ... arg-n`
 
the SerialCommandHandler will parse the string into a `commandName` and an array
of string arguments. If the string name matches an entry in the command dictionary
you've set up, it will call the associated dictonary "glue" function.

## What is a "Glue" function?

These are interface functions that translate the string arguments provided from the
command handler parser into actual values to be passed to your underlying functions.

## How do I use it?

### Write the Glue functions

In order to be saved into a single command dictionary, all "glue" function must take
the following form:

`CMD_ERROR myFunction(String args[], int arg_count)`

`CMD_ERROR` is an enum value that allows the function to return one of
the error values defined in SerialCommandHandler.h. This allows for
upstream error handling. Current error codes include those listed below, 
but double check the `SerialCommandHandler.h` for any potential updates that
have outpaced the documentation here. :-)

```cpp
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
```

`args[]` is an array of arguments passed as String objects that must
be converted to the appropriate type before being passed to your underlying function.

`arg_count` is the number of arguments that are being passed in the array.
This is used for error checking. The glue function could, for instance, return
an error code if the number of arguments passed does not match the number required
in the underlying function.

Within the glue function, you will do your error checking on the string argument,
covert them to the appropriate types, call the underlying function, and include
any other functionality that is useful specifically to a serial command.

Below is an example of an underlying function and a glue function.

```cpp

/****************************
 *    Underlying Function   *
 ****************************/
 
void blinkLED(int blink_count, int blink_millis) {
  for(int i = 0; i < blink_count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(blink_millis / 2);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blink_millis / 2);
  }
}


/****************************
 *       Glue Function      *
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
  Serial.println(String("Blinking the LED ") + String(count) + String(" times over a total of ") + 
    String(count * time) + String(" milliseconds."));

  // Call the underlying function
  blinkLED(count, time);

  // Remember to return an error code
  return ERR_NONE;
}
```

### Create the Command Dictionary

Once you have created your glue functions, you need to created a command dictionary,
which is simply an array of `Command` objects that the parser will reference. The
`Command` constructor takes two arguments: `Command(<string name>, <function pointer>)`.
The string name is the command you will type into the serial monitor. The function pointer
is simply the name of the glue function you wish to call with that text command. Shown below,
we set up a dictionary for two glue functions and bind them to the parser.

```cpp
// The dictionary
const Command commandDictionary[] = {
  {String("blink"), blinkLedCmd},
  {String("set"), setLedCmd}
};

// Number of commands in the dictionary
int command_count = sizeof(commandDictionary) / sizeof(Command);

// Binding the dictionary to the parser
Command::setCommandDictionary(commandDictionary, command_count);
```

### Run the Handler

Somewhere were it will be run frequently (probably in your main loop), put
this bit of code (having remembered to `#include "SerialCommandHandler.h"` at the top of your file):

```cpp
CMD_ERROR error = Command::runHandler();

if(error != ERR_NONE) {
  Command::printError(error);
}
```

The handler will check the serial port for incoing commands, parse them, attempt to run any
bound glue functions, and then print an error message if an error code is returned.

### Use the Commands

Once you've done the above setup, you can simply type the command and arguments (separated
by spaces by default, though if you dig into the libary, you can find how to change the delmiter)
and watch your commands execute.

At any time you may also type the built in command `help` into the serial monitor
to print a list of all the commands in your currently configured command dictionary. This command
is reserved and will preempt any command in your dictionary by the same name, so don't use it.
