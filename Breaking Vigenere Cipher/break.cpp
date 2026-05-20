#include <iostream>
#include <bits/stdc++.h>

using namespace std;

#define MAX_KEY_LEN 100
#define EPSILON 0.001


string readFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    file.seekg(0, ios::end);
    long length = file.tellg();
    file.seekg(0, ios::beg);
    string content(length, ' ');
    file.read(&content[0], length);

    return content;
}

void writeFile(const string &filename, const string &content) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    // Write the content to the file
    file << content;
    file.close();
}

double indexOfCoincidence(string &text) {
    vector<int> freq(256, 0);
    for (char c : text) {
        freq[(unsigned char)c]++;
    }
    double ic = 0.0;
    int N = text.size();
    for (int f : freq) {
        ic += f * (f - 1);
    }
    if (N > 1) {
        ic /= (N * (N - 1));
    }
    return ic;
}

double analyzeKeyLength(const string &ciphertext, int keylen) {
    double avg_ic = 0.0;
    for (int col = 0; col < keylen; col++) {
        string colstr = "";
        for (size_t i = col; i < ciphertext.length(); i += keylen) {
            colstr += ciphertext[i];
        }
        double ic = indexOfCoincidence(colstr);
        // cout << "col: " << col << ", IC: " << ic << endl;
        avg_ic += ic;
    }
    avg_ic /= keylen;
    // cout << "Average IC for key length " << keylen << ": " << avg_ic << endl;
    return avg_ic;
}

int main(int argc, char *argv[]){
    if(argc != 3){
        cerr << "Usage: " << argv[0] << " <ciphertext_file> <plaintext_file>" << endl;
        return 1;
    }

    string ciphertext = readFile(argv[1]);

    string key = "";
    for(int keylen=1; keylen <= MAX_KEY_LEN; keylen++){
        // cout << "Key length: " << keylen << endl;
        double score = analyzeKeyLength(ciphertext, keylen);
        // cout << "------------------------" << endl;
        if(abs(score - 1.73/26.0) < EPSILON){
            // cout << "Possible key length found: " << keylen << " with avg ic score: " << score << endl;

            // Decode each column as Caesar cipher

            string possible_key = "";
            for(int col=0; col < keylen; col++){
                string colstr = "";
                for(size_t i = col; i < ciphertext.size(); i += keylen){
                    colstr += ciphertext[i];
                }
                
                // find which letter is most frequent in colstr
                int freq[26] = {0};
                for(char c : colstr) freq[c - 'a']++;

                // Find the most frequent letter
                char most_frequent = 'e';
                int max_count = 0;
                for(int i = 0; i < 26; i++){
                    if(freq[i] > max_count){
                        max_count = freq[i];
                        most_frequent = (char)(i + 'a');
                    }
                }

                // Calculate the key character
                char key_char = most_frequent - 'e' + 'a';
                if(key_char < 'a') key_char += 26;
                possible_key += key_char;
            }
            cout << "Possible key : " << possible_key << ", " << "length = " << keylen << endl;

            // Let's chose the smallest possible key for now
            if(key == "") key = possible_key;
        }
    }

    cout << "Chosen key: " << key << endl;

    string plaintext = "";
    for(int i=0; i < ciphertext.length(); i++){
        plaintext += ((ciphertext[i] - key[i%key.length()] + 26)%26 + 'a');
    }

    writeFile(argv[2], plaintext);


    return 0;
}