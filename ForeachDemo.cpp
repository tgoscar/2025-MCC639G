#include <vector>
#include <string>
#include "foreach.h"
#include "types.h"
#include "util.h"

void DemoForeachInvoke(){
    std::vector<int> primes = {2, 3};
    // pass lambda as callable and an additional argument:
    foreach(primes.begin(), primes.end(),        // elements for 2nd arg of lambda
            [](std::string const& prefix, int i) // lambda to call
            { std::cout << prefix << i << '\n';
            }, 
            "- value: ");                        // 1st arg of lambda

    // call obj.memfunc() for/with each elements in primes passed as argument
    MyClassInvoke obj;
    foreach(primes.begin(), primes.end(),  // elements used as args
            &MyClassInvoke::memfunc,       // member function to call
            obj, "extra");                 // object to call memfunc() for

    foreach(primes.begin(), primes.end(),  // elements used as args
            &decltype(obj)::memfunc,       // member function to call
            obj, "New extra");             // object to call memfunc() for
}

void DemoInvoke()
{
    vector<int> vals{0, 8, 15, 42, 13, -1, 0};
    call([&vals]() -> void
    {  cout << "size: " << vals.size() << '\n';
    });
    call([&vals](int a)
    {  cout << "size: " << vals.size() << ", a=" << a << '\n';
    }, 5);
    print(vals);
    auto w0 = call(print, vals);
    cout << "w0=" << w0 << endl;

    call(&decltype(vals)::pop_back, vals);    // vals.pop_back();
    auto w1 = call(print, vals);
    cout << "w1=" << w1 << endl;

    call(&decltype(vals)::clear, vals);       // vals.clear();
    auto w2 = call(print, vals);
    cout << "w2=" << w2 << endl;

    cout << "Type of vals: " << type_name<decltype(vals)>() << endl;
    cout << "Type of w2: " << type_name<decltype(w2)>() << endl;

    auto pf = &print;
    cout << "Type of pf: " << type_name<decltype(pf)>() << endl;
}
