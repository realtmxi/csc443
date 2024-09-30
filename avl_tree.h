#ifndef AVL_TREE_H
#define AVL_TREE_H

struct AVLNode {
    int key;
    int value;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(int k, int v);
};

class AVLTree {
private:
    AVLNode* root;
    AVLNode* insert(AVLNode* node, int key, int value);
    int get(AVLNode* node, int key);
    int height(AVLNode* n);
    int getBalance(AVLNode* n);
    AVLNode* rightRotate(AVLNode* y);
    AVLNode* leftRotate(AVLNode* x);

public:
    AVLTree();
    void put(int key, int value);
    int get(int key);
};

#endif
