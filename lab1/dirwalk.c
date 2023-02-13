#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <locale.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>

#define FILE_NAME_LENGTH 256

struct DirConfig   // Изначальная конфигурация
{
  char * dir;
  char **ms;
  int size;
  bool null_flag; 
  bool type_f;
  bool type_d;
  bool type_l;
  bool flag_s;
};

void DefaultConfig(struct DirConfig* conf)
{
    conf->dir = ".";
    conf->ms = (char**)malloc(sizeof(char*));
    conf->size = 0;
    conf->null_flag = true;
    conf->type_f = false;
    conf->type_d = false;
    conf->type_l = false;
    conf->flag_s = false;
}

void PrintDir(const struct DirConfig* conf)
{
for(int i = 0; i<conf->size; i++)
{
    printf("%s\n", conf->ms[i]);
}
}

int CompareChar(const void *a, const void *b)
{
    if(strcoll((char*)a, (char*)b)>0)
        return 1;
    if(strcoll((char*)a, (char*)b)<0)
        return -1;
    if(strcoll((char*)a, (char*)b)==0)
        return 0;
}



void GetDir(struct DirConfig* conf) 
{
	DIR* dp;
	struct dirent* entry;
	struct stat statbuf;

	if ((dp = opendir(conf->dir)) == NULL) {
		fprintf(stderr, "Can`t open directory %s\n", conf->dir);
		return;
	}

	chdir(conf->dir); //смена директории 

	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);//если проверяемый объект ссылка - возвращается информация о ссылке, а не об объекте на который ссылается (отличие от stat)
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0)
				continue;
			if(conf->null_flag||conf->type_d)
                {
                    conf->ms = (char**)realloc(conf->ms, sizeof(char*)*(conf->size+1));
                    conf->ms[conf->size] = (char*)malloc(sizeof(char)*FILE_NAME_LENGTH);
                    strcpy(conf->ms[conf->size],entry->d_name);
                    conf->size++;
                }

            char *tmp = conf->dir;
            conf->dir = entry->d_name;
			GetDir(conf);
            conf->dir = tmp;
		}
		else if(S_ISREG(statbuf.st_mode)&&(conf->null_flag || conf->type_f))
        {
            conf->ms = (char**)realloc(conf->ms, sizeof(char*)*(conf->size+1));
            conf->ms[conf->size] = (char*)malloc(sizeof(char)*FILE_NAME_LENGTH);
            strcpy(conf->ms[conf->size],entry->d_name);
            conf->size++;
        }
		else if(S_ISLNK(statbuf.st_mode) && (conf->null_flag || conf->type_l))
        {
            conf->ms = (char**)realloc(conf->ms, sizeof(char*)*(conf->size+1));
            conf->ms[conf->size] = (char*)malloc(sizeof(char)*FILE_NAME_LENGTH);
            strcpy(conf->ms[conf->size],entry->d_name);
            conf->size++;
        }
		else if(conf->null_flag)
        {
            conf->ms = (char**)realloc(conf->ms, sizeof(char*)*(conf->size+1));
            conf->ms[conf->size] = (char*)malloc(sizeof(char)*FILE_NAME_LENGTH);
            strcpy(conf->ms[conf->size],entry->d_name);
            conf->size++;
        }
	}
	chdir("..");
    closedir(dp);
}



int main(int argc, char* argv[])
{
	struct DirConfig conf;
    DefaultConfig(&conf);

	if (argc >= 2) {
		conf.dir = argv[1];
		for(int i = 2; i<argc; i++) // определение флагов
		{
			if (!strcmp(argv[i], "-type")) 
			{
				if (i == argc - 1) {
					fprintf(stderr, "Не передан флаг после type \n");
					return 1;
				}

                conf.null_flag = false;
				i++;
				if (!strcmp(argv[i], "d"))
					conf.type_d = true;
				else if (!strcmp(argv[i], "f"))
					conf.type_f = true;
				else if (!strcmp(argv[i], "l"))
					conf.type_l = true;
				else {
					fprintf(stderr, "Некорректный флаг \n");
					return 1;
				}
			}

			if (!strcmp(argv[i], "-s")) 
                conf.flag_s = true;
		}
	}

	printf("Directory scan of %s\n", conf.dir);
	GetDir(&conf);
    if(conf.flag_s == true)
        qsort(conf.ms, conf.size, sizeof(char*), CompareChar);
    PrintDir(&conf);
	printf("done.\n");
	exit(0);
}
