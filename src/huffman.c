#include "huffman.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/* Création structure huff_table */
struct huff_table
{
    /* Paramètres d'entrée à garder */
    uint8_t *nb_symb_per_lengths;
    uint8_t *symbols;
    /* Pointeur vers l'arbre */
    struct node *root;
};


/* Création structure noeud */
struct node
{
    /* Pointeur vers valeur du noeud : le symbole */
    uint8_t *symbol;
    /* Pointeur vers les noeuds de droite et gauche */
    struct node *right_branch;
    struct node *left_branch;
};


/* Crée un nouveau noeud vide */
struct node new_node(){
    struct node new_node;
    new_node.right_branch = NULL;
    new_node.left_branch = NULL;
    new_node.symbol = NULL;
    return new_node;
}


/* Renvoie la chaîne de caractère contenant le chemin dans l'arbre
pour écrire un entier, permet de parcourir les branches par ordre croissant */
char *bin(int valeur, int magn){
    char *indice = (char *)malloc((magn+1)*sizeof(char));
    for (int i = 0; i < magn; i++) {
        if(valeur % 2 == 1){
            indice[magn-i-1] = '1';
        }
        else{
            indice[magn-i-1] = '0';
        }
    valeur = valeur / 2;
    }
    return indice;
}


/* Insère les symboles sur une même ligne et crée les noeuds vide de la prochaine ligne */
/* Dans le cas où length = 16, insère seulement les symboles */
void insert_symbols(uint8_t to_insert, uint8_t length, uint8_t inserted_symbols, uint8_t *symbols, struct node *root_node, uint16_t debut, uint16_t fin){
    uint8_t num_symbole = to_insert;
    /* On parcourt les noeuds de début à fin = 2^length -1 */
    for (uint32_t num_path = debut; num_path < (uint32_t)fin + 1; num_path++) {
        /* Le chemin dans l'arbre */
        char *path = bin(num_path, length);
        /* On va en bas de l'arbre */
        struct node *current = root_node;
        for (uint8_t i = 0; i < length; i++){
            if (path[i] == '0') {
                current = current->left_branch;
            }
            else{
                current = current->right_branch;
            }
        }
        /* Si on n'a plus de symbole à insérer */
        if ((to_insert == 0) & (length != 16) ) {
            /* On crée les noeuds droit et gauche */
            struct node *right_node = (struct node *)malloc(sizeof(struct node));
            *right_node = new_node();
            struct node *left_node = (struct node *)malloc(sizeof(struct node));
            *left_node = new_node();
            current->right_branch = right_node;
            current->left_branch = left_node;
        }
        /* Si on a des symboles à insérer */
        else{
            /* On a trouvé un chemin, on sait qu'il n'a pas de symbole
            car longueur différente d'avant */
            current->symbol = &symbols[inserted_symbols + num_symbole - to_insert];
            to_insert--;
        }
        free(path);
    }
}

/*
    Construit un arbre de Huffman à partir d'une table
    de symboles comme présenté en section 2.10.1 du sujet.
    nb_symb_per_lengths est un tableau contenant le nombre
    de symboles pour chaque longueur de 1 à 16,
    symbols est le tableau  des symboles ordonnés,
    et nb_symbols représente la taille du tableau symbols.
*/
struct huff_table *huffman_table_build(uint8_t *nb_symb_per_lengths,
                                       uint8_t *symbols,
                                       uint8_t nb_symbols){
    /* Initialisation table*/
    struct huff_table *table = malloc(sizeof(struct huff_table));
    table->nb_symb_per_lengths = nb_symb_per_lengths;
    table->symbols = symbols;
    struct node *root_node = malloc(sizeof(struct node));
    *root_node = new_node();
    table->root = root_node;
    /* Premier niveau */
    struct node *right_node = malloc(sizeof(struct node));
    *right_node = new_node();
    struct node *left_node = malloc(sizeof(struct node));
    *left_node = new_node();
    table->root->right_branch = right_node;
    table->root->left_branch = left_node;
    /* Initialisation paramètres de l'arbre */
    uint8_t length = 1;
    uint8_t inserted_symbols = 0;
    uint16_t debut = 0;
    uint16_t fin = 1;
    /* Remplissage de l'arbre */
    /* Tant qu'on a des symboles à insérer */
    while (inserted_symbols != nb_symbols){
        uint8_t to_insert = nb_symb_per_lengths[length - 1];
        insert_symbols(to_insert, length, inserted_symbols, symbols, root_node, debut, fin);
        /* Mise à jour des paramètres */
        inserted_symbols = inserted_symbols + nb_symb_per_lengths[length-1];
        /* debut = debut + (to_insert)^(length)*/
        debut = 2*(debut + nb_symb_per_lengths[length - 1]);
        fin = (fin+1)*2 - 1;
        length++;
    }
    return table;
}

/* Fonction récursive qui trouve le chemin du symbole dans l'arbre */
void path(struct node *node, uint32_t chemin, uint8_t bit,
					uint8_t value,
					uint8_t *nb_bits,
					uint32_t *res){
		if (node->symbol != NULL) {
				uint8_t symb = *node->symbol;
				if (symb == value) {
						res[0] = chemin;
						*nb_bits = bit;
				}
		}
		if (node->left_branch != NULL) {
				path(node->left_branch, chemin * 2, bit+1, value, nb_bits, res);
		}
		if (node->right_branch != NULL) {
				path(node->right_branch, chemin * 2 + 1, bit+1, value, nb_bits, res);
		}
}

/*
    Retourne le chemin dans l'arbre ht permettant d'atteindre
    la feuille de valeur value. nb_bits est un paramètre de sortie
    permettant de stocker la longueur du chemin retourné.
    Retourne -1 si pas de chemin.
*/
uint32_t huffman_table_get_path(struct huff_table *ht,
                                uint8_t value,
                                uint8_t *nb_bits){
    if (value == 0){
      *nb_bits = 2;
      return 0;
    }
		uint32_t res[1] = {-1};
    struct node *root_node = ht->root;
    path(root_node, 0, 0, value, nb_bits, res);
    return res[0];
}



/*
    Retourne le tableau des symboles associé à l'arbre de
    Huffman passé en paramètre.
*/
uint8_t *huffman_table_get_symbols(struct huff_table *ht){
    return ht->symbols;
}



/*
    Retourne le tableau du nombre de symboles de chaque longueur
    associé à l'arbre de Huffman passé en paramètre.
*/
uint8_t *huffman_table_get_length_vector(struct huff_table *ht){
    return ht->nb_symb_per_lengths;
}




/* Fonction récursive qui free les noeuds */
void destroy(struct node *node){
		if (node->left_branch != NULL) {
				destroy(node->left_branch);
		}
		if (node->right_branch != NULL) {
				destroy(node->right_branch);
		}
		free(node);
}



/*
    Détruit l'arbre de Huffman passé en paramètre et libère
    toute la mémoire qui lui est associée.
*/
void huffman_table_destroy(struct huff_table *ht){
  destroy(ht->root);

  free(ht);
}
