// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <libc.h>

#define MAX_LINE_LENGTH 512
#define MAX_COMMAND_LENGTH 20
#define MAX_RECIPE_NAME 50
#define MAX_INGREDIENT_LENGTH 50
#define MAX_INGREDIENTS 10

typedef struct {
    char ingredient[MAX_INGREDIENT_LENGTH];
    int quantity;
}Ingredient;

typedef struct {
    char name[MAX_RECIPE_NAME];
    Ingredient ingredients[20];
    int num_ingredients;
}Recipe;


int process_line(char *line){
    char command[MAX_COMMAND_LENGTH];
    int code;

    code = sscanf(line, "%s", command);
    if(code != 1){
        printf("Errore nella lettura del comando\n");
        return 1;
    }

    printf("%s\n", command);
    return 0;
    
}

int main() {
    // Apertura file in modalità lettura
    FILE *file = fopen("inputTest.txt", "r"); 
    if (file == NULL){
        printf("Errore apertura file\n");
        return 1;
    }

    // Lettura arrival (arrivo ogni...) e space (capacià)
    int arrival, space;
    char extra;
    if(fscanf(file, "%d %d", &arrival, &space) != 2){
        printf("Errore input file: mancanti o periodicita\' o capienza\n");
        fclose(file);
        return 1;
    }else{
        // Consumo resto della linea
        while((extra = fgetc(file) != '\n') && extra != EOF);
    }
    // printf("%d - %d\n", arrival, space);

    char line[MAX_LINE_LENGTH];
    int value;

    while (fgets(line, sizeof(line), file)) {
        value = process_line(line);
        if(value == 1){
            fclose(file);
            return 1;
        }
    }

    // Chiusura file
    fclose(file);
    return 0;
}
