#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 

#include <cassert>
#include <vector>
#include <functional>
#include <string>
#include "types.h"

using namespace std;

// Forward declarations
template <typename Traits> class CBinaryTree;
template <typename Container> class binary_tree_iterator;

// Nodo del árbol binario
template <typename Traits>
class CBinaryTreeNode {
public:
    using T = typename Traits::T;
    using Node = CBinaryTreeNode<Traits>;

protected:
    T m_data;
    Node* m_pParent = nullptr;
    Ref m_ref;
    vector<Node*> m_pChild = {nullptr, nullptr}; // 2 hijos

public:
    CBinaryTreeNode(Node* pParent, T data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_pParent(pParent), m_ref(ref) {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }

    T getData() { return m_data; }
    T& getDataRef() { return m_data; }
    
    void setpChild(Node* pChild, size_t pos) { m_pChild[pos] = pChild; }
    Node* getChild(size_t branch) { return m_pChild[branch]; }
    Node*& getChildRef(size_t branch) { return m_pChild[branch]; }
    Node* getParent() { return m_pParent; }
    
    // NUEVO: Método para actualizar el padre
    void setParent(Node* pParent) { m_pParent = pParent; }
    
    // Para el iterador (placeholder)
    Node* getpNext() { return nullptr; } // TODO: implementar recorrido
};

// Iterador del árbol binario
template <typename Container>
class binary_tree_iterator {
public:
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;

protected:
    Container* m_pContainer = nullptr;
    Node* m_pNode = nullptr;

public:
    binary_tree_iterator(Container* pContainer, Node* pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}
    
    binary_tree_iterator(const binary_tree_iterator& other)
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}
    
    binary_tree_iterator operator++() {
        m_pNode = m_pNode ? m_pNode->getpNext() : nullptr;
        return *this;
    }
    
    bool operator==(const binary_tree_iterator& other) const {
        return m_pContainer == other.m_pContainer && m_pNode == other.m_pNode;
    }
    
    bool operator!=(const binary_tree_iterator& other) const {
        return !(*this == other);
    }
    
    value_type& operator*() { return m_pNode->getDataRef(); }
};

// Traits para árbol binario ascendente
template <typename _T>
struct BinaryTreeAscTraits {
    using T = _T;
    using Node = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using CompareFn = std::less<_T>;
};

// Traits para árbol binario descendente
template <typename _T>
struct BinaryTreeDescTraits {
    using T = _T;
    using Node = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using CompareFn = std::greater<_T>;
};

// Árbol binario de búsqueda
template <typename Traits>
class CBinaryTree {
public:
    using value_type = typename Traits::T;
    using Node = typename Traits::Node;
    using CompareFn = typename Traits::CompareFn;
    using Container = CBinaryTree<Traits>;
    using iterator = binary_tree_iterator<Container>;

protected:
    Node* m_pRoot = nullptr;
    size_t m_size = 0;
    CompareFn Compfn;

public:
    size_t size() const { return m_size; }
    bool empty() const { return size() == 0; }
    
    void insert(value_type elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

protected:
    Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    
    // IMPORTANTE: virtual para que AVL pueda hacer override
    virtual Node* internal_insert(value_type elem, Ref ref,
                                   Node* pParent, Node*& rpOrigin) {
        if (!rpOrigin) {
            ++m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        
        // Crear una referencia temporal al hijo para que los cambios se propaguen
        auto& childRef = rpOrigin->getChildRef(branch);
        Node* child = static_cast<Node*>(childRef);
        
        // Insertar recursivamente (child se modifica por referencia)
        Node* inserted = internal_insert(elem, ref, rpOrigin, child);
        
        // Actualizar el puntero del hijo
        childRef = child;
        
        return inserted;
    }

public:
    CBinaryTree() {}
    
    // Move constructor
    CBinaryTree(CBinaryTree&& other) noexcept
        : m_pRoot(other.m_pRoot), m_size(other.m_size) {
        other.m_pRoot = nullptr;
        other.m_size = 0;
    }
    
    // Move assignment
    CBinaryTree& operator=(CBinaryTree&& other) noexcept {
        if (this != &other) {
            // TODO: liberar memoria actual
            m_pRoot = other.m_pRoot;
            m_size = other.m_size;
            other.m_pRoot = nullptr;
            other.m_size = 0;
        }
        return *this;
    }
    
    virtual ~CBinaryTree() {
        // TODO: liberar memoria
    }
    
    // Recorridos
    void inorder(ostream& os) { inorder(m_pRoot, os, 0); }
    
    void inorder(Node* pNode, ostream& os, size_t level) {
        if (pNode) {
            inorder(static_cast<Node*>(pNode->getChild(0)), os, level + 1);
            os << " --> " << pNode->getDataRef();
            inorder(static_cast<Node*>(pNode->getChild(1)), os, level + 1);
        }
    }
    
    void preorder(ostream& os) { preorder(m_pRoot, 0, os); }
    
    void preorder(Node* pNode, size_t level, ostream& os) {
        if (pNode) {
            os << " --> " << pNode->getDataRef();
            preorder(static_cast<Node*>(pNode->getChild(0)), level + 1, os);
            preorder(static_cast<Node*>(pNode->getChild(1)), level + 1, os);
        }
    }
    
    void postorder(ostream& os) { postorder(m_pRoot, 0, os); }
    
    void postorder(Node* pNode, size_t level, ostream& os) {
        if (pNode) {
            postorder(static_cast<Node*>(pNode->getChild(0)), level + 1, os);
            postorder(static_cast<Node*>(pNode->getChild(1)), level + 1, os);
            os << " --> " << pNode->getDataRef();
        }
    }
    
    void print(ostream& os) { print(m_pRoot, 0, os); }
    
    void print(Node* pNode, size_t level, ostream& os) {
        if (pNode) {
            Node* pParent = static_cast<Node*>(pNode->getParent());
            print(static_cast<Node*>(pNode->getChild(1)), level + 1, os);
            for (size_t i = 0; i < level; ++i) os << " | ";
            os << pNode->getDataRef() << "(";
            if (pParent) os << pParent->getData();
            else os << "Root";
            os << ")" << endl;
            print(static_cast<Node*>(pNode->getChild(0)), level + 1, os);
        }
    }
    
    void Write(ostream& os) { os << *this; }
};

// Operador 
template <typename Traits>
ostream& operator<<(ostream& os, CBinaryTree<Traits>& obj) {
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.inorder(os);
    return os;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__