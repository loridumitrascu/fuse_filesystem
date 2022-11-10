#define _XOPEN_SOURCE 700
#define FUSE_USE_VERSION 29
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fuse.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"

typedef struct Director
{
    char nume_director[20];
    char nume_fisiere[10][20];
    char continut_fisier[10][50];
} Director;

Director Directoare[10];

bool check_is_dir(const char *path)
{
    path++;
    for (int i = 0; i < 10; i++)
    {
        if (strcmp(Directoare[i].nume_director, path) == 0)
            return true;
    }
    return false;
}

int get_dir_index(const char *nume)
{
    for (int i = 0; i < 10; i++)
    {
        if (strcmp(Directoare[i].nume_director, nume) == 0)
            {
                return i;
            }
    }
    return -1;
}

int find_file_in_dir(int id, const char *nume)
{
    for (int i = 0; i < 10; i++)
    {
        // log_message("Cauta %s in director:%s\n",nume,Directoare[id].nume_fisiere[i]);
        if (strcmp(Directoare[id].nume_fisiere[i], nume) == 0)
        {
            // log_message("Index gasit: %d \n",i);
            return i;
        }
    }
    return -1;
}

bool check_is_file(const char *path) // aici era mai bine sa returnam ceva, ex:indicele directorului in care se afla fis
{
    char path_strtok[20];
    path++;
    strcpy(path_strtok, path);
    // log_message("Strtok:%s\n",path_strtok);
    char *token = strtok(path_strtok, "/");
    // verific existenta fisierului in root
    int rez = find_file_in_dir(0, token);
    if (rez >= 0)
        return true;

    char aux[20];
    strcpy(aux, token);
    token = strtok(NULL, "/");
    if (token == NULL)
        return false;
    // verific existenta fisierului in subdirectoare
    int index = get_dir_index(aux);
    rez = find_file_in_dir(index, token);
    // log_message("rezultat cautare:%d\n",rez);
    if (rez >= 0)
        return true;
    return false;
}

void *fs_init(struct fuse_conn_info *conn)
{
    // functie care initializeaza sistemul de fisiere
    // se aloca structurile
    // conn poate sa ofere cateva informatii suplimentar
    strcpy(Directoare[0].nume_director, "/");
    log_message("Initializat\n");
}

int fs_getattr(const char *path, struct stat *stbuf)
{
    // returneaza atributele fisierului.
    // pentru un anumit path, functia trebuie
    // sa seteze toate elementele din structura stat
    // daca un camp este lipsit de sens, trebuie setat
    // la valoarea 0

    //log_message("Se iau atributele fisierului %s\n", path);

    stbuf->st_gid = getgid();
    stbuf->st_uid = getuid();
    stbuf->st_atime = time(NULL);
    stbuf->st_mtime = time(NULL);

    // Initializare atribute root si directoare
    if ((strcmp(path, "/") == 0) || (check_is_dir(path) == 1))
    {
        //log_message("Root sau director gasit\n");
        stbuf->st_nlink = 2; // implicit asa e pentru directoare
        stbuf->st_mode = S_IFDIR | 0755;
    }
    // Initializare atribute pentru fisiere
    else if (check_is_file(path) == 1)
    {
        //log_message("Fisier gasit\n");
        stbuf->st_nlink = 1;
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_size = 1024;
    }
    else
    {
        // daca nu a fost nici printre directoare
        // nici printre fisiere, intoarce eroare de path gresit
        //log_message("Fisierul nu a fost gasit\n");
        return -ENOENT;
    }
    return 0;
}

int fs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    //log_message("Am intrat in readdir pentru directorul %s\n", path);
    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    if (strcmp(path, "/") == 0)
    {
        //log_message("Readdir in root...\n");
        for (int i = 1; i < 10; i++)
        {
            if (strcmp(Directoare[i].nume_director, "") != 0)
            {
                filler(buffer, Directoare[i].nume_director, NULL, 0);
            }
        }
        for (int i = 0; i < 10; i++)
        {
            if (strcmp(Directoare[0].nume_fisiere[i], "") != 0)
            {
                filler(buffer, Directoare[0].nume_fisiere[i], NULL, 0);
            }
        }
    }
    else
    {
        int dir_index_found = get_dir_index(++path);
        //log_message("Readdir la dir cu indexul %d...\n", dir_index_found);
        if (dir_index_found >= 0)
        {
            for (int i = 0; i < 10; i++)
            {
                if (strcmp(Directoare[dir_index_found].nume_fisiere[i], "") != 0)
                {
                    filler(buffer, Directoare[dir_index_found].nume_fisiere[i], NULL, 0);
                }
            }
        }
        else
        {
            // Intoarce -ENOENT daca nu gaseste path-ul
            return -ENOENT;
        }
    }
    return 0;
}

int fs_mkdir(const char *path, mode_t mode)
{
    char c = '/';
    path++;
    if (strchr(path, c) == NULL)
    {
        int i;
        for (i = 0; i < 10; i++)
            if (strcmp(Directoare[i].nume_director, "") == 0)
                break;
        if (i < 10)
        {
            log_message("Am adaugat %s printre directoarele lui root\n", path);
            strcpy(Directoare[i].nume_director, path);
        }
        // EROARE ca s-a umplut vectorul de directoare disponibile
        else
            return -ENOMEM;
    }
    return 0;
}

int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    char c = '/';
    if (strchr(++path, c) == NULL) // avem un fisier in root si doar il adaugam in Directoare[0]==Root
                                   // am pus +1 ca sa caute incepand de la path[1], adica nu ia in calcul primul '/'
    {
        int i;
        for (i = 0; i < 10; i++)
            if (strcmp(Directoare[0].nume_fisiere[i], "") == 0)
                break;
        if (i < 10)
        {
            log_message("Am adaugat %s printre fisierele lui root\n", path);
            strcpy(Directoare[0].nume_fisiere[i], path);
        }
        // EROARE ca s-a umplut vectorul de fisiere disponibile in root
        else
            return -ENOMEM;
    }
    else // avem forma /director/numefis
    {
        char path_tok[20];
        strcpy(path_tok, path);
        char *token = strtok(path_tok, "/");
        char check_dir[21];
        strcpy(check_dir,"/");
        strcat(check_dir,token);
        if (check_is_dir(check_dir) == true)
        {
            log_message("intra aici\n");
            int index = get_dir_index(token);
            log_message("index director:%s\n",index);
            token = strtok(NULL, "/");
            log_message("token:%s",token);
            int i;
            for (i = 0; i < 10; i++)
                if (strcmp(Directoare[index].nume_fisiere[i], "") == 0)
                {
                    break;
                }
            if (i < 10)
            {
                log_message("Am adaugat %s printre fisierele directorului %d\n", token, index);
                strcpy(Directoare[index].nume_fisiere[i], token);
            }
            // EROARE ca s-a umplut vectorul de fisiere disponibile
            else
                return -ENOMEM;
        }
        else
        {
            return -ENOENT;
        }
    }
    return 0;
}

int fs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
    // verificat dimensiune buffer; daca nu exista fisier, cred ca intorc eroare
    log_message("Am intrat sa scriu ceva intr-un fisier\n");
    if(size*sizeof(size_t)>50*sizeof(int))
    {
        //Eroare:dimensiune prea mare.
        log_message("Nu s-a copiat nimic din buffer, dimensiunea sa era prea mare\n");
        return 0;
    }

    path++;
    // verificare fisier in root
    for (int i = 0; i < 10; i++)
    {
        if (strcmp(Directoare[0].nume_fisiere[i], path) == 0)
        {
            log_message("Copiaza %s in fisier %s la index %d\n", buffer, Directoare[0].nume_fisiere[i], i);
            strcpy(Directoare[0].continut_fisier[i], buffer);
            return size;
        }
    }

    // verific existenta fisierului in subdirectoare
    char path_strtok[20];
    strcpy(path_strtok, path);
    char *token = strtok(path_strtok, "/");
    char *nume_director = strdup(token);
    log_message("Nume director:%s\n",nume_director);
    int dir_position_found = get_dir_index(nume_director);
    if(dir_position_found<0)
    {
        log_message("Directorul %s nu a fost gasit\n",nume_director);
        return 0;
    }
    log_message("Index director:%s\n",dir_position_found);
    free(nume_director);
    token = strtok(NULL, "/");
    log_message("Nume fisier:%s\n",token);
    if (dir_position_found > 0)
    {
        for (int i = 0; i < 10; i++)
        {
            if (strcmp(Directoare[dir_position_found].nume_fisiere[i], token) == 0)
            {
                log_message("Copiaza %s in fisier %s la index %d\n", buffer, Directoare[dir_position_found].nume_fisiere[i], i);
                strcpy(Directoare[0].continut_fisier[i], buffer);
                return size;
            }
        }
    }
    return 0;
}

int fs_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
    path++;
    // verificare fisier in root
    for (int i = 0; i < 10; i++)
    {
        if (strcmp(Directoare[0].nume_fisiere[i], path) == 0)
        {
            char * continut = Directoare[0].continut_fisier[i];
            memcpy(buffer, continut + offset,size);
            return strlen(continut)-offset;
        }
    }
    return -1;
}

int fs_truncate(const char * path, off_t offset, struct fuse_file_info *fi)
{
    if(offset<0)
    {
        return -EINVAL;
    }
    if(check_is_file(path)==false)
    {
        return -ENONET;
    }
    path++;
    // verificare fisier in root
    for (int i = 0; i < 10; i++)
    {
        if (strcmp(Directoare[0].nume_fisiere[i], path) == 0)
        {
            log_message("Sterge continut fisier %s\n",Directoare[0].nume_fisiere[i]);
            memset(Directoare[0].continut_fisier[i],0,50);
            return 0;
        }
    }

     // verific existenta fisierului in subdirectoare
    char path_strtok[20];
    strcpy(path_strtok, path);
    char *token = strtok(path_strtok, "/");
    char *nume_director = strdup(token);
    log_message("Nume director:%s\n",nume_director);
    int dir_position_found = get_dir_index(nume_director);
    if(dir_position_found<0)
    {
        log_message("Directorul %s nu a fost gasit\n",nume_director);
        return -1;
    }
    log_message("Index director:%s\n",dir_position_found);
    free(nume_director);
    token = strtok(NULL, "/");
    log_message("Nume fisier:%s\n",token);
    if (dir_position_found > 0)
    {
        for (int i = 0; i < 10; i++)
        {
            if (strcmp(Directoare[dir_position_found].nume_fisiere[i], token) == 0)
            {
                log_message("Sterge continut fisier %s\n",Directoare[dir_position_found].nume_fisiere[i]);
                memset(Directoare[dir_position_found].continut_fisier[i],0,50);
                return 0;
            }
        }
    }
    return 0;
}

int fs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
    dev_t dev_whatever = 0;
    return fs_mknod(path, mode, dev_whatever);
    return 0;
}

int fs_utimens(const char *path, const struct timespec times[2])
{
    return 0;
}

static struct fuse_operations operations =
    {
        .getattr = fs_getattr,
        .readdir = fs_readdir,
        .mkdir = fs_mkdir,
        .mknod = fs_mknod,
        .write = fs_write,
        .read = fs_read,
        .create = fs_create, // trebuie pusa ca sa mearga touch
        .utimens = fs_utimens, // trebuie pusa ca sa mearga touch
        .init = fs_init,
        .truncate = fs_truncate, // necesar pentru a putea redirecta outputul in fisier. Si in general pentru a sterge continutul unui fisier
};

int main(int argc, char *argv[])
{
    FILE *fis = log_open();
    fuse_main(argc, argv, &operations, NULL);
    log_close(fis);
    return 0;
}
