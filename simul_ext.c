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

/* Function Prototypes */
void PrintByteMaps(EXT_BYTE_MAPS *byte_maps);
int ParseCommand(char *input_command, char *command, char *arg1, char *arg2);
int SearchFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *filename);
void ListDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes);
int RenameFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *old_name, char *new_name);
int PrintFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, EXT_DATOS *data_blocks, char *filename);
int DeleteFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
              EXT_BYTE_MAPS *byte_maps, EXT_SIMPLE_SUPERBLOCK *superblock,
              char *filename, FILE *partition_file);
int CopyFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
             EXT_BYTE_MAPS *byte_maps, EXT_SIMPLE_SUPERBLOCK *superblock,
             EXT_DATOS *data_blocks, char *source, char *destination, FILE *partition_file);
void WriteInodesAndDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, FILE *partition_file);
void WriteByteMaps(EXT_BYTE_MAPS *byte_maps, FILE *partition_file);
void WriteSuperBlock(EXT_SIMPLE_SUPERBLOCK *superblock, FILE *partition_file);
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
        /* Handle the 'bytemaps' command */
        else if (strcmp(command, "bytemaps") == 0) {
            PrintByteMaps(&byte_maps);
            continue;
        }
        /* Handle the 'dir' command */
        else if (strcmp(command, "dir") == 0) {
            ListDirectory(directory, &inodes);
            continue;
        }
        /* Handle the 'rename' command */
        else if (strcmp(command, "rename") == 0) {
            /* Check if both old and new names are provided */
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                if (language == 0) {
                    printf("ERROR: Missing arguments. Usage: rename <old> <new>\n");
                } else {
                    printf("ERROR: Faltan argumentos. Uso: rename <viejo> <nuevo>\n");
                }
                continue;
            }
            if (RenameFile(directory, &inodes, arg1, arg2) == 0) {
                /* Successful rename: write changes to disk */
                WriteInodesAndDirectory(directory, &inodes, partition_file);
                WriteByteMaps(&byte_maps, partition_file);
                WriteSuperBlock(&superblock, partition_file);
            }
            continue;
        }
        /* Handle the 'remove' command */
        else if (strcmp(command, "remove") == 0) {
            if (strlen(arg1) == 0) {
                if (language == 0) {
                    printf("ERROR: Missing filename. Usage: remove <filename>\n");
                } else {
                    printf("ERROR: Falta el nombre del fichero. Uso: remove <fichero>\n");
                }
                continue;
            }
            if (DeleteFile(directory, &inodes, &byte_maps, &superblock,
                           arg1, partition_file) == 0) {
                /* Successfully removed */
            }
            continue;
        }
        /* Handle the 'imprimir' (print) command */
        else if (strcmp(command, "imprimir") == 0) {
            if (strlen(arg1) == 0) {
                if (language == 0) {
                    printf("ERROR: Missing filename. Usage: imprimir <filename>\n");
                } else {
                    printf("ERROR: Falta el nombre del fichero. Uso: imprimir <fichero>\n");
                }
                continue;
            }
            PrintFile(directory, &inodes, data_blocks, arg1);
            continue;
        }
        /* Handle the 'copy' command */
        else if (strcmp(command, "copy") == 0) {
            /* Check if both source and destination names are provided */
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                if (language == 0) {
                    printf("ERROR: Missing arguments. Usage: copy <source> <destination>\n");
                } else {
                    printf("ERROR: Faltan argumentos. Uso: copy <origen> <destino>\n");
                }
                continue;
            }
            if (CopyFile(directory, &inodes, &byte_maps, &superblock,
                         data_blocks, arg1, arg2, partition_file) == 0) {
                /* Successful copy: already written to disk within CopyFile */
            }
            continue;
        }
        /* Handle the 'language' command */
        else if (strcmp(command, "language") == 0) {
            /* language [en|es] */
            if (strcmp(arg1, "es") == 0) {
                language = 1;
                printf("Language set to Spanish.\n");
            } else if (strcmp(arg1, "en") == 0 || strlen(arg1) == 0) {
                language = 0;
                printf("Language set to English.\n");
            } else {
                printf("Unknown language. Use 'language es' or 'language en'.\n");
            }
            continue;
        }
        /* Handle the 'salir' (exit) command */
        else if (strcmp(command, "salir") == 0 || strcmp(command, "exit") == 0) {
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
 * Function: PrintByteMaps
 * -----------------------
 * Displays the inode and block byte maps.
 */
void PrintByteMaps(EXT_BYTE_MAPS *byte_maps) {
    int i;
    
    if (language == 0) {
        /* English */
        printf("Inodes   : ");
        for (i = 0; i < MAX_INODOS; i++) {
            printf("%d ", byte_maps->bmap_inodos[i]);
        }
        printf("\nBlocks [0-25]: ");
        for (i = 0; i < 25 && i < MAX_BLOQUES_PARTICION; i++) {
            printf("%d ", byte_maps->bmap_bloques[i]);
        }
        printf("\n");
    } else {
        /* Spanish */
        printf("Inodos   : ");
        for (i = 0; i < MAX_INODOS; i++) {
            printf("%d ", byte_maps->bmap_inodos[i]);
        }
        printf("\nBloques [0-25]: ");
        for (i = 0; i < 25 && i < MAX_BLOQUES_PARTICION; i++) {
            printf("%d ", byte_maps->bmap_bloques[i]);
        }
        printf("\n");
    }
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
 * Function: SearchFile
 * --------------------
 * Searches for a file in the directory by its name.
 * Returns the index of the directory entry if found, -1 otherwise.
 */
int SearchFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *filename) {
    int i;
    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directory[i].dir_inodo != NULL_INODO) {
            if (strcmp(directory[i].dir_nfich, filename) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/* 
 * Function: ListDirectory
 * -----------------------
 * Lists all files in the directory, excluding the root entry ('.').
 */
void ListDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes) {
    int i;
    
    for (i = 0; i < MAX_FICHEROS; i++) {
        unsigned short inode_idx = directory[i].dir_inodo;
        /* Skip empty entries */
        if (inode_idx == NULL_INODO) {
            continue;
        }
        /* Skip the root entry '.' */
        if (strcmp(directory[i].dir_nfich, ".") == 0) {
            continue;
        }
        /* Retrieve inode information */
        EXT_SIMPLE_INODE *inode = &inodes->blq_inodos[inode_idx];
        if (language == 0) {
            /* English */
            printf("%s\tsize: %u\tinode: %d\tblocks: ", 
                   directory[i].dir_nfich, 
                   inode->size_fichero, 
                   inode_idx);
        } else {
            /* Spanish */
            printf("%s\ttamaño: %u\tinodo: %d\tbloques: ", 
                   directory[i].dir_nfich, 
                   inode->size_fichero, 
                   inode_idx);
        }
        
        int j;
        for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
            if (inode->i_nbloque[j] == NULL_BLOQUE) {
                break;
            }
            printf("%d ", inode->i_nbloque[j]);
        }
        printf("\n");
    }
}

/* 
 * Function: RenameFile
 * --------------------
 * Renames a file in the directory.
 * Returns 0 on success, -1 on failure.
 */
int RenameFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *old_name, char *new_name) {
    int source_idx = SearchFile(directory, inodes, old_name);
    if (source_idx < 0) {
        if (language == 0)
            printf("ERROR: File %s not found\n", old_name);
        else
            printf("ERROR: Fichero %s no encontrado\n", old_name);
        return -1;
    }
    /* Check if the new name already exists */
    int dest_idx = SearchFile(directory, inodes, new_name);
    if (dest_idx >= 0) {
        if (language == 0)
            printf("ERROR: File %s already exists\n", new_name);
        else
            printf("ERROR: El fichero %s ya existe\n", new_name);
        return -1;
    }
    /* Proceed with renaming */
    strncpy(directory[source_idx].dir_nfich, new_name, LEN_NFICH - 1);
    directory[source_idx].dir_nfich[LEN_NFICH - 1] = '\0';  // Ensure null-termination
    
    if (language == 0) {
        printf("File renamed successfully.\n");
    } else {
        printf("Fichero renombrado exitosamente.\n");
    }
    
    return 0;
}

/* 
 * Function: PrintFile
 * -------------------
 * Prints the content of a specified file.
 * Returns 0 on success, -1 on failure.
 */
int PrintFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
             EXT_DATOS *data_blocks, char *filename) {
    int dir_idx = SearchFile(directory, inodes, filename);
    if (dir_idx < 0) {
        if (language == 0)
            printf("ERROR: File %s not found\n", filename);
        else
            printf("ERROR: Fichero %s no encontrado\n", filename);
        return -1;
    }
    unsigned short inode_idx = directory[dir_idx].dir_inodo;
    EXT_SIMPLE_INODE *inode = &inodes->blq_inodos[inode_idx];
    unsigned int file_size = inode->size_fichero;
    
    if (file_size == 0) {
        if (language == 0)
            printf("File is empty.\n");
        else
            printf("El fichero está vacío.\n");
        return 0;
    }
    
    /* Allocate buffer to hold the file content */
    unsigned char *buffer = (unsigned char *)malloc(file_size + 1);
    if (buffer == NULL) {
        if (language == 0)
            printf("ERROR: Not enough memory.\n");
        else
            printf("ERROR: Memoria insuficiente.\n");
        return -1;
    }
    memset(buffer, 0, file_size + 1);
    
    /* Read each block and concatenate content */
    int offset = 0;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode->i_nbloque[i] == NULL_BLOQUE) {
            break;
        }
        int block_num = inode->i_nbloque[i];
        /* Calculate the index within data_blocks (0-based) */
        int data_block_idx = block_num - PRIM_BLOQUE_DATOS;
        if (data_block_idx < 0 || data_block_idx >= MAX_BLOQUES_DATOS) {
            /* Invalid block number */
            continue;
        }
        /* Determine the number of bytes to copy */
        int bytes_to_copy = file_size - offset;
        if (bytes_to_copy > SIZE_BLOQUE) {
            bytes_to_copy = SIZE_BLOQUE;
        }
        memcpy(buffer + offset, data_blocks[data_block_idx].dato, bytes_to_copy);
        offset += bytes_to_copy;
        if (offset >= file_size) {
            break;
        }
    }
    /* Ensure null-termination */
    buffer[file_size] = '\0';
    printf("%s\n", buffer);
    free(buffer);
    
    return 0;
}

/* 
 * Function: DeleteFile
 * --------------------
 * Deletes a specified file from the directory, freeing its inode and blocks.
 * Returns 0 on success, -1 on failure.
 */
int DeleteFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
              EXT_BYTE_MAPS *byte_maps, EXT_SIMPLE_SUPERBLOCK *superblock,
              char *filename, FILE *partition_file) {
    int dir_idx = SearchFile(directory, inodes, filename);
    if (dir_idx < 0) {
        if (language == 0)
            printf("ERROR: File %s not found\n", filename);
        else
            printf("ERROR: Fichero %s no encontrado\n", filename);
        return -1;
    }
    unsigned short inode_idx = directory[dir_idx].dir_inodo;
    EXT_SIMPLE_INODE *inode = &inodes->blq_inodos[inode_idx];
    
    /* Free the blocks in the byte map */
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inode->i_nbloque[i] == NULL_BLOQUE) {
            break;
        }
        unsigned short block = inode->i_nbloque[i];
        byte_maps->bmap_bloques[block] = 0;  // Mark block as free
        superblock->s_free_blocks_count++;
        inode->i_nbloque[i] = NULL_BLOQUE;
    }
    
    /* Free the inode in the byte map */
    byte_maps->bmap_inodos[inode_idx] = 0;  // Mark inode as free
    superblock->s_free_inodes_count++;
    
    /* Reset the inode's file size */
    inode->size_fichero = 0;
    
    /* Clear the directory entry */
    memset(directory[dir_idx].dir_nfich, 0, LEN_NFICH);
    directory[dir_idx].dir_inodo = NULL_INODO;
    
    /* Write the updated structures to disk */
    WriteInodesAndDirectory(directory, inodes, partition_file);
    WriteByteMaps(byte_maps, partition_file);
    WriteSuperBlock(superblock, partition_file);
    
    if (language == 0) {
        printf("File %s removed successfully.\n", filename);
    } else {
        printf("Fichero %s eliminado exitosamente.\n", filename);
    }
    
    return 0;
}

/* 
 * Function: CopyFile
 * ------------------
 * Copies a file from source to destination.
 * Returns 0 on success, -1 on failure.
 */
int CopyFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
             EXT_BYTE_MAPS *byte_maps, EXT_SIMPLE_SUPERBLOCK *superblock,
             EXT_DATOS *data_blocks, char *source, char *destination, FILE *partition_file) {
    int source_idx = SearchFile(directory, inodes, source);
    if (source_idx < 0) {
        if (language == 0)
            printf("ERROR: File %s not found\n", source);
        else
            printf("ERROR: Fichero %s no encontrado\n", source);
        return -1;
    }
    /* Check if the destination file already exists */
    int dest_idx = SearchFile(directory, inodes, destination);
    if (dest_idx >= 0) {
        if (language == 0)
            printf("ERROR: File %s already exists\n", destination);
        else
            printf("ERROR: El fichero %s ya existe\n", destination);
        return -1;
    }
    
    /* Find the first free inode */
    int free_inode = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (byte_maps->bmap_inodos[i] == 0) {
            free_inode = i;
            break;
        }
    }
    if (free_inode < 0) {
        if (language == 0)
            printf("ERROR: No free inodes available\n");
        else
            printf("ERROR: No hay inodos libres disponibles\n");
        return -1;
    }
    
    /* Find the first free directory entry */
    int free_dir = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directory[i].dir_inodo == NULL_INODO) {
            free_dir = i;
            break;
        }
    }
    if (free_dir < 0) {
        if (language == 0)
            printf("ERROR: No free directory entries available\n");
        else
            printf("ERROR: No hay entradas libres en el directorio\n");
        return -1;
    }
    
    /* Get the source inode */
    unsigned short source_inode_idx = directory[source_idx].dir_inodo;
    EXT_SIMPLE_INODE *source_inode = &inodes->blq_inodos[source_inode_idx];
    
    /* Initialize the destination inode */
    EXT_SIMPLE_INODE *dest_inode = &inodes->blq_inodos[free_inode];
    dest_inode->size_fichero = source_inode->size_fichero;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        dest_inode->i_nbloque[i] = NULL_BLOQUE;
    }
    
    /* Mark the destination inode as occupied */
    byte_maps->bmap_inodos[free_inode] = 1;
    superblock->s_free_inodes_count--;
    
    /* Copy each block from source to destination */
    unsigned int file_size = source_inode->size_fichero;
    int offset = 0;
    int block_index = 0;
    
    while (block_index < MAX_NUMS_BLOQUE_INODO && source_inode->i_nbloque[block_index] != NULL_BLOQUE) {
        int source_block_num = source_inode->i_nbloque[block_index];
        
        /* Find the first free block */
        int free_block = -1;
        for (int b = 0; b < MAX_BLOQUES_PARTICION; b++) {
            if (byte_maps->bmap_bloques[b] == 0) {
                free_block = b;
                break;
            }
        }
        if (free_block < 0) {
            if (language == 0)
                printf("ERROR: No free blocks available\n");
            else
                printf("ERROR: No hay bloques libres disponibles\n");
            return -1;
        }
        /* Mark the block as occupied */
        byte_maps->bmap_bloques[free_block] = 1;
        superblock->s_free_blocks_count--;
        
        /* Assign the block to the destination inode */
        dest_inode->i_nbloque[block_index] = free_block;
        
        /* Copy the block content */
        int source_data_idx = source_block_num - PRIM_BLOQUE_DATOS;
        int dest_data_idx = free_block - PRIM_BLOQUE_DATOS;
        
        if (source_data_idx >= 0 && source_data_idx < MAX_BLOQUES_DATOS &&
            dest_data_idx >= 0 && dest_data_idx < MAX_BLOQUES_DATOS) 
        {
            memcpy(data_blocks[dest_data_idx].dato, data_blocks[source_data_idx].dato, SIZE_BLOQUE);
        }
        
        offset += SIZE_BLOQUE;
        if (offset >= file_size) {
            break;
        }
        block_index++;
    }
    
    /* Create the directory entry for the destination file */
    strncpy(directory[free_dir].dir_nfich, destination, LEN_NFICH - 1);
    directory[free_dir].dir_nfich[LEN_NFICH - 1] = '\0';  // Ensure null-termination
    directory[free_dir].dir_inodo = free_inode;
    
    /* Write the updated structures to disk */
    WriteInodesAndDirectory(directory, inodes, partition_file);
    WriteByteMaps(byte_maps, partition_file);
    WriteSuperBlock(superblock, partition_file);
    WriteDataBlocks(data_blocks, partition_file);
    
    if (language == 0) {
        printf("File copied successfully.\n");
    } else {
        printf("Fichero copiado exitosamente.\n");
    }
    
    return 0;
}

/* 
 * Function: WriteInodesAndDirectory
 * ----------------------------------
 * Writes the inodes and directory structures back to the partition file.
 */
void WriteInodesAndDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, FILE *partition_file) {
    /* Move to inode block (block 2) */
    fseek(partition_file, SIZE_BLOQUE * 2, SEEK_SET);
    fwrite(inodes, SIZE_BLOQUE, 1, partition_file);
    
    /* Move to directory block (block 3) */
    fseek(partition_file, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(directory, SIZE_BLOQUE, 1, partition_file);
    fflush(partition_file);
}

/* 
 * Function: WriteByteMaps
 * -----------------------
 * Writes the byte maps back to the partition file.
 */
void WriteByteMaps(EXT_BYTE_MAPS *byte_maps, FILE *partition_file) {
    /* Move to byte maps block (block 1) */
    fseek(partition_file, SIZE_BLOQUE * 1, SEEK_SET);
    fwrite(byte_maps, SIZE_BLOQUE, 1, partition_file);
    fflush(partition_file);
}

/* 
 * Function: WriteSuperBlock
 * -------------------------
 * Writes the superblock back to the partition file.
 */
void WriteSuperBlock(EXT_SIMPLE_SUPERBLOCK *superblock, FILE *partition_file) {
    /* Move to superblock block (block 0) */
    fseek(partition_file, SIZE_BLOQUE * 0, SEEK_SET);
    fwrite(superblock, SIZE_BLOQUE, 1, partition_file);
    fflush(partition_file);
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
