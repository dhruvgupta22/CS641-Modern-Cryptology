#include <iostream>
#include <vector>
#include <gmpxx.h>

using namespace std;

// (a) Re-using the Miller-Rabin primality test from Q2 
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

// Generate a random prime of specifically requested bits using our Miller-Rabin test
mpz_class generate_prime(int bits, gmp_randclass& rand_gen) {
    mpz_class p;
    mpz_class min_val;
    mpz_ui_pow_ui(min_val.get_mpz_t(), 2, bits - 1);

    while (true) {
        p = rand_gen.get_z_bits(bits);
        p = p | min_val; // Set the MSB to 1
        p = p | 1;       // Ensure it is odd

        if (miller_rabin(p, 40, rand_gen)) {
            return p;
        }
    }
}

int main() {
    gmp_randclass rand_gen(gmp_randinit_default);
    rand_gen.seed(time(NULL));

    cout << "========== Question 3: Diffie-Hellman Key Exchange ==========\n\n";

    // (b) Generate a Sophie-Germain Prime p = 2p' + 1 for another prime p'.
    // To get a 512-bit prime p, we generate a 511-bit prime p' first.
    cout << "(b) Generating a 512-bit prime p such that (p-1) = 2p'...\n";
    
    mpz_class p, p_prime;
    while (true) {
        p_prime = generate_prime(511, rand_gen); // Generate 511-bit prime p'
        p = 2 * p_prime + 1;                             // Makes p exactly 512 bits
        
        // Check if our generated p is also prime
        if (miller_rabin(p, 40, rand_gen)) {
            break;
        }
    }
    
    cout << "Prime p (512-bit) = " << p.get_str(16) << " (hex)\n";
    cout << "Prime p' (511-bit)= " << p_prime.get_str(16) << " (hex)\n\n";

    // (c) Find a generator of the group Zp*
    cout << "(c) Finding a generator for the group Zp*...\n";
    // Since p is prime, the order of the group is p - 1 = 2 * p'.
    // The prime factors of the order are 2 and p'.
    // An element g is a generator if and only if order of g is p - 1, which means:
    // 1) g^2 != 1 mod p
    // 2) g^(p') != 1 mod p
    // Because of Lagrange's theorem, O(g) | (p-1), so O(g) can only be 1, 2, p', or 2*p'. We want O(g) = 2*p', so we just need to ensure it is not 1, 2, or p'. 
    mpz_class g;
    while (true) {
        // Pick a random element in Zp* (from 2 to p-2). g!=1 is ensured.
        g = rand_gen.get_z_range(p - 3) + 2; 
        
        mpz_class test1, test2;
        mpz_powm_ui(test1.get_mpz_t(), g.get_mpz_t(), 2, p.get_mpz_t());                // g^2 mod p
        mpz_powm(test2.get_mpz_t(), g.get_mpz_t(), p_prime.get_mpz_t(), p.get_mpz_t()); // g^p' mod p

        if (test1 != 1 && test2 != 1) {
            break; // Found a valid generator
        }
    }
    
    cout << "Generator g       = " << g.get_str(16) << " (hex)\n\n";

    // (d) Implement Diffie-Hellman Key Exchange
    cout << "(d) Simulating Diffie-Hellman Key Exchange...\n\n";
    
    // --- Alice's Side ---
    cout << "Alice generates private key 'a'...\n";
    mpz_class a = rand_gen.get_z_range(p - 3) + 2; 
    
    mpz_class A;
    mpz_powm(A.get_mpz_t(), g.get_mpz_t(), a.get_mpz_t(), p.get_mpz_t()); // A = g^a mod p
    cout << "Alice's public key A = " << A.get_str(16) << "\n\n";

    // --- Bob's Side ---
    cout << "Bob generates private key 'b'...\n";
    mpz_class b = rand_gen.get_z_range(p - 3) + 2;
    
    mpz_class B;
    mpz_powm(B.get_mpz_t(), g.get_mpz_t(), b.get_mpz_t(), p.get_mpz_t()); // B = g^b mod p
    cout << "Bob's public key B   = " << B.get_str(16) << "\n\n";

    // --- Exchange and Compute Secrets ---
    cout << "Alice computes shared secret S_A = B^a mod p...\n";
    mpz_class s_a;
    mpz_powm(s_a.get_mpz_t(), B.get_mpz_t(), a.get_mpz_t(), p.get_mpz_t());

    cout << "Bob computes shared secret S_B = A^b mod p...\n";
    mpz_class s_b;
    mpz_powm(s_b.get_mpz_t(), A.get_mpz_t(), b.get_mpz_t(), p.get_mpz_t());

    cout << "\nShared Secret (Alice) = " << s_a.get_str(16) << "\n";
    cout << "Shared Secret (Bob)   = " << s_b.get_str(16) << "\n\n";

    if (s_a == s_b) {
        cout << "SUCCESS: Both parties computed the same common secret!\n";
    } else {
        cout << "FAILURE: Shared secrets do not match.\n";
    }

    return 0;
}
