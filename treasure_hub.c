/* SA SCHIMBI dir_principal ( variabila locala) cu directorul principal cu toate hunt urle.
gcc -Wall -o hub treasure_hub.c
./hub
cand vezi [Hub]: introduci una dintre optinunile prezentate in meniu. asteapta pana vezi [Hub]: 
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include<stdarg.h>

pid_t monitor_pid = -1;
int monitor_inchis = 0;
int monitor_pornit = 0;
char comanda[512];
char *dir_principal="treasure_hunt";
// luate din treasure_manager
typedef struct {
    int id;
    char name[40];
    float lant, lont;
    char clue[512];
    int value;
} COMOARA;


void list(char *hunt_path) {
    DIR *hunt_dir = opendir(hunt_path);
    if (!hunt_dir) {
        char *eroare = "nu s-a putut deschide directorul dat\n";
        write(1, eroare, strlen(eroare));
        return;
    }

    struct dirent *file;
    while ((file = readdir(hunt_dir)) != NULL) {
        if (file->d_type == DT_REG && strstr(file->d_name, "comoara") != NULL) {
            char file_path[128];
	    // snprintf(file_path, sizeof(file_path), "%s/%s", hunt_path, file->d_name);
	    strcpy(file_path,hunt_path);
	    strcat(file_path,"/");
	    strcat(file_path,file->d_name);

            struct stat st;
            if (stat(file_path, &st) == -1) {
                write(1, "eroare la stat\n", 15);
                continue;
            }

            printf("fisier: %s\n", file->d_name);
            printf("dimensiune: %ld bytes\n", st.st_size);
            printf("ultima modificare: %s", ctime(&st.st_mtime));

            int fd = open(file_path, O_RDONLY);
            if (fd == -1) {
                char *eroare = "eroare la deschiderea comorii\n";
                write(1, eroare, strlen(eroare));
                continue;
            }

            COMOARA c;
            while (read(fd, &c, sizeof(COMOARA)) == sizeof(COMOARA)) {
                printf("--> ID: %d | USER: %s | GPS: %.2f, %.2f | CLUE: %s | VALUE: %d\n",
                    c.id, c.name, c.lant, c.lont, c.clue, c.value);
            }
            close(fd);
        }
    }
    closedir(hunt_dir);
}

void view(char *hunt_path, int id) {
    DIR *dir = opendir(hunt_path);
    if (!dir) {
        char *eroare = "eroare la deschiderea directorului hunt\n";
        write(1, eroare, strlen(eroare));
        return;
    }

    struct dirent *comoara;
    char comoara_path[128];
    int exista = 0;

    while ((comoara = readdir(dir)) != NULL && !exista) {
        if (comoara->d_type == DT_REG && strstr(comoara->d_name, "comoara") != NULL) {
	    strcpy(comoara_path,hunt_path);
	    strcat(comoara_path,"/");
	    strcat(comoara_path,comoara->d_name);
            int fd = open(comoara_path, O_RDONLY);
            if (fd == -1) continue;

            COMOARA c;
            while (read(fd, &c, sizeof(COMOARA)) == sizeof(COMOARA)) {
                if (c.id == id) {
                    printf("Comoara gasita:\n");
                    printf("  ID: %d\n", c.id);
                    printf("  USER: %s\n", c.name);
                    printf("  GPS: %.2f - %.2f\n", c.lant, c.lont);
                    printf("  CLUE: %s\n", c.clue);
                    printf("  VALUE: %d\n", c.value);
                    exista = 1;
                    break;
                }
            }
            close(fd);
        }
    }

    closedir(dir);
    if (!exista)
        printf("Comoara cu id-ul %d nu a fost gasita.\n", id);
}

//de aici incepe partea noua a acestui milestone

void scrie_comanda(const char *cmd) {
    int fd = open("comanda.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0) {
        write(fd, cmd, strlen(cmd));
        close(fd);
    }
}

void citeste_comanda() {
    int fd = open("comanda.txt", O_RDONLY);
    if (fd == -1) {
        perror("[Monitor] Nu pot deschide fisierul comanda.txt");
        strcpy(comanda, "");//"golim" comanda
        return;
    }
    int len = read(fd, comanda, sizeof(comanda) - 1);
    if (len > 0) comanda[len] = '\0';
    else strcpy(comanda, "");
    close(fd);
}

void handle_monitor(int sig) {
    citeste_comanda();
    char *moni_text="[Monitor]:\n";
    write(1,moni_text,strlen(moni_text));
    
    if (strncmp(comanda, "list_hunts", 10) == 0) {
        DIR *dir = opendir(dir_principal);
        if (!dir) {
            write(1, "[Monitor] Nu există directorul treasure_hunt\n", 45);
            return;
        }
        struct dirent *d;
        while ((d = readdir(dir)) != NULL) {
            if (d->d_type == DT_DIR && strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
                char path[128];
		strcpy(path,dir_principal);
		strcat(path,"/");
		strcat(path,d->d_name);
                int count = 0;

                DIR *sub = opendir(path);
                if (sub) {
                    struct dirent *f;
                    while ((f = readdir(sub)) != NULL) {
                        if (f->d_type == DT_REG && strstr(f->d_name, "comoara") != NULL) count++;
                    }
                    closedir(sub);
                }

                char buf[256];
                //snprintf(buf, sizeof(buf), "[Monitor] Hunt %s are %d comori\n", d->d_name, count);
		char count_str[16];
		sprintf(count_str,"%d",count);
		strcpy(buf,d->d_name);
		strcat(buf," nr. comori: ");
		strcat(buf,count_str);
		strcat(buf,"\n");
                write(1, buf, strlen(buf)+1);
            }
        }
        closedir(dir);
    }

    else if (strncmp(comanda, "list treasures", 14) == 0) {
        char hunt[64];
        sscanf(comanda, "list treasures %s", hunt);
        char path[128];
        strcpy(path,dir_principal);//aici
	strcat(path,"/");
	strcat(path,hunt);
        list(path);
    }

    else if (strncmp(comanda, "view treasure", 13) == 0) {
        char hunt[64];
        int id;
	int verific=1;
        char *p=strtok(comanda," "); //aici
        p=strtok(NULL," ");
        if(p!=NULL)
          {
	    p=strtok(NULL," ");
	    if(p){
	       strcpy(hunt,p);
	     }
		else verific=0;
	   
	  }
	else verific=0;
        p=strtok(NULL," ");
        if(p==NULL)
	  verific=0;
	if(verific==1){
	id=atoi(p);
	  
        char path[128];
        strcpy(path,dir_principal);//aici
	strcat(path,"/");
	strcat(path,hunt);
        view(path, id);
	}
    }

    else if (strcmp(comanda, "stop_monitor") == 0) {
      char *mesaj="Oprire în 3 secunde...\n";
      write(1,mesaj,strlen(mesaj));
      sleep(3);
      char *mesaj2="Închis.\n";
      write(1,mesaj2,strlen(mesaj2));
      exit(0);
    }

    else {
      char *mesaj="Comandă necunoscută.\n";
      write(1, mesaj,strlen(mesaj));
    }
}

void handler_hub(int sig) {
    waitpid(monitor_pid, NULL, 0);
    monitor_pid = -1;
    monitor_inchis = 0;
    monitor_pornit = 0;
    char *mesaj="[Monitor oprit complet]\n";
    write(1,mesaj,strlen(mesaj));
}

int main() {

    struct sigaction sa;
    sa.sa_handler = handler_hub;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char comanda_input[64];
    int inca_merge = 1;

    char *mesaj_de_bun_venit="Bine ai venit la treasure hub!\n";
    write(1,mesaj_de_bun_venit,strlen(mesaj_de_bun_venit));
    char *meniu="Alege din optiunile: start monitor | list hunts | list treasures <huntid> | view treasure <huntid> <comoaraid> | stop monitor | exit\n";
    write(1,meniu,strlen(meniu));
    
    while (inca_merge) {
    
      if (monitor_inchis) {
	char *mesaj="Monitorul se opreste...\n";
	write(1,mesaj,strlen(mesaj));
      }

        printf("[Hub]: ");
	fflush(stdout);
	
        if (read(0, comanda_input, sizeof(comanda_input)) <= 0) continue;
        comanda_input[strcspn(comanda_input,"\n")]=0;
	
	if (strcmp(comanda_input, "exit") == 0) {
            if (monitor_pornit) {
                char *mesaj="!MAI INTAI OPRESTE MONITORUL!\n";
		write(1,mesaj,strlen(mesaj));
            } else {
                inca_merge = 0;
		continue;
            }
	}

	if(monitor_pornit==0 && strcmp(comanda_input, "start monitor") != 0)
	{
	  char *mesaj="Monitorul nu e pornit deci nu se pot efectua operatii.\n";
	  write(1,mesaj,strlen(mesaj));
	  continue;
	}
    
	
        if (strcmp(comanda_input, "start monitor") == 0) {
            if (monitor_pornit) {
	      char *mesaj="Monitorul este deja activ.\n";
	      write(1,mesaj,strlen(mesaj));
              continue;
            }
            pid_t pid = fork();
            if (pid == 0) {
                struct sigaction sa;
                sa.sa_handler = handle_monitor;
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = 0;
                sigaction(SIGUSR1, &sa, NULL);
                char *mesaj="Monitorul a fost pornit.\n";
		write(1,mesaj,strlen(mesaj));
		write(1,"[Hub]: ",7);
		while(1)
		  {
		    pause(); //acest loop infinit imi rezolva toate problemele (haha) monitorul ramane "activ" si asteapta semnalele
		  }
		
            }
            monitor_pid = pid;
            monitor_pornit = 1;
        }

        else if (strncmp(comanda_input, "list treasures", 14) == 0) {
            char hunt[64];
	    int verific=1;
	    char *p=strtok(comanda_input," ");
	    p=strtok(NULL," ");
	    if(p!=NULL)
	      {
		p=strtok(NULL," ");
		if(p){
		  strcpy(hunt,p);
		  }
		else verific=0;
	      }
	    else verific=0;
	    
            if (hunt!=NULL && verific==1) {
                char buffer[128];
                strcpy(buffer,"list treasures ");
		strcat(buffer,hunt);
	        scrie_comanda(buffer);
	        kill(monitor_pid, SIGUSR1);
		sleep(1);
            } else {
                printf("Format corect: list treasures <hunt>\n");
            }
        }

        else if (strncmp(comanda_input, "view treasure", 13) == 0) {
            char hunt[64];
	    int id=0;
	    int verific=1;
	    char *p=strtok(comanda_input," ");
	    p=strtok(NULL," ");
	    if(p!=NULL)
	      {
		p=strtok(NULL," ");
		if(p){
		  strcpy(hunt,p);
		  p=strtok(NULL," ");
		  if(p)
		    id=atoi(p);
		  else
		    verific=0;
		}
	        else
	           verific=0;
		}
              else
       	         verific=0;
	     
            if (id!=0 && hunt!=NULL && verific==1) {
                char buffer[128];
		strcpy(buffer,"view treasure ");
		strcat(buffer,hunt);
		strcat(buffer," ");
		strcat(buffer,p);
                scrie_comanda(buffer);
		kill(monitor_pid, SIGUSR1);
		sleep(1);
            } else {
	       char *mesaj="Format corect: view treasure <hunt> <id>\n";
	       write(1,mesaj,strlen(mesaj));
	       }
        }

        else if (strcmp(comanda_input, "list hunts") == 0) {
            scrie_comanda("list_hunts");
	    kill(monitor_pid, SIGUSR1);
	    sleep(1);
        }

        else if (strcmp(comanda_input, "stop monitor") == 0) {
            if (monitor_pornit && !monitor_inchis) {
                scrie_comanda("stop_monitor");
                kill(monitor_pid, SIGUSR1);
		sleep(1);
                monitor_inchis = 1;
            } else {
	       char *mesaj="!MONITORUL E DEJA OPRIT!\n";
	       write(1,mesaj,strlen(mesaj));
            }
        }

        else if(strcmp(comanda_input,"exit")!=0) {
	  char *mesaj="!COMANDA NCUNOSCUTA.\n";
	  write(1,mesaj,strlen(mesaj));
        }

	
    }
    
    char *mesaj="A-ti iesit din treasure_hub.\n";
    write(1,mesaj,strlen(mesaj));
    return 0;
}
