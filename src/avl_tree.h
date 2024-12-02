#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <utility>
#include <vector>

class AVLTree
{
   private:
    struct AVLNode
    {
        int key;
        int value;
        AVLNode *left;
        AVLNode *right;
        int height;

        AVLNode(int k, int v);
    };

    AVLNode *root;
    int current_size_;

    // Helper function for AVL Tree implementation.
    int height(AVLNode *n);
    int getBalance(AVLNode *n);
    AVLNode *rightRotate(AVLNode *y);
    AVLNode *leftRotate(AVLNode *x);
    AVLNode *minValueNode(AVLNode *node);
    void inorderTraversal(AVLNode *node,
                          std::vector<std::pair<int, int> > &result, int key1,
                          int key2);
    void clear(AVLNode *node);
    AVLNode *insert(AVLNode *node, int key, int value);

   public:
    AVLTree();

    int GetSize();
    void insert(int key, int value);
    int search(int key);
    std::vector<std::pair<int, int> > scan(int key1, int key2);

    void clear();
};

#endif