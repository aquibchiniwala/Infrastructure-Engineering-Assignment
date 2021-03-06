#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

typedef struct topFiles         //selected top 10 files.
{        
    float fsize;
    char fpath[300];
    char fname[100];
    
}files;

files top10Files[10];
files desktopFiles[500];
int top10_Count=0,desktop_Count=0,total_Count=0,flag=0;                                
//total_Count for total files scanned and top10_Count for total no. structure objects created for top 10 files.
//desktop_Count to calculate total no of files scanned on desktop.

int checkPath(char * path)      //check whether the user defined path exists or not.
{

    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
        return 0;
    else
        return -1;
}

char *get_filename_ext(char *filename)      //get file extention of the file. 
{
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "no_ext";            //if file with no extention(eg.webbing file)
    return dot + 1;
}

void sort(files top10Files[])               //sort the top 10 files in descending order.
{
   int i, j;
   files key;
   for (i = 1; i < top10_Count; i++)
   {
       key = top10Files[i];
       j = i-1;
 
       while (j >= 0 && top10Files[j].fsize > key.fsize)
       {
           top10Files[j+1] = top10Files[j];
           j = j-1;
       }
       top10Files[j+1] = key;
   }
}

float file_Size(const char *filename) //returns the size of the file.
{         
    struct stat st; 
    if (stat(filename, &st) == 0)
    {
        return ((float)st.st_size/1048576);  //convert bytes into MBs.
    }
    return -1; 
}

void insert_into_top10(float newFsize,char newFpath[],char newFname[])  //insert into the top10files list if the new file is larger than the smallest file in the list
{
    if(top10_Count<10)                //first [ 0 to 9 ] 10 files will be inserted without comparission.
    {
        top10Files[top10_Count].fsize=newFsize;
        strcpy(top10Files[top10_Count].fpath,newFpath);
        strcpy(top10Files[top10_Count].fname,newFname);
        top10_Count++;
        sort(top10Files);
    }   
    else if(top10Files[0].fsize<newFsize)   //after 10th file, the new file will be checked with the smallest file in the top 10 list.
    {                                       //if new file would be larger than it will be swapped with the smallest file than the list will be sorted.
        top10Files[0].fsize=newFsize;
        strcpy(top10Files[0].fpath,newFpath);
        strcpy(top10Files[0].fname,newFname);  
        sort(top10Files);
    }
}

void get_Files_N_Directories(char * root)   //get all files and directories recursively.
{
    struct dirent *de; 
    
    DIR *dr = opendir(root);
    if (dr == NULL)  
    {
        printf("\nCould not open [ %s ] directory.\n",root );
        exit(1);
    }
    while ((de = readdir(dr)) != NULL)
    {
       if(strcmp(de->d_name, ".") != 0  && strcmp(de->d_name, "..") != 0)
       {
           char path[300]={'\0'};
           strcpy(path,root);
           
            #ifdef __linux__             //path seperator for linux.
                strcat(path,"/");
            #else            
                strcat(path,"\\");      //path seperator for windows.
            #endif
           
           strcat(path,de->d_name);
           
            if(flag==0)             //WHEN SCANNING TOP 10 FILES
            {
                if(de->d_type!=4)       //if "FILE", then send to the insert_into_top10 function. 
                {
                     insert_into_top10(file_Size(path),path,de->d_name);
                     total_Count++;
                }
                else                         //if DIRECTORY, recursively call get_Files_N_Directories to get its files and directories.
                {
                     if(access(path, W_OK) == 0 || access(path, R_OK) == 0) 
                      //check whether the current user has access to the path or not.
                     {
                         get_Files_N_Directories(path);
                     }
                }
            }
            else                //WHEN SCANNING DESKTOP FILES WHICH ARE TO BE MOVED.
            {
                
                if(de->d_type!=4 && de->d_type!=10 && (strcmp(get_filename_ext(path),"lnk"))!=0 && (access(path, W_OK) == 0 || access(path, R_OK) == 0))      
                    //should be file but not shortcut and current user should have access to it.
                {
                    //if "FILE", then insert into desktopFiles.
                    strcpy(desktopFiles[desktop_Count].fpath,path);
                    strcpy(desktopFiles[desktop_Count].fname,de->d_name);
                    desktop_Count++;
                    total_Count++;
                }
                
            }
       }           
    }
    closedir(dr);   
}

void move_Files_In_Documents()        //create folders on documents with top 10 file extensions and move files there accordingly.
{
    char documents[50]={'\0'};
    #ifdef __linux__    //get documents path of current user in LINUX/UNIX.
        {
          strcpy(documents,getenv("HOME"));
          strcat(documents,"/Documents/");
        }
       #else            //get documents path of current user in Windows.
        {
         strcpy(documents,getenv("USERPROFILE"));
         strcat(documents,"\\My Documents\\");
        }
    #endif
    int i=0;
    for(i=0;i<desktop_Count;i++)
    {
        char temp[100]={'\0'};
        char *fileExt;
        fileExt=get_filename_ext(desktopFiles[i].fpath);
        strcat(temp,documents);
        strcat(temp,fileExt);
        
        
        #ifdef __linux__            //create folders on linux.
	{
	    mkdir(temp,0777);
            strcat(temp,"/");
	}
       #else                //create folders on windows.
	{            
	    mkdir(temp,777);
            strcat(temp,"\\");
	}
        #endif
        
        strcat(temp,desktopFiles[i].fname);
        
        rename(desktopFiles[i].fpath,temp);     //move  files  into the folders accordingly.

	temp[0]='\0';
    }
        printf("\n%d Files Moved Successfully..Thank You..\n",total_Count);
}

void display()          //display the top 10 files.
{
    int i=top10_Count,j=1;   
    for(j=1,i=top10_Count-1;i>=0;i--)
        printf("%d). %.4f MBs.==> %s\n",j++,top10Files[i].fsize,top10Files[i].fpath);

    printf("\nTotal files scanned : %d\n",total_Count);
}

int main() {
    
    char scanPath[300]={'\0'};
    printf("\nProgram 1 being executed.\tLISTING TOP 10 FILES");
    printf("\nIf you want to scan all the files Press ENTER..\nelse enter specific path to be scanned : \n");
    scanf("%[^\n]",scanPath);   //scan user defined path.
    if(scanPath[0]=='\0')       //if default path.
    {
        
        #ifdef __linux__              //default path for linux.
          strcpy(scanPath,"/home/");
        #else                         //default path for windows.
          strcpy(scanPath,"C:\\Users");
        #endif
    }
    else                    //if user defined path.
    {
     	if(checkPath(scanPath)!=0)  //check whether the user defined path exists or not.
    	{
           printf("Specified directory path not found..Program Exiting..\n");
           exit(1);
	}
        //check whether the current user has access to user defined path or not.
	else if(access(scanPath, W_OK) != 0 || access(scanPath, R_OK) != 0) 
    	{
           printf("You don't have permission to access specified directory path..Program Exiting..\n");
           exit(1);
    	}
    }
    printf("\nScanning path [ %s ]...This may take few minutes...Please wait...\n\n",scanPath);
    
    get_Files_N_Directories(scanPath);          //scanning path for top 10 files.
    
    printf("\nTop 10 files with maximum size : \n");
    
    display();
    total_Count=0;
    flag++;
    scanPath[0]='\0';
    
    #ifdef __linux__    //get Desktop path of current user in LINUX/UNIX.
        {
          strcpy(scanPath,getenv("HOME"));
          strcat(scanPath,"/Desktop/");
        }
#else            //get Desktop path of current user in Windows.
        {
         strcpy(scanPath,getenv("USERPROFILE"));
         strcat(scanPath,"\\Desktop\\");
        }
    #endif
    
    printf("\nProgram 2 being executed.\tDESKTOP CLEANER.");
    
    printf("\nMoving files from DESKTOP to DOCUMENTS...\n\n");

    get_Files_N_Directories(scanPath);      //scanning desktop files.
    
    move_Files_In_Documents();              //move files from desktop to documents.

    return 0;
}