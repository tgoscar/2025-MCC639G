#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
//#include <utility>
//#include <algorithm>
#include <cassert>
#include <vector>
#include "types.h"
//#include "util.h"
using namespace std;


template <typename Traits>
class CBinaryTreeNode{
public:
  // TODO: Change T by KeyNode
  using value_type = typename Traits::value_type;
  using Node = CBinaryTreeNode<Traits>;

private:
    value_type       m_data;
    Node *  m_pParent = nullptr;
    Ref     m_ref;
    vector<Node *> m_pChild = {nullptr, nullptr}; // 2 hijos inicializados en nullptr
public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_pParent(pParent), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }

    // TODO: Keynode 
    value_type         getData()                {   return m_data;    }
    value_type        &getDataRef()             {   return m_data;    }
 
 // TODO: review if these functions must remain public/private
    void      setpChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
};

// TODO: Implement binary_tree_iterator properly
// For now, commenting out incomplete iterator implementation
/*
template <typename Container>
class binary_tree_iterator {  
public:
    using Node   = typename Container::Node;
    using value_type = typename Container::value_type;

private:
    Container *m_pContainer = nullptr;
    Node      *m_pNode = nullptr;

public:
    binary_tree_iterator(Container *pContainer, Node *pNode) 
        : m_pContainer(pContainer), m_pNode(pNode) {}
    binary_tree_iterator(binary_tree_iterator &other)  
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}
    binary_tree_iterator(binary_tree_iterator &&other) 
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}

public:
    // TODO: Fuentes Patrick - implement proper tree traversal
    binary_tree_iterator operator++() {
        // TODO: implement in-order traversal
        m_pNode = nullptr;
        return *this;
    }
    
    bool operator==(binary_tree_iterator other){ 
        return m_pContainer == other.m_pContainer && m_pNode == other.m_pNode; 
    }
    bool operator!=(binary_tree_iterator other){ return !(*this == other); }
    value_type &operator*(){    return m_pNode->getDataRef();   }
};
*/

template <typename _T>
struct BinaryTreeAscTraits{
    using  value_type = _T;
    using  Node       = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  Func       = less<value_type>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  value_type = _T;
    using  Node       = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using  Func       = greater<value_type>;
};

template <typename Traits>
class CBinaryTree{
  public:
    using value_type    = typename Traits::value_type;
    using Node          = CBinaryTreeNode<Traits>;
    
    using Func          = typename Traits::Func;
    using Container     = CBinaryTree<Traits>;
    // using iterator      = binary_tree_iterator<Container>; // TODO: uncomment when iterator is implemented

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    Func Compfn;
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }
    // TODO: insert must receive two paramaters: elem and LinkedValueType value
    virtual void insert(value_type &elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, nullptr, m_pRoot);
    }

protected:
    Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    Node* internal_insert(value_type &elem, Ref ref, void* value,
                          Node* pParent, Node*& rpOrigin)
    {
        if (!rpOrigin) {
            ++m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        return internal_insert(elem, ref, nullptr, rpOrigin, rpOrigin->getChildRef(branch));
    }
public:
    CBinaryTree() = default;
    
    // TODO: Selis Luis (Copy Constructor)
    CBinaryTree(Container &other);
    
    CBinaryTree(Container &&other)
        : m_pRoot(std::move(other.m_pRoot)), 
          m_size (std::move(other.m_size)), 
          Compfn (std::move(other.Compfn))
    { }

    // TODO: Selis Luis (Destructor)
    virtual ~CBinaryTree(){  } 
    
    // TODO: Quispe David
        void inorder  (ostream &os)    {   inorder  (m_pRoot, os, 0);  }
    // TODO: Quispe David
    void inorder(Node  *pNode, ostream &os, size_t level){
        if( pNode ){
            //Node *pParent = pNode->getParent();
            inorder(pNode->getChild(0), os, level+1);
            os << " --> " << pNode->getDataRef();
            inorder(pNode->getChild(1), os, level+1);
        }
    }

    // TODO: generalize this function by using iterators and apply any function
    // TODO: Quispe David
    void inorder(Node  *pNode, void (*visit) (value_type& item)){
        if( pNode ){   
            inorder(pNode->getChild(0), *visit);
            (*visit)(pNode->getDataRef());
            inorder(pNode->getChild(1), *visit);
        }
    }

    // Variadic templates (See foreach.h)
    template <typename Function, typename... Args>
    void postorder(Function func, Args const&... args)
    {    postorder(m_pRoot, 0, func, args...);}

    template <typename Function,typename... Args>
    void postorder(Node* pNode, size_t level, Function func, Args const&... args) {
        if (pNode) {
            postorder(pNode->getChild(0), level + 1, func, args...);
            postorder(pNode->getChild(1), level + 1, func, args...);
            func(pNode, level); 
        }
    }
    // TODO: Villanueva Richard
    void preorder (ostream &os)    {   preorder (m_pRoot, 0, os);  }
    // TODO: Generalize this function by using iterators and apply any function
    // Create a new iterator to walk in postorder
    // TODO: Villanueva Richard
    void preorder(Node  *pNode, size_t level, ostream &os){
        //foreach(preorderbegin(), preorderend(), fn)
        if( pNode ){   
            os << " --> " << pNode->getDataRef();
            preorder(pNode->getChild(0), level+1, os);
            preorder(pNode->getChild(1), level+1, os);            
        }
    }

    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
    // TODO: generalize this function by using iterators and apply any function
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            os << string(level * 3, ' ') << " | " << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            print(pNode->getChild(0), level+1, os);
        }
    }

    // TODO: Open question for everyone
    // Generalizar el recorrido para recibir cualquier funcion
    // con una cantidad flexible de parametros conm variadic templates
    // https://en.cppreference.com/w/cpp/language/parameter_packs
    
    // TODO: Alcazar Joseph
    void postorder(Node  *pNode, size_t level, ostream &os){
        //foreach(postorderbegin(), postorderend(), fn)
        if( pNode ){   
            postorder(pNode->getChild(0), level+1, os);
            postorder(pNode->getChild(1), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }

    // TODO: Arriola Aldo
    void Write(ostream &os) { os << *this;  }

    // TODO: Toledo Oscar
    void Read(istream &is)  { /* TODO */  }
};

// TODO: Arriola Aldo
// operator <<
template <typename Traits>
ostream & operator<<(ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.preorder(os);
    return os;
}

// TODO: Toledo Oscar
template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    // Leer el arbol
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__