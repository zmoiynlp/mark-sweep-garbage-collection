# mark-sweep-garbage-collection
a C++ library for mark-sweep gc.

The gc library is in gc.h.   
please compile with ```-std=c++17```.  
Usage below.   
## For functions

* type ```enter();``` at the beginning of function.  
* do not forget initialize local pointers to ```nullptr```.
* call ```setRoot``` for each variable you want gc to monitor.   
for example ``` Human a,*b(nullptr);setRoot(a,b);``` where Human may have some pointer members.
you do not have to use ```enter()``` and ```finish()``` with general coumpound statements, though it may help reduce memory space.
* replace ```new T(a...)``` with ``` New<T>(a...)```.   

balabala...  

* append ```finish();``` to the end of a function returning nothing;   
or surround the return value with ```finish```, 
like ``` return x;``` to ``` return finish(x);```.  
 an ```enter()``` cannot live alone without a ```finish()```.   
 
* In fact there is a macro ```gc_call``` for non-void functions and ```gc_do``` for those returning something.   
For example ```gc_call(sum, 1, 2, 3)``` and ```gc_do(ReturnNothing, x, y)```.
 
 ## For classes
 
 implement a public method called ```mark``` though which gc would mark reachable objects.   
 for example:
 
 ```
 struct Node{
     Node *next;
     Node *prev;
     void mark()const{
         ::mark(next);
         ::mark(prev);
     }
 };
 ```
 
 ## Demo
 There is a demo about how gc works.

 
