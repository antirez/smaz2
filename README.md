# SMAZ2 small messages compression algorithm

*Note: this library is no longer compatible with the old version, Smaz (still available in the old repository). It was redesigned to be much more resistant to non compressible input (it rarely enlarges the input text, almost never actually). It also compresses better than before.*

## Motivations

LoRa networks have an extremely limited bandwidth and requires a long
channel time in order to send even small messages. When LoRa is used
to send messages between humans, a form of compression improves the
channel utilization in a sensible way.

This compression scheme is designed to compress small messages in extremely
memory constrained devices, like ESP32 devices running MicroPython.
The basic idea is to use a pre-computed bigrams and words tables to encode
short messages more efficiently, for a total RAM usage of less than
2kbytes.

The words table is composed of 256 words. Short words (len less than 4 bytes)
are not present because they are better encoded with bigrams.
This is the full list of the 256 words:

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

If a word match is not found, the bigram table is used. The table is composed of the most common 128 bigrams by frequency, for a total of 256 bytes:

*"intherreheanonesorteattistenntartondalitseediseangoulecomeneriroderaioicliofasetvetasihamaecomceelllcaurlachhidihofonsotacnarssoprrtsassusnoiltsemctgeloeebetrnipeiepancpooldaadviunamutwimoshyoaiewowosfiepttmiopiaweagsuiddoooirspplscaywaigeirylytuulivimabty"*

When not even a matching bigram is found, bytes with value 0 or in the range
from 9 to 127 can be encoded with a single byte (this happens for instance for
all the ASCII uppercase letters, symbols, numbers...). The byte value can be
left as it is.

For bytes in the range from 1 to 8 and from 128 to 255, an escape sequence
is generated and from 1 to 5 verbatim bytes are emitted. Bytes with values
6, 7, 8 are used as special escapes to emit a word from the table. The
value of 6 is used.

## Encoding

So this is how the encoding works:

* A byte with value from 128 to 255 encodes a bigram with ID from 0 to 127.
* A byte with value 0 or from 9 to 127 is just what it is.
* A byte with value of 6 is followed by a byte representing the word ID to emit.
* A byte with value 7 is like 6, but after the word a space is emitted.
* A byte with value 8 is like 6, but before the word a space is emitted.
* A byte with a value from 1 to 5 means that from 1 to 5 verbatim bytes follow.

This means that this compression scheme will use more space than the input
string only when emtting verbatim bytes, that is when the string contains
special or unicode characters.

As long as the messages are latin letters natural language messages with common statistical properties, the program will only seldom use more space than needed and will often be able to compress words to less bytes. However programs using this scheme should have a one bit flag in the header in order to signal if the message is compressed or not, so that every time the result would be larger than the uncompressed message, no compression is used in order to trasmit the message.

## Real world compression achieved

./smax c "The program is designed to work well with English text"
Compressed length: 44.44%

./smax c "As long as the messages are latin letters natural language messages with common statistical properties, the program will only seldom use more space than needed"
Compressed length: 54.72%

./smax c "Anche se in maniera meno efficiente, questo algoritmo di compressione è in grado di comprimere testi in altre lingue."
Compressed length: 66.95%

## Implementations

In this repository you will find both a C and a Python implementation.
The implementation is optimized for space (both RAM and code executable)
and not for speed, since most use cases will use it very seldom, only when
a short message will be sent. So the algorithm scans the tables at every
string position, which is very costly in general, but should still be
adequate for this library.

## License

MIT license.
