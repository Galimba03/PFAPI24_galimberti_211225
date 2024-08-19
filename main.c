#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_COMMAND_LENGTH 20

#define MAX_INGREDIENT_NAME 255
#define MAX_RECIPE_NAME 255

#define RP_TABLE_SIZE 50
#define WH_TABLE_SIZE 100

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


// -----------------------------------------
/*
    Funzione che crea l'indice della tabella di hash in cui andra' ad essere inserito il valore trovata su internet
    Returns:
        unit32_t = indice in cui verra' posizionato il valore nella tabella di hash
*/
uint32_t hash_function(char* name, int hash_table_dim) {
    uint32_t hash_value = 5381;
    uint8_t c;
    while ((c = *name++))
        hash_value = ((hash_value << 5) + hash_value) + c;
    return hash_value % hash_table_dim;
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
        - false -> no errors
        - true -> errors
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
        - Recipe_t =    NULL, se non trovata
                        ricetta, se trovata
*/
Recipe_t* search_hash_table_recipe(char* recipe_name) {
    int index = hash_function(recipe_name, RP_TABLE_SIZE);
    Recipe_t* temp = hash_table_recipe[index];

    while(temp != NULL && strncmp(temp->name, recipe_name, MAX_RECIPE_NAME) != 0) {
        temp = temp->next;
    }

    return temp;
}

/*
    Funzione che elimina dalla hash table la ricetta inserita in ingresso
    Returns:
        - Recipe_t =    NULL, se ricetta non presente
                        ricetta, se eliminata dalla tabella
*/
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

// ----------------------------------------
// FUNZIONI PER LA HASH TABLE DEL MAGAZZINO
// ----------------------------------------
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
        - false -> no errors
        - true -> errors
*/
bool update_and_add_hash_table_warehouse(char* ingredient_name, int quantity, int expiring_date, int day) {
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
            printf("Errore: allocazione della memoria.\n");
            return true;
        }
        scroller->name = (char*)malloc((strlen(ingredient_name) + 1) * sizeof(char));
        if(scroller->name == NULL) {
            printf("Errore: allocazione memoria\n");
            free(scroller);
            return true;
        }
        strcpy(scroller->name, ingredient_name);
        scroller->total_quantity = 0;
        scroller->head = NULL;
        scroller->tail = NULL;
        scroller->next = hash_table_warehouse[index];
        hash_table_warehouse[index] = scroller;
    }

    // Aggiungi la nuova quantità e scadenza in ordine nella lista di scadenze
    Expiring_t* new_expiring = (Expiring_t*)malloc(sizeof(Expiring_t));
    if(new_expiring == NULL) {
        printf("Errore: allocazione di memoria.\n");
        return true;
    }
    new_expiring->quantity = quantity;
    new_expiring->expiring_date = expiring_date;
    new_expiring->next = NULL;

    if (scroller->head == NULL) {
        // Caso lista vuota
        scroller->head = new_expiring;
        scroller->tail = new_expiring;
        scroller->total_quantity += new_expiring->quantity;
    } else {
        if(scroller->tail->expiring_date < new_expiring->expiring_date) {
            // Caso scadenza maggiore di tutte quelle già presenti -> worst case
            scroller->tail->next = new_expiring;
            scroller->tail = new_expiring;
            scroller->total_quantity += new_expiring->quantity;
        } else {
            Expiring_t* scroller_expiring = scroller->head;
            Expiring_t* prev_expiring = NULL;

            // Aggiornamento della lista eliminando tutti gli elementi scaduti + posizionamento nel magazzino in ordine di scadenza
            while(scroller_expiring != NULL && scroller_expiring->expiring_date < expiring_date) {
                // Rimozione elemento scaduto
                if(scroller_expiring->expiring_date <= day) {
                    // Rimozione degli elementi scaduti
                    if(prev_expiring == NULL) {
                        // L'elemento da rimuovere è la testa della lista
                        scroller->head = scroller_expiring->next;
                        if(scroller->head == NULL) {
                            // La lista diventa vuota, aggiorna anche la coda
                            scroller->tail = NULL;
                        }
                    } else {
                        // L'elemento da rimuovere non è la testa -> o è in mezzo o è la coda
                        prev_expiring->next = scroller_expiring->next;
                        if(prev_expiring->next == NULL) {
                            // L'elemento rimosso era la coda
                            scroller->tail = prev_expiring;
                        }
                    }

                    Expiring_t* temp = scroller_expiring;
                    scroller->total_quantity -= temp->quantity;
                    scroller_expiring = scroller_expiring->next;
                    free(temp);
                } else {
                    // Terminata fase 'rimozione scaduti'
                    prev_expiring = scroller_expiring;
                    scroller_expiring = scroller_expiring->next;
                }

            }

            // Fase di inserimento dell'elemento
            if(prev_expiring == NULL) {
                // Inserimento in testa alla coda
                scroller->total_quantity += new_expiring->quantity;
                new_expiring->next = scroller->head;
                scroller->head = new_expiring;
            } else {
                // Inserimento in mezzo o in fondo alla coda
                scroller->total_quantity += new_expiring->quantity;
                new_expiring->next = prev_expiring->next;
                prev_expiring->next = new_expiring;
            }

            // Caso in cui l'elemento inserito sia finito in fondo alla coda
            if(new_expiring->next == NULL) {
                scroller->tail = new_expiring;
            }
        }
    }

    return false;
}

/*
    Funzione che controlla se la ricetta è realizzabile cancellando anche gli ingredienti scaduti
    Returns:
        - false -> non realizzabile
        - true -> realizzabile
*/
bool check_warehouse(Order_t* order, int day) {
    Ingredient_recipe_t* recipe_ingredient = order->recipe->head;

    while(recipe_ingredient != NULL) {
        int index = hash_function(recipe_ingredient->name, WH_TABLE_SIZE);
        Ingredient_warehouse_t* warehouse_ingredient = hash_table_warehouse[index];

        while(warehouse_ingredient != NULL && strncmp(warehouse_ingredient->name, recipe_ingredient->name, MAX_INGREDIENT_NAME) != 0) {
            warehouse_ingredient = warehouse_ingredient->next;
        }

        if (warehouse_ingredient == NULL) {
            return false;
        }

        // Verificando se la quantità totale è sufficiente
        if (warehouse_ingredient->total_quantity < recipe_ingredient->quantity * order->quantity) {
            return false;
        }

        if(warehouse_ingredient->head != NULL){
            // Controllo e rimozione degli ingredienti scaduti
            Expiring_t* scroller_expiring = warehouse_ingredient->head;
            Expiring_t* prev_expiring = NULL;


            // Aggiornamento della lista eliminando tutti gli elementi scaduti
            while(scroller_expiring != NULL && scroller_expiring->expiring_date <= day) {
                // Rimozione elemento scaduto
                if(prev_expiring == NULL) {
                    // L'elemento da rimuovere è la testa della lista
                    warehouse_ingredient->head = scroller_expiring->next;
                    if(warehouse_ingredient->head == NULL) {
                        // La lista diventa vuota, aggiorna anche la coda
                        warehouse_ingredient->tail = NULL;
                    }
                } else {
                    // L'elemento da rimuovere non è la testa
                    prev_expiring->next = scroller_expiring->next;
                    if(prev_expiring->next == NULL) {
                        // L'elemento rimosso era la coda
                        warehouse_ingredient->tail = prev_expiring;
                    }
                }
                Expiring_t* temp = scroller_expiring;
                warehouse_ingredient->total_quantity -= temp->quantity;
                scroller_expiring = scroller_expiring->next;
                free(temp);
            }
        }

        // Calcolo quantità necessaria
        int required_quantity = recipe_ingredient->quantity * order->quantity;
        if(warehouse_ingredient->total_quantity < required_quantity) {
            return false;
        }

        // Passa all'ingrediente successivo nella ricetta
        recipe_ingredient = recipe_ingredient->next;

    }
    return true;
}

/*
    Funzione che controlla se l'ordine è realizzabile. Se lo è aggiorna il magazzino togliendogli tutti gli elementi necessari alla realizzazione della ricetta
    Returns:
        - false -> non realizzabile
        - true -> realizzabile e realizzato
*/
bool time_to_cook(Order_t* order, int day) {
    if(check_warehouse(order, day) == false) {
        return false;
    }

    Ingredient_recipe_t* recipe_ingredient = order->recipe->head;
    // Finché gli ingredienti della ricetta non son finiti...
    while(recipe_ingredient != NULL) {
        int index = hash_function(recipe_ingredient->name, WH_TABLE_SIZE);
        Ingredient_warehouse_t* warehouse_ingredient = hash_table_warehouse[index];

        while(warehouse_ingredient != NULL && strncmp(warehouse_ingredient->name, recipe_ingredient->name, MAX_INGREDIENT_NAME) != 0) {
            warehouse_ingredient = warehouse_ingredient->next;
        }

        if (warehouse_ingredient == NULL) {
            return false;
        }

        // Verificando se la quantità totale è sufficiente
        if (warehouse_ingredient->total_quantity < recipe_ingredient->quantity * order->quantity) {
            return false;
        }

        if(warehouse_ingredient->head != NULL){
            // Controllo e rimozione degli ingredienti scaduti
            Expiring_t* scroller_expiring = warehouse_ingredient->head;

            // Calcolo quantità necessaria, ed in caso affermativo, si procede alla preparazione
            int required_quantity = recipe_ingredient->quantity * order->quantity;
            scroller_expiring = warehouse_ingredient->head;

            while(scroller_expiring != NULL && required_quantity > 0) {
                if(scroller_expiring->quantity > required_quantity) {
                    // non devo "prosciugare tutto il lotto"
                    scroller_expiring->quantity -= required_quantity;
                    warehouse_ingredient->total_quantity -= required_quantity;
                    required_quantity = 0;
                } else {
                    // devo "prosciugare" tutto il lotto
                    required_quantity -= scroller_expiring->quantity;
                    warehouse_ingredient->total_quantity -= scroller_expiring->quantity;

                    Expiring_t* temp = scroller_expiring;
                    scroller_expiring = scroller_expiring->next;
                    free(temp);

                    warehouse_ingredient->head = scroller_expiring;

                    if(scroller_expiring == NULL) {
                        warehouse_ingredient->tail = NULL;
                    }
                }
            }

            if(required_quantity > 0) {
                // Non è stato possibile soddisfare l'ordine per mancanza di quantità sufficiente
                return false;
            }
            
        }
        
        // Passa all'ingrediente successivo nella ricetta
        recipe_ingredient = recipe_ingredient->next;
    }

    return true;
}

// -------------------------------------
// FUNZIONI PER LA GESTIONE DEGLI ORDINI
// -------------------------------------
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
    if(list->head == NULL || list->head->day_of_arrive > order->day_of_arrive) {
        // Inserimento in testa
        order->next = list->head;
        order->prev = NULL;
        if(list->head != NULL) {
            list->head->prev = order;
        }
        list->head = order;
        if(list->tail == NULL) {
            // Caso in cui la lista era vuota
            list->tail = order;
        }
    } else {
        // Inserimento in mezzo o in fondo
        if(order->day_of_arrive > list->tail->day_of_arrive) {
            // Inserimento in fondo direttamente in coda
            order->prev = list->tail;
            order->next = NULL;
            list->tail->next = order;
            list->tail = order;
        } else {
            // Inserimento in mezzo
            Order_t* order_scroller = list->head;
            while(order_scroller->next != NULL && order_scroller->next->day_of_arrive <= order->day_of_arrive) {
                order_scroller = order_scroller->next;
            }
            order->next = order_scroller->next;
            order_scroller->next->prev = order;
            order_scroller->next = order;
            order->prev = order_scroller;
        }
    }
}

/*
    Funzione che cerca un elemento nella lista passata in ingresso per nome
    Returns:
        - NULL -> if not found
        - Order_t -> if found
*/
Order_t* search_list(Order_list_t* list, char* recipe_name) {
    Order_t* order_scroller = list->head;

    while(order_scroller != NULL && strncmp(order_scroller->recipe->name, recipe_name, MAX_RECIPE_NAME) != 0) {
        order_scroller = order_scroller->next;
    }

    return order_scroller;
}

/*
    Funzione che elimina un ordine passato in ingresso, dalla lista passata in ingresso
    Returns:
        - NULL -> if not deleted
        - Order_t -> if deleted
*/
Order_t* delete_order_list_element(Order_list_t* list, Order_t* to_delete) {
    if(to_delete == NULL) {
        return NULL;
    }

    // Cancellazione dell'ordine
    if(to_delete == list->head) {
        // Ordine in testa alla lista
        list->head = to_delete->next;
        if(list->head != NULL) {
            list->head->prev = NULL;
        } else {
            // Lista è vuota
            list->tail = NULL;
        }
    } else {
        if(to_delete == list->tail) {
            // Ordine in coda alla lista
            list->tail = to_delete->prev;
            if(list->tail != NULL) {
                list->tail->next = NULL;
            }
        } else {
            // Ordine in mezzo alla lista
            to_delete->prev->next = to_delete->next;
            to_delete->next->prev = to_delete->prev;
        }
        
    }
    
    return to_delete;
}

/*
    Funzione che cucina gli ordini in attesa
*/
void cook_waiting(Order_list_t* ready_list, Order_list_t* waiting_list, int day) {
    Order_t* scroller = waiting_list->head;

    while(scroller != NULL) {
        Order_t* next_order = scroller->next;

        // Si cerca se è possibile cucinare la ricetta
        if(time_to_cook(scroller, day) == true) {
            delete_order_list_element(waiting_list, scroller);
            add_order_list(ready_list, scroller);
        }

        scroller = next_order;
    }

    return;
}

// ----------------------------------------
// FUNZIONI PER IL CONTROLLO DEL CAMIONCINO
// ----------------------------------------
/*
    Funzione che inizializza il camioncino della dimensione scelta
*/
void init_lorry() {
    lorry.size = 0;
    lorry.capacity = 5;
    lorry.orders = malloc(sizeof(Order_t*) * lorry.capacity);
}

/*
    Funzione che aumenta la dimensione del camioncino (sotto ci sta il ragionamento di un VLA)
*/
void resize_lorry() {
    lorry.capacity *= 2;
    lorry.orders = realloc(lorry.orders, sizeof(Order_t*) * lorry.capacity);
}

/*
    Funzione che aggiunge al camioncino un ordine, e se serve ne aumenta la dimensione
*/
void add_order_to_lorry(Order_t* order) {
    if(lorry.size >= lorry.capacity) {
        // Incremento della dimensione del camioncino di 5
        resize_lorry();
    }
    lorry.orders[lorry.size++] = order;

    return;
}

/*
    Funzione che resetta il camioncino
*/
void free_lorry() {
    free(lorry.orders);
    lorry.orders = NULL;
    lorry.size = 0;
    lorry.capacity = 0;
}

/*
    Funzione per il caricamento del camioncino
*/
void load_lorry(Order_list_t* ready_list, int lorry_space) {
    Order_t* current_order = ready_list->head;

    while(current_order != NULL) {
        int order_weight = current_order->recipe->weight * current_order->quantity;

        if(order_weight <= lorry_space) {
            lorry_space -= order_weight;

            // Prima di rimuovere l'ordine dalla lista, viene salvato il prossimo ordine
            Order_t* next_order = current_order->next;

            // Rimozione dell'ordine corrente dalla lista
            Order_t* order_to_load = delete_order_list_element(ready_list, current_order);
            add_order_to_lorry(order_to_load);

            // Avanzamento all'ordine successivo
            current_order = next_order;
        } else {
            // Se l'ordine non può essere caricato, interrompi il caricamento
            break;
        }
    }

    return;
}

/*
    Insertion sort per riordinare gli elementi del camioncino in ordine di peso
*/
void insertion_sort_weight(Order_t* orders[], int n) {
    for(int i = 1; i < n; i++) {
        Order_t* key = orders[i];
        int j = i-1;

        while(j >= 0 && (orders[j]->recipe->weight * orders[j]->quantity) < (key->recipe->weight * key->quantity)) {
            orders[j+1] = orders[j];
            j -= 1;
        }
        orders[j+1] = key;
    }
}

/*
    Funzione per la stampa in ordine di peso degli elementi del camioncino
*/
void print_lorry() {
    if(lorry.size == 0) {
        printf("camioncino vuoto\n");
    } else {
        // Ordinamento degli ordini in base al peso
        insertion_sort_weight(lorry.orders, lorry.size);

        // Stampa gli ordini
        for (int i = 0; i < lorry.size; i++) {
            Order_t* order = lorry.orders[i];
            printf("%d %s %d\n", order->day_of_arrive, order->recipe->name, order->quantity);
        }

        // Pulizia della memoria
        for (int i = 0; i < lorry.size; i++) {
            free(lorry.orders[i]);
        }

        // Camioncino vuoto
        lorry.size = 0;
    }
}

// ------------------------------------
// FUNZIONI PER LA GESTIONE DEI COMANDI
// ------------------------------------
/*
    Funzione che inserisce nella ricetta un nuovo ingrediente. Gestisce correttamente la lista
*/
void add_ingredient_recipe(Recipe_t* recipe, char* name, int quantity) {
    Ingredient_recipe_t* ingredient = (Ingredient_recipe_t*)malloc(sizeof(Ingredient_recipe_t));
    ingredient->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
    if(ingredient->name == NULL){
        printf("Errore: errata allocazione memoria.\n");
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

/*
    Funzione che implementa la lettura di 'aggiungi_ricetta'
*/
void manage_aggiungi_ricetta(char* line) {
    char* token;

    // Salta il comando 'aggiungi_ricetta'
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
        free(new_recipe->name);
        free(new_recipe);
        printf("ignorato\n");
        return;
    }
    
    // Lettura degli ingredienti e delle loro quantità
    while((token = strtok(NULL, " ")) != NULL) {
        char* ingredient_name = (char*)malloc((strlen(token) + 1) * sizeof(char));
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
        free(new_recipe->name);
        free(new_recipe);
        return;
    }

    printf("aggiunta\n");
}

/*
    Funzione che implementa la lettura di 'rimuovi_ricetta'
*/
void manage_rimuovi_ricetta(char* line, Order_list_t* ready_list, Order_list_t* waiting_list) {
    char* token;

    // Salta il comando 'rimuovi_ricetta'
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
    if(recipe_name == NULL) {
        printf("Errore: allocazione memoria.\n");
        return;
    }
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

/*
    Funzione che implementa la lettura di 'ordine'
*/
void manage_ordine(char* line, int day, Order_list_t* ready_orders, Order_list_t* waiting_orders) {
    char* token;

    // Salta il comando 'ordine'
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
    if(recipe_name == NULL) {
        printf("Errore: allocazione memoria.\n");
        return;
    }
    strcpy(recipe_name, token);

    // Controllo ricetta esista
    Recipe_t* recipe = search_hash_table_recipe(recipe_name);
    free(recipe_name);

    if(recipe == NULL) {
        printf("rifiutato\n");
        return;
    }

    Order_t* new_order = (Order_t*)malloc(sizeof(Order_t));
    if(new_order == NULL) {
        printf("Errore: allocazione memoria.\n");
        return;
    }
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

/*
    Funzione che implementa la lettura di 'rifornimento'
*/
void manage_rifornimento(char* line, int day, Order_list_t* ready_orders, Order_list_t* waiting_orders) {
    char* token;

    // Salta il comando 'rifornimento'
    token = strtok(line, " ");
    if(token == NULL) {
        printf("Errore: comando mancante.\n");
        return;
    }
    
    // Lettura dell'ingrediente, della quantita' da rifornire e della scadenza
    while ((token = strtok(NULL, " ")) != NULL) {
        char* ingredient_name = (char*)malloc((strlen(token) + 1) * sizeof(char));
        if(ingredient_name == NULL) {
            printf("Errore: allocazione memoria.\n");
            return;
        }
        strcpy(ingredient_name, token);

        token = strtok(NULL, " ");
        if(token == NULL) {
            printf("Errore: quantità dell'ingrediente '%s' mancante.\n", ingredient_name);
            free(ingredient_name);
            return;
        }
        int quantity = atoi(token);

        token = strtok(NULL, " ");
        if(token == NULL) {
            printf("Errore: scadenza dell'ingrediente '%s' mancante.\n", ingredient_name);
            free(ingredient_name);
            return;
        }
        int expiring_date = atoi(token);

        update_and_add_hash_table_warehouse(ingredient_name, quantity, expiring_date, day);
    }
    printf("rifornito\n");
    cook_waiting(ready_orders, waiting_orders, day);
}

/*
    Funzione che dato in input una stringa gestisce il tipo di comando assegnato mediante input stdin
*/
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
        manage_ordine(line, day, ready_orders, waiting_orders);
    } else if(strcmp(command, "rifornimento") == 0) {
        manage_rifornimento(line, day, ready_orders, waiting_orders);
    }
    return;
}

int main() {
    // Inizializzazione delle strutture
    init_hash_table_recipe();
    init_hash_table_warehouse();

    // Inizializzazione delle liste per gli ordini
    Order_list_t ready_orders;
    Order_list_t waiting_orders;
    init_order_list(&ready_orders);
    init_order_list(&waiting_orders);

    // Inizializzazione camioncino
    init_lorry();

    // Lettura della periodicità d'arrivo e lettura dello spazio a disposizione del camioncino
    int arrive_time, capacity;
    if(fscanf(stdin, "%d %d\n", &arrive_time, &capacity) != 2) {
        printf("Errore: input stdin mancanti o periodicita\' o capienza\n");
        return 1;
    }

    // Lettura dei comandi e loro gestione
    char* line = NULL;
    size_t len = 0;
    int day = 0;
    while(getline(&line, &len, stdin) != -1) {
        // Controllo del camioncino
        if(day % arrive_time == 0 && day != 0 && arrive_time != 0) {
            load_lorry(&ready_orders, capacity);
            print_lorry();
        }

        // Cancellazione del carattere '\n'
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len-1] == '\n') {
            line[line_len-1] = '\0';
        }
        manage_line(line, day, &ready_orders, &waiting_orders);

        day++;
    }

    // Controllo dovuto al fatto che si usa un ciclo while
    if(day % arrive_time == 0 && day != 0 && arrive_time != 0) {
        load_lorry(&ready_orders, capacity);
        print_lorry();
    }

    // Pulizia della linea
    free(line);

    // Pulizia del camioncino
    free_lorry();

    return 0;
}