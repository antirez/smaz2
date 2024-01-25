/* Copyright (C) 2024 by Salvatore Sanfilippo -- All rights reserved.
 * This code is licensed under the MIT license. See LICENSE file for info. */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* 128 common bigrams. */
const char *bigrams = "intherreheanonesorteattistenntartondalitseediseangoulecomeneriroderaioicliofasetvetasihamaecomceelllcaurlachhidihofonsotacnarssoprrtsassusnoiltsemctgeloeebetrnipeiepancpooldaadviunamutwimoshyoaiewowosfiepttmiopiaweagsuiddoooirspplscaywaigeirylytuulivimabty";

/* 256 common English words of length four letters or more. */
char *words[256] = {
"have", "that", "with", "this", "they", "from", "that", "what", "their",
"would", "make", "about", "know", "will", "time", "there", "year", "think",
"when", "which", "them", "some", "people", "take", "into", "just", "your",
"come", "could", "than", "like", "other", "then", "more", "these", "want",
"look", "first", "also", "because", "more", "find", "here", "thing", "give",
"many", "well", "only", "those", "tell", "very", "even", "back", "good",
"woman", "through", "life", "child", "there", "work", "down", "after", "should",
"call", "world", "over", "school", "still", "last", "need", "feel", "three",
"when", "state", "never", "become", "between", "high", "really", "something",
"most", "another", "much", "family", "leave", "while", "mean", "keep",
"student", "great", "same", "group", "begin", "seem", "country", "help", "talk",
"where", "turn", "problem", "every", "start", "hand", "might", "show", "part",
"about", "against", "place", "over", "such", "again", "case", "most", "week",
"company", "where", "system", "each", "right", "program", "hear", "question",
"during", "work", "play", "government", "small", "number", "always", "move",
"like", "night", "live", "point", "believe", "hold", "today", "bring", "happen",
"next", "without", "before", "large", "million", "must", "home", "under",
"water", "room", "write", "mother", "area", "national", "money", "story",
"young", "fact", "month", "different", "right", "study", "book", "word",
"though", "business", "issue", "side", "kind", "four", "head", "black",
"long", "both", "little", "house", "after", "since", "long", "provide",
"service", "around", "friend", "important", "father", "away", "until", "power",
"hour", "game", "often", "line", "political", "among", "ever", "stand",
"lose", "however", "member", "meet", "city", "almost", "include", "continue",
"later", "community", "much", "name", "five", "once", "white", "least",
"president", "learn", "real", "change", "team", "minute", "best", "several",
"idea", "body", "information", "nothing", "right", "lead", "social",
"understand", "whether", "back", "watch", "together", "follow", "around",
"parent", "only", "stop", "face", "anything", "create", "public", "already",
"speak", "others", "read", "level", "allow", "office", "spend", "door",
"health", "person", "sure", "such", "history", "party"
};

/* Compress the string 's' of 'len' bytes and stores the compression
 * result in 'dst' for a maximum of 'dstlen' bytes. Returns the
 * amount of bytes written. */
unsigned long smaz2_compress(unsigned char *dst, unsigned long dstlen, unsigned char *s, unsigned long len)
{

    int debug = 0;       // Log debugging messages.
    int verblen = 0;     /* Length of the emitted verbatim sequence, 0 if
                          * no verbating sequence was emitted last time,
                          * otherwise 1...5, it never reaches 8 even if we have
                          * vertabim len of 8, since as we emit a verbatim
                          * sequence of 8 bytes we reset verblen to 0 to
                          * star emitting a new verbatim sequence. */
    unsigned long y = 0; // Index of next byte to set in 'dst'.

    while(len && y < dstlen) {
        /* Try to emit a word. */
        if (len >= 4) {
            unsigned int i, wordlen;
            for (i = 0; i < 256; i++) {
                const char *w = words[i];
                wordlen = strlen(w);
                unsigned int space = s[0] == ' ';

                if (len >= wordlen+space &&
                    memcmp(w,s+space,wordlen) == 0) break; // Match.
            }

            /* Emit word if a match was found.
             * The escapes are:
             * byte value 6: simple word.
             * byte value 7: word + space.
             * byte value 8: space + word. */
            if (i != 256) {
                if (s[0] == ' ') {
                    if (debug) printf("( %s)", words[i]);
                    if (y < dstlen) dst[y++] = 8; // Space + word.
                    if (y < dstlen) dst[y++] = i; // Word ID.
                    s++; len--; // Account for the space.
                } else if (len > wordlen && s[wordlen] == ' ') {
                    if (debug) printf("(%s )", words[i]);
                    if (y < dstlen) dst[y++] = 7; // Word + space.
                    if (y < dstlen) dst[y++] = i; // Word ID.
                    s++; len--; // Account for the space.
                } else {
                    if (debug) printf("(%s)", words[i]);
                    if (y < dstlen) dst[y++] = 6; // Simple word.
                    if (y < dstlen) dst[y++] = i; // Word ID.
                }
                
                /* Consume. */
                s += wordlen;
                len -= wordlen;
                verblen = 0;
                continue;
            }
        }

        /* Try to emit a bigram. */
        if (len >= 2) {
            int i;
            for (i = 0; i < 128; i++) {
                const char *b = bigrams + i*2;
                if (s[0] == b[0] && s[1] == b[1]) break;
            }

            /* Emit bigram if a match was found. */
            if (i != 128) {
                int x = 1;
                if (y < dstlen) dst[y++] = x<<7 | i;
                
                /* Consume. */
                s += 2;
                len -= 2;
                verblen = 0;
                if (debug) printf("[%c%c]", bigrams[i*2], bigrams[i*2+1]);
                continue;
            }
        }

        /* No word/bigram match. Let's try if we can represent this
         * byte with a single output byte without escaping. We can
         * for all the bytes values but 1, 2, 3, 4, 5, 6, 7, 8. */
        if (!(s[0] > 0 && s[0] < 9) && s[0] < 128) {
            if (y < dstlen) dst[y++] = s[0];

            /* Consume. */
            if (debug) printf("{%c}", s[0]);
            s++;
            len--;
            verblen = 0;
            continue;
        }

        /* If we are here, we got no match nor in the bigram nor
         * with the single byte. We have to emit 'varbatim' bytes
         * with the escape sequence. */
        verblen++;
        if (verblen == 1) {
            if (debug) printf("_%c", s[0]);
            if (y+1 == dstlen) break; /* No room for 2 bytes. */
            dst[y++] = verblen;
            dst[y++] = s[0];
        } else {
            if (debug) printf("%c", s[0]);
            dst[y++] = s[0];
            dst[y-(verblen+1)] = verblen; // Fix the verbatim bytes length.
            if (verblen == 5) verblen = 0; // Start to emit a new sequence.
        }

        /* Consume. */
        s++;
        len--;
    }
    return y;
}

/* Decompress the string 'c' of 'len' bytes and stores the compression
 * result in 'dst' for a maximum of 'dstlen' bytes. Returns the
 * amount of bytes written. */
unsigned long smaz2_decompress(unsigned char *dst, unsigned long dstlen, unsigned char *c, unsigned long len)
{
    unsigned long orig_dstlen = dstlen, i = 0;

    while (i < len) {
        if ((c[i] & 128) != 0) {
            /* Emit bigram. */
            unsigned char idx = c[i]&127;
            if (dstlen && dstlen-- && i < len) *dst++ = bigrams[idx*2];
            if (dstlen && dstlen-- && i < len) *dst++ = bigrams[idx*2+1];
            i++;
        } else if (c[i] > 0 && c[i] < 6) {
            /* Emit verbatim sequence. */
            unsigned char vlen = c[i++];
            while(vlen-- && i < len)
                if (dstlen && dstlen--) *dst++ = c[i++];
        } else if (c[i] > 5 && c[i] < 9) {
            /* Emit word. */
            unsigned char escape = c[i];
            if (dstlen && escape == 8 && dstlen--) *dst++ = ' ';
            i++; // Go to word ID byte.
            if (i == len) return 0; // Malformed input.
            unsigned char idx = c[i++], j = 0;
            while(words[idx][j] != 0)
                if (dstlen && dstlen--) *dst++ = words[idx][j++];
            if (dstlen && escape == 7 && dstlen--) *dst++ = ' ';
        } else {
            /* Emit byte as it is. */
            if (dstlen--) *dst++ = c[i++];
        }
    }
    return orig_dstlen - dstlen;
}
