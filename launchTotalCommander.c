#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>



#define __FTP_FILE__

#define MAX_FILENAME_LEN	256
#define MAX_CONFIG_NUM		16

#define OP_FAIL				-1
#define OP_SUCCESS			1

#define TC_PATH				"C:\\totalcmd\\"
#define TC_FILE				"C:\\totalcmd\\TOTALCMD.EXE"
#define CONF_PATH			"C:\\totalcmd\\configrations\\"
#define FTP_FILE			CONF_PATH "Wcx_ftp.ini"

#define ARGS_EXE			0
#define ARGS_WINCMD			1
#define ARGS_FTP			2


//paths[0] is used as temp buffer
char paths[MAX_CONFIG_NUM][MAX_FILENAME_LEN];

char args[][MAX_FILENAME_LEN] = 
{
	TC_FILE,
	0,
#ifdef __FTP_FILE__	
	FTP_FILE,
#endif	
};


char* cmds[MAX_FILENAME_LEN] = 
{
	args[0],
	args[1],
#ifdef __FTP_FILE__	
	args[2],
#endif	
	(char *)0
};


int strSubInvalidchar(char *src,char *des);
int readconfs(void);
int newConf(char* name);
int getUserChoice(void);
int createFileName(int userChoice);
void setupExecArgs(void);

int isInConf(int confSum,char* fileName);
void cleanUp(int confSum);


int main()
{
	int confNum = 0;
	int userChoice = 0;

	//read configureations from file
	if((confNum = readconfs()) == OP_FAIL)
	{
		printf("read configuration fail!\n");
		return -1;
	}	
	
	//get user command.
	userChoice = getUserChoice();
	if((userChoice > confNum) || (userChoice < 0))//now index is pointing to next free slot
	{
		printf("it is not a configuration\n");
		return -1;
	}

	//perform command
	if(userChoice == 0)
	{
		///perform clean up
		cleanUp(confNum);
		printf("clean up complete\n");
	}
	else
	{
		createFileName(userChoice);
		setupExecArgs();
		if(_access((const char*)(&(args[ARGS_WINCMD][3])),_A_NORMAL))//exclude "/i="
		{
			//file does not exists.
			newConf(&(args[ARGS_WINCMD][3]));
		}	
		_execv(args[ARGS_EXE],cmds);
	}
	system("pause");
	return 0;
}

/********************************************************/
/* @function	cleanUp					*/
/* @param	confNum: number of configurations. 	*/
/* @return						*/
/* @description check if file is now in configure	*/
/********************************************************/
void cleanUp(int confSum)
{
	struct _finddata_t	filedes;
	intptr_t fhandle;

	//change to configuration directory.
	_chdir(CONF_PATH);

	if((fhandle = _findfirst("*.ini",&filedes) )== -1)
	{
		printf("no configuration files\n");
		return ;
	}
	if(!isInConf(confSum,filedes.name))
	{	
		if(remove(filedes.name) == -1)
		{
			printf("remove  %s failed\n",filedes.name);
		}
		else
		{
			printf("remove %s\n",filedes.name);
		}
	}	
	while((_findnext(fhandle,&filedes)) == 0)
	{
		if(!isInConf(confSum,filedes.name))
		{	
			if(remove(filedes.name) == -1)
			{
				printf("remove  %s failed\n",filedes.name);
			}
			else
			{
				printf("remove %s\n",filedes.name);
			}
		}		
	}
	_findclose(fhandle);

	return;
}

/********************************************************/
/* @function	isInConf				*/
/* @param	confNum: number of configurations.	*/
/*		fileName:config file name 		*/
/* @return						*/
/* @description check if file is now in configure	*/
/********************************************************/
int isInConf(int confSum,char* fileName)
{
	int i;
	for(i = 1;i <= confSum;i++)
	{
		createFileName(i);
		if(!strcmp(paths[0],fileName))
		{
			return 1;
		}
	}
	return 0;
}

//create new configure file
int newConf(char* name)
{
	char cmd[MAX_FILENAME_LEN<<1] = {0};
	sprintf(cmd,"copy %swincmd.ini %s",TC_PATH,name);
	if(system(cmd) != 0)
	{
		printf("newConf fail\n");
		return OP_FAIL;
	}
	return OP_SUCCESS;
}

/********************************************************/
/* @function	setupExecArgs				*/
/* @param						*/
/* @return						*/
/* @description setup arguments for exec call		*/
/********************************************************/
void setupExecArgs(void)
{
	//args[0]
	//sprintf(args[ARGS_EXE],TC_PATH);
	//strcat(args[ARGS_EXE],"TOTALCMD.EXE");

	//args[1]
	sprintf(args[ARGS_WINCMD],"/i=");
	strcat(args[ARGS_WINCMD],CONF_PATH);
	strcat(args[ARGS_WINCMD],paths[0]);

#ifdef __FTP_FILE__
	//args[2]
	//sprintf(args[ARGS_FTP],"/F=");;
	//strcat(args[ARGS_FTP],TC_PATH);
	//strcat(args[ARGS_FTP],FTP_FILE);
#endif
	return;
}


/********************************************************/
/* @function	createFileName				*/
/* @param						*/
/* @return						*/
/* @description create file name string according 
				to user choice		*/
/********************************************************/
int createFileName(int userChoice)
{
	if(strSubInvalidchar(paths[userChoice],paths[0]) == OP_FAIL)
	{
		printf("createFileName fail\n");
		return OP_FAIL;
	}
	strcat(paths[0],".ini");

	return OP_SUCCESS;
}


/********************************************************/
/* @function	getUserChoice				*/
/* @param						*/
/* @return		configuration index		*/
/* @description get user choice of configuration frmo
				console			*/
/********************************************************/
int getUserChoice(void)
{
	int in ;
	
	printf("\nchoose:");
	scanf("%d",&in);

	return in;
}

/********************************************************/
/* @function	readconfs				*/
/* @param						*/
/* @return		number of valid configurations	*/
/* @description read configurations to paths[][] 	*/
/********************************************************/
int readconfs(void)
{
	int index = 1;
	char commentChar;
	FILE* fpconf ;

	fpconf = fopen("C:\\totalcmd\\configrations\\paths.txt","r");
	if(fpconf == NULL)
	{
		printf("paths.txt open fail\n");
		//system("pause");
		return OP_FAIL;
	}

	//show config file list
	printf("0.clean unused file\n");

	while(!feof(fpconf) && (index < (MAX_CONFIG_NUM)))
	{
		commentChar = fgetc(fpconf);
		ungetc(commentChar,fpconf);
		if(commentChar == '#')
		{
			fgets(paths[0],MAX_FILENAME_LEN,fpconf);
		}
		else
		{
			fgets(paths[index],MAX_FILENAME_LEN,fpconf);
			if(strlen(paths[index]) < 2)//skip empty line
				continue;

			printf("%d.",index);
			printf("%s",paths[index]);
			index++;
		}
		
	}
	
	fclose(fpconf);
	
	return index-1;

}

//substitute certain chars with specified char
int strSubInvalidchar(char *src,char *des)//,char s,char s2,char t)
{
	char* p = src;
	char* q = des;

	if((src == NULL) || (des == NULL))// || (sizeof(src) > sizeof(des)))
	{
		printf("strSubInvalidchar fail\n");
		return OP_FAIL;
	}

	while( *p != '\0')
	{
		if((*p == '\\') || (*p == '\n') || (*p == ' ') || (*p == ':'))
		{
			*q = '_';//t;
		}
		else
		{
			*q = *p;
		}
		p++;
		q++;
	}
	*q = '\0';
	
	return OP_SUCCESS;
}
