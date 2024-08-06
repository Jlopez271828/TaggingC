/*****************************************************************************************************************************
    This set of functions is designed to allow the user to create a database 
in which "tags" can be assigned to "Nodes". A Matrix of Bytes stores the 
relation, where each bit is a boolean relation. A Node is assigned a column 
of these bits, a tag is assigned a row. The actual Tag and Node objects are 
stored in respective collections, which are arrays of the respective structs.
An objects index in this array can be thought of as it's ID within the array, 
as it is most often used for identification. 



    tagging.h and tagging.c were both written by Jacob Lopez as a personal project
with aims of learning the C language.

********************************************************************************************************************************/


#ifndef TAGGING_H
#define TAGGING_H


#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define GET_BYTE_LENGTH(A, B) ( (A.B / 8) + 1)

/*controlls how many */
#define DEFAULT_SEARCH_BUFFER_CELLS 10
#define SEARCH_BUFFER_INCRIMENT_SIZE 10
#define MAX_NODE_NAME_LENGTH 20
#define MAX_TAG_NAME_LENGTH 20
#define MAX_DB_NAME_LENGTH 20

#define PRIORITIZE_SPACE 1
#define PRIORITIZE_SPEED 2
#define PRIORITIZATION PRIORITIZE_SPACE

#define NO_ERROR 0
#define MATRIX_ERROR 1
#define TAG_COLLECTION_ERROR 2
#define NODE_COLLECTION_ERROR 3
#define MAX_NODES_REACHED 10
#define MAX_TAGS_REACHED 11

typedef unsigned char Byte;

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

struct temp_search_node {
    uint32_t index;
    struct temp_search_node * next;
};


/*struct containing the desired data */
typedef struct {
    char* name;
    //void* data;

} TAG;

#if PRIORITIZATION == PRIORITIZE_SPACE
typedef struct {
    char* name;
    //void* data
} NODE;
#endif

#if PRIORITIZATION == PRIORITIZE_SPEED
struct Node {
    char* name;
    
    struct Node* left_child;
    struct Node* right_child;

    uint32_t index;
};

typedef struct Node NODE;
#endif

typedef struct {
    Byte* byte_matrix; /*byte matrix of booleans, horizontal dimension is nodes, vertical dimension is tags*/

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

    //struct temp_search_node * multi_search_buffer; /*array designed for re-use to avoid overhead in multi-tag searching*/
    
} DB;

/* creates a tagger database struct and return it by value*/
DB createTagger(uint32_t initial_tag_cells, uint32_t initial_node_cells);

/* initializes a tag from the memory allocated in the DB struct */
void addTag(DB* thisDB, TAG to_add);

/* initialized a node from the memory allocated in the DB struct */
void addNode(DB* thisDB, NODE to_add);

/*attaches a tag to a created node in the DB*/
void attachTag(DB* thisDB, uint32_t tag_index, uint32_t node_index);

/*detaches a tag from a created node in the DB*/
void detachTag(DB* thisDB, uint32_t tag_index, uint32_t node_index);

/* searches the tag collection structure in the DB parameter to find the selected tags index that corrosponds to a matrix vertical location*/
uint32_t getTagIndexByName(DB* thisDB, char* subject);

/* Dissascociates the tag corrosponding to the index from DB. Adds a deleted_tag struct to DB. Incriments deleted counter*/
void deleteTagByIndex(DB* thisDB, uint32_t to_delete_index);

/* Searches the DB for the named tag and then dissascociates it from its row in the matrix, creates a deleted_tag struct to add to DB*/
void deleteTagByName(DB* thisDB, char* subject);

/* Dissascociates the node corrosponding to the index from DB. Adds a deleted_node struct to DB. Incriments deleted counter */
void deleteNodeByIndex(DB* thisDB, uint32_t to_delete_index);

/*searches the collection array in the DB to find the selected tag's index that corrosponds to a matrix horizontal location*/
uint32_t getNodeIndexByName(DB* thisDB, char* subject);

/*searches for a node with the desired name, then dissascociates it from its column in the matrix, creates a deleted_node struct to add to DB*/
void deleteNodeByName(DB* thisDB, char* subject);

/*takes a node index, finds all of its assoscated tags, to which it stores ther indexes in array hit_indexes with size num_hits.
    WARNING: hit_indexes is a malloced array that will not be automatically freed.

*/
void getAllTagsOfNode(DB* thisDB, uint32_t node_index, uint32_t* num_hits, uint32_t** hit_indexes);

/*takes a tag index, fings all of its assosciated nodes, to which it stores their indexes in array hit_indexes with size num_hits
    WARNING: hit_indexes is a malloced array that will not be automatically freed.

*/
void getAllNodesWithTag(DB* thisDB, uint32_t tag_index, uint32_t* num_hits, uint32_t** hit_indexes);

/*takes an array of multiple tag indexes, finds all nodes that have all the tags*/
void findAllNodesWithMultipleTags(DB* thisDB, uint32_t* tag_index_array, uint32_t num_tags, uint32_t* number_hit_nodes, uint32_t** hit_indexes);

/*prints the names of all tags attached to a node, one every line, each tag preceded by a tab*/
void printAllTagsNamesOfNode(DB* thisDB, uint32_t to_find_index);

/*prints the indexes of all tags attached to a node, one every line, each tag preceded by a tab*/
void printAllTagsIndexesOfNode(DB* thisDB, uint32_t to_find_index);

/*prints the names of all nodes that have this tag, one every line, each node preceded by a tab*/
void printAllNodesNamesWithTag(DB* thisDB, uint32_t to_find_index);

/*prints the indexes of all nodes that have this tag, one every line, each node preceded by a tab*/
void printAllNodesIndexesWithTag(DB* thisDB, uint32_t to_find_index);

void printHexMatrix(DB* thisDB);

void printBinaryMatrix(DB* thisDB);

void serializeDB(DB* thisDB, char* path);

void freeDB(DB* thisDB);






#endif