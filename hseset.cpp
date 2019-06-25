#include <cstdlib>
#include <utility>


// Реализовано с помощью 2-3 дерева с асимптотикой O(log n) на добавление, удаление и поиск
//   источник: http://neerc.ifmo.ru/wiki/index.php?title=2-3_дерево.

// Каждый объект Set отвечает за свою вершину дерева, публично доступен только корень.

// Для удобства реализации и работы в дереве всегда хранится ровно один фиктивный лист, который является 
//   максимальным и последним в списке листов. На него указывает итератор end.
//   (Значение в этом листе не обязательно максимальное, но из-за особеностей реализации это не учитывается.)

template<class ValueType> 
class Set {
public:
    Set(): Set(1) {} // Используется констурктор Set(bool) (его описание есть ниже)

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
        leftLeaf(other.leftLeaf),
        rightLeaf(other.rightLeaf),
        val(other.val),
        sz(other.sz) {}

    const Set& operator = (const Set<ValueType> &other) {
        Set<ValueType> help(other);
        std::swap(father, help.father);
        std::swap(next, help.next);
        std::swap(prev, help.prev);
        std::swap(son, help.son);
        std::swap(leftLeaf, help.leftLeaf);
        std::swap(rightLeaf, help.rightLeaf);
        std::swap(val, help.val);
        std::swap(sz, help.sz);
        update(this);
        return *this;
    }

    const Set& operator = (Set<ValueType> &&other) {
        std::swap(father, other.father);
        std::swap(next, other.next);
        std::swap(prev, other.prev);
        std::swap(son, other.son);
        std::swap(leftLeaf, other.leftLeaf);
        std::swap(rightLeaf, other.rightLeaf);
        std::swap(val, other.val);
        std::swap(sz, other.sz);
        update(this);
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
        // Возвращает количество листов (не считая фиктивного).
        //   Т.е. возвращает число элементов во множестве.
        return sz;
    }

    bool empty() const {
        // Проверяет что есть только один лист (фиктивный).
        //   Т.е. проверяет что нет ни одного элемента во множестве.
        return sz == 0;
    }

    void insert(const ValueType &v) noexcept {
        auto p = go_down(v);
        auto curr = p.first;
        // Проверяет что значение v не лежит в дереве.
        if (p.second == false || (curr->val < v || v < curr->val)) {
            // Добавляет новую вершину.
            auto add = new Set(0);
            add->father = curr->father;
            add->val = v;
            add->sz = 1;
            add->next = curr;
            add->prev = curr->prev;
            curr->prev = add;
            if (add->prev != nullptr) {
                add->prev->next = add;
            } else {
                add->father->son = add;
            }
            // Перестраивает дерево.
            add->go_up();
        }
    }

    void erase(const ValueType &v) noexcept {
        auto p = go_down(v);
        auto curr = p.first;
        // Проверяет что значение v лежит в дереве.
        if (p.second && !(curr->val < v || v < curr->val)) {
            // Удаляет вершину.
            auto curr = p.first;
            if (curr->prev != nullptr) {
                curr->prev->next = curr->next;
            }
            if (curr->next != nullptr) {
                curr->next->prev = curr->prev;
            }
            // Перестраивает дерево.
            if (curr->prev != nullptr) {
                curr->prev->go_up();
            } else {
                curr->father->son = curr->next;
                curr->next->go_up();
            }
            curr->next = nullptr;
            delete curr;
        }
    }

    class iterator {
        // Константный итеретор по дереву, всегда указывает на лист дерева.
    public:
        iterator(): curr(nullptr) {}

        iterator(const Set* s): curr(s) {}

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
        const Set* curr;
    };

    iterator begin() const {
        // Возвращает итератор на минимальный элемент.
        return iterator(leftLeaf);
    }

    iterator end() const {
        // Возвращает итератор на последний (фиктивный) лист.
        return iterator(rightLeaf);
    }

    iterator find(const ValueType &v) const {
        // Выдаёт итератор на элемент со значением v или end, если v нет в множестве.
        auto curr = get_val_leaf(v);
        if (curr->val < v || v < curr->val) {
            return end();
        } else {
            return iterator(curr);
        }
    }

    iterator lower_bound(const ValueType &v) const {
        // Возвращает итератор на первый элемент больше или равный по значение чем v
        //   или end если таких элементов нет.
        return iterator(get_val_leaf(v));
    }

private:

    Set<ValueType>* father; // Указатель на отца в дереве (nullptr если данная вершина - корень).
    Set<ValueType>* next; // Указатель на следующего (правого) брата (сына отца) в дерева (nullptr если такого нет).
    Set<ValueType>* prev; // Указатель на предыдущего (левого) брата (сына отца) в дерева (nullptr если такого нет).
    Set<ValueType>* son; // Указатель на первого слева (с минимальным значением) сына в дереве.
    Set<ValueType>* leftLeaf; // Указатель на самого левого (минимального по значению) листа в поддереве.
    Set<ValueType>* rightLeaf; // Указатель на самого правого (максимального по значению) листа в поддереве.

    ValueType val; // Значение в вершине, если это лист, иначе значение максимального листа в поддереве.

    size_t sz; // Количество листов в поддереве, не считая фиктивный лист.

    Set(const Set<ValueType>* other, Set<ValueType>* newFather, Set* newPrev):
        father(newFather), prev(newPrev), val(other->val), sz(other->sz) {
        // Конструктор копирования other, если уже заинициализирован отец и предыдущий брат.
        // При построении так же инициализирует rightLeaf у father если тот не nullptr.

        // Проверяет что у other есть сын. В этом случае копирует поддерево вершины other в качестве своего поддерева.
        //   В процессе этого инициализирует leftLeaf и при копировании поддерева инициализируется rightLeaf.
        if (other->son == nullptr) {
            son = nullptr;
            leftLeaf = this;
            rightLeaf = this;
        } else {
            son = new Set(other->son, this, nullptr);
            leftLeaf = son->leftLeaf;
        }

        // В случае если у вершины next=nullptr, это значит, что вершина самый правый сын отца. 
        //   Тогда у отца можно заинициализировать rightLeaf как rightLeaf у текущей вершины.
        if (other->next == nullptr) {
            next = nullptr;
            if (newFather != nullptr) {
                newFather->rightLeaf = rightLeaf;
            }
        } else {
            next = new Set(other->next, newFather, this);
        }
    }

    Set(bool hasSon):
        father(nullptr),
        next(nullptr), 
        prev(nullptr),
        val(),
        sz(0) {
        // Конструктор, который если hasSon = 1 создаёт целое дерево с фиктивным листом,
        //   а если hasSon = 0, то создаёт одиночную вершину дерева.
        if (!hasSon) {
            son = nullptr;
            leftLeaf = this;
            rightLeaf = this;
        } else {
            son = new Set(0);
            son->father = this;
            leftLeaf = son;
            rightLeaf = son;
        }
    }

    std::pair<Set*, bool> go_down(const ValueType &v) {
        // Неконстантный метод спуска к листу со значением больше или равным чем v. 
        //   Возвращает пару из указателя на этот лист и булевской переменной, равной true, если лист не фиктивный.
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
        // Константный метод спуска к листу со значением больше или равным v.
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
        // Возвращает число братьев вершины.
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
        // Возвращает самого правого брата.
        auto curr = this;
        while (curr->next != nullptr) {
            curr = curr->next;
        }
        return curr;
    }

    void update(Set<ValueType>* s) {
        // Пересчитывает параметры val и sz у вершины, а так же всем её детям присваивает в отца указатель на себя.
        auto curr = s->son;
        s->sz = 0;
        s->leftLeaf = curr->leftLeaf;
        while (curr->next != nullptr) {
            s->sz += curr->sz;
            curr->father = s;
            curr = curr->next;
        }
        s->rightLeaf = curr->rightLeaf;
        s->sz += curr->sz;
        curr->father = s;
        s->val = curr->val;
    }

    void go_up() {
        // Главный метод 2-3 дерева, перестраивает дерево, чтобы оно оставалось правильным 
        //   и заодно обновляет параметры всех вершин на пути до корня.
        size_t c = brothers_cnt();
        if (c == 1) { // Случай когда у вершины 1 брат.
            if (father == nullptr) { // Случай когда вершина - корень.
                // В этом случае ничего делать не надо.
                return;
            }
            if (father->father == nullptr) { // Случай когда отец - корень.
                if (son != nullptr) { // Случай когда у вершины нет братьев и она не лист.
                    // В этом случае можно сместить корень на эту вершину и удалить старый корень.
                    // Так как там, где используется Set, хранится указатель на корень, то сам корень удалять нельзя.
                    //   Поэтому все параметры текущей вершины перемещаются в старый корень, а сама она удаляется.
                    father->son = son;
                    update(father);
                    son = nullptr;
                    delete this;
                } else { // Случай когда текущая вершина - единственный лист и сын корня.
                    update(father);
                }
                return;
            }
            // В остальный случаях берётся какой-то соседний брат отца.
            Set* fatherL;
            Set* fatherR;
            Set* sonL;
            Set* sonR;
            if (father->next != nullptr) {
                fatherL = father;
                fatherR = father->next;
                sonL = this;
                sonR = fatherR->son;
                sonR->prev = this;
                sonL->next = sonR;
                sonR = sonR->go_right();
            } else {
                fatherL = father->prev;
                fatherR = father;
                sonL = fatherL->son->go_right();
                sonR = this;
                sonL->next = sonR;
                sonR->prev = sonL;
                sonL = fatherL->son;
            }
            // Теперь fatherL или fatherR указывает на отца текущей вершина,
            //   sonL - сын fatherL и sonR - сын fatherR,
            //   списки детей fatherL и fatherR объединяются.
            if (sonL->brothers_cnt() <= 3) { // Случай когда у fatherL и fatherR было суммарно не больше 3 детей.
                // В этом случае fatherR удаляется, и объединённый списки детей становятся списком детей fatherL.
                fatherL->next = fatherR->next;
                if (fatherL->next != nullptr) {
                    fatherL->next->prev = fatherL;
                }
                fatherR->next = nullptr;
                fatherR->son = nullptr;
                delete fatherR;
                update(fatherL);
            } else { // Случай когда у fatherL и fatherR было 4 детей (больше не может быть).
                // В этом случае объединённые списки детей делятся на 2 равные части по 2 ребёнка,
                //   первый становится списком детей fatherL, второй - списком детей fatherR.
                sonL = sonL->next;
                sonR = sonL->next;
                sonL->next = nullptr;
                sonR->prev = nullptr;
                fatherR->son = sonR;
                update(fatherR);
                update(fatherL);
            }
            fatherL->go_up();
        } else if (c > 3) { // Случай когда число братьев больше 3 (т.е. равно 4).
            // fatherL - отец текущей вершины, создаётся его новый правый брат fatherR.
            //   Список детей fatherL делится на 2 части по 2 и подвешивается к fatherL и fatherR.
            auto sonR = go_right();
            sonR = sonR->prev;
            auto sonL = sonR->prev;
            sonR->prev = nullptr;
            sonL->next = nullptr;
            auto fatherL = father;
            auto fatherR = new Set(0);
            fatherR->son = sonR;
            fatherR->father = fatherL->father;
            fatherR->prev = fatherL;
            fatherR->next = fatherL->next;
            fatherL->next = fatherR;
            if (fatherR->next != nullptr) {
                fatherR->next->prev = fatherR;
            }
            update(fatherL);
            update(fatherR);
            fatherL->go_up();
        } else if (c > 1 && father == nullptr) { // Случай когда у корня создался брат.
            // В этом случае создаётся новая вершина, куда копируются все параметры текущей, а текущая вершина 
            //   становится корнем и её дети - новая вершина и её старый брат.
            auto curr = new Set(0);
            std::swap(curr->next, next);
            std::swap(curr->prev, prev);
            std::swap(curr->son, son);
            update(curr);
            curr->father = this;
            curr->next->prev = curr;
            son = curr;
            update(this);
        } else { // Никаких особых случаев, обновляется текущая и перестраивается дерево сверху над ней.
            update(father);
            father->go_up();
        }
    }
};