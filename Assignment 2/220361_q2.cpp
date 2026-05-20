#include <iostream>
#include <vector>
#include <gmpxx.h>

using namespace std;

// (a) Miller-Rabin primality test
// Returns true if probably prime, false if composite
bool miller_rabin(const mpz_class& n, int k, gmp_randclass& rand_gen) {
    if (n <= 1) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    // Write n - 1 as d * 2^s
    mpz_class d = n - 1;
    int s = 0;
    while (d % 2 == 0) {
        d /= 2;
        s++;
    }

    for (int i = 0; i < k; i++) {
        // Random base a in [2, n-2]
        mpz_class a = rand_gen.get_z_range(n - 3) + 2;
        
        mpz_class x;
        mpz_powm(x.get_mpz_t(), a.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

        if (x == 1 || x == n - 1) {
            continue;
        }

        bool flag = true;
        for (int j = 1; j < s; j++) {
            mpz_powm_ui(x.get_mpz_t(), x.get_mpz_t(), 2, n.get_mpz_t());
            if (x == n - 1) {
                flag = false;
                break;
            }
        }

        if (flag) {
            return false; // Definitely composite
        }
    }

    return true; // Probably prime
}

// (b) Generate a random prime of specifically requested bits using our Miller-Rabin test
mpz_class generate_prime(int bits, gmp_randclass& rand_gen) {
    mpz_class p;
    // ensure the most significant bit is 1 so it's strictly a 'bits' sized integer
    mpz_class min_val;
    mpz_ui_pow_ui(min_val.get_mpz_t(), 2, bits - 1);

    while (true) {
        p = rand_gen.get_z_bits(bits);
        p = p | min_val; // Set the MSB to 1
        p = p | 1;       // Ensure it is odd

        // k = 40 iterations provides a very strong guarantee
        if (miller_rabin(p, 40, rand_gen)) {
            return p;
        }
    }
}

// (c) Extended Euclidean Algorithm
// Finds x, y such that: a*x + b*y = gcd(a,b)
// Returns the gcd(a,b)
mpz_class extended_euclidean(const mpz_class& a, const mpz_class& b, mpz_class& x, mpz_class& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }

    mpz_class x1, y1;
    mpz_class gcd = extended_euclidean(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return gcd;
}

int main(int argc, char* argv[]) {
    // Initialize GMP random generator
    gmp_randclass rand_gen(gmp_randinit_default);
    rand_gen.seed(time(NULL));

    if (argc > 1) {
        string mode = argv[1];
        if (mode == "a") {
            if (argc != 3) {
                cerr << "Usage for a: " << argv[0] << " a <number_to_test>\n";
                return 1;
            }
            mpz_class num(argv[2], 10);
            bool is_prime = miller_rabin(num, 40, rand_gen);
            cout << num.get_str(10) << " is " << (is_prime ? "probably prime" : "composite") << "\n";
            return 0;
        } else if (mode == "c") {
            if (argc != 4) {
                cerr << "Usage for c: " << argv[0] << " c <num1> <num2>\n";
                return 1;
            }
            mpz_class a(argv[2], 10);
            mpz_class b(argv[3], 10);
            mpz_class x, y;
            mpz_class gcd = extended_euclidean(a, b, x, y);
            cout << "gcd(" << a.get_str(10) << ", " << b.get_str(10) << ") = " << gcd.get_str(10) << "\n";
            cout << "x = " << x.get_str(10) << ", y = " << y.get_str(10) << "\n";
            return 0;
        }
    }

    cout << "========== Question 2: RSA Implementation ==========\n\n";

    // (b) Generate p and q (512 bits) (p != q)
    cout << "Generating 512-bit primes p and q...\n";
    mpz_class p = generate_prime(512, rand_gen);
    mpz_class q = generate_prime(512, rand_gen);
    
    while (p == q) {
        q = generate_prime(512, rand_gen);
    }

    cout << "p = " << p.get_str(16) << " (hex)\n";
    cout << "q = " << q.get_str(16) << " (hex)\n\n";

    // (d) Set n = p*q and compute d = e^-1 mod phi(n)
    mpz_class n = p * q;
    mpz_class phi_n = (p - 1) * (q - 1);
    mpz_class e = 65537; // Let us choose a common public exponent (65537 = 2^16 + 1)
    
    cout << "n (1024-bit) = " << n.get_str(16) << "\n";
    cout << "e = " << e.get_str() << "\n";

    mpz_class x, y;
    mpz_class gcd = extended_euclidean(e, phi_n, x, y);

    if (gcd != 1) {
        cout << "Error: e and phi(n) are not coprime!\n";
        return 1;
    }
    
    mpz_class d = (x % phi_n + phi_n) % phi_n; // Since x could be negative, make sure d is positive modulo phi(n)
    cout << "d (private key) = " << d.get_str(16) << "\n\n";

    // (e) Generate a random message of 1024 bits and encrypt/decrypt
    cout << "Generating 1024-bit random message...\n";
    
    // A strict 1024-bit message might end up being larger than n, which is mathematically invalid for textbook RSA. Thus we constrain the random message to be strictly less than n
    mpz_class m = rand_gen.get_z_range(n - 1);
    
    cout << "Original Message (M):\n" << m.get_str(16) << "\n\n";

    // Encrypt: c = m^e mod n
    mpz_class c;
    mpz_powm(c.get_mpz_t(), m.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
    
    cout << "Ciphertext (C = M^e mod n):\n" << c.get_str(16) << "\n\n";

    // Decrypt: m_dec = c^d mod n
    mpz_class m_dec;
    mpz_powm(m_dec.get_mpz_t(), c.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
    
    cout << "Decrypted Message (M'):\n" << m_dec.get_str(16) << "\n\n";

    // Check if decryption was successful
    if (m == m_dec) {
        cout << "SUCCESS: Decrypted message matches the original message!\n";
    } else {
        cout << "FAILURE: Messages do not match.\n";
    }

    return 0;
}
