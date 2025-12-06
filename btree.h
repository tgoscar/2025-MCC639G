#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
#include <mutex>          // Para soporte de concurrencia
#include <fstream>        // Para operaciones de lectura/escritura
#include <memory>         // Para punteros inteligentes
#include <functional>     // Para foreach/firstthat generalizados
#include <stack>          // Para implementaciones de iteradores
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using LinkedValueType = _ObjIDType;
};

template <typename Trait>
class BTree;

// Declaración avanzada del iterador
template <typename Trait>
class BTreeIterator;

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand
       // Declaraciones friend para iteradores y serialización
       friend class BTreeIterator<Trait>;
       friend std::ostream& operator<< <Trait>(std::ostream& os, const BTree<Trait>& tree);
       friend std::istream& operator>> <Trait>(std::istream& is, BTree<Trait>& tree);

public:
       // ====================================================================
       // TIPOS DE FUNCIONES GENERALIZADAS (Característica solicitada)
       // ====================================================================
       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       
       // Nuevas funciones generalizadas con std::function
       using ForEachFunction = std::function<void(const keyType&, const ObjIDType&)>;
       using ForEachFunctionExt = std::function<void(const keyType&, const ObjIDType&, void*)>;
       using FirstThatPredicate = std::function<bool(const keyType&, const ObjIDType&)>;
       using FirstThatPredicateExt = std::function<bool(const keyType&, const ObjIDType&, void*)>;

       typedef typename BTNode::ObjectInfo      ObjectInfo;

       // ====================================================================
       // TIPOS DE ITERADORES
       // ====================================================================
       using iterator = BTreeIterator<Trait>;
       using const_iterator = BTreeIterator<Trait>;
       using reverse_iterator = std::reverse_iterator<iterator>;
       using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
       // ====================================================================
       // CONSTRUCTORES & DESTRUCTORES
       // ====================================================================
       // Constructor por defecto
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       // Constructor de copia
       BTree(const BTree& other)
           : m_Order(other.m_Order),
             m_Root(other.m_Root),
             m_Unique(other.m_Unique),
             m_NumKeys(other.m_NumKeys),
             m_Height(other.m_Height) {}

        // ====================================================================
       // CONSTRUCTOR DE MOVIMIENTO (Característica solicitada)
       // ====================================================================
       BTree(BTree&& other) noexcept
           : m_Order(std::move(other.m_Order)),
             m_Root(std::move(other.m_Root)),
             m_Unique(std::move(other.m_Unique)),
             m_NumKeys(std::move(other.m_NumKeys)),
             m_Height(std::move(other.m_Height)),
             m_Mutex()  // Mutex no es movable, inicializar uno nuevo
       {
              // Reiniciar el objeto fuente
              other.m_Order = DEFAULT_BTREE_ORDER;
              other.m_NumKeys = 0;
              other.m_Height = 1;
              other.m_Unique = true;
       }
       
       // Operador de asignación por movimiento
       BTree& operator=(BTree&& other) noexcept {
              if (this != &other) {
                     std::lock_guard<std::mutex> lock1(m_Mutex, std::adopt_lock);
                     std::lock_guard<std::mutex> lock2(other.m_Mutex, std::adopt_lock);
                     
                     m_Order = std::move(other.m_Order);
                     m_Root = std::move(other.m_Root);
                     m_Unique = std::move(other.m_Unique);
                     m_NumKeys = std::move(other.m_NumKeys);
                     m_Height = std::move(other.m_Height);
                     
                     other.m_Order = DEFAULT_BTREE_ORDER;
                     other.m_NumKeys = 0;
                     other.m_Height = 1;
                     other.m_Unique = true;
              }
              return *this;
       }

       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();

      

       // ====================================================================
       // OPERACIONES BÁSICAS
       // ====================================================================

       // Implementación común para Insert
       bool Insert(const keyType& key, const long ObjID) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              bt_ErrorCode error = m_Root.Insert(key, ObjID);
              if( error == bt_duplicate )
                     return false;
              m_NumKeys++;
              if( error == bt_overflow )
              {
                     m_Root.SplitRoot();
                     m_Height++;
              }
              return true;
       }

       // Implementación común para Remove
       bool Remove(const keyType& key, const long ObjID) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              bt_ErrorCode error = m_Root.Remove(key, ObjID);
              if( error == bt_duplicate || error == bt_nofound )
                     return false;
              m_NumKeys--;
              if( error == bt_rootmerged )
                     m_Height--;
              return true;
       }
       
       ObjIDType Search(const keyType& key) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }

       {      ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t            size()  { 
                                   std::lock_guard<std::mutex> lock(m_Mutex);
                                  return m_NumKeys; }
       size_t            height() { 
                                   std::lock_guard<std::mutex> lock(m_Mutex);
                                   return m_Height;      }
       size_t            GetOrder() { 
                                   std::lock_guard<std::mutex> lock(m_Mutex);
                                     return m_Order;     }

       void            Print (ostream &os) {               
                            m_Root.Print(os);
                            std::lock_guard<std::mutex> lock(m_Mutex);}
       
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              m_Root.ForEach(lpfn, 0, pExtra1);              
       }
       
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              return m_Root.FirstThat([&pred](const keyType& key, const ObjIDType& id) {
                     return pred(key, id);
              });
       }
       
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              return m_Root.FirstThat(lpfn, 0, pExtra1);     
       }
       
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   
       }
       
       //typedef               ObjectInfo iterator;

       // ====================================================================
       // MÉTODOS GENERALIZADOS FOREACH (Característica solicitada)
       // ====================================================================
       void ForEach(const ForEachFunction& func) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              m_Root.ForEach([&func](const keyType& key, const ObjIDType& id) {
                     func(key, id);
              });
       }
       
       void ForEach(const ForEachFunctionExt& func, void* extraData) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              m_Root.ForEach([&func, extraData](const keyType& key, const ObjIDType& id) {
                     func(key, id, extraData);
              });
       }
       
       // ====================================================================
       // MÉTODOS GENERALIZADOS FIRSTTHAT (Característica solicitada)
       // ====================================================================
       ObjectInfo* FirstThat(const FirstThatPredicate& pred) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              return m_Root.FirstThat([&pred](const keyType& key, const ObjIDType& id) {
                     return pred(key, id);
              });
       }
       
       ObjectInfo* FirstThat(const FirstThatPredicateExt& pred, void* extraData) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              return m_Root.FirstThat([&pred, extraData](const keyType& key, const ObjIDType& id) {
                     return pred(key, id, extraData);
              });
       }

       // ====================================================================
       // MÉTODOS DE ITERADOR (Característica solicitada)
       // ====================================================================
       iterator begin() {
              std::lock_guard<std::mutex> lock(m_Mutex);
              return iterator(this, false);
       }
       
       iterator end() {
              std::lock_guard<std::mutex> lock(m_Mutex);
              return iterator(this, true);
       }
       
       const_iterator begin() const {
              return const_iterator(const_cast<BTree*>(this), false);
       }
       
       const_iterator end() const {
              return const_iterator(const_cast<BTree*>(this), true);
       }
       
       // ====================================================================
       // MÉTODOS DE ITERADOR HACIA ATRÁS (Característica solicitada)
       // ====================================================================
       reverse_iterator rbegin() {
              return reverse_iterator(end());
       }
       
       reverse_iterator rend() {
              return reverse_iterator(begin());
       }
       
       const_reverse_iterator rbegin() const {
              return const_reverse_iterator(end());
       }
       
       const_reverse_iterator rend() const {
              return const_reverse_iterator(begin());
       }
       // ====================================================================
       // READ/WRITE OPERATIONS (Característica solicitada)
       // ====================================================================
       bool Write(const std::string& filename) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              std::ofstream ofs(filename, std::ios::binary);
              if (!ofs) return false;
              
              ofs.write(reinterpret_cast<const char*>(&m_Order), sizeof(m_Order));
              ofs.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
              ofs.write(reinterpret_cast<const char*>(&m_NumKeys), sizeof(m_NumKeys));
              ofs.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
              
              return m_Root.Write(ofs);
       }
       
       bool Read(const std::string& filename) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              std::ifstream ifs(filename, std::ios::binary);
              if (!ifs) return false;
              
              ifs.read(reinterpret_cast<char*>(&m_Order), sizeof(m_Order));
              ifs.read(reinterpret_cast<char*>(&m_Unique), sizeof(m_Unique));
              ifs.read(reinterpret_cast<char*>(&m_NumKeys), sizeof(m_NumKeys));
              ifs.read(reinterpret_cast<char*>(&m_Height), sizeof(m_Height));
              
              m_Root = BTNode(2 * m_Order + 1, m_Unique);
              m_Root.SetMaxKeysForChilds(m_Order);
              
              return m_Root.Read(ifs);
       }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?

       // ====================================================================
       // CONCURRENCY SUPPORT (Característica solicitada)
       // ====================================================================
       mutable std::mutex m_Mutex;

};     

// ============================================================================
// CLASE DE ITERADOR HACIA ADELANTE
// ============================================================================
template <typename Trait>
class BTreeIterator {
private:
       using BTreeType = BTree<Trait>;
       using BTNode = CBTreePage<Trait>;
       using keyType = typename Trait::keyType;
       using ObjIDType = typename Trait::ObjIDType;
       
       struct StackItem { BTNode* node; size_t index; 
                        StackItem(BTNode* n, size_t i) : node(n), index(i) {} };

public:
       using iterator_category = std::forward_iterator_tag;
       using value_type = std::pair<keyType, ObjIDType>;
       using difference_type = std::ptrdiff_t;
       using pointer = value_type*;
       using reference = value_type&;
       
       BTreeIterator(BTreeType* tree = nullptr, bool isEnd = false) : m_Tree(tree) {
              if (tree && !isEnd) InicializarHaciaAdelante();
       }
       
       BTreeIterator(const BTreeIterator& other) = default;
       
       BTreeIterator(BTreeIterator&& other) noexcept
           : m_Tree(other.m_Tree), m_Stack(std::move(other.m_Stack)),
             m_CurrentKey(std::move(other.m_CurrentKey)),
             m_CurrentValue(std::move(other.m_CurrentValue)) {
              other.m_Tree = nullptr;
       }
       
       void InicializarHaciaAdelante() {
              m_Stack.clear();
              if (m_Tree && m_Tree->m_NumKeys > 0) {
                     BTNode* current = &(m_Tree->m_Root);
                     while (current && current->GetKeyCount() > 0) {
                            m_Stack.emplace_back(current, 0);
                            current = current->GetChild(0);
                     }
                     AvanzarHaciaAdelante();
              }
       }
       
       void AvanzarHaciaAdelante() {
              if (m_Stack.empty()) {
                     m_CurrentKey = keyType(); m_CurrentValue = ObjIDType(); return;
              }
              auto& top = m_Stack.back();
              m_CurrentKey = top.node->GetKey(top.index);
              m_CurrentValue = top.node->GetValue(top.index);
              top.index++;
              if (top.index > top.node->GetKeyCount()) {
                     m_Stack.pop_back();
                     if (!m_Stack.empty()) {
                            auto& parent = m_Stack.back();
                            size_t childIdx = parent.index - 1;
                            if (childIdx < parent.node->GetChildCount()) {
                                   BTNode* rightChild = parent.node->GetChild(childIdx + 1);
                                   BTNode* current = rightChild;
                                   while (current && current->GetKeyCount() > 0) {
                                          m_Stack.emplace_back(current, 0);
                                          current = current->GetChild(0);
                                   }
                            }
                     }
              }
       }
       
       value_type operator*() const { return {m_CurrentKey, m_CurrentValue}; }
       BTreeIterator& operator++() { AvanzarHaciaAdelante(); return *this; }
       BTreeIterator operator++(int) { BTreeIterator temp = *this; ++(*this); return temp; }
       bool operator==(const BTreeIterator& other) const {
              return m_Tree == other.m_Tree && m_Stack.size() == other.m_Stack.size() &&
                     (m_Stack.empty() || m_Stack.back().node == other.m_Stack.back().node);
       }
       bool operator!=(const BTreeIterator& other) const { return !(*this == other); }

private:
       BTreeType* m_Tree;
       std::vector<StackItem> m_Stack;
       keyType m_CurrentKey;
       ObjIDType m_CurrentValue;
};

// TODO Add operator<<
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree) {
       BTree<Trait>& nonConstTree = const_cast<BTree<Trait>&>(tree);
       std::lock_guard<std::mutex> lock(nonConstTree.m_Mutex);
       
       os << "BTree Statistics:" << std::endl;
       os << "  Order: " << tree.m_Order << std::endl;
       os << "  Height: " << tree.m_Height << std::endl;
       os << "  Number of keys: " << tree.m_NumKeys << std::endl;
       os << "  Unique keys only: " << (tree.m_Unique ? "Yes" : "No") << std::endl;
       os << "  Tree structure:" << std::endl;
       
       tree.m_Root.Print(os);
       return os;
}

// TODO Add operator>>

 // Avanzar iterador en dirección hacia adelante
 template <typename Trait>
std::istream& operator>>(std::istream& is, BTree<Trait>& tree) {
       std::lock_guard<std::mutex> lock(tree.m_Mutex);
       
       tree = BTree<Trait>();
       
       is >> tree.m_Order;
       is >> tree.m_Unique;
       is >> tree.m_NumKeys;
       is >> tree.m_Height;
       
       tree.m_Root = CBTreePage<Trait>(2 * tree.m_Order + 1, tree.m_Unique);
       tree.m_Root.SetMaxKeysForChilds(tree.m_Order);
       
       tree.m_Root.Read(is);
       
       return is;

#endif

