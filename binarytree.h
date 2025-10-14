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
#include <iostream>

using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::value_type;
  using Node = CBinaryTreeNode<Traits>;

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
    const value_type&  getDataRef() const       {   return m_data;    }
 
    void      setpChild(Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch) const { return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
    const Ref& getRef() const { return m_ref; }
};

template <typename _T>
struct BinaryTreeAscTraits{
    using  value_type = _T;
    using  Node       = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  Func       = less<value_type>;
};

template <typename _T>
struct BinaryTreeDescTraits{
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
    
    Node* internal_insert(value_type &elem, Ref ref, void* /*value*/,
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
        clear();
    } 
    
    // Limpia el árbol en postorden
    // Wrapper simple: solo recibe el functor
    template <typename Function>
    void postorder(Function func) {
        postorder_impl(m_pRoot, 0, func);
    }
    
    // Implementación recursiva con nombre distinto (evita choque de sobrecargas)
    template <typename Function>
    void postorder_impl(Node* pNode, size_t level, Function func) {
        if (pNode) {
            postorder_impl(pNode->getChild(0), level + 1, func);
            postorder_impl(pNode->getChild(1), level + 1, func);
            func(pNode, level);
        }
    }

    void clear() {
        postorder([](Node* node, size_t) { delete node; });
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
            os << string(level * 3, ' ') << " | " << pNode->getDataRef()
               << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
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

    // Serialización
    void Write(ostream &os) { os << *this; }
    void Read(istream &is)  { 
        clear();
        size_t elementCount;
        is >> elementCount;
        if (elementCount > 0) {
            std::vector<std::pair<value_type, Ref>> elements;
            elements.reserve(elementCount);
            for (size_t i = 0; i < elementCount; ++i) {
                value_type data; Ref ref;
                is >> data;
                is.read(reinterpret_cast<char*>(&ref), sizeof(Ref));
                elements.emplace_back(data, ref);
            }
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
        auto [data, ref] = elements[index++];
        Node* newNode = CreateNode(parent, data, ref);
        newNode->getChildRef(0) = internalReadPreorder(is, newNode, elements, index);
        newNode->getChildRef(1) = internalReadPreorder(is, newNode, elements, index);
        return newNode;
    }

public:
    void writePreorder(ostream &os) const {
        os << m_size << "\n";
        if (m_pRoot) internalWritePreorder(m_pRoot, os);
    }

private:
    void internalWritePreorder(Node* node, ostream &os) const {
        if (!node) return;
        os << node->getDataRef() << " ";
        os.write(reinterpret_cast<const char*>(&node->getRef()), sizeof(Ref));
        os << " ";
        internalWritePreorder(node->getChild(0), os);
        internalWritePreorder(node->getChild(1), os);
    }

public:
    void writePreorderSimple(ostream &os) const {
        internalWritePreorderSimple(m_pRoot, os);
        os << endl;
    }

private:
    void internalWritePreorderSimple(Node* node, ostream &os) const {
        if (!node) { os << "# "; return; }
        os << node->getDataRef() << " ";
        internalWritePreorderSimple(node->getChild(0), os);
        internalWritePreorderSimple(node->getChild(1), os);
    }

public:
    // ==== Impresión vertical (top-down con / y \) ====
    void PrintVertical() const;

private:
    int  height(Node* n) const;
    void buildVerticalCanvas(std::vector<std::string>& canvas,
                             Node* n, int row, int col, int offset,
                             int nodeWidth) const;
};

// operadores de flujo
template <typename Traits>
ostream & operator<<(ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements." << endl;
    obj.writePreorder(os);
    return os;
}
template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    obj.Read(is);
    return is;
}

// ====== Implementación de impresión vertical ======
template <typename Traits>
inline int CBinaryTree<Traits>::height(Node* n) const {
    if (!n) return -1;
    int hl = height(n->getChild(0));
    int hr = height(n->getChild(1));
    return (hl > hr ? hl : hr) + 1;
}

template <typename Traits>
inline void CBinaryTree<Traits>::buildVerticalCanvas(std::vector<std::string>& canvas,
                                                     Node* n, int row, int col, int offset,
                                                     int nodeWidth) const {
    if (!n) return;

    // 1) escribir el valor centrado
    std::ostringstream oss; oss << n->getDataRef();
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

    // 2) izquierdo
    if (n->getChild(0)) {
        int childCol = col - offset;
        int slashCol = (col + childCol) / 2;
        if (row + 1 < (int)canvas.size() &&
            slashCol >= 0 && slashCol < (int)canvas[row + 1].size()) {
            canvas[row + 1][slashCol] = '/';
        }
        int nextOffset = std::max(1, offset / 2);
        buildVerticalCanvas(canvas, n->getChild(0), row + 2, childCol, nextOffset, nodeWidth);
    }

    // 3) derecho
    if (n->getChild(1)) {
        int childCol = col + offset;
        int slashCol = (col + childCol) / 2;
        if (row + 1 < (int)canvas.size() &&
            slashCol >= 0 && slashCol < (int)canvas[row + 1].size()) {
            canvas[row + 1][slashCol] = '\\';
        }
        int nextOffset = std::max(1, offset / 2);
        buildVerticalCanvas(canvas, n->getChild(1), row + 2, childCol, nextOffset, nodeWidth);
    }
}

template <typename Traits>
inline void CBinaryTree<Traits>::PrintVertical() const {
    if (!m_pRoot) { std::cout << "(árbol vacío)\n"; return; }

    int h = height(m_pRoot);
    int rows = 2 * h + 1;

    const int nodeWidth = 3;           // sube a 4/5 si tus números son más anchos
    int baseCols = (1 << (h + 1)) - 1; // 2^(h+1) - 1
    int cols = baseCols * nodeWidth;

    std::vector<std::string> canvas(rows, std::string(cols, ' '));

    int rootCol = (cols - 1) / 2;
    int cellOffset = (1 << h) / 2;
    if (cellOffset < 1) cellOffset = 1;
    int pixelOffset = cellOffset * nodeWidth;

    buildVerticalCanvas(canvas, m_pRoot, 0, rootCol, pixelOffset, nodeWidth);

    for (auto& line : canvas) {
        int end = (int)line.size() - 1;
        while (end >= 0 && line[end] == ' ') --end;
        if (end < 0) std::cout << '\n';
        else std::cout << line.substr(0, end + 1) << '\n';
    }
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__

