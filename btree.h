#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>          // Para soporte de concurrencia
#include <fstream>        // Para operaciones de lectura/escritura
#include <memory>         // Para punteros inteligentes
#include <functional>     // Para foreach/firstthat generalizados
#include <stack>          // Para implementaciones de iteradores
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

// ============================================================================
// ESTRUCTURA DE TRAITS
// ============================================================================
template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
};

// ============================================================================
// DECLARACIONES AVANZADAS
// ============================================================================
template <typename Trait>
class BTree;

// Declaración avanzada del iterador
template <typename Trait>
class BTreeIterator;

// ============================================================================
// CLASE B-TREE
// ============================================================================
template <typename Trait>
class BTree // esta es la versión completa del BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType  ObjIDType;
       typedef CBTreePage<Trait>          BTNode;  // abreviatura útil

       // Declaraciones friend para iteradores y serialización
       friend class BTreeIterator<Trait>;
       friend std::ostream& operator<< <Trait>(std::ostream& os, const BTree<Trait>& tree);
       friend std::istream& operator>> <Trait>(std::istream& is, BTree<Trait>& tree);

public:
       // ====================================================================
       // TIPOS DE FUNCIONES GENERALIZADAS (Característica solicitada)
       // ====================================================================
       // Se reemplazan punteros a función con std::function para generalización
       using ForEachFunction = std::function<void(const keyType&, const ObjIDType&)>;
       using ForEachFunctionExt = std::function<void(const keyType&, const ObjIDType&, void*)>;
       using FirstThatPredicate = std::function<bool(const keyType&, const ObjIDType&)>;
       using FirstThatPredicateExt = std::function<bool(const keyType&, const ObjIDType&, void*)>;
       
       // Estructura de información de objeto
       typedef typename BTNode::ObjectInfo ObjectInfo;

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
             m_Height(other.m_Height)
       {
       }
       
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
       
       ~BTree() = default;

       // ====================================================================
       // OPERACIONES BÁSICAS
       // ====================================================================
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
       
       // ====================================================================
       // MÉTODOS FOREACH GENERALIZADOS (Característica solicitada)
       // ====================================================================
       void ForEach(const ForEachFunction& func) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              m_Root.ForEach([&func](const keyType& key, const ObjIDType& id) {
                     func(key, id);
              });
       }
       
       void ForEach(const ForEachFunctionExt& func, void* extraData) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              m_Root.ForEach([&func, extraData](const keyType& key, const ObjIDType& id) {
                     func(key, id, extraData);
              });
       }
       
       // ====================================================================
       // MÉTODOS FIRSTTHAT GENERALIZADOS (Característica solicitada)
       // ====================================================================
       ObjectInfo* FirstThat(const FirstThatPredicate& pred) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              return m_Root.FirstThat([&pred](const keyType& key, const ObjIDType& id) {
                     return pred(key, id);
              });
       }
       
       ObjectInfo* FirstThat(const FirstThatPredicateExt& pred, void* extraData) {
              std::lock_guard<std::mutex> lock(m_Mutex);  // Protección de concurrencia
              return m_Root.FirstThat([&pred, extraData](const keyType& key, const ObjIDType& id) {
                     return pred(key, id, extraData);
              });
       }
       
       // ====================================================================
       // MÉTODOS DE ITERADOR (Característica solicitada)
       // ====================================================================
       // Iterador hacia adelante begin/end
       iterator begin() {
              std::lock_guard<std::mutex> lock(m_Mutex);
              return iterator(this, false);  // Comenzar desde el elemento más pequeño
       }
       
       iterator end() {
              std::lock_guard<std::mutex> lock(m_Mutex);
              return iterator(this, true);   // Posición final
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
       // OPERACIONES DE LECTURA/ESCRITURA (Característica solicitada)
       // ====================================================================
       bool Write(const std::string& filename) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              std::ofstream ofs(filename, std::ios::binary);
              if (!ofs) return false;
              
              // Escribir información del encabezado
              ofs.write(reinterpret_cast<const char*>(&m_Order), sizeof(m_Order));
              ofs.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
              ofs.write(reinterpret_cast<const char*>(&m_NumKeys), sizeof(m_NumKeys));
              ofs.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
              
              // Escribir estructura del árbol
              return m_Root.Write(ofs);
       }
       
       bool Read(const std::string& filename) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              std::ifstream ifs(filename, std::ios::binary);
              if (!ifs) return false;
              
              // Leer información del encabezado
              ifs.read(reinterpret_cast<char*>(&m_Order), sizeof(m_Order));
              ifs.read(reinterpret_cast<char*>(&m_Unique), sizeof(m_Unique));
              ifs.read(reinterpret_cast<char*>(&m_NumKeys), sizeof(m_NumKeys));
              ifs.read(reinterpret_cast<char*>(&m_Height), sizeof(m_Height));
              
              // Reinicializar raíz con el orden correcto
              m_Root = BTNode(2 * m_Order + 1, m_Unique);
              m_Root.SetMaxKeysForChilds(m_Order);
              
              // Leer estructura del árbol
              return m_Root.Read(ifs);
       }
       
       // ====================================================================
       // MÉTODOS UTILITARIOS
       // ====================================================================
       size_t size() const  { 
              std::lock_guard<std::mutex> lock(m_Mutex);
              return m_NumKeys; 
       }
       
       size_t height() const { 
              std::lock_guard<std::mutex> lock(m_Mutex);
              return m_Height;      
       }
       
       size_t GetOrder() const { 
              std::lock_guard<std::mutex> lock(m_Mutex);
              return m_Order;     
       }
       
       void Print(std::ostream& os) {
              std::lock_guard<std::mutex> lock(m_Mutex);
              m_Root.Print(os);                              
       }

private:
       BTNode          m_Root;
       size_t          m_Height;   // altura del árbol
       size_t          m_Order;    // orden del árbol
       size_t          m_NumKeys;  // número de claves
       bool            m_Unique;   // ¿Acepta los elementos solo una vez?
       
       // ====================================================================
       // SOPORTE DE CONCURRENCIA (Característica solicitada)
       // ====================================================================
       mutable std::mutex m_Mutex;  // Mutex para operaciones thread-safe
};     

// ============================================================================
// CLASE DE ITERADOR HACIA ADELANTE (Característica solicitada)
// ============================================================================
template <typename Trait>
class BTreeIterator {
private:
       using BTreeType = BTree<Trait>;
       using BTNode = CBTreePage<Trait>;
       using keyType = typename Trait::keyType;
       using ObjIDType = typename Trait::ObjIDType;
       
       struct StackItem {
              BTNode* node;
              size_t index;
              StackItem(BTNode* n, size_t i) : node(n), index(i) {}
       };

public:
       // Características del iterador
       using iterator_category = std::forward_iterator_tag;
       using value_type = std::pair<keyType, ObjIDType>;
       using difference_type = std::ptrdiff_t;
       using pointer = value_type*;
       using reference = value_type&;
       
       BTreeIterator(BTreeType* tree = nullptr, bool isEnd = false) 
           : m_Tree(tree) {
              if (tree && !isEnd) {
                     InicializarHaciaAdelante();
              }
       }
       
       // Constructor de copia
       BTreeIterator(const BTreeIterator& other)
           : m_Tree(other.m_Tree),
             m_Stack(other.m_Stack),
             m_CurrentKey(other.m_CurrentKey),
             m_CurrentValue(other.m_CurrentValue) {
       }
       
       // Constructor de movimiento
       BTreeIterator(BTreeIterator&& other) noexcept
           : m_Tree(other.m_Tree),
             m_Stack(std::move(other.m_Stack)),
             m_CurrentKey(std::move(other.m_CurrentKey)),
             m_CurrentValue(std::move(other.m_CurrentValue)) {
              other.m_Tree = nullptr;
       }
       
       // Inicializar para recorrido hacia adelante (más pequeño a más grande)
       void InicializarHaciaAdelante() {
              m_Stack.clear();
              if (m_Tree && m_Tree->m_NumKeys > 0) {
                     // Comenzar desde la raíz y ir al nodo más a la izquierda
                     BTNode* current = &(m_Tree->m_Root);
                     while (current) {
                            if (current->GetKeyCount() > 0) {
                                   m_Stack.emplace_back(current, 0);
                                   current = current->GetChild(0);
                            } else {
                                   break;
                            }
                     }
                     AvanzarHaciaAdelante();
              }
       }
       
       // Inicializar para recorrido hacia atrás (más grande a más pequeño)
       void InicializarHaciaAtras() {
              m_Stack.clear();
              if (m_Tree && m_Tree->m_NumKeys > 0) {
                     // Comenzar desde la raíz y ir al nodo más a la derecha
                     BTNode* current = &(m_Tree->m_Root);
                     while (current) {
                            size_t keyCount = current->GetKeyCount();
                            if (keyCount > 0) {
                                   m_Stack.emplace_back(current, keyCount);
                                   current = current->GetChild(keyCount);
                            } else {
                                   break;
                            }
                     }
                     AvanzarHaciaAtras();
              }
       }
       
       // Avanzar iterador en dirección hacia adelante
       void AvanzarHaciaAdelante() {
              if (m_Stack.empty()) {
                     m_CurrentKey = keyType();
                     m_CurrentValue = ObjIDType();
                     return;
              }
              
              auto& top = m_Stack.back();
              BTNode* node = top.node;
              size_t& index = top.index;
              
              // Obtener par clave-valor actual
              m_CurrentKey = node->GetKey(index);
              m_CurrentValue = node->GetValue(index);
              
              // Mover a la siguiente posición
              index++;
              
              // Si hemos procesado todas las claves en este nodo
              if (index > node->GetKeyCount()) {
                     m_Stack.pop_back();
                     
                     // Ir al hijo derecho si existe
                     if (!m_Stack.empty()) {
                            auto& newTop = m_Stack.back();
                            BTNode* parent = newTop.node;
                            size_t parentIndex = newTop.index - 1;
                            
                            if (parentIndex < parent->GetChildCount()) {
                                   BTNode* rightChild = parent->GetChild(parentIndex + 1);
                                   if (rightChild) {
                                          // Empujar hijo derecho y su camino más a la izquierda
                                          BTNode* current = rightChild;
                                          while (current) {
                                                 if (current->GetKeyCount() > 0) {
                                                        m_Stack.emplace_back(current, 0);
                                                        current = current->GetChild(0);
                                                 } else {
                                                        break;
                                                 }
                                          }
                                   }
                            }
                     }
              }
       }
       
       // Avanzar iterador en dirección hacia atrás
       void AvanzarHaciaAtras() {
              if (m_Stack.empty()) {
                     m_CurrentKey = keyType();
                     m_CurrentValue = ObjIDType();
                     return;
              }
              
              auto& top = m_Stack.back();
              BTNode* node = top.node;
              size_t& index = top.index;
              
              // Ajustar índice si apunta más allá del final
              if (index > node->GetKeyCount()) {
                     index = node->GetKeyCount();
              }
              
              // Obtener par clave-valor actual
              m_CurrentKey = node->GetKey(index - 1);
              m_CurrentValue = node->GetValue(index - 1);
              
              // Mover a la posición anterior
              index--;
              
              // Si hemos procesado todas las claves en este nodo
              if (index == 0) {
                     m_Stack.pop_back();
                     
                     // Ir al hijo izquierdo si existe
                     if (!m_Stack.empty()) {
                            auto& newTop = m_Stack.back();
                            BTNode* parent = newTop.node;
                            size_t parentIndex = newTop.index;
                            
                            if (parentIndex > 0 && parentIndex <= parent->GetChildCount()) {
                                   BTNode* leftChild = parent->GetChild(parentIndex - 1);
                                   if (leftChild) {
                                          // Empujar hijo izquierdo y su camino más a la derecha
                                          BTNode* current = leftChild;
                                          while (current) {
                                                 size_t keyCount = current->GetKeyCount();
                                                 if (keyCount > 0) {
                                                        m_Stack.emplace_back(current, keyCount);
                                                        current = current->GetChild(keyCount);
                                                 } else {
                                                        break;
                                                 }
                                          }
                                   }
                            }
                     }
              }
       }
       
       // Operaciones del iterador
       value_type operator*() const {
              return std::make_pair(m_CurrentKey, m_CurrentValue);
       }
       
       BTreeIterator& operator++() {
              AvanzarHaciaAdelante();
              return *this;
       }
       
       BTreeIterator operator++(int) {
              BTreeIterator temp = *this;
              ++(*this);
              return temp;
       }
       
       bool operator==(const BTreeIterator& other) const {
              return m_Tree == other.m_Tree && 
                     m_Stack.size() == other.m_Stack.size() &&
                     (m_Stack.empty() || m_Stack.back().node == other.m_Stack.back().node);
       }
       
       bool operator!=(const BTreeIterator& other) const {
              return !(*this == other);
       }

private:
       BTreeType* m_Tree;
       std::vector<StackItem> m_Stack;
       keyType m_CurrentKey;
       ObjIDType m_CurrentValue;
};

// ============================================================================
// IMPLEMENTACIÓN DE OPERACIÓN INSERT
// ============================================================================
template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
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

// ============================================================================
// IMPLEMENTACIÓN DE OPERACIÓN REMOVE
// ============================================================================
template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const long ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

// ============================================================================
// IMPLEMENTACIÓN DE OPERATOR<< (Característica solicitada)
// ============================================================================
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree) {
       // Crear copia no-const para impresión (para adquirir lock)
       BTree<Trait>& nonConstTree = const_cast<BTree<Trait>&>(tree);
       std::lock_guard<std::mutex> lock(nonConstTree.m_Mutex);
       
       os << "Estadísticas del BTree:" << std::endl;
       os << "  Orden: " << tree.m_Order << std::endl;
       os << "  Altura: " << tree.m_Height << std::endl;
       os << "  Número de claves: " << tree.m_NumKeys << std::endl;
       os << "  Claves únicas solamente: " << (tree.m_Unique ? "Sí" : "No") << std::endl;
       os << "  Estructura del árbol:" << std::endl;
       
       tree.m_Root.Print(os);
       return os;
}

// ============================================================================
// IMPLEMENTACIÓN DE OPERATOR>> (Característica solicitada)
// ============================================================================
template <typename Trait>
std::istream& operator>>(std::istream& is, BTree<Trait>& tree) {
       std::lock_guard<std::mutex> lock(tree.m_Mutex);
       
       // Limpiar árbol existente
       tree = BTree<Trait>();
       
       // Leer encabezado
       is >> tree.m_Order;
       is >> tree.m_Unique;
       is >> tree.m_NumKeys;
       is >> tree.m_Height;
       
       // Reinicializar raíz
       tree.m_Root = CBTreePage<Trait>(2 * tree.m_Order + 1, tree.m_Unique);
       tree.m_Root.SetMaxKeysForChilds(tree.m_Order);
       
       // Leer datos del árbol (asumiendo que BTNode tiene operator>>)
       tree.m_Root.Read(is);
       
       return is;
}

#endif
