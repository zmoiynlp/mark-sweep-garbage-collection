//
// Created by 贾书瑞 on 2018/06/29.
//

#ifndef GC_GC_H
#define GC_GC_H

#include <cstdint>
#include<functional>
#include<iostream>
#include <vector>
#include <list>
#include<tuple>
#include <utility>
#include <typeindex>
#include <type_traits>
#include <typeinfo>
#include <map>
#include <set>

using std::cout;
using std::vector;
#define dump(x) do{cout<<#x<<"="<<(x)<<"\n";}while(0)

template<typename A, typename B>
std::ostream &operator<<(std::ostream &os, const std::pair<A, B> &x) {
    return cout << '(' << x.first << ", " << x.second << ')';
};

template<typename T>
std::ostream &operator<<(std::ostream &os, const vector<T> &x) {
    cout << '{';
    if (auto s = x.size()) {
        cout << x[0];
        for (decltype(s) i = 1; i < s; ++i) cout << ", " << x[i];
    }
    return cout << '}';
};
namespace {
    class Trigger {
        static bool trigger;

        friend void collect();

    public:
        static bool getTrigger() { return trigger; }
    };

    bool Trigger::trigger = false;

    struct Edge;

    struct Node {
        mutable bool flag = !Trigger::getTrigger();
        Edge *first = nullptr;

        virtual void mark() const =0;

        virtual void *get()=0;

        virtual void sweep()=0;

        virtual bool isFrameBottom()=0;

        virtual const std::type_info &type() const =0;

        virtual ~Node() {}
    };

    std::vector<Node *> gc_roots;
    std::unordered_map<void *, Node *> global_nodes;
    int numEnter;

    inline void mark(const int &) {}

    inline void mark(const unsigned int &) {}

    inline void mark(const short &) {}

    inline void mark(const unsigned short &) {}

    inline void mark(const char &) {}

    inline void mark(const unsigned char &) {}

    inline void mark(const long &) {}

    inline void mark(const unsigned long &) {}

    inline void mark(const long long &) {}

    inline void mark(const unsigned long long &) {}

    inline void mark(const float &) {}

    inline void mark(const double &) {}

    inline void mark(const void *&) {}

    template<typename T>
    inline void mark(T *const &p) {
        if (p == nullptr)
            return;
        auto x = global_nodes.find((void *) p);
        if (x != global_nodes.end()) {
            x->second->mark();
        }
    }

    template<typename T>
    inline void mark(const T &x) {
        x.mark();
    }

    template<typename T, size_t N>
    inline void mark(const T (&a)[N]) {
        for (size_t i = 0; i < N; ++i) mark(a[i]);
    };

    template<typename T1, typename T2>
    inline void mark(const std::pair<T1, T2> &p) {
        mark(p.first);
        mark(p.second);
    };

    template<typename T, typename Alloc>
    inline void mark(const std::vector<T, Alloc> &v) {
        for (auto &x:v) mark(x);
    }

    template<typename T, typename Alloc>
    inline void mark(const std::list<T, Alloc> &v) {
        for (auto &x:v) mark(x);
    }

    template<typename Key, typename T, typename Compare, typename Alloc>
    inline void mark(const std::map<Key, T, Compare, Alloc> &v) {
        for (auto &x:v) mark(x);
    }

    template<typename Key, typename Compare, typename Alloc>
    inline void mark(const std::set<Key, Compare, Alloc> &v) {
        for (auto &x:v) mark(x);
    }

    template<typename T>
    inline void mark(const T *&p, size_t n) {
        for (; n; --n) mark(p[n]);
    }


    struct Edge {
        Node *to;
        Edge *next;

        explicit Edge(Node *to, Edge *next = nullptr) : to(to), next(next) { assert(to); }

        void mark(const Edge &) const {
            ::mark(to);
            ::mark(next);
        }
    };



//    template <typename T> constexpr auto hasMark(T* x)-> decltype(x->mark(),true){ return true;}
//    template <typename T> constexpr auto hasMark(...)->bool{ return false;}

    template<typename T>
    struct NodeImpl : public Node {

        T *obj = nullptr;
        size_t length = 1;

        explicit NodeImpl(T *obj, size_t length = 1)
                : obj(obj), length(length) {}

        friend std::ostream &operator<<(std::ostream &os, const NodeImpl &x) {
            return cout << '(' <<
                        x.flag << ", " <<
                        x.obj << ", " <<
                        x.first << ')';
        };

        void mark() const override {
            if (flag == Trigger::getTrigger())
                return;
            flag = Trigger::getTrigger();
//            dump(flag);
//            cout << "hhhh, "<<"\n";
            ::mark(*obj);
            ::mark(first);
        }

        void *get() override {
            return (void *) obj;
        }

        void sweep() override {
            delete obj;
        }

        const std::type_info &type() const override {
            return typeid(T);
        }

        bool isFrameBottom() override {
            return false;
        }
    };

    template<>
    struct NodeImpl<void> : public Node {

        friend std::ostream &operator<<(std::ostream &os, const NodeImpl &x) {
            return cout << '(' <<
                        x.flag << ", " <<
                        x.first << ')';
        };


        void mark() const override {
            if (flag == Trigger::getTrigger())
                return;
            flag = Trigger::getTrigger();
            ::mark(first);
        }

        void *get() override {
            return nullptr;
        }

        const std::type_info &type() const override {
            return typeid(void);
        }

        void sweep() override {}

        bool isFrameBottom() override {
            return true;
        }
    };


    void collect() {
        dump(gc_roots.size());
        for (auto &x:gc_roots) {
//            dump(x->get());
            x->mark();
        }
        decltype(global_nodes) other;
        int c = 0, k = 0;
        dump(global_nodes.size());
        for (auto &it:global_nodes) {
//            cout<<"["<<k++<<"]="<<it.second->flag<<", "<<Trigger::getTrigger()<<"\n";
            if (it.second->flag != Trigger::getTrigger()) {
                it.second->sweep();
//                cout << "sweep!\n";
                c++;
            } else other.emplace(it);
        }
        if (!c)
            cout << "no object deleted.\n";

        else if (c == 1)
            cout << c << " object deleted.\n";

        else
            cout << c << " objects deleted.\n";

        global_nodes.swap(other);
        Trigger::trigger ^= 1;
    }


    void enter() {
        numEnter++;
        gc_roots.emplace_back(new NodeImpl<void>);
    }

//    template<typename H, typename ...T>
//    void setRoot(H &h, T &...x) {
//        gc_roots.emplace_back(new NodeImpl(&h));
//        if constexpr (sizeof...(T)>0)
//            setRoot(x...);
//    }
    template<typename H, typename ...T>
    void setRoot(const H &h, const T &...x) {
        gc_roots.emplace_back(new NodeImpl(&h));
        if constexpr (sizeof...(T) > 0)
            setRoot(x...);
    }
}

void finish() {
    assert(numEnter--);
    for (; !gc_roots.back()->isFrameBottom(); gc_roots.pop_back()) {
        auto x = gc_roots.back();
        delete x;
    }
    auto x = gc_roots.back();
    delete x;
    gc_roots.pop_back();

}

template<typename T>
T &&finish(T &&x) {
    finish();
    return std::forward<T>(x);
}

#define gc_call(f, ...) (enter(),finish(f(__VA_ARGS__)))
#define gc_do(f, ...) do{enter();f(__VA_ARGS__);finish();}while(0)
//    template <typename F,typename ...A>
//    decltype(auto) gc_call(F &&f,A&&...a){
//        enter();
////        setRoot(&a...);
//        return finish(std::invoke(std::forward<F>(f),std::forward<A>(a)...));
//    };



template<typename T, typename ...A>
T *New(A &&...a) {
    auto res = new T(std::forward<A>(a)...);
//    cout << "New(" << sizeof(T) << "): " << res << "\n";

    auto p = (void *) res;
    global_nodes.emplace(p, new NodeImpl(res));
    return res;
}

//namespace {


//}
//template<typename T>
//T *NewArray(std::size_t size) {
//    cout << "NewArray(" << size << ")\n";
//    return new T[size];
//}

#undef dump

#endif //GC_GC_H
