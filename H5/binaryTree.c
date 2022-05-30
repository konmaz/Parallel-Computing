#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
#include <math.h>

#include "tree.h"

/**
 * Implementation of a standard binary tree.
 */

#define RIGHT 0
#define LEFT 1

// Our Node implementation is as simple as it gets for a binary tree.
struct Node {
    long value;
    Tree left, right;
};

// We predefine some functions here that are not included in the header file,
// but that are used by our implementation
Tree emptyTree();

Tree newTree(long val, Tree treeLeft, Tree treeRight);

void registerValue(Tree *root, long val);

long computeSumSerial(Tree root);

/**
 * Construction operations
 */

Tree emptyTree() {
    return NULL;
}

Tree newTree(long val, Tree treeLeft, Tree treeRight) {
    Tree new = malloc(sizeof(*new));
    new->value = val;
    new->left = treeLeft;
    new->right = treeRight;

    return new;
}

Tree constructFromArray(long *arr, int length) {
    Tree new = malloc(sizeof(*new));
    new->value = arr[0];
    new->left = NULL;
    new->right = NULL;

    for (int i = 1; i < length; i++) {
        registerValue(&new, arr[i]);
    }

    return new;
}

Tree constructSequentialTree(int N) {
    Tree new = malloc(sizeof(*new));
    new->value = 0;
    new->left = NULL;
    new->right = NULL;

    Tree curr_node = new;
    for (int i = 1; i < N; i++) {
        registerValue(&curr_node, (long) i);
        curr_node = curr_node->right;
    }

    return new;
}

void freeTree(Tree root) {
    if (root->left != NULL) {
        freeTree(root->left);
    }
    if (root->right != NULL) {
        freeTree(root->right);
    }

    free(root);
}


/*
 *  Operations on tree
 */

void registerValue(Tree *root, long val) {
    assert(root != NULL && *root != NULL);

    Tree curr_node = *root;
    Tree prev_node = curr_node;
    int direction = -1;

    while (curr_node != NULL) {
        prev_node = curr_node;
        if (curr_node->value < val) {
            // Add to right side of sub-tree
            direction = RIGHT;
            curr_node = curr_node->right;
        } else {
            // Add to left side of sub-tree
            direction = LEFT;
            curr_node = curr_node->left;
        }
    }

    Tree new_node = newTree(val, NULL, NULL);

    // Add new node to the original tree
    if (direction == LEFT) {
        prev_node->left = new_node;
    } else {
        prev_node->right = new_node;
    }
}

int computeDepth(Tree root) {
    if (root == NULL) {
        return 0;
    }

    int depthLeft = computeDepth(root->left);
    int depthRight = computeDepth(root->right);

    return 1 + (depthLeft > depthRight ? depthLeft : depthRight);
}

long computeSumSerial(Tree root) {
    if (root == NULL) {
        return 0;
    }
    long val = root->value;
    val += computeSumSerial(root->left);
    val += computeSumSerial(root->right);
    return val;
}

long traverse(Tree root){
    if (root == NULL) {
        return 0;
    }
    long val = root->value;
    long v1 = 0, v2 =0;
    #pragma omp task shared(v1)
    {
        v1 = computeSumSerial(root->left);
    }
    #pragma omp taskwait
    {
        v2 = computeSumSerial(root->right);
    }
    #pragma omp taskwait
    return val + v1 + v2;
}
long parallelSum(Tree root, int threads) {
    long s;
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp single
        s =  traverse(root);
    }
    return s;
}
