#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "compiler.h"


typedef struct
{
  int kind;
  char name[12];
  int val;
  int level;
  int addr;
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

symbol symbol_table[500];
int symPointer = 0;

char word[12];
int wordPointer = 0;

// Token <- atoi(word)
int token = 0;


// HW 3 pseudocode functions
int parse(int);
int program(FILE*, int);
int block(FILE*);
int declConst(FILE*);
int declVar(FILE*);
int statement(FILE*);
int condition(FILE*);
int expression(FILE*);
int term(FILE*);
int fact(FILE*);

// Helper functions here
int scanWord(FILE*);
int symTableCheck(char[]);
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


int block (FILE *input)
{
  int constResult = declConst(input);

  int varNum = declVar(input);

  emit(6, "INC", 0, (4+varNum));

  // STATEMENT
  scanWord(input);
  int result = statement(input);

  return result;
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

      symbol_table[symPointer].kind = 1;
      strcpy(symbol_table[symPointer].name, identName);
      symbol_table[symPointer].val = token;
      symbol_table[symPointer].level = 0;
      symbol_table[symPointer].addr = 0;

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

                symbol_table[symPointer].kind = 2;
                strcpy(symbol_table[symPointer].name, name);
                symbol_table[symPointer].val = 0;
                symbol_table[symPointer].level = 0;
                symbol_table[symPointer].addr = numVars + 3;

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

// STATEMENT
// Returns 1 if everything goes smoothly
// Returns -1 if there's an error that requires the program to be terminated
int statement(FILE *input)
{
  int index = 0, jpcIndex = 0, loopIndex = 0;

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

      if (symbol_table[index].kind != 2)
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

      emit(4, "STO", 0, symbol_table[index].addr);
      return 1;
    case beginsym:
      do
      {
        scanWord(input);
        if (statement(input) == -1) return -1;
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

      statement(input);

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

      statement(input);

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
      if (symbol_table[index].kind != 2)
      {
        printf("Error : only variable values may be altered\n");
        return -1;
      }

      scanWord(input);

      emit(9, "SYS", 0, 2);
      emit(4, "STO", 0, symbol_table[index].addr);

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
int fact(FILE *input)
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

      if (symbol_table[result].kind == 1)
      {
        // Constant
        emit(1, "LIT", 0, symbol_table[result].val);
      }
      else if (symbol_table[result].kind == 2)
      {
        // Variable
        emit(3, "LOD", 0, symbol_table[result].addr);
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

int symTableCheck(char ident[])
{
  for (int i = 0; i < symPointer; i++)
  {
    if (strcmp(ident, symbol_table[i].name) == 0)
    {
      return i;
    }
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
