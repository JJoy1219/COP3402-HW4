int procedureCount = 0;


typedef struct
{
	int kind;
	char name[10];
	int val;
	int level;
	int addr;
	int mark;
	int param;
}symbol;

symbol symboltable[MAX_SYMBOL_TABLE_SIZE = 500];

int symbolTableCheck(char string[10], int lexlevel)
{
	for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
		if (symboltable[i].level == lexlevel && strcmp(symboltable[i].name, string))
			return i;
	return -1;
}

int findProcedure(int num)
{
	for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
		if (symboltable[i].kind == 3 && symboltable[i].val == num)
			return i;
}

int program(FILE* input)
{
  int numProc = 1;
  JMP;

  for (int i = 0; i < lexeme; i++)
  {
  	if (lexeme.type == proceduresym)
  	{
  		numProc++;
  		JMP;
  	}
  }

  procedureCount++;
  block(0,0,0);
  if (token != '.')
  	ERROR;

  for (int i = 0; i < numProc; i++)
  	code[i].m = symboltable[findProcedure(i)].addr;
  HALT;
}

int block(int lexlevel, int param, int procedureIndex)
{
	int c = constantDeclaration(lexlevel);
	int v = varDeclaration(lexlevel);
	int p = procedureDeclaration(lexlevel);

	symboltable[procedureIndex].addr = currentCodeIndex;
	INC(M = 4 + v);
	statement(lexlevel);

	if (param == 1)
		mark(c + v + p + 1);
	else
		mark(c + v + p);
}

constantDeclaration(lexlevel)
{
	int numConst = 0;
	
	if (token == const)
	{
		do
		{
			numConst++;
			get next token;
			if (token != ident)
				ERROR;
			if (symbolTableCheck(token,lexlevel) != -1)
				ERROR;

		}
		while (token == ',');

		if (token != ';')
			ERROR;
		get next token
	}


	return numConst;
}
