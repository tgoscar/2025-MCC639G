#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <iostream>
#include "types.h"
#include <mutex>


template <typename T, typename _Func>
struct LLinkedListTraits{
    using value_type = T;
    using Func       = _Func;
};

template <typename T>
struct LLinkedListAsc : 
    public LLinkedListTraits<T, std::less<T> >{
};

template <typename T>
struct LLinkedListDesc : 
    public LLinkedListTraits<T, std::greater<T> >{
};

template <typename Traits>
class LLNode{
private:
    using    value_type = typename Traits::value_type;
    using    Node       = LLNode<Traits>;
    using    MySelf     = LLNode<Traits>;
    value_type          m_data;
    Ref      m_ref;
    Node    *m_pNext = nullptr;

public:
    LLNode(value_type &elem, Ref ref, LLNode<Traits> *pNext = nullptr)
        : m_data(elem), m_ref(ref), m_pNext(pNext){
    }
    value_type   GetData()    { return m_data;     }
    value_type  &GetDataRef() { return m_data;     }
    Ref    GetRef()     { return m_ref;      }
    Node * GetNext()    { return m_pNext;    }
    Node *&GetNextRef() { return m_pNext;    }
};

// 
// TODO Activar el iterator
template <typename Container> // HERE: TTraits -> Container
class forward_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
     using iterator   = forward_linkedlist_iterator<Container>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     forward_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     forward_linkedlist_iterator(iterator &other)
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
template <typename Traits>
class CLinkedList{
public:
    using value_type = typename Traits::value_type; 
    using Func       = typename Traits::Func;
    using Node       = LLNode<Traits>; 
    using Container  = CLinkedList<Traits>;
    using iterator   = forward_linkedlist_iterator<Container>;

private:
    Node   *m_pRoot = nullptr;
    size_t m_nElem = 0;
    Func   m_fCompare;
    std::mutex m_mutex;
public:
    // Constructor
    CLinkedList();
    CLinkedList(CLinkedList &other);

    // TODO: Move contructor (leer bibliografia)
    CLinkedList(CLinkedList &&other);

    // Destructor seguro
    virtual ~CLinkedList();

    void Insert(value_type &elem, Ref ref);

    // <-- agregado: lectura desde istream
    void Read(std::istream &is);

private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref);
    Node *GetRoot()    {    return m_pRoot;     };

public:
    iterator begin(){ return iterator(this, m_pRoot); };
    iterator end()  { return iterator(this, nullptr); } 

    friend std::ostream& operator<<(std::ostream &os, CLinkedList<Traits> &obj){
        auto pRoot = obj.GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
            pRoot = pRoot->GetNext();
        }
        return os;
    }
};

template <typename Traits>
void CLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    // concurrencia: proteger operación de inserción pública
    std::lock_guard<std::mutex> lg(m_mutex);
    InternalInsert(m_pRoot, elem, ref);
}

template <typename Traits>
void CLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    if( !rParent || m_fCompare(elem, rParent->GetDataRef()) ){
        rParent = new Node(elem, ref, rParent);
        return;
    }
    // Tail recursion
    InternalInsert(rParent->GetNextRef(), elem, ref);
}

template <typename Traits>
CLinkedList<Traits>::CLinkedList(CLinkedList &other){
    // Nota: mantenemos el estilo original y reutilizamos Insert para respetar el orden
    // y la lógica de comparación definida por Traits::Func (m_fCompare).
    auto p = other.m_pRoot;
    while (p){
        auto val = p->GetDataRef(); // se crea una copia del valor
        Insert(val, p->GetRef());   // Insert está protegido con mutex y usa InternalInsert
        p = p->GetNext();
    }
}

// Move Constructor
template <typename Traits>
CLinkedList<Traits>::CLinkedList(CLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
}

template <typename Traits>
CLinkedList<Traits>::~CLinkedList()
CLinkedList<Traits>::~CLinkedList()
{
    // Destructor seguro: liberar todos los nodos
    // No es necesario bloquear el mutex aquí (destrucción del propio objeto).
    auto p = m_pRoot;
    while (p){
        auto q = p->GetNext();
        delete p;
        p = q;
    }
    m_pRoot = nullptr;
    m_nElem = 0;
}

// <-- agregado: implementación Read(istream&)
// Lee pares (value_type, Ref) hasta EOF/entrada inválida e inserta cada uno
template <typename Traits>
void CLinkedList<Traits>::Read(std::istream &is){
    std::lock_guard<std::mutex> lg(m_mutex); // proteger construcción concurrente
    value_type v;
    Ref r;
    while (is >> v >> r){
        // usar inserción interna porque ya tenemos el lock
        InternalInsert(m_pRoot, v, r);
    }
}

// TODO: Este operador debe quedar fuera de a clase
template <typename Traits>
std::ostream &operator<<(std::ostream &os, CLinkedList<Traits> &obj){
    auto pRoot = obj.GetRoot();
    while( pRoot )
        os << pRoot->GetData() << " ";
    return os;
}

void DemoLinkedList();

#endif // __LINKEDLIST_H__
