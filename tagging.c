#include "tagging.h"

/*creates a  handler for our tagged tatabase*/
DB createTagger(uint32_t initial_tag_cells, uint32_t initial_node_cells){

    DB toReturn;
    toReturn.max_nodes = initial_node_cells;
    toReturn.max_tags = initial_tag_cells;
    toReturn.current_nodes = 0;
    toReturn.current_tags = 0;
    toReturn.nodes_byte_length = (toReturn.max_nodes / 8) + 1;

    toReturn.byte_matrix = (BYTE*)malloc(toReturn.nodes_byte_length * toReturn.max_tags);
    if (toReturn.byte_matrix == NULL){
        printf("tagger could not be initialized");
        toReturn.error_code = MATRIX_ERROR;
        return toReturn;

    }

    toReturn.node_collection = (NODE*)malloc(sizeof(NODE) * toReturn.max_nodes);
    if(toReturn.node_collection == NULL){
        printf("max nodes could not be initialized");
        toReturn.error_code = COLLECTIONS_ERROR;
        return toReturn;
    }

    toReturn.tag_collection = (TAG*)malloc(sizeof(TAG) * toReturn.max_tags);
    if (toReturn.tag_collection == NULL){
        printf("x");
        toReturn.error_code = COLLECTIONS_ERROR;
        return toReturn;
    }

    toReturn.root_deleted_node = NULL;
    toReturn.root_deleted_tag = NULL;

    toReturn.error_code = NO_ERROR;
    return toReturn;
}

void addTag(DB* thisDB, TAG to_add){
    if(thisDB->root_deleted_tag != NULL){

        //if the root_Deleted_tag in this linked list is not null, then a tag was deleted and it's memory still exists
        //its spot in the tag collection is garbage, and can be re-used
        thisDB->tag_collection[thisDB->root_deleted_tag->index] = to_add;

        struct deleted_tag *temp = thisDB->root_deleted_tag->next;
        free(thisDB->root_deleted_tag);
        thisDB->root_deleted_tag = temp;

        thisDB->current_tags++;

        return;
    }

    if (thisDB->current_tags + 1 <= thisDB->max_tags){
        thisDB->tag_collection[thisDB->current_tags] = to_add;
        thisDB->current_tags++;
        return;
    }
    else{
        printf("max number of tags exceeded");

        thisDB->error_code = MAX_TAGS_REACHED;

        return;
    }


}

void addNode(DB* thisDB, NODE to_add){
    if(thisDB->root_deleted_node != NULL){

        thisDB->node_collection[thisDB->root_deleted_node->index] = to_add;

        struct deleted_node *temp = thisDB->root_deleted_node->next;
        free(thisDB->root_deleted_node);
        thisDB->root_deleted_node = temp;

        thisDB->current_nodes++;

        return;
    }

    if (thisDB->current_nodes + 1 <= thisDB->max_nodes){
        thisDB->node_collection[thisDB->current_nodes] = to_add;
        
        thisDB->current_nodes++;
        
        return;
    }
    else{
        printf("max number of nodes exceeded");
        thisDB->error_code = MAX_NODES_REACHED;
    }
}


uint32_t getTagIndexByName(DB* thisDB, char* subject){


    for (uint32_t i = 0; i < thisDB->current_tags; i++){
        if (strcmp(subject, thisDB->tag_collection[i].name) == 0){
            return i;
        }
    }

    printf("this tag could not be found");
    return;    
}

void deleteTagByName(DB* thisDB, char* subject){
    uint32_t to_delete_index = getTagIndexByName(thisDB, subject);

    //uint32_t node_byte_offset = to_delete_index / 8;
    //uint16_t byte_offset = to_delete_index % 8;

    BYTE* row_start = thisDB->byte_matrix + (thisDB->nodes_byte_length * (to_delete_index));


    /*from our the start of our row, set every byte to 0*/
    for (uint32_t i = 0; i < thisDB->current_nodes + thisDB->num_deleted_nodes; i++){
        *(row_start + i) = 0;
    }

    struct deleted_tag* new_deleted_tag = (struct deleted_tag*)malloc(sizeof(struct deleted_tag));

    new_deleted_tag->index = to_delete_index;
    new_deleted_tag->next = thisDB->root_deleted_tag;
    thisDB->root_deleted_tag = new_deleted_tag;

    thisDB->current_tags--;

    
}

uint32_t getNodeIndexByName(DB* thisDB, char* subject){
    
    for (uint32_t i = 0; i < thisDB->current_nodes; i++){
        if (strcmp(subject, thisDB->tag_collection[i].name) == 0){
            return i;
        }
    }

    printf("this node could not be found");
    return;
}

void deleteNodeByName(DB* thisDB, char* subject){

    uint32_t to_delete_index = getTagIndexByName(thisDB, subject);

    uint32_t node_byte_offset = to_delete_index / 8;
    uint32_t node_bit_offset = to_delete_index % 8;

    BYTE* column_start = thisDB->byte_matrix + node_byte_offset;

    //this cannot be used to leave bytes un-touched, allthough, that is unneeded here.
    BYTE toAND = ~(1 << (7 - node_bit_offset));

    //going down the column, setting all bits assosciated with the deleted tag to 0
    for(uint32_t i = 0; i < thisDB->current_tags + thisDB->num_deleted_tags; i++){

        *(column_start + (thisDB->max_nodes * i)) &= toAND;                

    }


    struct deleted_node * new_deleted_node = (struct deleted_node *)malloc(sizeof(struct deleted_node));

    new_deleted_node->index = to_delete_index;
    new_deleted_node->next = thisDB->root_deleted_node;
    thisDB->root_deleted_node = new_deleted_node;

    thisDB->current_nodes--;

}

