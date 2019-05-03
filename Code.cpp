#include<iostream>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <stdio.h>
#include <fstream>
#define LINE_LENGTH 80
#define MAX_ARGS 64
#define MAX_PATH_LENGTH 96
using namespace std;
string prompt_string="LUCKY>> ";
struct Command_t
{
	char * name;
	int argc;
	char *argv[MAX_ARGS];
	Command_t()
	{
		for(int i=0;i<MAX_ARGS;i++)
			argv[i]=NULL;
	}
};
void printPrompt(){
	cout<<prompt_string;}
void readCommand(char *buffer){
	cin.getline(buffer,LINE_LENGTH);}
int parsePath(char (*dirs)[MAX_PATH_LENGTH]){
	char *pathEnvVar,*thePath;
	pathEnvVar=getenv("PATH");
	thePath=new char[strlen(pathEnvVar)+1];
	strcpy(thePath,pathEnvVar);
	char *token=strtok(thePath,":");
	int j=0;
	while(token!=NULL){
		strcpy(dirs[j++],token);
		dirs[j-1][strlen(dirs[j-1])+1]=NULL;
		token=strtok(NULL,":");}
	strcpy(dirs[j++],".");
	return j;
}
int parseCommand(char *commandLine,Command_t *cmd){
	char *token=strtok(commandLine," ");
	int j=0;
	while(token!=NULL)	{
		cmd->argv[j]=new char[strlen(token)+1];
		strcpy(cmd->argv[j++],token);
		token=strtok(NULL," ");}
	cmd->argc=j;
	return 1;
}

int parseCommand1(char *commandLine,Command_t *cmd){
	int i=0;
	int split=-1;
	char temp[LINE_LENGTH];
	while(commandLine[i])
	{
		if(commandLine[i]!='|' || commandLine[i]!='>')
			temp[i]=commandLine[i];
		if(commandLine[i]=='|')
		{
			split=i;
			break;
		}
		else if(commandLine[i]=='>')
		{
			split=i;
			break;
		}
		i++;
	}
	temp[i]=NULL;	
	if(split!=-1)
	{
		int m=0;
		for(int k=split+1;commandLine[k]!='\0';k++)
			commandLine[m++]=commandLine[k];
		commandLine[m++]=NULL;

	}
	char *token=strtok(temp," ");
	int j=0;
	while(token!=NULL)	{
		cmd->argv[j]=new char[strlen(token)+1];
		strcpy(cmd->argv[j++],token);
		token=strtok(NULL," ");}
	cmd->argc=j;
	cmd->argv[j]=NULL;
	return 1;
}
char* lookUpPath(char **argv,char dir[][MAX_PATH_LENGTH],int count_args){
	int flag=2;
	char *pName;
	char token[2]={'/','\0'};
	for(int i=0;i<count_args;i++){
		pName=new char[MAX_PATH_LENGTH];
		pName=strcat(pName,dir[i]);
		if(*argv[0]!='/' )
			pName=strcat(pName,token);
		pName=strcat(pName,argv[0]);
		flag=access(pName,F_OK);
		if(flag==0)
			break;}
	if(flag!=-1)
		return pName;
	return NULL;
}
int FileFlag=0;
int Numpipes(char buff[LINE_LENGTH]){
	int pipeNum=0;
	for (int i=0; buff[i]!='\0';i++){
		if (buff[i]=='|' || buff[i]=='>')
			pipeNum++;
		if(buff[i]=='>')
			FileFlag++;
	}
	return pipeNum;
}
int foo(int pipecount,char *commandLine,Command_t &command,char (*pathv)[MAX_PATH_LENGTH],int directorysize)
{
	if (pipecount==3)
	{
		int File_Descriptor[2],File_Descriptor1[2],File_Descriptor2[2];
		pipe (File_Descriptor);pipe (File_Descriptor1);pipe(File_Descriptor2);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}

		pid_t pid=fork();
		if (pid==0)
		{
			dup2 (File_Descriptor[1],1);
			execv(command.name,command.argv);
			return 0;
		}
		wait (NULL);
		close(File_Descriptor[1]);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t num=fork();
		if (num==0){
			dup2(File_Descriptor[0],0);
			dup2 (File_Descriptor1[1],1);
			execv(command.name,command.argv);
			return 0;
		}
		wait (NULL);
		close (File_Descriptor[0]);
		close (File_Descriptor1[1]);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t num2=fork();
		if (num2==0){
			dup2 (File_Descriptor1[0],0);
			dup2 (File_Descriptor2[1],1);
			execv(command.name,command.argv);
			return 0;
		}
		wait(NULL);
		close(File_Descriptor2[1]);
		close (File_Descriptor1[0]);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL && FileFlag ==0)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t perk=fork();
		if(perk==0)
		{
			if( FileFlag==1)
			{
				char Buf[1000];
				read(File_Descriptor2[0],Buf,1000);
				ofstream fout(command.argv[0]);
				for(int k=0;k<strlen(Buf);k++)
					fout<<Buf[k];       
				close(File_Descriptor2[0]);	 
				fout.close();  
				return 0;   
			}
			return 0;
		}
	}
	else	if (pipecount==2){
		int File_Descriptor[2], File_Descriptor1[2];
		pipe (File_Descriptor);pipe (File_Descriptor1);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t pid=fork();
		if (pid==0){
			dup2 (File_Descriptor[1],1);
			execv(command.name,command.argv);
			return 0;
		}
		wait (NULL);
		close(File_Descriptor[1]);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL && FileFlag ==0)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t num3=fork();
		if (num3==0){
			dup2(File_Descriptor[0],0);
			dup2 (File_Descriptor1[1],1);
			execv(command.name,command.argv);
			return 0;
		}

		wait (NULL);
		close (File_Descriptor[0]);
		close (File_Descriptor1[1]);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL && FileFlag ==0)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t num=fork();
		if (num==0){
			if( FileFlag==1){
				char Buf[1000];
				read(File_Descriptor1[0],Buf,1000);
				ofstream fout(command.argv[0]);
				for(int k=0;k<strlen(Buf);k++)
					fout<<Buf[k];       
				close(File_Descriptor1[0]);	 
				fout.close();  
				return 0;   
			}
			dup2 (File_Descriptor1[0],0);
			int num2=fork();
			if(num2==0){
				execv(command.name,command.argv);
				return 0;
			}
			wait (NULL);
			close (File_Descriptor1[0]);
		}
	}
else if (pipecount==1){
		int File_Descriptor[2];pipe (File_Descriptor);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t pid=fork();
		if (pid==0){
			close(File_Descriptor[0]);
			dup2 (File_Descriptor[1],1);
			execv(command.name,command.argv);
			return 0;
		}
		wait (NULL);
		char br[100];
		close(File_Descriptor[1]);
		parseCommand1(commandLine,&command);
		command.name=lookUpPath((command.argv),pathv,directorysize);
		if(command.name==NULL && FileFlag ==0)
		{
			cout<<"Command not Found in Respective Directories "<<command.argv[0]<<endl;
			return 0;
		}
		pid_t n=fork();
		if (n==0){
			close (File_Descriptor[1]);
			if( FileFlag==1){
				char Buf[1000];
				read(File_Descriptor[0],Buf,1000);
				ofstream fout(command.argv[0]);
				for(int k=0;k<strlen(Buf);k++)
					fout<<Buf[k];   
				fout.close();  
				return 0;   
			}
			dup2(File_Descriptor[0],0);
			execv(command.name,command.argv);
			return 0;
		}
	}

	return 0;
}
int main()
{
	char commandLine[LINE_LENGTH];
	char pathv[MAX_ARGS][MAX_PATH_LENGTH];
	pid_t pidd=fork();
	if(pidd==0)
	{
		execlp("clear","clear",NULL);
		return 0;
	}
	wait(NULL);
	int directorysize=parsePath(pathv);	
	while(true)
	{
		FileFlag=0;
		Command_t command;
		printPrompt();
		readCommand(commandLine);
		if(commandLine[0]=='q' && strlen(commandLine)==1)
			break;
		int pipe= Numpipes(commandLine);
		if(pipe==0)
		{
			parseCommand(commandLine,&command);
			command.name=lookUpPath((command.argv),pathv,directorysize);
			if(command.name==NULL)
			{
				cout<<"Command not Found in Respective Directories"<<endl;
				continue;
			}
			pid_t pid=fork();
			if(pid==0)
			{
				execv(command.name,command.argv);
				return 0;
			}
		}
		else
			foo(pipe,commandLine,command,pathv,directorysize);
		wait(NULL);
	}
	cout<<"Program has been terminated"<<endl;
	return 0;
}

//
//enum EndMode{R,W};
//int A[2];
//int B[2];
//
//int GetNumPipes(char **args,int argc)
//{
//	int count = 0;
//	for (int i=0; i<argc; i++) 
//	{
//		if (strcmp(args[i], "|") == 0)
//			count++;
//	}
//	return count;
//}
//int GetNumGreater(char **args,int argc)
//{
//	int count = 0;
//	for (int i=0; i<argc; i++) 
//	{
//		if (strcmp(args[i], ">") == 0)
//			count++;
//	}
//	return count;
//}
//void parent(int i,int total)
//{
//	if(i==0)
//		close(B[W]);
//	else if(i==total)
//	{
//		if(i%2==0)
//			close(B[W]);
//		else
//			close(A[W]);
//	}
//	else
//	{
//		if(i%2==0)
//		{
//			close(A[R]);
//			close(B[W]);
//		}
//		else{
//			close(B[R]);
//			close(A[W]);}
//	}
//	wait(NULL);
//
//	cout<<"I am Parent"<<endl;
//}
//void child(int i,int total)
//{
//	if(i==0)
//		dup2(B[W],1);
//	else if(i=total)
//	{
//		if(i%2==0)
//			dup2(B[R],0);
//		else
//			dup2(A[R],0);
//	}
//	else
//	{
//
//		if(i%2==0){
//			dup2(A[R],0);
//			dup2(B[W],1);}
//		else{
//			dup2(B[R],0);
//			dup2(A[W],1);
//		}
//
//	}
//	cout<<"I am child"<<endl;
//}
//
//void Filing(char** cmd, char** file,int *File_Descriptors) {
//	int count;  // used for reading from stdout
//	int File_Descriptor;     
//	char c; 
//	pid_t pid; 
//	pipe(File_Descriptors);
//	if (fork() == 0) {
//		File_Descriptor = open(file[0], O_RDWR | O_CREAT, 0666);
//		if (File_Descriptor < 0) {
//			cout<<"File cannot be created"<<endl;
//			return;
//		}
//		dup2(File_Descriptors[0], 0);
//		close(File_Descriptors[1]);
//		while ((count = read(0, &c, 1)) > 0)
//			write(File_Descriptor, &c, 1); 
//	} 
//	else if ((pid = fork()) == 0) {
//		dup2(File_Descriptors[1], 1);
//		close(File_Descriptors[0]);
//		execvp(cmd[0], cmd);
//		cout<<"File not found"<<endl;
//	}
//	else {
//		wait(NULL);
//		close(File_Descriptors[0]);
//		close(File_Descriptors[1]);
//	}
//}
//int main()
//{
//	char commandLine[LINE_LENGTH];
//	char pathv[MAX_ARGS][MAX_PATH_LENGTH];
//	int directorysize=parsePath(pathv);
//	pid_t pidd=fork();
//	if(pidd==0)
//	{
//		execlp("clear","clear",NULL);
//		return 0;
//	}
//	wait(NULL);
//	while(true)
//	{
//
//		Command_t command;
//		printPrompt();
//		readCommand(commandLine);
//		if(commandLine[0]=='q' && strlen(commandLine)==1)
//			break;
//		parseCommand(commandLine,&command);
//		int totalpipes=GetNumPipes(command.argv,command.argc);
//		cout<<"Total Pipes:: "<<totalpipes<<endl;
//		if(GetNumGreater(command.argv,command.argc)!=0 || totalpipes!=0)
//		{
//			for(int i=0;i<=totalpipes ;i++)
//			{
//				char* temp[MAX_ARGS];
//				int pipecheck=-1;
//				int greatercheck=-1;
//				int val=-1;
//				for(int l=1;l<command.argc;l++)
//				{
//					if (strcmp(command.argv[l], "|") == 0)
//					{
//						pipecheck=1;
//						val=l;
//						break;
//					}
//					else if (strcmp(command.argv[l], ">") == 0)
//					{
//						i--;
//						greatercheck=1;
//						val=l;
//						break;
//					}
//				}
//				if(pipecheck==1 || greatercheck==1){
//					int k=0;
//					for(;k<val;k++)
//					{
//						temp[k]=new char[strlen(command.argv[k])+1];
//						strcpy(temp[k],command.argv[k]);
//					}
//					temp[k++]=NULL;	
//					int j=0;
//					for(int m=val+1;m<command.argc;m++,j++)
//					{
//						command.argv[j]=new char[strlen(command.argv[m])+1];
//						strcpy(command.argv[j],command.argv[m]);
//					}
//					command.argv[j++]=NULL;	
//					command.argc=j-1;
//					command.name=lookUpPath(temp,pathv,directorysize);
//				}
//				else
//					command.name=lookUpPath(command.argv,pathv,directorysize);
//				if(command.name==NULL)
//				{
//					cout<<"Command not Found in Respective Directories"<<command.argv[0]<<endl;
//					continue;
//				}
//				if(greatercheck==1)
//				{
//					if(i%2==0)
//						Filing(temp, command.argv,B);
//					else
//						Filing(temp, command.argv,A);
//				}
//				else
//				{
//					if(i%2==0)
//						pipe(B);
//					else
//						pipe(A);
//					pid_t pid=fork();
//					if(pid==-1)
//						cout<<"Error in Fork"<<endl;
//					else if(pid==0)
//					{
//						cout<<command.name<<i<<endl;
//						child(i,totalpipes+1);
//						if(pipecheck==1 || greatercheck==1 )
//							execv(command.name,temp);
//						else
//							execv(command.name,command.argv);
//
//						return 0;			
//					}
//					else
//					{
//						wait(NULL);
//						parent(i,totalpipes+1);
//					}
//				}
//			}
//		}
//		else
//		{
//			cout<<command.argv[0]<<endl;
//			command.name=lookUpPath((command.argv),pathv,directorysize);
//			if(command.name==NULL)
//			{
//				cout<<"Command not Found in Respective Directories"<<endl;
//				continue;
//			}			
//			pid_t pid=fork();
//			if(pid==0)
//			{
//				execv(command.name,command.argv);
//				return 0;
//			}
//		}
//		wait(NULL);
//	}
//
//	cout<<"Program has been terminated"<<endl;
//	return 0;
//}
