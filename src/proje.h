#define NEWUSER 1  //new user will be signed up
#define LOGIN 2   // a member will be log in
#define LOGOUT 3  // member will be log out
#define DELETEUSER 4  // delete a member
#define DELETE 5  // delete mail
#define FORWARD 6   // forward message
#define READ 7     // messages read by member
#define COMPOSE 8  // to compose new message
#define LISTMAIL 9    // show all message again
#define REPLY 10
#define EXIT 11 // to exit from program
#define HELP 12
#define UNKNOWN 13   // this will be used when an unknown command used
#define LOGGED 14  
#define NOTLOGGED 15
#define ERROR 16    // there is an external error
#define EXISTUSER 17 //  an already member has same name
#define ACCEPT 18  // action is completed
#define INVALID 19    // invalid name or password




#define PORT 4045  //will be used as port number

typedef struct {
	char username[15];  //username will be used when loging in
	char password[9];   //password for save login
} CRIPTO;

typedef struct {
	char name[30];  //users general name
	CRIPTO cripto;  // store username and password
	char telno[12]; // telephone number
	char age;      // users age
} INFO;

typedef struct {
	char from[15];    // mails sender username
	char subject[50];  // mails subject
	char to[200];    // who mail sent to , can be more than one username
	char status;   //read=1 or unread=0 ?
	char id[5];	//store mails id
	char mail[1024]; // mails content
} MAIL;

/* convert str to uppercase */
void upper(char *str) {
	for(; *(str)=toupper(*str); str++);
}

/* it is used instead of scanf and gets.it is a better solution */
void getstr(void *str) {
	char ch;
	scanf("%[^\n]",str); scanf("%c",&ch);
}

/* converts  integer to */
char *inttostr(int k) {
	char *p;
	int i=0,j,tmp;
	p=(char *)malloc(10);
	while(k) {
		j=k % 10;	
		p[i]=j+'0';
		i++;
		k/=10;
	}
	p[i]='\0';
	for(j=0; j<i/2; j++) {
		tmp=p[j];
		p[j]=p[i-j-1];
		p[i-j-1]=tmp;
	}
	return p;
}

int sSOCKADDR=sizeof(struct sockaddr);
int sINT=sizeof(int);
int sINFO=sizeof(INFO);
int sCRIPTO=sizeof(CRIPTO);
int sMAIL=sizeof(MAIL);
