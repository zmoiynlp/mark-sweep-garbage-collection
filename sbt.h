//
// Created by 贾书瑞 on 2018/06/14.
//

#ifndef UNTITLED_SBT_H
#define UNTITLED_SBT_H

#include <array>
#include <utility>
#include <iostream>
#include <vector>
#include <functional>
#include <experimental/optional>
#include "gc.h"

#define dump(x) do{std::cout<<#x<<"="<<(x)<<"\n";}while(0)

template<typename T>
struct Node {
    static Node<T> *const null;
    T *ptr;
    int size = 1;
    Node<T> *ch[2] = {null, null};

    explicit Node(const T &obj) : ptr(New<T>(obj)) {}

    explicit Node(T &&obj) : ptr(New<T>(std::move(obj))) {}

    Node() : ptr(nullptr), size(0) {
        //std::cout << "Node()\n";
        ch[0] = ch[1] = this;
    }

    ~Node() = default;

    void mark() const {
        ::mark(ptr);
        ::mark(ch);
    }
};

template<typename T> Node<T> *const Node<T>::null = New<Node<T>>();


template<typename T>
class SBT {


private:
    static const int L = 0, R = 1;
    using ConstNodeConstPtr=const Node<T> *const;
    using NodePtrRef = Node<T> *&;
    using NodePtr = Node<T> *;
    using ConstTRef = const T &;
    using TPtr = T *;
    using TRef = T &;
    NodePtr root;

    template<int c, typename = typename std::enable_if<c == 0 || c == 1>::type>
    void rot(NodePtrRef x) noexcept {
        setRoot(x);
        auto b = x->ch[!c];
        setRoot(b);
        x->ch[!c] = b->ch[c];
        b->ch[c] = x;
        b->size = x->size;
        x->size = x->ch[0]->size + x->ch[1]->size + 1;
        x = b;
    }

    template<int f, typename = typename std::enable_if<f == 0 || f == 1>::type>
    void maintain(NodePtrRef t) noexcept {
        setRoot(t);
        if (t->ch[f]->ch[f]->size > t->ch[!f]->size)
            rot<!f>(t);

        if (t->ch[f]->ch[!f]->size > t->ch[!f]->size)
            rot<f>(t->ch[f]), rot<!f>(t);

        else return;
        gc_do(maintain<0>, t->ch[L]);
        gc_do(maintain<1>, t->ch[R]);
        gc_do(maintain<1>, t);
        gc_do(maintain<0>, t);

    }

    void insert(NodePtrRef t, ConstTRef key) {
        setRoot(t, key);
        if (t == Node<T>::null) {
            //pool.push_back(std::forward<T>(key));
            t = New<Node<T>>(key);
            return;
        }
        t->size++;
        gc_do(insert, t->ch[!(key < *t->ptr)], key);

        if (key < *t->ptr)
            gc_do(maintain<0>, t);

        else
            gc_do(maintain<1>, t);
    }

    TPtr remove(NodePtrRef t, ConstTRef key) {
        assert(t != Node<T>::null);
        setRoot(t, key);
        t->size--;
        const bool lt = key < *t->ptr;
        const bool gt = *t->ptr < key;

        if (!(lt or gt) or (lt && t->ch[L] == Node<T>::null) or (gt && t->ch[R] == Node<T>::null)) {
            auto val = t->ptr;
            setRoot(val);
            if (t->ch[L] == Node<T>::null or t->ch[R] == Node<T>::null) {
                if (t->ch[L] == Node<T>::null) {
                    auto temp = t->ch[R];
                    setRoot(temp);
                    t = temp;
                } else {
                    auto temp = t->ch[L];
                    setRoot(temp);
                    t = temp;
                }
            } else t->ptr = gc_call(remove, t->ch[L], key);
            return val;

        } else {
            return gc_call(remove, t->ch[gt], key);
        }
    }

    TPtr select(ConstNodeConstPtr t, const int k) const noexcept {
        setRoot(t);
        if (k == t->ch[L]->size + 1) return t->ptr;
        if (k <= t->ch[L]->size)return gc_call(select, t->ch[L], k);
        return gc_call(select, t->ch[R], k - 1 - t->ch[L]->size);
    }

    int rank(ConstNodeConstPtr t, ConstTRef key) const {
        setRoot(t, key);
        if (t == Node<T>::null) return 1;
        if (*t->ptr < key)
            return t->ch[L]->size + 1 + gc_call(rank, t->ch[R], key);
        return gc_call(rank, t->ch[L], key);
    }

    ConstNodeConstPtr pred(ConstNodeConstPtr x, ConstNodeConstPtr f, ConstTRef key) const noexcept {
        setRoot(x, f, key);
        if (x == Node<T>::null) return f;
        if (key > *x->ptr)
            return gc_call(pred, x->ch[R], x, key);
        else return gc_call(pred, x->ch[L], f, key);
    }

    ConstNodeConstPtr succ(ConstNodeConstPtr x, ConstNodeConstPtr f, ConstTRef key) const noexcept {
        setRoot(x, f, key);
        if (x == Node<T>::null) return f;
        if (key < *x->ptr)
            return gc_call(succ, x->ch[L], x, key);
        else return gc_call(succ, x->ch[R], f, key);

    }

    void inorder(ConstNodeConstPtr x, const std::function<bool(ConstTRef)> &callback, bool &terminated) const {
        setRoot(x);
        if (x == Node<T>::null || terminated)
            return;
        gc_do(inorder, x->ch[L], callback, terminated);
        if (!terminated)
            terminated = not callback(*x->ptr);
        gc_do(inorder, x->ch[R], callback, terminated);
//        finish();
    }

public:
    SBT() : root(Node<T>::null) {

    }

    ~SBT() {}

    SBT <T> &insert(ConstTRef key) {
//        enter();
//        setRoot(key);
//        T k{key};
        gc_do(insert, root, key);
        return *this;
    }


    TPtr remove(ConstTRef key) {
        return gc_call(remove, root, key);
    }

    TRef select(const int k) const {
        if (k <= 0 or k > this->size())
            throw std::invalid_argument("k must be between 1 and size()!");
        return *gc_call(select, root, k);
    }

    int rank(ConstTRef key) const {
        return gc_call(rank, root, key);
    }

    TRef pred(ConstTRef key) {
        return *gc_call(pred, root, root, key)->ptr;
    }

    TRef succ(ConstTRef key) {
        return *gc_call(succ, root, root, key)->ptr;
    }

    int size() const {
        return root->size;
    }

    bool empty() const {
        return root->size == 0;
    }

    void inorder(const std::function<bool(ConstTRef)> &callback) const {
        bool terminated = false;
        gc_do(inorder, root, callback, terminated);
    }

    void enumerate(const std::function<bool(ConstTRef, int)> &callback, int begin = 0) {
        gc_do(inorder, [&](ConstTRef x) {
            return callback(x, begin++);
        });
    }

    void mark() const {
        ::mark(root);
    }

    friend std::ostream &operator<<(std::ostream &os, const SBT <T> &t) {
        os << "sbt{";
        bool comma = false;
        t.inorder([&](auto key) {
            if (comma) {
                os << ", ";
            } else comma = true;
            os << key;
            return true;
        });
        os << '}';
        return os;
    }
};


//template <typename T> constexpr typename SBT<T>::Node SBT<T>::Node::null=&SBT<T>::nullNode;
#undef dump
#endif //UNTITLED_SBT_H
