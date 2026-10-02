#include "mydb.hpp"

#include <cassert>

int main() {
    BTree* tree = btree_create();
    btree_insert(tree, 10);
    btree_insert(tree, 20);
    btree_insert(tree, 5);
    btree_insert(tree, 15);
    btree_insert(tree, 25);

    assert(btree_contains(tree, 5));
    assert(btree_contains(tree, 10));
    assert(btree_contains(tree, 15));
    assert(btree_contains(tree, 20));
    assert(btree_contains(tree, 25));
    assert(!btree_contains(tree, 99));

    btree_destroy(tree);
    return 0;
}
