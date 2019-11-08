# concurrentTextEditor
Concurrent text editor project. 

Naming conventions
 - Types start with capitals: MyClass - MyTemplate - MyNamespace
 - functions and variables start with lower case: myMethod
 - constants are all capital: const int PI=3.14159265358979323;
 - private member variables start with  "_" e.g: private: int _x;

Practical coding notes:
 - Usare il passaggio delle variabile per reference e non i puntatori: se passiamo per reference non dobbiamo preoccuparci che il puntatore possa avere valore nullo: si scatena un'eccezione automaticamente
