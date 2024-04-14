#include <fstream>
#include <iostream>
#include <ostream>

using namespace std;

class dllist {
  struct item {
    int value;
    item *prev = NULL;
    item *next = NULL;

    item(int v) { value = v; };
  };

public:
  class iter {
    friend dllist;

    item *cur;
    iter(item *item) { cur = item; }

  public:
    static void swap(iter &, iter &);

    iter &operator++() {
      cur = cur->next;
      return *this;
    };

    iter const operator++(int) {
      iter old = *this;
      cur = cur->next;
      return old;
    }

    iter &operator--() {
      cur = cur->prev;
      return *this;
    };

    iter const operator--(int) {
      iter old = *this;
      cur = cur->prev;
      return old;
    }

    bool operator==(iter const &other) const { return other.cur == cur; }
    bool operator!=(iter const &other) const { return other.cur != cur; }

    bool operator>(iter const &other) const {
      for (item *cur = this->cur->prev; cur; cur = cur->prev)
        if (cur == other.cur)
          return true;

      return false;
    }

    bool operator>=(iter const &other) const {
      return *this == other || *this > other;
    }

    bool operator<(iter const &other) const {
      for (item *cur = this->cur->next; cur; cur = cur->next)
        if (cur == other.cur)
          return true;

      return false;
    }

    bool operator<=(iter const &other) const {
      return *this == other || *this < other;
    }

    int &operator*() const { return cur->value; }
    item *operator->() const { return cur; }
  };

private:
  // идея в том, что свап (и итераторы) всегда
  // работают с элементами, будто они в центре.
  // но если они в начале/конце, возникают проблемы
  // с заменой указателей на них.
  // решение: два поддельных элемента в начале и в конце
  item *prefirst;
  item *postlast;

public:
  dllist();
  ~dllist();

  iter begin() const { return iter(prefirst->next); };
  iter end() const { return iter(postlast); };

  int length() const;

  int &operator[](int);
  friend ostream &operator<<(ostream &, const dllist &);
  friend istream &operator>>(istream &, dllist &);
};

dllist::dllist() {
  // -1337 это волшебное число, чтобы видеть,
  // когда что-то жутко сломалось при доступе.
  prefirst = new item(-1337);
  postlast = new item(-1337);

  prefirst->next = postlast;
  postlast->prev = prefirst;
}

dllist::~dllist() {
  for (auto cur = prefirst->next; cur; cur = cur->next)
    delete cur->prev;

  // удалили все, кроме последнего
  delete postlast;
}

int dllist::length() const {
  unsigned long long len = 0;

  // просто идет от начала до конца, ничего хитрого
  for (int const &v : *this) {
    // v не нужен, но без его использования
    // компилятор жалуется на unused variable
    (void)v; // каст к void = операция бездействия
    len += 1;
  }

  return len;
}

int &dllist::operator[](int n) {
  for (int &v : *this)
    if (n-- == 0)
      return v;

  throw "out of range";
}

ostream &operator<<(ostream &out, const dllist &list) {
  bool first = true;
  for (int const &v : list) {
    if (!first)
      out << " ";

    out << v;
    first = false;
  }

  return out;
}

istream &operator>>(istream &in, dllist &list) {
  int value;
  while (!in.eof() && in >> value) {
    dllist::item *item = new dllist::item(value);

    // сначала подвязываем элемент
    item->prev = list.postlast->prev;
    item->next = list.postlast;

    // затем подвязываем его соседей
    item->prev->next = item;
    item->next->prev = item;
  }

  return in;
}

void dllist::iter::swap(iter &i, iter &j) {
  if (i.cur == j.cur)
    return;

  // меняем next
  item *temp = i->next;
  i->next = j->next;
  j->next = temp;

  i->next->prev = i.cur;
  j->next->prev = j.cur;

  // меняем prev
  temp = i->prev;
  i->prev = j->prev;
  j->prev = temp;

  i->prev->next = i.cur;
  j->prev->next = j.cur;

  // меняем сами указатели
  temp = i.cur;
  i.cur = j.cur;
  j.cur = temp;
}

void quick_sort(dllist::iter beg, dllist::iter end) {
  if (beg >= end--)
    return;

  auto pivot = end;
  auto i = beg;

  for (auto j = beg; j != pivot; ++j)
    if (*j < *pivot) {
      if (i == beg)
        beg = j;

      dllist::iter::swap(i, j);
      i++;
    }

  dllist::iter::swap(i, pivot);

  quick_sort(beg, i);
  quick_sort(++i, ++pivot);
}

void insertion_sort(dllist::iter beg, dllist::iter end) {
  for (dllist::iter i = beg; i != end; i++) {
    dllist::iter j = i;
    while (j != beg && *j < *(--j)) {
      auto oj = j++;
      dllist::iter::swap(j, oj);
      j--;
    }
  }
}

int main() {
  int sort_type;
  dllist list;

  { // читаем список
    ifstream input("input.txt");
    if (!input || !(input >> sort_type) || !(input >> list))
      return 1;

    input.close();
    cout << "got list of len " << list.length() << ": " << list;
  }

  cout << "\n";

  switch (sort_type) {
  case 0:
    cout << "using quick sort\n";
    quick_sort(list.begin(), list.end());
    break;
  case 1:
    cout << "using insertion sort\n";
    insertion_sort(list.begin(), list.end());
    break;
  default:
    cout << "wrong sort type";
    return 1;
  }

  { // выводим список
    ofstream output("output.txt");
    if (!(output << list.length() << " " << list))
      return 1;

    output.close();
    cout << "out list of len " << list.length() << ": " << list;
  }

  return 0;
}
