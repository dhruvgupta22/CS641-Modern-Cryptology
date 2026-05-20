import sys
import re
from collections import Counter
from wordfreq import top_n_list


# ---------- File Reading ----------

def read_file(filename):
    try:
        with open(filename, "r") as f:
            return f.read()
    except IOError:
        print(f"Error opening file: {filename}")
        sys.exit(1)


# ---------- Display ----------

def print_state(ciphertext, mapping):
    for i in range(0, len(ciphertext), 150):
        chunk = ciphertext[i:i + 150]

        # Ciphertext line
        for c in chunk:
            print('0' if c == '\n' else c, end='')
        print()

        # Substitution line
        for c in chunk:
            if c in mapping:
                m = mapping[c]
                print('0' if m == '\n' else m, end='')
            else:
                print('*', end='')
        print("\n\n")


def save_cmap(mapping):
    try:
        with open("cmap.txt", "a") as f:
            f.write("--- State ---\n")
            for k, v in mapping.items():
                key = '0' if k == '\n' else k
                val = '0' if v == '\n' else v
                f.write(f"{key}->{val}\n")
            f.write("\n")
    except IOError:
        print("Error appending to cmap.txt")


# ---------- Dictionary Search ----------

def search_dictionary(pattern, dictionary):
    # * = any single character
    regex = "^" + pattern.replace("*", ".") + "$"
    r = re.compile(regex)
    return [w for w in dictionary if r.match(w)]


# ---------- Main ----------

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <ciphertext_file>")
        return

    # DO NOT change case
    ciphertext = read_file(sys.argv[1])

    # ciphertext = ciphertext[:1000]  # Limit to first 1,000 characters

    # Dictionary words (lowercase by nature)
    dictionary = top_n_list("en", 300000)

    # Frequency
    freq_map = Counter(ciphertext)
    freq_vec = sorted(freq_map.items(), key=lambda x: x[1], reverse=True)

    history = []
    current_map = {}
    history.append(current_map.copy())

    while True:
        print_state(ciphertext, current_map)

        try:
            # IMPORTANT: do NOT strip
            cmd = input("> ")
        except EOFError:
            break

        if cmd in ("quit", "exit"):
            break

        if cmd == "undo":
            if len(history) > 1:
                history.pop()
                current_map = history[-1].copy()
                save_cmap(current_map)
            continue

        if cmd == "clear":
            current_map.clear()
            history.append(current_map.copy())
            continue

        if cmd == "freq":
            for k, v in freq_vec:
                print(f"{'0' if k == '\n' else k}: {v}")
            input("Press Enter to continue...")
            continue

        if cmd.startswith("cfnd "):
            s = cmd[5:]
            count = 0
            pos = ciphertext.find(s)
            while pos != -1:
                count += 1
                pos = ciphertext.find(s, pos + 1)
            print(f'"{s}" appears {count} times')
            input("Press Enter to continue...")
            continue

        if cmd == "cmap":
            for k, v in current_map.items():
                print(f"{'0' if k == '\n' else k}->{ '0' if v == '\n' else v }")
            input("Press Enter to continue...")
            continue

        # ---------- sdict ----------
        if cmd.startswith("sdict "):
            pattern = cmd[6:]

            # dictionary is lowercase → normalize pattern ONLY here
            matches = search_dictionary(pattern.lower(), dictionary)

            if matches:
                for w in matches[:50]:
                    print(w)
                if len(matches) > 50:
                    print(f"... and {len(matches) - 50} more")
            else:
                print("No matching words found.")

            input("Press Enter to continue...")
            continue

        # ---------- Substitution ----------
        if "->" in cmd:
            lhs, rhs = cmd.split("->", 1)

            if lhs:
                key = '\n' if lhs == "0" else lhs[0]

                # RHS handling
                if rhs == "" or rhs == " ":
                    val = " "
                elif rhs == "0":
                    val = "\n"
                else:
                    val = rhs[0]

                current_map[key] = val
                history.append(current_map.copy())
                save_cmap(current_map)


if __name__ == "__main__":
    main()