#include <cstdlib>
#include <utility>
template<class ValueType> 
class Set {

public:
    Set(): Set(1) {}

    template<typename Iter>
    Set(Iter begin_it, Iter end_it): Set(1) {
        while (begin_it != end_it) {
            insert(*begin_it);
            begin_it++;
        }
    }


    Set(std::initializer_list<ValueType> lst): Set(lst.begin(), lst.end()) {}

    Set(const Set<ValueType> &other): Set(&other, nullptr, nullptr) {}

    Set(Set<ValueType> &&other):
        father(other.father),
        next(other.next),
        prev(other.prev),
        son(other.son),
        val(other.val),
        sz(other.sz) {}

    const Set& operator = (const Set<ValueType> &other) {
        Set<ValueType> help(other);
        std::swap(father, help.father);
        std::swap(next, help.next);
        std::swap(prev, help.prev);
        std::swap(son, help.son);
        std::swap(val, help.val);
        std::swap(sz, help.sz);
        upd(this);
        return *this;
    }

    const Set& operator = (Set<ValueType> &&other) {
        std::swap(father, other.father);
        std::swap(next, other.next);
        std::swap(prev, other.prev);
        std::swap(son, other.son);
        std::swap(val, other.val);
        std::swap(sz, other.sz);
        upd(this);
        return *this;
    }

    ~Set() {
        if (next != nullptr) {
            delete next;
        }
        if (son != nullptr) {
            delete son;
        }
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    void insert(const ValueType &v) noexcept {
        auto p = go_down(v);
        auto curr = p.first;
        if (p.second == false || (curr->val < v || v < curr->val)) {
            auto add = new Set(0);
            add->father = curr->father;
            add->val = v;
            add->sz = 1;
            add->next = curr;
            add->prev = curr->prev;
            curr->prev = add;
            if (add->prev != nullptr) {
                add->prev->next = add;
            }
            else {
                add->father->son = add;
            }
            add->go_up();
        }
    }

    void erase(const ValueType &v) noexcept {
        auto p = go_down(v);
        auto curr = p.first;
        if (p.second && !(curr->val < v || v < curr->val)) {
            auto curr = p.first;
            if (curr->prev != nullptr) {
                curr->prev->next = curr->next;
            }
            if (curr->next != nullptr) {
                curr->next->prev = curr->prev;
            }
            if (curr->prev != nullptr) {
                curr->prev->go_up();
            }
            else {
                curr->father->son = curr->next;
                curr->next->go_up();
            }
            curr->next = nullptr;
            delete curr;
        }
    }

    class iterator {
    public:
        iterator(): curr(nullptr) {}

        iterator(const Set *s): curr(s) {}

        iterator(const Set<ValueType>::iterator &x): curr(x.curr) {}

        iterator& operator = (const Set<ValueType>::iterator x) {
            curr = x.curr;
            return *this;
        }

        iterator& operator++() {
            while (curr->next == nullptr) {
                curr = curr->father;
            }
            curr = curr->next;
            while (curr->son != nullptr) {
                curr = curr->son;
            }
            return *this;
        }

        iterator operator++(int) {
            auto it = *this;
            ++(*this);
            return it;
        }

        iterator& operator--() {
            while (curr->prev == nullptr) {
                curr = curr->father;
            }
            curr = curr->prev;
            while (curr->son != nullptr) {
                curr = curr->son->go_right();
            }
            return *this;
        }

        iterator operator--(int) {
            auto it = *this;
            --(*this);
            return it;
        }

        const ValueType& operator *() const {
            return curr->val;
        }

        const ValueType* operator ->() const {
            return &(curr->val);
        }

        bool operator == (const Set<ValueType>::iterator &it) {
            return it.curr == curr;
        }

        bool operator != (const Set<ValueType>::iterator &it) {
            return it.curr != curr;
        }

    private:
        const Set *curr;
    };

    iterator begin() const {
        auto curr = this;
        while (curr->son != nullptr) {
            curr = curr->son;
        }
        return iterator(curr);
    }

    iterator end() const {
        auto curr = this;
        while (curr->son != nullptr || curr->next != nullptr) {
            if (curr->next != nullptr) {
                curr = curr->next;
            }
            else {
                curr = curr->son;
            }
        }
        return iterator(curr);
    }

    iterator find(const ValueType &v) const {
        auto curr = get_val_leaf(v);
        if (curr->val < v || v < curr->val) {
            return end();
        }
        else {
            return iterator(curr);
        }
    }

    iterator lower_bound(const ValueType &v) const {
        return iterator(get_val_leaf(v));
    }

private:
    Set<ValueType> *father, *next, *prev, *son;

    ValueType val;

    size_t sz;

    Set(const Set<ValueType> *other, Set<ValueType> *new_father, Set *new_prev):
        father(new_father), prev(new_prev), val(other->val), sz(other->sz) {
        if (other->next == nullptr) {
            next = nullptr;
        }
        else {
            next = new Set(other->next, new_father, this);
        }
        if (other->son == nullptr) {
            son = nullptr;
        }
        else {
            son = new Set(other->son, this, nullptr);
        }
    }

    Set(bool hasson):
        father(nullptr),
        next(nullptr), 
        prev(nullptr),
        val(),
        sz(0) {
        if (!hasson) {
            son = nullptr;
        }
        else {
            son = new Set(0);
            son->father = this;
        }
    }

    std::pair<Set*, bool> go_down(const ValueType &v) {
        if (son == nullptr) {
            return std::make_pair(this, false);
        }
        auto curr = son;
        while (curr->next != nullptr && curr->val < v) {
            curr = curr->next;
        }
        auto p = curr->go_down(v);
        p.second |= curr->next != nullptr;
        return p;
    }

    const Set* get_val_leaf(const ValueType &v) const {
        if (son == nullptr) {
            return this;
        }
        auto curr = son;
        while (curr->next != nullptr && curr->val < v) {
            curr = curr->next;
        }
        return curr->get_val_leaf(v);
    }

    size_t brothers_cnt() const {
        auto curr = this;
        size_t cnt = 1;
        while (curr->prev != nullptr) {
            curr = curr->prev;
            cnt++;
        }
        curr = this;
        while (curr->next != nullptr) {
            curr = curr->next;
            cnt++;
        }
        return cnt;
    }

    Set* go_right() {
        auto curr = this;
        while (curr->next != nullptr) {
            curr = curr->next;
        }
        return curr;
    }

    void upd(Set<ValueType> *s) {
        auto curr = s->son;
        s->sz = 0;
        while (curr->next != nullptr) {
            s->sz += curr->sz;
            curr->father = s;
            curr = curr->next;
        }
        s->sz += curr->sz;
        curr->father = s;
        s->val = curr->val;
    }

    void go_up() {
        size_t c = brothers_cnt();
        if (c == 1) {
            if (father == nullptr) {
                return;
            }
            if (father->father == nullptr) {
                if (son != nullptr) {
                    father->son = son;
                    upd(father);
                    son = nullptr;
                    delete this;
                }
                else {
                    upd(father);
                }
                return;
            }
            Set* father1;
            Set* father2;
            Set* son1;
            Set* son2;
            if (father->next != nullptr) {
                father1 = father;
                father2 = father->next;
                son1 = this;
                son2 = father2->son;
                son2->prev = this;
                son1->next = son2;
                son2 = son2->go_right();
            }
            else {
                father1 = father->prev;
                father2 = father;
                son1 = father1->son->go_right();
                son2 = this;
                son1->next = son2;
                son2->prev = son1;
                son1 = father1->son;
            }
            if (son1->brothers_cnt() <= 3) {
                father1->next = father2->next;
                if (father1->next != nullptr) {
                    father1->next->prev = father1;
                }
                father2->next = nullptr;
                father2->son = nullptr;
                delete father2;
                upd(father1);
            }
            else {
                son1 = son1->next;
                son2 = son1->next;
                son1->next = nullptr;
                son2->prev = nullptr;
                father2->son = son2;
                upd(father2);
                upd(father1);
            }
            father1->go_up();
        }
        else if (c > 3) {
            auto son2 = go_right();
            son2 = son2->prev;
            auto son1 = son2->prev;
            son2->prev = nullptr;
            son1->next = nullptr;
            auto father1 = father;
            auto father2 = new Set(0);
            father2->son = son2;
            father2->father = father1->father;
            father2->prev = father1;
            father2->next = father1->next;
            father1->next = father2;
            if (father2->next != nullptr) {
                father2->next->prev = father2;
            }
            upd(father1);
            upd(father2);
            father1->go_up();
        }
        else if (c > 1 && father == nullptr) {
            auto curr = new Set(0);
            std::swap(curr->next, next);
            std::swap(curr->prev, prev);
            std::swap(curr->son, son);
            upd(curr);
            curr->father = this;
            curr->next->prev = curr;
            son = curr;
            upd(this);
        }
        else {
            upd(father);
            father->go_up();
        }
    }
};