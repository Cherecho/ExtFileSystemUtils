# ExtFileSystemUtils

ExtFileSystemUtils is a C-based program that simulates an EXT-like file system stored in a binary file (`particion.bin`). It provides a command-line interface for managing files and directories within the simulated file system.

## Features

ExtFileSystemUtils supports the following operations:
- **Retrieve File System Information**: Displays metadata such as block size, total/free inodes, and blocks.
- **View Byte Maps**: Displays inode and block usage maps.
- **List Files**: Displays details about files in the directory.
- **Rename Files**: Changes the name of an existing file.
- **Delete Files**: Removes files from the file system, freeing associated resources.
- **Print File Content**: Outputs the content of a file to the console.
- **Copy Files**: Duplicates a file within the file system.
- **Language Support**: Allows switching between English and Spanish (`language en` or `language es`).
- **Exit Command**: Safely exits the program, saving all changes.

## Requirements

To run the program, ensure you have:
- GCC or any compatible C compiler installed.
- A binary file named `particion.bin`, representing the simulated file system.

## Installation

1. Clone the repository or download the source files.
2. Compile the program using GCC:
   ```bash
   gcc -o ExtFileSystemUtils main.c

## Usage

Run the program using the following command:
```bash
./ExtFileSystemUtils
```

Available Commands (table)

| Command | Description |
| --- | --- |
| `info` | Displays metadata about the file system. |
| `bytemaps` | Displays inode and block usage maps. |
| `dir` | Lists all files in the directory with their details. |
| `rename <old_name> <new_name>` | Renames a file. |
| `remove <file_name>` | Deletes a file and frees its resources. |
| `imprimir <file_name>` | Prints the content of the specified file. |
| `copy <source> <destination>` | Copies a file to a new destination. |
| `language <en/es>` | Switches the language to English or Spanish. |
| `exit` `salir` | Exits the program, saving all changes to the binary file. |

## Example

```bash
>> info
Block 1024 Bytes
Inodes in partition = 128
Free inodes = 120
Blocks in partition = 1024
Free blocks = 900
First data block = 4

>> dir
file1.txt    size: 512    inode: 1    blocks: 4
file2.txt    size: 2048   inode: 2    blocks: 5 6 7

>> rename file1.txt document.txt
File renamed successfully.

>> remove file2.txt
File file2.txt removed successfully.

>> imprimir document.txt
This is the content of document.txt.

>> copy document.txt backup.txt
File copied successfully.

>> exit
Exiting...
```

## Project Structure

The project consists of the following files:
- `simul_ext.c`: Contains the main program logic and command-line interface.
- `headers.h`: Header file with function prototypes and data structures.
- `particion.bin`: Binary file representing the simulated file system.

## License

This project is licensed under the MIT License. See the `LICENSE` file for more information.
