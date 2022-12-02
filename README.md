# memleakd
This project is supposed to be a memory leak detector for me, but I've tried to make it more stranger-friendly with configuration macros defined in <code>mhshim.h</code>.
## How to use it?
It's simple. First you need to understand how the memory leak detector works. It's a shim. It edits the functions <code>malloc, calloc, realloc, free</code>.
What are the modifications? It uses a linked list to store void pointers and their sizes. It's not like the original allocation and deallocation functions aren't being used.
They're also being used with the help of <code>dlsym</code>. 
### So what do I need to know in order to use it?
You need to know what build systems are. Some examples are <code>make, cmake, ninja, etc</code>. They automate compilation.
You also need to know what environment variables are. More specifically, what <code>LD_PRELOAD</code> is. After compilation is done, you should see a .so file.
That is a shared object file. You need to set <code>LD_PRELOAD</code> to <code>./(shared object path) ./(target executable)</code>. This will be automated in the future
with the help of functions like <code>setenv, fork, execvp</code>.
## NOTE
This only works on unix-based operating systems.
