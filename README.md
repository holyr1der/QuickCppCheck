##ABOUT

QuickCppCheck is a C++ testing tool trying to imitate Haskell's
quickcheck. While there are other attempts to do so, like QuickCheck++
and CppQuickCheck, they provide an OOP interface which just doesn't feel
'right'. QuickCppCheck, on the other hand, provides a functional interface
where properties are defined as functions that take arbitrary number of arguments
and return bool, much like quickcheck.

Moreover, it allows the customization of the generators for each argument
separately, either by using the provided generators for the primitive types,
or by providing a function that generates random values of the appropriate type.

Note that QuickCppCheck makes heavy use of new C++11 features and is incompatible
with C++03.

###Properties

Properties are defined as function objects, which can be function pointers,
classes with an operator(), lambdas, pretty much anything that is callable
in C++.

For each property function, an object of type Property has to be created, which
then can be customized and run. This can be done with the help of the function
qcppc::property e.g.

    qcppc::property(prop_fun, "Test property")(1000);

This will create a Property object and run 1000 tests.

###Arbitrary class

Class template Arbitrary<T> may be specialized for user defined types,
in order for QuickCppCheck to be able to automatically generate random
instances of that type. The specialization should provide at least
a constructor with no arguments and a call operator with type T operator()().

Given that Arbitrary<T> is specialized for type MyType, then the following
can be used:

    qcppc::property([](MyType t) {/*...*/ })()

Additional constructors may be defined and later used with the help of
the template function Rnd of class Property, e.g.

    qcppc::property([](MyType t1, MyType t2) { /* ... */ })
        .Rnd<0>(/* ... args for Arbitrary<MyType> constructor... */)
        .Rnd<1>(/* ... args for Arbitrary<MyType> constructor... */)
        ();

Rnd<N> simply forwards its arguments to the constructor of Arbitrary<T>,
where T is the type of the argument at position N, and sets Arbitrary<T>(args...)
as the generator for the argument at position N.

Specializing class template Arbitrary<T> is not the only way to create generators
for user defined types. It can also be done with a plain function which
accepts no arguments and returns an object of that type, e.g.

    MyType my_random();

    qcppc::property(...)
        .Rnd<0>(my_random);

###Other generators

Other generators my be defined with the following functions:

    qcppc::Property::Fix<N>(const T &)
    qcppc::Property::One<N>(const std::vector<T> &)
    qcppc::Property::Frq<N>(const std::map<T, double> &)

where T is the type of the argument at position N.

A parameter of the property function can be fixed to a specific value with
the template function Fix:

    // y will be always 0
    qcppc::property([](int x, int y){...})
        .Fix<1>(0)
        ();


A parameter can take its value from a predefined set of values with the template
function One:

    //x will get randomly a value from the given list
    qcppc::property([](int x){...})
        .One<0>({1,3,42})
        ();


Template function Frq offers the weighted version of One:

    //x will get false with twice the probability of getting true
    qcppc::property([](bool x){...})
        .Frq<0>({{false,2},{true,1}})

###Examples

Project Euler problem 9

    A Pythagorean triplet is a set of three natural numbers, a  b  c, for which,
    a2 + b2 = c2

    For example, 32 + 42 = 9 + 16 = 25 = 52.

    There exists exactly one Pythagorean triplet for which a + b + c = 1000.
    Find the product abc.

We define a property that fails on a Pythagorean triplet and then we try
to fail that property with QuickCppCheck.

    //Solving Euler problem 9!
    //will, hopefully, fail with the solution!
    property([] (int a, int b, int c, int d)
                    {
                        return (a*a + b*b != c*c) ||
                            (a + b + c != d);
                    },
        "Euler problem 9!!")
        //a, b and c can't be bigger than 500
        .Rnd<0>(1, 500)
        .Rnd<1>(1, 500)
        .Rnd<2>(1, 500)
        //d must be always 1000
        .Fix<3>(1000)
        //run and hopefully will hit the solution
    (100000000);

