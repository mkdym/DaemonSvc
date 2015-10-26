#pragma once
#include <boost/noncopyable.hpp>
#include <boost/thread/once.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>


//singleton template class
//thread-safety relies on boost::call_once
//manage instance memory by boost::shared_ptr
//
//usage steps:
//      1. your class T must inherit from Singleton<T>
//      2. Singleton<T> must be a friend class to your class,
//  because Singleton<T> will use your class's ctor to create the single instance.
//  typically, first line of your class declaration is "friend class Singleton<T>;"
//      3. you'd better declare your class's ctor as private,
//  in order to prevent instance creating by any other ways.
//      4. you must declare your class's dtor as public,
//  because shared_ptr will delete the single instance when never needed.
//  (I don't know how to make shared_ptr be a friend class.)
//
//
//example:
//
//class TestClass : public Singleton<TestClass>//step 1
//{
//    //step 2
//    friend class Singleton<TestClass>;
//
//private://step 3
//    TestClass()
//    {
//        cout << "TestClass" << endl;
//    }
//
//public://step 4
//    ~TestClass()
//    {
//        cout << "~TestClass" << endl;
//    }
//
//public:
//    void print()
//    {
//        cout << "print" << endl;
//    }
//};
//
//
//int main()
//{
//    TestClass::get_instance_ref().print();
//    //TestClass a; //illegal
//
//    return 0;
//}
template<typename Type>
class Singleton : public boost::noncopyable
{
public:
    static Type& get_instance_ref()
    {
        boost::call_once(once_, init);
        return *(p_.get());
    }

protected:
    Singleton(){}
    virtual ~Singleton(){}

private:
    static void init()
    {
        p_.reset(new Type());
    }

private:
    typedef boost::shared_ptr<Type> InstancePtr;
    static InstancePtr p_;

    static boost::once_flag once_;
};


template<typename Type>
boost::once_flag Singleton<Type>::once_;

template<typename Type>
typename Singleton<Type>::InstancePtr Singleton<Type>::p_;



