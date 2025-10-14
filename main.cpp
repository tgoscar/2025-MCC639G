#include <iostream>
#include "doublelinkedlist.h"
int main(){
    DoublyLinkedList<int> L;
    L.push_back(1);
    L.push_front(0);
    L.push_back(2);
    std::cout << "size=" << L.size() << " front=" << L.front() << " back=" << L.back() << "\n";
    auto it = L.begin();
    L.insert(it, -1);
    L.insert(L.end(), 3);
    for (auto i=L.begin(); i!=L.end(); ){
        if (*i==2) i = L.erase(i);
        else ++i;
    }
    std::cout << "Contenido: ";
    for (const auto& v: L) std::cout << v << ' ';
    std::cout << "\nOK\n";
    return 0;
}
