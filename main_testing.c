#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define MAX_COMMAND_LENGTH 20

// Strutture per le ricette e gli ingredienti che le contengono
typedef struct ingredient_recipe {
    char *name;
    int quantity;

    struct ingredient_recipe* next;
} Ingredient_recipe_t;

typedef struct recipe {
    char *name;
    int weight;

    Ingredient_recipe_t* head;
    struct recipe* next;
} Recipe_t;

typedef struct order{
    Recipe_t* recipe;
    int quantity;
    int day_of_arrive;

    struct order* prev;
    struct order* next;
} Order_t;

// Struttura per le liste di ordini pronti o in attesa
typedef struct {
    Order_t* head;
    Order_t* tail;
} Order_list_t;

// Struttura per il magazzino e i lotti
typedef struct expiring {
    int quantity;
    int expiring_date;

    struct expiring* next;
} Expiring_t;

typedef struct ingredient_warehouse {
    char *name;
    int total_quantity;

    Expiring_t* head;
    Expiring_t* tail;
    struct ingredient_warehouse* next;
} Ingredient_warehouse_t;

// Struttura per il camioncino
typedef struct {
    Order_t** orders;
    int size;
    int capacity;
} Lorry_t;

Lorry_t lorry;

// ---------------------------------------
// -------------------------------------
// FUNZIONI PER LA GESTIONE DEGLI ORDINI
// -------------------------------------
void init_order_list(Order_list_t* list) {
    list->head = NULL;
    list->tail = NULL;
}

// ------------------------------------
// FUNZIONI PER LA GESTIONE DEI COMANDI
// ------------------------------------
void add_ingredient_recipe(Recipe_t* recipe, char* name, int quantity) {
    Ingredient_recipe_t* ingredient = (Ingredient_recipe_t*)malloc(sizeof(Ingredient_recipe_t));
    if(ingredient == NULL){
        printf("Errore: errata allocazione memoria.\n");
        return;
    }
    
    ingredient->name = malloc((strlen(name) + 1) * sizeof(char));
    if (ingredient->name == NULL) {
        printf("Errore: errata allocazione memoria.\n");
        free(ingredient);
        return;
    }
    strcpy(ingredient->name, name);

    ingredient->quantity = quantity;
    ingredient->next = NULL;

    if(recipe->head == NULL) {
        recipe->head = ingredient;
    } else {
        Ingredient_recipe_t* scroller = recipe->head;
        while(scroller->next != NULL) {
            scroller = scroller->next;
        }
        scroller->next = ingredient;
    }

    return;
}

void manage_aggiungi_ricetta(char* line) {
    char* token;

    // Salta il comando "aggiungi_ricetta"
    token = strtok(line, " ");
    if(token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }

    // Lettura nome della ricetta
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("Errore: nome della ricetta mancante.\n");
        return;
    }

    Recipe_t* new_recipe = (Recipe_t*)malloc(sizeof(Recipe_t));
    if(new_recipe == NULL) {
        printf("Errore: errata allocazione della memoria\n");
        return;
    }

    new_recipe->name = (char*)malloc((strlen(token) + 1) * sizeof(char));
    if(new_recipe->name == NULL){
        printf("Errore: errata allocazione della memoria\n");
        free(new_recipe);
        return;
    }

    strcpy(new_recipe->name, token);
    new_recipe->weight = 0;
    new_recipe->head = NULL;

    if(search_hash_table_recipe(new_recipe->name) != NULL) {
        free(new_recipe);
        printf("ignorato\n");
        return;
    }
    
    // Lettura degli ingredienti e delle loro quantità
    while((token = strtok(NULL, " ")) != NULL) {
        char* ingredient_name = (char*)malloc((strlen(token) + 1) * sizeof(char));;
        if(ingredient_name == NULL) {
            printf("Errore: errata allocazione della memoria\n");
            free(new_recipe->name);
            free(new_recipe);
            return;
        }
        strcpy(ingredient_name, token);

        token = strtok(NULL, " ");
        if(token == NULL) {
            printf("Errore: quantità dell'ingrediente '%s' mancante.\n", ingredient_name);
            free(ingredient_name);
            free(new_recipe->name);
            free(new_recipe);
            return;
        }
        int quantity = atoi(token);

        add_ingredient_recipe(new_recipe, ingredient_name, quantity);
        new_recipe->weight += quantity;

        free(ingredient_name);
    }

    if(add_hash_table_recipe(new_recipe) == true) {
        printf("Errore: inserimento nella tabella di hash fallito.\n");
        free(new_recipe);
        return;
    }

    printf("aggiunta\n");
}

void manage_rimuovi_ricetta(char* line, Order_list_t* ready_list, Order_list_t* waiting_list) {
    char* token;

    // Salta il comando "rimuovi_ricetta"
    token = strtok(line, " ");
    if(token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }

    // Lettura nome della ricetta da cancellare
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("Errore: nome della ricetta mancante.\n");
        return;
    }
    char* recipe_name = (char*)malloc((strlen(token) + 1) * sizeof(char));
    strcpy(recipe_name, token);

    // Eliminazione della ricetta
    Recipe_t* recipe_to_delete = search_hash_table_recipe(recipe_name);
    free(recipe_name);

    if(recipe_to_delete == NULL) {
        printf("non presente\n");
    } else {
        if(search_list(waiting_list, recipe_to_delete->name) == NULL && search_list(ready_list, recipe_to_delete->name) == NULL) {
            delete_hash_table_recipe(recipe_to_delete->name);
            free(recipe_to_delete->name);
            free(recipe_to_delete);
            printf("rimossa\n");
        } else {
            printf("ordini in sospeso\n");
        }
    }

    return;
}

void manage_ordine(char* line, int day, Order_list_t* ready_orders, Order_list_t* waiting_orders) {
    char* token;

    // Salta il comando "ordine"
    token = strtok(line, " ");
    if(token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }

    // Lettura nome della ricetta da ordinare
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("Errore: nome della ricetta mancante.\n");
        return;
    }
    char* recipe_name = (char*)malloc((strlen(token) + 1) * sizeof(char));
    strcpy(recipe_name, token);

    // Controllo ricetta esista
    Recipe_t* recipe = search_hash_table_recipe(recipe_name);
    free(recipe_name);

    if(recipe == NULL) {
        printf("rifiutato\n");
        return;
    }

    Order_t* new_order = (Order_t*)malloc(sizeof(Order_t));
    new_order->recipe = recipe;

    // Lettura quantita' di elementi della ricetta da ordinare
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("Errore: quantità da ordinare della ricetta mancante.\n");
        free(new_order);
        return;
    }
    new_order->quantity= atoi(token);
    
    new_order->day_of_arrive = day;

    if(time_to_cook(new_order, day) == true) {
        // aggiornamento lista ready
        add_order_list(ready_orders, new_order);
    } else {
        // aggiornamento lista waiting
        add_order_list(waiting_orders, new_order);
    }
    
    printf("accettato\n");
}


void manage_line(char* line, int day, Order_list_t* ready_orders, Order_list_t* waiting_orders) {
    char command[MAX_COMMAND_LENGTH];

    if(sscanf(line, "%s", command) != 1) {
        printf("Errore: lettura del comando\n");
        return;
    }

    if(strcmp(command, "aggiungi_ricetta") == 0) {
        manage_aggiungi_ricetta(line);
    } else if(strcmp(command, "rimuovi_ricetta") == 0) {
        manage_rimuovi_ricetta(line, ready_orders, waiting_orders);
    } else if(strcmp(command, "ordine") == 0) {
        // manage_ordine(line, day, ready_orders, waiting_orders);
    } else if(strcmp(command, "rifornimento") == 0) {
        // manage_rifornimento(line, day, ready_orders, waiting_orders);
    }

    return;
}

int main() {
    // Inizializzazione delle liste per gli ordini
    Order_list_t ready_orders;
    Order_list_t waiting_orders;
    init_order_list(&ready_orders);
    init_order_list(&waiting_orders);

    // Lettura della periodicità d'arrivo e lettura dello spazio a disposizione del camioncino
    int arrive_time, capacity;
    if(fscanf(stdin, "%d %d\n", &arrive_time, &capacity) != 2) {
        printf("Errore: input stdin mancanti o periodicita\' o capienza\n");
        return 1;
    }

    char* line = NULL;
    size_t len = 0;
    int day = 0;

    // Lettura dei comandi e loro gestione
    while(getline(&line, &len, stdin) != -1) {
        // Cancellazione del carattere '\n'
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len-1] == '\n') {
            line[line_len-1] = '\0';
        }
        manage_line(line, day, &ready_orders, &waiting_orders);

        day++;
    }
    
    // Pulizia della linea
    free(line);

    return 0;
}