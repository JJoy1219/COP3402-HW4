// Authored by: Daniel Hernandez-Otero, Joel Joy

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

#include "compiler.h"

typedef enum token_type{
modsym = 1,
identsym,
numbersym,
plussym,
minussym,
multsym,
slashsym,
oddsym,
eqsym,
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
} token_type;

int CheckTokenNum(char*);
bool EndOfWord(char);
bool IsReserved(char*);
bool IsVar(char*);
bool checkNumLength(char*);


bool currentlyVar = false;

char *reservedWords[] = {
  "const",
  "var",
  "procedure",
  "call",
  "if",
  "then",
  "else",
  "while",
  "do",
  "read",
  "write",
  "odd",
  "return",
  "begin",
  "end"
};

char *varWords[11];

int lex_analyze(char *args, int print)
{
  // initialize varWords
  for (int i = 0; i < 15; i++)
  {
    varWords[i] = malloc(11 * sizeof(char));
  }


  char character;
  char futureChar;

  char *tokenList = malloc(200 * sizeof(char));
  char *conversion = malloc(sizeof(char));
  char word[50];

  int wordCounter = 0, varCount = 0, numCount = 0;

  bool isComment = false, isNum = false, begin = false, invalidIdent = false, maxDigit = false;

  FILE *input = fopen(args, "r");
  FILE *output = fopen("Output/lexOutput.txt", "w");
  FILE *listOutput = fopen("Output/tokenList.txt", "w");

  memset(word, 0, sizeof(word));

  if (print)
    printf("Lexeme Table:\n\tLexeme\t\ttoken type\n");

  fprintf(output, "Lexeme Table:\n\tLexeme\t\ttoken type\n");

  while (!feof(input))
  {
    int token = 0;

    fscanf(input, "%c", &character);

    if((iscntrl(character) || EndOfWord(character)) && wordCounter > 0)
    {
      wordCounter = 0;

      if (!isComment && !invalidIdent)
      {
        // This is a reserved word
        if (IsReserved(word))
        {
          //printf("Reserved word: %s\n", word);
          if (strcmp(word, "begin") == 0)
            begin = true;
        }
        else if (!isdigit(character))
        {
          // This is a reserved variable already
          if (varCount > 0 && IsVar(word))
          {
            //printf("Reserved var: %s\n", word);
          }
          else if (!(strcmp(word, " ") == 0))
          {
            // This is a new variable
            strcpy(varWords[varCount], word);
            varCount++;
            if (strlen(word) > 11 && !maxDigit)
              printf("Error : Identifier names cannot exceed 11 characters\n");
          }
        }



        // Here comes the big ladder
        token = CheckTokenNum(word);
        if (!(strlen(word) > 11)&&!(checkNumLength(word) && strlen(word) > 5))
        {
          // Output to tscreen and lexOuput.txt
          if (print)
            printf("%10s\t\t%d\n", word, token);
          fprintf(output, "%8s\t\t%d\n", word, token);

          sprintf(conversion,"%d ", token);
          strncat(tokenList, conversion, strlen(conversion));
          //printf("Adding %s to tokenList\n", conversion);
          if (token == 2 || token == 3)
          {
            strncat(tokenList, word, strlen(word));
            strncat(tokenList, " ", 1);
            //printf("Adding %s to tokenList\n", word);
          }
        }



        if (strcmp(word, "end") == 0)
        {
          char lastChar[2];

          memset(lastChar, 0, sizeof(lastChar));

          lastChar[0] = character;

          token = CheckTokenNum(lastChar);

          if (print)
            printf("%10c\t\t%d\n", character, token);
          fprintf(output, "%8c\t\t%d\n", character, token);

          sprintf(conversion,"%d ", token);
          strncat(tokenList, conversion, strlen(conversion));
          begin = false;
          //printf("Adding %s to tokenList\n", conversion);
          break;
        }

        // Reset word
        memset(word, 0, sizeof(word));
        isNum = false;

        // Recognize special character
        if (EndOfWord(character) && character != ' ')
        {
          char special[2];

          memset(special, 0, sizeof(special));

          special[0] = character;

          token = CheckTokenNum(special);

          if (print)
            printf("%10s\t\t%d\n", special, token);
          fprintf(output, "%8s\t\t%d\n", special, token);
          sprintf(conversion,"%d ", token);
          strncat(tokenList, conversion, strlen(conversion));
          //printf("Adding %s to tokenList\n", conversion);
        }

      }
      else
      {
        invalidIdent = false;
        isNum = false;
        maxDigit = false;

        // Reset word
        memset(word, 0, sizeof(word));
      }
    }
    else if (wordCounter == 0 && EndOfWord(character) && character != ' ')
    {
      // Handle special, SPECIAL characters here
      // like /*, */, >=, := etc.

      char recent[5];

      memset(recent, 0, sizeof(recent));

      if (!isComment)
      {
        if (character == '/')
        {
          fscanf(input, "%c", &futureChar);
          if (futureChar == '*')
            isComment = true;
            else
            {
              recent[0] = character;
              recent[1] = 0;

              token = CheckTokenNum(recent);

              if (print)
                printf("%10s\t\t%d\n", recent, token);

              sprintf(conversion, "%d ", token);
              strncat(tokenList, conversion, strlen(conversion));

              fseek(input, -1, SEEK_CUR);
            }
        }
        else if (character == '*')
        {
          fscanf(input, "%c", &futureChar);
          if (futureChar == '/')
            isComment = false;
          else
          {
            recent[0] = character;
            recent[1] = 0;

            token = CheckTokenNum(recent);

            if (print)
              printf("%10c\t\t%d\n", character, token);
            sprintf(conversion,"%d ", token);
            strncat(tokenList, conversion, strlen(conversion));
            //printf("Adding %s to tokenList\n", conversion);
            fseek(input, -1, SEEK_CUR);
          }
        }
        else if (character == ':')
        {
          fscanf(input, "%c", &futureChar);
          if (futureChar == '=')
          {
            recent[0] = character;
            recent[1] = futureChar;

            token = CheckTokenNum(recent);

            if (print)
              printf("%10s\t\t%d\n", recent, token);

            sprintf(conversion,"%d ", token);
            strncat(tokenList, conversion, strlen(conversion));
            //printf("Adding %s to tokenList\n", conversion);
          }
          else
            fseek(input, -1, SEEK_CUR);
        }
        else if (character == '<')
        {
          fscanf(input, "%c", &futureChar);
          if (futureChar == '>' || futureChar == '=')
          {
            recent[0] = character;
            recent[1] = futureChar;

            token = CheckTokenNum(recent);

            if (print)
              printf("%10s\t\t%d\n", recent, token);
            sprintf(conversion,"%d ", token);
            strncat(tokenList, conversion, strlen(conversion));
            //printf("Adding %s to tokenList\n", conversion);
          }
          else
          {
            // This is in fact a less than symbol
            recent[0] = character;
            recent[1] = 0;

            token = CheckTokenNum(recent);

            if (print)
              printf("%10s\t\t%d\n", recent, token);

            sprintf(conversion, "%d ", token);
            strncat(tokenList, conversion, strlen(conversion));

            fseek(input, -1, SEEK_CUR);
          }
        }
        else if (character == '>')
        {
          fscanf(input, "%c", &futureChar);
          if (futureChar == '=')
          {
            recent[0] = character;
            recent[1] = futureChar;

            token = CheckTokenNum(recent);

            if (print)
              printf("%10s\t\t%d\n", recent, token);
            sprintf(conversion,"%d ", token);
            strncat(tokenList, conversion, strlen(conversion));
            //printf("Adding %s to tokenList\n", conversion);
          }
          else
          {
            // This is in fact a greater than symbol
            recent[0] = character;
            recent[1] = 0;

            token = CheckTokenNum(recent);

            if (print)
              printf("%10s\t\t%d\n", recent, token);

            sprintf(conversion, "%d ", token);
            strncat(tokenList, conversion, strlen(conversion));

            fseek(input, -1, SEEK_CUR);
          }
        }
        else
        {
          recent[0] = character;

          token = CheckTokenNum(recent);

          if (print)
            printf("%10s\t\t%d\n", recent, token);
          sprintf(conversion,"%d ", token);
          strncat(tokenList, conversion, strlen(conversion));
          //printf("Adding %s to tokenList\n", conversion);
        }
      }

    }
    else
    {
      // Add to the word here
      if (isalpha(character))
      {
        if (isdigit(word[wordCounter-1]) && !invalidIdent && !maxDigit)
        {
          printf("Error : Identifiers cannot begin with a digit\n");
          invalidIdent = true;

          word[wordCounter] = character;
          wordCounter++;
        }
        else
        {
          word[wordCounter] = character;
          wordCounter++;
        }

      }
      else if (isdigit(character))
      {
        word[wordCounter] = character;

        if (isNum && wordCounter > 4 && !maxDigit)
        {
          printf("Error : Numbers cannot exceed 5 digits\n");
          maxDigit = true;
        }

        if (wordCounter == 0)
          isNum = true;

        wordCounter++;
      }
      else if (!iscntrl(character) && character != ' ')
      {
        if (begin)
        {
          if (print)
            printf("%10c\t\t3\n", character);
          sprintf(conversion,"%d ", 3);
          strncat(tokenList, conversion, strlen(conversion));
          strncat(tokenList, &character, 1);
          strncat(tokenList, " ", 1);
          //printf("Adding %s to tokenList\n", conversion);
        }
        else
        {
          printf("Error : Invalid Symbol\n");
        }
      }
    }

    // ERROR HANDLING HERE

  }

  if (print)
    printf("Token List:\n%s\n",tokenList);
  fprintf(listOutput, "%s", tokenList);

  fclose(input);
  fclose(output);
  fclose(listOutput);

  return 0;
}

bool EndOfWord(char character)
{
  char specialCharacters[] = {
    ' ',
    '+',
    '-',
    '*',
    '/',
    '(',
    ')',
    '=',
    ',',
    '.',
    '<',
    '>',
    ';',
    ':',
    '%'
  };

  for (int i = 0; i < strlen(specialCharacters); i++)
  {
    if (character == specialCharacters[i])
      return true;
  }

  return false;
}

bool checkNumLength(char *word)
{
  bool validNum = true;
  int limit = 5;
  if (strlen(word) < limit)
    limit  = strlen(word);
  for (int i = 0;i < limit; i++)
  {
    if (!isdigit(word[i]))
      validNum = false;
  }
  return validNum;
}

bool IsReserved(char *word)
{
  for (int i = 0; i < 15; i++)
  {
    if (strcmp(word, reservedWords[i]) == 0)
      return true;
  }

  return false;
}

bool IsVar(char *word)
{
  for (int i = 0; i < 15; i++)
  {
    //printf("\t\t%d. %s\n", i+1, varWords[i]);
    if (strcmp(word, varWords[i]) == 0)
      return true;
  }

  return false;
}

int CheckTokenNum(char *word)
{
  // this is really ugly I'm not a fan of this but idk what else to do don't sue me
  if (strcmp(word, "%") == 0)
    return 1;
  else if (strcmp(word, "+") == 0)
    return 4;
  else if (strcmp(word, "-") == 0)
    return 5;
  else if (strcmp(word, "*") == 0)
    return 6;
  else if (strcmp(word, "/") == 0)
    return 7;
  else if (strcmp(word, "odd") == 0)
    return 8;
  else if (strcmp(word, "=") == 0)
    return 9;
  else if (strcmp(word, "<>") == 0)
    return 10;
  else if (strcmp(word, "<") == 0)
    return 11;
  else if (strcmp(word, "<=") == 0)
    return 12;
  else if (strcmp(word, ">") == 0)
    return 13;
  else if (strcmp(word, ">=") == 0)
    return 14;
  else if (strcmp(word, "(") == 0)
    return 15;
  else if (strcmp(word, ")") == 0)
    return 16;
  else if (strcmp(word, ",") == 0)
    return 17;
  else if (strcmp(word, ";") == 0)
  {
    currentlyVar = false;
    return 18;
  }
  else if (strcmp(word, ".") == 0)
    return 19;
  else if (strcmp(word, ":=") == 0)
    return 20;
  else if (strcmp(word, "begin") == 0)
    return 21;
  else if (strcmp(word, "end") == 0)
    return 22;
  else if (strcmp(word, "if") == 0)
    return 23;
  else if (strcmp(word, "then") == 0)
    return 24;
  else if (strcmp(word, "while") == 0)
    return 25;
  else if (strcmp(word, "do") == 0)
    return 26;
  else if (strcmp(word, "call") == 0)
    return 27;
  else if (strcmp(word, "const") == 0)
    return 28;
  else if (strcmp(word, "var") == 0)
  {
    currentlyVar = true;
    return 29;
  }
  else if (strcmp(word, "procedure") == 0)
    return 30;
  else if (strcmp(word, "write") == 0)
    return 31;
  else if (strcmp(word, "read") == 0)
    return 32;
  else if (strcmp(word, "else") == 0)
    return 33;
  else if (strcmp(word, "return") == 0)
    return 34;
  else
  {
    if (currentlyVar || IsVar(word))
    {
      if (checkNumLength(word))
        return 3;
      return 2;
    }

    if (checkNumLength(word))
      return 3;

  }

  printf("For some reason, I couldn't find anything for %s", word);
  return -1;
}
