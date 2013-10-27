/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#include <stack>
#include <utility>

template<typename Type>
class BasicPool
    {
    public:
    BasicPool() {}

    //delete the copy constructor; we can't copy it:
    BasicPool(const BasicPool &)=delete;

    //move constructor; we can move it:
    BasicPool(BasicPool && other)
        {
        this->free(std::move(other.free));
        }

    //#### create an instance of Type:
    template<typename... Args>
    Type * create(Args && ...args)
        {
        Type * place = (Type*)(allocate());
        try{ new (place) Type(std::forward<Args>(args)...); }
        catch(...) { free.push(place); throw; }
        return place;
        }

    //#### remove an instance of Type (add memory to the pool):
    void remove(Type * o)
        {
        o->~Type();
        free.push(o);
        }

    //allocate a chunk of memory as big as Type needs:
    void * allocate()
        {
        void * place;
        if(!free.empty())
            {
            place = static_cast<void*>(free.top());
            free.pop();
            }
        else
            {
            place = operator new(sizeof(Type));
            }
        return place;
        }

    //mark some memory as available (no longer used):
    void deallocate(void * o)
        {
        free.push(static_cast<Type*>(o));
        }

    //delete all of the available memory chunks:
    ~BasicPool()
        {
        while(!free.empty())
            {
            ::operator delete(free.top());
            free.pop();
            }
        }

    private:

    //stack to hold pointers to free chunks:
    std::stack<Type*> free;
};

class Movegen{
	void pippo(void){
		BasicPool<int> p;
	    //create an instance of SomeObject from the pool:
	    int * o = p.create();
	    //destroy it when done, putting memory back into the pool:
	    p.remove(o);


	}
};






