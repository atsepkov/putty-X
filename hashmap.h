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

#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap hashmap;

/*
 * Create a new hash table
 */
hashmap *Hashmap();

/*
 * Return the bucket offset for a given key as determined by the hashing
 * algorithm
 */
unsigned int hash(hashmap *h, char *key);

/*
 * Add a new element to the hash table, return 1 on success, 0 otherwise
 */
unsigned int hashmap_add(hashmap *h, char *key, char *value);

/*
 * Get the value of a given key
 */
char *hashmap_get(hashmap *h, char *key);

/*
 * I will not bother with clean-up since this hash only gets populated at load
 * time and will not need to be dynamically modified. For the same reason I will
 * not bother with resize logic either, I don't forsee people having hundreds of
 * entries in their config. Ideally, however, I should resize + rehash when the
 * hash becomes over 50% full.
 */

#endif			/* HASHMAP_H */