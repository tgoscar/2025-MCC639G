#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<value_type>{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<value_type>;
protected:
    int     m_balanceFactor = 0; // Balance factor for AVL tree
public:
};

template <typename _T>
struct AVLAscTraits{
    using  value_type = _T;
    using  Node       = CAVLNode<T>;
    using  CompareFn  = less<T>;
};

template <typename _T>
struct AVLDescTraits{
    using  value_type = _T;
    using  Node       = CAVLNode<T>;
    using  CompareFn  = greater<T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::value_type;  
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    using iterator   = binary_tree_iterator<Container>;

protected:
    // Additional members for AVL tree balancing can be added here
    // TODO: modificar la insercion para que mantenga
    //       el balance del arbo y realice las 
    //       rotaciones necesarias
    Node *internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        // TODO 1. insertar
        // TODO 2. verificar balance
        // TODO 3. realizar rotaciones si es necesario
        
        return nullptr;
    }
public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__