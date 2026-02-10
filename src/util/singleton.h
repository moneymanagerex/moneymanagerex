/*******************************************************
Copyright (C) 2006-2012 Lisheng Guan (guanlisheng@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************/

#ifndef MM_EX_SINGLETON_H_
#define MM_EX_SINGLETON_H_

template <class T>
class Singleton
{
public:
    /**
    * Returns the address of the static instance created for the class.
    * Creates the instance on first call.
    */
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

#endif // MM_EX_SINGLETON_H_
