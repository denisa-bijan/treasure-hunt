/*
compilare: gcc -Wall -o hunt proiect1.c
executie: ./hunt treasure_hunt (functie) (hunt) [id]
adaugare: ./hunt treasure_hunt add hunt1
list: ./hunt treasure_hunt list hunt1
view: ./hunt treasure_hunt view hunt1 1
stergere hunt: ./hunt treasure_hunt remove_hunt hunt1
stergere comoara: ./hunt treasure_hunt remove_treasure hunt1 1
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


typedef struct {
    int id;
    char name[40];
    float lant, lont;
    char clue[512];
    int value;
} COMOARA;

int add(COMOARA c, char *hunt_path) {
  
    char comoara_path[60];
    char buf[20];
    int cnt=0;
    int x=c.id; //facem id-ul din int in char
    while(x){
      buf[cnt++]=(x%10)+'0';
      x=x/10;
    }
    buf[cnt]='\0';
    
    strcpy(comoara_path, hunt_path);
    strcat(comoara_path, "/comoara");
    strcat(comoara_path,buf); //adaugam in titlul comenzii id-ul sau
    strcat(comoara_path, ".bin");
   

    int fd = open(comoara_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
      char *eroare="eroare la deschiderea comorii\n";
      write(1, eroare,strlen(eroare));
        return 1;
    }

    ssize_t size = write(fd, &c, sizeof(COMOARA));
    close(fd);

    if (size != sizeof(COMOARA)) {
      char *eroare= "eroare la scrierea in comoara\n";
       write(1, eroare,strlen(eroare));
        return 1;
    }

    // scriere in logg
    char logg_path[100];
    strcpy(logg_path, hunt_path);
    strcat(logg_path, "/logged_hunt");

    int log_fd = open(logg_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd != -1) {
        char buffer[256];
        int len = snprintf(buffer, sizeof(buffer), "ID: %d | NUME: %s | GPS: %.2f, %.2f | INDICIU: %s | VALOARE: %d\n", c.id, c.name, c.lant, c.lont, c.clue, c.value);  //copiem informatiile primite de la tastatura si le punem frumos in fisierul logg
        write(log_fd, buffer, len);
        close(log_fd);
    }

    return 0;
}

void list(char *hunt_path) {
    DIR *hunt_dir = opendir(hunt_path);
    if (!hunt_dir) {
      char *eroare="nu s-a putut deschide directorul dat\n";
      write(1,eroare,strlen(eroare));
        exit(1);
    }

    struct dirent *file;
    while ((file = readdir(hunt_dir)) != NULL) {
        if (file->d_type == DT_REG && strstr(file->d_name, "comoara") != NULL) {
            char file_path[60];
            strcpy(file_path, hunt_path);
            strcat(file_path, "/");
            strcat(file_path, file->d_name);

            struct stat st;
            if (stat(file_path, &st) == -1) {
                write(1, "eroare la stat\n", 15);
                exit(1);
            }

            printf("fisier: %s\n", file->d_name);
            printf("dimensiune: %ld bytes\n", st.st_size);
            printf("ultima modificare: %s", ctime(&st.st_mtime));
	    //nu stiu cum sa le printez pe astea cu write si cu tot cu %s,%ld,%s
	    
            int fd = open(file_path, O_RDONLY);
            if (fd == -1) {
	      char *eroare="eroare la deschiderea comorii\n";
	      write(1, eroare,strlen(eroare));
                exit(1);
            }

            COMOARA c;
            while (read(fd, &c, sizeof(COMOARA)) == sizeof(COMOARA)) {
	      printf("--> ID: %d | USER: %s | GPS: %.2f, %.2f | CLUE: %s | VALUE: %d\n", c.id, c.name, c.lant, c.lont, c.clue, c.value); //nu stiu cum sa le printez cu write
            }
            close(fd);
        }
    }
    closedir(hunt_dir);
}

void view(char *hunt_path, int id) {
    DIR *dir = opendir(hunt_path);
    if (!dir) {
      char *eroare= "eroare la deschiderea directorului hunt\n";
      write(1, eroare,strlen(eroare));
        exit(1);
    }
    struct dirent *comoara;
    char comoara_path[60];
    int exista = 0;

    while ((comoara = readdir(dir)) != NULL && exista == 0) {
        if (comoara->d_type == DT_REG && strstr(comoara->d_name, "comoara") != NULL) {
            strcpy(comoara_path, hunt_path);
            strcat(comoara_path, "/");
            strcat(comoara_path, comoara->d_name);

            int fd = open(comoara_path, O_RDONLY);
            if (fd == -1) {
	      char *eroare="eroare la deschiderea comorii\n";
	      write(1,eroare, strlen(eroare));
              exit(1);
            }
            COMOARA c;
            while (read(fd, &c, sizeof(COMOARA)) == sizeof(COMOARA) && exista == 0) {
                if (c.id == id) {
                    printf("Comoara gasita:\n");
                    printf("  ID: %d\n", c.id);
                    printf("  USER: %s\n", c.name);
                    printf("  GPS: %.2f - %.2f\n", c.lant, c.lont);
                    printf("  CLUE: %s\n", c.clue);
                    printf("  VALUE: %d\n", c.value);
                    exista = 1;
                }
            }
            close(fd);
        }
    }
    closedir(dir);
    if (exista == 0)
        printf("Comoara cu id-ul: %d nu a fost gasita.\n", id);
}

void remove_hunt(char *hunt_path) {
    DIR *dir = opendir(hunt_path);
    if (!dir) {
      char *eroare="eroare la deschiderea directorului pentru stergere\n";
      write(1,eroare,strlen(eroare));
      exit(1);
    }

    struct dirent *file;
    char file_path[256];

    while ((file = readdir(dir)) != NULL) {
      if (strcmp(file->d_name, ".") != 0 &&  strcmp(file->d_name, "..") != 0) {
        strcpy(file_path,hunt_path);
	strcat(file_path,"/");
	strcat(file_path,file->d_name);
        unlink(file_path); //sterge tot ce e in director pe rand
      }
    }

    closedir(dir);
    rmdir(hunt_path);//stergem directorul in sine

    // stergem si symlink-ul daca exista
    char *hunt_id = strrchr(hunt_path, '/'); //pointeaza spre locul unde gaseste / in hunt path
    if (hunt_id!=NULL)
      hunt_id++; //daca gaseste / atunci creste ++ ca sa pointeze spre numele directorului pe care vrem sa il stergem 
    else
      hunt_id = hunt_path; //daca nu gaseste / inseamna ca vrem sa stergem directorul parinte

    char link_path[100];
    strcpy(link_path,"logged_hunt-");
    strcat(link_path,hunt_id);
    unlink(link_path); //stergem si link-ul directorului
}

int facem_symlink(char *hunt_path,char *hunt_name) {
    char link_name[100];
    strcpy(link_name,"logged_hunt-");
    strcat(link_name,hunt_name);
    strcat(link_name,".txt");
    
    char link_path[100];
    strcpy(link_path,hunt_path);
    strcat(link_path,"/logged_hunt"); // clea spre logg ul din interiorul hunt ului
    struct stat st;
    if(lstat(link_name,&st)==0)
      return 1;
    else
     symlink(link_path, link_name); //legam logg ul din hunt cu logg-ul din directorul treasure_hunt
    return 0;
    
}

void remove_treasure(const char *hunt_path, int id) {
    DIR *dir = opendir(hunt_path);
    if (!dir) {
      char *eroare="nu s-a putut deschide directorul.\n";
      write(1,eroare,strlen(eroare));
      exit(1);
    }
    int id_reverse=0;
    int aux=id;
    while(aux)
      {
	id_reverse=id_reverse*10+aux%10;
	aux=aux/10;
      }
    struct dirent *comoara;
    char comoara_path[256];

    while ((comoara = readdir(dir)) != NULL) {
                if (comoara->d_type == DT_REG && strstr(comoara->d_name, "comoara") != NULL) {
            strcpy(comoara_path, hunt_path);
            strcat(comoara_path, "/");
            strcat(comoara_path, comoara->d_name);

            int fd = open(comoara_path, O_RDONLY);
            if (fd == -1) continue;

            COMOARA treasure[100];
            int count = 0;
            COMOARA c;
            while (read(fd, &c, sizeof(COMOARA)) == sizeof(COMOARA)) {
                if (c.id != id) {
                    treasure[count++] = c;  // păstrăm doar comorile care nu trebuie șterse
                }
            }
            close(fd);

            if (count == 0) {
                // Nu mai e nicio comoară → ștergem fișierul
                unlink(comoara_path);
            } else {
                // Rescriem fișierul cu comorile rămase
                fd = open(comoara_path, O_WRONLY | O_TRUNC);
                if (fd == -1) continue;
                for (int i = 0; i < count; i++) {
                    write(fd, &treasure[i], sizeof(COMOARA));
                }
                close(fd);
	    }
            }
	       
        }


    closedir(dir);
      

    // eliminare linie din logged_hunt
   char log_path[256];
    strcpy(log_path, hunt_path);
    strcat(log_path, "/logged_hunt");

    int log_read_fd = open(log_path, O_RDONLY);
    if (log_read_fd == -1) {
        char *eroare = "Eroare la deschiderea logged_hunt pentru citire.\n";
        write(1, eroare, strlen(eroare));
        return;
    }

    int aux_fd = open("aux_log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (aux_fd == -1) {
        char *eroare = "Eroare la crearea aux_log.txt.\n";
        write(1, eroare, strlen(eroare));
        close(log_read_fd);
        return;
    }

    char line[1024];
    int idx = 0;
    char c;
    char target[20];
    sprintf(target, "ID: %d", id);

    while (read(log_read_fd, &c, 1) == 1) {
        if (c != '\n') {
            line[idx++] = c;
        } else {
            line[idx] = '\0';
            if (strstr(line, target) == NULL) {
                write(aux_fd, line, strlen(line));
                write(aux_fd, "\n", 1);
            }
            idx = 0;
        }
    }

    close(log_read_fd);
    close(aux_fd);

    unlink(log_path);//stergem fisierul logg vechi
    rename("aux_log.txt", log_path);//fisierul log temporal va deveni cel principal fiind redenumit cu calea corecta
	    
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
      char *eroare="Nu ati introdus un nr corect de argumente(verificati cum trebuie rulat programul.\n";
      write(1,eroare,strlen(eroare));
        return 1;
    }

    char buffer[128];

    char hunt_path[256];
    strcpy(hunt_path,argv[1]);
    strcat(hunt_path,"/");
    strcat(hunt_path,argv[3]);

    struct stat st; //verificam daca directorul hunt dat exista
    if(stat(hunt_path,&st)!=0 || !S_ISDIR(st.st_mode)){
      char *eroare="Cel putin unul dintre directoarele date nu exista.\n";
      write(1,eroare,strlen(eroare));
      return 1;
    }

    if (strcmp(argv[2], "add") == 0) {
        COMOARA c;
	char *ID_comoara = "ID comoara: ";
	write(1, ID_comoara, strlen(ID_comoara));
	memset(buffer, 0, sizeof(buffer));
	read(0, buffer, sizeof(buffer));
	c.id = atoi(buffer); 

	char *nume = "Nume utilizator: ";
	write(1, nume, strlen(nume));
	memset(c.name, 0, sizeof(c.name));
	read(0, c.name, sizeof(c.name));
	c.name[strlen(c.name)-1]='\0';

	char *lant = "Latitudine GPS: ";
	write(1, lant, strlen(lant));
	memset(buffer, 0, sizeof(buffer));
	read(0, buffer, sizeof(buffer));
	c.lant = atof(buffer); 

	char *lont = "Longitudine GPS: ";
	write(1, lont, strlen(lont));
	memset(buffer, 0, sizeof(buffer));
	read(0, buffer, sizeof(buffer));
	c.lont = atof(buffer);

	char *indiciu = "Indiciu: ";
	write(1, indiciu, strlen(indiciu));
	memset(c.clue, 0, sizeof(c.clue));
	read(0, c.clue, sizeof(c.clue));
	c.clue[strlen(c.clue)-1]='\0';
	
	char *valoare = "Valoare comoara: ";
	write(1, valoare, strlen(valoare));
	memset(buffer, 0, sizeof(buffer));
	read(0, buffer, sizeof(buffer));
	c.value = atoi(buffer);

        add(c, hunt_path);

	char *hunt_name=strrchr(hunt_path,'/');
	if(hunt_name!=NULL){
	  hunt_name++;
	}
	else
	  hunt_name=argv[3];

	facem_symlink(hunt_path,hunt_name);
    } else if (strcmp(argv[2], "list") == 0) {
        list(hunt_path);
    } else if (strcmp(argv[2], "view") == 0 && argc == 5) {
        int id = atoi(argv[4]);
        view(hunt_path, id);
    } else if (strcmp(argv[2], "remove_treasure") == 0 && argc == 5) {
        int id = atoi(argv[4]);
        remove_treasure(hunt_path, id);
       	char *mesaj="S-a efectuat stergerea comori.\n";
	write(1,mesaj,strlen(mesaj));
    } else if (strcmp(argv[2], "remove_hunt") == 0) {
        remove_hunt(hunt_path);
	char *mesaj="S-a efectuat stergerea hunt-ului.\n";
	write(1,mesaj,strlen(mesaj));
    } else {
      char *eroare="Comanda necunoscuta sau parametri lipsa.\n";
      write(1,eroare,strlen(eroare));
    }

    return 0;
}
