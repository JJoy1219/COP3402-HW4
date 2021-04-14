// Authored by: Daniel Hernandez-Otero, Joel Joy

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include "compiler.h"

#define MAX_STACK_HEIGHT 50
#define MAX_CODE_LENGTH 100
#define EMPTY (-1)
#define STACK_EMPTY INT_MIN

int stack[MAX_STACK_HEIGHT];
int counter = 0, BP = 0, SP = -1;

bool push(int);
int pop();
int base(int[], int, int);

typedef struct instruction
{
  int opcode;
  char *op;
  int L;
  int M;
} instruction;

int execute(int print)
{
  FILE *input = fopen("Output/parseOutput.txt", "r");
  int instruction[MAX_CODE_LENGTH][3];

  struct instruction line;

  int halt = 1;

  while (!feof(input))
  {
    // This loops through the entire line and increases the counter afterwards
    for (int i = 0; i < 3; i++)
    {
      fscanf(input, "%d", &instruction[counter][i]);
    }

    counter++;
  }


  // Reading input is over, now convert it into ISA:
  if (print)
  {
    printf("\t\t\tPC\tBP\tSP\tstack\n");
    printf("Initial values:\t\t0\t%d\t%d\n", BP, SP);
  }

  for (int PC = 0; PC < counter; PC++)
  {
    // Counter is separate from PC to make printing output easier
    int userInput = 0;
    int counter = PC;

    line.L = instruction[PC][1];
    line.M = instruction[PC][2];

    switch(instruction[PC][0])
    {
      case 1:
        line.op = "LIT";
        SP ++;
        push(line.M);

        break;
      case 2:

        // Do some stuff here with operations based on line.M:
        // 0 -> RET
        // 1 -> NEG
        // 2 -> ADD
        // 3 -> SUB
        // 4 -> MUL
        // 5 -> DIV
        // 6 -> ODD
        // 7 -> MOD
        // 8 -> EQL
        // 9 -> NEQ
        // 10 -> LSS
        // 11 -> LEQ
        // 12 -> GTR
        // 13 -> GEQ
        switch (line.M)
        {
          case 0:
            line.op = "RTN";
            stack[BP - 1] = stack[SP];
            SP = BP - 1;
            BP = stack[SP + 2];
            PC = stack[SP + 3] - 1;
            break;
          case 1:
            line.op = "NEG";
            stack[SP] = -1 * stack[SP];
            break;
          case 2:
            line.op = "ADD";
            SP -= 1;
            stack[SP] = stack[SP] + stack[SP + 1];
            break;
          case 3:
            line.op = "SUB";
            SP -= 1;
            stack[SP] = stack[SP] - stack[SP + 1];
            break;
          case 4:
            line.op = "MUL";
            SP -= 1;
            stack[SP] = stack[SP] * stack[SP + 1];
            break;
          case 5:
            line.op = "DIV";
            SP -= 1;
            stack[SP] = stack[SP] / stack[SP + 1];
            break;
          case 6:
            line.op = "ODD";
            stack[SP] = stack[SP] % 2;
            break;
          case 7:
            line.op = "MOD";
            SP -= 1;
            stack[SP] = stack[SP] % stack[SP + 1];
            break;
          case 8:
            line.op = "EQL";
            SP -= 1;
            stack[SP] = stack[SP] == stack[SP + 1];
            break;
          case 9:
            line.op = "NEQ";
            SP -= 1;
            stack[SP] = stack[SP] != stack[SP + 1];
            break;
          case 10:
            line.op = "LSS";
            SP -= 1;
            stack[SP] = stack[SP] < stack[SP + 1];
            break;
          case 11:
            line.op = "LEQ";
            SP -= 1;
            stack[SP] = stack[SP] <= stack[SP + 1];
            break;
          case 12:
            line.op = "GTR";
            SP -= 1;
            stack[SP] = stack[SP] > stack[SP + 1];
            break;
          case 13:
            line.op = "GEQ";
            SP -= 1;
            stack[SP] = stack[SP] >= stack[SP + 1];
            break;
          default:
            line.op = "ERR";
            break;
        }

        break;
      case 3:
        line.op = "LOD";
        SP++;
        stack[SP] = stack[base(stack, line.L, BP) + line.M];

        break;
      case 4:
        line.op = "STO";
        stack[base(stack, line.L, BP) + line.M] = stack[SP];
        SP--;

        break;
      case 5:
        line.op = "CAL";

        stack[SP + 1] = base(stack, line.L, BP);
        stack[SP + 2] = BP;
        stack[SP + 3] = PC + 1;
        stack[SP + 4] = stack[SP];

        BP = SP + 1;
        PC = line.M - 1;

        break;
      case 6:
        line.op = "INC";

        // sp <- sp + line.M
        SP += line.M;

        break;
      case 7:
        line.op = "JMP";

        // pc <- line.M, adjusted for arrays being indexed at 0
        PC = line.M - 1;

        break;
      case 8:
        // pc <- line.M iff top of stack == 0
        if (stack[SP] == 0)
          PC = line.M - 1;

        SP--;

        line.op = "JPC";

        break;
      case 9:

        line.op = "SYS";

        // SYS's function varies based on M
        // 1 -> Display top of stack, pop
        // 2 -> Push user input number onto the stack
        // 3 -> Halt program
        switch (line.M)
        {
          case 1:
            if (print)
              printf("Top of Stack Value: %d\n", pop(stack));
            break;
          case 2:
            printf("Please Enter an Integer: ");
            scanf("%d", &userInput);

            SP++;
            push(userInput);

            break;
          case 3:
            halt = 0;
            break;
          default:
            break;
        }

        break;
      default:
        break;
    }

    // Main output here: Line #, operation, L, M, next PC, BP, & SP in that order
    if (print)
      printf("%d\t%s %d %d \t%d \t%d \t%d", counter, line.op, line.L, line.M, PC + 1, BP, SP);


    // This prints out the stack(s) at the end of each line
    if (SP > -1)
    {
      if (print)
        printf("\t");

      for (int i = 0; i <= SP; i++)
      {
        // Separate stacks from each other
        if (i == BP && BP != 0)
          if (print)
            printf("| ");

        if (print)
          printf("%d ", stack[i]);
      }
    }

    if (print)
      printf("\n");

    // Only applicable when SYS 0 9 is called
    if (halt == 0)
      break;

  }

  // Close the FILE to save resources
  fclose(input);

  return 0;
}

bool push(int value) {
  if(SP >= MAX_STACK_HEIGHT - 1)
    return false;

  stack[SP] = value;

  return true;
}

int pop() {
  if (SP == EMPTY)
    return STACK_EMPTY;

  int result = stack[SP];
  SP--;

  return result;
}

int base(int stack[], int level, int BP)
{
  int base = BP;
  int counter = level;

  while (counter > 0)
  {
    base = stack[base];
    counter--;
  }

  return base;
}
