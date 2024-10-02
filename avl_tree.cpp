#include "avl_tree.h"
#include <algorithm>

AVLNode::
AVLNode(int k, int v) : 
key(k), value(v), left(nullptr), right(nullptr), height(1) {}

AVLTree::
AVLTree() : 
root(nullptr) {}

class AVLTree 
{
private:
    AVLNode* root;

    int height(AVLNode* n) 
    {
        return n ? n->height : 0;
    }

    int getBalance(AVLNode* n)
    {
        return n ? height(n->left) - height(n->right) : 0;
    }

    AVLNode* rightRotate (AVLNode* y) 
    {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;
    }

    AVLNode* leftRotate (AVLNode* x) 
    {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;
    }

    AVLNode* insert (AVLNode* node, int key, int value) 
    {
        if (!node) 
            return new AVLNode(key, value);

        if (key < node->key)
            node->left = insert (node->left, key, value);
        else if (key > node->key)
            node->right = insert (node->right, key, value);
        else
            return node;

        node->height = 1 + std::max (height(node->left), height (node->right));

        int balance = getBalance (node);

        // Left Left Case
        if (balance > 1 && key < node->left->key)
            return rightRotate(node);

        // Right Right Case
        if (balance < -1 && key > node->right->key)
            return leftRotate(node);

        // Left Right Case
        if (balance > 1 && key > node->left->key) 
        {
            node->left = leftRotate (node->left);
            return rightRotate (node);
        }

        // Right Left Case
        if (balance < -1 && key < node->right->key) 
        {
            node->right = rightRotate (node->right);
            return leftRotate(node);
        }

        return node;
    }

    int get(AVLNode* node, int key) 
    {
        if (!node) 
            return -1;

        if (key < node->key) 
            return get(node->left, key);
        else if (key > node->key) 
            return get(node->right, key);
        else 
            return node->value;
    }

    void inorderTraversal(AVLNode* node, std::vector<std::pair<int, int>>& result) 
    {
        if (node == nullptr)
            return;
        // Recursively traverse the left subtree
        inorderTraversal(node->left, result);
        // Visit the current node and add its key-value pair to the result vector
        result.push_back({node->key, node->value});
        // Recursively traverse the right subtree
        inorderTraversal(node->right, result);
    }

public:
    AVLTree() : root(nullptr) {}

    void put(int key, int value) {
        root = insert (root, key, value);
    }

    int get(int key) {
        return get (root, key);
    }

    // Scan function to get key-value pairs in sorted order (added)
    std::vector<std::pair<int, int>> scan() {
        std::vector<std::pair<int, int>> result;
        inorderTraversal(root, result); 
        return result;
    }
};

