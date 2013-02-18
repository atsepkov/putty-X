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

#include <assert.h>
#include "hashmap.h"
#include "ssh.h"

#define NUM_BUCKETS (256)

struct hashmap_entry {
    char *key;
    char *value;
    unsigned int has_entry;
    hashmap_element *next; // in case of collision
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
    sfree(h->data);
    sfree(h);
}

unsigned int hash(hashmap *h, char *key)
{
    unsigned long index = crc32_compute((void*)(key), sizeof(key));
    return index % h->num_buckets;
}

unsigned int hashmap_add(hashmap *h, char *key, char *value)
{
    unsigned int index = hash(h, key);
    hashmap_element *cell = (hashmap_element*)(h->data + index);
    /*
     * We expect duplicates to only occur when a setting is specified in Xresources
     * file in addition to normal config, so overwriting them inside our hash table
     * insertion logic directly will make the logic easier for us later.
     */
    if (cell->has_entry == 1 && key != cell->key) {
	// resolve collisions through separate chaining via linked lists (with list heads)
	while (cell->next != NULL && key != cell->key) {
	    cell = cell->next;
	}
	if (key != cell->key) {
	    cell->next = snew(hashmap_element);
	    cell = cell->next;
	}
    } else {
        cell->has_entry = 1;
    }
    
    cell->key = key;
    cell->value = value;
    cell->next = NULL;
}

char *hashmap_get(hashmap *h, char *key)
{
    unsigned int index = hash(h, key);
    hashmap_element *cell = (hashmap_element*)(h->data + index);
    while (cell->next != NULL && cell->key != key) {
        cell = cell->next;
    }
    
    assert(cell->key == key);
    return cell->value;
}