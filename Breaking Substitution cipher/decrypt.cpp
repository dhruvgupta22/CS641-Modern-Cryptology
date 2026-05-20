#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace std;

int main() {
    // Read the secret mapping
    ifstream mappingFile("secret_mapping.txt");
    if (!mappingFile) {
        cerr << "Error: Could not open secret_mapping.txt" << endl;
        return 1;
    }

    map<char, char> decryptMap;
    string line;
    
    while (getline(mappingFile, line)) {
        if (line.length() >= 4 && line.substr(1, 2) == "->") {
            char cipherChar = line[0];
            char plainChar = (line.length() > 3) ? line[3] : ' ';
            
            // Handle special case for space (represented as " ->N" etc.)
            if (cipherChar == ' ' && line[0] == ' ') {
                decryptMap[' '] = plainChar;
            } else {
                decryptMap[cipherChar] = plainChar;
            }
        }
    }
    mappingFile.close();

    // Read the ciphertext
    ifstream ctFile("ct.txt");
    if (!ctFile) {
        cerr << "Error: Could not open ct.txt" << endl;
        return 1;
    }

    // Open output file
    ofstream ptFile("pt.txt");
    if (!ptFile) {
        cerr << "Error: Could not create pt.txt" << endl;
        return 1;
    }

    // Decrypt and write
    char ch;
    while (ctFile.get(ch)) {
        if (decryptMap.find(ch) != decryptMap.end()) {
            ptFile.put(decryptMap[ch]);
        } else {
            ptFile.put(ch); // Keep unmapped characters as-is
        }
    }

    ctFile.close();
    ptFile.close();

    cout << "Decryption complete. Output written to pt.txt" << endl;
    
    return 0;
}
