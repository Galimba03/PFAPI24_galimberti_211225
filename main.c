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
    int order_quantity;
    int ingredient_expiring;
    int command_value;
    int day;
    while (fgets(line, sizeof(line), file)) {
        // Sostituzione carattere '\n' con terminatore stringa '\0' -> agevole per tokenizzazione
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        command_value = process_command(line);
        if(command_value == -1) {
            fclose(file);
            return 1;
        }
        // Tokenizzazione del comando -> da fare per forza
        token = strtok(line, " ");
        switch(command_value) {
            case 0: {
                // aggiungi_ricetta
                // lettura ricetta + ingrediente + quantità OBBLIGATORI
                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Ricetta mancante\n\tErrore riscontrato al giorno %d\n", day);
                    return 1;
                }
                strcpy(recipe_name, token);
                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Ingrediente mancante\n\tErrore riscontrato al giorno %d\n", day);
                    return 1;
                }
                strcpy(ingredient_name, token);

                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Quantità ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", ingredient_name, day);
                    return 1;
                }
                sscanf(token, "%d", &ingredient_quantity);

                // lettura ricetta + ingrediente + quantità FACOLTATIVI
                while(token != NULL){
                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        break;
                    }
                    strcpy(ingredient_name, token);

                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        printf("Quantità ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", ingredient_name, day);
                        return 1;
                    }
                    sscanf(token, "%d", &ingredient_quantity);
                }

                break;
            }
            case 1: {
                // rimuovi_ricetta
                // Lettura della ricetta da rimuovere
                token = strtok(NULL, " ");
                if(token == NULL){
                    printf("Mancante ricetta da rimuovere\n\tErrore riscontrato al giorno %d\n", day);
                    return 1;
                }
                strcpy(recipe_name, token);

                break;
            }
            case 2: {
                // ordine
                // Lettura della ricetta + quantità da ordinare
                token = strtok(NULL, " ");
                if(token == NULL){
                    printf("Mancante ricetta da ordinare\n\tErrore riscontrato al giorno %d\n", day);
                    return 1;
                }
                strcpy(recipe_name, token);
                token = strtok(NULL, " ");
                if(token == NULL){
                    printf("Mancante quantità della ricetta '%s'\n\tErrore riscontrato al giorno %d\n", recipe_name, day);
                    return 1;
                }
                sscanf(token, "%d", &order_quantity);

                break;
            }
            case 3: {
                // rifornimento
                // Lettura ingrediente + quantità + scadenza OBBLIGATORIE
                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Ingrediente da rifornire mancante\n\tErrore riscontrato al giorno %d\n", day);
                    return 1;
                }
                strcpy(ingredient_name, token);
                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Quantità rifornita di '%s' mancante\n\tErrore riscontrato al giorno %d\n", ingredient_name, day);
                    return 1;
                }
                sscanf(token, "%d", &ingredient_quantity);
                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Scadenza ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", ingredient_name, day);
                    return 1;
                }
                sscanf(token, "%d", &ingredient_expiring);
                printf("%s: %d - %d\n", ingredient_name, ingredient_quantity, ingredient_expiring);

                // Lettura ingrediente + quantità + scadenza FACOLTATIVE
                while(token != NULL){
                    token = strtok(NULL, " ");
                    if(token == NULL){
                        break;
                    }
                    strcpy(ingredient_name, token);
                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        printf("Quantità rifornita di '%s' mancante\n\tErrore riscontrato al giorno %d\n", ingredient_name, day);
                        return 1;
                    }
                    sscanf(token, "%d", &ingredient_quantity);
                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        printf("Scadenza ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", ingredient_name, day);
                        return 1;
                    }
                    sscanf(token, "%d", &ingredient_expiring);
                    printf("%s: %d - %d\n", ingredient_name, ingredient_quantity, ingredient_expiring);
                }
                
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
