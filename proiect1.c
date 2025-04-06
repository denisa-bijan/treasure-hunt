#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
      char *eroare="Nu ati introdus un nr corect de argumente.\n";
      write(1, eroare, strlen(eroare));
        return 1;
    }

    char pdir[30]; //directorul principal - treasure_hunt - dat ca parametru
    strcpy(pdir, argv[1]);

    DIR *dir = opendir(pdir); //deschidem direcotrul
    if (!dir) {
      char *eroare="Eroare la deschiderea directorului principal.\n";
      write(1,eroare,strlen(eroare));
      return 1;
    }

    struct dirent *huntdir;
    while ((huntdir = readdir(dir)) != NULL) { //intram in directoarele hunt1,hunt2, etc
        if (huntdir->d_type == DT_DIR &&
            strcmp(huntdir->d_name, ".") != 0 &&
            strcmp(huntdir->d_name, "..") != 0) { //verificam sa fie director si sa nu fie . sau .. pentru ca o sa mearga la infinit
	  
            char hunt_path[512];
            strcpy(hunt_path, pdir);
            strcat(hunt_path, "/");
            strcat(hunt_path, huntdir->d_name);

            DIR *hdir = opendir(hunt_path);
            if (hdir==NULL){
	      char *eroare="Eroare la deschiderea directorului hunt.\n";
	      write(1,eroare,strlen(eroare));
	    }

           
            char logg_path[512]; //creem o cale catre fisierul logg din interiorul directorului hunt
            strcpy(logg_path, hunt_path);
            strcat(logg_path, "/logg-");
            strcat(logg_path, huntdir->d_name);
            strcat(logg_path, ".txt");

            int logg_fd = open(logg_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);//deschidem fisierul logg-hunt(id) sau il creem pt a putea scrie in el, dandu i mode ul specific fisierelor txt 0644
	    
            if (logg_fd == -1) {
                write(1, "Eroare logg\n", 12);
                closedir(hdir);
            }


            struct dirent *comoara;
            while ((comoara = readdir(hdir)) != NULL) { //deschidem comorile
                if (comoara->d_type == DT_REG &&
                    strstr(comoara->d_name, "comoara") != NULL) { //verificam sa fie fisier normal si sa aiba comoara in titlu

		  char comoara_path[512]; //salvam path ul comorii
                    strcpy(comoara_path, hunt_path);
                    strcat(comoara_path, "/");
                    strcat(comoara_path, comoara->d_name);

                    int fd = open(comoara_path, O_RDONLY);//deschidem comoara
                    if (fd == -1){
		      char *eroare="Eroare la deschiderea comorii.\n";
		      write(1,eroare,strlen(eroare));
		    }

                    char linie[1024];
                    ssize_t size_linie;
                    while ((size_linie = read(fd, linie, sizeof(linie))) > 0) {
                        write(logg_fd, linie, size_linie);
                    }

                    close(fd);
                }
            }

            close(logg_fd);
            closedir(hdir);
        }
    }

    closedir(dir);
    return 0;
}

	

	
