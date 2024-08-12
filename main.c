#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>


#define MAX_COMMAND_LENGTH 20

#define MAX_INGREDIENT_NAME 255
#define MAX_RECIPE_NAME 255

#define RP_TABLE_SIZE 20

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
    Funzione che inizializza la tabella di hash
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
void manage_ordine(char* line) {

}

/*
    Funzione che implementa la lettura di rifornimento
*/
void manage_rifornimento(char* line) {

}

/*
    Funzione che dato in input una stringa gestisce il tipo di comando assegnato mediante input stdin
*/
void manage_line(char* line) {
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
        manage_ordine(line);
    } else if (strcmp(command, "rifornimento") == 0) {
        manage_rifornimento(line);
    }
    return;
}

int main() {
    // inizializzazione delle strutture
    init_hash_table_recipe();


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
    while(getline(&line, &len, stdin) != -1){
        // cancellazione del carattere '\n'
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len-1] == '\n') {
            line[line_len-1] = '\0';
        }
        manage_line(line);
    }
    free(line);

    return 0;
}