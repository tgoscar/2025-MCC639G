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
    CAVLNode() : Base(), m_balanceFactor(0) {}
    
    int getBalanceFactor() const { return m_balanceFactor; }
    void setBalanceFactor(int bf) { m_balanceFactor = bf; }
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
    using  Node       = CAVLNode<_T>;
    using  CompareFn  = greater<_T>;
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
    using AVLNode = CAVLNode<Traits>;
    // ========================================================================
    // Funciones auxiliares
    // ========================================================================
    
    int getHeight(Node* node) {
        if (!node) return 0;
        return 1 + std::max(getHeight(node->m_pLeft), getHeight(node->m_pRight));
    }
    
    int calculateBalance(Node* node) {
        if (!node) return 0;
        return getHeight(node->m_pLeft) - getHeight(node->m_pRight);
    }
    
    void updateBalanceFactor(Node* node) {
        if (node) {
            int bf = calculateBalance(node);
            static_cast<AVLNode*>(node)->setBalanceFactor(bf);
        }
    }
    
    // ========================================================================
    // Rotaciones
    // ========================================================================
    
    Node* rotateRight(Node* y) {
        Node* x = y->m_pLeft;
        Node* T2 = x->m_pRight;
        
        x->m_pRight = y;
        y->m_pLeft = T2;
        
        x->m_pParent = y->m_pParent;
        y->m_pParent = x;
        if (T2) T2->m_pParent = y;
        
        updateBalanceFactor(y);
        updateBalanceFactor(x);
        
        return x;
    }
    
    Node* rotateLeft(Node* x) {
        Node* y = x->m_pRight;
        Node* T2 = y->m_pLeft;
        
        y->m_pLeft = x;
        x->m_pRight = T2;
        
        y->m_pParent = x->m_pParent;
        x->m_pParent = y;
        if (T2) T2->m_pParent = x;
        
        updateBalanceFactor(x);
        updateBalanceFactor(y);
        
        return y;
    }
    
    // ========================================================================
    // Balanceo
    // ========================================================================
    
    Node* balance(Node* node) {
        if (!node) return node;
        
        updateBalanceFactor(node);
        int bf = static_cast<AVLNode*>(node)->getBalanceFactor();
        
        // Izquierda-Izquierda
        if (bf > 1 && calculateBalance(node->m_pLeft) >= 0) {
            return rotateRight(node);
        }
        
        // Derecha-Derecha
        if (bf < -1 && calculateBalance(node->m_pRight) <= 0) {
            return rotateLeft(node);
        }
        
        // Izquierda-Derecha
        if (bf > 1 && calculateBalance(node->m_pLeft) < 0) {
            node->m_pLeft = rotateLeft(node->m_pLeft);
            return rotateRight(node);
        }
        
        // Derecha-Izquierda
        if (bf < -1 && calculateBalance(node->m_pRight) > 0) {
            node->m_pRight = rotateRight(node->m_pRight);
            return rotateLeft(node);
        }
        
        return node;
    }
    // TODO: modificar la insercion para que mantenga
    //       el balance del arbo y realice las 
    //       rotaciones necesarias
    virtual Node *internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        // TODO 1. insertar
        Node* inserted = internal_insert(elem, ref, pParent, rpOrigin);
        
        // TODO 2. verificar balance
        if (inserted) {
            // TODO 2. verificar balance
            updateBalanceFactor(rpOrigin);
            
            // TODO 3. realizar rotaciones si es necesario
            rpOrigin = balance(rpOrigin);
        }
        
      
        return inserted;
    }
public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__
