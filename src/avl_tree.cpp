#include "avl_tree.h"
#include <iostream>
#include <algorithm>

/* AVL Node Constructor. */
AVLTree::AVLNode::AVLNode(int k, int v)
    : key(k), value(v), left(nullptr), right(nullptr), height(1) {}

/* AVLTree Constructor. */
AVLTree::AVLTree()
    : root(nullptr) {}

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

/** 
 * Perform a right rotation on a subtree. 
 * Rotation performed as follows:
 *          y                  x
 *         / \                / \
 *        x   T3    =>      T1  y
 *       / \                    / \
 *      T1  T2                T2  T3
 */
AVLTree::AVLNode *
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

/**
 * Perform a left rotation on a subtree.
 * Rotation performed as follows:
 *          x                  y
 *         / \                / \
 *        T1  y      =>      x   T3
 *           / \            / \
 *          T2  T3         T1  T2
 */
AVLTree::AVLNode *
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

/* Return the node with minimum key value in a subtree. */
AVLTree::AVLNode *
AVLTree::minValueNode(AVLNode *node)
{
    AVLNode *current = node;
    while (current->left != nullptr)
    {
        current = current->left;
    }

    return current;
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

/* Recursively clear all nodes in the tree. */
void
AVLTree::clear(AVLNode *node)
{
    if (node == nullptr)
        return;

    clear(node->left);
    clear(node->right);
    delete node;
}

/* Insert a new key-value pair node into the AVL tree. */
AVLTree::AVLNode *
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

/**
 * Delete a AVL node with given key from subtree with given root. It returns the
 * root of the modified subtree. 
 */
AVLTree::AVLNode *
AVLTree::deleteNode(AVLNode *root, int key)
{
    // Perform a BST DELETION
    if (root == nullptr)
        return root;
    else if (key < root->key)
        root->left = deleteNode(root->left, key);
    else 
    {
        // Node with only one child or no child
        if (!root->left || !root->right)
        {
            AVLNode *temp = root->left ? root->left : root->right;
            
            if (!temp)
            {
                // No child
                temp = root;
                root = nullptr;
            }
            else
            {
                // One child
                *root = *temp;
            }

            delete temp;
        }
        else
        {
            // Node with two children
            AVLNode *temp = minValueNode(root->right);

            root->key = temp->key;
            root->value = temp->value;

            root->right = deleteNode(root->right, temp->key);
        }
    }

    // If the tree had only one node
    if (root == nullptr)
        return root;

    // Update height
    root->height = 1 + std::max(height(root->left), height(root->right));

    // Check the balance factor and re-balance the AVL tree if necessary
    int balance = getBalance(root);

    // Left Left Case
    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);
    
    // Left Right Case
    if (balance > 1 && getBalance(root->left) < 0)
        return rightRotate(root);
    
    // Right Right Case
    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);
    
    // Right Left Case
    if (balance < -1 && getBalance(root->right) > 0)
    {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

void
AVLTree::insert(int key, int value)
{
    root = insert(root, key, value);
}

void
AVLTree:: remove(int key)
{
    root = deleteNode(root, key);
}

/* Search for a value accociated with the given key. */
int
AVLTree::search(int key)
{
    AVLNode *curr = root;
    while (curr)
    {
        if (key < curr->key)
            curr = curr->left;
        else if (key > curr->key)
            curr = curr->right;
        else
            return curr->value;
    }

    return -1;
}

/* Get the key-value pairs within the specified range. */
std::vector<std::pair<int, int>> AVLTree::scan(int key1, int key2)
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
}