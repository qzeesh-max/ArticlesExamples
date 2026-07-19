# CRTP and Homogeneous Storage via Type-Erasure

*May 11^ th, 2017 By Zeeshan Qazi*

CRTP (Curiously Recurring Template Pattern) is described as a mechanism for avoiding having to have slow virtual functions dispatched from a base class. Only problem one runs into with CRTP is that there is no way to store the derived classes of the same CRTP base homogeneously in the same container as a member. Usually, people get around this by having a base class with virtual functions, but that means that binding between the base and the container cannot be arbitrary, meaning the container and the contained need to be aware of one-another.

There is a way around this problem, whereby we can have CRTP objects of different types derived from same CRTP base stored homogeneously within a container object. This may be accomplished by mere use of type-erasure, by which the pre-bounded sets of methods keyed by typeid of visitor objects, which may be visited by the dynamic dispatch based on visitor accepted by the container.

There are libraries that provide tools that may be used to implement similar functionality like LOKI and up-and-coming Open Multi-Methods for C++, except both of them are generalized solutions requiring further work to implement such a homogeneous storage specifically for CRTP. This tutorial steps in to show you from scratch, how this can be done, without going into various performance and memory optimizations possible for this solution.

The intent of this tutorial is to show how this can be done from the ground up. Which basically means we would walk through:

- How type lists can be used to specialize template methods for multiple visitor types and contained types?
- How type-erased void-pointer argument based methods can be bound to concrete type-safe methods via specialized template methods?
- How the CRTP objects derived from the same CRTP-base that are passed into another object's method can seamlessly integrate with visitor methods in the class of that object or classes provided by the class of the object while implementing a container?
- How the dynamic dispatch mechanism could potentially be implemented for each one of the types of the stored objects in the container?

First let us list out all the headers that we would use at start of our example:

[   Download](javascript:DoLink('/download-sourcecode.php?Example=template_variant');)
   Compilation Instructions:  g++ template_variant.cpp -std=c++14 -o ./template_variant

*Refer to the source file `template_variant_1.hpp` in this directory.*

For type-lists we are using our custom type list object that only has one meta-method to apply the type-list to another template. Here is what our ListTypes template class looks like:

*Refer to the source file `template_variant_2.hpp` in this directory.*

We have a helper class we use for binding the visitor method inside the visitor object to a concrete method. The VisitorBinding template takes care of that for us and applies appropriate conversions for the arguments and acts as the last leg of our dispatch to the visitor:

*Refer to the source file `template_variant_3.hpp` in this directory.*

We have a helper class to determine whether the provided type is actually derived from the same CRTP base. HasSameBase takes a template template class argument which represents the base and the actual type passed-in. It uses SFNAE to determine if the passed in type is derived from any class that is a specialization of CRTP:

*Refer to the source file `template_variant_4.hpp` in this directory.*

The actual container that can store homogeneously the CRTP-base derived object is a template class consisting of these template arguments:

- **TEMPL** – This template template class argument is basically the CRTP base for the template.
- **HINT** – Type of the value that is passed to the visitors as a hint as to why they are being called. Essentially meant to provide extra context.
- **ALLOWED_VISITORS** – This is meant to be a list of visitors that are allowed to receive values from this container. This is how the binding magic works, by pre-providing the visitor classes.

*Refer to the source file `template_variant_5.hpp` in this directory.*

Here is our sample usage example to utilize our class.Though the sample clearly does not show actual usage with CRTP for the sake of keeping things brief, you can clearly get the idea from it how it would work with CRTP just as well.

*Refer to the source file `template_variant_6.hpp` in this directory.*

Sample output from our program above:

`
- ```text
1: First Visitor = 10
2: Second Visitor = 10
3: First Visitor = 10
3: First Visitor = awesome
24: Second Visitor = 0
24: Second Visitor = arb-store-2
```

`

---
