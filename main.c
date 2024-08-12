#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>


#define MAX_COMMAND_LENGTH 20

#define MAX_INGREDIENT_NAME 255
#define MAX_RECIPE_NAME 255

#define RP_TABLE_SIZE 25
#define WH_TABLE_SIZE 25

typedef struct ingredient_recipe {
    char name[MAX_INGREDIENT_NAME];
    int quantity;

    struct ingredient_recipe* next;
} Ingredient_recipe_t;

typedef struct recipe {
    char name[MAX_RECIPE_NAME];
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

typedef struct {
    Order_t* head;
    Order_t* tail;
} Order_list_t;

typedef struct expiring {
    int quantity;
    int expiring_date;

    struct expiring* next;
} Expiring_t;

typedef struct ingredient_warehouse {
    char name[MAX_INGREDIENT_NAME];

    Expiring_t* head;
    Expiring_t* tail;
    struct ingredient_warehouse* next;
} Ingredient_warehouse_t;


// -----------------------------------------

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

// --------------------------------------------------
// FUNZIONI PER LA HASH TABLE DEL MENU' DELLE RICETTE
// --------------------------------------------------
Recipe_t* hash_table_recipe[RP_TABLE_SIZE];

/*
    Funzione che inizializza la tabella di hash delle ricette
*/
void init_hash_table_recipe() {
    for(int i = 0; i < RP_TABLE_SIZE; i++){
        hash_table_recipe[i] = NULL;
    }
}

/*
    Funzione che inizializza la funzione hash
    Returns:
        false -> no errors
        true -> errors
*/
bool add_hash_table_recipe(Recipe_t* recipe) {
    if(recipe == NULL) {
        return true;
    }

    int index = hash_function(recipe->name, RP_TABLE_SIZE);
    recipe->next = hash_table_recipe[index];
    hash_table_recipe[index] = recipe;
    return false;
}

/*
    Funzione che cerca se un elemento con il nome corrispondere e' gia' presente all'interno della hash table delle ricette
    Returns:
        ritorna il valore dell'elemento trovato all'interno della hash table. Nel caso in cui sia 'NULL', allora significa che l'elemento non è stato trovato
*/
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
    } else {
        prev->next = temp->next;
    }

    return temp;
}


// -----------------------------------------
// FUNZIONI PER LA GESTIONE DEGLI ORDINI
// -----------------------------------------
/*
    Funzione che inizializza una lista necessaria per gli ordini, che sia lista degli ordini pronti o degli ordini in attesa di preparazione
*/
void init_order_list(Order_list_t* list) {
    list->head = NULL;
    list->tail = NULL;
}

/*
    Funzione che aggiunge in coda alla lista un nuovo elemento
*/
void add_order_list(Order_list_t* list, Order_t* order) {
    order->next = NULL;
    if(list->tail == NULL) {
        // lista vuota
        order->prev = NULL;
        list->head = order;
        list->tail = order;
    } else {
        // lista non vuota -> aggiungo in coda alla lista
        order->prev = list->tail;
        list->tail->next = order;
        list->tail = order;
    }
    return;
}

/*
    Funzione che rimuove il primo elemento dalla lista
*/
/*
    TODO:
        migliorarla man mano che vanno avanti le specifiche del progetto
*/
Order_t* remove_order_list(Order_list_t* list, int day) {
    if(list->head == NULL) {
        // Lista vuota
        return NULL;
    }

    Order_t* order = list->head;
    if(list->head == list->tail) {
        // Un solo elemento nella lista
        list->head = NULL;
        list->tail = NULL;
    } else {
        // Più di un elemento nella lista
        while(order->day_of_arrive != day) {
            order = order->next;
        }
        list->head = list->head->next;
        list->head->prev = NULL;
    }
    return order;
}

// --------------------------------------------------
// FUNZIONI PER LA HASH TABLE DEL MAGAZZINO
// --------------------------------------------------
Ingredient_warehouse_t* hash_table_warehouse[WH_TABLE_SIZE];

/*
    Funzione che inizializza la tabella di hash del magazzino
*/
void init_hash_table_warehouse() {
    for(int i = 0; i < WH_TABLE_SIZE; i++){
        hash_table_warehouse[i] = NULL;
    }
}

/*
    Funzione che inizializza la funzione hash
    Returns:
        false -> no errors
        true -> errors
*/
bool add_hash_table_warehouse(char* ingredient_name, int quantity, int expiring_date) {
    if(quantity <= 0 || expiring_date < 0) {
        return true;
    }

    int index = hash_function(ingredient_name, WH_TABLE_SIZE);
    Ingredient_warehouse_t* scroller = hash_table_warehouse[index];

    while(scroller != NULL && strncmp(scroller->name, ingredient_name, MAX_INGREDIENT_NAME) != 0) {
        scroller = scroller->next;
    }

    if(scroller == NULL) {
        // Nuovo ingrediente, aggiungilo alla lista
        scroller = (Ingredient_warehouse_t*)malloc(sizeof(Ingredient_warehouse_t));
        if(scroller == NULL) {
            printf("Errore di allocazione della memoria\n");
            return true;
        }
        strcpy(scroller->name, ingredient_name);
        scroller->head = NULL;
        scroller->tail = NULL;
        scroller->next = hash_table_warehouse[index];
        hash_table_warehouse[index] = scroller;
    }

    // Aggiungi la nuova quantità e scadenza in ordine nella lista di scadenze
    Expiring_t* new_expiring = (Expiring_t*)malloc(sizeof(Expiring_t));
    if(new_expiring == NULL) {
        printf("Errore di allocazione della memoria\n");
        return true;
    }
    new_expiring->quantity = quantity;
    new_expiring->expiring_date = expiring_date;
    new_expiring->next = NULL;

    if (scroller->head == NULL) {
        // caso lista vuota
        scroller->head = new_expiring;
        scroller->tail = new_expiring;
    } else {
        if(scroller->tail->expiring_date < new_expiring->expiring_date) {
            // caso scadenza maggiore di tutte quelle già presenti -> worst case
            scroller->tail->next = new_expiring;
            scroller->tail = new_expiring;
        } else {
            Expiring_t* scroller_expiring = scroller->head;
            Expiring_t* prev_expiring = NULL;

            while(scroller_expiring != NULL && scroller_expiring->expiring_date < expiring_date) {
                prev_expiring = scroller_expiring;
                scroller_expiring = scroller_expiring->next;
            }

            if(prev_expiring == NULL) {
                // Inserimento in testa alla coda
                new_expiring->next = scroller->head;
                scroller->head = new_expiring;
            } else {
                // Inserimento in mezzo o in fondo alla coda
                new_expiring->next = prev_expiring->next;
                prev_expiring->next = new_expiring;
            }

            if (new_expiring->next == NULL) {
                scroller->tail = new_expiring;
            }
        }
    }

    return false;
}

// -----------------------------------------
// FUNZIONI PER LA GESTIONE DEI COMANDI
// -----------------------------------------
/*
    Funzione che inserisce nella ricetta un nuovo ingrediente. Gestisce correttamente la lista
*/
void add_ingredient_recipe(Recipe_t* recipe, char* name, int quantity) {
    Ingredient_recipe_t* ingredient = (Ingredient_recipe_t*)malloc(sizeof(Ingredient_recipe_t));
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
}

/*
    Funzione che implementa la lettura di aggiungi_ricetta
*/
void manage_aggiungi_ricetta(char* line) {
    char* token;

    // Salta il comando "aggiungi_ricetta"
    token = strtok(line, " ");
    if (token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }

    // Lettura nome della ricetta
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Errore: nome della ricetta mancante.\n");
        return;
    }

    Recipe_t* new_recipe = (Recipe_t*)malloc(sizeof(Recipe_t));
    if (new_recipe == NULL) {
        printf("Errore di allocazione della memoria\n");
        return;
    }
    strcpy(new_recipe->name, token);
    new_recipe->name[MAX_RECIPE_NAME - 1] = '\0';
    new_recipe->weight = 0;
    new_recipe->head = NULL;

    if(search_hash_table_recipe(new_recipe->name) != NULL) {
        free(new_recipe);
        printf("ignorato\n");
        return;
    }

    // Lettura degli ingredienti e delle loro quantità
    while ((token = strtok(NULL, " ")) != NULL) {
        char ingredient_name[MAX_INGREDIENT_NAME];
        int quantity;

        strcpy(ingredient_name, token);
        ingredient_name[MAX_INGREDIENT_NAME - 1] = '\0';

        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Errore: quantità dell'ingrediente '%s' mancante.\n", ingredient_name);
            free(new_recipe);
            return;
        }
        quantity = atoi(token);

        add_ingredient_recipe(new_recipe, ingredient_name, quantity);
        new_recipe->weight += quantity;

    }

    // Aggiunta della ricetta alla tabella di hash
    if(add_hash_table_recipe(new_recipe) == true) {
        printf("Errore: inserimento nella tabella di hash fallito.\n");
        free(new_recipe);
        return;
    }
    printf("aggiunta\n");
}

/*
    Funzione che implementa la lettura di rimuovi_ricetta
*/
void manage_rimuovi_ricetta(char* line) {
    char* token;
    char recipe_name[MAX_RECIPE_NAME];

    // Salta il comando "rimuovi_ricetta"
    token = strtok(line, " ");
    if (token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }

    // Lettura nome della ricetta da cancellare
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Errore: nome della ricetta mancante.\n");
        return;
    }
    strcpy(recipe_name, token);
    recipe_name[MAX_RECIPE_NAME - 1] = '\0';

    // Eliminazione della ricetta
    Recipe_t* recipe_to_delete = search_hash_table_recipe(recipe_name);
    if(recipe_to_delete == NULL) {
        printf("non presente\n");
    } else {
        /*
            TODO:
                implementare il caso ricetta esistente ma in fase di ordinamento
        */
        delete_hash_table_recipe(recipe_to_delete->name);
        free(recipe_to_delete);
        printf("rimossa\n");
    }
    
}

/*
    Funzione che implementa la lettura di ordine
*/
void manage_ordine(char* line, int day, Order_list_t* ready_orders, Order_list_t* waiting_orders) {
    char* token;
    char recipe_name[MAX_RECIPE_NAME];

    // Salta il comando "ordine"
    token = strtok(line, " ");
    if (token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }

    // Lettura nome della ricetta da ordinare
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Errore: nome della ricetta mancante.\n");
        return;
    }
    strcpy(recipe_name, token);
    recipe_name[MAX_RECIPE_NAME - 1] = '\0';

    // Controllo ricetta esista
    Recipe_t* recipe = search_hash_table_recipe(recipe_name);
    if(recipe != NULL) {
        printf("rifiutato\n");
        return;
    }

    Order_t* new_order = (Order_t*)malloc(sizeof(Order_t));
    new_order->recipe = recipe;

    // Lettura quantita' di elementi della ricetta da ordinare
    token = strtok(NULL, " ");
    if (token == NULL) {
        printf("Errore: quantità da ordinare della ricetta mancante.\n");
        free(new_order);
        return;
    }
    new_order->quantity= atoi(token);
    
    new_order->day_of_arrive = day;

    /*
        TODO:
            Zona 'accettato'
    */
    /*
    update_warehouse(new_order, day);
    bool result = check_warehouse(new_order);
    if(result == false) {
        // ordini in attesa
        add_order_list(waiting_orders, new_order);
    } else {
        // ordini pronti
        add_order_list(ready_orders, new_order);
    }
    */
}

/*
    Funzione che implementa la lettura di rifornimento
*/
void manage_rifornimento(char* line) {
    char* token;

    // Salta il comando "rifornimento"
    token = strtok(line, " ");
    if (token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }
    
    // Lettura dell'ingrediente, della quantita' da rifornire e della scadenza
    while ((token = strtok(NULL, " ")) != NULL) {
        char ingredient_name[MAX_INGREDIENT_NAME];

        strcpy(ingredient_name, token);
        ingredient_name[MAX_INGREDIENT_NAME - 1] = '\0';

        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Errore: quantità dell'ingrediente '%s' mancante.\n", ingredient_name);
            return;
        }
        int quantity = atoi(token);

        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Errore: scadenza dell'ingrediente '%s' mancante.\n", ingredient_name);
            return;
        }
        int expiring_date = atoi(token);

        add_hash_table_warehouse(ingredient_name, quantity, expiring_date);
        printf("rifornito\n");
    }
}

/*
    Funzione che dato in input una stringa gestisce il tipo di comando assegnato mediante input stdin
*/
void manage_line(char* line, int day, Order_list_t* ready_orders, Order_list_t* waiting_orders) {
    char command[MAX_COMMAND_LENGTH];

    if(sscanf(line, "%s", command) != 1) {
        printf("Errore nella lettura del comando\n");
        return;
    }

    if(strcmp(command, "aggiungi_ricetta") == 0) {
        manage_aggiungi_ricetta(line);
    } else if (strcmp(command, "rimuovi_ricetta") == 0) {
        manage_rimuovi_ricetta(line);
    } else if (strcmp(command, "ordine") == 0) {
        manage_ordine(line, day, ready_orders, waiting_orders);
    } else if (strcmp(command, "rifornimento") == 0) {
        manage_rifornimento(line);
    }
    return;
}

int main() {
    // inizializzazione delle strutture
    init_hash_table_recipe();
    init_hash_table_warehouse();

    // inizializzazione delle liste per gli ordini
    Order_list_t ready_orders;
    Order_list_t waiting_orders;
    init_order_list(&ready_orders);
    init_order_list(&waiting_orders);


    // lettura della periodicità d'arrivo e lettura dello spazio a disposizione del camioncino
    int arrive_time, capacity;
    char extra;
    if(fscanf(stdin, "%d %d", &arrive_time, &capacity) != 2) {
        printf("Errore input stdin: mancanti o periodicita\' o capienza\n");
        return 1;
    } else {
        // Consumo resto della linea
        while((extra = fgetc(stdin) != '\n') && extra != EOF);
    }

    // lettura dei comandi e loro gestione
    char* line = NULL;
    size_t len = 0;
    int day = 0;
    while(getline(&line, &len, stdin) != -1){
        // cancellazione del carattere '\n'
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len-1] == '\n') {
            line[line_len-1] = '\0';
        }
        manage_line(line, day, &ready_orders, &waiting_orders);

        day++;
    }
    free(line);

    return 0;
}