/* main.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic 
 *  synchronization.
 * ----------------------------------------------------------
 */

// Library Includes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Local Includes
#include "data.h"
#include "map.h"

// Defines
#define MAXWORDSIZE 25
#define MINWORDSIZE 1

// Globals
int NUMARTICLES = 10000;
int MINARTICLESIZE = 7500;
int MAXARTICLESIZE = 10000;



/* --------------------------------------------------------------------
 * GenerateWord
 * --------------------------------------------------------------------
 * Produces a string of randomly generated letters over [a-z] with
 * a maximum length of MAXWORDSIZE.
 * --------------------------------------------------------------------
 */
char * GenerateWord()
{
    // Create space for word
    unsigned int length = MINWORDSIZE + rand() % (MAXWORDSIZE-MINWORDSIZE);
    char * word = ( char * )malloc( ( length + 1 ) * sizeof( char ) );

    // Fill word with random letters
    for ( unsigned int i = 0; i < length; i ++ )
    {
        unsigned int c = 97 + rand() % 26;
        word[i] = (char)c;
    }

    word[length] = 0;
    return word;
}

struct Article * GenerateArticle()
{
    // Create article
    struct Article * article = (struct Article *)malloc( sizeof( struct Article ) );

    // How many words?
    //unsigned int numWords = MINARTICLESIZE + rand() % (MAXARTICLESIZE - MINARTICLESIZE);
    unsigned int numWords = 3;
    // Allocate space for words
    char ** words = (char **)malloc( numWords * sizeof( char * ));

    // Create words, add to article
    /*
    for ( unsigned int i = 0; i < numWords; i ++ )
    {
        if (i == 1){
            words[i] = "hah";
        }
        else 
        words[i] = "else";
    }
    */
    words[0] = "I";
    words[1] = "am";
    words[2] = "9Tempest";
    article->words = words;
    article->numWords = numWords;
    return article;
}

struct Library * GenerateLibrary()
{
    // Choose the number of articles
    unsigned int numArticles = NUMARTICLES;
    
    // Create Library
    struct Library * library = ( struct Library * )malloc( sizeof( struct Library ) );

    // Allocate space for articles
    library->articles = ( struct Article ** )malloc( numArticles * sizeof( struct Article * ) );

    // Produce articles
    for ( unsigned int i = 0; i < numArticles; i ++ )
    {
        library->articles[i] = GenerateArticle();
    }

    library->numArticles = numArticles;
    return library;
}

void FreeLibrary( struct Library * lib )
{
    for ( unsigned int i = 0; i < lib->numArticles; i ++ )
    {
        struct Article * art = lib->articles[i];
        for ( unsigned int j = 0; j < art->numWords; j ++)
        {
            char * word = art->words[j];
            free( word );
        }
        free( art->words );
        free( art );
    }

    free( lib->articles );
    free( lib );
}

int main( int argv, char ** argc )
{
    // Usage
    if ( argv != 3 )
    {
        printf( "Usage: a1q1 [NUMARTICLES] [WORD]\n" );
        return 0;
    }
    else
        NUMARTICLES = atoi( argc[1] );

    // Init random number generator
    srand( 43 );

    // Allocate space for the library of articles
    struct Library * lib = GenerateLibrary();
    
    // Count
    clock_t start;
    clock_t stop;

    start = clock();
    int count = CountOccurrences( lib, argc[2] );
    printf( "There are %d occurrences of %s\n", count, argc[2] );

    stop = clock();
    double time = (double)(stop - start)/ (double)CLOCKS_PER_SEC;
    printf( "Total time: %f\n", time );

    free( lib );
    
    return 0;
}