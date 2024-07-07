#include <iostream>
#include <vector>
#include <list>

// HashTable class
template<typename KeyType, typename ValueType>
class HashTable {
private:
    // Entry struct to hold key-value pairs
    struct Entry {
        KeyType key;
        ValueType value;
    };

    std::vector<std::list<Entry>> buckets; // Vector of linked lists (buckets)
    size_t size; // Number of elements in the hash table

    // Hash function to determine the index of a key in the vector
    size_t hash(const KeyType& key) const {
        // TODO: Implement your own hash function here
        // You can use std::hash<KeyType> to get a hash value for the key
        // and then use the modulo operator to get the index within the vector
        return std::hash<KeyType>{}(key) % buckets.size();
    }

public:
    // Constructor
    HashTable(size_t numBuckets = 10) : buckets(numBuckets), size(0) {}

    // Insert a key-value pair into the hash table
    void insert(const KeyType& key, const ValueType& value) {
        size_t index = hash(key);
        for (const auto& entry : buckets[index]) {
            if (entry.key == key) {
                // If the key already exists, update the value
                entry.value = value;
                return;
            }
        }
        // If the key doesn't exist, add a new entry
        buckets[index].push_back({key, value});
        size++;
    }

    // Remove a key-value pair from the hash table
    void remove(const KeyType& key) {
        size_t index = hash(key);
        for (auto it = buckets[index].begin(); it != buckets[index].end(); ++it) {
            if (it->key == key) {
                buckets[index].erase(it);
                size--;
                return;
            }
        }
    }

    // Get the value associated with a key
    ValueType get(const KeyType& key) const {
        size_t index = hash(key);
        for (const auto& entry : buckets[index]) {
            if (entry.key == key) {
                return entry.value;
            }
        }
        // If the key is not found, return a default-constructed value
        return ValueType();
    }

    // Check if a key exists in the hash table
    bool contains(const KeyType& key) const {
        size_t index = hash(key);
        for (const auto& entry : buckets[index]) {
            if (entry.key == key) {
                return true;
            }
        }
        return false;
    }

    // Get the number of elements in the hash table
    size_t getSize() const {
        return size;
    }
};

// Example usage
int main() {
    HashTable<std::string, int> myHashTable;
    myHashTable.insert("apple", 5);
    myHashTable.insert("banana", 10);
    myHashTable.insert("orange", 7);

    std::cout << "Size: " << myHashTable.getSize() << std::endl;
    std::cout << "Does it contain 'apple'? " << (myHashTable.contains("apple") ? "Yes" : "No") << std::endl;
    std::cout << "Value of 'banana': " << myHashTable.get("banana") << std::endl;

    myHashTable.remove("orange");

    std::cout << "Size: " << myHashTable.getSize() << std::endl;
    std::cout << "Does it contain 'orange'? " << (myHashTable.contains("orange") ? "Yes" : "No") << std::endl;

    return 0;
}