#include <iostream>
#include <unordered_map>

// TrieNode class represents a single node in the trie
template<typename T>
class TrieNode {
public:
    std::unordered_map<T, TrieNode*> children;
    bool isEndOfWord;

    TrieNode() {
        isEndOfWord = false;
    }
};

// Trie class represents the trie data structure
template<typename T>
class Trie {
private:
    TrieNode<T>* root;

public:
    Trie() {
        root = new TrieNode<T>();
    }

    // Inserts a word into the trie
    void insert(const std::basic_string<T>& word) {
        TrieNode<T>* current = root;

        for (const T& ch : word) {
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = new TrieNode<T>();
            }

            current = current->children[ch];
        }

        current->isEndOfWord = true;
    }

    // Searches for a word in the trie
    bool search(const std::basic_string<T>& word) {
        TrieNode<T>* current = root;

        for (const T& ch : word) {
            if (current->children.find(ch) == current->children.end()) {
                return false;
            }

            current = current->children[ch];
        }

        return current->isEndOfWord;
    }

    // Deletes a word from the trie
    void remove(const std::basic_string<T>& word) {
        TrieNode<T>* current = root;

        for (const T& ch : word) {
            if (current->children.find(ch) == current->children.end()) {
                return;
            }

            current = current->children[ch];
        }

        current->isEndOfWord = false;
    }
};

int main() {
    // Example usage
    Trie<char> trie;

    trie.insert("apple");
    trie.insert("banana");
    trie.insert("orange");

    std::cout << "Search 'apple': " << (trie.search("apple") ? "Found" : "Not found") << std::endl;
    std::cout << "Search 'banana': " << (trie.search("banana") ? "Found" : "Not found") << std::endl;
    std::cout << "Search 'orange': " << (trie.search("orange") ? "Found" : "Not found") << std::endl;
    std::cout << "Search 'grape': " << (trie.search("grape") ? "Found" : "Not found") << std::endl;

    trie.remove("banana");

    std::cout << "Search 'banana' after removal: " << (trie.search("banana") ? "Found" : "Not found") << std::endl;

    return 0;
}