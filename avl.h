#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <string>

using namespace std;

// ===========================================================================
// CAVLNode: Nodo especializado para árboles AVL
// ===========================================================================
template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits> {
public:
    using Base = CBinaryTreeNode<Traits>;
    
protected:
    int m_balanceFactor = 0;
    
public:
    template<typename... Args>
    CAVLNode(Args&&... args)
        : Base(std::forward<Args>(args)...) {}
    
    int getBalance() const { return m_balanceFactor; }
    void setBalance(int bf) { m_balanceFactor = bf; }
    
    // Quité const para que compile
    CAVLNode* getLeft() { 
        return static_cast<CAVLNode*>(this->getChild(0)); 
    }
    
    CAVLNode* getRight() { 
        return static_cast<CAVLNode*>(this->getChild(1)); 
    }
    
    void setLeft(CAVLNode* n)  { this->setpChild(n, 0); }
    void setRight(CAVLNode* n) { this->setpChild(n, 1); }
};

// ===========================================================================
// Traits para AVL Ascendente
// ===========================================================================
template <typename ValueType>
struct AVLAscTraits {
    using T = ValueType;
    using Node = CAVLNode<AVLAscTraits<ValueType>>;
    using CompareFn = less<ValueType>;
};

// ===========================================================================
// Traits para AVL Descendente
// ===========================================================================
template <typename ValueType>
struct AVLDescTraits {
    using T = ValueType;
    using Node = CAVLNode<AVLDescTraits<ValueType>>;
    using CompareFn = greater<ValueType>;
};

// ===========================================================================
// CAVLTree: Árbol AVL con auto-balanceo
// ===========================================================================
template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base = CBinaryTree<Traits>;
    using value_type = typename Traits::T;
    using Node = typename Traits::Node;
    using CompareFn = typename Traits::CompareFn;
    using Container = CAVLTree<Traits>;
    
protected:
    // Rotación izquierda
    Node* rotateLeft(Node* pivotNode) {
        Node* newRoot = pivotNode->getRight();
        if (!newRoot) return pivotNode;
        
        Node* orphanSubtree = newRoot->getLeft();
        
        newRoot->setLeft(pivotNode);
        pivotNode->setRight(orphanSubtree);
        
        newRoot->setParent(pivotNode->getParent());
        pivotNode->setParent(newRoot);
        if (orphanSubtree) {
            orphanSubtree->setParent(pivotNode);
        }
        
        updateBalance(pivotNode);
        updateBalance(newRoot);
        
        return newRoot;
    }

    // Rotación derecha
    Node* rotateRight(Node* pivotNode) {
        Node* newRoot = pivotNode->getLeft();
        if (!newRoot) return pivotNode;
        
        Node* orphanSubtree = newRoot->getRight();
        
        newRoot->setRight(pivotNode);
        pivotNode->setLeft(orphanSubtree);
        
        newRoot->setParent(pivotNode->getParent());
        pivotNode->setParent(newRoot);
        if (orphanSubtree) {
            orphanSubtree->setParent(pivotNode);
        }
        
        updateBalance(pivotNode);
        updateBalance(newRoot);
        
        return newRoot;
    }

    // Calcular altura
    int getHeight(Node* n) const {
        if (!n) return 0;
        int lh = getHeight(n->getLeft());
        int rh = getHeight(n->getRight());
        return 1 + std::max(lh, rh);
    }

    // Calcular balance
    int getBalance(Node* n) const {
        if (!n) return 0;
        return getHeight(n->getRight()) - getHeight(n->getLeft());
    }
    
    // Actualizar balance
    void updateBalance(Node* n) {
        if (n) {
            n->setBalance(getBalance(n));
        }
    }

    // Balancear nodo
    Node* balanceNode(Node* node, const value_type& elem) {
        int bf = node->getBalance();
        CompareFn cmp;
        
        if (bf > 1) {
            if (cmp(elem, node->getRight()->getData())) {
                node->setRight(rotateRight(node->getRight()));
            }
            return rotateLeft(node);
        }
        
        if (bf < -1) {
            if (!cmp(elem, node->getLeft()->getData())) {
                node->setLeft(rotateLeft(node->getLeft()));
            }
            return rotateRight(node);
        }
        
        return node;
    }

    // Inserción con balanceo
    virtual Node* internal_insert(value_type elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override {
        CompareFn cmp;
        
        if (!rpOrigin) {
            Node* newNode = new Node(pParent, elem, ref);
            this->m_size++;
            rpOrigin = newNode;
            return newNode;
        }
        
        size_t branch = cmp(elem, rpOrigin->getDataRef()) ? 0 : 1;
        auto& childRef = rpOrigin->getChildRef(branch);
        Node* child = static_cast<Node*>(childRef);
        childRef = internal_insert(elem, ref, rpOrigin, child);
        
        updateBalance(rpOrigin);
        
        Node* balanced = balanceNode(rpOrigin, elem);
        if (balanced != rpOrigin) {
            rpOrigin = balanced;
        }
        
        return rpOrigin;
    }

public:
    CAVLTree() : Base() {}
    
    void insert(value_type elem, Ref ref = 0) {
        Node* root = static_cast<Node*>(this->m_pRoot);
        this->m_pRoot = internal_insert(elem, ref, nullptr, root);
    }
    
    // =======================================================================
    // write: Serializa el árbol a un stream de salida
    // Escribe los elementos en inorder, separados por espacios
    // Compatible con read() - NO incluye flechas ni formato extra
    // =======================================================================
    void write(std::ostream& os) {
        write_helper(static_cast<Node*>(this->m_pRoot), os);
        os << "\n";
    }
    
protected:
    // Helper recursivo para write (inorder sin formato)
    void write_helper(Node* node, std::ostream& os) {
        if (node) {
            write_helper(node->getLeft(), os);
            os << node->getData() << " ";
            write_helper(node->getRight(), os);
        }
    }
    
public:
    // =======================================================================
    // read: Deserializa el árbol desde un stream de entrada
    // Lee elementos separados por espacios y los inserta con balanceo
    // =======================================================================
    void read(std::istream& is) {
        value_type val;
        while (is >> val) {
            insert(val, 0);
        }
    };

#endif // __AVL_H__
