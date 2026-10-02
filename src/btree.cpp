#include "mydb.hpp"

#include <cstddef>

namespace {

constexpr uint32_t BTREE_MAX_CHILDREN = BTREE_MAX_KEYS + 1;

bool btree_search_node(const BTreeNode* node, uint32_t key) {
    uint32_t index = 0;
    while (index < node->num_keys && key > node->keys[index]) {
        ++index;
    }

    if (index < node->num_keys && key == node->keys[index]) {
        return true;
    }

    if (node->is_leaf) {
        return false;
    }

    return btree_search_node(node->children[index], key);
}

void btree_split_child(BTreeNode* parent, uint32_t child_index) {
    BTreeNode* child = parent->children[child_index];
    BTreeNode* right = new BTreeNode();
    right->is_leaf = child->is_leaf;
    right->num_keys = BTREE_MIN_DEGREE - 1;

    for (uint32_t i = 0; i < BTREE_MIN_DEGREE - 1; ++i) {
        right->keys[i] = child->keys[BTREE_MIN_DEGREE + i];
    }

    if (!child->is_leaf) {
        for (uint32_t i = 0; i < BTREE_MIN_DEGREE; ++i) {
            right->children[i] = child->children[BTREE_MIN_DEGREE + i];
        }
    }

    child->num_keys = BTREE_MIN_DEGREE - 1;

    for (uint32_t i = parent->num_keys; i > child_index; --i) {
        parent->children[i + 1] = parent->children[i];
    }

    parent->children[child_index + 1] = right;

    for (uint32_t i = parent->num_keys; i > child_index; --i) {
        parent->keys[i] = parent->keys[i - 1];
    }

    parent->keys[child_index] = child->keys[BTREE_MIN_DEGREE - 1];
    parent->num_keys += 1;
}

void btree_insert_non_full(BTreeNode* node, uint32_t key) {
    uint32_t index = node->num_keys;

    if (node->is_leaf) {
        while (index > 0 && key < node->keys[index - 1]) {
            node->keys[index] = node->keys[index - 1];
            --index;
        }

        node->keys[index] = key;
        node->num_keys += 1;
        return;
    }

    while (index > 0 && key < node->keys[index - 1]) {
        --index;
    }

    if (node->children[index]->num_keys == BTREE_MAX_KEYS) {
        btree_split_child(node, index);
        if (key > node->keys[index]) {
            ++index;
        }
    }

    btree_insert_non_full(node->children[index], key);
}

void btree_destroy_node(BTreeNode* node) {
    if (node == nullptr) {
        return;
    }

    if (!node->is_leaf) {
        for (uint32_t i = 0; i <= node->num_keys; ++i) {
            btree_destroy_node(node->children[i]);
        }
    }

    delete node;
}

}  // namespace

BTree* btree_create() {
    BTree* tree = new BTree();
    tree->root = new BTreeNode();
    tree->root->is_leaf = true;
    tree->root->num_keys = 0;
    return tree;
}

void btree_destroy(BTree* tree) {
    if (tree == nullptr) {
        return;
    }

    btree_destroy_node(tree->root);
    delete tree;
}

bool btree_contains(const BTree* tree, uint32_t key) {
    if (tree == nullptr || tree->root == nullptr) {
        return false;
    }

    return btree_search_node(tree->root, key);
}

void btree_insert(BTree* tree, uint32_t key) {
    if (tree == nullptr) {
        return;
    }

    BTreeNode* root = tree->root;
    if (root->num_keys == BTREE_MAX_KEYS) {
        BTreeNode* new_root = new BTreeNode();
        new_root->is_leaf = false;
        new_root->num_keys = 0;
        new_root->children[0] = root;
        tree->root = new_root;
        btree_split_child(new_root, 0);
    }

    btree_insert_non_full(tree->root, key);
}
