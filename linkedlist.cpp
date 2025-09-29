#include <iostream>
#include "linkedlist.h"

template <typename T>
struct LLinkedListAsc{
    using value_type = T;
    using Func       = std::less;
};

template <typename T>
struct LLinkedListDesc{
    using value_type = T;
    using Func       = std::greater;
};

void DemoLinkedList(){
    CLinkedList< LLinkedListAsc<int> > l1;
    int x = 5;
    l1.Insert(x, 3);
    x = 7;
    l1.Insert(x, 14);
    std::cout << l1 << std::endl;

    CLinkedList<LLinkedListDesc<float>> l2;
    float x = 5.3;
    l2.Insert(x, 3);
    x = 7.2;
    l2.Insert(x, 14);
    std::cout << l2 << std::endl;
    
    // std::cout << l1 << x << "hola";
    // CLinkedList<float> l2;
    // float f = 3.14;
    // l2.Insert(f, 17);
    // std::cout << l2;
}