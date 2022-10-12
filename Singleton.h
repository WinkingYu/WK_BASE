#pragma once

template <typename T>
class Singleton
{
public:
    static T* Instance()
    {
        static T* t = NULL;
        if (t == NULL)
            t = new T();
        return t;
    }

protected:
    Singleton() {}
    Singleton(const Singleton<T>&);
    Singleton<T>& operator = (const Singleton<T>&);
    ~Singleton() {}
};
