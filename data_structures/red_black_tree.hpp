#include <iostream>

enum class Color { RED, BLACK };

template <typename T>
class Node
{
public:
    T data;
    Color color;
    Node<T>* left;
    Node<T>* right;
    Node<T>* parent;

    explicit Node(T data) : data(data), color(Color::RED), left(nullptr), right(nullptr), parent(nullptr)
    {
    }
};


template <typename T>
class RedBlackTree
{
private:
    Node<T>* root;

    void rotateLeft(Node<T>* node)
    {
        Node<T>* rightChild = node->right;
        node->right = rightChild->left;

        if (rightChild->left != nullptr)
        {
            rightChild->left->parent = node;
        }

        rightChild->parent = node->parent;

        if (node->parent == nullptr)
        {
            root = rightChild;
        }
        else if (node == node->parent->left)
        {
            node->parent->left = rightChild;
        }
        else
        {
            node->parent->right = rightChild;
        }

        rightChild->left = node;
        node->parent = rightChild;
    }

    void rotateRight(Node<T>* node)
    {
        Node<T>* leftChild = node->left;
        node->left = leftChild->right;

        if (leftChild->right != nullptr)
        {
            leftChild->right->parent = node;
        }

        leftChild->parent = node->parent;

        if (node->parent == nullptr)
        {
            root = leftChild;
        }
        else if (node == node->parent->left)
        {
            node->parent->left = leftChild;
        }
        else
        {
            node->parent->right = leftChild;
        }

        leftChild->right = node;
        node->parent = leftChild;
    }

    void fixInsertion(Node<T>* node)
    {
        while (node != root && node->parent->color == Color::RED)
        {
            if (node->parent == node->parent->parent->left)
            {
                Node<T>* uncle = node->parent->parent->right;

                if (uncle != nullptr && uncle->color == Color::RED)
                {
                    node->parent->color = Color::BLACK;
                    uncle->color = Color::BLACK;
                    node->parent->parent->color = Color::RED;
                    node = node->parent->parent;
                }
                else
                {
                    if (node == node->parent->right)
                    {
                        node = node->parent;
                        rotateLeft(node);
                    }

                    node->parent->color = Color::BLACK;
                    node->parent->parent->color = Color::RED;
                    rotateRight(node->parent->parent);
                }
            }
            else
            {
                Node<T>* uncle = node->parent->parent->left;

                if (uncle != nullptr && uncle->color == Color::RED)
                {
                    node->parent->color = Color::BLACK;
                    uncle->color = Color::BLACK;
                    node->parent->parent->color = Color::RED;
                    node = node->parent->parent;
                }
                else
                {
                    if (node == node->parent->left)
                    {
                        node = node->parent;
                        rotateRight(node);
                    }

                    node->parent->color = Color::BLACK;
                    node->parent->parent->color = Color::RED;
                    rotateLeft(node->parent->parent);
                }
            }
        }

        root->color = Color::BLACK;
    }

    void insert(Node<T>* newNode)
    {
        Node<T>* current = root;
        Node<T>* parent = nullptr;

        while (current != nullptr)
        {
            parent = current;

            if (newNode->data < current->data)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }

        newNode->parent = parent;

        if (parent == nullptr)
        {
            root = newNode;
        }
        else if (newNode->data < parent->data)
        {
            parent->left = newNode;
        }
        else
        {
            parent->right = newNode;
        }

        fixInsertion(newNode);
    }

    void inorderTraversal(Node<T>* node)
    {
        if (node != nullptr)
        {
            inorderTraversal(node->left);
            std::cout << node->data << " ";
            inorderTraversal(node->right);
        }
    }

public:
    RedBlackTree() : root(nullptr)
    {
    }

    void insert(T data)
    {
        Node<T>* newNode = new Node<T>(data);
        insert(newNode);
    }

    void printInorder()
    {
        inorderTraversal(root);
        std::cout << std::endl;
    }
};

int main()
{
    RedBlackTree<int> tree;

    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    tree.insert(40);
    tree.insert(50);

    tree.printInorder();

    return 0;
}