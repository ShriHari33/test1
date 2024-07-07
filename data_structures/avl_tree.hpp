#include <iostream>
#include <algorithm>

// AVLTree class template
template <typename T>
class AVLTree {
private:
    // Node structure
    struct Node {
        T data;             // Data stored in the node
        Node* left;         // Pointer to the left child node
        Node* right;        // Pointer to the right child node
        int height;         // Height of the node

        // Constructor
        Node(const T& value)
            : data(value), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;             // Pointer to the root node

    // Helper function to get the height of a node
    int getHeight(Node* node) {
        if (node == nullptr) {
            return 0;
        }
        return node->height;
    }

    // Helper function to get the balance factor of a node
    int getBalanceFactor(Node* node) {
        if (node == nullptr) {
            return 0;
        }
        return getHeight(node->left) - getHeight(node->right);
    }

    // Helper function to perform left rotation
    Node* rotateLeft(Node* node) {
        Node* newRoot = node->right;
        node->right = newRoot->left;
        newRoot->left = node;

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        newRoot->height = 1 + std::max(getHeight(newRoot->left), getHeight(newRoot->right));

        return newRoot;
    }

    // Helper function to perform right rotation
    Node* rotateRight(Node* node) {
        Node* newRoot = node->left;
        node->left = newRoot->right;
        newRoot->right = node;

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        newRoot->height = 1 + std::max(getHeight(newRoot->left), getHeight(newRoot->right));

        return newRoot;
    }

    // Helper function to insert a node into the tree
    Node* insertNode(Node* node, const T& value) {
        if (node == nullptr) {
            return new Node(value);
        }

        if (value < node->data) {
            node->left = insertNode(node->left, value);
        }
        else if (value > node->data) {
            node->right = insertNode(node->right, value);
        }
        else {
            return node; // Duplicate values are not allowed
        }

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));

        int balanceFactor = getBalanceFactor(node);

        // Perform rotations if the tree becomes unbalanced
        if (balanceFactor > 1) {
            if (value < node->left->data) {
                return rotateRight(node);
            }
            else {
                node->left = rotateLeft(node->left);
                return rotateRight(node);
            }
        }

        if (balanceFactor < -1) {
            if (value > node->right->data) {
                return rotateLeft(node);
            }
            else {
                node->right = rotateRight(node->right);
                return rotateLeft(node);
            }
        }

        return node;
    }

    // Helper function for inorder traversal of the tree
    void inorderTraversal(Node* node) {
        if (node != nullptr) {
            inorderTraversal(node->left);
            std::cout << node->data << " ";
            inorderTraversal(node->right);
        }
    }

public:
    // Constructor
    AVLTree() : root(nullptr) {}

    // Function to insert a value into the tree
    void insert(const T& value) {
        root = insertNode(root, value);
    }

    // Function for inorder traversal of the tree
    void inorder() {
        inorderTraversal(root);
    }
};