// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <libc.h>

#define MAX_LINE_LENGTH 1024
#define MAX_COMMAND_LENGTH 20
#define MAX_RECIPE_NAME 255
#define MAX_INGREDIENT_NAME 255
#define MAX_INGREDIENTS 10
#define TABLE_SIZE 25

typedef struct {
    char name[MAX_INGREDIENT_NAME];
    int quantity;
} Ingredient_t;

typedef struct recipe{
    char name[MAX_RECIPE_NAME];
    Ingredient_t ingredients[20];
    int num_ingredients;
    struct recipe* next;
} Recipe_t;

typedef struct {
    Recipe_t recipe;
    int quantity;
} Order_t;

/*
    Funzione che ritorna un valore in base al tipo di informazione letta dal file 
    Values:
        -1 -> error
        0 -> aggiungi_ricetta
        1 -> rimuovi_ricetta
        2 -> ordine
        3 -> rifornimento
*/
int process_command(char *line) {
    char command[MAX_COMMAND_LENGTH];
    int code;

    code = sscanf(line, "%s", command);
    if(code != 1) {
        printf("Errore nella lettura del comando\n");
        return -1;
    }

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

// FUNZIONI PER LA HASH TABLE
Recipe_t* hash_table_recipe[TABLE_SIZE];
/*
    Funzione che crea l'indice della tabella di hash in cui andra' ad essere inserito il valore
    Returns:
        unsigned int = indice in cui verra' posizionato il valore nella tabella di hash
*/
unsigned int hash_function(char* name) {
    int length = strnlen(name, MAX_RECIPE_NAME);
    unsigned int hash_value = 0;
    for(int i = 0; i < length; i++) {
        hash_value += name[i];
        hash_value = hash_value % TABLE_SIZE;
        //    TODO:
        //        cercare eventuali metodi per rendere ancora più randomica la funzione di hash
        // hash_value = (hash_value * name[i]) % TABLE_SIZE;
    }
    return hash_value;
}

/*
    Funzione che inizializza la tabella di hash
*/
void init_hash_table_recipe() {
    for(int i=0; i < TABLE_SIZE; i++){
        hash_table_recipe[i] = NULL;
    }
}

/*
    Funzione che inizializza la funzione hash
    Returns:
        0 -> no errors
        1 -> errors
*/
//    TODO:
//        cercare eventualmente se la funzione di hashing con remapping è migliore rispetto a questa
int insert_hash_table_recipe(Recipe_t* recipe) {
    if(recipe == NULL) {
        return 1;
    }

    int index = hash_function(recipe->name);
    recipe->next = hash_table_recipe[index];
    hash_table_recipe[index] = recipe;
    return 0;
}

Recipe_t* search_hash_table_recipe(char* recipe_name) {
    int index = hash_function(recipe_name);
    Recipe_t* temp = hash_table_recipe[index];
    while(temp != NULL && strncmp(temp->name, recipe_name, MAX_RECIPE_NAME) != 0) {
        temp = temp->next;
    }
    return temp;
}

Recipe_t* delete_hash_table_recipe(char* recipe_name) {
    int index = hash_function(recipe_name);
    Recipe_t* temp = hash_table_recipe[index];
    Recipe_t* prev = NULL;
    while(temp != NULL && strncmp(temp->name, recipe_name, MAX_RECIPE_NAME) != 0) {
        prev = temp; 
        temp = temp->next;
    }
    // in fondo alla lista
    if(temp == NULL) {
        return NULL;
    }
    // eliminazione testa della lista
    if(prev == NULL){
        hash_table_recipe[index] = temp->next;
    } else{
        prev->next = temp->next;
    }
    return temp;
}

/*
    Funzione di debugging che stampa la tabella di hash
*/
void print_hash_table_recipe() {
    printf("START\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        printf("Index %d: ", i);
        Recipe_t* temp = hash_table_recipe[i];
        while (temp != NULL) {
            printf("%s -> ", temp->name);
            temp = temp->next;
        }
        printf("NULL\n");
    }
    printf("END\n");
    return;
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

    //    TODO:
    //        Vedere se 512 basta come dimensione massima lineea altrimenti incrementarla
    //        Inoltre provare a vedere se esistono metodi migliori senza usar le linee ma prendendo parola per parola
    //        -> soluzione possibile su GT
    
    init_hash_table_recipe();
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
        // Tokenizzazione del comando -> da fare per forza causa comando
        token = strtok(line, " ");
        switch(command_value) {
            case 0: {
                // aggiungi_ricetta
                Recipe_t *new_recipe = (Recipe_t*)malloc(sizeof(Recipe_t));
                if (new_recipe == NULL) {
                    printf("Errore di allocazione della memoria\n");
                    return 1;
                }

                int ingredient_counter = 0;

                // lettura ricetta + ingrediente + quantità OBBLIGATORI
                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Ricetta mancante\n\tErrore riscontrato al giorno %d\n", day);
                    free(new_recipe);
                    return 1;
                }
                strcpy(new_recipe->name, token);

                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Ingrediente mancante\n\tErrore riscontrato al giorno %d\n", day);
                    free(new_recipe);
                    return 1;
                }
                strcpy(new_recipe->ingredients[ingredient_counter].name, token);

                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Quantità ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", new_recipe->ingredients[ingredient_counter].name, day);
                    free(new_recipe);
                    return 1;
                }
                sscanf(token, "%d", &new_recipe->ingredients[ingredient_counter].quantity);

                // lettura ingrediente + quantità FACOLTATIVI
                ingredient_counter++;
                while(token != NULL){
                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        break;
                    }
                    strcpy(new_recipe->ingredients[ingredient_counter].name, token);

                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        printf("Quantità ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", new_recipe->ingredients[ingredient_counter].name, day);
                        free(new_recipe);
                        return 1;
                    }
                    sscanf(token, "%d", &new_recipe->ingredients[ingredient_counter].quantity);
                    ingredient_counter++;
                }
                
                new_recipe->num_ingredients = ingredient_counter;
                new_recipe->next = NULL;
                if (insert_hash_table_recipe(new_recipe) != 0) {
                    printf("Errore nell'inserimento della ricetta nella tabella hash\n");
                    free(new_recipe);
                    return 1;
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

                Recipe_t* recipe_to_delete = search_hash_table_recipe(recipe_name);
                if(recipe_to_delete == NULL) {
                    // non esistente

                    // TODO:
                    //      Implementare il caso ricetta non esistente
                } else{
                    delete_hash_table_recipe(recipe_to_delete->name);
                    free(recipe_to_delete);
                    printf("Ricetta eliminata con successo!\n");
                }

                break;
            }
            case 2: {
                // ordine
                Order_t *new_order = (Order_t*)malloc(sizeof(Order_t));
                
                // Lettura della ricetta + quantità da ordinare
                token = strtok(NULL, " ");
                if(token == NULL){
                    printf("Mancante ricetta da ordinare in file\n\tErrore riscontrato al giorno %d\n", day);
                    free(new_order);
                    return 1;
                }
                
                Recipe_t* recipe = search_hash_table_recipe(token);
                if(recipe == NULL) {
                    // ricetta non esiste
                    printf("Non esistente ricetta da ordinare\n\tErrore riscontrato al giorno %d\n", day);
                    free(new_order);
                    break;
                }
                
                // Copia della ricetta nell'ordine
                memcpy(&new_order->recipe, recipe, sizeof(Recipe_t));

                token = strtok(NULL, " ");
                if(token == NULL){
                    printf("Mancante quantità dell'ordine '%s'\n\tErrore riscontrato al giorno %d\n", recipe->name, day);
                    free(new_order);
                    return 1;
                }
                sscanf(token, "%d", &new_order->quantity);

                // Stampa dell'ordine
                printf("Ordine: %s - Quantità: %d\n", new_order->recipe.name, new_order->quantity);
                
                // ordine pronto
                // TODO:
                //      implementare ciò che accade una volta che l'ordine è pronto

                // Libera la memoria allocata per l'ordine
                free(new_order);
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

    print_hash_table_recipe();
    /*
        Chiusura file
    */
    fclose(file);
    return 0;
}
/*
rifornimento farina 100 15 farina 50 13 uova 45 20 zucchero 20 20 burro 15 20
*/
