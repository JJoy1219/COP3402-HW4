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

line code[500];
int linePointer = 0;

symbol symbolTable[500];
int symPointer = 0;

char word[12];
int wordPointer = 0;

// Token <- atoi(word)
int token = 0;


// HW 3 pseudocode functions
int parse(int);
int program(FILE*, int);
int block(int, int, int);
int declConst(FILE*);
int declVar(FILE*);
int statement(int lexlevel);
int condition(FILE*);
int expression(FILE*);
int term(FILE*);
int fact(FILE*);

// HW 4 pseudocode functions
int mark(int);
int declProc(int);

// Helper functions here
int scanWord(FILE*);
int symTableCheck(char[]);
int symTableSearch(char[], int, int);
int findProcedure(int);
void resetWord();
void emit(int, char[], int, int);

int parse(int print)
{
  FILE *input = fopen("Output/tokenList.txt", "r");

  if (program(input, print) == -1)
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
int program(FILE *input, int print)
{
  // BLOCK
  // terminate program if BLOCK returns -1
  if (block(input) == -1)
  {
    return -1;
  }

  // Next token should be a period
  if (token != periodsym)
  {
    printf("Error : program must end with period %d %s\n", token, word);
    return -1;
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


int block (int lexlevel, int param, int procedureIndex)
{
  int c = declConst(lexlevel);

  int v = declVar(lexlevel, param);

  int p = declProc(lexlevel);

  symbol_table[procedureIndex].addr = linePointer;

  emit(6, "INC", 0, (4+v));

  // STATEMENT
  statement(lexlevel);

  mark(c+v+p);
}

// CONST-DECLARATION
int declConst(FILE *input)
{
  char num[100];
  int numPoint = 0;

  for (int i = 0; i < 100; i++)
    num[i] = 0;

  scanWord(input);

  if (token == constsym)
  {
    do
    {
      scanWord(input);

      if (token != identsym)
      {
        printf("Error : const, var, and read keywords must be followed by identifier\n");
        return -1;
      }

      scanWord(input);

      if (symTableCheck(word) != -1)
      {
        printf("Error : symbol name has already been declared\n");
        return -1;
      }

      // Save ident name

      char identName[12];
      strcpy(identName, word);

      scanWord(input);

      if (token != eqlsym)
      {
        printf("Error : constants must be assigned with =\n");
        return -1;
      }

      scanWord(input);

      if (token != numbersym)
      {
        printf("Error : constants must be assigned an integer value\n");
        return -1;
      }

      scanWord(input);

      symbolTable[symPointer].kind = 1;
      strcpy(symbolTable[symPointer].name, identName);
      symbolTable[symPointer].val = token;
      symbolTable[symPointer].level = 0;
      symbolTable[symPointer].addr = 0;

      symPointer++;

      scanWord(input);
    } while(token == commasym);
  }
  else
  {
    // Push pointer back if there's no constants to declare
    fseek(input, -strlen(word) - 1, SEEK_CUR);
    return 1;
  }

  return 1;
}

// VAR-DECLARATION
// Returns # of variables
int declVar(FILE *input)
{
  int numVars = 0, namePointer = 0;

  char name[12];

  for (int i = 0; i < 12; i++)
    name[i] = 0;

  scanWord(input);

  if (token == varsym)
  {

    do
    {

      // Get next token
      scanWord(input);

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
              //printf("Error : Variable cannot start with a digit. %s\n\n", name);
            }
            else
            {
              int result = symTableCheck(name);
              //printf("New ident: %s\n", name);

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
                symbolTable[symPointer].level = 0;
                symbolTable[symPointer].addr = numVars + 3;

                symPointer++;
                namePointer = 0;

                for (int i = 0; i < 12; i++)
                  name[i] = 0;

                scanWord(input);

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

  printf("Token: %d\n", token);

  if (token == procsym)
  {
    do
    {
      numProc++;

      scanWord(input);

      if (token != ident)
        printf("ERROR: Ident\n");

      // Defaulted L = 0
      // Change this ASAP to actually work with lexLevel
      if (symTableCheck(token, 0) != -1)
        printf("ERROR: LexLevel\n");

      // procIdx = end of symbol table

      symbolTable[symPointer].kind = 3;
      symbolTable[symPointer].name =
    } while (token == procsym);
  }
  return 0;
}

// STATEMENT
// Returns 1 if everything goes smoothly
// Returns -1 if there's an error that requires the program to be terminated
int statement(int lexlevel)
{
  int index = 0, jpcIndex = 0, loopIndex = 0, jmpIndex = 0;

  switch(token)
  {
    case identsym:
      scanWord(input);

      index = symTableCheck(word);

      if (index == -1)
      {
        printf("Error : undeclared symbol\n");
        return -1;
      }

      if (symbolTable[index].kind != 2)
      {
        printf("Error : Not a var. Terminating program.\n");
        return -1;
      }

      // Get next token
      scanWord(input);

      if (token != becomessym)
      {
        printf("Error : assignment statements must use :=\n");
        return -1;
      }

      // Get next token
      scanWord(input);

      if (expression(input) == -1)
      {

        return -1;
      }

      emit(4, "STO", 0, symbolTable[index].addr);
      return 1;
    case callsym:
      scanWord(input);


      if (token != identsym)
      {
        printf("Error: const, var, procedure, call, and read keywords must be followed by identifier\n");
        return -1;
      }
      symPointer = symTableSearch(token, lexlevel, 3);
      if (symPointer == -1)
      {
        printf("Error\n");
        return -1;
      }
      scanWord(input);
      if (token == lparentsym)
      {
        scanWord(input);
        if (symbolTable[symPointer].param != 1)
        {
          printf("Error\n");
          return -1;
        }
        expression(lexlevel);
        if (token != rparentsym)
        {
          printf("Error\n");
          return -1;
        }
        scanWord(input);
      }
      else
      {
        emit(1, "LIT", 0, 0);
        // This is sketch
      }
      emit(5, "CAL", lexlevel - symboltable[symPointer].level, symboltable[symPointer].value);
    case returnsym:
      if (lexlevel == 0)
      {
        printf("Error\n");
        return -1;
      }
      scanWord(input);
      if (token == lparentsym)
      {
        scanWord(input);
        expression(lexlevel);
        emit(2, "OPR", 0, 0);
        if (token != rparentsym)
        {
          printf("Error\n");
          return -1;
        }
        scanWord(input);
      }
      else
      {
        emit(1, "LIT", 0, 0);
        emit(2, "OPR", 0, 0);
      }
    case beginsym:
      do
      {
        scanWord(input);
        if (statement(lexlevel) == -1) return -1;
      } while(token == semicolonsym);

      if (token != endsym)
      {
        printf("Error : begin must be followed by end %s\n", word);
        return -1;
      }

      scanWord(input);

      return 1;
    case ifsym:
      scanWord(input);

      condition(input);

      // jpcIndex <- current code index
      jpcIndex = linePointer;

      emit(8, "JPC", 0, 0);

      if (token != thensym)
      {
        printf("Error : if must be followed by then\n");
        return -1;
      }

      scanWord(input);

      statement(lexlevel);

      if (token == elsesym)
      {
        scanWord(input);
        jmpIndex = linePointer;
        emit(7,"JMP", 0, jmpIndex);
        code[jpcIndex].M = linePointer;
        statement(lexlevel);
        code[jmpIndex].M = linePointer;
      }
      else
        code[jpcIndex].M = linePointer;

      return 1;
    case whilesym:
      scanWord(input);

      loopIndex = linePointer;

      condition(input);

      if (token != dosym)
      {
        printf("Error : while must be followed by do\n");
        return -1;
      }

      scanWord(input);

      jpcIndex = linePointer;
      emit(8, "JPC", 0, 0);

      statement(lexlevel);

      emit(7, "JMP", 0, loopIndex);

      code[jpcIndex].M = linePointer;

      return 1;
    case readsym:
      scanWord(input);

      if (token != identsym)
      {
        printf("Error : const, var, and read keywords must be followed by identifier\n");
        return -1;
      }

      scanWord(input);
      index = symTableCheck(word);

      // Symbol not found
      if (index == -1)
      {
        printf("Error : undeclared symbol\n");
        return -1;
      }

      // Not a var
      if (symbolTable[index].kind != 2)
      {
        printf("Error : only variable values may be altered\n");
        return -1;
      }

      scanWord(input);

      emit(9, "SYS", 0, 2);
      emit(4, "STO", 0, symbolTable[index].addr);

      return 1;
    case writesym:
      scanWord(input);

      expression(input);

      emit(9, "SYS", 0, 1);

      return 1;
    default:
      if (token == endsym)
        return 1;

      return -1;
  }
}

// CONDITION
int condition(FILE *input)
{
  if (token == oddsym)
  {
    scanWord(input);

    if (expression(input) == -1) return -1;

    emit(2, "OPR", 0, 6);
  }
  else
  {
    if (expression(input) == -1) return -1;

    switch(token)
    {
      case eqlsym:
        scanWord(input);
        if (expression(input) == -1) return -1;
        emit(2, "OPR", 0, 8);
        break;
      case neqsym:
        scanWord(input);
        if (expression(input) == -1) return -1;
        emit(2, "OPR", 0, 9);
        break;
      case lessym:
        scanWord(input);
        if (expression(input) == -1) return -1;
        emit(2, "OPR", 0, 10);
        break;
      case leqsym:
        scanWord(input);
        if (expression(input) == -1) return -1;
        emit(2, "OPR", 0, 11);
        break;
      case gtrsym:
        scanWord(input);
        if (expression(input) == -1) return -1;
        emit(2, "OPR", 0, 12);
        break;
      case geqsym:
        scanWord(input);
        if (expression(input) == -1) return -1;
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
int expression(FILE *input)
{
  if (token == minussym)
  {
    scanWord(input);

    if (term(input) == -1)
    {
      return -1;
    }

    emit(2, "OPR", 0, 1);

    while (token == plussym || token == minussym)
    {
      if (token == plussym)
      {
        scanWord(input);
        if (term(input) == -1)
        {
          return -1;
        }
        emit(2, "OPR", 0, 2);
      }
      else
      {
        scanWord(input);
        if (term(input) == -1)
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
      scanWord(input);

    if (term(input) == -1)
    {
      return -1;
    }

    while (token == plussym || token == minussym)
    {
      if (token == plussym)
      {
        scanWord(input);
        if (term(input) == -1)
        {
          return -1;
        }
        emit(2, "OPR", 0, 2);
      }
      else
      {
        scanWord(input);
        if (term(input) == -1)
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
int term(FILE *input)
{
  if (fact(input) == -1)
  {
    return -1;
  }

  while (token == multsym || token == slashsym || token == modsym)
  {
    switch(token)
    {
      case multsym:
        scanWord(input);
        if (fact(input) == -1)
          return -1;
        emit(2, "OPR", 0, 4);
        break;
      case slashsym:
        scanWord(input);
        if (fact(input) == -1)
          return -1;
        emit(2, "OPR", 0, 5);
        break;
      case modsym:
        scanWord(input);
        if (fact(input) == -1)
          return -1;
        emit(2, "OPR", 0, 7);
        break;
      default:
        printf("Error : Unexpected token %d. Line %d.\n", token, __LINE__);
        return -1;
    }

  }

  return 1;
}

// FACTOR
int fact(int lexlevel)
{
  int result = 0;

  switch (token)
  {
    case identsym:
      //// This isn't in the pseudocode, just wait and see if it works

      scanWord(input);
      result = symTableCheck(word);

      if (result == -1)
      {
        printf("Error : undeclared symbol\n");
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

      scanWord(input);

      break;
    case numbersym:
      // Get next token
      scanWord(input);

      int num = token;

      emit(1, "LIT", 0, num);

      scanWord(input);
      break;
    case lparentsym:
      scanWord(input);

      if (expression(input) == -1) return -1;

      if (token != rparentsym)
      {
        printf("Error : right parenthesis must follow left parenthesis\n");
        return -1;
      }

      scanWord(input);
    case callsym:
      statement(lexlevel);
    default:
      printf("Error : Unexpected token %d. Line %d.\n", token, __LINE__);
      return -1;
  }

  return 1;
}


// Original functions, not specified in the pseudocode
// Mostly to help facilitate the above code

// Helper function to get next token
int scanWord(FILE *input)
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
      symbolTable[i].mark == 1;

      count--;
    }
  }

  return 0;
}

int symTableCheck(char ident[])
{
  for (int i = 0; i < symPointer; i++)
  {
    if (strcmp(ident, symbolTable[i].name) == 0)
    {
      return i;
    }
  }
  return -1;
}

int symTableSearch(char string[], int lexlevel, int kind)
{
  int index = -1;
  int mindiff = INT_MAX;
  for (int i = 0; i < 500; i++)
    if (strcmp(symbolTable[i].name, string) && symbolTable[i].kind == kind)
      if (symbolTable[i].mark == UNMARKED)
      {
        int diff = abs(symbolTable[i].level - lexlevel);
        if (diff < mindiff)
        {
          mindiff = diff;
          index = i;
        }
      }
  return index;
}

int findProcedure(int num)
{
  for (int i = 0; i < 500; i++)
    if (symbolTable[i].kind == 3 && symbolTable[i].val == num)
      return i;
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
