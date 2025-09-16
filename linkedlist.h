#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

template <typename T>
class LLNode{
private:
    using Type = T;
    Type        m_data;
    LLNode<T>  *m_pNext = nullptr;

public:
    LLNode(Type &elem, LLNode<T> *pNext = nullptr)
        : m_data(elem), m_pNext(pNext){
    }
};

template <typename T>
class CLinkedList{
private:
    using Type = T;    
    // Node<Type> *m_pHead = nullptr;
public:
    // Constructor
    CLinkedList();
    // Destructor
    virtual ~CLinkedList();

    void insert(Type &elem);
private:
    // TODO: Implementar
    // void InternalInsert(Node<Type> *&rParent, Type &elem);
};

template <typename T>
void CLinkedList<T>::insert(Type &elem){
    // InternalInsert(m_pHead, elem);
}

// template <typename T>
// void CLinkedList<T>::InternalInsert(Node<Type> *&rParent, Type &elem){

// }

template <typename T>
CLinkedList<T>::CLinkedList()
{
}

template <typename T>
CLinkedList<T>::~CLinkedList()
{
}


void DemoLinkedList();

#endif // __LINKEDLIST_H__