#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 30

typedef struct{
    char c;
    int rep;
}CharRepeat;

CharRepeat* CR_array_create(){
    CharRepeat* res = (CharRepeat*) malloc(26 * sizeof(CharRepeat));
    for(int i = 0; i < 26; i ++){
        res[i].c = i + 'a';
        res[i].rep = 0;
    }
    return res;
}

typedef struct{
    char* str;
    CharRepeat* res;
}pthrData;

void repeat(CharRepeat* res, char* str){
    for(int i = 0; i < strlen(str); i ++){
	if(str[i] >= 'a' && str[i] <= 'z'){
	    res[str[i] - 'a'].rep ++;
	} else if(str[i] >= 'A' && str[i] <= 'Z'){
	    res[str[i] - 'A'].rep ++;
        } else if(str[i] == '\n'){
            break;
        }
    }
}

void* thread_func(void* thread_data){
    pthrData* data = (pthrData*) thread_data;
    repeat(data -> res, data -> str);
    return NULL;
}

int int_read(char* buf){
    int res = 0;
    for(int i = 0; i < strlen(buf); i ++){
        if(buf[i] >= '0' && buf[i] <= '9'){
            res = res * 10 + (buf[i] - '0');
        } else {
            return -1;
        }
    }
    return res;
}
        
        

int main(int argc, char *argv[]){
    if(argc != 3){
	printf("Incorrect number of arguments\n");
        exit(1);
    }
    int fr;
    fr = open(argv[1], O_RDONLY, 0);
    if (fr == -1) {
	printf("Cannot open file.\n");
	exit(1);
    }

    int number_thread = int_read(argv[2]);
    if(number_thread < 0){
        printf("Incorrect input of the second argument");
        exit(1);
    }

    struct stat st;
    if(fstat(fr, &st) < 0){
        printf("fstat faild\n");
        exit(1);
    }

    int n = 0;
    int fsize = st.st_size;
    char str[fsize];
    CharRepeat* res = CR_array_create();
    read(fr, str, fsize);
    close(fr);
    for(int i = 0; i < fsize; i++){
        if(str[i] == EOF || str[i] == '\n'){
            n++;
        }
    }
    
    pthread_t* threads = (pthread_t*) malloc(n * sizeof(pthread_t));

    pthrData* threadData = (pthrData*) malloc(n * sizeof(pthrData));

    int k = 0;
    int i = 0;
    while(k < number_thread && k < n && i < fsize){
        threadData[k].res = CR_array_create();
        threadData[k].str = (char*) malloc(SIZE * sizeof(char));
        int j = 0;
        while(i < fsize){
            if(str[i] == EOF || str[i] == '\n'){
                break;
            }
            threadData[k].str[j] = str[i];
            j ++;
            i ++;
        }
        int tmp = pthread_create(&(threads[k]), NULL, thread_func, &threadData[k]);
        if(tmp != 0){
            printf("Maximum Thread Exceeded");
            thread_func(&threadData[1]);
            k ++;
            i ++;
            break;
        }
        k ++;
        i ++;
    } 
    for(int h = 0; h < k; h++){
        pthread_join(threads[h], NULL);
    }
    free(threads);

    if(i != fsize){
        char buf[fsize - i];
        int f = 0;
        while(i < fsize){
            if(str[i] == '\n'){
                i ++;
                continue;
            }
            buf[f] = str[i];
            f ++;
            i ++;
        }
        repeat(res, buf);
    }
    for(int i = 0; i < k; i ++){
        for(int j = 0; j < 26; j ++){
            res[j].rep += threadData[i].res[j].rep;
        }
    }
    free(threadData);
    for(int i = 0; i < 26; i ++){
        if(res[i].rep != 0){
            printf("%c - %d \n", res[i].c, res[i].rep);
        }
    }
}
