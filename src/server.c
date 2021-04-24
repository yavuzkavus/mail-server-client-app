#include "stdio.h"         // for standart input output
#include "sys/socket.h"    //for socket, protocol families
#include "sys/types.h"     //for sockaddr_in
#include "error.h"         //for perror and errorno
#include "pthread.h"       //for thread_create
#include "netinet/in.h"    //for htonx,ntohx
#include "dirent.h"        // for opendir,readdir
#include "proje.h"         // for functions and macros and global declerations used by server and client both
#include "sys/stat.h"      // will be used for directory modes when mkdir(*,*)
#include "stdlib.h"        // for malloc

void *handle_client(int *);
char scontroluser(char *str);
char scontrolpassword(char *username,char *password);
void sremovedir(char *str);
void smember(char *username,int client_d);
void savemail(MAIL *mail);
void snewuser(int);
void sdeleteuser(int);
char slogin(int);
void sforward(char*,int);
void sdelete(char*,int);
void sread(char*,int);
void snewmail(int);
void slistmail(char*,int);
void sreply(char *,int);

int main() {
	int server_d,client_d;
	pthread_t new_thread;
	struct sockaddr_in server_addr,client_addr;
	int yes=1;

	if(  ( server_d=socket(AF_INET,SOCK_STREAM,0 ) ) ==-1) {
		perror("SOCKET ");
		exit(1);
	}
	if (setsockopt(server_d, SOL_SOCKET, SO_REUSEADDR,&yes, sINT) == -1)    {
		perror("SETSOCKOPT ");
		exit(1);
	}

	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=INADDR_ANY;
	server_addr.sin_port=htons(PORT);
	memset(&(server_addr.sin_zero),'\0',8);

	if(  bind(server_d,(struct sockaddr *)&server_addr,sSOCKADDR) == -1  )  {
		perror("BIND ");
		exit(1);
	}
	if(  listen(server_d,10) ==-1  ) {
		perror("LISTEN : ");
		exit(1);
	}
	printf("...SERVER IS READY TO ACCEPT ANY CONNECTION REQUEST...\n");
	while (1) {
		if(  (client_d = accept(server_d,(struct sockaddr *)&client_addr,&sSOCKADDR) ) ==-1 )
			perror("ACCEPT ");
		else  {
			if ( pthread_create(&new_thread,NULL,(void *)(int *)handle_client,&client_d) )
				perror("THREAD ");
			else   
				printf("%s IS CONNECTED.\n",inet_ntoa(client_addr.sin_addr));
		}
	}
	return 0;
}

void *handle_client(int *sock_d)  {
	int what;
	int client_d=*sock_d;
	while(1) {
		if( sINT>recv(client_d,&what,sINT,0) ) {
			what=ERROR;
			send(client_d,&what,sINT,0);
			continue;
		}
		switch(what) {
			case NEWUSER :
				snewuser(client_d);
				break;
			case DELETEUSER :
				sdeleteuser(client_d);
				break;
			case LOGIN :
				slogin(client_d);
				break;
			case HELP : break;
			case EXIT : 
				{
					close(client_d);
					return;
			}
		} // switch ends here
	} //while ends here
}

void smember(char *username,int client_d) {
	int what;
	while (1) {
		if(sINT==recv(client_d,&what,sINT,0) ) {
			switch(what) {
				case READ :
					sread(username,client_d);
					break;
				case COMPOSE :
					snewmail(client_d);
					break;
				case FORWARD :
					sforward(username,client_d);
					break;
				case DELETE :
					sdelete(username,client_d);
					break;
				case LISTMAIL :
					slistmail(username,client_d);
					break;
				case REPLY : 
					sreply(username,client_d);
					break;
				case HELP : break;
				case LOGOUT :  return;
			} //switch ends here
		} //else ends here
	} //while ends here;
}

char scontroluser(char *str) {
	DIR *dirp;
	struct dirent *direntp;
	dirp=opendir("accounts");
	while (  direntp=readdir(dirp) )
		if ( !strcmp(direntp->d_name,str) ) {
			closedir(dirp);
			return 1;
		}
	closedir(dirp);
	return 0;
}

char scontrolpassword(char *username,char *password) {
	DIR *dirp;
	FILE *fp;
	INFO info;
	char tmp[NAME_MAX];
	struct dirent *direntp;
	dirp=opendir("accounts");
	while (  direntp=readdir(dirp) ) {
		strcpy(tmp,"accounts/");
		if ( !strcmp(direntp->d_name,username) ) {
			strcat(tmp,username);
			strcat(tmp,"/info.dat");

			if ( !(fp=fopen(tmp,"rb")) ) {
				closedir(dirp);
				fclose(fp);
				return 0;
			}
			else {
				fread(&info,sizeof(INFO),1,fp);
				fclose(fp);
				closedir(dirp);
				if(!strcmp(password,info.cripto.password) ) return 1;
				else return 0;
			}
		}
	}
	closedir(dirp);
	return 0;
}

void sremovedir(char *username) {
	DIR *dirp;
	struct dirent *dirent;
	chdir("accounts");
	dirp=opendir(username);
	chdir(username);
	while(dirent=readdir(dirp)) 
		remove(dirent->d_name);
	chdir("..");
	rmdir(username);
	chdir("..");
	closedir(dirp);
}

void savemail(MAIL *mail) {
	long i=0,k,j;
	long now;
	char new[200]="\0";
	char new2[200]="\0";
	char *p;
	char username[15];
	char tmp[NAME_MAX]="accounts/";
	FILE *fp;
	srand(time(&now)%37);
	strcpy(new2,mail->to);
	strcat(new2,",#,");
	sscanf(new2+i,"%[^,]",username);	
	while( strcmp(username,"#") ) {
		j=0;
		strcpy(tmp+9,username);
		strcat(tmp,"/");
		if (!opendir(tmp) ) {
			j=-1;
			strcat(new,username);
			strcat(new,",");
		}		
		i+=strlen(username)+1;
		while(j==0&&j!=-1) {
			k=rand()%1000;
			p=inttostr(k);
			strcpy(tmp+10+strlen(username),p);
			strcpy(mail->id,p);			
			free(p);			
			strcat(tmp,".dat");
			if(!fopen(tmp,"rb")){
				fp=fopen(tmp,"wb");
				mail->status=0;
				fwrite(mail,sMAIL,1,fp);
				fclose(fp);
				j=1;
			}
			else fclose(fp);
		}
		
		sscanf(new2+i,"%[^,]",username);
	}
	if(*new)
		new[strlen(new)-1]='\0';
	strcpy(mail->to,new);	
}

void snewuser(int client_d) {
	char tmp[NAME_MAX];
	int what;
	char control=1;
	INFO info;
	FILE *fp;
	while(control) {
		strcpy(tmp,"accounts/");
		if(sINFO==recv(client_d,&info,sINFO,0) ){
			if( scontroluser(info.cripto.username) ) {
				what=EXISTUSER;
				send(client_d,&what,sINT,0);
			}
			else {
				strcat(tmp,info.cripto.username);
				mkdir(tmp,S_ISUID|S_ISGID|S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
				strcat(tmp,"/info.dat");
				if( !(fp=fopen(tmp,"wb") ) ) {
					what=ERROR;
					send(client_d,&what,sINT,0);
				}
				else {
					fwrite(&info,sINFO,1,fp);
					fclose(fp);
					what=ACCEPT;
					send(client_d,&what,sINT,0);
					control=0;
				}
			}
		}
		else control=0;
	}
}

void sdeleteuser(int client_d) {
	CRIPTO cripto;
	int what;
	if(sCRIPTO==recv(client_d,&cripto,sCRIPTO,0) ) {
		if( !scontrolpassword(cripto.username,cripto.password) ) {
			what=INVALID;
			send(client_d,&what,sINT,0);
		}
		else {
			sremovedir(cripto.username);
			what=ACCEPT;
			send(client_d,&what,sINT,0);
		}
	} // while ends here
}

char slogin(int client_d) {
	int what;
	CRIPTO cripto;
	if(sCRIPTO==recv(client_d,&cripto,sCRIPTO,0) ) {
		if( !scontrolpassword(cripto.username,cripto.password) ) {
			what=INVALID;
			send(client_d,&what,sINT,0);
		}
		else    {
			what=ACCEPT;
			if(sINT==send(client_d,&what,sINT,0))
				smember(cripto.username,client_d);
		}
	}
}

void snewmail(int client_d) {
	MAIL Mail;
	if(sMAIL==recv(client_d,&Mail,sMAIL,0) ) {
		savemail(&Mail);
		send(client_d,&Mail,sMAIL,0);
	}
}

void sforward(char *username,int client_d) {
	int what;
	char cmd2[10];
	char tmp[NAME_MAX]="accounts/";
	FILE *fp;
	MAIL Mail;
	if( sizeof(cmd2)==recv(client_d,cmd2,sizeof(cmd2),0) ) {
		strcat(tmp,username);
		strcat(tmp,"/");
		strcat(tmp,cmd2);
		strcat(tmp,".dat");
		if ( fp=fopen(tmp,"rb") )  {
			fread(&Mail,sMAIL,1,fp);
			fclose(fp);
			what=ACCEPT;
			send(client_d,&what,sINT,0);
			if ( sMAIL==send(client_d,&Mail,sMAIL,0) ) {
				recv(client_d,&Mail,sMAIL,0);
				savemail(&Mail);
				send(client_d,&Mail,sMAIL,0);				
			}
		}
		else {
			what=INVALID;
			send(client_d,&what,sINT,0);
		}
	}
}

void sdelete(char *username,int client_d) {
	int what;
	FILE *fp;
	char tmp[NAME_MAX]="accounts/";
	char cmd2[10];
	if(sizeof(cmd2)==recv(client_d,cmd2,sizeof(cmd2),0) ) {
		strcat(tmp,username);
		strcat(tmp,"/");
		strcat(tmp,cmd2);
		strcat(tmp,".dat");
		if( fp=fopen(tmp,"rb") ) {
			fclose(fp);
			remove(tmp);
			what=ACCEPT;
		}
		else what=INVALID;
		send(client_d,&what,sINT,0);
	}
}

void sread(char *username,int client_d) {
	int what;
	char cmd2[10];
	FILE *fp;
	MAIL Mail;
	char tmp[NAME_MAX]="accounts/";
	if(sizeof(cmd2)==recv(client_d,cmd2,sizeof(cmd2),0) ) {
		strcat(tmp,username);
		strcat(tmp,"/");		
		strcat(tmp,cmd2);
		strcat(tmp,".dat");
		if( !(fp=fopen(tmp,"r+b") )) {
			what==INVALID;
			send(client_d,&what,sINT,0);
		}
		else {
			what=ACCEPT;
			send(client_d,&what,sINT,0);
			fread(&Mail,sMAIL,1,fp);
			send(client_d,&Mail,sMAIL,0);
			Mail.status=1;
			fseek(fp,0,0);
			fwrite(&Mail,sMAIL,1,fp);
			fclose(fp);
		}
	}
}

void sreply(char *username,int client_d) {
	int what;
	char cmd2[10];
	FILE *fp;
	MAIL Mail;
	char tmp[NAME_MAX]="accounts/";
	if(sizeof(cmd2)==recv(client_d,cmd2,sizeof(cmd2),0) ) {
		strcat(tmp,username);
		strcat(tmp,"/");		
		strcat(tmp,cmd2);
		strcat(tmp,".dat");
		if( !(fp=fopen(tmp,"rb") )) {
			what==INVALID;
			send(client_d,&what,sINT,0);
		}
		else {
			what=ACCEPT;
			send(client_d,&what,sINT,0);
			fread(&Mail,sMAIL,1,fp);
			fclose(fp);
			strcpy(Mail.to,Mail.from);
			strcpy(Mail.from,username);
			send(client_d,&Mail,sMAIL,0);
			recv(client_d,&Mail,sMAIL,0);
			savemail(&Mail);
		}
	}
}	

void slistmail(char *username,int client_d) {
	int what;
	MAIL Mail;
	FILE *fp;
	DIR *dirp;
	struct dirent *direntp;
	chdir("accounts");
	dirp=opendir(username);
	chdir(username);
	while(direntp=readdir(dirp)) {
		if(strcmp(direntp->d_name,"info.dat")&& 
		   strcmp(direntp->d_name,".")&&
		   strcmp(direntp->d_name,"..")&&
		   (fp=fopen(direntp->d_name,"rb")) ) {
			fread(&Mail,sMAIL,1,fp);
			fclose(fp);
			send(client_d,&Mail,sMAIL,0);
		}
	}
	chdir("..");
	chdir("..");
	closedir(dirp);
	strcpy(Mail.from,"#");
	send(client_d,&Mail,sMAIL,0);
}
