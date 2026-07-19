# Building a Sortable Random Access Tree

*February 2^ nd, 2014 By Zeeshan Qazi*

Though **`boost`** and **`STL`** both provide a variety of containers, sometimes it becomes necessary to design your own containers, simply because the containers that are provided are simply not meant to provide you with the abilities you are seeking. For example, if you are looking for a container that provides the insertion and deletion speeds of a linked container (eg. no shifting of a large array to insert or remove an element), random accessibility (eg. positional indexing) that is actually viable for large amounts of data, and ability to dynamically switch sorting keys, finding an efficient solution using **`boost`** or **`STL`** is nowhere near trivial anymore. In such cases people are either forced to make poor choices or stick with what they have, especially due to time constraints or fears of a pilot implementation of a data structure.

It has actually become a common practice amongst the programmers to be overly reliant on prepackaged libraries of containers. There is nothing wrong with using prepackaged libraries of containers, if they work for your solution efficiently, but the fact is that even a Swiss Army Knife is not an omnipotent tool. The reason we are all taught those data structures at the university level is so that we all understand their function and to provide us a good understanding of these structures, so when time comes for us to decide on which ones to use for implementation of complex problems, we are able to decide on appropriate ones or design new ones as needed.

Frankly, there appears to be a general trend plaguing our education system that is well-evident from some studies that were done recently. For instance, Jon Bentley in his book *Programming Pearls (2nd edition), page 341,* states it is a problem in course for professional programmers, where an astounding ninety percent of them fail to code a binary search implementation correctly after several hours of working on its implementation. It has also been found that only five out of twenty textbooks contain the correct algorithm for binary search (see: [Pattis, Richard E. (1988). "Textbook errors in binary searching". SIGCSE Bulletin 20: 190 194](https://dl.acm.org/citation.cfm?doid=52965.53012)).

The fact that we have **`STL`** and **`boost`** or the containers provided by the Microsoft's .NET Framework means that most programmers will likely never ever venture to write their own containers. They may even end up relying on the limited number of containers to implement complex problems that could have been better implemented using the ones not provided by any of the stock implementations. The lack of confidence in ones own ability to develop new implementations of containers when needed is fuelled by our production line treatment of programmers. The idea that programmers must deliver solutions to very complex problems in short time means that we are subjecting them to be very fallible to making fairly basic errors, and fearful of taking risks.

In the end it was not much more than a very human instinct to try and test things that brought to us many of our innovations. It is normal to make mistakes and there is no reason to fear making them, as there is no such thing as absolute perfection. Our fear of trying out new things is nothing more than an impediment to our future development and ability to innovate. Of course, it does not mean you sit down and design an equivalent to **`STL mapped containers`** just because you think you can do them better. There has to be a rational reason for implementation of a container, especially in a professional context, as we are liable for proper use of time in such a context.

Now let us move onto the problem we are trying to solve this time around. Let us assume that we want to implement a fairly complex website that contains AJAX-based display of filterable, sortable and searchable views into large amounts of data updating in real-time, with a different view visible to different users as per user's preferences and permissioning. Such an implementation may be needed for an orders blotter in a complex OMS system with realtime market data and large number of institutional orders. If one wants to implement such a website efficiently it makes very little sense to be repeatedly traversing large amounts of data to build different views for different users, while it would be clearly cheaper to maintain states of such views on the server-side available for the users to be displayed in a UI container on demand by acquiring partial or complete state of such views (or blotters) from the server-side.

Typically, implementation of a problem like this involves, a central container (or efficiently queryable light-weight database) whose copies are distributed across many servers on a server farm, with individual views used by users represented using lightweight trees that contain references to the actual data. One may also be required to maintain additional lightweight trees with a fixed set of indexes to make searching functionality a lot more efficient. Many of the readily available containers (with possible exception of **`boost multiindex`**) do not really provide for the an efficient mechanism for gaining access to the benifits of array-like indexing. This type of indexing is essential for displaying large amounts of data in paged-manner or in a scrollable container (such as virtual UI containers in many UI frameworks).

If you have been reading this forum, you may remember that we had implemented a container suitable for efficient random positional access for large amounts of data, while providing a container somewhat similar to **`STL map`** in another article (see: [Indexing into Associative Containers and Augmented Red-Black Trees](https://www.zeeshan.im/articles.php?Page=8#augmentedredblacktree)). We will take a step further and design another container that will behave sort of like a sorted list like the one implemented in C# [SortedList](https://msdn.microsoft.com/en-us/library/system.collections.sortedlist(v=vs.110).aspx) using arrays, but we would implement it like a tree, and with ability to change the sort keys on the fly. We would be implementing this functionality by borrowing some of the code from our existing augmented red-black tree that we wrote earlier, and modifying it to make this new container.

Our implementation is designed for usability for storage of relatively heavy data structures directly into the container, instead of pointers of those in the container. Though a more efficient implementation designed for storage of pointers to objects held elsewhere is possible in this container is possible, such an implementation is identical in computational complexity, with only minor modification to the code we are about to exhibit in this article.

Let us recall that the Augmented Red-Black Tree, we had implemented earlier had the following augmentations:

- **Insertion of new nodes** – We will need to modify the traditional *BST Insert* to add additional logic to update all the parent nodes along the path of a successful insertion to increment the counter in each node.- **Deletion of a node** – We will need to modify the traditional *BST Delete* to to add additional logic to update all the parent nodes along the path of a sucessful deletion to decrement the counter in each node.- **Rotations / House-keeping on insertion or deletion** – We will need to modify the pivot operations that are done to keep the Red-Black tree maintain the definition of Red-Black tree.

We will modify this container to eliminate from each node the explicit ability or need of carriage of the key used for arranging the Red-Black Tree, but we will continue to use the Red-Black Tree style arrangment albeit now with a dynamic key no longer stored in the node itself. We will first start by implementation of a mechanism for providing keys to the container dynamically. This will be facilitated by implementation of a new class called **`DynamicComparatorBuilder`**, meant to be used for creation of a sort key for the Red-Black Tree. We will be using some of other classes we wrote for implementation of reflection earlier to provide for the ability to have dynamic searches of our tree structure using the dynamically assigned keys, with proper error checking. We are keeping our implementation concise by leaving out the ability to pass arguments to the search functions as a **`STL list`** of variantly typed components of the search keys, but such an implementation would be relatively trivial addition and a definite requirement for a container like the one we are implementing.

We will now walk-through the implementation of the  **`DynamicComparatorBuilder`** class. It is a very simple class consisting of a simple list of instances of an interior class called **`FieldComparator`**, which will be used by our tree container for sorting its elements, searching for specific matches to the sort key and for re-sorting the container once the user decides to change the key. The sorting functionality is provided by two methods implemented in a local class in the two versions of **`DynamicComparatorBuilder::AddField`** method. The two methods that are used for sorting the tree are:

- **`FieldsComparatorAscending::SortComparator`**

- **`FieldsComparatorDescending::SortComparator`**

Both of the above methods are called once for every component of the sort key and chained together using short-circuited boolean evaluation. The other two methods in the local classes **`FieldsComparatorAscending`** and **`FieldsComparatorDescending`** are used for searching the tree using a key provided by the caller. Those methods are:

- **`FieldsComparatorAscending::SearchComparator`**

- **`FieldsComparatorDescending::SearchComparator`**

Both of the above methods are invoked using reflection, because of need to invoke them with variably typed arguments as per the components of the sort key. Let us now look at the entire header file that contains the actual implementation of the **`DynamicComparatorBuilder`** class.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=dynamic_comparator_builder');)

*Refer to the source file `dynamic_comparator_builder.hpp` in this directory.*

Now let us move onto the implementation of the container. The container has the following features:

- **Insertions or Deletions** – The insertions and deletions in the container may be performed on the container as operations of *O(log N)* complexitiy.

- **Traversing the container to the *nth* item by index** – The container can be traversed to any item by index with operational complexity of *O(log N)*, with subsequent items accessible more expediently using the iterators provided.

- **The container is re-sortable** – The container must be created with `DynamicComparatorBuilder` instance providing its default sort order, which can be replaced at anytime by a call to a member function. This key used by the `DynamicComparatorBuilder` is not stored in the container, and can also be used for searching for items in the container. The container uses the **Quick Sort** algorithm for performing sort operations, and leaves user objects in the container in-place while adjusting their order by changing the links between the various nodes in the container.

- **The container behaves like a multi-map** – The container supports multiple items with identical keys, and provides operations for seeking out items along with their positional index by these keys with operational complexity of *O(log N)*.

- The container uses reflection to provide ability to search the container using multi-field keys that consist of arbitrarily typed components.

We will now proceed with the implementation of the container, which is loosely based on the Augmented Red-Black Tree container we had implemented previously.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=sortable-random-access-tree');)

*Refer to the source file `sortable_random_access_tree.hpp` in this directory.*

The above code is dependent on the source code provided in our implementation of the reflection, which may be downloaded using the following links:

- [/reflection/reflection_helpertypes.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection_helpertypes)

- [/reflection/reflection_call.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection_call)

- [/reflection/reflection_method.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection_method)

- [/reflection/reflection_constructor.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection_constructor)

- [/reflection/reflection_class.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection_class)

- [/reflection/reflection_module.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection_module)

- [/reflection/reflection.hpp](https://www.zeeshan.im/download-sourcecode.php?Example=reflection)

Now we can move onto providing an example for testing the classes we provided in the article. 

[   Download](javascript:DoLink('/download-sourcecode.php?Example=sortable-random-access-tree-example');)
   Compilation Instructions:  g++ -std=c++11 sortable-random-access-tree-example.cpp -o sortable-random-access-tree-example

*Refer to the source file `sortable_random_access_tree_example.cpp` in this directory.*
