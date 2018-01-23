/**
 * @file wordharvest.c
 * @brief Program which searches the filesystem for files and extract words from them
 * 
 * The program search for files in the directory passed as an option (-d), with extensions
 * specified by another option (-e), extract alphanumeric words from them and save unique
 * occurences of these words in the output file specified by an option (-o). To ensure
 * no repeated words, they are stored in a hash table and each word found is checked if
 * already in the table prior to file write.
 * Implemented to be used in linux systems and with limitations of 4 char extensions and
 * 29 char words for common sense (but that can be easily increased).
 *
 * @author Victor C. Leal
 *
 * @date 14/10/2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

/** Hash table size */
#define MAXTABLE 100000

/**
 * @brief Struct for extensions linked list nodes.
 */
typedef struct _node_ext {
	char ext[5];	/* file extension (4 char max) */
	struct _node_ext *next;
} node_ext;

/**
 * @brief Struct for hash table linked list nodes.
 */
typedef struct _node_tab {
	char *word;
	struct _node_tab *next;
} node_tab;

/**
 * @brief Struct for extensions linked list.
 */
typedef struct _list {
	node_ext *head;
	node_ext *tail;
} list;

/**
 * @brief Struct for hash table of linked lists.
 */
typedef struct _list_ht {
	node_tab *head;
	node_tab *tail;
} list_ht;

/**
 * Inserts a extension string in the end of the linked list.
 * 
 * @param l  pointer to extensions linked list
 * @param e  extension string
 */
void insert_list(list *l, char *e);

/**
 * Inserts a word in the appropriate index in the hash table,
 * if there is index collision,
 * insert in the end of the linked list for that index.
 * 
 * @param l  pointer to hash table
 * @param n  word string
 */
void insert_list_ht(list_ht *l, char *n);

/**
 * Finds the specified word in the hash table,
 * used to check for duplicates and avoid insertion,
 * verifiying on the index (word hash) of the table.
 * 
 * @param l  pointer to linked list in hash table
 * @param word  string to be searched
 *
 * @return pointer to node with searched word, or NULL if not found
 */
node_tab* list_find(list_ht *l, char *word);

/**
 * Deallocates memory for extensions linked list.
 * 
 * @param l  pointer to extensions linked list 
 */
void free_list(list *l);

/**
 * Deallocates memory for linked list in hash table.
 * 
 * @param l  pointer to hash table linked list
 */
void free_list_ht(list_ht *l);

/**
 * Calculates the hash value for the string (word)
 * using XOR version of djb2 algorithm.
 * 
 * @param str  string to be hashed
 *
 * @return hash value for string received 
 */
unsigned long hash_function(char *str);

/**
 * Prints all hash table entries with index.
 * 
 * @param hash  pointer to hash table 
 */
void print_hashtable(list_ht *hash);

/**
 * Deallocates memory for the hash table
 * "freeing" all the linked list inside it.
 * 
 * @param hash  pointer to hash table
 */
void destroy_hash(list_ht *hash);

/**
 * Breaks the string with extensions separated with ':'
 * and insert each one in the passed linked list.
 * 
 * @param str  string with extension
 * @param list  pointer to extensions linked list
 */
void break_ext(char *str, list *list);

/**
 * Save unique words in the hash table,
 * and writes these words to the output file.
 * 
 * @param hash  pointer to hash table
 * @param word  string with word
 * @param fp  file pointer to output file
 */
void write_file(list_ht *ht, char *word, FILE *fp);

/**
 * Opens the file with the specified path, searching for words
 * and calling the write_file function for words found
 * to save them in the hash table and file passed by the pointer.
 * 
 * @param filename  string with file path to be harvested
 * @param ht  pointer to hash table
 * @param ofp  file pointer to output file 
 */
void harvest_words(char *filename, list_ht *ht, FILE *ofp);

/**
 * Finds all files in the search directory 
 * with the extensions inside the linked list
 * calling the harvest_words the function on these files.
 * 
 * @param l  pointer to extensions linked list
 * @param ht  pointer to hash table
 * @param dir  string with search directory
 * @param outfile  string with output file name
 */
void find_and_harvest(list *l, list_ht *ht, char *dir, char *outfile);

/**
 * Prints program help message with proper usage options
 */
void usage(void);

/*** Linked list functions ***/

void insert_list(list *l, char *e)
{
	node_ext *new = malloc(sizeof(node_ext));
	strncpy(new->ext,e,4);	/* 4 char extension */
	new->next = NULL;
	/* first element */
	if (l->head == NULL) {
		l->head = new;
		l->tail = l->head;
	}
	/* insert at the end */
	else {
		l->tail->next = new;
		l->tail = new;
	}
}

void insert_list_ht(list_ht *l, char *n)
{
	node_tab *new = malloc(sizeof(node_tab));
	new->word = malloc(strlen(n)+1);
	strncpy(new->word,n,strlen(n));
	new->next = NULL;
	/* first element */
	if (l->head == NULL) {
		l->head = new;
		l->tail = l->head;
	}
	/* insert at the end */
	else {
		l->tail->next = new;
		l->tail = new;
	}
}

node_tab* list_find(list_ht *l, char *word)
{
	node_tab *cur = l->head;
	while (cur != NULL && strcmp(cur->word,word) != 0) {
		cur = cur->next;
	}
	return cur;
}

void free_list(list *l)
{
	node_ext *cur = l->head, *nxt;
	while (cur != NULL) {
		nxt = cur->next;
		free(cur);
		cur = nxt;
	}
	free(l);
}

void free_list_ht(list_ht *l)
{
	node_tab *cur = l->head, *nxt;
	while (cur != NULL) {
		nxt = cur->next;
		free(cur->word);
		free(cur);
		cur = nxt;
	}
}

/*** Hash table functions ***/

/** XOR version djb2 algorithm */
unsigned long hash_function(char *str)
{
    int c;
    unsigned long hash = 5381;	/* seed */
    while ((c = *str++ ))
         hash = ((hash << 5) + hash) ^ c;	/* hash(i - 1) * 33 ^ str[i] */
    return hash;
}

void print_hashtable(list_ht *hash)
{
	node_tab *cur;

	for (long i = 0; i < MAXTABLE; i++) {
		cur = hash[i].head;
		while (cur != NULL) {
			printf("(%ld) \"%s\"\n", i, cur->word);
			cur = cur->next;
		}
	}
}

void destroy_hash(list_ht *hash)
{
	for (int i = 0; i < MAXTABLE; i++) {
		free_list_ht(&hash[i]);
	}
	free(hash);
}

/** Other program functions */

void break_ext(char *str, list *list)
{
	char *saveptr, *token;

	for (int j = 0 ; ; j++, str = NULL) {
		token = strtok_r(str, ":", &saveptr);	/* ':' delimiter */
		if (token == NULL)
			break;
		/* not checking token size (max 4 char extensions are stored) */
		insert_list(list,token);
	}
}

void write_file(list_ht *ht, char *word, FILE *fp)
{
	/* hash index fitting the table size (hash mod MAXTABLE) */
	unsigned long index = hash_function(word) % MAXTABLE;
	/* searching only at hash index */
	node_tab *found = list_find(&ht[index],word);

	if (found) {}
	else {	/* new word -> insert and write to file */
		insert_list_ht(&ht[index],word);
		fprintf(fp, "%s\n", word);
	}
}

void harvest_words(char *filename, list_ht *ht, FILE *ofp)
{
	FILE *fp;
	char word[30];	/* 29 char words */

	if ((fp = fopen(filename,"r")) == NULL) {
		fprintf(stderr,"can't open file %s", filename);
	}
	else {
		/* scan for alphanumeric words */
		while (fscanf(fp, "%29[a-zA-Z0-9]", word) == 1) {
			write_file(ht, word, ofp);
			/* any other char is a word separator */
			if (fscanf(fp, "%29[^a-zA-Z0-9]", word) != 1) {
				/* not checking for very long (30 chars plus) words, might "cut" them */
				continue;
			}
		}
	}
	fclose(fp);
}

void find_and_harvest(list *l, list_ht *ht, char *dir, char *outfile)
{
	node_ext *p = l->head;
	FILE *fp, *ofp;
	char command[1024], filename[1024];

	ofp = fopen(outfile,"w");
	if (!ofp) {
		perror("can't open write file");
		exit(1);
	}
	while (p) {
		/* find files with 'find', removing errors */
		if ( snprintf(command, sizeof(command),
		     "find %s -name \"*.%s\" -type f 2>/dev/null",
		     dir, p->ext) > sizeof(command) )
		{
			fprintf(stderr,"path too long, it may be truncated");
		}
		/* send 'find' command */
		if ((fp = popen(command, "r")) == NULL) {
			perror("find");
		}
		/* harvest on each file found */
		while (fgets(filename, sizeof(filename)-1, fp) != NULL) {
			filename[strcspn(filename, "\n")] = 0;	/* remove '\n' */
			harvest_words(filename, ht, ofp);
		}
		pclose(fp);
		p = p->next;
	}
	fclose(ofp);
}

void usage(void)
{
	fprintf(stderr, "Usage: wordharvest [-e extensions] -d directory -o outfile\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int opt, misopt, dflag = 0, eflag = 0, oflag = 0;
	char *default_ext[]={"txt","text"}, *path, *outfile;
	list *list = calloc(1,sizeof(list));
	list_ht *htable = calloc(MAXTABLE,sizeof(list_ht));

	if (argc < 5)
		usage();
	/* comand-line options and arguments */
	while ((opt = getopt (argc, argv, ":d:o:e:")) != -1) {
		switch (opt) {
			case 'e':
				eflag = 1;
				if (optarg[0]=='-')
					misopt = 'e';
				if (optarg)
					break_ext(optarg, list);
				break;
			case 'd':
				if (optarg[0]=='-')
					misopt = 'd';
				dflag = 1;
				path = argv[optind-1];
				break;
			case 'o':
				if (optarg[0]=='-')
					misopt = 'o';
				oflag = 1;
				outfile = argv[optind-1];
				break;
			case ':':
			/* missing option argument */
				fprintf(stderr, "option '-%c' requires an argument\n", optopt);
				usage();
				break;
			case '?':
			default:
			/* invalid option */
				fprintf(stderr, "option '-%c' is invalid\n", optopt);
				usage();
				break;
		}
	}
	/* missing any option argument */
	if (optind < argc) {
		fprintf(stderr, "option '-%c' requires an argument\n", misopt);
		usage();
	}
	/* missing required options */
	if ((dflag && oflag) == 0) {
		fprintf(stderr, "missing required option '-%c'\n", (dflag==0) ? 'd' : 'o');
		usage();
	}
	/* missing '-e' option */
	if (eflag == 0) {
		insert_list(list,default_ext[0]);
		insert_list(list,default_ext[1]);
	}
	/* search for files and harvest words */
	find_and_harvest(list,htable,path,outfile);

	destroy_hash(htable);
	free_list(list);

	return 0;
}