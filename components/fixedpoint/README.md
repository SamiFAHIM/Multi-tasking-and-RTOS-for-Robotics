# Fixed point library

The purpose of this component is to provide a generic abstraction of the fixed point operations using meta-programming.

This component helps with fixed point arithmetic by defining a FixedPoint class optimized by template parameters.
The template parameters are :
    - the size in bit of the fractional part of the number. It is the position of the point in the number
    - the integer part size in bit : this is used as an information to implement some optimizations, especially on product and divisions. This is mostly informative and the integer part of the number may exceed this number of bit usually without consequences (except where the optimization take place)


## Considerations

The implementation is based on int base type. Some functions use int64_t to prevent overflow, and it is costly in terms of CPU time. If the template arguments allow optimization, it will be done at compilation time to allow the fastest execution possible.


The implementation support cross-template operation, that is to say operations between different template of the same class. Supported operations are :
    - assignation operator
    - relational and comparison operators : ==, !=, >, <, >=, <= (these operations are supported only when the fractional part sizes are equal)
    - compound assignment operator : +=, -=, *=, /= (optimization take place here for *= and /= operators)
    - arithmetic operators : +, -, *, / (the output type depends on inputs and the result follow generic loose of precision, aka the product of 2 numbers will have the fractional part of the number with the smallest fractional part's size)
    - type casting operator : current support types are int, double and float. Floating point support depends on configuration.


