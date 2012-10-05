#include <set>
#include <unistd.h>

#include "rbtree.hpp"
#include "../../quickcppcheck/property.hpp"

using namespace qcppc;
using namespace rbtree;

//----------------------------------------------------------------------
//--------------- RBTree random generators -----------------------------
//
// Make RBTree instance of Arbitrary template class.
namespace qcppc {

template<class T>
struct Arbitrary<RBTree<T>> : ArbitraryBase
{
    std::function<T()> gen;
    std::uniform_int_distribution<size_t> dist;

    Arbitrary(size_t low = 0, size_t high = 50):
        Arbitrary(Arbitrary<T>(), low, high) {}

    Arbitrary(const std::function<T()>& gen, size_t low, size_t high):
            gen(gen), dist(low, high)
    {}

    RBTree<T> operator()() {
        RBTree<T> tree;
        size_t n = dist(engine);
        for (size_t i = 0;i < n;++i) {
            tree.insert(gen());
        }
        return tree;
    }
};

} // namespace qcppc

// Creates an RBTree from a vector.
template<class T>
RBTree<T> fromVector(const std::vector<T> & vals) {
    RBTree<T> tree;
    for (auto &v:vals) tree.insert(v);
    return tree;
}
// -------------------------------------------------------------------
// -------------------------------------------------------------------


//---------------------------------------------------------------------
//----------------- RBTree property checkers --------------------------
//
// Checks that a given RBTree is a valid BST
template<class T>
bool _is_valid_BST0(RBNode<T>* root, RBNode<T>* prev)
{
    if (root) {
        if (!_is_valid_BST0(root->left, prev)) {
            return false;
        }
        if (prev && root->data <= prev->data)
            return false;
        prev = root;
        return _is_valid_BST0(root->right, prev);
    }
    return true;
}
template<class T>
bool _is_valid_BST(const RBTree<T>& tree)
{
    RBNode<T> * prev = NULL;
    return _is_valid_BST0(tree.root(), prev);
}

// Checks that in a given RBTree, both children of all RED nodes
// are BLACK (NULL leaves are considered to be BLACK).
template<class T>
bool _all_red_node_children_are_black2(RBNode<T>* root)
{
    return !root || (root->color == Color::RED?(!root->left || root->left->color == Color::BLACK)
                                    &&  (!root->right || root->right->color == Color::BLACK)
                                        :true)
                        && _all_red_node_children_are_black2(root->left)
                        && _all_red_node_children_are_black2(root->right);
}

template<class T>
bool _all_red_node_children_are_black(const RBTree<T>& tree)
{
    return _all_red_node_children_are_black2(tree.root());
}

// Checks that in a given RBTree, all paths from any node to all its
// descendant leaves, have the same number of BLACK nodes.
template<class T>
bool _n_black_nodes2(RBNode<T>* root, int &n)
{
    if (!root) return ++n;

    int l = 0, r = 0;
    bool true_left = _n_black_nodes2(root->left, l),
         true_right = _n_black_nodes2(root->right, r);
    n = l + (root->color == Color::BLACK);
    return true_left && true_right && l == r;
}

template<class T>
bool _n_black_nodes(const RBTree<T>& tree)
{
    int n = 0;
    return _n_black_nodes2(tree.root(), n);
}

// Returns the shortest and the longest path in a given RBTree.
template<class T>
std::tuple<size_t, size_t> _get_minmax_paths(RBNode<T>* root)
{
    if (!root) 
        return std::make_tuple(0, 0);
    size_t min_left, min_right, max_left, max_right;
    std::tie(min_left, max_left) = _get_minmax_paths(root->left);
    std::tie(min_right, max_right) = _get_minmax_paths(root->right);

    return std::make_tuple(std::min(min_left, min_right) + 1,
                           std::max(max_left, max_right) + 1);
}

// Checks that in a given RBTree the longest path is not more
// than twice the shortest.
template<class T>
bool _longest_path_not_more_than_twice_the_shortest(const RBTree<T>& tree)
{
    auto minmax = _get_minmax_paths(tree.root());
    return std::get<0>(minmax) * 2 >= std::get<1>(minmax);
}
// -------------------------------------------------------------------
// -------------------------------------------------------------------


// -------------------------------------------------------------------
// Helper struct to test RBTree against std::set
template<class T>
struct RBTreeVsStdSet
{
    RBTree<T> tree;
    std::set<T> set;

    void insert(const T & t) {
        tree.insert(t);
        set.insert(t);
    }
    void remove(const T & t) {
        tree.remove(t);
        set.erase(t);
    }
    bool findBoth(const T & t) const {
        return tree.find(t) == (set.find(t) != set.end());
    }

    bool sizeBoth() const {
        return tree.size() == set.size();
    }
};
// --------------------------------------------------------------------


// -------------------------------------------------------------------
// ------------- Tests -----------------------------------------------
//
// Checks properties for find operation.
template<class T>
void test_find(const std::string & id)
{
    property([](RBTree<T>& tree, const std::vector<T>& v)
            { for (auto i:v) tree.insert(i);
              for (auto i:v) if (!tree.find(i)) return false;
              return true; },
            id + "[Find] All values inserted in a RBTree should be found.")
        // generate vectors of up to 1000 elements
        .template Rnd<1>(0, 1000)
        ();

    /*
    RBTree<T> tree;
    auto vals = Arbitrary<std::vector<T>>(Arbitrary<T>(), 1000, 4000)();
    for (auto &v:vals) tree.insert(v);

    property(std::mem_fun_ref(&RBTree<T>::find),
            "All values inserted in a RBTree should be found.")
        .template Fix<0>(tree)
        .template One<1>(vals)
    ();
    */
}

// Checks properties for insert operation.
// We can check directly the random generated trees, since
// they are generated using the insert operation.
template<class T>
void test_insert(const std::string & id)
{
    property(_is_valid_BST<T>,
        id + "[Insert] A Red-Black Tree is a BST.")
    ();

    property(_all_red_node_children_are_black<T>,
        id + "[Insert] All children of RED nodes should be BLACK (or NULL).")
    ();

    property(_longest_path_not_more_than_twice_the_shortest<T>,
        id + "[Insert] Longest path to a leaf should not be more than twice the shortest.")
    ();

    property(_n_black_nodes<T>,
        id + "[Insert] All paths from a given node to leaves should have the same number of black nodes.")
    ();
}

template<class T>
struct test_size;

template<>
struct test_size<int> { static const int min = 0; static const int max = 1000; };

template<>
struct test_size<std::string> { static const int min = 0; static const int max = 10; };

template<class T>
struct test_size<RBTree<T>> { static const int min = 0; static const int max = 1000; };

// Checks properties for remove operation.
template<class T>
void test_remove(const std::string & id)
{
    int min_tree_size = test_size<RBTree<T>>::min,
        max_tree_size = test_size<RBTree<T>>::max;
    auto gen = Arbitrary<T>(test_size<T>::min, test_size<T>::max);

    property([](RBTree<T> &tree, const T& t)
            { tree.insert(t);
              tree.remove(t);
              return !tree.find(t); },
        id + "[Remove] Removed elements should not be found.")
    ();

    property([](RBTree<T> &tree, const T& t)
            { tree.remove(t);
              return _is_valid_BST(tree); },
        id + "[Remove] Tree remains valid BST.",1)
        .template Rnd<0>(gen, min_tree_size, max_tree_size)
        .template Rnd<1>(gen)
    ();

    property([](RBTree<T> &tree, const T& t)
            { tree.remove(t);
              return _all_red_node_children_are_black(tree);},
        id + "[Remove] All children of RED nodes should be BLACK (or NULL).")
        .template Rnd<0>(gen, min_tree_size, max_tree_size)
        .template Rnd<1>(gen)
        .Classify([] (const RBTree<T> &tree, const T& t)
                { return tree.find(t)?"Present in tree":"Not present in tree";})
    ();

    property([](RBTree<T>& tree, const T& t)
            { tree.remove(t);
              return _longest_path_not_more_than_twice_the_shortest(tree);},
        id + "[Remove] Longest path to a leaf should not be more than twice the shortest.")
        .template Rnd<0>(gen, min_tree_size, max_tree_size)
        .template Rnd<1>(gen)
        .Classify([] (const RBTree<T> &tree, const T& t)
                { return tree.find(t)?"Present in tree":"Not present in tree";})
    ();

    /*
    std::vector<T> v = Arbitrary<std::vector<T>>(Arbitrary<T>(0,500), 100, 200)();
    RBTree<T> tree = fromVector(v);
    property([&tree](const T& t)
            { tree.remove(t);
              return _all_red_node_children_are_black(tree); },
        id + "[Remove] All red node children are black.",1)
        .template One<0>(v)
        // accept a value only if it's already in the tree
        //.If(std::bind(&RBTree<int>::find, std::ref(tree), std::placeholders::_1))
        .Classify([&tree] (const T& t)
                { return tree.find(t)?"Present in tree":"Not present in tree";})
    ();

    //v = Arbitrary<std::vector<T>>(Arbitrary<T>(0,1000), 200, 200)();
    tree = fromVector(v);
    property([&tree](const T& t)
            { tree.remove(t);
              return _is_valid_BST(tree);
              },
        id + "[Remove] Tree remains valid BST.",1)
        .template One<0>(v)
        // accept a value only if it's already in the tree
        //.If(std::bind(&RBTree<int>::find, std::ref(tree), std::placeholders::_1))
        .Classify([&tree] (const T& t)
                { return tree.find(t)?"Present in tree":"Not present in tree";})
    ();
    */
}

template<class T>
void test_remove2()
{
    std::vector<T> v = Arbitrary<std::vector<T>>(Arbitrary<T>(0,10000), 2000, 2000)();
    RBTree<T> tree = fromVector(v);
    property([&tree](T n)
            { tree.remove(n);
              return _all_red_node_children_are_black(tree); },
        "[Remove] All red node children are black.",1)
        .template One<0>(v)
        .Classify([&tree] (T n)
                { return tree.find(n)?"Present in tree":"Not present in tree";})
    ();

    auto gen = Arbitrary<T>(0, 10);
    property([] (RBTree<T> &tree, const T &t)
            { tree.remove(t);
              return _all_red_node_children_are_black(tree); },
        "[Remove] All red node children are black.",1)
        .Classify([] (const RBTree<T> &tree, const T &t)
            { return tree.find(t)?"Present in tree":"Not present in tree";})
        .template Rnd<0>(gen, 1000, 2000)
        .template Rnd<1>(gen)
    ();
}

template<class T>
void test_set()
{
    RBTreeVsStdSet<T> ts;

    property([&ts] (const T & t, bool op)
            { if (op) ts.insert(t); else ts.remove(t);
              return ts.findBoth(t) && ts.sizeBoth(); },
        "RBTree should behave similarly to std::set.")
        .template Rnd<0>(0, 1000)
        .template Frq<1>({{true, 2},{false, 1}})
    (1000000);

}
// --------------- Tests ------------------------------------------
// ----------------------------------------------------------------


template<class T>
void animate(int n)
{
    auto gen = Arbitrary<T>();
    RBTree<T> tree;
    while (n--) {
        sleep(1);
        tree.insert(gen());
        std::cout<<tree;
    }
}

int main()
{
    test_find<int>("int ");
    test_find<std::string>("string ");
    test_insert<int>("int ");
    test_insert<std::string>("string ");
    test_remove<int>("int ");
    test_remove<std::string>("string ");
    //test_remove2<int>();
    //test_remove2<std::string>();
    test_set<int>();
    return 0;
}

