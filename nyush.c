#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

// #include "argmanip.h"

int temp_index;
int rec_num;


// void handler(){

// }

typedef struct{
  pid_t pid;
  char* command[128];
} JOB;

JOB jobs[100];
int job_count = 0;


void add(pid_t pid, char* comm[]){
  int n =0;

  //How many words in command

  while (comm[n]!=NULL){
    // printf("%s",comm[n]);
    n++;
  }

  //copying command
  for(int i=0;i<n;i++){
    int k=0;
    while(comm[k]!=NULL){
      k++;
    }
    jobs[job_count].command[i] = (char*)malloc((k+1)*sizeof(char));
    
    strcpy(jobs[job_count].command[i],comm[i]);
    // printf("k= %d",k);
    jobs[job_count].command[k] = NULL;
  }

  //coyping pid
  jobs[job_count].pid = pid;

  // strcpy(jobs[job_count].command,command);
  job_count++;
}

void listing(void){
  // printf("%d\n",job_count);
  if(job_count>0){
    for(int i=0;i<job_count;i++){
      int n =0;
      while (jobs[i].command[n]!=NULL){
        n++;
      }
      printf("[%d] ", i+1 );
      for(int j=0; j<n;j++){
        if(j==n-1) printf("%s",jobs[i].command[j]);
        else{
          printf("%s ",jobs[i].command[j]);
        }
        
      }
      printf("\n");
      // printf("[%d] %s\n",i,jobs[i].command);
    }
  }
  
}

void resume(int index){

  index = index-1;

  if(index <= job_count-1){


    pid_t pid = jobs[index].pid;

    char* comm[128];
    memcpy(comm,jobs[index].command,128);


    for(int i=0;i<job_count;i++){
      if(i>=index ) jobs[i] = jobs[i+1];
    }
    job_count--;


    kill(pid,SIGCONT);


    int status;
    waitpid(pid, &status,WUNTRACED);
    if(WIFSTOPPED(status)){
      add(pid,comm);
    }

  }
  else{
    fprintf(stderr,"Error: invalid job\n");
  }
}


void my_system(char* command[], char* left[]){
  
  pid_t pid;

  // signal(SIGINT, SIG_IGN);
  // signal(SIGTSTP, SIG_IGN);
  // signal(SIGCHLD, SIG_IGN);

  if((pid=fork())==0){
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    execvp(command[0],command);
    // execvpe(command[0],command,"/usr/bin"); ??? try this later?
    fprintf(stderr,"Error: invalid program\n");
    exit(-1);

  }
  else{


    int status;

    
    waitpid(pid,&status,WUNTRACED);



    if(WIFSTOPPED(status)){
      add(pid,left);

    }


  }
  
}




char** process_command(char command[]){
  char** ret = (char**)malloc(1000*sizeof(char*));

  int count =0;
  char* token2 = strtok(command," ");



  while(token2!=NULL){
    ret[count] = token2;
    token2 = strtok(NULL," ");
    count = count +1;  
  }

  return ret;


}


bool includes(char command[]){

  int ind[8] = {62,60,124,42,33,96,39,34};
  for(int i=0;i<8;i++){
    if(strchr(command,ind[i])!=NULL) return true;
  }
  return false;
}


bool is_terminate(char* processed_command[],int n){
  temp_index = n;
  if(processed_command[n]==NULL) return true;
  if(strcmp(processed_command[n],">")==0){
    if(processed_command[n+1]==NULL){
      // printf("term1\n");
      return false;
    }
    if(!includes(processed_command[n+1])){
      temp_index++;
      if(processed_command[n+2]==NULL) return true;
      if(strcmp(processed_command[n+2],"<")==0){
        if(processed_command[n+3]==NULL) return false;
        if(!includes(processed_command[n+3])){
          if (processed_command[n+4]==NULL) return true;
        }
      }
    }
    return false;
  }

  if(strcmp(processed_command[n],">>")==0){
    if(processed_command[n+1]==NULL){
      return false;
    }
    if(!includes(processed_command[n+1])){
      temp_index++;
      if(processed_command[n+2]==NULL) return true;
      if(strcmp(processed_command[n+2],"<")==0){
        if(processed_command[n+3]==NULL) return false;
        if(!includes(processed_command[n+3])){
          if (processed_command[n+4]==NULL) return true;
        }
      }
    }
    return false;
  }
  return false;
}

int is_cmd(char* processed_command[],int n){
  if(processed_command[n+1]==NULL) return 0;
  if(!includes(processed_command[n+1]))  return 1;
    // return 1+is_cmd(processed_command,n+1);
  return 0;
}

bool is_recursive(char* processed_command[],int n){
  if(processed_command[n]==NULL){
    return false;
  }
  if(strcmp(processed_command[n],"|")==0){
    if(processed_command[n+1]==NULL){
      return false;
    }
    int k = is_cmd(processed_command,n+1);
    if(includes(processed_command[n+1])){
      return false;
    }
    if(processed_command[n+2+k]==NULL){
      return true;
    } 

    if(strcmp(processed_command[n+2+k],"|")==0){
      return is_recursive(processed_command,n+2+k);
    }
    return is_terminate(processed_command, n+2+k);
  }
  return false;
}


int determine_which(char* processed_command[]){


    int i =0;

    if(processed_command[i]==NULL) return 0;

    else if(strcmp(processed_command[i],"cd")==0){
      if(processed_command[1]!=NULL){
        if(includes(processed_command[1])) return -3;
        if(processed_command[3]== NULL) return 1;
      }
      

      return -3;
    }

    else if(strcmp(processed_command[i],"jobs")==0){
      if(processed_command[1]==NULL) return 2;
      return -3;
    }

    else if(strcmp(processed_command[i],"fg")==0){
      if(processed_command[1]!=NULL){
        if(includes(processed_command[1])) return -3;
        if(processed_command[3]== NULL) return 3;
      }
      
      return -3;
    }

    else if(strcmp(processed_command[i],"exit")==0){
      if(processed_command[1]==NULL) return 4;
      return -3;
    } 
    
    else if(strcmp(processed_command[i],"fsck")==0){
      if(processed_command[1]==NULL) return 5;
      return -3;
    } 
    
    else if(includes(processed_command[i])) return -1;


    int k = is_cmd(processed_command,i);

    //2 [cmd] [terminate]
    if(processed_command[1+k]==NULL) return 6;

    
    // [cmd] '<' [filename] [terminate]
    // [cmd] '<' [filename] [recursive]
    if(strcmp(processed_command[1+k],"<")==0){
      if(processed_command[2+k]==NULL) return -1;
      if(!includes(processed_command[2+k])){
        if(processed_command[3+k]==NULL) return 6;
        if(is_terminate(processed_command,3+k)) return 6;
        if(is_recursive(processed_command,3+k)) return 7;
        return 6;
      }
    }

    else if(is_recursive(processed_command,1+k)) return 8;
    else if(is_terminate(processed_command,1+k)) return 9;

  return 6;
}


void redirecting(char* processed_command[]){
  //may be if it's moving to the same directory it should print nothing?
  int a = chdir(processed_command[1]);
  if (a==-1) fprintf(stderr, "Error: invalid directory\n");
}


void filing(char* command[],char* which, char* filename, char* left[]){

  // printf("hi this is filing");
  // pid_t pid;

  int stdout_copy = dup(STDOUT_FILENO);

  // if((pid=fork())==0){
    int fd;

    if(strcmp(which,">>")==0) {
      fd = open(filename, O_WRONLY|O_CREAT|O_APPEND);
    }

    else{
      fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC);
    }

    if(dup2(fd,STDOUT_FILENO)<0){
      close(fd);
      exit(-1);
    }

    close(fd);

    my_system(command, left);
    // execvp(command[0],command);
    
    
    // exit(-1);

  // }
  // else if(pid >0){
  //   wait(NULL);
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
  // } 
  // else{
  //   perror("fork failed");
  //   exit(-1);
  // }


}


void filing2(char* command[], char* inputfile, char* which, char* filename, char* left[]){

  // pid_t pid;
  int stdin_copy = dup(STDIN_FILENO);
  int stdout_copy = dup(STDOUT_FILENO);


  // if((pid=fork())==0){
    int fd;
    int fd2 = open(inputfile, O_RDONLY);

    if(fd2 <0) fprintf(stderr,"Error: invalid file\n");

    else{
    if(strcmp(which,">>")==0) {
      fd = open(filename, O_WRONLY|O_CREAT|O_APPEND);
    }

    else{
      fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC);
    }

    if(dup2(fd,STDOUT_FILENO)<0){
      close(fd);
      exit(-1);
    }

    if(dup2(fd2,STDIN_FILENO)<0){
      close(fd2);
      exit(-1);
    }


    close(fd);
    close(fd2);

    
    my_system(command,left);


    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdin_copy);
    close(stdout_copy);
  }

}

void filing3(char* command[], char* inputfile, char* left[]){

  // pid_t pid;
  int stdin_copy = dup(STDIN_FILENO);

    


  int fd2 = open(inputfile, O_RDONLY);

  if(fd2 <0) fprintf(stderr,"Error: invalid file\n");

  else{

  if(dup2(fd2,STDIN_FILENO)<0){
    close(fd2);
    exit(-1);
  }

  close(fd2);

  
  my_system(command,left);

  dup2(stdin_copy, STDIN_FILENO);

  close(stdin_copy);

  }
}






void recursing(char* res[], int rec_num, int count){

  // printf("hi recursing\n");


  if(rec_num > count){

    int length = snprintf(NULL,0,"%d",count);
    char* output = malloc(length+1+4);
    snprintf(output, length+1,"%d",count);
    strcat(output,".txt");
    char* input = malloc(length+1+4);
    snprintf(input, length+1,"%d",count-1);
    strcat(input,".txt");

    int n =0;
    int inp = 0;
    int k =0;

    while(res[k]!=NULL){
      k=k+1;
    }

    while(res[n]!=NULL&& strcmp(res[n],"|")!=0){
      if (strcmp(res[n],"<")==0) inp = n;
      n= n+1;
    }


    char** next_rec = (char**)malloc((k-n)*sizeof(char*));
    
    for(int i=n+1;i<k;i++){
      next_rec[i-n-1] = res[i];
    }

    char** run_comm = (char**)malloc((n+1)*sizeof(char*));
    char** left = (char**)malloc((n+1)*sizeof(char*));


    if(inp){
      for(int i=0; i<n;i++){
        left[i] = res[i];
        run_comm[i] = res[i];
      }

      char* filename = run_comm[inp+1];


      run_comm[inp] = NULL;
      run_comm[inp+1] = NULL;



      // printf("firstcase: \n");
      filing2(run_comm,filename,">",output,left);

    }

    else{
      for(int i=0; i<n;i++){
        left[i] = res[i];
        run_comm[i] = res[i];
      }
      run_comm[n] = NULL;

      // printf("secondcase: \n");
      if(count ==0) filing(run_comm,">",output,left);


      else{
        filing2(run_comm,input,">",output,left);
      }
    }

    recursing(next_rec,rec_num,count+1);

  }

  else{

    int n =0;
    int move = 0;

    while(res[n]!=NULL&& strcmp(res[n],"|")!=0){
      if (strcmp(res[n],">")==0||strcmp(res[n],">>")==0) move = n;
      n= n+1;

    }

    int length = snprintf(NULL,0,"%d",count);
    char* input = malloc(length+1+4);
    snprintf(input, length+1,"%d",count-1);
    strcat(input,".txt");
    
    char** run_comm = (char**)malloc((n+1)*sizeof(char*));
    char** left = (char**)malloc((n+1)*sizeof(char*));

    if(move){
          for(int i =0; i< n; i++){
            left[i] = res[i];
            if(i<move) run_comm[i] = res[i];
            else if((i==move)||(i==move+1)){
              continue;
            }
            else{
              run_comm[i-2] = res[i];
            }
          }
          run_comm[n-2] = res[move]; //<< or <
          run_comm[n-1] = res[move+1]; // filename

          char** input_red = (char**)malloc((n+2)*sizeof(char*));
          for(int i=0; i<n-2;i++){
            input_red[i] = res[i];
          }
          input_red[n-2] =NULL;
          // printf("thrid: \n");
          filing2(input_red, input, run_comm[n-2] ,run_comm[n-1],left);
    }
    else{
      for(int i=0; i<n;i++){
        left[i] = res[i];
        run_comm[i] = res[i];
      }
      run_comm[n] = NULL;

      // printf("fourth: \n");
      filing3(run_comm,input,left);
    }



  }

}


int main(void) {
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);

  temp_index =0;
  int root =0;
  char cwd[256];
  char buffer[32];
  char *b = buffer;
  size_t bufsize =1024;


  while (true){
    rec_num =0;
    root =0;

    getcwd(cwd,sizeof(cwd));
    char* token = strtok(cwd,"/");
    char* comm;
    while (token!=NULL){
      comm = token;
      token = strtok(NULL,"/");
      root++;
    }
    if(root<=1){
      printf("[nyush /]$ ");
    }
    //Printing Bash
    else{
      printf("[nyush %s]$ ",comm);
    }

    fflush(stdout);
    getline(&b, &bufsize, stdin);
    b[strlen(b)-1] = '\0';
    
    char** res = process_command(b);
    int det = determine_which(res);
    if(det==1){
      redirecting(res);
    } 
    else if(det<0){
      fprintf(stderr, "Error: invalid command\n");
    }
    else if(det==4){
      if(job_count==0){
        // printf("\n");
        exit(0);
      }

      // printf("\n");
      // exit(0);
      else{
        fprintf(stderr,"Error: there are suspended jobs\n");
      }
      // free(res);
      // exit(0);
    }
    else if(det==2){
      // printf("jobs done\n");
      listing();
    }
    else if(det ==3){
      int num1 = atoi(res[1]);
      // printf("indeex - %d",num1);
      resume(num1);
    }
    else if(det==5){
      fprintf(stderr, "Error: invalid program\n");
    }

    else{

      //if I could make this into a function...?

      // if rec_num = 0 => Then just do the same thing

      // if rec_num != 0 => Then do output => temp.file. Then make it (input, commandline)

      
      int m =0;
      
      

      while(res[m]!=NULL){
        if (strcmp(res[m],"|")==0) rec_num++;
        m++;
      }



      

      if(rec_num==0){

        // Function that takes rec(res,rec_num)

        int n =0;
        int move = 0;
        int inp = 0;

        while(res[n]!=NULL&& strcmp(res[n],"|")!=0){
          if (strcmp(res[n],">")==0||strcmp(res[n],">>")==0) move = n;
          if (strcmp(res[n],"<")==0) inp = n;
          n= n+1;

        }


        



        if(feof(stdin)){

          printf("\n");
          exit(0);
          // if(job_count==0){
          //   printf("\n");
          //   exit(0);
          // }
          // // printf("\n");
          // // exit(0);
          // else{
          //   printf("Error: there are suspended jobs\n");
          //   signal(SIGQUIT, handler);
          // }
        }

        char** run_comm = (char**)malloc((n+1)*sizeof(char*));
        char** left = (char**)malloc((n+1)*sizeof(char*));
        if(move){
          for(int i =0; i< n; i++){
            left[i] = res[i];
            if(i<move){
              run_comm[i] = res[i];

            }
            else if((i==move)||(i==move+1)){
              continue;
            }
            else{
              run_comm[i-2] = res[i];
            }
          }
          run_comm[n-2] = res[move]; //<< or <
          run_comm[n-1] = res[move+1]; // filename


          if(inp){




            char** input_red = (char**)malloc((n+2)*sizeof(char*));

            for(int i=0; i<n-2;i++){
              input_red[i] = run_comm[i];
            }
            input_red[n-2] = NULL;
            input_red[n-4] = NULL; // filename
            input_red[n-3] = NULL; //

            // for(int i=0;i<n-3;i++) printf("%s",input_red[i]);
            filing2(input_red, res[inp+1], run_comm[n-2] ,run_comm[n-1],left);

            

          }

          else{
            char** input_red = (char**)malloc((n+2)*sizeof(char*));
            for(int i=0; i<n-2;i++){
              input_red[i] = res[i];
            }
            input_red[n-2] =NULL;
            filing(input_red, run_comm[n-2] ,run_comm[n-1],left);
          }

        }

        else{
          if(inp){
            for(int i=0; i<n;i++){
              left[i] =res[i];
              run_comm[i] = res[i];
            }

            char* filename = run_comm[inp+1];

            run_comm[inp] = NULL;
            run_comm[inp+1] = NULL;

            filing3(run_comm,filename,left);

          }
          else{
            for(int i=0; i<n;i++){
              left[i]= res[i];
              run_comm[i] = res[i];
            }
            run_comm[n] = NULL;


            my_system(run_comm,run_comm);

          }
        }
      }

      else{
        recursing(res,rec_num,0);
        // printf("recursing done!\n");
        for(int i=0;i<rec_num;i++){
          int length = snprintf(NULL,0,"%d",i);
          char* fil = malloc(length+1+4);
          snprintf(fil, length+1,"%d",i);
          strcat(fil,".txt");

          remove(fil);

        }

      }

      

    }

  }
}


