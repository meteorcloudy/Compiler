#include <iostream>
#include <fstream>
#include <string>
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
"$BEGIN",
"$END",
"$INTEGER",
"$IF",
"$THEN",
"$ELSE",
"$FUNCTION",
"$READ",
"$WRITE",
"$SYMBOL",
"$CONSTANT",
"$EQU",
"$NEQU",
"$LE",
"$L",
"$GE",
"$G",
"$SUB",
"$MUL",
"$ASSIGN",
"$LPAR",
"$RPAR",
"$SEM",
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
};

string fileName;
string code;
int pos;
ofstream fout;
int line;

string getfileName(string s){
	int pos = s.find(".");
	return s.substr(0,pos);
}


bool isletter(char x){
	if (x>='a'&&x<='z') return true;
	if (x>='A'&&x<='Z') return true;
	if (x=='_') return true;
	return false;
}

bool isdigit(char x){
	return (x>='0')&&(x<='9');
}

int getState(int now,char x){
	switch (now){
		case 0 :
			if (x=='\n'||x==' '||x=='\t') return 0;			
			if (isletter(x)) return 1;
			if (isdigit(x)) return 3;
			switch(x)
			{
				case '=' : return 5;
				case '-' : return 6;
				case '*' : return 7;
				case '(' : return 8;
				case ')' : return 9;
				case '<' : return 10;
				case '>' : return 14;
				case ':' : return 17;
				case ';' : return 20;
				default : return 21;
			}
		case 1 :
			if (isletter(x)||isdigit(x)) return 1;
			return 2;
		case 3 :
			if (isdigit(x)) return 3;
			return 4;
		case 10 : 
			switch (x){
				case '=' : return 11;
				case '>' : return 12;
				default : return 13;
			}
			break;
		case 14 :
			switch (x){
				case '=' : return 15;
				default : return 16;
			}
			break;
		case 17 :
			switch (x){
				case '=' : return 18;
				default : return 19;
			}
			break;
	}
	return -1;
}

int isReserve(string s){
	if (s=="begin") return $BEGIN;
	if (s=="end") return $END;
	if (s=="integer") return $INTEGER;
	if (s=="if") return $IF;
	if (s=="then") return $THEN;
	if (s=="else") return $ELSE;
	if (s=="function") return $FUNCTION;
	if (s=="read") return $READ;
	if (s=="write") return $WRITE;
	return $SYMBOL;
}

void error(int line,string msg){
	fout << "***LINE:"<< line <<"  " << msg <<endl;;
}

BinExp Lex(string code,int &pos){
	char x;
	BinExp err("",-1);	
	int Now = 0;
	int st = pos;
	int Len = code.length();
	int ID;
	string val;

	while (pos<Len){
		if (pos>0&&code[pos-1]=='\n')
		{
			line++;
			BinExp("EOLN",$EOLN).print();
		}
		x = code[pos++];
		//if (x=='\n') line++;
		switch (Now=getState(Now,x)){
			case 0:
				st = pos;
				break;
			case 2 :
				pos--;
				//if (code[pos]=='\n') line--;
				val = code.substr(st,pos-st);
				ID = isReserve(val);
				return BinExp(val,ID);
			case 4 :
				pos--;
				//if (code[pos]=='\n') line--;
				return BinExp(code.substr(st,pos-st),$CONSTANT);
			case 5 :
				return BinExp("=",$EQU);
			case 6 :
				return BinExp("-",$SUB);
			case 7 :
				return BinExp("*",$MUL);
			case 8 :
				return BinExp("(",$LPAR);
			case 9 :
				return BinExp(")",$RPAR);
			case 11 :
				return BinExp("<=",$LE);
			case 12 :
				return BinExp("<>",$NEQU);
			case 13 :
				pos--;
				//if (code[pos]=='\n') line--;
				return BinExp("<",$L);
			case 15 :
				return BinExp(">=",$GE);
			case 16 :
				pos--;
				//if (code[pos]=='\n') line--;
				return BinExp(">",$G);
			case 18 :
				return BinExp(":=",$ASSIGN);
			case 19 :
				error(line,"\""+code.substr(st,pos-st)+"\" -> '=' expected before "+code.substr(pos-1,1));
				return err;
			case 20 :
				return BinExp(";",$SEM);
			case 21 :
				error(line,"\""+code.substr(st,1)+"\" -> unknown character");
				return err;
		}		
	}
	return err;
}


int main(int arg,char **args)
{	
	fileName = getfileName(string(args[1]));
	fout.open((fileName+".lex.err").c_str());
	ifstream fin(args[1]);

	freopen((fileName+".dyd").c_str(),"w",stdout);

	if (!fin.is_open()){
		fout << "can't open file!" <<endl;
		return 0;
	} else {
		code = "";
		string tmp;
		while (!fin.eof()){
			getline(fin,tmp);
			code += tmp + "\n";
		}
		int pos=0;
		line = 1;
		BinExp res;
		do {
			res = Lex(code,pos);
			if (res.type!=-1)
				res.print();
		} while (res.type != -1);
		BinExp("EOF",$EOF).print();
	}
	return 0;
}