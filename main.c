// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <libc.h>

#define MAX_LINE_LENGTH 512
#define MAX_COMMAND_LENGTH 20
#define MAX_RECIPE_NAME 50
#define MAX_INGREDIENT_NAME 50
#define MAX_INGREDIENTS 10

typedef struct {
    char ingredient[MAX_INGREDIENT_NAME];
    int quantity;
} Ingredient;

typedef struct {
    char name[MAX_RECIPE_NAME];
    Ingredient ingredients[20];
    int num_ingredients;
} Recipe;

/**
 *  Funzione che ritorna un valore in base al tipo di informazione letta dal file 
 *  Values:
 *          -1 -> error
 *          0 -> aggiungi_ricetta
 *          1 -> rimuovi_ricetta
 *          2 -> ordine
 *          3 -> rifornimento
 */
int process_command(char *line) {
    char command[MAX_COMMAND_LENGTH];
    int code;

    code = sscanf(line, "%s", command);
    if(code != 1) {
        printf("Errore nella lettura del comando\n");
        return -1;
    }
    // printf("%s\n", command);

    if(strcmp(command, "aggiungi_ricetta") == 0) {
        return 0;
    } else if (strcmp(command, "rimuovi_ricetta") == 0) {
        return 1;
    } else if (strcmp(command, "ordine") == 0){
        return 2;
    } else if (strcmp(command, "rifornimento") == 0) {
        return 3;
    }

    return -1; 
}

int main() {
    /*
        Apertura file in modalità lettura
    */
    FILE *file = fopen("inputTest.txt", "r"); 
    if (file == NULL){
        printf("Errore apertura file\n");
        return 1;
    }

    /*
        Lettura arrival (arrivo ogni...) e space (capacià)
    */
    int arrival, space;
    char extra;
    if(fscanf(file, "%d %d", &arrival, &space) != 2) {
        printf("Errore input file: mancanti o periodicita\' o capienza\n");
        fclose(file);
        return 1;
    } else {
        // Consumo resto della linea
        while((extra = fgetc(file) != '\n') && extra != EOF);
    }
    // printf("%d - %d\n", arrival, space);

    /*
        Lettura riga per riga del file | Lettura delle operazioni da fare di giorno in giorno
    */
    char line[MAX_LINE_LENGTH];
    char *token;
    char recipe_name[MAX_RECIPE_NAME];
    char ingredient_name[MAX_INGREDIENT_NAME];
    int ingredient_quantity;
    int command_value;
    int day;
    while (fgets(line, sizeof(line), file)) {
        command_value = process_command(line);
        if(command_value == -1) {
            fclose(file);
            return 1;
        }
        switch(command_value){
            case 0: {
                // aggiungi_ricetta
                // printf("%d | aggiungi_ricetta -> ", day);
                token = strtok(line, " ");
                token = strtok(NULL, " ");
                sscanf(token, "%s", recipe_name);
                printf("Ricetta: %s\n\t", recipe_name);
                while(token != NULL){
                    token = strtok(NULL, " ");
                    if(token == NULL){
                        break;
                    }
                    strcpy(ingredient_name, token);
                    printf("%s + ", ingredient_name);

                    token = strtok(NULL, " ");
                    if(token == NULL){
                        break;
                    }
                    sscanf(token, "%d", &ingredient_quantity);
                    printf("%d | ", ingredient_quantity);
                }
                printf("\n");
                break;
            }
            case 1: {
                // rimuovi_ricetta
                printf("%d | rimuovi_ricetta\n", day);
                break;
            }
            case 2: {
                // ordine
                printf("%d | ordine\n", day);
                break;
            }
            case 3: {
                // rifornimento
                printf("%d | rifornimento\n", day);
                break;
            }
        }
        day++;
    }

    /*
        Chiusura file
    */
    fclose(file);
    return 0;
}
