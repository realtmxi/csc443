#include "avl_tree.h"

#include <algorithm>

/* AVL Node Constructor. */
AVLNode::AVLNode(int k, int v)
    : key(k), value(v), left(nullptr), right(nullptr), height(1) {}

/* AVLTree Constructor. */
AVLTree::AVLTree()
    : root(nullptr), size(0) {}

/* Get the height of a node. */
int
AVLTree::height(AVLNode *n)
{
    return n ? n->height : 0;
}

/* Get the balance factor of a node. */
int
AVLTree::getBalance(AVLNode *n)
{
    return n ? height(n->left) - height(n->right) : 0;
}

/* Perform a right rotation on a subtree. */
AVLNode *
AVLTree::rightRotate(AVLNode *y)
{
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;

    // Perform rotation.
    x->right = y;
    y->left = T2;

    // Update heights.
    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    // Return the new root.
    return x;
}

/* Perform a left rotation on a subtree. */
AVLNode *
AVLTree::leftRotate(AVLNode *x)
{
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;

    // Perform rotation.
    y->left = x;
    x->right = T2;

    // Update heights.
    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    // Return the new root.
    return y;
}


/* Insert a new key-value pair node into the AVL tree. */
AVLNode *
AVLTree::insert(AVLNode *node, int key, int value)
{
    // Perform a normal BST Insertion.
    if (!node)
        return new AVLNode(key, value);

    if (key < node->key)
        node->left = insert(node->left, key, value);
    else if (key > node->key)
        node->right = insert(node->right, key, value);
    else
        return node;

    // Update the height of this ancestor node.
    node->height = 1 + std::max(height(node->left), height(node->right));

    // Get the balance factor.
    int balance = getBalance(node);

    // Left Left Case
    if (balance > 1 && key < node->left->key)
        return rightRotate(node);

    // Right Right Case
    if (balance < -1 && key > node->right->key)
        return leftRotate(node);

    // Left Right Case
    if (balance > 1 && key > node->left->key)
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    // Right Left Case
    if (balance < -1 && key < node->right->key)
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

/* Perform in-order traversal of the AVL tree. */
void
AVLTree::inorderTraversal(AVLNode *node, std::vector<std::pair<int, int>> &result, int key1, int key2)
{
    if (node == nullptr)
        return;

    // Go left if key1 is less than the current key
    if (key1 < node->key)
    {
        inorderTraversal(node->left, result, key1, key2);
    }

    // Add to result if the key is within the range
    if (key1 <= node->key && node->key <= key2)
    {
        result.push_back({node->key, node->value});
    }

    // Go right if key2 is greater than the current key
    if (key2 > node->key)
    {
        inorderTraversal(node->right, result, key1, key2);
    }
}


/* Recursively clear the no*/
void
AVLTree::clear(AVLNode *node)
{
    if (node == nullptr)
        return;

    clear(node->left);
    clear(node->right);
    delete node;
}

int
AVLTree::get(AVLNode *node, int key)
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

void
AVLTree::put(int key, int value)
{
    root = insert(root, key, value);
    size++;
}

int
AVLTree::get(int key)
{
    return get(root, key);
}

std::vector<std::pair<int, int>>
AVLTree::scan(int key1, int key2)
{
    std::vector<std::pair<int, int>> result;
    inorderTraversal(root, result, key1, key2);
    return result;
}

void
AVLTree::clear()
{
    clear(root);
    root = nullptr;
    size = 0;
}

size_t
AVLTree::getSize()
{
    return size;
}