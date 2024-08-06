#include "tagging.h"

static Byte single_bits[8] ={
    0x01, 
    0x02,
    0x04,
    0x08,
    0x10,
    0x20,
    0x40,
    0x80
};


/*creates a  handler for our tagged tatabase*/
DB createTagger(uint32_t initial_tag_cells, uint32_t initial_node_cells){

    DB toReturn;
    toReturn.max_nodes = initial_node_cells;
    toReturn.max_tags = initial_tag_cells;
    toReturn.current_nodes = 0;
    toReturn.current_tags = 0;
    toReturn.nodes_byte_length = (toReturn.max_nodes / 8) + 1;
    toReturn.num_deleted_tags = 0;
    toReturn.num_deleted_nodes = 0;

    toReturn.byte_matrix = (Byte*)malloc(toReturn.nodes_byte_length * toReturn.max_tags);
    if (toReturn.byte_matrix == NULL){
        printf("tagger could not be initialized\n");
        toReturn.error_code = MATRIX_ERROR;
        return toReturn;

    }

    toReturn.node_collection = (NODE*)malloc(sizeof(NODE) * toReturn.max_nodes);
    if(toReturn.node_collection == NULL){
        printf("max nodes could not be initialized\n");
        toReturn.error_code = NODE_COLLECTION_ERROR;
        return toReturn;
    }

    toReturn.tag_collection = (TAG*)malloc(sizeof(TAG) * toReturn.max_tags);
    if (toReturn.tag_collection == NULL){
        printf("x\n");
        toReturn.error_code = TAG_COLLECTION_ERROR;
        return toReturn;
    }

    toReturn.root_deleted_node = NULL;
    toReturn.root_deleted_tag = NULL;

    //toReturn.multi_search_buffer = (struct temp_search_node*)malloc(sizeof(struct temp_search_node) * toReturn.max_nodes);

    toReturn.error_code = NO_ERROR;
    return toReturn;
}

void addTag(DB* thisDB, TAG to_add){

    if (to_add.name == NULL){
        printf("this tag is malformed\n");
        return;
    }

    if(thisDB->root_deleted_tag != NULL){

        //if the root_Deleted_tag in this linked list is not null, then a tag was deleted and it's memory still exists
        //its spot in the tag collection is garbage, and can be re-used
        thisDB->tag_collection[thisDB->root_deleted_tag->index] = to_add;

        struct deleted_tag *temp = thisDB->root_deleted_tag->next;
        free(thisDB->root_deleted_tag);
        thisDB->root_deleted_tag = temp;

        //printf("TAG %s ADDED\n\n", to_add.name);

        thisDB->current_tags++;
        

        return;
    }

    if (thisDB->current_tags + 1 <= thisDB->max_tags){
        thisDB->tag_collection[thisDB->current_tags] = to_add;

        //printf("TAG %s ADDED\n\n", to_add.name);

        thisDB->current_tags++;
        return;
    }
    else{
        printf("max number of tags exceeded\n");

        thisDB->error_code = MAX_TAGS_REACHED;

        return;
    }

    //printf("TAG %s ADDED\n\n", to_add.name);

}

void addNode(DB* thisDB, NODE to_add){
    if(thisDB->root_deleted_node != NULL){

        thisDB->node_collection[thisDB->root_deleted_node->index] = to_add;

        struct deleted_node *temp = thisDB->root_deleted_node->next;
        free(thisDB->root_deleted_node);
        thisDB->root_deleted_node = temp;

        thisDB->current_nodes++;
        //printf("NODE %s ADDED\n\n", to_add.name);

        return;
    }

    if (thisDB->current_nodes + 1 <= thisDB->max_nodes){
        thisDB->node_collection[thisDB->current_nodes] = to_add;
        
        thisDB->current_nodes++;
        //printf("NODE %s ADDED\n\n", to_add.name);
        
        return;
    }
    else{
        printf("max number of nodes exceeded\n");
        thisDB->error_code = MAX_NODES_REACHED;
        return;
    }

    
}

void attachTag(DB* thisDB, uint32_t tag_index, uint32_t node_index){

    Byte* matrix_cursor = thisDB->byte_matrix + (thisDB->nodes_byte_length * tag_index);

    uint32_t byte_offset = node_index / 8;

    uint32_t bit_offset = node_index % 8;

    matrix_cursor += byte_offset;

    (*matrix_cursor) |= (1u << (7u - bit_offset));
    


}

void detachTag(DB* thisDB, uint32_t tag_index, uint32_t node_index){

    Byte* matrix_cursor = thisDB->byte_matrix + (thisDB->nodes_byte_length * tag_index);

    uint32_t byte_offset = node_index / 8;

    uint32_t bit_offset = node_index % 8;

    matrix_cursor += byte_offset;

    (*matrix_cursor) &= ~(1u << (7u - bit_offset));

}


uint32_t getTagIndexByName(DB* thisDB, char* subject){


    for (uint32_t i = 0; i < thisDB->current_tags; i++){
        if (strcmp(subject, thisDB->tag_collection[i].name) == 0){
            //printf("found tag %s at index %d\n\n", subject, i);
            return i;
        }
    }

    printf("this tag could not be found\n");
    return;    
}

uint32_t getNodeIndexByName(DB* thisDB, char* subject){
    
    //printf("GNIBN marker 0\n");

    for (uint32_t i = 0; i < thisDB->current_nodes; i++){
        if (strcmp(subject, thisDB->node_collection[i].name) == 0){
            //printf("found node %s at index %d\n\n", subject, i);
            return i;
        }
    }

    //printf("GNIBN marker 1");

    printf("this node could not be found\n");
    return;
}

void deleteTagByName(DB* thisDB, char* subject){
    uint32_t to_delete_index = getTagIndexByName(thisDB, subject);

    //uint32_t node_byte_offset = to_delete_index / 8;
    //uint16_t byte_offset = to_delete_index % 8;

    deleteTagByIndex(thisDB, to_delete_index);


    

    
}

void deleteTagByIndex(DB* thisDB, uint32_t to_delete_index){

    Byte* row_start = thisDB->byte_matrix + (thisDB->nodes_byte_length * (to_delete_index));

    for (uint32_t i = 0; i < thisDB->current_nodes + thisDB->num_deleted_nodes; i++){
        *(row_start + i) = 0;
    }

    struct deleted_tag* new_deleted_tag = (struct deleted_tag*)malloc(sizeof(struct deleted_tag));

    new_deleted_tag->index = to_delete_index;
    new_deleted_tag->next = thisDB->root_deleted_tag;
    thisDB->root_deleted_tag = new_deleted_tag;

    thisDB->current_tags--;
    thisDB->num_deleted_tags++;
}

void deleteNodeByIndex(DB* thisDB, uint32_t to_delete_index){

    uint32_t node_byte_offset = to_delete_index / 8;
    uint32_t node_bit_offset = to_delete_index % 8;

    Byte* column_start = thisDB->byte_matrix + node_byte_offset;

    //this cannot be used to leave bytes un-touched, allthough, that is unneeded here.
    Byte toAND = ~(1 << (7 - node_bit_offset));

    //going down the column, setting all bits assosciated with the deleted tag to 0
    for(uint32_t i = 0; i < thisDB->current_tags + thisDB->num_deleted_tags; i++){

        *(column_start + (thisDB->max_nodes * i)) &= toAND;                

    }


    struct deleted_node * new_deleted_node = (struct deleted_node *)malloc(sizeof(struct deleted_node));

    new_deleted_node->index = to_delete_index;
    new_deleted_node->next = thisDB->root_deleted_node;
    thisDB->root_deleted_node = new_deleted_node;

    thisDB->current_nodes--;
    thisDB->num_deleted_nodes++;

}

void deleteNodeByName(DB* thisDB, char* subject){

    uint32_t to_delete_index = getTagIndexByName(thisDB, subject);

    deleteTagByIndex(thisDB, to_delete_index);

}

void getAllTagsOfNode(DB* thisDB, uint32_t node_index, uint32_t* number_hit_tags, uint32_t** hit_indexes){
    
    //printf("GATON MARKER WTF\n");

    Byte* matrix_cursor = thisDB->byte_matrix + (node_index / 8);

    //printf("GATON marker 0\n");

    *hit_indexes = (uint32_t*)malloc(sizeof(uint32_t) * (thisDB->current_tags + thisDB->num_deleted_tags));

    if(*hit_indexes == NULL){
        printf("error in hit_indexes\n");
        return;
    }

    //printf("GATON marker 1\n");

    *number_hit_tags = 0;

    uint16_t row_length = thisDB->nodes_byte_length;

    uint16_t bit_offset = node_index % 8;

    Byte mask = 1u << (7u - bit_offset);

    //printf("bound: %d", thisDB->current_tags + thisDB->num_deleted_tags);
    for (uint32_t i = 0; i < thisDB->current_tags + thisDB->num_deleted_tags; i++){
        if ( (*(matrix_cursor + i * (row_length)) & mask)){
            //printf("\thit, %x, loop number %d\n", (*(matrix_cursor + i * (row_length)) & mask), i);
            (*hit_indexes)[*number_hit_tags] = i;
            (*number_hit_tags)++;
        }
    }
    //printf("GATON marker 2\n");

    realloc(*hit_indexes, (*number_hit_tags) * sizeof(uint32_t));
    //printf("getAllTagsOfNode realloc has worked\n");

}

void getAllNodesWithTag(DB* thisDB, uint32_t tag_index, uint32_t* number_hit_nodes, uint32_t** hit_indexes){

    Byte* matrix_cursor = thisDB->byte_matrix + (tag_index * thisDB->nodes_byte_length);

    *hit_indexes = (uint32_t*)malloc(sizeof(uint32_t) * (thisDB->current_nodes));

    if(*hit_indexes == NULL){
        printf("error in hit_indexes\n");
        return;
    }

    *number_hit_nodes = 0;

    //printf("marker1\n");
    
    for(uint32_t i = 0; i < (thisDB->current_nodes + thisDB->num_deleted_nodes); i++){


        if ( ((*matrix_cursor + (i / 8)) << (Byte)(i % 8)) & 0x80 ){
            (*hit_indexes)[*number_hit_nodes] = i;
            (*number_hit_nodes)++;
        }


        
    }
    //printf("marker2, %d\n", (*number_hit_nodes));

    
    hit_indexes = realloc(*hit_indexes, (*number_hit_nodes) * sizeof(uint32_t));
    //printf("getAllNodesWithTag realloc worked\n");

}

void findAllNodesWithMultipleTags(DB* thisDB, uint32_t* tag_index_array, uint32_t num_tags, uint32_t* number_hit_nodes, uint32_t** hit_indexes){

    if (num_tags <= 0){
        return;
    }

    Byte* matrix_cursor = thisDB->byte_matrix;

    *number_hit_nodes = 0;

    matrix_cursor += (thisDB->nodes_byte_length * tag_index_array[0]);

    //maybe I should just have this memory as eternally ours in the DB struct
    struct temp_search_node* temp_found_collection = (struct temp_search_node*)malloc(sizeof(struct temp_search_node) * (thisDB->current_nodes));

    //Building a temporary linked list to contain the indexes of the nodes we have found

    struct temp_search_node* prev = NULL;

    for(uint32_t i = 0; i < (thisDB->current_nodes + thisDB->num_deleted_nodes); i++){

        if( ((*(matrix_cursor + (i / 8))) << (i % 8)) &  0x80){
            //printf("found node %s at index %d\n", thisDB->node_collection[i].name, i);

            temp_found_collection[*number_hit_nodes].index = i;
            temp_found_collection[*number_hit_nodes].next = NULL;

            if(prev != NULL){//prev does exist
                prev->next = &temp_found_collection[*number_hit_nodes];
                prev = &temp_found_collection[*number_hit_nodes];
            }
            else{//prev doesn't exist
                
                prev = &temp_found_collection[*number_hit_nodes];
                
            }

            (*number_hit_nodes)++;
        }
        else{
            //printf("did not find node %s at index %d\n", thisDB->node_collection[i].name, i);
        }


        
    }

    
    // printf("nodes initially found with just %s:\n", thisDB->tag_collection[tag_index_array[0]].name);
    // for (int i = 0; i < *number_hit_nodes; i++){
    //     printf("\t%s\n", thisDB->node_collection[temp_found_collection[i].index].name);
    // }


    // struct temp_search_node* temp_debug = &temp_found_collection[0];
    // printf("our ll:\n");
    // for(int i = 0; i < *number_hit_nodes; i++){
    //     if(temp_debug == NULL){
    //         printf(" N \n");
    //     }
    //     else{
    //         printf(" %s -> \0", thisDB->node_collection[temp_debug->index].name);
    //         temp_debug = temp_debug->next;
    //     }
        
    // }
    // printf("\n");
    

    //pruning off un-needed memory
    //realloc(temp_found_collection, *number_hit_nodes * sizeof(struct temp_search_node));
    //printf("pruning temp_found_collection has worked\n");

    struct temp_search_node* root = &temp_found_collection[0];

    struct temp_search_node* ll_cursor = root;
    struct temp_search_node* last_success = NULL;

    uint32_t ll_bound = *number_hit_nodes;
    //printf("\tLL_BOUND: %u\n", ll_bound);

    //looping once for each remaining tag, starting at 1 since the first tag has already been searched
    for(uint32_t i = 1; i < num_tags; i++){
        matrix_cursor = thisDB->byte_matrix + (thisDB->nodes_byte_length * tag_index_array[i]);

        //printf("looking for %s\n", thisDB->tag_collection[tag_index_array[i]].name);

        //looping through our linked list of hit nodes
        for(uint32_t k = 0; k < ll_bound; k++){
            
            //if one of our nodes is found to not have the next tag we are searching for
            if( !( (*(matrix_cursor + (ll_cursor->index / 8)) << (ll_cursor->index % 8) ) & 0x80 )  ){

                // printf("this byte at index %d: %x\n", ll_cursor->index, *(matrix_cursor + (ll_cursor->index / 8)));
                // printf("this bit at place %d: %u\n", ll_cursor->index % 8, (*(matrix_cursor + (ll_cursor->index / 8)) << (ll_cursor->index % 8) ) & 0x80 );

                // printf("\t%s not found, trimming\n", thisDB->node_collection[ll_cursor->index].name);

                (*number_hit_nodes)--;

                if(last_success == NULL){
                    root = ll_cursor->next;
                }
                else{
                    last_success->next = ll_cursor->next;
                }
            }
            else{
                last_success = ll_cursor;
            }

            // if(ll_cursor->next == NULL){
            //     printf("LL_CUSOR next is null\n");
            //     printf("\t k = %d\n", k);
            // }
            // else{
            //     printf("ll_cursor next is %s\n", thisDB->node_collection[ll_cursor->next->index].name);
            // }

            ll_cursor = ll_cursor->next;
            

        }

        ll_bound = *number_hit_nodes;

        last_success = NULL;

        ll_cursor = root;
    }

    if(!(*number_hit_nodes)){
        return;
    }

    *hit_indexes = (uint32_t*)malloc(sizeof(uint32_t) * (*number_hit_nodes));

    if(*hit_indexes == NULL){
        printf("error in searching\n");
        return;
    }

    for(uint32_t i = 0; i < *number_hit_nodes; i++){

        (*hit_indexes)[i] = ll_cursor->index;
        ll_cursor = ll_cursor->next;

    }

    return;


}

void printAllTagsNamesOfNode(DB* thisDB, uint32_t to_find_index){

    uint32_t num_found_tags = 0;

    uint32_t* found_tag_indexes = NULL;
    
    //printf("PATNOF marker 0 :: %d\n", to_find_index);

    //printf("node we are looking for: %s\n", thisDB->node_collection[to_find_index].name);
    
    getAllTagsOfNode(thisDB, to_find_index, &num_found_tags, &found_tag_indexes);
    //printf("PATNOF marker 1\n");

    //printf("\tfound %d tags attached to this node\n", num_found_tags);

    for(uint32_t i = 0; i < num_found_tags; i++){
        printf("\t%s,\n", thisDB->tag_collection[found_tag_indexes[i]].name);
    }

    free(found_tag_indexes);


}


void printAllTagsIndexesOfNode(DB* thisDB, uint32_t to_find_index){

    uint32_t num_found_tags = 0;

    uint32_t* found_tag_indexes = NULL;

    getAllTagsOfNode(thisDB, to_find_index, &num_found_tags, &found_tag_indexes);


    for(uint32_t i = 0; i < num_found_tags; i++){
        printf("\t%u\n", found_tag_indexes[i]);
    }

    free(found_tag_indexes);
}

void printAllNodesNamesWithTag(DB* thisDB, uint32_t to_find_index){

    uint32_t num_found_nodes = 0;

    uint32_t* found_node_indexes = NULL;

    getAllNodesWithTag(thisDB, to_find_index, &num_found_nodes, &found_node_indexes);

    //printf("PANNWT marker 1\n");

    for(uint32_t i = 0; i < num_found_nodes; i++){
        printf("\t%s\n", thisDB->node_collection[found_node_indexes[i]].name);
    }

    free(found_node_indexes);
}

void printAllNodesIndexesWithTag(DB* thisDB, uint32_t to_find_index){

    uint32_t num_found_nodes = 0;

    uint32_t* found_node_indexes = NULL;

    getAllNodesWithTag(thisDB, to_find_index, &num_found_nodes, &found_node_indexes);

    for(uint32_t i = 0; i < num_found_nodes; i++){
        printf("\t%u\n", found_node_indexes[i]);
    }

    free(found_node_indexes);
}

void printHexMatrix(DB* thisDB){

    Byte* matrix_cursor = thisDB->byte_matrix;

    for (uint32_t i = 0; i < thisDB->current_tags; i++){
        for (uint32_t k = 0; k < thisDB->nodes_byte_length; k++){
            printf("%2x ", *matrix_cursor);
            matrix_cursor++;
        }
        printf("\n");
    }
}

void printBinaryMatrix(DB* thisDB){
    Byte* matrix_cursor = thisDB->byte_matrix;
    char temp[9];
    temp[8] = '\0';

    for(uint32_t i = 0; i < thisDB->current_nodes; i++){
        printf("%s, ", thisDB->node_collection[i].name);
    }
    printf("\n");

    for (uint32_t i = 0; i < thisDB->current_tags; i++){
        printf("%10s:   ", thisDB->tag_collection[i].name);
        for (uint32_t k = 0; k < thisDB->nodes_byte_length; k++){
            for(uint32_t h = 0; h < 8; h++){
                if( (*matrix_cursor) & (1u << (7 - h))){
                    temp[h] = '1';
                }
                else{
                    temp[h] = '0';
                }
            }

            printf("%s ", temp);
            matrix_cursor++;
        }
        printf("\n");
    }
}

/*
void saveDB(DB* thisDB, char* path){

    FILE* f = fopen(path, "wb");
    if(f == NULL){
        printf("error in saving\n");
        return;
    }

    fseek(f, 10, SEEK_SET);
    fwrite(thisDB->name, strlen(thisDB->name), 1, f);
    fseek(f, MAX_DB_NAME_LENGTH, SEEK_CUR);
    fwrite(thisDB->current_nodes, sizeof(thisDB->current_nodes), 1, f);


    fclose(f);

}*/



void freeDB(DB* thisDB){

    free(thisDB->byte_matrix);

    struct deleted_node* temp_node = thisDB->root_deleted_node;
    struct deleted_node* next_node = NULL;
    while(temp_node != NULL){
        next_node = temp_node->next;
        free(temp_node);
        temp_node = next_node;
    }
    thisDB->root_deleted_node = NULL;

    struct deleted_tag* temp_tag = thisDB->root_deleted_tag;
    struct deleted_tag* next_tag = NULL;
    while(temp_tag != NULL){
        next_tag = temp_tag->next;
        free(temp_tag);
        temp_tag = next_tag;
    }
    thisDB->root_deleted_tag = NULL;

    free(thisDB->tag_collection);

    free(thisDB->node_collection);

    return;

    

}

/*TODO: make sure that when a tag or node is added, all of its respective memory in the matrix is 0'd out
        make all the literals in the bit shifts unsigned with u
        check all the loop bounds
        write to stderr when weird things happen


*/
        



