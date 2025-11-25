#ifndef __VARIADIC_TEMPLATE_H__
#define __VARIADIC_TEMPLATE_H__

template<typename T, typename ... Args>
T VarFn (T val){
    return val;
}

template<typename T, typename ... Args>
auto VarFn (T val, Args const& ... args){
    return val + VarFn( std::forward<Args>(args)... );
}

void VariadicDemo();

#endif