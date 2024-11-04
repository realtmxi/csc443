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
    size_t size;

    // Helper function for AVL Tree implementation.
    int height(AVLNode *n);
    int getBalance(AVLNode *n);
    AVLNode *rightRotate(AVLNode *y);
    AVLNode *leftRotate(AVLNode *x);
    AVLNode *minValueNode(AVLNode *node);
    void clear(AVLNode *node);
    
  public:
    AVLTree();

    AVLNode *insert(AVLNode *node, int key, int value);
    AVLNode *deleteNode(AVLNode *root, int key);
    int search (int key);

    void inorderTraversal(AVLNode *node,
                          std::vector<std::pair<int, int> > &result, 
                          int key1, int key2);
    size_t getSize();
    void clear();
};

#endif
