#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <stdlib.h>
#define $BEGIN	1
#define $END	2
#define $INTEGER	3
#define $IF	4
#define $THEN	5
#define $ELSE	6
#define $FUNCTION	7
#define $READ	8
#define $WRITE	9
#define $SYMBOL	10
#define $CONSTANT	11
#define $EQU	12
#define $NEQU	13
#define $LE	14
#define $L	15
#define $GE	16
#define $G	17
#define $SUB	18
#define $MUL	19
#define $ASSIGN	20
#define $LPAR	21
#define $RPAR	22
#define $SEM	23
#define $EOLN 24
#define $EOF 25
using namespace std;

string Name[] =  {"",
"begin",
"end",
"integer",
"if",
"then",
"else",
"function",
"read",
"write",
"SYMBOL",
"CONSTANT",
"=",
"<>",
"<=",
"<",
">=",
">",
"-",
"*",
":=",
"(",
")",
";",
"$EOLN",
"$EOF"
};

struct BinExp
{
	string val;
	int type;
	BinExp() {}
	BinExp(string s,int x){
		val = s;
		type = x;
	}
	void print(){
		cout << val <<" " << type <<endl;
	}
	void read(ifstream &fin){
		fin >> val >> type;
	}
};

struct Var
{
	string vname;
	string vproc;
	int vkind;
	int vtype;
	int vlev;
	int vadr;
	Var(){}
	Var(string _vname,string _vproc,int _vkind,int _vtype,int _vlev,int _vadr){
		vname = _vname;
		vproc = _vproc;
		vkind = _vkind;
		vtype = _vtype;
		vlev = _vlev;
		vadr = _vadr;
	}
};

struct Proc
{
	string pname;
	string ptype;
	string arg;
	int plev;
	int fadr;
	int ladr;
	Proc() { fadr=ladr=-1; }
	Proc(string _pname,string _ptype,string _arg,int _plev)
	{
		fadr=ladr=-1;
		pname = _pname;
		ptype = _ptype;
		arg = _arg;
		plev = _plev;
	}
};

struct Element
{
	string name;
	string val;
	int type;
	Element() {}
	Element(string _name){ name=_name; val=""; type=0; }
	Element(string _name,string _val,int _type){
		val = _val;
		name = _name;
		type = _type;
	}	
};

string fileName;
vector<BinExp> Bin;
map<string,Var> var;
map<string,Proc> proc;

string now_proc;
int now_lev;
int now_adr;
stack<string> proc_stk;
Element stk[1005];
int top;

int pos;
ofstream fout;
int line;
int lookahead;

bool Exe_Statement();
bool Con_Statement();
bool Proc_Body();
bool Proc_Invoke();
void printStack();

string getfileName(string s){
	int pos = s.find(".");
	return s.substr(0,pos);
}

void calcLine(){
	line=1;
	for (int i=0;i<lookahead;i++)
		if (Bin[i].type==$EOLN)
			line++;
}

void error(string msg){
	calcLine();
	fout << "***LINE:"<< line <<"  " << msg <<endl;
}

void error_expect(int x){
	calcLine();
	fout << "***LINE:"<< line <<"  " << Name[x]+" expected instead of "+Name[Bin[lookahead].type] <<endl;
}

void advance()
{
	if (Bin[lookahead].type!=$EOF)
		lookahead++;
	while (Bin[lookahead].type==$EOLN)
		line++,lookahead++;
}

bool match(int id)
{
	if (Bin[lookahead].type==id)
	{
		printStack();
		printf("Move_In : %s\n",Bin[lookahead].val.c_str());
		stk[top++]=Element(Name[id],Bin[lookahead].val,1);
		advance();
		return true;
	} else {
		if (id==$SEM&&Bin[lookahead].type==$END) return false;
		if (id==$SEM||id==$RPAR||id==$END){
			error_expect(id);
			printStack();
			printf("Move_In : %s\n",Name[id].c_str());
			stk[top++]=Element(Name[id],Name[id],1);
			return true;
		}
	}
	return false;
}

void printStack()
{
	printf("\nStack:\n");
	for (int i=0;i<top;i++){
		printf("#%d : %s",i,stk[i].name.c_str());
		if (stk[i].val!="")
			printf("(%s)",stk[i].val.c_str());
		printf("\n");
	}
	printf("\n");
}

bool Variable()
{
	if (match($SYMBOL))
	{
		printStack();
		top--;
		stk[top++]=Element("Variable",stk[top].val,0);
		printf("Reduce : <Variable>  =>  <SYMBOL>\n");
		return true;
	} else error_expect($SYMBOL);
	return false;
}

bool addProc(string name,string arg)
{
	if (proc.find(name)!=proc.end()){
		error("Multilply Declares of function "+name);
		return false;
	}
	proc_stk.push(now_proc);
	now_proc = name;	
	proc[name] = Proc(name,"integer",arg,now_lev);
	now_lev++;
	return true;
}

bool Argument()
{
	if (Variable())
	{
		printStack();
		top--;		
		stk[top++]=Element("Argument",stk[top].val,0);	
		if (!addProc(stk[top-3].val,stk[top-1].val)) return false;
		printf("Reduce : <Argument>  =>  <Variable>\n");
		return true;
	}
	return false;
}

void addVar(string name)
{
	if (var.find(name)!=var.end()){
		error("Multilply Declares of Variable "+name);
		return ;
	}
	var[name] = Var(name,now_proc,name==proc[now_proc].arg,0,now_lev,now_adr);
	if (proc[now_proc].fadr==-1)
		proc[now_proc].fadr=now_adr;
	now_adr++;
}

bool Dec_Var()
{
	if (match($INTEGER))
	{
		if (Variable()) {
			printStack();
			top-=2;			
			stk[top++]=Element("Dec_Var",stk[top+1].val,0);
			addVar(stk[top-1].val);
			printf("Reduce : <Dec_Var>  =>  integer<Variable>\n");
			return true;
		}
	} else error_expect($INTEGER);
	return false;
}

void checkVar(string name)
{
	if (var.find(name)==var.end()&&name!=now_proc)
		error(name+" is not defined");
}

void checkProc(string name)
{
	if (proc.find(name)==proc.end())
		error(name+" is not defined");
}

bool Read_Statement()
{
	if (match($READ))
	{
		if (match($LPAR))
		{
			if (!Variable()) return false;
		} 	else { error_expect($LPAR); return false;}
		if (match($RPAR))
		{
			checkVar(stk[top-2].val);
			printStack();
			top-=4;
			stk[top++]=Element("Read_Statement");
			printf("Reduce : <Read_Statement>  →  read(<Variable>)\n");
			return true;
		}	else { error_expect($RPAR); return false;}
	} else error_expect($READ);
	return false;
}

bool Write_Statement()
{
	if (match($WRITE))
	{
		if (match($LPAR))
		{
			if (!Variable()) return false;
		}	else { error_expect($LPAR); return false;}
		if (match($RPAR))
		{
			checkVar(stk[top-2].val);
			printStack();
			top-=4;
			stk[top++]=Element("Write_Statement");			
			printf("Reduce : <Write_Statement>  →  write(<Variable>)\n");
			return true;
		}	else { error_expect($RPAR);	return false;}
	} else error_expect($WRITE);
	return false;
}

bool Factor()
{
	if (Bin[lookahead].type==$CONSTANT)
	{
		if (match($CONSTANT))
		{
			printStack();
			top--;
			stk[top++]=Element("Factor",stk[top].val,0);
			printf("Reduce : <Factor>  =>  <CONSTANT>\n");
			return true;
		}
		return false;
	} else if (Bin[lookahead].type==$SYMBOL&&Bin[lookahead+1].type==$LPAR)
	{
		if (Proc_Invoke())
		{
			checkProc(stk[top-1].val);
			printStack();
			top--;
			stk[top++]=Element("Factor",stk[top].val,0);
			printf("Reduce : <Factor>  =>  <Proc_Invoke>\n");
			return true;
		} 
		return false;
	} else if (Bin[lookahead].type==$SYMBOL)
	{
		if (Variable())
		{
			checkVar(stk[top-1].val);
			printStack();
			top--;
			stk[top++]=Element("Factor",stk[top].val,0);		
			printf("Reduce : <Factor>  =>  <Variable>\n");
			return true;
		}
		return false;
	}
	error("CONSTANT or SYMBOL expected instead of "+Name[Bin[lookahead].type]);
	return false;
}

bool _Item()
{
	if (match($MUL))
	{
		if (Factor()&&_Item())
		{
			printStack();
			top-=3;
			stk[top++]=Element("_Item");
			printf("Reduce : <_Item>  =>  *<Factor><_Item>\n");
			return true;
		}
	} else {
		printStack();
		stk[top++]=Element("_Item");
		printf("Reduce : <_Item>  =>  null\n");	
		return true;
	}
	return false;
}

bool Item()
{
	if (!Factor()) return false;
	if (!_Item()) return false;
	printStack();
	top-=2;
	stk[top++]=Element("Item");
	printf("Reduce : <Item>  =>  <Factor><_Item>\n");
	return true;
}

bool _Arith_Expression()
{
	if (match($SUB))
	{
		if (Item()&&_Arith_Expression())
		{
			printStack();
			top-=3;
			stk[top++]=Element("_Arith_Expression");
			printf("Reduce : <_Arith_Expression>  =>  -<Item><_Arith_Expression>\n");
			return true;
		}
	} else 
	{
		printStack();
		stk[top++]=Element("_Arith_Expression");
		printf("Reduce : <_Arith_Expression> => null\n");
		return true;	
	}
	return false;
}

bool Arith_Expression()
{
	if (!Item()) return false;
	if (!_Arith_Expression()) return false;
	printStack();
	top-=2;
	stk[top++]=Element("Arith_Expression");
	printf("Reduce : <Arith_Expression>  =>  <Item><Arith_Expression>\n");
	return true;
}

bool Proc_Invoke()
{
	if (match($SYMBOL))
	{
		if (match($LPAR))
		{
			if (!Arith_Expression()) return false;
		} else { error_expect($LPAR); return false;}
		if (match($RPAR))
		{
			printStack();
			top-=4;
			stk[top++]=Element("Proc_Invoke",stk[top].val,0);
			printf("Reduce : <Proc_Invoke>  =>  <SYMBOL>(<Arith_Expression>)\n");
			return true;
		} else { error_expect($RPAR); return false;}
	} else error_expect($SYMBOL);
	return false;
}

bool Ass_Statement()
{
	if (!Variable()) return false;
	checkVar(stk[top-1].val);
	if (match($ASSIGN))
	{
		if (Arith_Expression()){
			printStack();
			top-=3;
			stk[top++]=Element("Ass_Statement",stk[top].val,0);
			printf("Reduce : <Ass_Statement>  =>  <Variable>:=<Arith_Expression>\n");
			return true;
		}
	} else { error_expect($ASSIGN); return false;}
	return false;
}

bool Real_Op()
{
	if (match($EQU))
	{
		printStack();
		top--;
		stk[top++]=Element("Real_Op",stk[top].val,0);
		printf("Reduce : <Real_Op>  =>  =\n");
		return true;
	} else
	if (match($L))
	{
		printStack();
		top--;
		stk[top++]=Element("Real_Op",stk[top].val,0);
		printf("Reduce : <Real_Op>  =>  <\n");
		return true;
	} else
	if (match($LE))
	{
		printStack();
		top--;
		stk[top++]=Element("Real_Op",stk[top].val,0);
		printf("Reduce : <Real_Op>  =>  <=\n");
		return true;
	} else
	if (match($G))
	{
		printStack();
		top--;
		stk[top++]=Element("Real_Op",stk[top].val,0);
		printf("Reduce : <Real_Op>  =>  >\n");
		return true;
	} else
	if (match($GE))
	{
		printStack();
		top--;
		stk[top++]=Element("Real_Op",stk[top].val,0);
		printf("Reduce : <Real_Op>  =>  >=\n");
		return true;
	} else
	if (match($NEQU))
	{
		printStack();
		top--;
		stk[top++]=Element("Real_Op",stk[top].val,0);
		printf("Reduce : <Real_Op>  =>  <>\n");
		return true;
	}
	error("< or <= or = or >= or > or <> expected instead of "+Name[Bin[lookahead].type]);
	return false;
}

bool Con_Expression()
{
	if (!Arith_Expression()) return false;
	if (!Real_Op()) return false;
	if (!Arith_Expression()) return false;
	printStack();
	top-=3;
	stk[top++]=Element("Con_Expression");
	printf("Reduce : <Con_Expression>  =>  <Arith_Expression><Real_Op><Arith_Expression>\n");
	return true;
}

bool Con_Statement()
{
	if (match($IF))
	{
		if (!Con_Expression()) return false;
		if (match($THEN))
		{
			if (!Exe_Statement()) return false;
			if (match($ELSE))
			{
				if (!Exe_Statement()) return false;
				printStack();
				top-=6;
				stk[top++]=Element("Con_Statement");
				printf("Reduce : <Con_Statement>  =>  if<Con_Expression>then<Exe_Statement>else<Exe_Statement>\n");
				return true;
			} else { error_expect($ELSE); return false;}
		} else { error_expect($THEN); return false;}
	} else { error_expect($IF); return false;}
	return false;
}

bool Exe_Statement()
{
	if (Bin[lookahead].type==$READ)
	{
		if (Read_Statement())
		{
			printStack();
			top--;
			stk[top++]=Element("Exe_Statement");
			printf("Reduce : <Exe_Statement>  =>  <Read_Statement>\n");
			return true;
		}
		return false;
	} else if (Bin[lookahead].type==$WRITE)
	{
		if (Write_Statement())
		{
			printStack();
			top--;
			stk[top++]=Element("Exe_Statement");
			printf("Reduce : <Exe_Statement>  =>  <Write_Statement>\n");
			return true;
		}
		return false;
	} else if (Bin[lookahead].type==$SYMBOL)
	{
		if (Ass_Statement())
		{
			printStack();
			top--;
			stk[top++]=Element("Exe_Statement");
			printf("Reduce : <Exe_Statement>  =>  <Ass_Statement>\n");
			return true;
		}
		return false;
	} else if (Bin[lookahead].type==$IF)
	{
		if (Con_Statement())
		{
			printStack();
			top--;
			stk[top++]=Element("Exe_Statement");
			printf("Reduce : <Exe_Statement>  =>  <Con_Statement>\n");
			return true;
		}
		return false;
	}
	error("read or write or SYMBOL or if expected instead of "+Name[Bin[lookahead].type]);
	return false;
}

bool _Exe_Statement_List()
{
	if (match($SEM))
	{
		if (Exe_Statement()&&_Exe_Statement_List()){
			printStack();
			top-=3;
			stk[top++]=Element("_Exe_Statement_List");
			printf("Reduce : <_Exe_Statement_List>  =>  ;<Exe_Statement><_Exe_Statement_List>\n");
			return true;
		}
	} else 
	{
		printStack();
		stk[top++]=Element("_Exe_Statement_List");
		printf("Reduce : <_Exe_Statement_List>  =>  null\n");
		return true;
	}
	return false;	
}

bool Exe_Statement_List()
{
	if (!Exe_Statement()) return false;
	if (!_Exe_Statement_List()) return false;
	printStack();
	top-=2;
	stk[top++]=Element("Exe_Statement_List");
	printf("Reduce : <Exe_Statement_List>  =>  <Exe_Statement><_Exe_Statement_List>\n");
	return true;
}

bool Dec_Proc()
{
	if (match($INTEGER))
	{
		if (match($FUNCTION))
		{
			if (match($SYMBOL))
			{
				if (match($LPAR))
				{
					if (!Argument()) return false;
					if (match($RPAR))
					{
						if (match($SEM))
						{
							if (Proc_Body())
							{
								printStack();
								top-=8;
								stk[top++]=Element("Dec_Proc",stk[top+2].val,0);
								proc[now_proc].ladr = now_adr-1;
								now_proc = proc_stk.top();								
								proc_stk.pop();
								now_lev--;
								printf("Reduce : <Dec_Proc>  =>  integer function <SYMBOL>(<Argument>);<Proc_Body>\n");
								return true;
							}
						} else { error_expect($SEM); return false;}
					} else { error_expect($RPAR); return false;}
				} else { error_expect($LPAR); return false;}
			} else { error_expect($SYMBOL); return false;}
		} else { error_expect($FUNCTION); return false;}
	} else { error_expect($INTEGER); return false;}
	return false;
}

bool Dec_Statement()
{
	if (Bin[lookahead].type==$INTEGER&&Bin[lookahead+1].type==$FUNCTION)
	{
		if (Dec_Proc()){
			printStack();
			top--;
			stk[top++]=Element("Dec_Statement");
			printf("Reduce : <Dec_Statement>  =>  <Dec_Proc>\n");
			return true;
		}	
		return false;
	} else if (Bin[lookahead].type==$INTEGER)
	{
		if (Dec_Var())
		{
			printStack();
			top--;
			stk[top++]=Element("Dec_Statement");
			printf("Reduce : <Dec_Statement>  =>  <Dec_Var>\n");
			return true;
		} 
		return false;
	}
	error("integer expected instead of "+Name[Bin[lookahead].type]);
	return false;
}

int find_next(int pos)
{
	while (Bin[pos].type==$EOLN)
		pos++;
	return pos;
}

bool _Dec_Statement_List()
{
	if (((Bin[lookahead].type==$SEM||true)&&Bin[find_next(lookahead+1)].type==$INTEGER)||(Bin[lookahead].type==$INTEGER))
	{
		match($SEM);
		if (Dec_Statement()&&_Dec_Statement_List())
		{
			printStack();
			top-=3;
			stk[top++]=Element("_Dec_Statement_List");
			printf("Reduce : <_Dec_Statement_List>  =>  ;<Dec_Statement><_Dec_Statement_List>\n");
			return true;
		}
	} else 
	{
		printStack();
		stk[top++]=Element("_Dec_Statement_List");
		printf("Reduce : <_Dec_Statement_List>  =>  null\n");
		return true;	
	}
	return false;
}

bool Dec_Statement_List()
{
	if (!Dec_Statement()) return false;
	if (!_Dec_Statement_List()) return false;
	printStack();
	top-=2;
	stk[top++]=Element("Dec_Statement_List");
	printf("Reduce : <Dec_Statement_List>  =>  <Dec_Statement><_Dec_Statement_List>\n");
	return true;
}

bool Proc_Body()
{
	if (match($BEGIN))
	{
		if (!Dec_Statement_List()) return false;
		if (match($SEM))
		{
			if (!Exe_Statement_List()) return false;
			if (match($END))
			{
				printStack();
				top-=5;
				stk[top++]=Element("Proc_Body");
				printf("Reduce : <Proc_Body>  =>  begin <Dec_Statement_List>;<Exe_Statement_List> end\n");
				return true;
			} { error_expect($END); return false;}
		} else { error_expect($SEM); return false;}
	}{ error_expect($BEGIN); return false;}
	return false;
}

bool Program()
{
	if (match($BEGIN))
	{
		if (!Dec_Statement_List()) return false;
		if (match($SEM))
		{
			if (!Exe_Statement_List()) return false;
			if (match($END))
			{
				printStack();
				top-=5;
				stk[top++]=Element("Program");
				proc[now_proc].ladr=now_adr-1;
				printf("Reduce : <Program>  =>  begin <Dec_Statement_List>;<Exe_Statement_List> end\n");
				return true;
			} { error_expect($END); return false;}
		} else { error_expect($SEM); return false;}
	} else { error_expect($BEGIN); return false;}
	return false;
}

void printVar(){
	freopen((fileName+".var").c_str(),"w",stdout);
	map<string,Var> :: iterator it;
	printf("vname    vproc    vkind   vtype   vlev   vadr\n");
	for (it = var.begin();it!=var.end();it++){
		Var tmp = it->second;
		printf("%s        %s       %d       %d      %d      %d\n",tmp.vname.c_str(),tmp.vproc.c_str(),tmp.vkind,tmp.vtype,tmp.vlev,tmp.vadr);
	}
}

void printProc(){
	freopen((fileName+".pro").c_str(),"w",stdout);
	map<string,Proc> :: iterator it;
	printf("pname    ptype       plev   fadr   ladr\n");
	for (it = proc.begin();it!=proc.end();it++){
		Proc tmp = it->second;
		printf("%s     %s     %d     %d      %d\n",tmp.pname.c_str(),tmp.ptype.c_str(),tmp.plev,tmp.fadr,tmp.ladr);
	}	
}


int main(int arg,char **args)
{	
	fileName = getfileName(string(args[1]));
	fout.open((fileName+".yacc.err").c_str());
	ifstream fin(args[1]);

	freopen((fileName+".dys").c_str(),"w",stdout);

	if (!fin.is_open()){
		cout << "can't open file!" <<endl;
		return 0;
	} else {
		while (!fin.eof()){
			BinExp tmp;
			tmp.read(fin);
			tmp.print();
			Bin.push_back(tmp);
			if (tmp.type==$EOF)
				break;
		}
		freopen((fileName+".stk").c_str(),"w",stdout);
		top=lookahead = 0;
		while (Bin[lookahead].type==$EOLN) lookahead++;
		now_proc = "main";
		now_adr = 0;
		now_lev = 0;
		proc[now_proc]=Proc("main","procedure","",now_lev);
		Program();
		printStack();
		printVar();
		printProc();
	}
	return 0;
}