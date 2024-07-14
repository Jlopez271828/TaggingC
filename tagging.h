#ifndef TAGGING_H
#define TAGGING_H


#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define GET_BYTE_LENGTH(A, B) ( (A.B / 8) + 1)

#define NO_ERROR 0
#define MATRIX_ERROR 1
#define COLLECTIONS_ERROR 2
#define MAX_NODES_REACHED 10
#define MAX_TAGS_REACHED 10

typedef unsigned char BYTE;

/*linked list node that contains the corrosponding index of a deleted node*/
struct deleted_node {
    uint32_t index;
    struct deleted_node* next;
};

/*linked list node that contains the corrosponding index of a deleted tag*/
struct deleted_tag {
    uint32_t index;
    struct deleted_tag* next;
};

/*struct containing the desired data */
typedef struct tag{
    char* name;
    //void* data;

} TAG;

typedef struct node {
    char* name;
    //void* data
} NODE;

typedef struct db{
    BYTE* byte_matrix; /*byte matrix of booleans, horizontal dimension is nodes, vertical dimension is tags*/

    uint32_t current_tags; /*the number of tags currently in use*/
    uint32_t max_tags; /*the number of tags this database can currently handle, may change*/
    uint32_t current_nodes; /*the number of nodes currently in use*/
    uint32_t max_nodes; /*the number of nodes this dadabase can currently handle, may change*/
    uint32_t nodes_byte_length; /*the number of bytes taken to store the information in one row of our byte matrix*/

    uint32_t num_deleted_tags; /*the number of tags that have been deleted and thus are empty, restored tags do not count as deleted*/
    uint32_t num_deleted_nodes; /*the number of nodes that have been deleted and are now empty*/

    struct deleted_node * root_deleted_node; /*root of a linked list structure to store date on deleted nodes*/

    struct deleted_tag * root_deleted_tag; /*root of a linked list structure to store data on deleted tags*/

    char* name; /*this database's name*/

    TAG* tag_collection; /*array that contains a tag struct for every tag our database can potentially handle, may not all be in use*/
    NODE* node_collection; /*array that contains a tag struct for every tag our database can handle, may not all be in use*/

    uint16_t error_code; /*holder for misc error codes generated*/


    
} DB;


DB create_tagger();




#endif