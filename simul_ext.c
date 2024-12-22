#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "headers.h"

#define COMMAND_LENGTH 100

/* 
   Global variable to control the language (0 -> English, 1 -> Spanish).
   Default: 0 (English).
*/
static int language = 0;

/* 
   Function Prototypes 
*/
int ParseCommand(char *input_command, char *command, char *arg1, char *arg2);
void WriteDataBlocks(EXT_DATOS *data_blocks, FILE *partition_file);

int main() {
    char input_command[COMMAND_LENGTH];
    char command[COMMAND_LENGTH];
    char arg1[COMMAND_LENGTH];
    char arg2[COMMAND_LENGTH];

    EXT_SIMPLE_SUPERBLOCK superblock;
    EXT_BYTE_MAPS byte_maps;
    EXT_BLQ_INODOS inodes;
    EXT_ENTRADA_DIR directory[MAX_FICHEROS];
    EXT_DATOS data_blocks[MAX_BLOQUES_DATOS];
    EXT_DATOS all_blocks[MAX_BLOQUES_PARTICION];
    
    FILE *partition_file;

    /* Open the partition file in read and write binary mode */
    partition_file = fopen("particion.bin", "r+b");
    if (partition_file == NULL) {
        printf("ERROR: Unable to open 'particion.bin'\n");
        return -1;
    }

    /* Read all blocks from the partition */
    if (fread(&all_blocks, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, partition_file) != MAX_BLOQUES_PARTICION) {
        printf("ERROR: Unable to read the partition correctly.\n");
        fclose(partition_file);
        return -1;
    }
    
    /* Copy data into respective structures */
    memcpy(&superblock, (EXT_SIMPLE_SUPERBLOCK *)&all_blocks[0], SIZE_BLOQUE);
    memcpy(&byte_maps, (EXT_BYTE_MAPS *)&all_blocks[1], SIZE_BLOQUE);
    memcpy(&inodes, (EXT_BLQ_INODOS *)&all_blocks[2], SIZE_BLOQUE);
    memcpy(&directory, (EXT_ENTRADA_DIR *)&all_blocks[3], SIZE_BLOQUE);
    memcpy(&data_blocks, (EXT_DATOS *)&all_blocks[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    /* Infinite loop to process commands */
    while (1) {
        printf(">> ");
        fflush(stdout);

        if (fgets(input_command, COMMAND_LENGTH, stdin) == NULL) {
            /* If EOF or read error, exit the loop */
            break;
        }

        /* Parse the command */
        if (ParseCommand(input_command, command, arg1, arg2) != 0) {
            /* If parsing fails, continue to next iteration */
            continue;
        }

        /* Handle the 'info' command */
        if (strcmp(command, "info") == 0) {
            if (language == 0) {
                /* English */
                printf("Block %u Bytes\n", superblock.s_block_size);
                printf("Inodes in partition = %u\n", superblock.s_inodes_count);
                printf("Free inodes = %u\n", superblock.s_free_inodes_count);
                printf("Blocks in partition = %u\n", superblock.s_blocks_count);
                printf("Free blocks = %u\n", superblock.s_free_blocks_count);
                printf("First data block = %u\n", superblock.s_first_data_block);
            } else {
                /* Spanish */
                printf("Bloque %u Bytes\n", superblock.s_block_size);
                printf("Inodos de la particion = %u\n", superblock.s_inodes_count);
                printf("Inodos libres = %u\n", superblock.s_free_inodes_count);
                printf("Bloques de la particion = %u\n", superblock.s_blocks_count);
                printf("Bloques libres = %u\n", superblock.s_free_blocks_count);
                printf("Primer bloque de datos = %u\n", superblock.s_first_data_block);
            }
            continue;
        }
        /* Handle the 'salir' or 'exit' command */
        if (strcmp(command, "salir") == 0 || strcmp(command, "exit") == 0) {
            /* Before exiting, write all data blocks to disk */
            WriteDataBlocks(data_blocks, partition_file);
            fclose(partition_file);
            if (language == 0) {
                printf("Exiting...\n");
            } else {
                printf("Saliendo...\n");
            }
            return 0;
        }
        /* Handle unknown commands */
        else {
            if (language == 0) {
                printf("ERROR: Unknown command.\n");
            } else {
                printf("ERROR: Comando desconocido.\n");
            }
        }
    }

    /* If the loop exits unexpectedly, write data and close the file */
    WriteDataBlocks(data_blocks, partition_file);
    fclose(partition_file);
    return 0;
}

/* 
 * Function: ParseCommand
 * ----------------------
 * Parses the input command into the command and its arguments.
 * Returns 0 on success, non-zero on failure.
 */
int ParseCommand(char *input_command, char *command, char *arg1, char *arg2) {
    char *token;

    /* Initialize command and arguments to empty strings */
    memset(command, 0, COMMAND_LENGTH);
    memset(arg1, 0, COMMAND_LENGTH);
    memset(arg2, 0, COMMAND_LENGTH);

    /* Get the first token (command) */
    token = strtok(input_command, " \t\n");
    if (token == NULL) {
        /* Empty command */
        return 1;
    }
    strcpy(command, token);

    /* Get the second token (arg1) */
    token = strtok(NULL, " \t\n");
    if (token != NULL) {
        strcpy(arg1, token);
    }

    /* Get the third token (arg2) */
    token = strtok(NULL, " \t\n");
    if (token != NULL) {
        strcpy(arg2, token);
    }

    return 0;
}

/* 
 * Function: WriteDataBlocks
 * -------------------------
 * Writes all data blocks back to the partition file.
 */
void WriteDataBlocks(EXT_DATOS *data_blocks, FILE *partition_file) {
    /* Move to data blocks starting at block 4 */
    fseek(partition_file, SIZE_BLOQUE * 4, SEEK_SET);
    fwrite(data_blocks, SIZE_BLOQUE, MAX_BLOQUES_DATOS, partition_file);
    fflush(partition_file);
}
