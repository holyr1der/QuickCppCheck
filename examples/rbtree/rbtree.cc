#include <sstream>
#include <cassert>

#include "rbtree.hpp"

#ifdef DEBUG
#define PRINT_DEBUG(X,Y) do {\
        std::cout<<"\n=================== "<<X<<" =======================\n";\
        std::cout<<Y;\
        std::cout<<"\n==========================================\n";\
        std::cout.flush();\
    } while (0)
#else
#define PRINT_DEBUG(X,Y)
#endif

namespace rbtree {

template<class T>
bool RBTree<T>::find(const T& t) const {
    RBNode<T> *cur = m_root;
    while (cur) {
        if (cur->data == t) {
            return true;
        }
        if (cur->data < t) {
            cur = cur->right;
        } else {
            cur = cur->left;
        }
    }
    return false;
}

template<class T>
inline bool RBTree<T>::_isBlackOrNull(RBNode<T> *node)
{
    return !node || node->color == Color::BLACK;
}

template<class T>
inline bool RBTree<T>::_isRed(RBNode<T> *node)
{
    return node && node->color == Color::RED;
}

template<class T>
inline RBNode<T>* RBTree<T>::_sibling(RBNode<T> *node) {
    RBNode<T> *ret = nullptr;
    if (node && node->parent) {
        if (node == node->parent->right)
            ret = node->parent->left;
        else
            ret = node->parent->right;
    }
    assert(ret);
    return ret;
}

template<class T>
inline RBNode<T>* RBTree<T>::_grandparent(RBNode<T> *node) {
    if (node != nullptr && node->parent != nullptr)
        return node->parent->parent;
    return nullptr;
}

template<class T>
inline RBNode<T>* RBTree<T>::_uncle(RBNode<T> *node) {
    RBNode<T> *gp = _grandparent(node);
    if (gp == nullptr)
        return nullptr;
    if (node->parent == gp->left)
        return gp->right;
    else
        return gp->left;
}

template<class T>
inline void RBTree<T>::_rotate_left(RBNode<T> *node) {
    RBNode<T> *pivot = node->right;
    node->right = pivot->left;
    pivot->left = node;
    if (node->right)
        node->right->parent = node;

    if (node->parent) {
        if (node == node->parent->left) {
            node->parent->left = pivot;
        } else {
            node->parent->right = pivot;
        }
    }
    pivot->parent = node->parent;
    node->parent = pivot;
    if (m_root == node)
        m_root = pivot;
}

template<class T>
inline void RBTree<T>::_rotate_right(RBNode<T> *node) {
    RBNode<T> *pivot = node->left;
    node->left = pivot->right;
    pivot->right = node;
    if (node->left)
        node->left->parent = node;

    if (node->parent) {
        if (node == node->parent->left) {
            node->parent->left = pivot;
        } else {
            node->parent->right = pivot;
        }
    }
    pivot->parent = node->parent;
    node->parent = pivot;
    if (m_root == node)
        m_root = pivot;
}

template<class T>
void RBTree<T>::insert(const T& t) {
    RBNode<T> *cur = m_root, *parent = nullptr;
    while (cur) {
        if (cur->data == t) {
            return;
        }
        parent = cur;
        if (cur->data < t) {
            cur = cur->right;
        } else {
            cur = cur->left;
        }
    }

    cur = new RBNode<T>(t);
    ++m_size;

    cur->parent = parent;
    if (parent) {
        if (parent->data > cur->data)
            parent->left = cur;
        else
            parent->right = cur;
    }

    while (_isRed(cur->parent)) {
        RBNode<T> *uncle = _uncle(cur);
        RBNode<T> *gp = _grandparent(cur);

        if (_isRed(uncle)) {
            cur->parent->color = Color::BLACK;
            uncle->color = Color::BLACK;
            gp->color = Color::RED;
            cur = gp;
        } else {
            if (cur == cur->parent->right
                    && cur->parent == gp->left) {
                _rotate_left(cur->parent);
                cur = cur->left;
            } else if (cur == cur->parent->left
                    && cur->parent == gp->right) {
                _rotate_right(cur->parent);
                cur = cur->right;
            }
            gp = _grandparent(cur);
            cur->parent->color = Color::BLACK;
            gp->color = Color::RED;
            if (cur == cur->parent->left)
                _rotate_right(gp);
            else
                _rotate_left(gp);
        }
    }

    if (cur->parent == nullptr)
        m_root = cur;
    m_root->color = Color::BLACK;
}

template<class T>
void RBTree<T>::remove(const T & t)
{
    RBNode<T> *target = m_root, *source = nullptr;
    while (target) {
        if (target->data == t)
            break;
        if (target->data < t)
            target = target->right;
        else
            target = target->left;
    }

    if (!target) return;

    --m_size;

    if (target->left) {
        source = target->left;
        while (source->right) source = source->right;
    } else if (target->right) {
        source = target->right;
        while (source->left) source = source->left;
    }

    if (source) {
        std::swap(target->data, source->data);
        target = source;
    }

    RBNode<T> *child = target->left?target->left:target->right;

    RBNode<T> *to_delete = target;
    if (!target->parent) {
        m_root = nullptr;
    } else  if (target->color == Color::BLACK) {
        if (child) {
            if (target->parent) {
                if (target == target->parent->left)
                    target->parent->left = child;
                else
                    target->parent->right = child;
                child->parent = target->parent;
            } else {
                m_root = child;
            }
            child->color = Color::BLACK;
        } else {
case1:
            if (target->parent) {
                RBNode<T> *sibling = _sibling(target);

                if (_isRed(sibling)) {
                    target->parent->color = Color::RED;
                    sibling->color = Color::BLACK;;
                    if (target == target->parent->left)
                        _rotate_left(target->parent);
                    else
                        _rotate_right(target->parent);
                    sibling = _sibling(target);
                }
                if (_isBlackOrNull(target->parent)
                        && _isBlackOrNull(sibling->left)
                        && _isBlackOrNull(sibling->right)) {
                    sibling->color = Color::RED;
                    target = target->parent;
                    goto case1;
                } else {
                    if (_isRed(target->parent)
                            && _isBlackOrNull(sibling->left)
                            && _isBlackOrNull(sibling->right)) {
                        sibling->color = Color::RED;
                        target->parent->color = Color::BLACK;
                    } else {
                        if (target == target->parent->left
                                && _isBlackOrNull(sibling->right)) {
                            sibling->color = Color::RED;
                            if (sibling->left)
                                sibling->left->color = Color::BLACK;
                            _rotate_right(sibling);
                        } else if (target == target->parent->right
                                && _isBlackOrNull(sibling->left)) {
                            sibling->color = Color::RED;
                            if (sibling->right)
                                sibling->right->color = Color::BLACK;
                            _rotate_left(sibling);
                        }
                        sibling = _sibling(target);
                        sibling->color = target->parent->color;
                        target->parent->color = Color::BLACK;
                        if (target == target->parent->left) {
                            if (sibling->right)
                                sibling->right->color = Color::BLACK;
                            _rotate_left(target->parent);
                        } else {
                            if (sibling->left)
                                sibling->left->color = Color::BLACK;
                            _rotate_right(target->parent);
                        }
                    }
                }
            }
            if (to_delete == to_delete->parent->left)
                to_delete->parent->left = nullptr;
            else if (to_delete == to_delete->parent->right)
                to_delete->parent->right = nullptr;
        }
    } else {
        if (target->parent)
            if (target == target->parent->left)
                target->parent->left = nullptr;
            else if (target ==  target->parent->right)
                target->parent->right = nullptr;
    }
    delete to_delete;
}


template<class T>
size_t RBTree<T>::_get_print_size(const RBNode<T> *node) const
{
    std::stringstream out;
#ifdef HAS_COLOR
    out<<"["<<node->data<<"]";
#else
    out<<"["<<((node->color==Color::RED)?"R":"B")<<","<<node->data<<"]";
#endif
    return out.str().size();
}

template<class T>
std::ostream& operator<<(std::ostream& out, const RBNode<T> & node)
{
#ifdef HAS_COLOR
    out<<"\033[1;3"<<((node.color==Color::RED)?"1":"4")<<"m"
        <<"["<<node.data<<"]"
        <<"\033[m";
#else
    out<<"["<<((node.color==Color::RED)?"R":"B")<<","<<node.data<<"]";
#endif
    return out;
}

void _print_line_with_branches(std::ostream& out, const std::vector<int> & indents)
{
    out<<std::endl;
    for (unsigned int i = 0;i < indents.size();++i) {
        if (indents[i] < 0) {
            // ancestor i has no more branches, print only spaces.
            for (int j = 0;j > indents[i];--j)
                out<<" ";
        } else {
            // ancestor i has still a branch, draw for him a vertical bar.
            for (int j = 0;j < indents[i] - 1;++j)
                out<<" ";
            //out<<"|";
            //out<<"\0xB3";
            out<<"\u2502";
        }
        out<<"      ";
    }
}

template<class T>
void RBTree<T>::print(std::ostream& out) const
{
    std::vector<int> indents;
    _print_tree_center(out, m_root, indents);
}

template<class T>
void RBTree<T>::_print_tree(std::ostream& out, const RBNode<T> *node,
        std::vector<int> & indents) const
{
    if (!node)
        return;
    // This is a little bit messy but works.
    // The idea is to keep a vector of extra indent lengths for ancestors,
    // along with the default indent, to compensate for the space nodes
    // themselves take; if the indent length for ancestor X is positive,
    // he has a left branch and we are not on that, so we print a vertical bar
    // and we fill the rest with spaces, if it's negative, we print only spaces.
    int extra_indent = _get_print_size(node);
    out<<*node;

    if (node->right) {
        //out<<"------";
        //out<<"\0xC4\0xC4\0xC4\0xC4\0xC4\0xC4";
        out<<"\u2500\u2500\u2500\u2500\u2500\u2500";
        if (node->left)
            indents.push_back(extra_indent);
        else
            indents.push_back(-extra_indent);
        _print_tree(out, node->right, indents);
        indents.pop_back();
    }
    if (node->left) {
        indents.push_back(extra_indent);
        _print_line_with_branches(out, indents);
        int last_indent = indents.back();
        indents.pop_back();
        _print_line_with_branches(out, indents);
        for (int i = 0;i < last_indent - 1;++i) {
            out<<" ";
        }
        //out<<"+------";
        //out<<"\0xC0\0xC4\0xC4\0xC4\0xC4\0xC4";
        out<<"\u2514\u2500\u2500\u2500\u2500\u2500\u2500";
        indents.push_back(-extra_indent);
        _print_tree(out, node->left, indents);
        indents.pop_back();
    }
}

template<class T>
void RBTree<T>::_print_tree_center(std::ostream& out, const RBNode<T> *node,
        std::vector<int> & indents) const
{
    if (!node)
        return;
    int extra_indent = _get_print_size(node) + 2;

    if (node->right) {
        indents.push_back(-extra_indent);
        _print_tree_center(out, node->right, indents);
        indents.pop_back();
    }
    if (indents.size() > 0) {
        int last_indent = indents.back();
        indents.pop_back();
        assert(abs(last_indent) > 0);
        _print_line_with_branches(out, indents);
        for (int i = 0;i < abs(last_indent) - 1;++i) {
            out<<" ";
        }
        out<<(last_indent>0?"\u2514":"\u250C");
        //out<<(last_indent>0?"\u2570":"\u256D");
        out<<"\u2500\u2500\u2500\u2500\u2500\u2500";
        indents.push_back(last_indent);
    } else {
        out<<std::endl;
    }
    out<<*node;
    if (node->right && node->left)
        out<<"\u2500\u2524";
    else if (node->right)
        out<<"\u2500\u2518";
    else if (node->left)
        out<<"\u2500\u2510";

    if (node->left) {
        if (indents.size() > 0)
            indents.back() = -indents.back();
        indents.push_back(extra_indent);
        _print_tree_center(out, node->left, indents);
        indents.pop_back();
    }
}

template<class T>
void RBTree<T>::_delete_all(RBNode<T>* node)
{
    if (!node)
        return;
    _delete_all(node->left);
    _delete_all(node->right);
    delete node;
    node = nullptr;
}

template<class T>
RBTree<T>::~RBTree()
{
    _delete_all(m_root);
}

template<class T>
std::ostream& operator<<(std::ostream& out, const RBTree<T> & tree)
{
    out<<std::endl;
    tree.print(out);
    out<<std::endl;

    return out;
}

} // namespace rbtree
