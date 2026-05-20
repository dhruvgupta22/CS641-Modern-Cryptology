# Breaking Vigenère Cipher

C++ implementation that uses Index of Coincidence (IoC) and frequency analysis to break Vigenère cipher.

## Algorithm

### 1. Key Length Detection
- Test key lengths from 1 to 100
- For each potential key length, splits ciphertext into columns (characters encrypted with the same key character)
- Calculate the average Index of Coincidence across all columns
- Accept key lengths where average IoC ≈ 0.0665 (expected for English text: 1.73/26)
- Use tolerance of 0.001 to identify candidate lengths

### 2. Key Recovery
Select the first (smallest) valid key length found. This is because, if say, the key length is 5, multiples of 5 will also show high IoC value. Although this is still a guess, it works for our case.

For the accepted key length:
- Treat each column as a Caesar cipher
- Find the most frequent letter in each column
- Assume it corresponds to 'e' (most common English letter). This may not always be correct, but it should work with high probability.
- Calculate the key character as the shift from that letter to 'e'. 


### 3. Decryption
- Apply Vigenère decryption: `plaintext[i] = (ciphertext[i] - key[i % keylen] + 26) % 26`
- Write decrypted plaintext to output file

## Compilation

```bash
g++ -o break break.cpp
```

## Usage

```bash
./break <ciphertext_file> <plaintext_file>
```

**Arguments:**
- `<ciphertext_file>`: Input file containing the Vigenère-encrypted text
- `<plaintext_file>`: Output file where decrypted plaintext will be written

**Example:**
```bash
./break ct.txt pt.txt
```

## Output

The program displays:
- All possible keys found with their lengths
- The chosen key (first valid match)

Example output:
```
Possible key : buodx, length = 5
Possible key : buodxbuodx, length = 10
Possible key : buodxbuodxbuodx, length = 15
Possible key : buodxbuodxquodxbuodx, length = 20
<more such lines>
Chosen key: buodx
```

## Files

- `break.cpp`: Main implementation
- `ct.txt`: Provided ciphertext
- `pt.txt`: Decrypted plaintext
- `secret_code.txt`: Answer for the assignment, i.e., the random ten letters after decryption.



