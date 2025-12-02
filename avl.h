#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

// Nodo AVL hereda de CBinaryTreeNode
template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits> {
public:
    using value_type = typename Traits::T;
    using Base = CBinaryTreeNode<Traits>;
    
protected:
    int m_balanceFactor = 0;
    
public:
    CAVLNode(typename Base::Node* pParent, value_type data, Ref ref)
        : Base(pParent, data, ref), m_balanceFactor(0) {}
    
    int getBalanceFactor() const { return m_balanceFactor; }
    void setBalanceFactor(int bf) { m_balanceFactor = bf; }
};

template <typename _T>
struct AVLAscTraits {
    using T = _T;
    using Node = CAVLNode<AVLAscTraits<_T>>;
    using CompareFn = std::less<_T>;
};

template <typename _T>
struct AVLDescTraits {
    using T = _T;
    using Node = CAVLNode<AVLDescTraits<_T>>;
    using CompareFn = std::greater<_T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base = CBinaryTree<Traits>;
    using Node = typename Traits::Node;
    using value_type = typename Traits::T;
    using CompareFn = typename Traits::CompareFn;
    using Container = CAVLTree<Traits>;
    using iterator = binary_tree_iterator<Container>;
    using AVLNode = CAVLNode<Traits>;
    
protected:
    int getHeight(Node* node) {
        if (!node) return 0;
        return 1 + std::max(
            getHeight(static_cast<Node*>(node->getChild(0))), 
            getHeight(static_cast<Node*>(node->getChild(1)))
        );
    }
    
    int calculateBalance(Node* node) {
        if (!node) return 0;
        return getHeight(static_cast<Node*>(node->getChild(0))) - 
               getHeight(static_cast<Node*>(node->getChild(1)));
    }
    
    void updateBalanceFactor(Node* node) {
        if (node) {
            int bf = calculateBalance(node);
            static_cast<AVLNode*>(node)->setBalanceFactor(bf);
        }
    }
    
    Node* rotateRight(Node* nodeDesbalanceado) {
        Node* nuevoRoot = static_cast<Node*>(nodeDesbalanceado->getChild(0));
        
        if (!nuevoRoot) {
            return nodeDesbalanceado;
        }
        
        Node* subArbolTemp = static_cast<Node*>(nuevoRoot->getChild(1));
        Node* padre = static_cast<Node*>(nodeDesbalanceado->getParent());
        
        // Realizar la rotación - actualizar hijos
        nuevoRoot->setpChild(nodeDesbalanceado, 1);
        nodeDesbalanceado->setpChild(subArbolTemp, 0);
        
        // Actualizar los punteros padre
        nuevoRoot->setParent(padre);
        nodeDesbalanceado->setParent(nuevoRoot);
        if (subArbolTemp) {
            subArbolTemp->setParent(nodeDesbalanceado);
        }
        
        // Actualizar hijo del padre
        if (padre) {
            if (padre->getChild(0) == nodeDesbalanceado) {
                padre->setpChild(nuevoRoot, 0);
            } else {
                padre->setpChild(nuevoRoot, 1);
            }
        }
        
        updateBalanceFactor(nodeDesbalanceado);
        updateBalanceFactor(nuevoRoot);
        
        return nuevoRoot;
    }
    
    Node* rotateLeft(Node* nodeDesbalanceado) {
        Node* nuevoRoot = static_cast<Node*>(nodeDesbalanceado->getChild(1));
        Node* subArbolTemp = static_cast<Node*>(nuevoRoot->getChild(0));
        Node* padre = static_cast<Node*>(nodeDesbalanceado->getParent());
        
        // Realizar la rotación - actualizar hijos
        nuevoRoot->setpChild(nodeDesbalanceado, 0);
        nodeDesbalanceado->setpChild(subArbolTemp, 1);
        
        // Actualizar los punteros padre
        nuevoRoot->setParent(padre);
        nodeDesbalanceado->setParent(nuevoRoot);
        if (subArbolTemp) {
            subArbolTemp->setParent(nodeDesbalanceado);
        }
        
        // Actualizar hijo del padre
        if (padre) {
            if (padre->getChild(0) == nodeDesbalanceado) {
                padre->setpChild(nuevoRoot, 0);
            } else {
                padre->setpChild(nuevoRoot, 1);
            }
        }
        
        updateBalanceFactor(nodeDesbalanceado);
        updateBalanceFactor(nuevoRoot);
        
        return nuevoRoot;
    }
    
    Node* balance(Node* node) {
        if (!node) return node;
        
        updateBalanceFactor(node);
        int bf = static_cast<AVLNode*>(node)->getBalanceFactor();
        
        // Izquierda-Izquierda
        if (bf > 1 && calculateBalance(static_cast<Node*>(node->getChild(0))) >= 0) {
            return rotateRight(node);
        }
        
        // Derecha-Derecha
        if (bf < -1 && calculateBalance(static_cast<Node*>(node->getChild(1))) <= 0) {
            return rotateLeft(node);
        }
        
        // Izquierda-Derecha
        if (bf > 1 && calculateBalance(static_cast<Node*>(node->getChild(0))) < 0) {
            node->setpChild(rotateLeft(static_cast<Node*>(node->getChild(0))), 0);
            return rotateRight(node);
        }
        
        // Derecha-Izquierda
        if (bf < -1 && calculateBalance(static_cast<Node*>(node->getChild(1))) > 0) {
            node->setpChild(rotateRight(static_cast<Node*>(node->getChild(1))), 1);
            return rotateLeft(node);
        }
        
        return node;
    }
    
    Node* internal_insert(value_type elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override {
        
        // TODO 1. insertar - Implementación BST completa
        if (!rpOrigin) {
            this->m_size++;
            rpOrigin = static_cast<Node*>(this->CreateNode(pParent, elem, ref));
            return rpOrigin;
        }
        
        // Determinar la rama (izquierda=0 o derecha=1)
        CompareFn compare;
        size_t branch = compare(elem, rpOrigin->getDataRef()) ? 0 : 1;
        
        // Obtener referencia al hijo
        auto& childRef = rpOrigin->getChildRef(branch);
        Node* child = static_cast<Node*>(childRef);
        
        // Insertar recursivamente
        Node* inserted = internal_insert(elem, ref, rpOrigin, child);
        
        // Actualizar el puntero del hijo
        childRef = child;
        
        // TODO 2 y 3. verificar balance y realizar rotaciones si es necesario
        if (inserted) {
            updateBalanceFactor(rpOrigin);
            int bf = static_cast<AVLNode*>(rpOrigin)->getBalanceFactor();
            
            // Solo hacer rotación si hay desbalance (bf < -1 o bf > 1)
            if (bf > 1 || bf < -1) {
                rpOrigin = balance(rpOrigin);
            }
        }
        
        return inserted;
    }
    
public:
    CAVLTree() : Base() {}
    
    // Override del insert público
    void insert(value_type elem, Ref ref) {
        internal_insert(elem, ref, nullptr, this->m_pRoot);
    }
    
    // Métodos adicionales para diagnóstico
    void printBalance(std::ostream& os) {
        printBalance(this->m_pRoot, os);
        os << std::endl;
    }
    
    void printBalance(Node* node, std::ostream& os) {
        if (node) {
            printBalance(static_cast<Node*>(node->getChild(0)), os);
            os << node->getDataRef() << "(BF=" 
               << static_cast<AVLNode*>(node)->getBalanceFactor() << ") ";
            printBalance(static_cast<Node*>(node->getChild(1)), os);
        }
    }
    
    bool isBalanced() {
        return isBalanced(this->m_pRoot);
    }
    
    bool isBalanced(Node* node) {
        if (!node) return true;
        
        int bf = calculateBalance(node);
        if (bf < -1 || bf > 1) return false;
        
        return isBalanced(static_cast<Node*>(node->getChild(0))) && 
               isBalanced(static_cast<Node*>(node->getChild(1)));
    }
    
    bool search(value_type elem) {
        return search(this->m_pRoot, elem) != nullptr;
    }
    
    Node* search(Node* node, value_type elem) {
        if (!node) return nullptr;
        
        if (elem == node->getDataRef()) return node;
        
        CompareFn compare;
        if (compare(elem, node->getDataRef())) {
            return search(static_cast<Node*>(node->getChild(0)), elem);
        } else {
            return search(static_cast<Node*>(node->getChild(1)), elem);
        }
    }
};

#endif // __AVL_H__