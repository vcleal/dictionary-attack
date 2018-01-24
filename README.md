# dictionary-attack
Application to perform a dictionary attack on encrypted zip files through a brute force attack. Also include an application to create a dictionary file from words harvested from text files in a directory path.

## wordharvest.c
Creates the dictionary searching the filesystem for files and extract alphanumeric words from them, saving unique occurrences.

*Implemented to be used in linux systems and with max 4 char file extensions*

**Arguments:**
- `-d` specify the directory path
- `-e` specify file extensions to be searched
- `-o` specify the output dictionary file  

## bruteforce.py
Does the bruteforce dictionary attack to find the password of a password protected ZIP file

**Dependencies:**
- Python 2.7.x
- zipfile *(Only works with encrypted files using ZipCrypto)*

**Arguments:**
- `-l` specify the dictionary file
- `-f` specify the ZIP file

###### TODO
- break the C file in list/hash libraries
- solution to AES encryption and zipfile library