# Copyright (C) 2024 Salvatore Sanfilippo <antirez@gmail.com>
# All Rights Reserved
#
# This code is released under the MIT license.
# See the LICENSE file for more information.

# Define common bigrams and words
bigrams = "intherreheanonesorteattistenntartondalitseediseangoulecomeneriroderaioicliofasetvetasihamaecomceelllcaurlachhidihofonsotacnarssoprrtsassusnoiltsemctgeloeebetrnipeiepancpooldaadviunamutwimoshyoaiewowosfiepttmiopiaweagsuiddoooirspplscaywaigeirylytuulivimabty"

words = [
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
"health", "person", "sure", "such", "history", "party"]

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
