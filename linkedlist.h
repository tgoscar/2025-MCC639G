#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <iostream>
#include "types.h"

template <typename T>
class LLNode{
private:
    using    value_type = T;
    using    Node = LLNode<T>;
    using    MySelf=LLNode<T>;
    value_type     m_data;
    Ref      m_ref;
    Node    *m_pNext = nullptr;

public:
    LLNode(value_type &elem, Ref ref, LLNode<value_type> *pNext = nullptr)
        : m_data(elem), m_ref(ref), m_pNext(pNext){
    }
    value_type   GetData()    { return m_data;     }
    value_type  &GetDataRef() { return m_data;     }
    Ref    GetRef()     { return m_ref;      }
    Node * GetNext()    { return m_pNext;    }
    Node *&GetNextRef() { return m_pNext;    }
};

// TODO Activar el iterator
template <typename T>
class forward_linkedlist_iterator{
 private:
     using value_type = T;
     using Node       = LLNode<T>;
     using iterator   = forward_linkedlist_iterator<T>;
     using Container  = class CLinkedList<T>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     forward_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     forward_linkedlist_iterator(forward_linkedlist_iterator<T> &other)
             : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
     bool operator==(iterator other){ return m_pList == other.m_pList && m_pNode == other.m_pNode; }
     bool operator!=(iterator other){ return !(*this == other);    }

     iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetNext();
         return *this;
     }
     value_type &operator*(){    return m_pNode->GetDataRef();   }
};

// TODO Agregar control de concurrencia

// TODO Agregar que sea ascendente o descendente con el mismo codigo
template <typename T>
class CLinkedList{
private:
    using value_type = T; 
    using Node       = LLNode<value_type>; 
    using iterator   = forward_linkedlist_iterator<T>;

    Node *m_pRoot = nullptr;

public:
    // Constructor
    CLinkedList();
    CLinkedList(CLinkedList &other);

    // TODO: Move contructor (leer bibliografia)
    CLinkedList(CLinkedList &&other);

    // Destructor seguro
    virtual ~CLinkedList();

    void Insert(value_type &elem, Ref ref);
private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref);
    Node *GetRoot()    {    return m_pRoot;     };

    iterator begin(){ return forward_linkedlist_iterator(this, m_pRoot); };
    iterator end()  { return forward_linkedlist_iterator(this, nullptr); } 

    friend std::ostream& operator<<(std::ostream &os, CLinkedList<T> &obj){
        auto pRoot = obj.GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
            pRoot = pRoot->GetNext();
        }
        return os;
    }
};

template <typename T>
void CLinkedList<T>::Insert(value_type &elem, Ref ref){
    InternalInsert(m_pRoot, elem, ref);
}

template <typename T>
void CLinkedList<T>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    if( !rParent || elem < rParent->GetDataRef() ){
        rParent = new Node(elem, ref, rParent);
        return;
    }
    // Tail recursion
    InternalInsert(rParent->GetNextRef(), elem, ref);
}

template <typename T>
CLinkedList<T>::CLinkedList()
{
}

// TODO Constructor por copia
template <typename T>
CLinkedList<T>::CLinkedList(CLinkedList &other)
{
}

template <typename T>
CLinkedList<T>::CLinkedList(CLinkedList &&other)
{
}

template <typename T>
CLinkedList<T>::~CLinkedList()
{
}

// TODO: Este operador debe quedar fuera de a clase
template <typename T>
std::ostream &operator<<(std::ostream &os, CLinkedList<T> &obj){
    auto pRoot = obj.GetRoot();
    while( pRoot )
        os << pRoot->GetData() << " ";
    return os;
}

void DemoLinkedList();

#endif // __LINKEDLIST_H__