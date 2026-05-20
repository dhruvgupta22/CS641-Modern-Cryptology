# Breaking Substitution Cipher

## Finding "space"
Initial frequency analysis of the ciphertext reveals that 'd' is the most frequent character, suggesting it likely represents the space character in the plaintext. This hypothesis is supported by the observation that substituting d->space produces text segmented into words of reasonable lengths, whereas other character choices result in implausibly long word sequences.

To rigorously validate this hypothesis, we performed a statistical word length distribution analysis. The script `word_length_analysis.py` systematically evaluates each character by treating it as a potential space delimiter, then compares the resulting word length distribution against the known distribution of English word lengths (based on Peter Norvig's Google Books corpus analysis: http://norvig.com/mayzner.html).

The analysis confirms that d->space yields the closest match to authentic English word length patterns, establishing it as the optimal substitution. 

## Guessing "e"
**S** is the most frequent character after **d** in the ciphertext, making it a suitable candidate for the letter **e** in the plaintext.

## Finding other letters
After replacing S->e, I looked for long words with many e's, since there would be fewer such words. I found one such word - **,S"S/tvS@S{{** which appears as **\*e\*e\*\*\*e\*e\*\*** in the plaintext. Searching in the dictionary gives three matches - **nevertheless**, **resettlement**, **developement**. Note that since the last two letters must be the same, the only match is **nevertheless**. This gives: 

- ,->n
- "->v
- /->r
- t->t
- v->h
- @->l
- {->s

After this, it becomes easy to guess the remaining letters, like o, a, etc., from common words like "on", "that", etc. With these mappings in place, several lines become readable, and searching them on Google reveals the exact Wikipedia webpage from which this ciphertext was taken - https://en.wikipedia.org/wiki/Freedom_of_speech

## Finding Uppercase letters, numbers and punctuation
- Many uppercase letters were easy to identify since the text contains numerous country names, which always start with an uppercase letter.
- A transition from lowercase to uppercase indicates that the intermediate character is a period (full stop).
- Lists of items signal the presence of commas.
- Dates and years can be used to decode the numbers.
- Some letters can be found using common names. For example, *rinceton indicates that the first character must be a P.
- Any remaining characters can be found by matching with the webpage.
- Since the text is not an exact match with the webpage, caution is required. For example, the reference numbers do not match, so for decoding numbers, dates, years, and other contextual data should be used instead.

## Files

### `break.py`
Interactive tool for manually breaking substitution ciphers through iterative hypothesis testing and refinement.

**Usage:** `python3 break.py <ciphertext_file>`

**Features:**
- **Display Format:** Shows ciphertext in 150-character chunks with corresponding substitutions below (using `*` for unmapped characters, `0` for newlines)
- **Substitution Commands:** `<char1>-><char2>` - Map cipher character to plaintext character (use `0` for newline)
- **State Management:** 
  - `undo` - Revert to previous mapping state
  - `cmap` - Display current character mapping
  - `clear` - Reset all mappings
- **Analysis Tools:**
  - `freq` - Show character frequency distribution (in ciphertext)
  - `cfnd <string>` - Count occurrences of a pattern in ciphertext
  - `sdict <pattern>` - Search dictionary for words matching pattern (use `*` as wildcard). For eg., you can search `sdict **e*e*`.
- All mapping states are automatically logged to `cmap.txt` after each substitution or undo operation
- **Exit:** `quit` or `exit`

### `word_length_analysis.py`
Statistical analysis tool for identifying the space character in substitution ciphers by comparing word length distributions.

**Usage:** `python3 word_length_analysis.py`

**Methodology:**
1. Reads ciphertext from `ct.txt`
2. For each unique character in the ciphertext:
   - Treats the character as a potential space delimiter
   - Extracts word length distribution
   - Calculates total word count
3. Ranks characters by word count (higher counts typically indicate space)
4. Generates comparative visualization for candidates producing >1000 words
5. Displays bar chart comparing English word length distribution (Norvig's corpus) against candidate distributions

### `decrypt.cpp`
**Functionality:**
1. Reads character mapping from `secret_mapping.txt` (format: `<cipher_char>-><plain_char>`)
2. Loads ciphertext from `ct.txt`
3. Applies the substitution mapping to decrypt the text
4. Writes the plaintext output to `pt.txt`

**Usage:** 
```bash
g++ -o decrypt decrypt.cpp
./decrypt
```

**Input Files:**
- `secret_mapping.txt` - Character substitution mappings
- `ct.txt` - Ciphertext to decrypt

**Output:**
- `pt.txt` - Decrypted plaintext

### Other files
- `ct.txt` - Contains the ciphertext
- `pt.txt` - Contains the derypted plaintext
- `secret_mapping.txt` - Contains the key for decryption
- `cmap.txt` - Contains saved progress during the decoding.
- `secret_code.txt` - Answer for the assignment, i.e., the random line after decryption.

## References
1. http://norvig.com/mayzner.html
2. https://en.wikipedia.org/wiki/Freedom_of_speech