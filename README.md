##ABOUT

QuickCppCheck is a C++ testing framework much inspired by Haskell's  
quickcheck. Currently it's more a proof of concept rather than a 
working testing library.

##Usage

Each property that is to be tested is expressed as a callable (lambda, function 
pointer, function object) that accepts an arbitrary number of arguments and 
returns a bool. 

Let's say we have a function that divides its argument by 2.

    int div2(int n){
        return n / 2;
    }

The above function should have obviously the following property
(this should return always true)

    bool prop_f(int n) {
        return 2 * f(n) == n;
    }

But, apparently, it will fail as soon as it gets an odd number!
The following code will run prop_f on a number of arbitrary inputs and will
eventually catch the bug!

    Property<int>(prop_f) ();

##More Advanced Example 

###Euler problem 9

    A Pythagorean triplet is a set of three natural numbers, a  b  c, for which,
    a2 + b2 = c2

    For example, 32 + 42 = 9 + 16 = 25 = 52.

    There exists exactly one Pythagorean triplet for which a + b + c = 1000.
    Find the product abc.

---
Arbitrary data generators can be overridden (independently for each argument of
the property callable) by creating objects of type 

    Arbitrary<T, I>

where I is the position of the argument and T its type.
eg.

    Property<int>(prop_f)
    <= Arbitrary<int, 0>([] () -> int { return rand() % 10;}) ();

will create and arbitrary data generator that provides ints and attach it to 
0th argument of prop_f. Arbitrary can accept any callable as long as it has the 
correct signature (that should be T()).



    //Solving Euler problem 9!
    //will, hopefully, fail with the solution!
    (Property<int, int, int, int>([](int a, int b, int c, int d){
                                          return (a*a + b*b != c*c) ||
                                                  (a + b + c != d);
                                            },
                                        "Euler problem 9!!")
        //a, b and c can't be bigger than 500
        <= Arbitrary<int, 0>([]() {return rand() % 500;})
        <= Arbitrary<int, 1>([]() {return rand() % 500;})
        <= Arbitrary<int, 2>([]() {return rand() % 500;})
        //d must be always 1000
        <= Arbitrary<int, 3>([]() {return 1000;}))
        
        //run 1000000 times and hopefully will hit the solution
        (1000000);
