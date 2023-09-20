#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_WORD_LEN 30

struct WordNode
{
    int count;
    char word[MAX_WORD_LEN + 1];
    struct WordNode *left;
    struct WordNode *right;
};


// 
struct WordNode *init_tree(const char *word, int count) 
{
    struct WordNode *root = malloc(sizeof(struct WordNode));
    memset(root, 0, sizeof(*root));
    root->count = count;
    strcpy(root->word, word);
    return root;
}


// return count for this word or -1 or error
int upsert_word_into_tree(struct WordNode *root, const char *word)
{
    if (root == NULL)
        return -1;

    for (;;)
    {
        int smp_res = strcmp(word, root->word);

        if (smp_res == 0)
        {
            root->count++;
            return root->count;
        }

        if (smp_res < 0)
        {
            // go left

            if (root->left == NULL)
            {
                // create new node
                root->left = malloc(sizeof(struct WordNode));
                memset(root->left, 0, sizeof(struct WordNode));
                strcpy(root->left->word, word);
            }
            
            root = root->left;
        }
        else
        {
            // go right

            if (root->right == NULL)
            {
                // create new node
                root->right = malloc(sizeof(struct WordNode));
                memset(root->right, 0, sizeof(struct WordNode));
                strcpy(root->right->word, word);
            }
            
            root = root->right;
        }
    }

    return -1;
}


// free all the memory taken by the tree
void free_tree(struct WordNode *root)
{
    if (root != NULL)
    {
        free_tree(root->left);
        free_tree(root->right);
        root->left = NULL;
        root->right = NULL;

        free(root);
    }
}


// (debugging purposes)
void show_tree(struct WordNode *root, int intent)
{
    if (root != NULL)
    {
        if (intent)
        for (int i = 0; i < intent; ++i)
            printf(" | ");
        printf(" \"%s\", %d\n", root->word, root->count);
        show_tree(root->left, intent+1);
        show_tree(root->right, intent+1);
    }
}


// for every node in the tree pring node->word if node->count equals count
void print_words_by_count(struct WordNode *root, int count)
{
    if (root != NULL)
    {
        if (root->count == count)
        {
            printf("- %s\n", root->word);
        }
        print_words_by_count(root->left, count);
        print_words_by_count(root->right, count);
    }
}


int process_file(FILE *file, int verbose)
{
    int processed = 0;
    int max_count = 0;
    struct WordNode *root = NULL;
    char word_buf[MAX_WORD_LEN + 1];
    int buf_p = 0;

    for (;feof(file) == 0 && ferror(file) == 0; processed++)
    {
        char c = fgetc(file);

        // check if c is a part of a word
        if ((isalnum(c) == 0 && (buf_p == 0 || c != '-')) || feof(file))
        {
            // check if the word is empty
            if (buf_p == 0)
                continue;
            
            word_buf[buf_p] = 0;   // set str delimiter
            
            // count +1 for this word
            int current_count;
            if (root == NULL)
            {
                root = init_tree(word_buf, 1);
                current_count = 1;
            }
            else
            {
                current_count = upsert_word_into_tree(root, word_buf);
            }
            // update max_count
            max_count = max_count > current_count ? max_count : current_count;
            // reset buffer's state
            buf_p = 0;
        }
        else
        {
            word_buf[buf_p] = tolower(c);
            buf_p++;

            // check if the word is not too long
            if (buf_p > MAX_WORD_LEN)
            {
                printf("Encountered word is too long; position %d\n", processed);
                free_tree(root);
                return -1;
            }
        }
    }

    // process word-count tree
    
    if (verbose)
    {
        printf("Word counts dictionary (tree):\n\n");
        show_tree(root, 0);
    }

    printf("\nHighest count: %d\nWords with highest count:\n", max_count);
    print_words_by_count(root, max_count);
    
    free_tree(root);
    return 0;
}


void show_usage(const char *argv0)
{
    printf("Usage: %s <path> [--verbose]\n", argv0);
}


int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3) 
    {
        show_usage(argv[0]);
        return -1;
    }

    int verbose = 0;
    if (argc == 3)
    {
        if (strcmp(argv[2], "--verbose"))
        {
            printf("Unknown option \"%s\"\n", argv[2]);
            show_usage(argv[0]);
            return -1;
        }
        verbose = 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        printf("File cannot be opened.\n");
        return -1;
    }

    if (process_file(file, verbose))
    {
        printf("An error has happened!");
        return -1;
    }

    fclose(file);

    return 0;
}
