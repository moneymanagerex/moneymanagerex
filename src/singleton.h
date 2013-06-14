/*
    Author: Lisheng Guan (guanlisheng@gmail.com) 
*/

#ifndef _MM_EX_SINGLETON_H_
#define _MM_EX_SINGLETON_H_

template <class T>
class Singleton
{
public:
    static inline T& instance()
    {   
        static T _instance;
        return _instance;
    }   

private:
    Singleton(void);
    ~Singleton(void);
    Singleton(const Singleton<T>&);
    Singleton<T>& operator= (const Singleton<T> &); 
};

#endif // _MM_EX_SINGLETON_H_
