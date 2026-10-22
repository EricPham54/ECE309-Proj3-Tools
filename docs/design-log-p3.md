# Project 3 Design Log
**Author:** Eric Pham · **P2 code used:** instructor reference
**Tools used:** The main AI assistant I used was Claude's Sonnet model. I used it to understand what the Project 3 spec wanted me to do. Specifically, it helped me understand what a lambda is, how a lambda works, how to use the vector library (std::vector) for a hash table, how the FNV-1a hashing works, understand the naming conventions for class' private member data I saw from the P3 starter files, understand how to use std::unique_ptr<Tool> to not lose any buckets during rehashing, learn what to include in my private section of the tool_registry class, understand what the different steps of workspace::resolve meant, understand how to use the vector library (std::vector) as a stack for calculator_eval, understand what the calculator header file gives for calculator_eval, and understand what to write for the test cases for inserting 10000 tools in the tool registry using my hashing method and alternate hash method. 

## 1. Hash table
In terms of the collision strategy, I used separate chaining. Each bucket is a std::vector of std::unique_ptr<Tool>, so two names in the same bucket just share a vector/list. I didn't use linear probing because of its tendency to cause more collisions as more buckets are filled and its need for a method to make the buckets wrap around. With chaining, a collision only makes one vector/list a little longer up until the load factor I chose, which is 0.75, is achieved. My hash function uses the FNV-1a string hash from Appendix A of the Project 3 specification because it seemed simple enough to differentiate strings of various lengths. My load-factor threshold is 0.75 because it is the minimum recommended load factor for resizing a hash table using separate chaining from the Project 3 specification and allows for fewer collisions. In terms of the resize policy, I had the hash table double the bucket count, which starts at 1 to use as little resources as possible at first, every time add() would make the load factor more than the limit. 

My 10,000-tool numbers are 16384 for the bucket_count, 0.610352 for the load factor, and 6 for the max probe length. For the alternative hash's longest chain, it was 9000.

Add() is amortized O(1) because all add does is hash the name and push it onto the bucket or the back of one chain/vector, which is just one action. It doesn't need a for-loop to access a bucket in the hash table because it just needs the index of a hash table, and it just needs to use the push_back function to push an element to the back of a chain/vector. 

## 2. Pointer stability
Tool* returned by find() survives a resize in my design because the Tool objects live on the heap, and the buckets only hold unique_ptrs that point at them. When the table resizes, only those pointers move into the new buckets using the std::move function. The tools would just stay where they are in the heap. find() returns .get(), which is the address of the Tool, not the address of a slot in a bucket. Therefore, a pointer from before a resize still points at the same tool after it.

## 3. Calculator trace
The values stack holds numbers, and the ops stack holds operators and parentheses. For the expression 2 * (3 + 4) - 5:

1. 2 gets pushed onto the values stack, so values = 2

2. `*` gets pushed onto the empty ops stack, so ops = `*` 

3. `(` gets pushed onto the ops stack, ops = `*`, `(`.

4. 3 gets pushed onto the values stack, so values = 2, 3.

5. `+` then gets pushed onto the ops stack, but it's not a `)`, so ops = `*`, `(`, `+`

6. 4 gets pushed onto the values stack, so  values = 2, 3, 4

7. `)` is detected, so the `+` is popped out of the ops stack, so ops = `*`, `(`. The 3 and 4 get added together because of the popped `+` and make the values = 2, 7. The top of the ops stack is now `(`, so that gets popped out to make ops = [*].

8. The - is detected, which is lower in rank than the `*` on top of the ops stack, so the `*` gets popped out of the ops stack and is applied to the 2 and 7 to get 14. Now, the values = 14, and ops gets pushed with the `-` to make ops = `-`.

9. 5 gets pushed onto the values stack, so values = 14, 5.

10. The end of the expression/token is reached, so the `-` is popped out of the ops stack and is applied to the 14 and 5, which makes values = 9, so 9 would be returned.

## 4. Errors as values
From what the project specification described, in an agent loop, a tool failing is common, such as the file not existing, the path not being allowed, or the expression dividing by zero. This AI miniharness is supposed to read the error and try something else, so tools return ToolResult::failure("..."), and resolve and calc_evaluate return false plus a message. A returned error is just information being exchanged, while an exception makes the whole AI miniharness stop working. An exception is the right choice for a bug in the C++ code, like calling as_string() on a JSON number. That is a mistake in the program, not something the model can fix.

## 5. What I would design differently
For what I would design differently, I would probably try to see if I can design a more intuitive calc_evaluate function because the methodology and logic behind it are a little bit complicated to learn and explain for a beginner or someone who is new to this project.
