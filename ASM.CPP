#include<stdio.h>
#include<conio.h>
#include<string.h>
#include<stdlib.h>

static int sc=0,lc=0,tc=0,ltc=0,lnctr=1;
char c='\0';

struct directive{
	char symbol[10];
	int code;
}dir[5]={"start",1,"end",0,"origin",1,"ltorg",2};

struct buffer{
	char lbl[10];
	char m[10];
	char op1[10];
	char op2[10];
}buf;

struct mnemtab{
	char mnem[20];
	int opcode;
	int len;
}l[12]={"stop",0,1,"add",1,1,"sub",2,1,"mult",3,1,"mover",4,1,"movem",5,1,
	"comp",6,1,"bc",7,1,"div",8,1,"read",9,1,"print",10,1};

struct symtab{
	char symbol[10];
	int add;
}sym[50];

struct regtab{
	char regsym[10];
	int code;
}reg[4]={"areg",1,"breg",2,"creg",3,"dreg",4};

struct littab{
	char lit[10];
	int address;
}literal[20];

struct dclcode{
	char mne[5];
	int address;
}dcl[2]={"ds",1,"dc",2};

struct tab_inc{
	int index;
	long fpos;
	int type;
}t_inc[10];


struct compcode{
	char sym[5];
	int code;
}ccode[7]={"lt",1,"le",2,"eq",3,"gt",4,"ge",5,"any",6};

int get_btcode(char []);
int get_sym(char []);
int get_reg(char []);
int get_dcl(char []);
int get_dir(char []);
void update_symtab(FILE *);
void parser(FILE *);
void pass1(FILE *,FILE *);
void pass2(FILE *);
void clean_buf();
int decode_lit(char [],FILE *);
int get_ccode(char []);

void main()
{
	FILE *fi,*fe,*fo;
	clrscr();
	fi=fopen("asm1.txt","r");
	fe=fopen("asmError.txt","a+");
	fo=fopen("asmOutput.txt","w");

	fprintf(fe,"\n\n\n");
	while(c!=EOF)
	{
		parser(fi);
		printf("\n");
		pass1(fe,fo);
	}
	pass2(fo);
	fclose(fi);
	fclose(fe);
	fclose(fo);
	getch();
}

int get_dir(char a[10]){
	int i;
	for(i=0;i<4;i++){
		if(!strcmp(a,dir[i].symbol))
			return dir[i].code;
	}
	return -1;
}

void update_symtab(FILE *fe){
	int i,j;
	for(i=0;i<sc;i++){
		if(!strcmp(sym[i].symbol,buf.m)){
			 if(sym[i].add==0)
				sym[i].add=lc;
			 else{
				printf("\nMultcple declaration of %s",buf.m);
				fprintf(fe,"\nMultcple declaration of %s",buf.m);
			 }
			 return;
		}
	}
}

int get_sym(char s[10]){
	int i,len=strlen(s);
	if(s[0]=='=')
	{
		for(i=0;i<ltc;i++)
		{
			if(!strcmp(literal[i].lit,s))
			{
				t_inc[tc].type=0;
				return i;
			}
		}
		strcpy(literal[ltc].lit,s);
		ltc++;
		t_inc[tc].type=0;
		return (ltc-1);
	}
	else if(s[len-1]==':')
	{
		s[len-1]='\0';
		strcpy(sym[sc].symbol,s);
		sym[sc].add=lc;
		sc++;
		return (sc-1);
	}
	else
	{
		for(i=0;i<sc;i++)
		{
			if(!strcmp(s,sym[i].symbol))
			{
				t_inc[tc].type=1;
				return i;
			}
		}
		strcpy(sym[sc].symbol,s);
		t_inc[tc].type=1;
		sym[sc].add=0;
		sc++;
		return (sc-1);
	}
}

int get_reg(char a[])
{
	int i;
	for(i=0;i<3;i++)
	{
		if(!strcmp(a,reg[i].regsym))
			return reg[i].code;
	}
	return 0;
}

int get_btcode(char a[20])
{
	int i;
	for(i=0;i<11;i++)
	{
		if(strcmp(l[i].mnem,a)==0)
			return l[i].opcode;
	}
	return -1;
}

int get_dcl(char a[10])
{
	int i;
	for(i=0;i<3;i++)
	{
		if(!strcmp(dcl[i].mne,a))
			return dcl[i].address;
	}
	return -1;
}

void clean_buf()
{
	strcpy(buf.lbl,"");
	strcpy(buf.m,"");
	strcpy(buf.op1,"");
	strcpy(buf.op2,"");
}

int decode_lit(char a[10],FILE *fe)
{
	int i,length,j=0;
	char val[5];
	length=strlen(a);
	for(i=0;i<ltc;i++)
	{
		if(!strcmp(literal[i].lit,buf.m))
		{
			if(literal[i].address==0)
				literal[i].address=lc;
			else
			{
				printf("\n Multcple declaration of %s",buf.m);
				fprintf(fe,"\n Multcple declaration of %s",buf.m);
			}
		}
	}

	for(i=2;i<length;i++,j++)
		val[j]=a[i];

	return atoi(val);
}

int get_ccode(char a[5])
{
	int i;
	for(i=0;i<7;i++)
	{
		if(!strcmp(ccode[i].sym,a))
			return ccode[i].code;
	}
	return -1;
}

void parser(FILE *fi)
{
	char a[20];
	int i;
	int ctr=0;
	while((c=getc(fi))!=EOF && (c!='\n')){
		if((c!=' ') && (c!='\t') && (c!='\n')){
			i=0;
			a[i++]=c;
			while(((c=getc(fi))!=' ') && (c!=' ') && (c!='\n') &&
		       (c!=EOF) && (c!=','))
			{
				a[i++]=c;
			}
			a[i]='\0';
			if(a[0]!='\0'){
				if(ctr==0){
					if(a[i-1]==':'){
						strcpy(buf.lbl,a);
						printf("\nlbl:%s",buf.lbl);
						continue;
					}
					else{
						strcpy(buf.m,a);
						printf("\nm:%s",buf.m);
						ctr++;
					}
				}

				else if(ctr==1){
					strcpy(buf.op1,a);
					printf("\nop1:%s",buf.op1);
					ctr++;
				}
				else if(ctr==2){
					strcpy(buf.op2,a);
					printf("\nop2:%s",buf.op2);
					ctr=0;
				}
			}
		if(c=='\n'){
			ctr=0;
			lnctr++;
			return;
		}
	}
   }
}

void pass1(FILE *fe,FILE *fo)
{
	int code,opcode,dcode;
	static int fstop=1,fend=1;
	long fpos;
	if(fstop)
	{
		if(strcmp(buf.lbl,""))
			get_sym(buf.lbl);
		opcode=get_btcode(buf.m);
		if(opcode==0)
		{
			fstop=0;
			fprintf(fo,"%d) 0 000 \n",lc);
			lc++;
			return;
		}
		if(opcode==7)
		{
			fprintf(fo,"%d) %d ",lc,opcode);
			code=get_ccode(buf.op1);
			fprintf(fo,"%d",code);
			code=get_sym(buf.op2);
			fpos=ftell(fo);
			t_inc[tc].index=code;
			t_inc[tc].fpos=fpos;
			tc++;
			fprintf(fo,"   \n");
			lc++;
			return;
		}
		if(opcode!=-1)
		{
			fprintf(fo,"%d) %d ",lc,opcode);
			code=get_reg(buf.op1);
			if(code==0)
			{
				strcpy(buf.op2,buf.op1);
			}
			fprintf(fo,"%d",code);
			code=get_sym(buf.op2);
			fpos=ftell(fo);
			t_inc[tc].index=code;
			t_inc[tc].fpos=fpos;
			tc++;
			lc++;
			fprintf(fo,"   \n");
		}

		else
		{
			dcode=get_dir(buf.m);
			if(dcode!=-1)
			{
				switch(dcode)
				{
					case 1:
						lc=atoi(buf.op1);
						break;
				}
			}
			else
			{
				fprintf(fo,"%d) ***",lc);
				code=get_reg(buf.op1);
				if(code==0)
				{
					if(!strcmp(buf.op2,""))
						strcpy(buf.op2,buf.op1);
					else
					{
						fprintf(fo," *** ");
						fprintf(fe," Error at line %d :: Illegal register %s",lnctr - 1,buf.op1);
						code=get_sym(buf.op2);
						fpos=ftell(fo);
						t_inc[tc].index=code;
						t_inc[tc].fpos=fpos;
						tc++;
						lc++;
					}
				}
				fprintf(fo," %d",code);
				code=get_sym(buf.op2);
				fpos=ftell(fo);
				t_inc[tc].index=code;
				t_inc[tc].fpos=fpos;
				tc++;
				lc++;
				fprintf(fo,"   \n");
				printf("\n Error at line %d :: Unknown mnemonics: %s",lnctr-1,buf.m);
				fprintf(fe,"\n Error at line %d :: Unknown mnemonics: %s",lnctr-1,buf.m);
			}
		}
	}
	else
	{
		if(fend)
		{
			if(!strcmp(buf.m,"end"))
			{
				fprintf(fo,"%d)   \n",lc);
				lc++;
				fend=0;
			}
			else
			{
				code=get_dcl(buf.op1);
				switch(code)
				{
					case 1:
						fprintf(fo,"%d)  \n",lc);
						update_symtab(fe);
						lc+=atoi(buf.op2);
						break;
					case 2:
						fprintf(fo,"%d)00 0 001  \n ",lc);
						update_symtab(fe);
						lc++;
						break;
				}
			}
		}
		else if(strcmp(buf.m,""))
			{
				code=decode_lit(buf.m,fe);
				fprintf(fo,"%d) 00 0 %d  \n",lc,code);
				lc++;
			}
		}
	clean_buf();
	return;
}

void pass2(FILE *fop)
{
	int i,adr,x;
	long pos;
	for(i=0;i<tc;i++)
	{
		pos=t_inc[i].fpos;
		adr=t_inc[i].index;
		fseek(fop,pos,0);
		if(t_inc[i].type==0)
		{
			if(literal[adr].address!=0)
				fprintf(fop,"%d \t",literal[adr].address);
			else
			{
				fprintf(fop,"***");
				printf("\n Undelared literal: %s",literal[adr].lit);
				fprintf(fop,"\n Undelared literal: %s",literal[adr].lit);
			}
		}
		else
		{
			if(sym[adr].add!=0)
				fprintf(fop,"%d\t",sym[adr].add);
			else
			{
				fprintf(fop,"***");
				printf("\n Undeclared symbol: %s",sym[adr].symbol);
				fprintf(fop,"\n Undeclared symbol: %s",sym[adr].symbol);
			}
		}
	}
}