#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <utility>
#include <vector>
struct AVLNode
{
    int key;
    int value;
    AVLNode *left;
    AVLNode *right;
    int height;

    AVLNode(int k, int v);
};

class AVLTree
{
   private:
    AVLNode *root;
    size_t size;

    int get(AVLNode *node, int key);
    int height(AVLNode *n);
    int getBalance(AVLNode *n);
    AVLNode *rightRotate(AVLNode *y);
    AVLNode *leftRotate(AVLNode *x);
    AVLNode *insert(AVLNode *node, int key, int value);
    AVLNode *deleteNode(AVLNode *root, int key);
    void inorderTraversal(AVLNode *node,
                          std::vector<std::pair<int, int> > &result, int key1, int key2);
    void clear(AVLNode *node);
    AVLNode *minValueNode(AVLNode *node);
    
   public:
    AVLTree();
    void put(int key, int value);
    int get(int key);
    std::vector<std::pair<int, int> > scan(int key1, int key2);
    void clear();
    size_t getSize();
};

#endif
