#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>


#define MAX_COMMAND_LENGTH 20

/*
    Funzione che implementa la lettura di aggiungi_ricetta
*/
void manage_aggiungi_ricetta(char* line) {

}

/*
    Funzione che implementa la lettura di rimuovi_ricetta
*/
void manage_aggiungi_ricetta(char* line) {

}

/*
    Funzione che implementa la lettura di ordine
*/
void manage_aggiungi_ricetta(char* line) {

}

/*
    Funzione che implementa la lettura di rifornimento
*/
void manage_aggiungi_ricetta(char* line) {

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