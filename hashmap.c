/*
 * Simple Hash Table Implementation
 *
 * Copyright (c) 2013 Alexander Tsepkov

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * C selection of hash libraries seems to be limited, and most add unnecessary
 * "features" I didn't want, I wanted a simple hash map rather than 
 * high-performance implementation that can handle millions of entries like Judy. 
 * Since the only tricky parts of writing a hash is the hashing function and 
 * collision handling, I decided to roll my own. PuTTY already comes with SSH, 
 * so I can reuse the included CRC32 generation logic as the heart of my hashing
 * function. I don't expect a lot of entries, so I will simply use linear
 * collision handling.
 *
 * Since I only use this hash for storing/loading file data, I'm guaranteed that
 * all key/value pairs will be strings, but this can easily be changed in the
 * future to allow any data type by using 'void *' instead of 'char *'.
 */

#include "hashmap.h"
#include "ssh.h"

#define NUM_BUCKETS (256)

typedef struct hashmap_entry hashmap_entry;

struct hashmap_entry {
    char *key;
    char *value;
    unsigned int has_entry;
    hashmap_entry *next; // in case of collision
};

struct hashmap {
    hashmap_entry *data;
    unsigned int num_buckets;
    unsigned int num_entries;
};

hashmap *Hashmap()
{
    hashmap *h = snew(hashmap);
    h->data = snewn(NUM_BUCKETS, hashmap_entry);
    memset(h->data, '\0', sizeof(h->data));
    h->num_entries = 0;
    h->num_buckets = NUM_BUCKETS;
    return h;
}

void hashmap_free(hashmap *h)
{
    // first we deallocate all the linked list entries inside the buckets
    unsigned int i;
    hashmap_entry *cell;
    hashmap_entry *next_cell;
    for (i = 0; i < h->num_buckets; i++) {
	cell = (hashmap_entry*)(h->data + i);
	if (cell->has_entry == 1 && cell->next) {
	    sfree(cell->key);
	    sfree(cell->value);
	    // we want to be careful not to deallocate the cell itself from the array yet,
	    // we want to deallocate the array as a whole, the way it was allocated
	    cell = cell->next;
	    while (cell->next != NULL) {
		next_cell = cell->next;
		sfree(cell->key);
		sfree(cell->value);
		sfree(cell);
		cell = next_cell;
	    }
	}
    }
    
    // and then the main container itself
    sfree(h->data);
    sfree(h);
}

char **hashmap_keys(hashmap *h)
{
    /*
     * A rather naive key iterator implementation for now, which scans through every bucket
     */
    char **key_array = snewn(NUM_BUCKETS, char*); // we're in trouble if our load factor is > 1
    hashmap_entry *cell;
    unsigned int key_index = 0;
    unsigned int i;
    for (i = 0; i < h->num_buckets; i++) {
	cell = (hashmap_entry*)(h->data + i);
	if (cell->has_entry == 1) {
	    while (cell->next != NULL) {
		key_array[key_index] = cell->key;
		cell = cell->next;
		key_index++;
	    }
	    key_array[key_index] = cell->key;
	    key_index++;
	}
    }
    return key_array;
}

#define HASHMAP_WIN_DEBUG
//#define DEBUG_HASHMAP_ADD
/*
 * Since I'm doing debugging on Windows, my debug logic is unfortunately Windows-specific.
 * Unless you have WIN_DEBUG set, however, this module should be cross-platform. Feel free
 * to add UNIX_DEBUG if you want, but unless there is something broken in this hashmap that
 * I'm not aware of, there shouldn't be a need for it.
 */

#ifdef HASHMAP_WIN_DEBUG
#include <windows.h>
#endif /* HASHMAP_WIN_DEBUG */

unsigned int hash(hashmap *h, char *key)
{
    unsigned long index = crc32_compute((void*)(key), strlen(key));
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHing_FUNCTION)
    char hashval[256];
    sprintf(hashval, "'%s' %d", key, index);
    MessageBox(NULL, hashval, "Hashed Key", MB_ICONINFORMATION | MB_OK);
#endif /* HASHMAP_WIN_DEBUG */
    return index % h->num_buckets;
}

unsigned int hashmap_add(hashmap *h, char *key, char *value)
{
    unsigned int index = hash(h, key);
    hashmap_entry *cell = (hashmap_entry*)(h->data + index);
    /*
     * We expect duplicates to only occur when a setting is specified in Xresources
     * file in addition to normal config, so overwriting them inside our hash table
     * insertion logic directly will make the logic easier for us later.
     *
     * I set this logic to copy keys and values by value rather than by reference to
     * make things  simpler to the outside logic calling this function. It doesn't 
     * need to worry about clean-up or overwriting existing entries by accident.
     */
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_ADD)
    int link_idx = 0;
#endif /* HASHMAP_WIN_DEBUG */
    if (cell->has_entry == 1 && strcmp(key, cell->key)) {
	// resolve collisions through separate chaining via linked lists (with list heads)
	while (cell->next != NULL && strcmp(key, cell->key)) {
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_ADD)
	    link_idx++;
#endif /* HASHMAP_WIN_DEBUG */
	    cell = cell->next;
	}
	if (strcmp(key, cell->key)) { // key doesn't match, go to first link
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_ADD)
	    link_idx++;
#endif /* HASHMAP_WIN_DEBUG */
	    cell->next = snew(hashmap_entry);
	    cell = cell->next;
	    memset(cell, '\0', sizeof(cell));
	}
    } else {
        cell->has_entry = 1;
    }

    if (cell->key && !strcmp(key, cell->key)) {
	// avoid memory leaks if container is already in use
	sfree(cell->value);
    } else {
	cell->key = snewn(strlen(key)+1, char);
	strcpy(cell->key, key);
    }
    cell->value = snewn(strlen(value)+1, char);
    strcpy(cell->value, value);
    cell->next = NULL;
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_ADD)
    char info[256];
    sprintf(info, "%d-%d: '%s'->'%s'", index, link_idx, cell->key, cell->value);
    MessageBox(NULL, info, "Adding", MB_ICONINFORMATION | MB_OK);
#endif /* HASHMAP_WIN_DEBUG */
}

char *hashmap_get(hashmap *h, char *key)
{
    unsigned int index = hash(h, key);
    hashmap_entry *cell = (hashmap_entry*)(h->data + index);

#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_GET)
    int link_idx = 0;
#endif /* HASHMAP_WIN_DEBUG */
    while (cell->next != NULL && strcmp(key, cell->key)) {
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_GET)
	link_idx++;
#endif /* HASHMAP_WIN_DEBUG */
	cell = cell->next;
    }
    
    if (!cell->key || strcmp(key, cell->key)) {
	// reached the end of the bucket and the key still doesn't match
	return NULL;
    }
#if defined(HASHMAP_WIN_DEBUG) && defined(DEBUG_HASHMAP_GET)
    char info[256];
    sprintf(info, "%d-%d: '%s'->'%s'", index, link_idx, cell->key, cell->value);
    MessageBox(NULL, info, "Getting", MB_ICONINFORMATION | MB_OK);
#endif /* HASHMAP_WIN_DEBUG */
    return cell->value;
}
