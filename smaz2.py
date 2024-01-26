# Copyright (C) 2024 Salvatore Sanfilippo <antirez@gmail.com>
# All Rights Reserved
#
# This code is released under the MIT license.
# See the LICENSE file for more information.

# Define common bigrams and words
bigrams = "intherreheanonesorteattistenntartondalitseediseangoulecomeneriroderaioicliofasetvetasihamaecomceelllcaurlachhidihofonsotacnarssoprrtsassusnoiltsemctgeloeebetrnipeiepancpooldaadviunamutwimoshyoaiewowosfiepttmiopiaweagsuiddoooirspplscaywaigeirylytuulivimabty"

words = [
"that", "this", "with", "from", "your", "have", "more", "will", "home",
"about", "page", "search", "free", "other", "information", "time", "they",
"what", "which", "their", "news", "there", "only", "when", "contact", "here",
"business", "also", "help", "view", "online", "first", "been", "would", "were",
"some", "these", "click", "like", "service", "than", "find", "date", "back",
"people", "list", "name", "just", "over", "year", "into", "email", "health",
"world", "next", "used", "work", "last", "most", "music", "data", "make",
"them", "should", "product", "post", "city", "policy", "number", "such",
"please", "available", "copyright", "support", "message", "after", "best",
"software", "then", "good", "video", "well", "where", "info", "right", "public",
"high", "school", "through", "each", "order", "very", "privacy", "book", "item",
"company", "read", "group", "need", "many", "user", "said", "does", "under",
"general", "research", "university", "january", "mail", "full", "review",
"program", "life", "know", "days", "management", "part", "could", "great",
"united", "real", "international", "center", "ebay", "must", "store", "travel",
"comment", "made", "development", "report", "detail", "line", "term", "before",
"hotel", "send", "type", "because", "local", "those", "using", "result",
"office", "education", "national", "design", "take", "posted", "internet",
"address", "community", "within", "state", "area", "want", "phone", "shipping",
"reserved", "subject", "between", "forum", "family", "long", "based", "code",
"show", "even", "black", "check", "special", "price", "website", "index",
"being", "women", "much", "sign", "file", "link", "open", "today", "technology",
"south", "case", "project", "same", "version", "section", "found", "sport",
"house", "related", "security", "both", "county", "american", "game", "member",
"power", "while", "care", "network", "down", "computer", "system", "three",
"total", "place", "following", "download", "without", "access", "think",
"north", "resource", "current", "media", "control", "water", "history",
"picture", "size", "personal", "since", "including", "guide", "shop",
"directory", "board", "location", "change", "white", "text", "small", "rating",
"rate", "government", "child", "during", "return", "student", "shopping",
"account", "site", "level", "digital", "profile", "previous", "form", "event",
"love", "main", "another", "class", "still"
]

# SMAX compression function
def smax_compress(s):
    s = s.encode()
    dst = bytearray()
    verblen = 0

    while len(s) > 0:
        # Try to find a matching word.
        if len(s) >= 4:
            for i, w in enumerate(words):
                wordlen = len(w)
                space = s[0] == 32

                if len(s) >= wordlen + space and s[space:wordlen+space] == w.encode():
                    break
            else:
                i = False

            if i:
                if s[0] == 32:
                    dst.append(8) # Space + word escape.
                    dst.append(i)
                    s = s[1:]
                elif len(s) > wordlen and s[wordlen] == 32:
                    dst.append(7) # Word + space escape.
                    dst.append(i)
                    s = s[1:]
                else:
                    dst.append(6) # Just word escape.
                    dst.append(i)

                s = s[wordlen:]
                verblen = 0
                continue

        # Try to find a matching bigram.
        if len(s) >= 2:
            for i in range(0, len(bigrams), 2):
                if s[:2] == bigrams[i:i+2].encode():
                    break
            else:
                i = False

            if i:
                dst.append(1 << 7 | i // 2)
                s = s[2:]
                verblen = 0
                continue

        # Can this byte be represented by itself?
        if not (0 < s[0] < 9) and s[0] < 128:
            dst.append(s[0])
            s = s[1:]
            verblen = 0
            continue

        # Otherwise, emit or update a verbatim sequence.
        verblen += 1
        if verblen == 1:
            dst.extend(bytes([verblen,s[0]]))
        else:
            dst.append(s[0])
            dst[-(verblen + 1)] = verblen
            if verblen == 5:
                verblen = 0

        s = s[1:]

    return bytes(dst)

# SMAX decompression function
def smax_decompress(c):
    i = 0
    res = bytearray()
    while i < len(c):
        if c[i] & 128 != 0: # Emit bigram
            idx = c[i]&127
            res.extend(bigrams[idx*2:idx*2+2].encode())
            i += 1
            continue
        elif 0 < c[i] < 6: # Emit verbatim
            res.extend(c[i+1:i+1+c[i]])
            i += 1+c[i]
            continue
        elif 5 < c[i] < 9: # Emit word
            if c[i] == 8: res.append(32)
            res.extend(words[c[i+1]].encode())
            if c[i] == 7: res.append(32)
            i += 2
        else: # Emit byte as it is
            res.append(c[i])
            i += 1
    return res.decode()

# Main function for command-line interface
if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        sys.exit("Usage: {} 'string to compress'".format(sys.argv[0]))

    compressed = smax_compress(sys.argv[1])
    print("Compressed length: {:.02f}%".format(len(compressed) / len(sys.argv[1].encode()) * 100))
    print(compressed)
    decompressed = smax_decompress(compressed)
    print("Decompress back: ", decompressed)
    print("The strings are the same after the back and forth?", sys.argv[1] == decompressed)
