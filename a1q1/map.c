/* map.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic 
 *  synchronization.
 *
 *  YOU MAY ADD WHATEVER YOU LIKE TO THIS FILE.
 *  YOU CANNOT CHANGE THE SIGNATURE OF CountOccurrences.
 * ----------------------------------------------------------
 */
#include "data.h"
#include <string.h>
#include <pthread.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
const int THREAD_CNT = 4;


struct args
{
    struct Article ** article_start_ptr;
    int len;
    char * word;
    volatile int * total_cnt;
    volatile int * cv_cnt;
};

//count the number of words in one article
int CountOneArticle(struct Article * article, char * word){
    if (!article || ! word) return 0;
    int cnt = 0;
    for (int i = 0; i < article->numWords; i++){
        if (!strcmp(article->words[i], word)){
            cnt++;
        }
    }
    return cnt;
}


// helper function that returns the count of word of a part of articles
void* CountSubOccurrences(void* argus){
    struct args* data = (struct args *)argus;
    int cnt = 0;
    for (int i = 0; i < data->len; i ++){
        cnt += CountOneArticle(data->article_start_ptr[i], data->word);
    }

    pthread_mutex_lock(&mutex);
    *data->total_cnt += cnt;
    *data->cv_cnt += 1;
    if (*data->cv_cnt == THREAD_CNT){
        pthread_cond_signal(&cv);
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}




/* --------------------------------------------------------------------
 * CountOccurrences
 * --------------------------------------------------------------------
 * Takes a Library of articles containing words and a word.
 * Returns the total number of times that the word appears in the 
 * Library.
 *
 * For example, "There the thesis sits on the theatre bench.", contains
 * 2 occurences of the word "the".
 * --------------------------------------------------------------------
 */



int CountOccurrences( struct  Library * lib, char * word )
{
    //init the number of threads, and split the articles to 4 parts
    volatile int cnt_total = 0;
    volatile int cv_cnt = 0;
    int part_len = lib->numArticles / 4;
    struct Article** part1_articles = lib->articles;
    struct Article** part2_articles = lib->articles + part_len;
    struct Article** part3_articles = lib->articles + 2 * part_len;
    struct Article** part4_articles = lib->articles + 3 * part_len;

    //init threads and its args
    pthread_t thread_1, thread_2, thread_3, thread_4;
    
    struct args t1_args = {part1_articles, part_len, word, &cnt_total, &cv_cnt};
    struct args t2_args = {part2_articles, part_len, word, &cnt_total, &cv_cnt};
    struct args t3_args = {part3_articles, part_len, word, &cnt_total, &cv_cnt};
    struct args t4_args = {part4_articles, lib->numArticles - 3 * part_len, word, &cnt_total, &cv_cnt};
    //create threads
    pthread_create(&thread_1, NULL, CountSubOccurrences, &t1_args);
    pthread_create(&thread_2, NULL, CountSubOccurrences, &t2_args);
    pthread_create(&thread_3, NULL, CountSubOccurrences, &t3_args);
    pthread_create(&thread_4, NULL, CountSubOccurrences, &t4_args);

    //provide a barrier


    pthread_mutex_lock(&mutex);

    while (cv_cnt != THREAD_CNT){
        pthread_cond_wait(&cv, &mutex);
    }
    
    pthread_mutex_unlock(&mutex);
    
    //clean up
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cv);
    





    return cnt_total;
}