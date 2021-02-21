// Implementation of Tiny BASIC

double internal_vars[26];

File sourceFile;

boolean openProgram(String fileName) {
  if (sourceFile != NULL) {
    sourceFile.close();
  }
  sourceFile = SD.open(fileName);
  if (sourceFile) {
    return true;
  }
  return false;
}

boolean execCommand() {
  if (sourceFile == NULL || !sourceFile || !sourceFile.available()) {
    return false;
  }
  switch (sourceFile.read()) {
    case 'P':
    case 'p':
      switch (sourceFile.read()) {
        case 'R':
        case 'r':
          switch (sourceFile.read()) {
            case 'I':
            case 'i':
              switch (sourceFile.read()) {
                case 'N':
                case 'n':
                  switch (sourceFile.read()) {
                    case 'T':
                    case 't':
                      if (isWhitespace(sourceFile.read()))
                        cmdPRINT();
                      break;
                  }
                  break;
              }
              break;
          }
          break;
      }
      break;
  }
  return true;
}

void cmdPRINT() {
  while (true) {
    int la = lookAhead();
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
        if (nextChar  < 0) {
          if (nextChar == -2)
            goto end_of_loop;
          break;
        }
        Serial.write(nextChar);
      }
      continue;
    }
    // Must be an expression
    //    Serial.println(F("Expressions aren't implemented yet, sorry!"));
    double val = evaluateExpression();
    if (val - ((long)val) == 0)
      Serial.print((long)val);
    else
      Serial.print(val);
  }
end_of_loop:
  Serial.println();
}

int getNextCharInString() {
  int character = sourceFile.read();
  if (character == -1) {
    // EOF
    return -2;
  }
  if (character == '\\') {
    if (lookAhead() == '\\') {
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
  int expLength = 0;
  int startPos = sourceFile.position();
  while (true) {
    int val = sourceFile.read();
    if (val == ',' || val == '"' || val == -1 || isControl(val))
      break;
  }
  expLength = sourceFile.position() - startPos - 1;
  if (expLength == 0)
    return 0;
  double res = evaluateExpression(startPos, expLength);
  sourceFile.seek(startPos + expLength);
  return res;
}

double evaluateExpression(int startPos, int len) {
  // Memory check because of evil recursion
  if (freeMemory() < 100) {
    Serial.print(F("OOM: "));
    Serial.println(freeMemory());
    Serial.print(F("GC: "));
    Serial.println(FreeRam());
    if (freeMemory() < 100) {
      Serial.print(F("OOM2: "));
      Serial.println(freeMemory());
      return INFINITY;
    }
  }
  int minOp = 255; // 255 == NOTHING, return INFINITY to indicate an error
  int opPos = startPos;
  int layer = 0;
  const int targetPos = startPos + len;
  sourceFile.seek(startPos);
  while (minOp > 1 && sourceFile.position() < targetPos) {
    int character = sourceFile.read();
    if (layer == 0) {
      if (minOp > 1) {
        int lb = 2;
        while (sourceFile.position() - lb >= startPos && isWhitespace(lookBehind(lb))) {
          lb++;
        }
        if ((character == '+' || character == '-') && !sourceFile.position() - 1 > startPos && lookBehind(lb) != '(' && lookBehind(lb) != ')') { // simple unary check
          if (character == '+')
            minOp = 0;
          else if (character == '-')
            minOp = 1;
          opPos = sourceFile.position();
          continue;
        }
      }
      if (minOp > 4 && (character == '*' || character == '/' || character == '%')) {
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
      if (minOp > 6 && character == '(' || character == ')') {
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
      if (minOp > 10 && (isAlphaNumeric(character) || character == '.')) {
        if (isAlpha(character))
          minOp = 10;
        else
          minOp = 9;
        opPos = sourceFile.position();
      }
    }
    if (character == '(')
      layer++;
    else if (character == ')')
      layer--;
  }
  if (minOp == 255)
    return INFINITY;
  opPos--;

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
        return (int) left_side % (int)right_side;
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
      return (double) sourceFile.parseFloat();
    case 10:
      sourceFile.seek(opPos);
      return varFromIndex(varIndexFromChar(sourceFile.read()));
  }
}

int varIndexFromChar(int letter) {
  if (isLowerCase(letter))
    letter -= 0x20;
  letter -= 0x41;
  letter %= 26;
  letter = abs(letter);
  return letter;
}

double varFromIndex(int index) {
  return internal_vars[index / 26];
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
   Reads the next byte without changing the position in the file
*/
int lookAhead() {
  int res = sourceFile.read();
  sourceFile.seek(sourceFile.position() - 1);
  return res;
}
