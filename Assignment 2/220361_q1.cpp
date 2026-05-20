#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <unordered_map>
#include <openssl/sha.h>
#include <cstring>

#ifndef MATCH_BITS
#define MATCH_BITS 16
#endif

#ifndef MATCH_BITS_FOR_TRIAL1
#define MATCH_BITS_FOR_TRIAL1 20
#endif

#ifndef MATCH_BITS_FOR_TRIAL2
#define MATCH_BITS_FOR_TRIAL2 40
#endif

#ifndef NUM_TRIALS1 
#define NUM_TRIALS1 50
#endif

#ifndef NUM_TRIALS2 
#define NUM_TRIALS2 50
#endif

#ifndef MAX_ATTEMPTS
#define MAX_ATTEMPTS 2000000000LL
#endif

#ifndef MAX_ATTEMPTS2
#define MAX_ATTEMPTS2 50000000LL
#endif

#define PRINT_OUTPUT
// #define PRINT_PROGRESS

using namespace std;

const string MY_EMAIL_ID = "dhruvgupta22@iitk.ac.in";
const string CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789";

// Convert hash to hex string for printing
string hash_to_hex(const unsigned char* hash) {
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Check if the first n bits of two hashes match
bool check_match(const unsigned char* h1, const unsigned char* h2, int n_bits) {
    int bytes = n_bits / 8;
    int extra_bits = n_bits % 8;
    
    // Check full bytes
    for (int i = 0; i < bytes; i++) {
        if (h1[i] != h2[i]) return false;
    }
    
    // Check remaining bits
    if (extra_bits > 0) {
        unsigned char mask = (unsigned char)(0xFF << (8 - extra_bits));
        if ((h1[bytes] & mask) != (h2[bytes] & mask)) {
            return false;
        }
    }
    
    return true;
}

// Extract the first n bits of the hash into a 64-bit integer
uint64_t extract_prefix(const unsigned char* hash, int n_bits) {
    uint64_t prefix = 0;
    int bytes = n_bits / 8;
    int extra_bits = n_bits % 8;
    
    for (int i = 0; i < bytes; i++) {
        prefix = (prefix << 8) | hash[i];
    }
    
    if (extra_bits > 0) {
        unsigned char mask = (unsigned char)(0xFF << (8 - extra_bits));
        prefix = (prefix << extra_bits) | ((hash[bytes] & mask) >> (8 - extra_bits));
    }
    
    return prefix;
}

// Generate random string of given length
string random_string(int length, mt19937& gen) {
    uniform_int_distribution<> dist(0, CHARSET.length() - 1);
    string str;
    str.reserve(length);
    for (int i = 0; i < length; i++) {
        str += CHARSET[dist(gen)];
    }
    return str;
}

void gen_next_string(int &prefix_len, char email_buffer[64], char next_char[256], int suffix_len, const char *suffix, int &total_len){
    for (int i = prefix_len - 1; i >= 0; i--){
        if (email_buffer[i] != CHARSET.back()){
            email_buffer[i] = next_char[(unsigned char)email_buffer[i]];
            break;
        }
        email_buffer[i] = CHARSET[0];
        if (i == 0){
            // Overflow ("999" to "aaa", instead go to "aaaa")
            prefix_len++;
            for (int j = 0; j < prefix_len; j++)
                email_buffer[j] = CHARSET[0];
            for (int j = 0; j < suffix_len; j++)
                email_buffer[prefix_len + j] = suffix[j];
            email_buffer[prefix_len + suffix_len] = '\0';
            total_len = prefix_len + suffix_len;
        }
    }
}

int main(int argc, char* argv[]) {
    bool run_all = false;
    char input_char = '\0';
    
    if (argc < 2) {
        run_all = true;
    }
    else if(strlen(argv[1]) != 1) {
        cerr << "Invalid argument. Please provide a single character: a, b, c, d, e, f, or g" << endl;
        return 1;
    }
    else if(argv[1][0] < 'a' || argv[1][0] > 'g') {
        cerr << "Invalid argument. Please use one of: a, b, c, d, e, f, g" << endl;
        return 1;
    } else {
        input_char = argv[1][0];
    }

    // (1a) Compute SHA256 hash of my email id
    unsigned char h1[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(MY_EMAIL_ID.c_str()), MY_EMAIL_ID.length(), h1);

#ifdef PRINT_OUTPUT
    cout << "(1a) Email-id: " << MY_EMAIL_ID << "" << endl;
    cout << "h1 (SHA256): " << hash_to_hex(h1) << endl << "" << endl;
#endif

    if(!run_all && input_char == 'a') return 0; 

    
    char email_buffer[64];
    unsigned char h2[SHA256_DIGEST_LENGTH];
    const char* suffix = "@iitk.ac.in";
    int suffix_len = 11; 
    
    // Creating a fast lookup table for getting the next character in CHARSET
    char next_char[256];
    for (int i = 0; i < 256; i++) next_char[i] = CHARSET[0];
    for(size_t i = 0; i < CHARSET.length() - 1; i++) {
        next_char[(unsigned char)CHARSET[i]] = CHARSET[i + 1];
    }
    next_char[(unsigned char)CHARSET.back()] = CHARSET[0]; // Wrap '9' back to 'a'
    
    // Seeding and generating the initial random string in email_buffer
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> len_dist(4, 16); 
    int prefix_len = len_dist(gen);
    
    string random_start = random_string(prefix_len, gen);
    for(int i = 0; i < prefix_len; i++) email_buffer[i] = random_start[i];
    for(int i = 0; i < suffix_len; i++) email_buffer[prefix_len + i] = suffix[i];
    email_buffer[prefix_len + suffix_len] = '\0';
    
    int total_len = prefix_len + suffix_len;
    
    int current_match_bits = MATCH_BITS;
    long long current_attempt = 0;

    if(run_all || input_char == 'b') {
        // (1b) Find another IIT Kanpur email-id with the same first n bits
    #ifdef PRINT_OUTPUT
        cout << "(1b) Searching for second-preimage with first " << MATCH_BITS << " bits..." << endl;
    #endif

        while (current_attempt < MAX_ATTEMPTS) {
            SHA256(reinterpret_cast<const unsigned char*>(email_buffer), total_len, h2);
            
            if (check_match(h1, h2, current_match_bits) && string(email_buffer) != MY_EMAIL_ID) {
    #ifdef PRINT_OUTPUT
                cout << endl << "Second-preimage found after " << current_attempt + 1 << " attempts!" << endl;
                cout << "Result:" << endl;
                cout << "  h1 email: " << MY_EMAIL_ID << "" << endl;
                cout << "  h1 hash:  " << hash_to_hex(h1) << "" << endl;
                cout << "  h2 email: " << email_buffer << "" << endl;
                cout << "  h2 hash:  " << hash_to_hex(h2) << "" << endl;
                cout << "  Attempts: " << current_attempt + 1 << "" << endl;
                cout << "  Match: first " << current_match_bits << " bits identical" << endl;
    #else // !PRINT_OUTPUT
                // If output is disabled, just print the number of attempts
                cout << current_attempt + 1 << "" << endl;
    #endif
                current_match_bits += 4;
                continue;
            }

            // Generate next string to try (mutate the buffer)
            gen_next_string(prefix_len, email_buffer, next_char, suffix_len, suffix, total_len);
            current_attempt++;
        }

    #ifdef PRINT_OUTPUT
        cout << "Max attempts reached for " << current_match_bits << " bits. Stopping." << endl;
    #endif

        if (!run_all) return 0;
    }
    
    if(run_all || input_char == 'c') {
        // (1c) Run for NUM_TRIALS1 times and report average attempts for MATCH_BITS_FOR_TRIAL1 bits
    #ifdef PRINT_OUTPUT
        cout << endl << "(1c) Running " << NUM_TRIALS1 << " trials for " << MATCH_BITS_FOR_TRIAL1 << " bits match..." << endl;
    #endif

        long long total_trials_attempts = 0;

        for (int trial = 0; trial < NUM_TRIALS1; trial++) {
            // Get a new random starting string for each trial
            prefix_len = len_dist(gen);
            random_start = random_string(prefix_len, gen);
            for(int i = 0; i < prefix_len; i++) email_buffer[i] = random_start[i];
            for(int i = 0; i < suffix_len; i++) email_buffer[prefix_len + i] = suffix[i];
            email_buffer[prefix_len + suffix_len] = '\0';
            total_len = prefix_len + suffix_len;

            long long trial_attempts = 0;
            bool found = false;

            while (!found && trial_attempts < 10*MAX_ATTEMPTS) {
                SHA256(reinterpret_cast<const unsigned char*>(email_buffer), total_len, h2);
                
                if (check_match(h1, h2, MATCH_BITS_FOR_TRIAL1) && string(email_buffer) != MY_EMAIL_ID) {
                    found = true;
                    break;
                }

                gen_next_string(prefix_len, email_buffer, next_char, suffix_len, suffix, total_len);
                trial_attempts++;
            }

            if (found) {
                total_trials_attempts += (trial_attempts + 1);
            } else {
                total_trials_attempts += 10*MAX_ATTEMPTS;
            }

    #ifdef PRINT_PROGRESS
            if ((trial + 1) % 10 == 0) {
                cout << "  Completed " << trial + 1 << "/" << NUM_TRIALS1 << " trials..." << endl;
            }
    #endif
        }

    #ifdef PRINT_OUTPUT
        cout << "Average attempts (t) for " << MATCH_BITS_FOR_TRIAL1 << " bits over " 
            << NUM_TRIALS1 << " trials : " << (double)total_trials_attempts / NUM_TRIALS1 << "" << endl;
        cout << "log2(t) = " << log2((double)total_trials_attempts / NUM_TRIALS1) << "" << endl;
    #else
        cout << "log2(t) = " << log2((double)total_trials_attempts / NUM_TRIALS1) << "" << endl;
    #endif
        if (!run_all) return 0;
    }

    if(run_all || input_char == 'd') {
        cout << endl << "(1d) We are trying to run a second preimage attack, which takes 2^n number of computations of the hash function (excluding the computation of h(x) for the given input x), where n is the number of initial bits to match. Therefore, if t is the number of attempts needed, then log_2(t) is expected to be close to n." << endl;
        if (!run_all) return 0;
    }

    unordered_map<uint64_t, string> seen_hashes;

    if(run_all || input_char == 'e') {
        // (1e)  Collision attack for MATCH_BITS
        int current_match_bits_1e = MATCH_BITS;
        
        while (true) {
    #ifdef PRINT_OUTPUT
            cout << endl << "(1e) Searching for collision with first " << current_match_bits_1e << " bits..." << endl;
    #endif

            seen_hashes.clear();

            prefix_len = len_dist(gen);
            random_start = random_string(prefix_len, gen);
            for(int i = 0; i < prefix_len; i++) email_buffer[i] = random_start[i];
            for(int i = 0; i < suffix_len; i++) email_buffer[prefix_len + i] = suffix[i];
            email_buffer[prefix_len + suffix_len] = '\0';
            total_len = prefix_len + suffix_len;

            current_attempt = 0;
            bool collision_found = false;

            while (current_attempt < MAX_ATTEMPTS2) {
                SHA256(reinterpret_cast<const unsigned char*>(email_buffer), total_len, h2);
                
                uint64_t prefix = extract_prefix(h2, current_match_bits_1e);
                
                auto it = seen_hashes.find(prefix);
                if (it != seen_hashes.end()) { // Collision in prefix found
                    string email1 = it->second;
    #ifdef PRINT_OUTPUT
                    unsigned char hx[SHA256_DIGEST_LENGTH];
                    SHA256(reinterpret_cast<const unsigned char*>(email1.c_str()), email1.length(), hx);
                    cout << endl << "Collision found after " << current_attempt + 1 << " attempts!" << endl;
                    cout << "Result:" << endl;
                    cout << "  x email: " << email1 << "" << endl;
                    cout << "  x hash:  " << hash_to_hex(hx) << "" << endl;
                    cout << "  y email: " << email_buffer << "" << endl;
                    cout << "  y hash:  " << hash_to_hex(h2) << "" << endl;
                    cout << "  Attempts: " << current_attempt + 1 << "" << endl;
                    cout << "  Match: first " << current_match_bits_1e << " bits identical" << endl;
    #else
                    cout << current_attempt + 1 << "" << endl;
    #endif
                    collision_found = true;
                    current_match_bits_1e += 4;
                    current_attempt = 0;
                    break;
                }
                
                seen_hashes[prefix] = string(email_buffer);
                gen_next_string(prefix_len, email_buffer, next_char, suffix_len, suffix, total_len);
                current_attempt++;

                // if((current_attempt+1)%1000000 == 0) cout << "  Attempts: " << current_attempt+1 << "..." << endl;
            }

            if (!collision_found) {
    #ifdef PRINT_OUTPUT
                cout << "No collision found for " << current_match_bits_1e << " bits within max attempts. Stopping." << endl;
    #endif
                break;
            }
        }
        if (!run_all) return 0;
    }

    if(run_all || input_char == 'f') {
        // (1f) Run for NUM_TRIALS2 times and report average attempts for MATCH_BITS_FOR_TRIAL2 bits
    #ifdef PRINT_OUTPUT
        cout << endl << "(1f) Running " << NUM_TRIALS2 << " trials for " << MATCH_BITS_FOR_TRIAL2 << " bits match..." << endl;
    #endif

        long long total_trials_attempts_1f = 0;

        for (int trial = 0; trial < NUM_TRIALS2; trial++) {
            unordered_map<uint64_t, string> seen_hashes;
            uint64_t reserve_capacity = 1ull << (MATCH_BITS_FOR_TRIAL2 / 2);
            if (reserve_capacity > 10000000) { reserve_capacity = 10000000; }
            seen_hashes.reserve(reserve_capacity);

            // Get a new random starting string for each trial
            prefix_len = len_dist(gen);
            random_start = random_string(prefix_len, gen);
            for(int i = 0; i < prefix_len; i++) email_buffer[i] = random_start[i];
            for(int i = 0; i < suffix_len; i++) email_buffer[prefix_len + i] = suffix[i];
            email_buffer[prefix_len + suffix_len] = '\0';
            total_len = prefix_len + suffix_len;

            long long trial_attempts = 0;
            bool found = false;

            while (!found && trial_attempts < MAX_ATTEMPTS2) {
                SHA256(reinterpret_cast<const unsigned char*>(email_buffer), total_len, h2);
                
                uint64_t prefix = extract_prefix(h2, MATCH_BITS_FOR_TRIAL2);
                
                auto it = seen_hashes.find(prefix);
                if (it != seen_hashes.end() && it->second != string(email_buffer)) {
                    found = true;
                    break;
                }
                
                seen_hashes[prefix] = string(email_buffer);
                gen_next_string(prefix_len, email_buffer, next_char, suffix_len, suffix, total_len);
                trial_attempts++;
            }

            if (found) {
                total_trials_attempts_1f += (trial_attempts + 1);
            } else {
                total_trials_attempts_1f += 10*MAX_ATTEMPTS;
            }

    #ifdef PRINT_PROGRESS
            if ((trial + 1) % 10 == 0) {
                cout << "  Completed " << trial + 1 << "/" << NUM_TRIALS2 << " trials..." << endl;
            }
    #endif
        }

    #ifdef PRINT_OUTPUT
        cout << "Average attempts (t) for " << MATCH_BITS_FOR_TRIAL2 << " bits over " 
            << NUM_TRIALS2 << " trials : " << (double)total_trials_attempts_1f / NUM_TRIALS2 << "" << endl;
        cout << "log2(t) = " << log2((double)total_trials_attempts_1f / NUM_TRIALS2) << "" << endl;
    #else
        cout << "log2(t) = " << log2((double)total_trials_attempts_1f / NUM_TRIALS2) << "" << endl;
    #endif
        if (!run_all) return 0;
    }

    if(run_all || input_char == 'g'){
        cout << endl << "(1g) We are trying to run a collision attack. Based on the birthday paradox derivation done in class, the number of queries q needed for a collision probability p is approximately q = sqrt(2^n) * sqrt(2 ln(1/(1-p))), where 2^n is the size of the co-domain (number of possible values for an n-bit hash prefix). For a 90\% chance of collision (p = 0.90), the constant factor is approx 2.15. Thus, if t is the number of attempts, there is a very high chance that t < 2.15 * 2^(n/2) => log_2(t) < n/2 + log_2(2.15) = n/2 + 1.1. Hence we expect log_2(t) will be close to n/2 (maybe slightly greater)" << endl;
    }
    return 0;
}
