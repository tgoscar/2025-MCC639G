#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <iostream>
#include <mutex>      // <-- agregado para concurrencia
#include "types.h"
#include "traits.h"

template <typename Traits>
class DLLNode{
private:
    using    value_type = typename Traits::value_type;
    using    Node       = DLLNode<Traits>;
    using    MySelf     = DLLNode<Traits>;
    value_type          m_data;
    Ref      m_ref;
    Node    *m_pNext = nullptr;
    Node    *m_pPrev = nullptr;

public:
    DLLNode(value_type &elem, Ref ref, DLLNode<Traits> *pNext = nullptr)
        : m_data(elem), m_ref(ref), m_pNext(pNext){
    }
    value_type   GetData()    { return m_data;     }
    value_type  &GetDataRef() { return m_data;     }
    Ref    GetRef()     { return m_ref;      }
    Node * GetNext()    { return m_pNext;    }
    Node *&GetNextRef() { return m_pNext;    }

    // Particular para la double LinkedList
    Node * GetPrev()    { return m_pPrev;    }
    Node *&GetPrevRef() { return m_pPrev;    }
};

// 
// TODO Activar el forward_iterator
template <typename Container>
class forward_double_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
     using iterator   = forward_double_linkedlist_iterator<Container>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     forward_double_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     forward_double_linkedlist_iterator(iterator &other)
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

template <typename Container> // HERE: TTraits -> Container
class backward_double_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
     using iterator   = backward_double_linkedlist_iterator<Container>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     backward_double_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     backward_double_linkedlist_iterator(iterator &other)
             : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
     bool operator==(iterator other){ return m_pList == other.m_pList && m_pNode == other.m_pNode; }
     bool operator!=(iterator other){ return !(*this == other);    }

     iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetPrev();
         return *this;
     }
     value_type &operator*(){    return m_pNode->GetDataRef();   }
};

// TODO Agregar control de concurrencia

// TODO Agregar que sea ascendente o descendente con el mismo codigo
template <typename Traits>
class CDoubleLinkedList{
public:
    using value_type = typename Traits::value_type; 
    using Func       = typename Traits::Func;
    using Node       = DLLNode<Traits>; 
    using Container  = CDoubleLinkedList<Traits>;
    using forward_iterator   = forward_double_linkedlist_iterator<Container>;
    using backward_iterator  = backward_double_linkedlist_iterator<Container>;
    
private:
    Node   *m_pRoot = nullptr;
    size_t m_nElem = 0;
    Func   m_fCompare;
    std::mutex m_mutex; // <-- agregado: control de concurrencia

public:
    // Constructor
    CDoubleLinkedList();
    CDoubleLinkedList(CDoubleLinkedList &other);

    // TODO: Done
    CDoubleLinkedList(CDoubleLinkedList &&other);

    // Destructor seguro
    virtual ~CDoubleLinkedList();

    void Insert(value_type &elem, Ref ref);
private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref);
    Node *GetRoot()    {    return m_pRoot;     };

public:
    forward_iterator begin(){ return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { return forward_iterator(this, nullptr); } 

    // TODO: verifricar donde debe comenzar apuntando el iterator reverso
    backward_iterator rbegin(){                        // <-- tipo corregido
        // iniciar en el último nodo
        Node *p = m_pRoot;
        if (!p) return backward_iterator(this, nullptr);
        while (p->GetNext()) p = p->GetNext();
        return backward_iterator(this, p);
    };
    backward_iterator rend()  { return backward_iterator(this, nullptr); } // <-- tipo corregido

    friend std::ostream& operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
        auto pRoot = obj.GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
            pRoot = pRoot->GetNext();
        }
        return os;
    }
public:
    // Persistence
    std::ostream &Write(std::ostream &os) { return os << *this; }
    
    // TODO: Read (istream &is)
    std::istream &Read (std::istream &is);
};

template <typename Traits>
void CDoubleLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    std::lock_guard<std::mutex> lg(m_mutex); // <-- proteger inserción
    InternalInsert(m_pRoot, elem, ref);
}

template <typename Traits>
void CDoubleLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    if( !rParent || m_fCompare(elem, rParent->GetDataRef()) ){
        // Insertamos antes de rParent: ajustar enlaces next/prev mínimamente
        Node* old = rParent;
        rParent = new Node(elem, ref, old);
        if (old) old->GetPrevRef() = rParent;   // <-- mantener doble enlace correcto
        m_nElem++;
        return;
    }
    // Tail recursion
    InternalInsert(rParent->GetNextRef(), elem, ref);
}

template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(){}

// TODO Constructor por copia
//      Hacer loop copiando cada elemento
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &other){
    // Copia profunda: reusar Insert para preservar orden/consistencia
    auto p = other.m_pRoot;
    while (p){
        auto v = p->GetDataRef();
        Insert(v, p->GetRef());   // Insert (thread-safe)
        p = p->GetNext();
    }
}

// Move Constructor
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
}

// TODO: Implementar y liberar la memoria de cada Node
template <typename Traits>
CDoubleLinkedList<Traits>::~CDoubleLinkedList()
{
    // Liberar todos los nodos (no se requiere bloquear al destruir)
    auto p = m_pRoot;
    while (p){
        auto q = p->GetNext();
        delete p;
        p = q;
    }
    m_pRoot = nullptr;
    m_nElem = 0;
}

// Read(istream&): lee pares (value, Ref) e inserta bajo lock
template <typename Traits>
std::istream &CDoubleLinkedList<Traits>::Read (std::istream &is){
    std::lock_guard<std::mutex> lg(m_mutex);
    value_type v;
    Ref r;
    while (is >> v >> r){
        InternalInsert(m_pRoot, v, r); // ya bajo lock
        m_nElem++;
    }
    return is;
}

// TODO: Este operador debe quedar fuera de la clase
template <typename Traits>
std::ostream &operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
    auto pRoot = obj.GetRoot();
    while( pRoot )
        os << pRoot->GetData() << " ";
    return os;
}

void DemoDoubleLinkedList();

#endif // __LINKEDLIST_H__
