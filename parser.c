#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "compiler.h"


typedef struct
{
  int kind;
  char name[12];
  int val;
  int level;
  int addr;
  int mark;
  int param;
} symbol;


enum token {
  modsym = 1,
  identsym,
  numbersym,
  plussym,
  minussym,
  multsym,
  slashsym,
  oddsym,
  eqlsym,
  neqsym,
  lessym,
  leqsym,
  gtrsym,
  geqsym,
  lparentsym,
  rparentsym,
  commasym,
  semicolonsym,
  periodsym,
  becomessym,
  beginsym,
  endsym,
  ifsym,
  thensym,
  whilesym,
  dosym,
  callsym,
  constsym,
  varsym,
  procsym,
  writesym,
  readsym,
  elsesym,
  returnsym
};

typedef struct
{
  int opcode;
  char op[4];
  int L;
  int M;
} line;

FILE *input;

line code[500];
int linePointer = 0;

symbol symbolTable[500];
int symPointer = 0;

char word[12];
int wordPointer = 0;

int procedureCount = 0;

// Token <- atoi(word)
int token = 0;


// HW 3 pseudocode functions
int parse(int);
int program(int);
int block(int, int, int);
int declConst(int);
int declVar(int, int);
int statement(int);
int condition(int);
int expression(int);
int term(int);
int fact(int);

// HW 4 pseudocode functions
int mark(int);
int declProc(int);

// Helper functions here
int scanWord();
int symTableCheck(char[], int);
int symTableSearch(char[], int, int);
int findProcedure(int);
void resetWord();
void emit(int, char[], int, int);

int parse(int print)
{
  input = fopen("Output/tokenList.txt", "r");

  if (program(print) == -1)
  {
    // return -1 if there's an error for any reason
    printf("Heyyy, something bad happened");
    fclose(input);
    return -1;
  }

  fclose(input);

  return 0;
}

// In pseudocode, this is PROGRAM
int program(int print)
{

  int numProc = 1;

  emit(7, "JMP", 0, 3);

  // Our version of foreach lexeme in list
  // We aren't keeping a list of lexemes and I don't feel like rewriting the code to do so
  // So this is my attempt at a workaround
  while (!feof(input))
  {
    scanWord();

    if (token == procsym)
    {
      numProc++;
      emit (7, "JMP", 0, 2);
    }
  }

  rewind(input);

  symbolTable[symPointer].kind = 3;
  strcpy(symbolTable[symPointer].name, "main");
  symbolTable[symPointer].val = 0;
  symbolTable[symPointer].level = 0;
  symbolTable[symPointer].addr = 0;
  symbolTable[symPointer].mark = 0;
  symbolTable[symPointer].param = 0;

  symPointer++;
  procedureCount++;

  // BLOCK
  // terminate program if BLOCK returns -1
  if (block(0, 0, 0) == -1)
  {
    return -1;
  }

  // Next token should be a period
  if (token != periodsym)
  {
    printf("Error : program must end with period %d %s\n", token, word);
    return -1;
  }

  for (int i = 0; i < numProc; i++)
  {
    code[i].M = symbolTable[findProcedure(i)].addr;
  }

  emit(9, "SYS", 0, 3);

  FILE *output = fopen("Output/parseOutput.txt", "w");

  if (print)
  {
    printf("Generated Assembly:\n");
    printf("Line\tOP\tL\tM\n");
  }

  for (int i = 0; i < linePointer; i++)
  {
    if (print)
      printf("%d\t%s\t%d\t%d\n", i, code[i].op, code[i].L, code[i].M);
    fprintf(output, "%d\t%d\t%d\n", code[i].opcode, code[i].L, code[i].M);
  }

  fclose(output);

  return 0;

}


int block(int lexLevel, int param, int procedureIndex)
{
  int c = declConst(lexLevel);

  int v = declVar(lexLevel, param);

  int p = declProc(lexLevel);

  symbolTable[procedureIndex].addr = linePointer;

  emit(6, "INC", 0, (4+v));

  // STATEMENT
  statement(lexLevel);

  mark(c+v+p);

  return 0;
}

// CONST-DECLARATION
int declConst(int lexLevel)
{
  char num[100];
  int numConst = 0;

  for (int i = 0; i < 100; i++)
    num[i] = 0;

  scanWord();

  if (token == constsym)
  {
    do
    {
      numConst++;

      scanWord();

      if (token != identsym)
      {
        printf("Error : const, var, and read keywords must be followed by identifier\n");
        return -1;
      }

      scanWord();

      if (symTableCheck(word, lexLevel) != -1)
      {
        printf("Error : symbol name has already been declared\n");
        return -1;
      }

      // Save ident name

      char identName[12];
      strcpy(identName, word);

      scanWord();

      if (token != eqlsym)
      {
        printf("Error : constants must be assigned with =\n");
        return -1;
      }

      scanWord();

      if (token != numbersym)
      {
        printf("Error : constants must be assigned an integer value\n");
        return -1;
      }

      scanWord();

      symbolTable[symPointer].kind = 1;
      strcpy(symbolTable[symPointer].name, identName);
      symbolTable[symPointer].val = token;
      symbolTable[symPointer].level = lexLevel;
      symbolTable[symPointer].addr = 0;
      symbolTable[symPointer].mark = 0;
      symbolTable[symPointer].param = 0;

      symPointer++;

      scanWord();
    } while(token == commasym);

    if (token != semicolonsym)
    {
      printf("Error : Semicolon expected.\n");
      return -1;
    }
  }
  else
  {
    // Push pointer back if there's no constants to declare
    fseek(input, -strlen(word) - 1, SEEK_CUR);
    return 1;
  }

  return numConst;
}

// VAR-DECLARATION
// Returns # of variables
int declVar(int lexLevel, int param)
{
  int numVars, namePointer = 0;

  if (param == 1)
    numVars = 1;
  else
    numVars = 0;

  char name[12];

  for (int i = 0; i < 12; i++)
    name[i] = 0;

  scanWord();

  if (token == varsym)
  {
    do
    {

      // Get next token
      scanWord();

      // if token == varsym
      if (token == identsym)
      {

        while (namePointer < 12)
        {
          char holder = fgetc(input);

          if (holder == ' ')
          {

            if (isdigit(name[0]))
            {
              // First letter of the variable is a number, that's illegal
              printf("Error : Variable cannot start with a digit. %s\n\n", name);
              return -1;
            }
            else
            {
              int result = symTableCheck(name, lexLevel);

              if (result != -1)
              {
                printf("Error : symbol name has already been declared.\n");
              }
              else
              {
                numVars++;

                symbolTable[symPointer].kind = 2;
                strcpy(symbolTable[symPointer].name, name);
                symbolTable[symPointer].val = 0;
                symbolTable[symPointer].level = lexLevel;
                symbolTable[symPointer].addr = numVars + 3;
                symbolTable[symPointer].mark = 0;
                symbolTable[symPointer].param = 0;

                symPointer++;
                namePointer = 0;

                for (int i = 0; i < 12; i++)
                  name[i] = 0;

                scanWord();

                break;
              }

            }

          }
          else
          {
            // Add letter to the name
            name[namePointer] = holder;
            namePointer++;
          }

        }

      }

      //printf("Token (second): %s %d\n", word, __LINE__);
    } while (token == commasym);

    if (token != semicolonsym)
    {
      printf("Error : constant and variable declarations must be followed by a semicolon\n");
      return -1;
    }

  }
  else if (token == semicolonsym)
  {
    return numVars;
  }
  else
  {
    // Push pointer back the length of the token if
    // there's no variables to declare
    fseek(input, -strlen(word) - 1, SEEK_CUR);
    return 0;
  }

  return numVars;
}

// PROCEDURE-DECLARATION
// Returns # of procedures
int declProc(int lexLevel)
{
  int numProc = 0;

  scanWord();

  //printf("Token: %d\n", token);

  if (token == procsym)
  {
    do
    {
      numProc++;

      scanWord();

      if (token != identsym)
      {
        printf("ERROR: Ident\n %d\n", token);
        return -1;
      }

      scanWord();

      // Defaulted L = 0
      // Change this ASAP to actually work with lexLevel
      if (symTableCheck(word, lexLevel) != -1)
      {
        printf("ERROR: lexLevel\n");
        return -1;
      }


      int procIdx = symPointer;

      procedureCount++;

      symbolTable[symPointer].kind = 3;
      strcpy(symbolTable[symPointer].name, word);
      symbolTable[symPointer].val = procedureCount;
      symbolTable[symPointer].level = lexLevel;
      symbolTable[symPointer].addr = 0;
      symbolTable[symPointer].mark = 0;
      symbolTable[symPointer].param = 0;

      symPointer++;

      scanWord();

      // if token == (
      if (token == lparentsym)
      {
        scanWord();

        if (token != identsym)
        {
          printf("Error: %d", __LINE__);
          return -1;
        }

        symbolTable[symPointer].kind = 2;
        strcpy(symbolTable[symPointer].name, word);
        symbolTable[symPointer].val = 0;
        symbolTable[symPointer].level = lexLevel + 1;
        symbolTable[symPointer].addr = 0;
        symbolTable[symPointer].mark = 0;
        symbolTable[symPointer].param = 0;

        symbolTable[procIdx].param = 1;

        scanWord();

        if (token != rparentsym)
        {
          printf("ERROR: Left parenthesis must be closed off with right parenthesis\n");
          return -1;
        }

        scanWord();

        if (token != semicolonsym)
        {
          printf("ERROR: Semicolon expected, line %d\n", __LINE__);
          return -1;
        }

        scanWord();

        printf("I never got here");
        block(lexLevel + 1, 1, procIdx);
      }
      else
      {
        if (token != semicolonsym)
        {
          printf("ERROR: Semicolon expected, line %d\n", __LINE__);
          return -1;
        }

        block(lexLevel + 1, 0, procIdx);
      }

      scanWord();

      if (code[linePointer - 1].opcode != 2 && code[linePointer - 1].M != 0)
      {
        emit(1, "LIT", 0, 0);
        emit(2, "RTN", 0, 0);
      }

      if (token != semicolonsym)
      {
        printf("ERROR: Semicolon expected, line %d\n", __LINE__);
        return -1;
      }

      scanWord();


    } while (token == procsym);
  }

  return numProc;
}

// STATEMENT
// Returns 1 if everything goes smoothly
// Returns -1 if there's an error that requires the program to be terminated
int statement(int lexLevel)
{
  //printf("Current token, word and lexLevel: %d %s %d\n", token, word, lexLevel);

  int index = 0, jpcIndex = 0, loopIndex = 0, jmpIndex = 0;

  switch(token)
  {
    case identsym:
      scanWord();

      index = symTableSearch(word, lexLevel, 2);

      printf("Searching for %s in the symbol table\n", word);

      if (index == -1)
      {
        printf("Error : undeclared symbol %s, %d\n", word, __LINE__);
        return -1;
      }

      if (symbolTable[index].kind != 2)
      {
        printf("Error : Not a var. Terminating program.\n");
        return -1;
      }

      // Get next token
      scanWord();

      if (token != becomessym)
      {
        printf("Error : assignment statements must use :=\n");
        return -1;
      }

      // Get next token
      scanWord();

      if (expression(lexLevel) == -1)
      {

        return -1;
      }

      emit(4, "STO", 0, symbolTable[index].addr);
      return 1;
    case callsym:
      scanWord();

      if (token != identsym)
      {
        printf("Error: const, var, procedure, call, and read keywords must be followed by identifier\n");
        return -1;
      }

      scanWord();
      printf("Searching for %s at %d %d\n", word, lexLevel, 3);

      int index = symTableSearch(word, lexLevel, 3);

      if (index == -1)
      {
        printf("Error %d\n", __LINE__);
        printf("INDEX: %d\n", index);
        return -1;
      }

      scanWord();

      if (token == lparentsym)
      {
        scanWord();
        if (symbolTable[symPointer].param != 1)
        {
          printf("Error %d\n", __LINE__);
          return -1;
        }
        expression(lexLevel);
        if (token != rparentsym)
        {
          printf("Error %d\n", __LINE__);
          return -1;
        }
        scanWord();
      }
      else
      {
        emit(1, "LIT", 0, 0);
        // This is sketch
      }
      emit(5, "CAL", lexLevel - symbolTable[index].level, symbolTable[index].val);

      return 1;
    case returnsym:
      if (lexLevel == 0)
      {
        printf("Error %d\n", __LINE__);
        return -1;
      }
      scanWord();
      if (token == lparentsym)
      {
        scanWord();
        expression(lexLevel);
        emit(2, "OPR", 0, 0);
        if (token != rparentsym)
        {
          printf("Error %d\n", __LINE__);
          return -1;
        }
        scanWord();
      }
      else
      {
        emit(1, "LIT", 0, 0);
        emit(2, "OPR", 0, 0);
      }

      return 1;
    case beginsym:
      do
      {
        scanWord();
        if (statement(lexLevel) == -1) return -1;
      } while(token == semicolonsym);

      if (token != endsym)
      {
        printf("Error : begin must be followed by end %s\n", word);
        return -1;
      }

      scanWord();

      return 1;
    case ifsym:
      scanWord();

      condition(lexLevel);

      // jpcIndex <- current code index
      jpcIndex = linePointer;

      emit(8, "JPC", 0, 0);

      if (token != thensym)
      {
        printf("Error : if must be followed by then\n");
        return -1;
      }

      scanWord();

      statement(lexLevel);

      if (token == elsesym)
      {
        scanWord();
        jmpIndex = linePointer;
        emit(7,"JMP", 0, jmpIndex);
        code[jpcIndex].M = linePointer;
        statement(lexLevel);
        code[jmpIndex].M = linePointer;
      }
      else
        code[jpcIndex].M = linePointer;

      return 1;
    case whilesym:
      scanWord();

      loopIndex = linePointer;

      condition(lexLevel);

      if (token != dosym)
      {
        printf("Error : while must be followed by do\n");
        return -1;
      }

      scanWord();

      jpcIndex = linePointer;
      emit(8, "JPC", 0, 0);

      statement(lexLevel);

      emit(7, "JMP", 0, loopIndex);

      code[jpcIndex].M = linePointer;

      return 1;
    case readsym:
      scanWord();

      if (token != identsym)
      {
        printf("Error : const, var, and read keywords must be followed by identifier\n");
        return -1;
      }

      scanWord();
      index = symTableCheck(word, lexLevel);

      // Symbol not found
      if (index == -1)
      {
        printf("Error : undeclared symbol %s, %d\n", word, __LINE__);
        return -1;
      }

      // Not a var
      if (symbolTable[index].kind != 2)
      {
        printf("Error : only variable values may be altered\n");
        return -1;
      }

      scanWord();

      emit(9, "SYS", 0, 2);
      emit(4, "STO", lexLevel - symbolTable[index].level, symbolTable[index].addr);

      return 1;
    case writesym:
      scanWord();

      expression(lexLevel);

      emit(9, "SYS", 0, 1);

      return 1;
    default:
      if (token == endsym)
        return 1;

      printf("???? LexLevel: %d\n", lexLevel);
      return -1;
  }
}

// CONDITION
int condition(int lexLevel)
{
  if (token == oddsym)
  {
    scanWord();

    if (expression(lexLevel) == -1) return -1;

    emit(2, "OPR", 0, 6);
  }
  else
  {
    if (expression(lexLevel) == -1) return -1;

    switch(token)
    {
      case eqlsym:
        scanWord();
        if (expression(lexLevel) == -1) return -1;
        emit(2, "OPR", 0, 8);
        break;
      case neqsym:
        scanWord();
        if (expression(lexLevel) == -1) return -1;
        emit(2, "OPR", 0, 9);
        break;
      case lessym:
        scanWord();
        if (expression(lexLevel) == -1) return -1;
        emit(2, "OPR", 0, 10);
        break;
      case leqsym:
        scanWord();
        if (expression(lexLevel) == -1) return -1;
        emit(2, "OPR", 0, 11);
        break;
      case gtrsym:
        scanWord();
        if (expression(lexLevel) == -1) return -1;
        emit(2, "OPR", 0, 12);
        break;
      case geqsym:
        scanWord();
        if (expression(lexLevel) == -1) return -1;
        emit(2, "OPR", 0, 13);
        break;
      default:
        printf("Error : condition must contain comparison operator");
        return -1;
    }
  }

  return 1;
}

// EXPRESSION
int expression(int lexLevel)
{
  if (token == minussym)
  {
    scanWord();

    if (term(lexLevel) == -1)
    {
      return -1;
    }

    emit(2, "OPR", 0, 1);

    while (token == plussym || token == minussym)
    {
      if (token == plussym)
      {
        scanWord();
        if (term(lexLevel) == -1)
        {
          return -1;
        }
        emit(2, "OPR", 0, 2);
      }
      else
      {
        scanWord();
        if (term(lexLevel) == -1)
        {
          return -1;
        }
        emit(2, "OPR", 0, 3);
      }
    } // end while plussym/minussym loop

  }
  else
  {
    if (token == plussym)
      scanWord();

    if (term(lexLevel) == -1)
    {
      return -1;
    }

    while (token == plussym || token == minussym)
    {
      if (token == plussym)
      {
        scanWord();
        if (term(lexLevel) == -1)
        {
          return -1;
        }
        emit(2, "OPR", 0, 2);
      }
      else
      {
        scanWord();
        if (term(lexLevel) == -1)
        {
          return -1;
        }
        emit(2, "OPR", 0, 3);
      }
    } // end while plussym/minussym loop

  } // end if/else

  return 1;
}

//TERM
int term(int lexLevel)
{
  if (fact(lexLevel) == -1)
  {
    return -1;
  }

  while (token == multsym || token == slashsym || token == modsym)
  {
    switch(token)
    {
      case multsym:
        scanWord();
        if (fact(lexLevel) == -1)
          return -1;
        emit(2, "OPR", 0, 4);
        break;
      case slashsym:
        scanWord();
        if (fact(lexLevel) == -1)
          return -1;
        emit(2, "OPR", 0, 5);
        break;
      case modsym:
        scanWord();
        if (fact(lexLevel) == -1)
          return -1;
        emit(2, "OPR", 0, 7);
        break;
      default:
        printf("Error : Unexpected token\n");
        return -1;
    }

  }

  return 1;
}

// FACTOR
int fact(int lexLevel)
{
  int result = 0;

  switch (token)
  {
    case identsym:
      //// This isn't in the pseudocode, just wait and see if it works

      scanWord();
      result = symTableCheck(word, lexLevel);

      if (result == -1)
      {
        printf("Error : undeclared symbol %s, %d\n", word, __LINE__);
        return -1;
      }

      if (symbolTable[result].kind == 1)
      {
        // Constant
        emit(1, "LIT", 0, symbolTable[result].val);
      }
      else if (symbolTable[result].kind == 2)
      {
        // Variable
        emit(3, "LOD", 0, symbolTable[result].addr);
      }

      scanWord();

      break;
    case numbersym:
      // Get next token
      scanWord();

      int num = token;

      emit(1, "LIT", 0, num);

      scanWord();
      break;
    case lparentsym:
      scanWord();

      if (expression(lexLevel) == -1) return -1;

      if (token != rparentsym)
      {
        printf("Error : right parenthesis must follow left parenthesis\n");
        return -1;
      }

      scanWord();

      break;
    case callsym:
      statement(lexLevel);
      break;
    default:
      printf("Error : Unexpected token %d. Line %d.\n", token, __LINE__);
      return -1;
  }

  return 1;
}


// Original functions, not specified in the pseudocode
// Mostly to help facilitate the above code

// Helper function to get next token
int scanWord()
{
  resetWord();

  while (1)
  {
    char holder = fgetc(input);

    if (holder == ' ' || holder == '\n')
    {
      // This is the word
      // Transfer it to token
      token = atoi(word);
      return 0;
    }
    else if (holder == EOF)
    {
      return 50;
    }

    word[wordPointer] = holder;
    wordPointer++;

    if (wordPointer > 12)
    {
      //printf("Error : Name too long\n");
      break;
    }
  }

  return 1;
}

int mark(int count)
{
  for (int i = symPointer - 1; i >= 0; i--)
  {
    if (symbolTable[i].mark == 0)
    {
      symbolTable[i].mark = 1;

      count--;
    }
  }

  return 0;
}

int symTableCheck(char ident[], int lexLevel)
{
  for (int i = 0; i < symPointer; i++)
  {
    if (strcmp(ident, symbolTable[i].name) == 0 && symbolTable[i].level == lexLevel)
    {
      return i;
    }
  }
  return -1;
}

int symTableSearch(char string[], int lexLevel, int kind)
{
  int index = -1;
  int mindiff = INT_MAX;

  for (int i = 0; i < symPointer; i++)
  {
    if (strcmp(symbolTable[i].name, string) == 0 && symbolTable[i].kind == kind)
    {
      if (symbolTable[i].mark == 0)
      {
        int diff = abs(symbolTable[i].level - lexLevel);
        if (diff < mindiff)
        {
          mindiff = diff;
          index = i;
        }

      }

      return i;
    }

  }

  return index;
}

int findProcedure(int num)
{
  printf("Finding procedure: %d\n", num);
  for (int i = 0; i < 500; i++)
    if (symbolTable[i].kind == 3 && symbolTable[i].val == num)
    {
      printf("I found procedure %d\n", num);
      return i;
    }

  return -1;
}

// Reset the word for being read to avoid any conflicts
void resetWord()
{
  wordPointer = 0;

  for (int i = 0; i < 12; i++)
    word[i] = 0;
}

void emit(int opcode, char op[], int L, int M)
{
  //printf("Trying to put %s into code.OP\n", op);
  code[linePointer].opcode = opcode;
  strcpy(code[linePointer].op, op);
  code[linePointer].L = L;
  code[linePointer].M = M;

  linePointer++;
}
