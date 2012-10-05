#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <iostream>
#include <vector>
#include <cassert>

namespace rbtree {

enum class Color {
    RED,
    BLACK
};

template<class T>
struct RBNode
{
    RBNode<T> *parent;
    RBNode<T> *left;
    RBNode<T> *right;

    Color color;
    T data;

    RBNode(const T& t):
        parent(nullptr), left(nullptr), right(nullptr), color(Color::RED), data(t){}
};

// A simple implementation of a Red-Black tree for demonstration purposes.
//
// WARNING: Class RBTree does not support "copy semantics",
// that is, any attempt to copy or assign an object of this class will fail
// at runtime with an assertion failure. It does, however, support
// move semantics, and this is what we actually care about here since
// QuickCppCheck does not copy any value, it only moves them.
template<class T>
class RBTree
{
private:
    size_t m_size;
    RBNode<T> *m_root;

    bool _isBlackOrNull(RBNode<T>*);
    bool _isRed(RBNode<T>*);
    RBNode<T>* _sibling(RBNode<T>*);
    RBNode<T>* _uncle(RBNode<T>*);
    RBNode<T>* _grandparent(RBNode<T>*);
    void _rotate_left(RBNode<T>*);
    void _rotate_right(RBNode<T>*);
    void _delete_all(RBNode<T>*);
    void _print_tree(std::ostream&, const RBNode<T> *, std::vector<int>&) const;
    void _print_tree_center(std::ostream&, const RBNode<T> *, std::vector<int>&) const;
    size_t _get_print_size(const RBNode<T>*) const;

public:
    // Makes an empty RBTree.
    RBTree():m_size(0),m_root(nullptr) {}

    // No copy constructor & copy assignment operator.
    // We can't delete or make them private since quickcppcheck
    // requires them in order to compile.
    // TODO: implement them!
    RBTree(const RBTree<T>&) {
        assert(false);
    }
    RBTree<T>& operator=(const RBTree<T>&) {
        assert(false);
        return *this;
    }

    // Move operations.
    RBTree(RBTree<T> && other):m_size(other.m_size),m_root(other.m_root){
        other.m_root = nullptr;
        other.m_size = 0;
    }
    RBTree<T> & operator=(RBTree<T> && other) {
        if (this != &other) {
            std::swap(this->m_root, other.m_root);
            std::swap(this->m_size, other.m_size);
        }
        return *this;
    }

    ~RBTree();

    void insert(const T&);
    bool find(const T&) const;
    void remove(const T&);
    void print(std::ostream&) const;
    RBNode<T>* root() const { return m_root; }
    size_t size() const { return m_size; }
};

} // namespace rbtree

#include "rbtree.cc"

#endif //_RBTREE_H_
