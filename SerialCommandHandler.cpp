/**
 Copyright (c) 2020 Michael G. Ploof

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**/

#include <Arduino.h>
#include "SerialCommandHandler.h"


char Command::start_char = '\0';
char Command::stop_char ='\0';
char Command::delimiter = ' ';
const Command* Command::command_dictionary;
int Command::command_count;

CMD_ERROR Command::process(String cmd_str) {

  cmd_str.trim();

  // Verify the start and stop characters, if present
  if(Command::start_char != '\0') {
    if(cmd_str[0] != Command::start_char) {
      return ERR_MISSING_START_CHAR;
    }
    else{
      // Remove the start character
      cmd_str = cmd_str.substring(1);
    }
  }

  if(Command::stop_char != '\0') {
    if(cmd_str[cmd_str.length() - 1] != Command::stop_char) {
      return ERR_MISSING_STOP_CHAR;
    }
    else{
      // Remove the stop character
      cmd_str = cmd_str.substring(0, cmd_str.length() - 2);
    }
  }

  // Separate the command name from the arguments if there are any
  String cmd_name;
  String arg_str;
  String args[MAX_ARGS];
  int arg_count = 0;
  int end_of_command_name_index = cmd_str.indexOf(Command::delimiter);
  if(end_of_command_name_index == -1) {
    cmd_name = cmd_str;
  }
  else {
    cmd_name = cmd_str.substring(0, end_of_command_name_index);
    arg_str = cmd_str.substring(end_of_command_name_index + 1);
    arg_str.trim();
  }

  if(arg_str.length()) {
    CMD_ERROR error = Command::splitArgString(args, arg_str, &arg_count);
    if(error != ERR_NONE) {
      return error;
    }
  }

  // Check the command name against the registered library  
  for(int i = 0; i < Command::command_count; i++) {    
    if(cmd_name.equals(Command::command_dictionary[i].cmd_name)) {     
      return Command::command_dictionary[i].callback(args, arg_count);     
    }
  }
  return ERR_NO_MATCHING_CMD;
}

CMD_ERROR Command::splitArgString(String arg_buff[], String arg_str, int* arg_count_buff) {
  
  // Make sure there's no whitespace screwing us up and that the string actually has content
  arg_str.trim();  
  if(arg_str.length() < 1) {
    return ERR_NO_ARGS;
  }

  // Count the delimiters
  int delimiter_count = 0;
  for(unsigned int i = 0; i < arg_str.length(); i++) {
    if(arg_str[i] == Command::delimiter) {
      delimiter_count++;
    }
  }
  *arg_count_buff = delimiter_count + 1;

  // Split the arg_str
  for(int i = 0; i <= delimiter_count ; i++) {
    String this_arg;
    if(i < delimiter_count) {
      int delimiter_index = arg_str.indexOf(Command::delimiter);
      this_arg = arg_str.substring(0, delimiter_index);
      arg_str = arg_str.substring(delimiter_index + 1);
    }
    // When there are no more delimiters left, the remaining arg_str is just the last argument
    else {
      this_arg = arg_str;
    }
    arg_buff[i] = this_arg;
  }
  return ERR_NONE;
}

Command::Command(String cmd_name, CMD_ERROR (*callback)(String args[], int arg_count)) {
  this->cmd_name = cmd_name;
  this->callback = callback;
}

void Command::setCommandDictionary(const Command dictionary[], int command_count) {
  Command::command_dictionary = dictionary;
  Command::command_count = command_count;
}

CMD_ERROR Command::runHandler() {
  if(Serial.available()) {
    Serial.setTimeout(100);
    String cmd_str = Serial.readString();
    cmd_str.trim();
    Serial.println();
    
    if(cmd_str == "help") {
      Serial.println("Command dictionary:");
      for(int i = 0; i < Command::command_count; i++) {
        Serial.println(Command::command_dictionary[i].cmd_name);
      }
    }
    else {
      Serial.println(cmd_str);
      return Command::process(cmd_str);
    }
  }
  return ERR_NONE;
}

void Command::printError(CMD_ERROR error) {
  String output;
  switch(error) {
    case ERR_NONE:
      return;
      break;    
    case ERR_MISSING_START_CHAR:
      output = String(F("Missing start character"));
      break;
    case ERR_MISSING_STOP_CHAR:
      output = String(F("Missing stop character"));
      break;
    case ERR_MISSING_DELIMITER:
      output = String(F("Missing delimiter character"));
      break;
    case ERR_INVALID_CMD:
      output = String(F("Invalid command"));
      break;
    case ERR_NO_MATCHING_CMD:
      output = String(F("No matching command"));
      break;
    case ERR_TOO_FEW_ARGS:
      output = String(F("Too few args"));
      break;
    case ERR_TOO_MANY_ARGS:
      output = String(F("Too many args"));
      break;
    case ERR_NO_ARGS:
      output = String(F("No args"));
      break;
    case ERR_INVALID_ARG:
      output = String(F("Invalid argument"));
      break;
  }

  Serial.println(output);
}