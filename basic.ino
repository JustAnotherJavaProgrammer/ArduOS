// Implementation of Tiny BASIC
#define internal_basic_stack_height 50

const char commands[] PROGMEM = { "PRINT REM RETURN*END*LET IF GOTO GOSUB DRAW " };
const byte commandsCount PROGMEM = 9;
const char drawCommands[] PROGMEM = { "PIXEL LINE FAST_VLINE FAST_HLINE RECT FRECT CIRC FCIRC ROUND_RECT FROUND_RECT ROUND_FRECT TRIANGLE FTRIANGLE SET_CURSOR SET_TEXT_COLOR SET_TEXT_SIZE FILL_SCREEN IMAGE SET_TEXT_WRAP TEXT " };
const byte drawCommandsCount PROGMEM = 20;

double internal_vars[26];
unsigned long internal_stack[internal_basic_stack_height];
byte stackPos = 0;

File sourceFile;

//int charToUpperCase(int letter);
//void cmdPRINT();
//void cmdLET();

boolean openProgram(String fileName) {
  if (sourceFile != NULL) {
    sourceFile.close();
  }
  stackPos = 0;
  sourceFile = SD.open(fileName);
  if (sourceFile) {
    return true;
  }
  return false;
}

byte identifyCommand(const char *starting_byte, byte commandsCount) {
  unsigned long startingPos = sourceFile.position();
  //  progmemPrint(starting_byte);
  //  Serial.println(startingPos, HEX);
  byte cmd = -1;  // equal to 255 (byte is unsigned)
  {
    byte currChar = 255;
    for (byte i = 0; i < commandsCount;) {
      sourceFile.seek(startingPos);
      for (byte j = 0; j < 255; j++) {
        currChar++;
        //        Serial.print(cmds(currChar));
        int c = sourceFile.read();
        //        Serial.print(c, HEX); Serial.print(F(","));
        //        Serial.print(cmds(starting_byte, currChar), HEX); Serial.write(cmds(starting_byte, currChar)); Serial.print(F(" "));
        switch (cmds(starting_byte, currChar)) {
          case '*':
            if (!isAlpha(c)) {
              cmd = i;
              goto command_found;
            } else
              goto not_this_command;
          case ' ':
            if (isWhitespace(c)) {
              cmd = i;
              goto command_found;
            } else
              goto not_this_command;
          default:
            if (cmds(starting_byte, currChar) == toUpperCase(c))
              continue;
            goto not_this_command;
        }
      }
not_this_command:
      while (cmds(starting_byte, currChar) != '*' && cmds(starting_byte, currChar) != ' ') {
        currChar++;
      }
      //      currChar++;
      //      Serial.print(F("Not "));
      //      Serial.println(i);
      i++;
      continue;
    }
  }
  sourceFile.seek(startingPos + 1);
command_found:
  return cmd;
}

boolean execCommand() {
  if (sourceFile == NULL || !sourceFile || !sourceFile.available()) {
    return false;
  }
  unsigned long startingPos = sourceFile.position();
  byte cmd = identifyCommand(commands, commandsCount);
  //    Serial.print(startingPos, HEX);
  //    Serial.print(F(": "));
  //  Serial.println(cmd);
  //  if (cmd == 8) {
  //    Serial.println(F("DRAW!"));
  //  }
  switch (cmd) {
    case 0:  // PRINT
      cmdPRINT();
      break;
    case 1:  // REM
      discardRestOfLine();
      break;
    case 2:  // RETURN
      if (stackPos > 0) {
        stackPos--;
        sourceFile.seek(internal_stack[stackPos]);
        break;
      }
    // else: END execution of current program
    case 3:  // END
      sourceFile.close();
      break;
    case 4:  // LET
      cmdLET();
      break;
    case 5:  // IF
      if (evalCmp() < 0.0000000001)
        discardRestOfLine();
      break;
    case 6:  // GOTO
    case 7:  // GOSUB
      {
        unsigned long jumpTarget = findGOTO();
        //      Serial.print(F("JT: "));
        //      Serial.println(jumpTarget, HEX);
        if (jumpTarget == sourceFile.position()) {
          discardRestOfLine();
          break;
        }
        if (cmd == 7) {
          discardRestOfLine();
          if (stackPos >= internal_basic_stack_height)
            return false;
          internal_stack[stackPos] = sourceFile.position();
          stackPos++;
        }
        sourceFile.seek(jumpTarget);
      }
      break;
    case 8:  // DRAW
      //      Serial.println(F("cmdsDraw"));
      cmdsDraw();
      break;
    default:
      if (!isWhitespaceOrControl(lookBehind()))
        while (!isWhitespaceOrControl(sourceFile.peek())) {
          sourceFile.read();
        }
      break;
  }
  return true;
}

void cmdPRINT() {
  while (true) {
    int la = sourceFile.peek();
    if (isWhitespace(la) || la == ',') {
      //      Serial.println(F("Whitespace or comma"));
      sourceFile.read();
      continue;
    }
    if (isControl(la) || la == -1) {
      sourceFile.read();
      break;
    }
    if (la == '"') {
      //      Serial.println(F("It's a string, baby!"));
      //      while (true) {};
      // It's a string!
      sourceFile.read();
      while (true) {
        int nextChar = getNextCharInString();
        if (nextChar < 0) {
          if (nextChar == -2)
            goto end_of_loop;
          break;
        }
        tft.write(nextChar);
      }
      continue;
    }
    // Must be an expression
    //    Serial.println(F("Expressions aren't implemented yet, sorry!"));
    double val = evaluateExpression();
    if (shouldDiscardFloatingPoint(val))
      tft.print((long)val);
    else
      tft.print(val);
  }
end_of_loop:
  tft.println();
}

boolean shouldDiscardFloatingPoint(double val) {
  val = abs(val);
  return val - ((long)val) < 0.0000000001;
}

void cmdLET() {
  while (!isAlpha(sourceFile.peek())) {
    if (readAndDiscardWithEOLandEOFchecks()) {
      return;
    }
  }
  int index = varIndexFromChar(sourceFile.read());
  while (sourceFile.peek() != '=') {
    if (readAndDiscardWithEOLandEOFchecks()) {
      return;
    }
  }
  sourceFile.read();
  //  Serial.println(sourceFile.position(), HEX);
  internal_vars[index] = evaluateExpression();
}

double evalCmp() {
  double left = evaluateExpression();
  int c = sourceFile.peek();
  if (!sourceFile.available())
    return left;
  byte comparator = 0;
  switch (c) {
    case '<':
      comparator = 1;
      break;
    case '>':
      comparator = 3;
      break;
    case '=':
      comparator = 4;
      break;
    default:
      return left;
  }
  sourceFile.read();
  c = sourceFile.peek();
  if (c == '=') {
    comparator *= 2;
    sourceFile.read();
  }
  double right = evaluateExpression();
  double diff = left - right;
  return (abs(diff) < 0.0000000001 && ((comparator < 3 && left < right) || (comparator % 3 == 0 && left > right) || comparator % 2 == 0)) ? 1.0 : 0.0;
}

unsigned long findGOTO() {
  while (isWhitespace(sourceFile.peek()))
    sourceFile.read();
  unsigned long startingPos = sourceFile.position();
  unsigned long res = startingPos;
  if (sourceFile.peek() == '(') {  // goto also works with expressions in brackets as arguments
    double trgt = evaluateExpression();
    sourceFile.seek(0);
    while (sourceFile.available()) {
      //      Serial.println("---");
      //      Serial.println(sourceFile.position(), HEX);
      skipWhitespace();
      //      Serial.println(sourceFile.position(), HEX);
      int c = sourceFile.peek();
      //      Serial.write(c);
      unsigned long lineBeg = sourceFile.position();
      if (((isDigit(c) || c == '-' || (c == '.' && !shouldDiscardFloatingPoint(trgt))) && ((shouldDiscardFloatingPoint(trgt) && sourceFile.parseInt() == (long)trgt) || sourceFile.parseFloat() == trgt)) || (c == '(' && (sourceFile.seek(lineBeg), trgt == evaluateExpression()))) {  // It also evaluates expressions between brackets at the beginning of possible target lines!
        res = lineBeg;
        goto findGOTO_done;
      }
      //      if (c == '(')
      //        Serial.println(sourceFile.seek(lineBeg), evaluateExpression());
      //      Serial.println(sourceFile.position(), HEX);
      sourceFile.seek(lineBeg);
      discardRestOfLine();
      //      Serial.println(sourceFile.position(), HEX);
    }
    goto findGOTO_done;
  }

  {
    unsigned int wordLength = 0;
    while (!isWhitespaceOrControl(sourceFile.read())) {
      wordLength++;
    }
    sourceFile.seek(0);
    while (sourceFile.available()) {
      skipWhitespace();
      unsigned long lineStart = sourceFile.position();
      for (unsigned int i = 0; i < wordLength; i++) {
        if ((sourceFile.seek(lineStart + i), sourceFile.read()) != (sourceFile.seek(startingPos + i), sourceFile.read())) {
          sourceFile.seek(lineStart);
          discardRestOfLine();
          goto findGOTO_nextLine;
        }
      }
      res = lineStart + wordLength;
      break;
findGOTO_nextLine:
      continue;
    }
  }
findGOTO_done:
  sourceFile.seek(startingPos);
  return res;
}

void skipWhitespace() {
  while (isWhitespace(sourceFile.peek())) {
    sourceFile.read();
  }
}

void discardRestOfLine() {
  while (!readAndDiscardWithEOLandEOFchecks()) {}
}

boolean isWhitespaceOrControl(int c) {
  return isWhitespace(c) || isControl(c);
}

boolean readAndDiscardWithEOLandEOFchecks() {
  //  Serial.print(sourceFile.peek(), HEX);
  //  Serial.println(isControl(sourceFile.peek()));
  return !sourceFile.available() || isControl(sourceFile.read());
}

void cmdsDraw() {
  byte cmd = identifyCommand(drawCommands, drawCommandsCount);
  // Read all numeric arguments for all DRAW commands at once to save on program memory (flash)
  uint16_t args[7];
  if (cmd < 18) {
    bool stopNext = false;
    for (byte i = 0; i < 7; i++) {
      // args[i] = sourceFile.peek() == ',' || i == 0 ? evaluateExpression() : 0;
      if (sourceFile.peek() == ',')
        sourceFile.read();
      skipWhitespace();
      args[i] = !isControl(sourceFile.peek()) && (cmd != 17 || i < 2) ? evaluateExpression() : 0;
      // Serial.print(i);
      // Serial.print(F(": "));
      // Serial.println(args[i]);
    }
  }
  switch (cmd) {
    case 0:  // DRAW PIXEL x, y, color
      //      unsigned long pos = sourceFile.position();
      //      while (!isControl(sourceFile.peek()))
      //        Serial.write(sourceFile.read());
      //      sourceFile.seek(pos);
      //      Serial.println((unsigned int)evaluateExpression());
      //      (sourceFile.peek() == ',' ? sourceFile.read() : 0);
      //      Serial.println((unsigned int)evaluateExpression());
      //      (sourceFile.peek() == ',' ? sourceFile.read() : 0);
      //      Serial.println((unsigned int)evaluateExpression());
      //      sourceFile.seek(pos);
      tft.drawPixel(args[0], args[1], args[2]);
      //      Serial.println(freeMemory());
      //      tft.drawPixel(100, 10, 0);
      break;
    case 1:  // DRAW LINE x0, y0, x1, y1, color
      tft.drawLine(args[0], args[1], args[2], args[3], args[4]);
      break;
    case 2:  // DRAW FAST_VLINE x0, y0, length, color
      tft.drawFastVLine(args[0], args[1], args[2], args[3]);
      break;
    case 3:  // DRAW FAST_HLINE x0, y0, length, color
      tft.drawFastHLine(args[0], args[1], args[2], args[3]);
      break;
    case 4:  // DRAW RECT x, y, width, height, color
      tft.drawRect(args[0], args[1], args[2], args[3], args[4]);
      break;
    case 5:  // DRAW FRECT x, y, width, height, color
      tft.fillRect(args[0], args[1], args[2], args[3], args[4]);
      break;
    case 6:  // DRAW CIRC x0, y0, radius, color
      tft.drawCircle(args[0], args[1], args[2], args[3]);
      break;
    case 7:  // DRAW FCIRC x0, y0, radius, color
      tft.fillCircle(args[0], args[1], args[2], args[3]);
      break;
    case 8:  // DRAW ROUND_RECT x0, y0, width, height, radius, color
      tft.drawRoundRect(args[0], args[1], args[2], args[3], args[4], args[5]);
      break;
    case 9:   // DRAW FROUND_RECT
    case 10:  // DRAW ROUND_FRECT
      tft.fillRoundRect(args[0], args[1], args[2], args[3], args[4], args[5]);
      break;
    case 11:  // DRAW TRIANGLE x0, y0, x1, y1, x2, y2, color
      tft.drawTriangle(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
      break;
    case 12:  // DRAW FTRIANGLE
      tft.fillTriangle(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
      break;
    case 13:  // DRAW SET_CURSOR x0, y0
      tft.setCursor(args[0], args[1]);
      break;
    case 14:  // DRAW SET_TEXT_COLOR color, [background_color]
      if (args[1] == 0) {
        tft.setTextColor(args[0]);
        break;
      }
      if (args[0] == args[1])
        args[1] = 0;
      tft.setTextColor(args[0], args[1]);
      break;
    case 15:  // DRAW SET_TEXT_SIZE size
      tft.setTextSize(args[0]);
      break;
    case 16:  // DRAW FILL_SCREEN color
      tft.fillScreen(args[0]);
      break;
    case 17:  // DRAW IMAGE x, y, filename_string
      if (sourceFile.peek() != '\"')
        break;
      sourceFile.read();
      // Serial.println(freeMemory());
      drawBmp(sourceFile.readStringUntil('"').c_str(), args[0], args[1]);
      // Serial.println(freeMemory());
      break;
    case 18:  // DRAW SET_TEXT_WRAP
      tft.setTextWrap(evalCmp() < 0.0000000001);
      break;
    case 19:  // DRAW TEXT
      cmdPRINT();
      return;
  }
  discardRestOfLine();
}

int getNextCharInString() {
  int character = sourceFile.read();
  if (character == -1) {
    // EOF
    return -2;
  }
  if (character == '\\') {
    if (sourceFile.peek() == '\\') {
      return character;
    }
    return 0;
  }
  if (character == '"' || isControl(character)) {
    if (lookBehind(2) == '\\') {
      return character;
    }
    return -1;
  }
  return character;
}

double evaluateExpression() {
  // Thought about a conversion to Polish Notation before evaluation here, at first
  unsigned long expLength = 0;
  unsigned long startPos = sourceFile.position();
  //  Serial.print(F("Expression found at "));
  //  Serial.println(startPos, HEX);
  while (true) {
    //    Serial.println(sourceFile.position(), HEX);
    int val = sourceFile.read();
    if (val == ',' || val == '"' || val == -1 || !sourceFile.available() || isControl(val) || val == '<' || val == '>' || val == '=' || (charToUpperCase(sourceFile.peek()) == 'T' && charToUpperCase(lookAhead(2)) == 'H' && charToUpperCase(lookAhead(3)) == 'E' && charToUpperCase(lookAhead(4)) == 'N' && isWhitespace(lookAhead(5))))
      break;
  }
  expLength = sourceFile.position() - startPos - (sourceFile.read() == -1 ? 0 : 1);
  //  Serial.println(expLength);
  if (expLength == 0)
    return 0;
  double res = evaluateExpression(startPos, expLength);
  sourceFile.seek(startPos + expLength);
  return res;
}

double evaluateExpression(int startPos, unsigned long len) {
  // Memory check because of evil recursion
  if (freeMemory() < 100) {
    //    Serial.print(F("OOM: "));
    //    Serial.println(freeMemory());
    //    //    Serial.print(F("GC: "));
    //    Serial.println(FreeRam());
    //    if (freeMemory() < 100) {
    ////      Serial.print(F("OOM2: "));
    ////      Serial.println(freeMemory());
    return INFINITY;
    //    }
  }
  int minOp = 255;  // 255 == NOTHING, return INFINITY to indicate an error
  unsigned long opPos = startPos;
  int layer = 0;
  const unsigned long targetPos = startPos + len;
  sourceFile.seek(startPos);
  while (sourceFile.position() < targetPos) {
    int character = sourceFile.read();
    //    Serial.println(sourceFile.position());
    if (layer == 0) {
      //      if (minOp > 1)
      {
        unsigned long lb = 2;
        while (sourceFile.position() - lb >= startPos && isWhitespace(lookBehind(lb))) {
          lb++;
        }
        if ((character == '+' || character == '-') && sourceFile.position() - lb >= startPos && lookBehind(lb) != '(' && lookBehind(lb) != ')') {  // simple unary check
          if (character == '+')
            minOp = 0;
          else if (character == '-')
            minOp = 1;
          opPos = sourceFile.position();
          continue;
        }
      }
      //      if (minOp > 4 && (character == '*' || character == '/' || character == '%')) {
      if (minOp > 1 && (character == '*' || character == '/' || character == '%')) {
        switch (character) {
          case '*':
            minOp = 2;
            break;
          case '/':
            minOp = 3;
            break;
          case '%':
            minOp = 4;
            break;
            //          default:
            //            // empty, should never be called
            //            break;
        }
        opPos = sourceFile.position();
        continue;
      }
      //      if (minOp > 6 && character == '(' || character == ')') {
      if (minOp > 4 && character == '(' || character == ')') {
        if (character == '(')
          minOp = 5;
        else
          minOp = 6;
        opPos = sourceFile.position();
        continue;
      }
      if (minOp > 8 && (character == '+' || character == '-')) {
        if (character == '+')
          minOp = 7;
        else
          minOp = 8;
        opPos = sourceFile.position();
        continue;
      }
      if (minOp > 10 && (isAlphaNumeric(character) || character == '.' || character == '_')) {
        //        Serial.write(character);
        //        Serial.println();
        if ((isAlpha(character) || character == '_') && !isAlpha(lookBehind(2)) && lookBehind() != '_') {
          minOp = 10;
          goto setPos;
        } else if (!isAlpha(lookBehind()) && character != '_') {
          minOp = 9;
          goto setPos;
        }
        if (false) {
setPos:
          opPos = sourceFile.position();
          continue;
        }
      }
    }
layerCheck:
    if (character == '(')
      layer++;
    else if (character == ')')
      layer--;
  }
  if (minOp == 255)
    return INFINITY;
  opPos--;

  //      Serial.print(F("{ \""));
  //    sourceFile.seek(startPos);
  //    while (sourceFile.position() < targetPos) {
  //      Serial.write(sourceFile.read());
  //    }
  //    Serial.println();
  //      Serial.print(F("\", "));
  //      sourceFile.seek(opPos);
  //      Serial.write(sourceFile.read());
  //      Serial.print(F(" at "));
  //      Serial.println(opPos, HEX);
  //      Serial.print(F(", "));
  //      Serial.print(minOp);
  //      Serial.println(F(" }"));

  if (minOp < 5) {
    double left_side = evaluateExpression(startPos, opPos - startPos);
    double right_side = evaluateExpression(opPos + 1, targetPos - opPos - 1);
    if (left_side == INFINITY || right_side == INFINITY)
      return INFINITY;
    switch (minOp) {
      case 0:
        return left_side + right_side;
      case 1:
        return left_side - right_side;
      case 2:
        return left_side * right_side;
      case 3:
        return left_side / right_side;
      case 4:
        return (int)left_side % (int)right_side;
    }
  }
  switch (minOp) {
    case 5:
    case 7:
      return evaluateExpression(opPos + 1, targetPos - opPos - 1);
    case 6:
      return evaluateExpression(startPos, opPos - startPos);
    case 8:
      return -evaluateExpression(opPos + 1, targetPos - opPos - 1);
    case 9:
      sourceFile.seek(opPos);
      return (double)sourceFile.parseFloat();
    case 10:
      sourceFile.seek(opPos);
      //      Serial.println(sourceFile.peek());
      return varFromIndex(varIndexFromChar(sourceFile.read()));
  }
}

int varIndexFromChar(int letter) {
  letter = charToUpperCase(letter);
  letter -= 0x41;
  letter %= 26;
  letter = abs(letter);
  return letter;
}

int charToUpperCase(int letter) {
  if (isLowerCase(letter))
    return letter - 0x20;
  return letter;
}

double varFromIndex(int index) {
  return internal_vars[index % 26];
}

/**
   Reads the last byte again
*/
int lookBehind() {
  sourceFile.seek(sourceFile.position() - 1);
  return sourceFile.read();
}

int lookBehind(int num) {
  sourceFile.seek(sourceFile.position() - num);
  int res = sourceFile.read();
  sourceFile.seek(sourceFile.position() + (num - 1));
  return res;
}

/**
   Reads the byte num bytes ahead without changing the position in the file
*/
int lookAhead(int num) {
  sourceFile.seek(sourceFile.position() + num - 1);
  int res = sourceFile.read();
  sourceFile.seek(sourceFile.position() - num);
  return res;
}

int cmds(const char *start_byte, byte i) {
  return pgm_read_byte_near(start_byte + i);
}
