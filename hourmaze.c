/*
Autores:
    Alejandro Budiño Regueira   
    Manuel Couto Pintos
*/
#include <sys/stat.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>


#define H 12

FILE *get_problem_head(char *filename, int *filas, int *columnas)
{
    FILE *fp;
    char buffer[100];
    
    // Open and Read FILE
    printf("* %s -----------------------------------------------------\n\n",filename);
    fp = fopen(filename,"r");

    if (fp != NULL) 
    {
        printf(" * PROBLEM HEAD:\n");
        
        fgets(buffer, 100, fp);
        (*columnas) = atoi(buffer);
        printf("\tcolumnas -> %d \n", (*columnas));

        fgets(buffer, 100, fp);
        (*filas) = atoi(buffer);
        printf("\tfilas -> %d \n", (*filas));

    }
    printf("\n");

    return fp;

}

char **process_problem(FILE *fp, FILE *file, int filas, int columnas)
{
    char **matrix = (char**)malloc(filas * 2 * sizeof(char *));

    for(int i=0; i<(filas*2); i++)
        matrix[i] = malloc(columnas * 2 * sizeof(char));

    for(int i=0; i<(filas*2); i++)
        for(int j=0; j<(columnas*2); j++){
            matrix[i][j]='\0';
        }

    if (fp != NULL) {
        int i, j, n;
        char buffer[100];
        char buffer2[100];

        fprintf(file, "%%Graph \n");
        fprintf(file, "#const h = %d.\n",H);
        fprintf(file, "#const r = %d.\n",filas);
        fprintf(file, "#const c = %d.\n",columnas);

        printf(" * IMPUT MAP:\n");

        i=0;
        while (fgets(buffer, sizeof buffer, fp) != NULL) {

            if(i<(filas-1)){
                fgets(buffer2, sizeof buffer2, fp);
            } else {
                buffer2[0]='\0';
            }

            for(j=0; buffer[j]!='\0'; j++){
                
                matrix[i*2][j] = buffer[j];
                if(i<(filas-1))
                    matrix[(i*2)+1][j] = buffer2[j];

                if((j%2)==0){
                    if(isxdigit(buffer[j])){
                        n = strtol((char[]){buffer[j], 0}, NULL, 16);
                        // Regla set(cell(x,y), houre(n))
                        fprintf(file,"set(cell(%d,%d), hour(%d)).\n", i+1,j/2+1, n);
                    } 
                    if(i<(filas-1) && buffer2[j]=='-'){
                        //Reglas wall(cell(i,j/2),cell(i+1,j/2));
                        fprintf(file,"wall(cell(%d,%d),cell(%d,%d)).\n",i+1, j/2+1, i+2, j/2+1);
                    } else {
                        //fprintf(file,"-wall(cell(%d,%d),cell(%d,%d)).\n",i+1, j/2+1, i+2, j/2+1);
                    }
                    if((j/2)<(columnas-1) && buffer[j+1]=='|'){
                        //Reglas wall(cell(i,j/2),cell(i,j/2 + 1));
                        fprintf(file,"wall(cell(%d,%d),cell(%d,%d)).\n",i+1,j/2+1, i+1, j/2 + 2);
                    }else {
                        //fprintf(file,"-wall(cell(%d,%d),cell(%d,%d)).\n",i+1,j/2+1, i+1, j/2 + 2);
                    }
                }
            }

            printf("\t%s", buffer);
            printf("\t%s", buffer2);
            i++;
        }

        printf("\n\n");
    }

    return matrix;
}

int StartsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}



char **read_result(char *fileName, int filas, int columnas)
{
    int i;
    char **board;
    int fila, columna, hora;

    FILE *file;
    char * line = NULL;
    char * token = NULL;
    size_t len = 0;
    ssize_t read;

    char *tokens [filas*columnas];

    //Board
    board = (char**) malloc((filas) * sizeof(char *));
    for(i=0; i<filas; i++)
        board[i] = malloc(columnas * sizeof(char));

    //Reading file
    if ((file = fopen("result.txt","r")) == NULL)
        exit(EXIT_FAILURE);

    int count = 0;
    char aux[4];
    while ((read = getline(&line, &len, file)) != -1) {
        if(StartsWith(line,"set")){
            token = strtok(line, "(");
            while( token != NULL ) {
                token = strtok(NULL, " ");
                tokens[count++] = token;
                token = strtok(NULL, "(");
            }
            for(i=0; i<count; i++){
                strtok(tokens[i], "(");
                fila = atoi(strtok(NULL, ","));
                columna =atoi( strtok(NULL, ")"));
                strtok(NULL, "(");
                hora = atoi(strtok(NULL, ")"));
                sprintf(&aux[0],"%X",(hora%H == 0)?H:hora%H);
                board[fila-1][columna-1] = aux[0];
            }
        }

        count = 0;
    }

    //Close and Free
    if (line)
        free(line);
    if(token)
        free(token);

    fclose(file);
    printf("\n");

    return board;
}

void print_result(int filas, int columnas, char **board, char **matrix)
{
    int i = 0, j = 0;
    char aux = '\0';

    printf("RESULT MAP:\n");
    for(i=0; i<filas; i++){
        printf("\t");
        for(j=0; j<columnas; j++){
            aux = matrix[(i*2)][(j*2)+1];
            printf("%c%c", board[i][j],((aux=='|')?'|':' '));
        }
        printf("\n\t");
        if((i+1)<filas){
            for(j=0; j<columnas; j++){
                aux = matrix[(i*2)+1][(j*2)];
                printf("%c ",aux);
            }
            printf("\n");
        }
    }
    printf("\n");
}

void hourmaze(char *filePath){
    char **matrix;
    char **board;
    FILE * file;
    FILE * fp;

    struct timespec ts1, ts2;
    
    int filas = 0;
    int columnas = 0;

    fp = get_problem_head(filePath, &filas, &columnas);
    file = fopen("pgraph.gph","w");
    matrix = process_problem(fp, file, filas, columnas);

    fclose(fp);
    fclose(file);
    // End Processing

    // Clasp work
    clock_gettime( CLOCK_REALTIME, &ts1 );
    system("clingo pgraph.gph maze.txt > result.txt");
    clock_gettime( CLOCK_REALTIME, &ts2 );

    printf(" * Tiempo de ejecucion(clasp): %f\n", 
        (float) ( 1.0*(1.0*ts2.tv_nsec - ts1.tv_nsec*1.0)*1e-9 + 1.0*ts2.tv_sec - 1.0*ts1.tv_sec )/60 );
    // End Clasp

    // Read result
    board = read_result("examples/dom01.txt", filas, columnas);
    print_result(filas, columnas, board, matrix);
    printf("\n");

    // Free dynamic memory
    for(int i=0; i<(filas*2); i++)
        free(matrix[i]);

    for(int i=0; i<filas; i++)
        free(board[i]);

    free(matrix);
    free(board);

   
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int main(int argc, char *argv[]) 
{
    int i;
    DIR *dir;
    struct dirent *ent;
    char filePath[100];

    for(i=1; i<argc; i++)
    {
        if(is_regular_file(argv[i]))
        {
            hourmaze(argv[i]);
        } else {
            dir = opendir(argv[i]);

            if(dir == NULL)
                perror("No puedo abrir el directorio");

            while ((ent = readdir (dir)) != NULL)
            {
                if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) )
                {
                    strcpy(filePath, argv[i]);
                    strcat(filePath, "/");
                    strcat(filePath, ent->d_name);
                    hourmaze(filePath);
                }
            }
        }
    }

    return 0;
}