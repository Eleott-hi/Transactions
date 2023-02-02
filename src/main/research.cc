//////////////////////////////////////////
//         created by pintoved          //
//////////////////////////////////////////

#include <random>

#include "b_plus_tree.h"
#include "console.h"
#include "hash_table.h"
#include "self_balancing_binary_search_tree.h"

using namespace s21;

std::vector<std::string> first_names = {
    "Иван",  "Андрей",  "Яков",      "Юрий",   "Татьяна",
    "Мария", "Авдотья", "Елизавета", "Виктор", "Поликарп"};

std::vector<std::string> last_names = {
    "Иванов", "Смирнова", "Кузнецов",  "Попова",       "Васильев",
    "Петров", "Соколов",  "Михайлова", "Мирошниченко", "Усенко"};

std::vector<std::string> cities = {
    "Новосибирск", "Киев", "Владивосток", "Нью-Йорк",  "Лондон",
    "Мадрид",      "Рим",  "Кипр",        "Гонг-Конг", "Москва"};

std::vector<std::string> birth_days = {"1996", "1997", "1998", "1999", "2000",
                                       "2001", "2002", "2003", "2004", "2005"};

std::vector<std::string> coins = {
    "1",      "10",      "100",      "1000",      "10000",
    "100000", "1000000", "10000000", "100000000", "1000000000"};

template <class Type>
Type Random(Type from, Type to) {
  std::random_device rd;
  std::uniform_int_distribution<Type> uniform_dist(from, to);
  return uniform_dist(rd);
}

void Generate(SelfBalancingBinarySearchTree& rb_tree, HashTable& hash_table,
              BPlusTree& b_tree, int count) {
  for (int i = 0; i < count; i++) {
    Person data = {first_names.at(Random(0, 9)),  //
                   last_names.at(Random(0, 9)),   //
                   birth_days.at(Random(0, 9)),   //
                   cities.at(Random(0, 9)),       //
                   coins.at(Random(0, 9))};
    rb_tree.Set("key" + std::to_string(i), data, -1);
    hash_table.Set("key" + std::to_string(i), data, -1);
    b_tree.Set("key" + std::to_string(i), data, -1);
  }
}

template <class Func, class... Args>
nanoseconds Research(int count, Func func, Args... args) {
  auto research_func = std::bind(func, args...);

  Timer timer;
  nanoseconds res = 0s;

  for (int i = 0; i < count; i++) {
    timer.Start();
    research_func();
    res += timer.Finish();
  }

  return count ? res / count : 0s;
}

// ChoseFunc

void PrintTableString(std::string const& name,        //
                      std::string const& rb_tree,     //
                      std::string const& hash_table,  //
                      std::string const& b_tree) {
  std::stringstream stream;
  stream << std::setw(15) << name << " "        //
         << std::setw(15) << rb_tree << " "     //
         << std::setw(15) << hash_table << " "  //
         << std::setw(15) << b_tree;
  Console::WriteLine(stream.str());
}

int main() {
  int num = Console::ReadInt("Number of items in the store: ");
  int count = Console::ReadInt("Number of iterations of one operation: ");

  SelfBalancingBinarySearchTree rb_tree;
  HashTable hash_table(num);
  BPlusTree b_tree;

  Generate(rb_tree, hash_table, b_tree, num);

  PrintTableString("Research", "BinaryTree[ns]", "HashTable[ns]",
                   "BPlusTree[ns]");

  auto rb_time =
      Research(count, [&]() { rb_tree.Set("key_new", {}, 1); }).count();
  auto h_time =
      Research(count, [&]() { hash_table.Set("key_new", {}, -1); }).count();
  auto b_time =
      Research(count, [&]() { b_tree.Set("key_new", {}, -1); }).count();

  PrintTableString("Set",                    //
                   std::to_string(rb_time),  //
                   std::to_string(h_time),   //
                   std::to_string(b_time));

  rb_time = Research(count, [&]() { rb_tree.Get("key_new"); }).count();
  h_time = Research(count, [&]() { hash_table.Get("key_new"); }).count();
  b_time = Research(count, [&]() { b_tree.Get("key_new"); }).count();

  PrintTableString("Get",                    //
                   std::to_string(rb_time),  //
                   std::to_string(h_time),   //
                   std::to_string(b_time));

  rb_time = Research(count, [&]() { rb_tree.Delete("key_new"); }).count();
  h_time = Research(count, [&]() { hash_table.Delete("key_new"); }).count();
  b_time = Research(count, [&]() { b_tree.Delete("key_new"); }).count();

  PrintTableString("Delete",                 //
                   std::to_string(rb_time),  //
                   std::to_string(h_time),   //
                   std::to_string(b_time));

  rb_time = Research(count, [&]() {
              rb_tree.Find({"-", "-", "1996", "-", "-"});
            }).count();
  h_time = Research(count, [&]() {
             hash_table.Find({"-", "-", "1996", "-", "-"});
           }).count();
  b_time = Research(count, [&]() {
             b_tree.Find({"-", "-", "1996", "-", "-"});
           }).count();

  PrintTableString("Find",                   //
                   std::to_string(rb_time),  //
                   std::to_string(h_time),   //
                   std::to_string(b_time));

  rb_time = Research(count, [&]() { rb_tree.ShowAll(); }).count();
  h_time = Research(count, [&]() { hash_table.ShowAll(); }).count();
  b_time = Research(count, [&]() { b_tree.ShowAll(); }).count();

  PrintTableString("ShowAll",                //
                   std::to_string(rb_time),  //
                   std::to_string(h_time),   //
                   std::to_string(b_time));

  return 0;
}
