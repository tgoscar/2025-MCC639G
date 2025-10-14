#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <cassert>
#include <vector>
#include <sstream>
#include "types.h"
#include <string>
#include <iomanip>
#include <algorithm>
#include <type_traits>
#include <utility>

using namespace std;


template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::value_type;
  using Node = CBinaryTreeNode<Traits>;

public:
  void PrintVertical() const;

private:
  int height(Node* n) const;
void buildVerticalCanvas(std::vector<std::string>& canvas,
                         Node* n, int row, int col, int offset,
                         int nodeWidth) const;

private:
    value_type       m_data;
    Node *  m_pParent = nullptr;
    Ref     m_ref;
    vector<Node *> m_pChild = {nullptr, nullptr};
public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_pParent(pParent), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }

    value_type         getData()                {   return m_data;    }
    value_type        &getDataRef()             {   return m_data;    }
    const value_type&  getDataRef() const       {   return m_data;    } // Added const version
 
    void      setpChild(Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch) const { return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
    const Ref& getRef() const { return m_ref; } // Added getter for ref
};

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

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    Func Compfn;
    
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }
    
    virtual void insert(value_type &elem, Ref ref) {
        internal_insert(elem, ref, nullptr, nullptr, m_pRoot);
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
    
    CBinaryTree(Container &other); // TODO: Selis Luis (Copy Constructor)
    
    CBinaryTree(Container &&other)
        : m_pRoot(std::move(other.m_pRoot)), 
          m_size (std::move(other.m_size)), 
          Compfn (std::move(other.Compfn))
    { }

    virtual ~CBinaryTree(){  
        clear(); // Added clear in destructor
    } 
    
    // Método auxiliar para limpiar el árbol
    void clear() {
        postorder([](Node* node, size_t) {
            delete node;
        });
        m_pRoot = nullptr;
        m_size = 0;
    }
    
    void inorder  (ostream &os)    {   inorder  (m_pRoot, 0, os);  }
    
    void inorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            inorder(pNode->getChild(0), level+1, os);
            os << " --> " << pNode->getDataRef();
            inorder(pNode->getChild(1), level+1, os);
        }
    }

    void inorder(Node  *pNode, void (*visit) (value_type& item)){
        if( pNode ){   
            inorder(pNode->getChild(0), *visit);
            (*visit)(pNode->getDataRef());
            inorder(pNode->getChild(1), *visit);
        }
    }

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
    
    void preorder (ostream &os)    {   preorder (m_pRoot, 0, os);  }
    
    void preorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            os << " --> " << pNode->getDataRef();
            preorder(pNode->getChild(0), level+1, os);
            preorder(pNode->getChild(1), level+1, os);            
        }
    }

    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
    
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            os << string(level * 3, ' ') << " | " << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            print(pNode->getChild(0), level+1, os);
        }
    }

    void postorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            postorder(pNode->getChild(0), level+1, os);
            postorder(pNode->getChild(1), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }

    // TODO: Arriola Aldo
    void Write(ostream &os) { 
        os << *this;  
    }

    // TODO: Toledo Oscar
    void Read(istream &is)  { 
        clear(); // Limpiar el árbol actual antes de leer
        
        size_t elementCount;
        is >> elementCount;
        
        if (elementCount > 0) {
            // Usar preorder para reconstruir la estructura exacta del árbol
            std::vector<std::pair<value_type, Ref>> elements;
            elements.reserve(elementCount);
            
            for (size_t i = 0; i < elementCount; ++i) {
                value_type data;
                Ref ref;
                
                // Leer datos y referencia
                is >> data;
                is.read(reinterpret_cast<char*>(&ref), sizeof(Ref));
                
                elements.emplace_back(data, ref);
            }
            
            // Reconstruir el árbol en preorder
            size_t index = 0;
            m_pRoot = internalReadPreorder(is, nullptr, elements, index);
            m_size = elementCount;
        }
    }

private:
    Node* internalReadPreorder(istream& is, Node* parent, 
                              const std::vector<std::pair<value_type, Ref>>& elements, 
                              size_t& index) {
        if (index >= elements.size()) return nullptr;
        
        // Crear nodo actual (preorder: raíz primero)
        auto [data, ref] = elements[index++];
        Node* newNode = CreateNode(parent, data, ref);
        
        // Leer hijos (preorder continúa con subárbol izquierdo y luego derecho)
        newNode->getChildRef(0) = internalReadPreorder(is, newNode, elements, index);
        newNode->getChildRef(1) = internalReadPreorder(is, newNode, elements, index);
        
        return newNode;
    }

public:
    void writePreorder(ostream &os) const {
        // Escribir cantidad de elementos primero
        os << m_size << "\n";
        
        if (m_pRoot) {
            internalWritePreorder(m_pRoot, os);
        }
    }

private:
    void internalWritePreorder(Node* node, ostream &os) const {
        if (!node) return;
        
        // Escribir datos del nodo actual (preorder: raíz primero)
        os << node->getDataRef() << " ";
        os.write(reinterpret_cast<const char*>(&node->getRef()), sizeof(Ref));
        os << " ";
        
        // Escribir hijos (preorder continúa con subárbol izquierdo y luego derecho)
        internalWritePreorder(node->getChild(0), os);
        internalWritePreorder(node->getChild(1), os);
    }

public:
    // Versión alternativa más simple si solo necesitas los datos
    void writePreorderSimple(ostream &os) const {
        internalWritePreorderSimple(m_pRoot, os);
        os << endl;
    }

private:
    void internalWritePreorderSimple(Node* node, ostream &os) const {
        if (!node) {
            os << "# "; // Marcador para nodo nulo
            return;
        }
        
        // Escribir datos del nodo actual
        os << node->getDataRef() << " ";
        
        // Escribir hijos
        internalWritePreorderSimple(node->getChild(0), os);
        internalWritePreorderSimple(node->getChild(1), os);
    }
};

// TODO: Arriola Aldo
// operator <<
template <typename Traits>
ostream & operator<<(ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements." << endl;
    
    // Usar preorder para escribir manteniendo la estructura del árbol
    obj.writePreorder(os);
    return os;
}

// TODO: Toledo Oscar
template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    obj.Read(is);
    return is;
}

// ====== Impresión vertical tipo diagrama con / y \ (header-only) ======
inline int BinaryTree::height(Node* n) const {
    if (!n) return -1;
    int hl = height(n->left);
    int hr = height(n->right);
    return (hl > hr ? hl : hr) + 1;
}

inline void BinaryTree::buildVerticalCanvas(std::vector<std::string>& canvas,
                                            Node* n, int row, int col, int offset,
                                            int nodeWidth) const {
    if (!n) return;

    // --- 1) Escribir el valor centrado en nodeWidth ---
    std::ostringstream oss;
    oss << n->value;              // ⚠️ Si tu campo NO se llama 'value', cambia por 'data' o 'key'
    std::string val = oss.str();

    int start = col - (nodeWidth / 2);
    if (start < 0) start = 0;
    int pad = (int)((nodeWidth - (int)val.size()) / 2);
    if (pad < 0) pad = 0;

    for (int i = 0; i < (int)val.size(); ++i) {
        int pos = start + pad + i;
        if (row >= 0 && row < (int)canvas.size() &&
            pos >= 0 && pos < (int)canvas[row].size()) {
            canvas[row][pos] = val[i];
        }
    }

    // --- 2) Conector izquierdo y recursión ---
    if (n->left) {
        int childCol = col - offset;
        int slashCol = (col + childCol) / 2;
        if (row + 1 < (int)canvas.size() &&
            slashCol >= 0 && slashCol < (int)canvas[row + 1].size()) {
            canvas[row + 1][slashCol] = '/';
        }
        int nextOffset = std::max(1, offset / 2);
        buildVerticalCanvas(canvas, n->left, row + 2, childCol, nextOffset, nodeWidth);
    }

    // --- 3) Conector derecho y recursión ---
    if (n->right) {
        int childCol = col + offset;
        int slashCol = (col + childCol) / 2;
        if (row + 1 < (int)canvas.size() &&
            slashCol >= 0 && slashCol < (int)canvas[row + 1].size()) {
            canvas[row + 1][slashCol] = '\\';
        }
        int nextOffset = std::max(1, offset / 2);
        buildVerticalCanvas(canvas, n->right, row + 2, childCol, nextOffset, nodeWidth);
    }
}

inline void BinaryTree::PrintVertical() const {
    if (!root) {                  // ⚠️ Si tu puntero raíz NO se llama 'root', cámbialo aquí
        std::cout << "(árbol vacío)\n";
        return;
    }

    // Altura y dimensiones del lienzo (canvas)
    int h = height(root);
    int rows = 2 * h + 1;

    const int nodeWidth = 3;      // Sube a 4/5 si tus valores son anchos (3 dígitos+)
    int baseCols = (1 << (h + 1)) - 1;   // 2^(h+1) - 1
    int cols = baseCols * nodeWidth;

    std::vector<std::string> canvas(rows, std::string(cols, ' '));

    int rootCol = (cols - 1) / 2;
    int cellOffset = (1 << h) / 2;
    if (cellOffset < 1) cellOffset = 1;
    int pixelOffset = cellOffset * nodeWidth;

    buildVerticalCanvas(canvas, root, 0, rootCol, pixelOffset, nodeWidth);

    // Imprimir con rtrim de espacios
    for (auto& line : canvas) {
        int end = (int)line.size() - 1;
        while (end >= 0 && line[end] == ' ') --end;
        if (end < 0) std::cout << '\n';
        else std::cout << line.substr(0, end + 1) << '\n';
    }
}
// ====== Fin impresión vertical ======


void DemoBinaryTree();

#endif // __BINARY_TREE_H__
