##ABOUT

QuickCppCheck is a C++ testing framework much inspired by Haskell's  
quickcheck. Currently it's more a proof of concept rather than a 
working testing library.


##Example usage

    //Solving Eurler problem 9!
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
