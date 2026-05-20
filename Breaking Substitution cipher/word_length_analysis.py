import matplotlib.pyplot as plt
import collections

def analyze_word_lengths(filename):
    # English word length distribution (exact percentages from Norvig's Google Books corpus)
    # Source: http://norvig.com/mayzner.html
    english_dist = {
        0: 0.0,
        1: 2.998, 2: 17.651, 3: 20.511, 4: 14.787, 5: 10.700,
        6: 8.388, 7: 7.939, 8: 5.943, 9: 4.437, 10: 3.076,
        11: 1.761, 12: 0.958, 13: 0.518, 14: 0.222, 15: 0.076,
        16: 0.020, 17: 0.010, 18: 0.004, 19: 0.001, 20: 0.001
    }

    try:
        with open(filename, 'r') as f:
            original_text = f.read()
    except FileNotFoundError:
        print(f"Error: {filename} not found.")
        return

    unique_chars = sorted(list(set(original_text)))
    print(f"Analyzing {len(unique_chars)} unique characters...")
    print("-" * 50)

    results = []
    
    for ch in unique_chars:
        # 1. Replace "\n" with "0"
        # 2. Replace ch with " "
        processed_text = original_text.replace('\n', '0').replace(ch, ' ')
        
        # 3. Create histogram of word lengths
        # Split by space and filter empty strings (consecutive spaces)
        words = [w for w in processed_text.split(' ') if w]
        
        if not words:
            continue
            
        total_words = len(words)
        results.append((ch, total_words))

    # Sort results by word count (descending)
    results.sort(key=lambda x: x[1], reverse=True)

    print("Character Word Counts (higher is better for space):")
    for ch, count in results:
        print(f"Char '{ch}': {count} words found.")

    if results:
        best_char = results[0][0]
    else:
        best_char = None

    print("-" * 50)
    print(f"Best candidate for space character based on word length distribution: '{best_char}'")

    # Filter candidates with more than 1000 words
    top_candidates = [res for res in results if res[1] > 1000]

    # Plot the histogram for the actual English language and all top candidates
    plt.figure(figsize=(14, 8))
    
    # English data
    eng_x = list(english_dist.keys())
    eng_y = list(english_dist.values())

    # Plot English distribution
    plt.bar([x - 0.2 for x in eng_x], eng_y, width=0.4, color='black', alpha=0.8, label='English Language', zorder=10)

    # Prepare data for top candidates
    colors = plt.cm.viridis([i/len(top_candidates) for i in range(len(top_candidates))])
    
    for idx, (cand_char, count) in enumerate(top_candidates):
        # Re-calculate distribution for this candidate
        processed_text = original_text.replace('\n', '0').replace(cand_char, ' ')
        words = [w for w in processed_text.split(' ') if w] # Filter empty strings for plot accuracy
        lengths = [len(w) for w in words]
        length_counts = collections.Counter(lengths)
        total_words = len(words)
        
        cand_y = []
        for l in range(0, 21):
             cand_y.append((length_counts.get(l, 0) / total_words) * 100)

        plt.plot(range(0, 21), cand_y, marker='o', linestyle='-', linewidth=2, color=colors[idx], label=f"Char '{cand_char}' ({count} words)")

    plt.title(f'Word Length Distribution: English vs Candidates (>1000 words)')
    plt.xlabel('Word Length')
    plt.ylabel('Frequency (%)')
    plt.xticks(range(0, 21))
    plt.legend()
    plt.grid(axis='y', alpha=0.3)
    
    print(f"\nDisplaying plot for English language and {len(top_candidates)} candidates...")
    plt.show()

if __name__ == "__main__":
    analyze_word_lengths("ct.txt")
