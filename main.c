#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 32768
#define MAX_COMMAND_LENGTH 20
#define MAX_RECIPE_NAME 255
#define MAX_INGREDIENT_NAME 255
#define MAX_INGREDIENTS 100
#define RP_TABLE_SIZE 20
#define WH_TABLE_SIZE 20

typedef struct {
    char name[MAX_INGREDIENT_NAME];
    int quantity;
} Ingredient_recepie_t;

typedef struct recipe{
    char name[MAX_RECIPE_NAME];
    //  TODO:
    //      fare una lista di ingredienti
    Ingredient_recepie_t ingredients[MAX_INGREDIENTS];
    int num_ingredients;
    int weight;
    struct recipe* next;
} Recipe_t;

typedef struct {
    Recipe_t recipe;
    int quantity;
    int arrival;
} Order_t;

typedef struct expiring {
    int quantity;
    int expire;
    struct expiring* next;
} Expiring_t;

typedef struct ingredient_warehouse {
    char name[MAX_INGREDIENT_NAME];
    Expiring_t* head;
    struct ingredient_warehouse* next;
} Ingredient_warehouse_t;

/*
    Funzione che ritorna un valore in base al tipo di informazione letta dallo standard input stdin 
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

// FUNZIONI PER LA HASH TABLE DELLE RICETTE
Recipe_t* hash_table_recipe[RP_TABLE_SIZE];
/*
    Funzione che crea l'indice della tabella di hash in cui andra' ad essere inserito il valore
    Returns:
        unsigned int = indice in cui verra' posizionato il valore nella tabella di hash
*/
unsigned int hash_function(char* name, int hash_table_dim) {
    int length = strnlen(name, MAX_RECIPE_NAME);
    unsigned int hash_value = 0;
    for(int i = 0; i < length; i++) {
        hash_value += name[i];
        hash_value = (hash_value * name[i]) % hash_table_dim;
    }
    return hash_value;
}

/*
    Funzione che inizializza la tabella di hash
*/
void init_hash_table_recipe() {
    for(int i=0; i < RP_TABLE_SIZE; i++){
        hash_table_recipe[i] = NULL;
    }
}

/*
    Funzione che inizializza la funzione hash
    Returns:
        false -> no errors
        true -> errors
*/
//    TODO:
//        cercare eventualmente se la funzione di hashing con remapping è migliore rispetto a questa
bool insert_hash_table_recipe(Recipe_t* recipe) {
    if(recipe == NULL) {
        return true;
    }

    int index = hash_function(recipe->name, RP_TABLE_SIZE);
    recipe->next = hash_table_recipe[index];
    hash_table_recipe[index] = recipe;
    return false;
}

Recipe_t* search_hash_table_recipe(char* recipe_name) {
    int index = hash_function(recipe_name, RP_TABLE_SIZE);
    Recipe_t* temp = hash_table_recipe[index];
    while(temp != NULL && strncmp(temp->name, recipe_name, MAX_RECIPE_NAME) != 0) {
        temp = temp->next;
    }
    return temp;
}

Recipe_t* delete_hash_table_recipe(char* recipe_name) {
    int index = hash_function(recipe_name, RP_TABLE_SIZE);
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
    for (int i = 0; i < RP_TABLE_SIZE; i++) {
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

// Array della hash table per il magazzino
Ingredient_warehouse_t* hash_table_warehouse[WH_TABLE_SIZE];

/*
    Funzione che inizializza la tabella di hash
*/
void init_hash_table_warehouse() {
    for(int i = 0; i < WH_TABLE_SIZE; i++) {
        hash_table_warehouse[i] = NULL;
    }
}

/*
    Funzione che crea un nuovo ingrediente da inserire nel magazzino
*/
Ingredient_warehouse_t* create_ingredient_warehouse(char* name) {
    Ingredient_warehouse_t* new_ingredient = (Ingredient_warehouse_t*)malloc(sizeof(Ingredient_warehouse_t));
    if(new_ingredient == NULL) {
        printf("Errore nell'allocazione della memoria.\n");
        return NULL;
    }
    strncpy(new_ingredient->name, name, MAX_INGREDIENT_NAME);
    new_ingredient->name[MAX_INGREDIENT_NAME - 1] = '\0';
    new_ingredient->head = NULL;
    new_ingredient->next = NULL;
    return new_ingredient;
}

/*
    Funzione che aggiunge una nuova scadenza ad un ingrediente nel magazzino
*/
void add_expiring_warehouse(Ingredient_warehouse_t* ingredient, int quantity, int expire) {
    Expiring_t* new_exp = (Expiring_t*)malloc(sizeof(Expiring_t));
    if(new_exp == NULL) {
        printf("Errore nell'allocazione della memoria.\n");
        return;
    }
    new_exp->quantity = quantity;
    new_exp->expire = expire;
    new_exp->next = NULL;

    if (ingredient->head == NULL || ingredient->head->expire >= expire) {
        new_exp->next = ingredient->head;
        ingredient->head = new_exp;
    } else {
        Expiring_t* temp = ingredient->head;
        while (temp->next != NULL && temp->next->expire < expire) {
            temp = temp->next;
        }
        new_exp->next = temp->next;
        temp->next = new_exp;
    }
}

/*
    Funzione che cerca un ingrediente nella tabella hash del magazzino
    Returns:
        Ingredient_warehouse_t* -> puntatore all'ingrediente trovato, NULL se non trovato
*/
Ingredient_warehouse_t* search_hash_table_warehouse(char* ingredient_name) {
    unsigned int index = hash_function(ingredient_name, WH_TABLE_SIZE);
    Ingredient_warehouse_t* temp = hash_table_warehouse[index];
    while(temp != NULL && strncmp(temp->name, ingredient_name, MAX_INGREDIENT_NAME) != 0) {
        temp = temp->next;
    }
    return temp;
}

/*
    Funzione che inserisce un ingrediente nella tabella hash del magazzino
    Richieste: ingredient deve avere al suo interno già una lista di scadenze di lotti ben composta ed inoltre il puntatore a next = NULL
    Returns:
        false -> no errors
        true -> errors
*/
bool insert_hash_table_warehouse(Ingredient_warehouse_t* ingredient) {
    if(ingredient == NULL) {
        return true;
    }

    unsigned int index = hash_function(ingredient->name, WH_TABLE_SIZE);

    if(hash_table_warehouse[index] == NULL) {
        hash_table_warehouse[index] = ingredient;
    } else {
        Ingredient_warehouse_t* temp_ing = search_hash_table_warehouse(ingredient->name);
        if(temp_ing != NULL){
            Expiring_t* temp_exp = ingredient->head;
            while(temp_exp != NULL) {
                add_expiring_warehouse(temp_ing, temp_exp->quantity, temp_exp->expire);
                temp_exp = temp_exp->next;
            }
        }else{
            Ingredient_warehouse_t* temp = hash_table_warehouse[index];
            while(temp->next != NULL && temp->name != ingredient->name) {
                temp = temp->next;
            }
            temp->next = ingredient;
        }
    }
    return false;
}

/*
    Funzione che stampa la tabella hash del magazzino
*/
void print_hash_table_warehouse() {
    printf("START\n");
    for (int i = 0; i < WH_TABLE_SIZE; i++) {
        printf("Index %d: ", i);
        Ingredient_warehouse_t* temp_ing = hash_table_warehouse[i];
        while (temp_ing != NULL) {
            printf("%s -> ", temp_ing->name);
            Expiring_t* temp_exp = temp_ing->head;
            while(temp_exp != NULL) {
                printf("(%d - %d) -> ", temp_exp->quantity, temp_exp->expire);
                temp_exp = temp_exp->next;
            }
            printf("NULL | ");
            temp_ing = temp_ing->next;
        }
        printf("NULL\n");
    }
    printf("END\n");
}

/*
    Funzione che ricerca se un ordine è possibile da cucinare o meno
    Returns:
        true -> realizzabile
        false -> non realizzabile
*/
bool check_order_disponibility(Order_t* order) {
    if(order == NULL) {
        return false;
    }

    for(int i = 0; i < order->recipe.num_ingredients; i++) {
        Ingredient_recepie_t* ingredient = &order->recipe.ingredients[i];
        Ingredient_warehouse_t* ingredient_warehouse = search_hash_table_warehouse(ingredient->name);

        if (ingredient_warehouse == NULL) {
            // Ingrediente non trovato nel magazzino
            return false;
        }

        int sum = 0;
        Expiring_t* temp_exp = ingredient_warehouse->head;

        // controllo quantità dell'ingrediente i-esimo
        while (temp_exp != NULL) {
            sum += temp_exp->quantity;
            if (sum >= (ingredient->quantity * order->quantity)) {
                break;
            }
            temp_exp = temp_exp->next;
        }

        if (sum < (ingredient->quantity * order->quantity)) {
            return false;
        }
    }

    return true;
}

/*
    Funzione che una volta controllata la disponibilità dell'ordine nel magazzino, crea l'ordine sottraendo al magazzino le quantità necessarie alla sua creazione
*/
void create_order(Order_t* order, int day) {
    if (order == NULL || check_order_disponibility(order) == false) {
        printf("Ordine non realizzabile\n");
        // add_waiting_list(order, day);
        return;
    }

    for (int i = 0; i < order->recipe.num_ingredients; i++) {
        Ingredient_recepie_t *ingredient = &order->recipe.ingredients[i];
        Ingredient_warehouse_t* ingredient_warehouse = search_hash_table_warehouse(ingredient->name);
        Expiring_t* temp_exp = ingredient_warehouse->head;

        int necessary = ingredient->quantity * order->quantity;

        // rimozione di tutti i lotti scaduti
        while (temp_exp != NULL && temp_exp->expire < day) {
            Expiring_t* delete = temp_exp;
            temp_exp = temp_exp->next;
            free(delete);
        }
        ingredient_warehouse->head = temp_exp;

        // uso delle quantità stockkate in magazzino
        while (necessary > 0 && temp_exp != NULL) {
            if (temp_exp->quantity <= necessary) {
                necessary -= temp_exp->quantity;
                Expiring_t* delete = temp_exp;
                temp_exp = temp_exp->next;
                free(delete);
            } else {
                temp_exp->quantity -= necessary;
                necessary = 0;
            }
        }
        ingredient_warehouse->head = temp_exp;
    }

    // add_ready_list(order, day);
    //  TODO:
    //      implementare add_ready_list()
    printf("Ordine creato\n");
}


int main() {
    /*
        Lettura arrival (arrivo ogni...) e space (capacià)
    */
    int arrival, space;
    char extra;
    if(fscanf(stdin, "%d %d", &arrival, &space) != 2) {
        printf("Errore input stdin: mancanti o periodicita\' o capienza\n");
        return 1;
    } else {
        // Consumo resto della linea
        while((extra = fgetc(stdin) != '\n') && extra != EOF);
    }

    /*
        Lettura riga per riga dello stdin | Lettura delle operazioni da fare di giorno in giorno
    */
    char line[MAX_LINE_LENGTH];
    char *token;
    char recipe_name[MAX_RECIPE_NAME];
    char ingredient_name[MAX_INGREDIENT_NAME];
    int ingredient_quantity;
    int ingredient_expiring;
    int command_value;
    int day = 0;

    //    TODO:
    //        Vedere se 512 basta come dimensione massima lineea altrimenti incrementarla
    //        Inoltre provare a vedere se esistono metodi migliori senza usar le linee ma prendendo parola per parola
    //        -> soluzione possibile su GT
    
    init_hash_table_recipe();
    init_hash_table_warehouse();
    
    while (fgets(line, sizeof(line), stdin)) {
        // Sostituzione carattere '\n' con terminatore stringa '\0' -> agevole per tokenizzazione
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        command_value = process_command(line);
        if(command_value == -1) {
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
                new_recipe->name[MAX_RECIPE_NAME - 1] = '\0';

                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Ingrediente mancante\n\tErrore riscontrato al giorno %d\n", day);
                    free(new_recipe);
                    return 1;
                }
                strcpy(new_recipe->ingredients[ingredient_counter].name, token);
                new_recipe->ingredients[ingredient_counter].name[MAX_INGREDIENT_NAME - 1] = '\0';

                token = strtok(NULL, " ");
                if(token == NULL) {
                    printf("Quantità ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", new_recipe->ingredients[ingredient_counter].name, day);
                    free(new_recipe);
                    return 1;
                }
                sscanf(token, "%d", &new_recipe->ingredients[ingredient_counter].quantity);
                new_recipe->weight += new_recipe->ingredients[ingredient_counter].quantity;

                // lettura ingrediente + quantità FACOLTATIVI
                ingredient_counter++;
                while(token != NULL){
                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        break;
                    }
                    strcpy(new_recipe->ingredients[ingredient_counter].name, token);
                    new_recipe->ingredients[ingredient_counter].name[MAX_INGREDIENT_NAME - 1] = '\0';

                    token = strtok(NULL, " ");
                    if(token == NULL) {
                        printf("Quantità ingrediente '%s' mancante\n\tErrore riscontrato al giorno %d\n", new_recipe->ingredients[ingredient_counter].name, day);
                        free(new_recipe);
                        return 1;
                    }
                    sscanf(token, "%d", &new_recipe->ingredients[ingredient_counter].quantity);
                    new_recipe->weight += new_recipe->ingredients[ingredient_counter].quantity;
                    ingredient_counter++;
                }
                
                new_recipe->num_ingredients = ingredient_counter;
                new_recipe->next = NULL;
                if(search_hash_table_recipe(new_recipe->name) != NULL) {
                    free(new_recipe);
                    printf("ignorato\n");
                } else{
                    if (insert_hash_table_recipe(new_recipe) == true) {
                        printf("Errore nell'inserimento della ricetta nella tabella hash\n");
                        free(new_recipe);
                        return 1;
                    }
                    printf("aggiunta\n");
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
                recipe_name[MAX_RECIPE_NAME - 1] = '\0';

                Recipe_t* recipe_to_delete = search_hash_table_recipe(recipe_name);
                if(recipe_to_delete == NULL) {
                    // non esistente
                    printf("non presente\n");
                } else{
                    //  TODO:
                    //      Implementare il caso ricetta esistente ma in fase di ordinamento
                    delete_hash_table_recipe(recipe_to_delete->name);
                    free(recipe_to_delete);
                    printf("rimossa\n");
                }

                break;
            }
            case 2: {
                // ordine
                Order_t *new_order = (Order_t*)malloc(sizeof(Order_t));
                if(new_order == NULL) {
                    printf("Errore nell'allocazione della memoria.\n");
                    return 1;
                }
                
                // Lettura della ricetta + quantità da ordinare
                token = strtok(NULL, " ");
                if(token == NULL){
                    printf("Mancante ricetta da ordinare in stdin\n\tErrore riscontrato al giorno %d\n", day);
                    free(new_order);
                    return 1;
                }
                
                Recipe_t* recipe = search_hash_table_recipe(token);
                if(recipe == NULL) {
                    // ricetta non esiste
                    printf("rifiutato\n");
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

                new_order->arrival = day;

                //  TODO:
                //      impostare funzioni per capire se l'ordine può essere preparato
                //      nel caso di ordini scaduti, aggiornare il tutto
                

                //  TODO:
                //      mettere l'ordine in attesa


                // Stampa dell'ordine
                // printf("Ordine: %s - Quantità: %d\n", new_order->recipe.name, new_order->quantity);
                
                // ordine pronto
                // TODO:
                //      implementare ciò che accade una volta che l'ordine è pronto
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
                ingredient_name[MAX_INGREDIENT_NAME - 1] = '\0';
                Ingredient_warehouse_t* temp = create_ingredient_warehouse(ingredient_name);

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
                // printf("%s: %d - %d\n", ingredient_name, ingredient_quantity, ingredient_expiring);

                add_expiring_warehouse(temp, ingredient_quantity, ingredient_expiring);

                // Lettura ingrediente + quantità + scadenza FACOLTATIVE
                while(token != NULL){
                    token = strtok(NULL, " ");
                    if(token == NULL){
                        break;
                    }
                    strcpy(ingredient_name, token);
                    ingredient_name[MAX_INGREDIENT_NAME - 1] = '\0';
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
                    // printf("%s: %d - %d\n", ingredient_name, ingredient_quantity, ingredient_expiring);

                    add_expiring_warehouse(temp, ingredient_quantity, ingredient_expiring);
                }
                insert_hash_table_warehouse(temp);
                printf("rifornito\n");
                
                break;
            }
        }
        day++;
    }

    print_hash_table_warehouse();

    return 0;
}
