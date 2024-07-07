# A Two-Pass SIC-XE assembler

<dl>
<dt>Course Name</dt>
<dd>Algorithmic Problem Solving</dd>
<dt>Course Code</dt>
<dd>23ECSE309</dd>
<dt>Name</dt>
<dd>Shrihari Hampiholi</dd>
<dt>University</dt>
<dd>KLE Technological University, Hubballi</dd>
</dl>

* * *

[comment]: # (> Robust implementation of a Two-Pass Assembler for the SIC/XE architecture)

#### Note:
This page hosts:

1. Pre-requisites
2. Introduction
3. Objectives
4. Design
5. Challenges
6. Future Scope



* * *

### Prerequisites
* Basic understanding of the SIC and SIC/XE architecture.  The staple recommendation to learn this would be the [Leland Beck](https://www.amazon.in/System-Software-Introduction-Systems-Programming/dp/0201423006) book.

* Modern C++ knowledge.  The code is written with features from C++11 and above.  The code is written in a way that it is easy to understand and follow, and is well-commented and structured!.


_Below is an introduction that I feel would be helpful to understand the project on an abstract level.  Feel free to skip to the next section if you are already familiar with the concepts._

### Introduction
* Understanding of the two-pass assembler:
    - An assembler is essentially an algorithm that is used to convert a respective assembly code into the respective machine code.  A two-pass assembler does this exact thing in _2 distinct steps_.  
    The first pass is used to generate the symbol table and the second pass is used to generate the object code.

* Understanding of the SIC/XE architecture:
    - The [SIC/XE](https://en.wikipedia.org/wiki/Simplified_Instructional_Computer#:~:text=There%20is%20also%20a%20more%20complicated%20machine%20built%20on%20top%20of%20SIC%20called%20the%20Simplified%20Instruction%20Computer%20with%20Extra%20Equipment%20(SIC/XE)) architecture is an *extension* of the [SIC](https://en.wikipedia.org/wiki/Simplified_Instructional_Computer) architecture.  The SIC/XE architecture has more features than the SIC architecture.  The SIC/XE architecture supports **more registers**, **more addressing modes**, and **more instructions** than the SIC architecture, making it _more powerful and versatile_ due to the hardware enhancements.

* Understanding of the SIC/XE instructions:
    - The SIC/XE instructions are divided into three categories: 1. Format 1 instructions, 2. Format 2 instructions, and 3. Format 3/4 instructions.  The Format 1 instructions are the simplest instructions.  The Format 2 instructions are the instructions that have two operands.  The Format 3/4 instructions are the instructions that have three operands.

* Understanding of the SIC/XE addressing modes:
    - The SIC/XE addressing modes are divided into five categories: 
        1. Immediate addressing mode
            - This mode is used to specify the value of the operand directly
        2. Direct addressing mode
            -  This mode is used to specify the address of the operand directly
        3. Indirect addressing mode
            - This mode is used to specify the address of the operand indirectly. 
        4. Indexed addressing mode
            - This mode is used to specify the address of the operand indirectly with an index register. 
        <!-- 5. Base-relative addressing mode.  
            - This mode is used to specify the address of the operand indirectly with a base register. -->

* Understanding of the SIC/XE directives:
    - The SIC/XE directives are used to specify the attributes of the program. 
    These are divided into two categories: 
        1. Assembler directives
            - The Assembler directives are used to specify the attributes of the program that are used by the assembler. 
            Examples of such directives are:  `START, END, BYTE, WORD, RESB, and RESW`.
        2. Machine directives.  
            - The Machine directives are used to specify the attributes of the program that are used by the machine.
            Examples of such directives are: `LDA, STA, ADD, SUB, MUL, DIV, and COMP`.

* Understanding of Control Sections:
    - Control Sections allow for the separation of the program into multiple sections that embrace the readability and maintainability of the program. 
    - The Control Section is a feature of the SIC/XE architecture that allows for the separation of the program into multiple sections.  The Linker, at link-time, is responsible to link the multiple sections of the program into a single executable program.


### Objectives
* The primary objective of this project is to implement an efficient Two-Pass assembler for the SIC/XE architecture.
* The secondary objective of this project is showcase an efficient linker that can link multiple sections of the program into a single executable program.
* The tertiary objective of this project is to showcase an efficient loader that can load the executable program into the memory and execute it.


### Design
* The design of the Two-Pass assembler is divided into two distinct steps: 
    1. The 1st pass
        - This is used to _generate the Symbol Table_ respective to **each Control Section**.  The symbol table is used to store the symbols and their respective addresses to help in the second pass.
    2. The 2nd pass
        - This is used to _generate the Object Code_ that is **further utilised by the Linker and Loader** to process the program.  The object code is generated by using the symbol table that was generated in the first pass.

### Possible Data Structures that can be used for the Two-Pass Assembler:

####  For the First Pass:

1. **For the Symbol Table**
    - As the Symbol Table is used to store the symbols (variable names) and their respective addresses in the input ASM file, and requires the _below operations_:
    - Insert a symbol and its address
    - Search for a symbol and its address
    - Delete a symbol and its address,

We can use the following data structures:

##### a) Hash Table
A [Hash Table](https://en.wikipedia.org/wiki/Hash_table) is used to store the symbols and their respective addresses in an efficient manner.  It uses a hash function to map the symbols to their respective addresses.  The hash function tries to ensure the best that the symbols are stored in a unique location in the hash table.  The hash table ensures that the symbols can be searched, inserted, and deleted in constant time.

Provided a [good hash function](https://stackoverflow.com/questions/34595/what-is-a-good-hash-function), the **time complexity** of insertion, deletion, and search operations is **O(1)** on _average_.  It is _on average_ because there can be collisions in the hash table, which can increase the time complexity to **O(n)** in the worst case.  
The **space complexity** of the hash table is **O(n)** where `n` is the number of symbols in the program.

```cpp
Time Complexity: O(1) for insertion, deletion, and search operations on average.
Space Complexity: O(n) where n is the number of symbols in the program.
```

##### b) Self-Balancing Binary Search Trees
If it is very crucial to us that we have a strict upper bound on the time complexity of the operations, we can opt the Symbol Table to be made of [Self-Balancing Binary Search Trees](https://en.wikipedia.org/wiki/Self-balancing_binary_search_tree) like the [AVL Tree](https://en.wikipedia.org/wiki/AVL_tree) or the [Red-Black Tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree).  These trees ensure that the height of the tree is balanced, which ensures a constant time complexity for insertion, deletion, and search operations.

```cpp
Time Complexity: O(log n) for insertion, deletion, and search operations.
Space Complexity: O(n) where n is the number of symbols in the program.
```

##### c) Trie
A [Trie](https://en.wikipedia.org/wiki/Trie) is a tree-like data structure that is used to store a dynamic set of strings.  The Trie here can be used to store the symbols and their respective addresses in an efficient manner.  The Trie ensures that the symbols can be searched, inserted, and deleted in constant time with respect to the length of the symbol.


```cpp
Time Complexity: O(L) for insertion, deletion, and search operations where `L` is the length of the symbol.
Space Complexity: O(L*n) where `n` is the number of symbols in the program.
```

##### d) Skip List
A [Skip List](https://en.wikipedia.org/wiki/Skip_list) is a probabilistic data structure that enables fast search, insertion, and deletion operations. By maintaining multiple layers of forward pointers, Skip Lists allow operations to skip over large sections of the list, achieving average `O(log n)` time complexity for these operations. This makes Skip Lists an efficient and practical choice for symbol tables where logarithmic operation time is desirable.

```cpp
Time Complexity: Average O(log n) for insertion, deletion, and search operations.
Space Complexity: O(n), with higher constants due to additional pointers.
```


Implementations of the above data structures can be found in the `data_structures` directory.
1. [Hash Table](/data_structures/hash_table.hpp)
2. [AVL Tree](/data_structures/avl_tree.hpp)
3. [Red-Black Tree](/data_structures/red_black_tree.hpp)
4. [Trie](/data_structures/trie.hpp)
5. [Skip List](/data_structures/skip_list.hpp)

2) **For the Opcode Table and Register Table**
    - Both, the Opcode Table and the Register Table are **Static Data Structures** that are used to store the _opcodes_ and their _respective machine codes_, and the _registers_ and their _respective machine codes_ respectively, dictated by the **hardware of the SIC/XE architecture**.
    The Opcode Table is used to validate the instructions and their respective opcodes, and the Register Table is used to validate the registers and their respective machine codes.
        - In Pass 1 to validate the instructions and their respective opcodes, reserving proper space for the instructions in the Intermediate File.
        - In Pass 1 to validate the Opcode if it is even supported by the SIC/XE architecture hardware.
        - In Pass 2 to generate the Object Code by using the Opcode Table.

We can use the exact same data structures as mentioned above for the Symbol Table.  Due to the virtue of it being a static data structure, we can opt for a Hash Table, as we _only perform search operations_ on the Opcode Table during the first and second pass.  We can meticulously design the hash function to ensure that the opcodes are stored in a unique location in the hash table, yielding the best case time complexity of $\Omega(1)$ for search operations.

```cpp
Time Complexity: O(1) (Omega) for search operations on average.
Space Complexity: O(n) where n is the number of opcodes in the SIC/XE architecture.
```


2) **For the Intermediate File**
    - The Intermediate File is used to store the intermediate results of the first pass.  The intermediate file is used to store the Control Section, the Symbol Table, and the Program.  The intermediate file is used to generate the Object Code in the second pass.

We can use the following data structures:
##### a. Secondary Memory
- The Intermediate File can be stored in the secondary memory like the hard disk.  The Intermediate File can be stored in a file in the secondary memory.  The Intermediate File can be read from the secondary memory in the second pass to generate the Object Code.

```cpp
Time Complexity: Dependent on the size of the data, Hardware of the Secondary Storage, underlying Operating System and system performance, not strictly O(1).
Space Complexity: O(n) where n is the size of the Intermediate File.
```

##### b. In-Memory Data Structures
- The Intermediate File can be stored in the memory.  The Intermediate File can be stored in the memory in the form of a data structure like a vector or a list.  The Intermediate File can be read from the memory in the second pass to generate the Object Code.

```cpp
Time Complexity: 
    - O(1) for insertion and search operations.
    - O(n) for deletion operations.
Space Complexity: O(n) where n is the size of the Intermediate File.
```

#### For the Second Pass:


