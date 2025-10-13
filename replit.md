# C++ Data Structures & Algorithms Project (12-BinaryTree Branch)

## Overview
This is an advanced C++ educational project for EDA - MCC639G that demonstrates implementation of fundamental data structures including linked lists, double linked lists, and binary trees using modern C++ templates and iterators.

## Project Structure
- `main.cpp` - Main entry point with demo calls
- `test.cpp/h` - Test utility functions
- `util.cpp/h` - Utility functions and templates
- `types.h` - Common type definitions
- `traits.h` - Template traits for data structures
- `linkedlist.h` - Template-based singly linked list implementation
- `doublelinkedlist.h` - Template-based doubly linked list implementation
- `binarytree.h` - Template-based binary search tree implementation
- `foreach.h` - Generic foreach implementation with variadic templates
- `pointers.cpp/h` - Pointer demonstrations
- `clasestemplate.cpp/h` - Template class examples
- `ContainersDemo.cpp` - Demonstrations of all container types
- `ForeachDemo.cpp` - Foreach functionality demos

## Build & Run
The project uses a Makefile for compilation:
```bash
make clean  # Clean previous builds
make        # Compile the project
./main      # Run the executable
```

Or simply run the workflow which does all three steps.

## Features

### Data Structures
1. **Linked List** (`linkedlist.h`)
   - Template-based implementation
   - Ascending and descending order support
   - Forward iterator support
   - Generic traits pattern

2. **Double Linked List** (`doublelinkedlist.h`)
   - Bidirectional iteration
   - Forward and backward iterators
   - Template-based with traits

3. **Binary Tree** (`binarytree.h`)
   - Binary search tree implementation
   - Multiple traversal methods (inorder, preorder, postorder)
   - Templated with comparison functors
   - Generic traits for ascending/descending order

### Advanced Features
- Template metaprogramming with traits
- C++17 features including variadic templates
- Iterator pattern implementation
- Foreach functionality with std::invoke
- Lambda expression support

## Contributors
1. Ernesto Cuadros-Vargas
2. Luis Antonio Selis Vasquez
3. Alex Segura
4. Juan Felix Arias Ramos
5. Oscar Toledo Guerrero
6. Richard Villanueva Retamozo
7. José Bazán
8. Aldo Arriola
9. David Quispe

## Recent Changes (12-BinaryTree Branch)
- Fixed compilation errors in binarytree.h
- Updated trait system to be compatible across all containers
- Fixed foreach.h to include functional header
- Corrected Makefile to remove non-existent vector.cpp
- Added proper template parameter resolution
- Fixed initialization order in CBinaryTreeNode

## Setup
The project is pre-configured for the Replit environment with C++ (Clang 14) toolchain installed. Simply run the workflow to compile and execute all demonstrations.

## TODO Items in Code
- Binary tree iterator implementation (Patrick Fuentes)
- Copy constructor for CBinaryTree (Luis Selis)
- Destructor implementation for CBinaryTree (Luis Selis)
- Read function for persistence (Various)
- General iterator base class implementation
