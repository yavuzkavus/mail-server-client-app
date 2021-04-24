#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "netdb.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "proje.h"
#include "stdarg.h"

//help topics
char help[][100]={"","->NEWUSER : TAKES NO PARAMETERS. IT IS TO CREATE A NEW USER",
		 "->LOGIN : TAKES NO PARAMETERS. IT IS TO LOGIN WITH AN EXISTING ACCOUNT",
		 "->LOGOUT : TAKES NO PARAMETERS. IT IS TO LOGOUT FROM YOUR ACCOUNT",
		 "->DELETEUSER : TAKES NO PARAMETERS. YOU CAN REMOVE AN EXISTING ACCOUNT",
		 "->DELETE : TAKES MESSAGE ID AS PARAMETER. DELETES MAIL WHOSE ID GIVEN AS PARAMETER",
		 "->FORWARD : TAKES MESSAGE ID AS PATAMETER. FORWARDS MESSAGE WHOSE ID GIVEN AS PATAMETER",
		 "->READ : TAKES MESSAGE ID AS PATAMETER. SHOW CONTENT OF MESSAGE WHOSE ID GIVEN AS PATAMETER",
		 "->COMPOSE : TAKES NO PARAMETERS.IT IS TO WRITE AND SEND A NEW MAIL",
		 "->LISTMAIL : TAKES NO PARAMETERS.IT IS TO WRITE AND SEND A NEW MAIL",
		 "->REPLY : TAKES MESSAGE ID AS PARAMETER. IT IS TO RESPONSE TO A MAIL IN MAIL BOX",
		 "->EXIT : TAKES NO PARAMETERS. TERMINATES THE PROGRAM",
		 "->HELP : TAKES PATAMETER AS COMMAND NAME WHICH YOU WANT TO SEE HELP ABOUT",
		 "->UNKOWN COMMAND. TYPE HELP [COMMAND NAME] TO SEE INFORMATION ABOUT COMMANDS",
		 "      TO USE THIS COMMAND YOU SHOULD BE LOGGED ON",
		 "      TO USE THIS COMMAND YOU SHOULD NOT BE LOGGED ON"
	};
char cmd[30];

//Function declerations 
void cmember(char *,int);
void getmessage(char*);
void cnewuser(int);
void cdeleteuser(int);
char clogin(int);
void cexit(int);
void ccompose(char*,int);
void cforward(char *,int);
void cdelete(int);
void cread(int);
void clistmail(char *,int);
void cprint_help(char *, ...) ;
void chelp(int);
void creply(int);

int main() {
	int server_d;
	struct sockaddr_in client_addr;
	struct hostent *he;
	char cmd1[15],cmd2[15];

	if((he=gethostbyname("tweety.ce.yildiz.edu.tr"))==NULL)   {
		perror("gethostbyname");
		exit(1);
	}
	if ((server_d=socket(AF_INET,SOCK_STREAM,0))==-1)  {
        	perror("SOCKET ");
        	exit(1);
   	}

   	client_addr.sin_family=AF_INET;
   	client_addr.sin_port=htons(PORT);
  	client_addr.sin_addr=*((struct in_addr *)he->h_addr);// inet_addr("127.0.0.1");  //
   	memset(&(client_addr.sin_zero),'\0',8);

   	if (connect(server_d,(struct sockaddr *)&client_addr,sizeof(struct sockaddr)) == -1) {
     		perror("CONNECT ");
     		exit(1);
   	}

	while(1) {
		printf("[atom]> ");
		getstr(cmd);
		upper(cmd);
		sscanf(cmd,"%s %s",cmd1,cmd2);
		if( !strcmp(cmd1,"NEWUSER") ) 
			cnewuser(server_d);
		else
		if( !strcmp(cmd1,"LOGIN") ) 
			clogin(server_d);
		else
		if( !strcmp(cmd1,"DELETEUSER")  )
			cdeleteuser(server_d);
		else
		if( !strcmp(cmd1,"EXIT") )
			cexit(server_d);
		else
		if( !strcmp(cmd1,"HELP") ) 
			chelp(server_d);
		else
		if( !strcmp(cmd1,"COMPOSE")||
		    !strcmp(cmd1,"DELETE")||
		    !strcmp(cmd1,"FORWARD")||
		    !strcmp(cmd1,"READ")||
		    !strcmp(cmd1,"REPLY")||
		    !strcmp(cmd1,"LISTMAIL")||
		    !strcmp(cmd1,"LOGOUT") )  cprint_help(" ",LOGGED,0);
		else cprint_help("",UNKNOWN,0);		
		*cmd='\0';
	}
	return 0;
}

void cmember(char *username,int server_d) {
	char cmd1[20],cmd2[10];
	int what;
	clistmail(username,server_d);
	while(1) {
		printf("[%s@atom]>",username);
		getstr(cmd);
		upper(cmd);
		sscanf(cmd,"%s",cmd1);
		if(!strcmp(cmd1,"COMPOSE") )
			ccompose(username,server_d);
		else 
		if(!strcmp(cmd1,"DELETE") )
			cdelete(server_d);
		else
		if(!strcmp(cmd1,"FORWARD") )
			cforward(username,server_d);
		else
		if(!strcmp(cmd1,"READ") )
			cread(server_d);
		else
		if(!strcmp(cmd1,"LISTMAIL") )
			clistmail(username,server_d);
		else
		if(!strcmp(cmd1,"REPLY") )
			creply(server_d);
		else
		if(!strcmp(cmd1,"LOGOUT") ) {
			what=LOGOUT;
			send(server_d,&what,sINT,0);
			return;
		}
		else 
		if(!strcmp(cmd1,"HELP")) 
			chelp(server_d);
		else
		if(!strcmp(cmd1,"LOGIN")||
		   !strcmp(cmd1,"NEWUSER")||
		   !strcmp(cmd1,"EXIT")||
		   !strcmp(cmd1,"DELETEUSER")) cprint_help("",NOTLOGGED,0);
		else cprint_help("",UNKNOWN,0);		
		*cmd='\0';
	}
}

void getmessage(char *str) {
	char tmp[1024]="";
	strcpy(str,"");
	getstr(tmp);
	while(strcmp(tmp,"#") ) {
		strcat(str,tmp);
		strcat(str,"\n");
		getstr(tmp);
	}
	str[strlen(str)-1]='\0';
}

void clistmail(char *username,int server_d) {
	MAIL mail;
	char tmp[60]="                                                           ";
	int what=LISTMAIL;
	if ( sINT>send(server_d,&what,sINT,0) ) return ;
	printf("\n-----------------------------MAILBOX----------------------------------\n");
	printf("MSGID\tFROM\t\tSUBJECT\t\t\t\t\tSTATUS\n");
	printf("-----\t----\t\t-------\t\t\t\t\t------\n");
	while (1) {
		recv(server_d,&mail,sMAIL,0);
		if( strcmp(mail.from,"#") ) {
			printf("%s\t",mail.id);
			tmp[16-strlen(mail.from)]='\0';
			printf("%s%s",mail.from,tmp);
			tmp[16-strlen(mail.from)]=' ';
			tmp[40-strlen(mail.subject)]='\0';
			printf("%s%s",mail.subject,tmp);
			tmp[40-strlen(mail.subject)]=' ';
			if(mail.status==1) printf("read\n");
			else printf("unread\n");
		}
		else break;
	}
	printf("----------------------------------------------------------------------\n\n");
}


void cnewuser(int server_d) {
	int what=NEWUSER;
	INFO info;
	int control=1;
	char ch;
	if( sINT>send(server_d,&what,sINT,0) )
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");

	else	{
		printf("NAME : ");  	getstr(info.name);    		upper(info.name);
		printf("USERNAME : "); 	getstr(info.cripto.username); 	upper(info.cripto.username);
		printf("PASSWORD : "); 	getstr(info.cripto.password);	upper(info.cripto.password);
		printf("TELEPHONE : ");	getstr(info.telno);
		printf("AGE : "); getstr(&info.age);
		while (control) {
			if(sINFO>send(server_d,&info,sINFO,0) ){
				printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
				control=0;
			}
			else {
				if( sINT>recv(server_d,&what,sINT,0) ) {
					printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
					control=0;
				}
				else {
					if(what==ACCEPT) {
						printf("YOUR REGISTERY COMPLETED SUCCESFULLY.\n");
						control=0;
					}
					if (what==EXISTUSER) {
						printf("THE USERNAME YOU ENTERED [%s] ALREADY EXISTS!\n",info.cripto.username);
						printf("DO YOU WANT TO ENTER A NEW ONE [Y\\N] : ");
						getstr(&ch);
						ch=toupper(ch);
						if(ch=='Y') {
							printf("USERNAME : "); getstr(info.cripto.username); 
							upper(info.cripto.username);
							printf("PASSWORD : "); getstr(info.cripto.password);
							upper(info.cripto.password);
						}
						else control=0;
					}
					if (what==ERROR) {
						printf("YOUR REGISTRY COULD NOT BE COMPLETED DUE TO AN ERROR AT SERVER SIDE!\n");
						control=0;
					}
				}
			}
		}
	}
}

void cdeleteuser(int server_d) {
	char control=1;
	CRIPTO cripto;
	int what=DELETEUSER;
	if( sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	printf("USERNAME : "); getstr(cripto.username);  upper(cripto.username);
	printf("PASSWORD : "); getstr(cripto.password);  upper(cripto.password); 
	if(sCRIPTO>send(server_d,&cripto,sCRIPTO,0)) {
		printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	else {
		if( sINT>recv(server_d,&what,sINT,0) ) 
			printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
		else {
			if(what==ACCEPT) {
				printf("USER DELETED SUCCESFULLY.\n");
				control=0;
			}
			if(what==INVALID) {
				printf("INVALID USERNAME OR PASSWORD!"); 
				printf("BE SURE YOUR USERNAME AND PASSWORD  ARE CORRECT.\n");
			}
		}
	}
}

char clogin(int server_d) {
	int what=LOGIN;
	CRIPTO cripto;
	if( sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return 0;
	}
	printf("USERNAME : "); getstr(cripto.username);  upper(cripto.username);
	printf("PASSWORD : "); getstr(cripto.password);  upper(cripto.password);
	if(sCRIPTO>send(server_d,&cripto,sCRIPTO,0))
		printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
	else {
		if( sINT>recv(server_d,&what,sINT,0) )
			printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
		else {
			if(what==ACCEPT) {
				cmember(cripto.username,server_d);
				return 1;
			}
			else {
				printf("INVALID USERNAME OR PASSWORD!"); 
				printf("BE SURE YOUR USERNAME AND PASSWORD  ARE CORRECT.\n");
			}
		}
	}
}

void cexit(int server_d) {
	int what=EXIT;
	send(server_d,&what,sINT,0);
	exit(0);
}

void ccompose(char *username,int server_d){
	int what=COMPOSE;
	MAIL Mail;
	if(sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	printf("T0 : "); getstr(Mail.to);  upper(Mail.to);
	printf("SUBJECT : "); 	getstr(Mail.subject);
	Mail.subject[49]='\0';
	printf("--------------------------------------------\n");
	getmessage(Mail.mail);
	printf("--------------------------------------------\n");
	strcpy(Mail.from,username);
	Mail.status=0;
	if(sMAIL>send(server_d,&Mail,sMAIL,0) )
		printf("COULD NOT SEND SERVER MESSAGE! SERVER MIGHT BE DISCONNECTED.\n");
	else {
		if(sMAIL==recv(server_d,&Mail,sMAIL,0) ) {
			if(Mail.to[0]) {
				printf("YOUR MESSAGE COULD NOT BE DELIVERED THOSE : %s\n",Mail.to);
				printf("BE SURE THOSE NAMES ARE VALID\n");
			}
			else   
				printf("YOUR MESSAGE HAS BEEN SENT SUCCESFULLY.\n");
					
		}
	}
}

void cforward(char* username,int server_d) {
	int what=FORWARD;
	char cmd1[20],cmd2[10];
	MAIL Mail;
	if(sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	sscanf(cmd,"%s%s",cmd1,cmd2);
	if(sizeof(cmd2)>send(server_d,cmd2,sizeof(cmd2),0)) {
		printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if (sINT>recv(server_d,&what,sINT,0)){
		printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if(what==ACCEPT) {		
		if(sMAIL>recv(server_d,&Mail,sMAIL,0) ) {
			printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
			return;
		}
		else {
			printf("TO : "); getstr(Mail.to); upper(Mail.to);
			Mail.status=0;
			strcpy(Mail.from,username);
			send(server_d,&Mail,sMAIL,0);
			recv(server_d,&Mail,sMAIL,0);		
				if(strlen(Mail.to)) 
			if(Mail.to[0]){
				printf("YOUR MESSAGE COULD NOT BE DELIVERED THOSE : %s\n",Mail.to);
				printf("BE SURE THOSE NAMES ARE VALID\n");
			}
			else   
				printf("YOUR MESSAGE HAS BEEN SENT SUCCESFULLY.\n");
		
		}
	}
	else {
		if(what==INVALID)
			printf("ENTER A VALID MESSAGE ID!\n");
		else printf("YOUR REGISTRY COULD NOT BE COMPLETED DUE TO AN ERROR AT SERVER SIDE!\n");
	}
}

void cdelete(int server_d) {
	int what=DELETE;
	char cmd1[20],cmd2[10];
	if(sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	sscanf(cmd,"%s %s",cmd1,cmd2);
	cmd2[9]='\0';
	if(sizeof(cmd2)>send(server_d,&cmd2,sizeof(cmd2),0)) {
		printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if (sINT>recv(server_d,&what,sINT,0) ){
		printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if(what==INVALID)
		printf("ENTER A VALID MESSAGE ID!\n");
	else {
		if(what==ACCEPT)
		printf("MESSAGE DELETED SUCCESFULLY.\n");
		else printf("YOUR REGISTRY COULD NOT BE COMPLETED DUE TO AN ERROR AT SERVER SIDE!\n");
	}
}

void cread(int server_d) {
	int what=READ;
	char cmd1[20],cmd2[10];
	MAIL Mail;
	if(sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	sscanf(cmd,"%s %s",cmd1,cmd2);
	cmd2[9]='\0';
	if(sizeof(cmd2)>send(server_d,cmd2,sizeof(cmd2),0)) {
		printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if (sINT>recv(server_d,&what,sINT,0)){
		printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if(what==ACCEPT) {
		if(sMAIL>recv(server_d,&Mail,sMAIL,0) ) {
			printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
			return;
		}
		else {
		printf("----------------------------------------------------------------------\n");
			printf("From : %s\n",Mail.from);
			printf("To   : %s\n",Mail.to);
			printf("Subject : %s\n",Mail.subject);
			printf("************\n");
			printf("%s\n",Mail.mail);
		printf("----------------------------------------------------------------------\n");
		}
	}
	else {
		printf("ENTER A VALID MESSAGE ID!\n");
	}
}

void creply(int server_d) {
	int what=REPLY;
	char cmd1[20],cmd2[10];
	MAIL Mail;
	if(sINT>send(server_d,&what,sINT,0) ) {
		printf("COULD NOT SEND SERVER REQUEST! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	sscanf(cmd,"%s %s",cmd1,cmd2);
	cmd2[9]='\0';
	if(sizeof(cmd2)>send(server_d,cmd2,sizeof(cmd2),0)) {
		printf("COULD NOT SEND SERVER INFORMATION! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if (sINT>recv(server_d,&what,sINT,0)){
		printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
		return;
	}
	if(what==ACCEPT) {
		if(sMAIL>recv(server_d,&Mail,sMAIL,0) ) {
			printf("SERVER HAVE NOT RESPONED! SERVER MIGHT BE DISCONNECTED.\n");
			return;
		}
		else {
			printf("Subject : ");    getstr(Mail.subject);
			printf("----------------------------------------------------------------------\n");
			getmessage(Mail.mail);
		printf("----------------------------------------------------------------------\n");
		}
		send(server_d,&Mail,sMAIL,0);
		printf("YOUR MESSAGE DELIVERED SUCCESFULLY.\n");
	}
	else {
		printf("ENTER A VALID MESSAGE ID!\n");
	}
}	

void chelp(int server_d) {
	char cmd1[20],cmd2[20],i;
	int what=HELP;
	send(server_d,&what,sINT,0);
	sscanf(cmd,"%s %s",cmd1,cmd2);
	upper(cmd2);
	if( !strcmp(cmd2,"NEWUSER") ) cprint_help(" ",NEWUSER,NOTLOGGED,0);
	else
	if( !strcmp(cmd2,"DELETEUSER") ) cprint_help(" ",DELETEUSER,NOTLOGGED,0);
	else
	if( !strcmp(cmd2,"HELP") ) cprint_help(" ",HELP,0);
	else
	if( !strcmp(cmd2,"LOGIN") ) cprint_help(" ",LOGIN,NOTLOGGED,0);
	else
	if( !strcmp(cmd2,"LOGOUT") ) cprint_help(" ",LOGOUT,LOGGED,0);
	else
	if( !strcmp(cmd2,"DELETE") ) cprint_help(" ",DELETE,LOGGED,0);
	else
	if( !strcmp(cmd2,"READ") ) cprint_help(" ",READ,LOGGED,0);
	else
	if( !strcmp(cmd2,"COMPOSE") ) cprint_help(" ",COMPOSE,LOGGED,0);
	else
	if( !strcmp(cmd2,"FORWARD") ) cprint_help(" ",FORWARD,LOGGED,0);
	else 
	if( !strcmp(cmd2,"LISTMAIL") ) cprint_help(" ",LISTMAIL,LOGGED,0);
	else
	if( !strcmp(cmd2,"REPLY") ) cprint_help(" ",REPLY,LOGGED,0);
	else 
	if( !strcmp(cmd2,"EXIT") ) cprint_help(" ",EXIT,0);
	else  cprint_help(" ",1,15,2,15,3,14,4,15,5,14,6,14,7,14,8,14,9,15,10,14,11,15,0);
}
	
void cprint_help(char *msg,...) {
	va_list ap;
	int k;
	va_start(ap,msg);
	while ( k=va_arg(ap,int) )
		printf("%s.\n",help[k]);
}
