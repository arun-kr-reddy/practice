#include <iostream>
#include <vector>

int foo(int arg)
{
    printf("foo %d\n", arg);

    return arg;
}

int main()
{
    int (*func_ptr)(int);
    func_ptr = &foo;
    int ret  = (*func_ptr)(10);  // foo 10

    // OR
    func_ptr = foo;
    ret      = func_ptr(10);  // foo 10

    printf("ret %d %p %p\n", ret, func_ptr, *func_ptr);  // ret 10

    return 0;
}

// ./exe_main command line arguments

int main(int argc, char const *argv[])
{
    // argc == 4
    // argv[] == {"exe_main", "command", "line", "arguments"}
}