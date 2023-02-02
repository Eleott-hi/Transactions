#include <gtest/gtest.h>

#include "b_plus_tree.h"
#include "hash_table.h"
#include "self_balancing_binary_search_tree.h"

using namespace s21;

std::vector<KeyValueStorage::V> persons = {
    {"LastName0", "FirstName0", "2001", "City0", "0"},
    {"LastName1", "FirstName1", "2002", "City1", "1"},
    {"LastName2", "FirstName2", "2003", "City2", "2"},
    {"LastName3", "FirstName3", "2004", "City3", "3"},
    {"LastName4", "FirstName1", "2005", "City4", "4"},
    {"LastName5", "FirstName2", "2001", "City5", "10"},
    {"LastName6", "FirstName3", "2002", "City6", "11"},
    {"LastName7", "FirstName1", "2003", "City7", "12"},
    {"LastName8", "FirstName2", "2004", "City8", "13"},
    {"LastName9", "FirstName3", "2005", "City9", "14"}};

std::vector<std::pair<KeyValueStorage::K, KeyValueStorage::V>> data = {
    {"foo0", persons[0]}, {"foo1", persons[1]}, {"foo2", persons[2]},
    {"foo3", persons[3]}, {"foo4", persons[4]}, {"foo5", persons[5]},
    {"foo6", persons[6]}, {"foo7", persons[7]}, {"foo8", persons[8]},
    {"foo9", persons[9]}};

void FillStorage(KeyValueStorage *storage) {
  for (auto node : data) {
    storage->Set(node.first, node.second);
  }
}

void TestSetCorrect(KeyValueStorage *storage) {
  bool expected = true;
  bool actual = storage->Set(data[0].first, persons[0]);

  ASSERT_EQ(actual, expected);
}

void TestSetIncorrect(KeyValueStorage *storage) {
  bool expected = false;
  FillStorage(storage);
  bool actual = storage->Set(data[0].first, persons[0]);

  ASSERT_EQ(actual, expected);
}

void TestGetCorrect(KeyValueStorage *storage) {
  auto expected = persons[0];
  FillStorage(storage);
  auto actual = storage->Get(data[0].first);

  ASSERT_EQ(actual, expected);
}

void TestGetIncorrect(KeyValueStorage *storage) {
  auto expected = KeyValueStorage::V{};
  auto actual = storage->Get("foo");

  ASSERT_EQ(actual, expected);
}

void TestExistsTrue(KeyValueStorage *storage) {
  bool expected = true;
  FillStorage(storage);
  bool actual = storage->Exists(data[0].first);

  ASSERT_EQ(actual, expected);
}

void TestExistsFalse(KeyValueStorage *storage) {
  bool expected = false;
  bool actual = storage->Exists(data[0].first);

  ASSERT_EQ(actual, expected);
}

void TestDeleteCorrect(KeyValueStorage *storage) {
  bool expected = true;
  FillStorage(storage);
  storage->Delete(data[0].first);
  bool actual = !storage->Exists(data[0].first);

  ASSERT_EQ(actual, expected);
}

void TestDeleteIncorrect(KeyValueStorage *storage) {
  bool expected = false;
  bool actual = storage->Delete(data[0].first);

  ASSERT_EQ(actual, expected);
}

void TestUpdateTrue(KeyValueStorage *storage) {
  bool expected = true;
  FillStorage(storage);
  storage->Update(data[0].first, persons[1]);
  bool actual = storage->Get(data[0].first) == storage->Get(data[1].first);

  ASSERT_EQ(actual, expected);
}

void TestUpdateFalse(KeyValueStorage *storage) {
  bool expected = false;
  FillStorage(storage);
  bool actual = storage->Update("foo", persons[1]);

  ASSERT_EQ(actual, expected);
}

void TestKeys(KeyValueStorage *storage) {
  auto expected = 10;
  FillStorage(storage);
  auto actual = storage->Keys().size();

  ASSERT_EQ(actual, expected);
}

void TestRenameNormal(KeyValueStorage *storage) {
  bool expected = true;
  storage->Rename(data[1].first, "foo");
  bool actual = storage->Exists("foo");

  ASSERT_EQ(actual, expected);
}

void TestRenameSameKeys(KeyValueStorage *storage) {
  bool expected = true;
  bool actual = storage->Rename(data[2].first, data[2].first);

  ASSERT_EQ(actual, expected);
}

void TestRenameTrue(KeyValueStorage *storage) {
  FillStorage(storage);
  TestRenameNormal(storage);
  TestRenameSameKeys(storage);
}

void TestRenameDoesnotExist(KeyValueStorage *storage) {
  bool expected = false;
  bool actual = storage->Rename("foo", "new_name");

  ASSERT_EQ(actual, expected);
}

void TestRenameSameKeysOneDoesnotExist(KeyValueStorage *storage) {
  bool expected = false;
  bool actual = storage->Rename("foo", "foo");

  ASSERT_EQ(actual, expected);
}

void TestRenameFalse(KeyValueStorage *storage) {
  FillStorage(storage);
  TestRenameDoesnotExist(storage);
  TestRenameSameKeysOneDoesnotExist(storage);
}

void TestTtlCorrect(KeyValueStorage *storage) {
  int expected = 99;
  FillStorage(storage);
  storage->Set("foo", data[0].second, 100);
  int actual = storage->Ttl("foo");

  ASSERT_EQ(actual, expected);
}

void TestTtlIncorrect(KeyValueStorage *storage) {
  int expected = -1;
  FillStorage(storage);
  int actual = storage->Ttl("foo");

  ASSERT_EQ(actual, expected);
}

void TestFindFull(KeyValueStorage *storage) {
  auto expected = data[1].first;
  auto actual = storage->Find(persons[1])[0];

  ASSERT_EQ(actual, expected);
}

void TestFindPartial(KeyValueStorage *storage) {
  auto expected = 3;
  KeyValueStorage::V value = persons[1];
  value.last_name = "-";
  value.birthday = "-";
  value.city = "-";
  value.coins = "-";
  auto actual = storage->Find(value).size();

  ASSERT_EQ(actual, expected);
}

void TestFind(KeyValueStorage *storage) {
  FillStorage(storage);
  TestFindFull(storage);
  TestFindPartial(storage);
}

void TestShowAll(KeyValueStorage *storage) {
  auto expected = 10;
  FillStorage(storage);
  auto actual = storage->ShowAll().size();

  ASSERT_EQ(actual, expected);
}

void TestUpload(KeyValueStorage *storage) {
  int expected = 10;
  int actual = storage->Upload("storage_export.txt");

  ASSERT_EQ(actual, expected);
}

void TestExport(KeyValueStorage *storage) {
  int expected = 10;
  FillStorage(storage);
  int actual = storage->Export("storage_export.txt");

  ASSERT_EQ(actual, expected);
}

// ========= B_PLUS_TREE

TEST(B_Plus_Tree, Set_Correct) {
  BPlusTree storage;
  TestSetCorrect(&storage);
}

TEST(B_Plus_Tree, Set_Incorrect) {
  BPlusTree storage;
  TestSetIncorrect(&storage);
}

TEST(B_Plus_Tree, Get_Correct) {
  BPlusTree storage;
  TestGetCorrect(&storage);
}

TEST(B_Plus_Tree, Get_Incorrect) {
  BPlusTree storage;
  TestGetIncorrect(&storage);
}

TEST(B_Plus_Tree, Exists_True) {
  BPlusTree storage;
  TestExistsTrue(&storage);
}

TEST(B_Plus_Tree, Exists_False) {
  BPlusTree storage;
  TestExistsFalse(&storage);
}

TEST(B_Plus_Tree, Delete_True) {
  BPlusTree storage;
  TestDeleteCorrect(&storage);
}

TEST(B_Plus_Tree, Delete_False) {
  BPlusTree storage;
  TestDeleteIncorrect(&storage);
}

TEST(B_Plus_Tree, Update_True) {
  BPlusTree storage;
  TestUpdateTrue(&storage);
}

TEST(B_Plus_Tree, Update_False) {
  BPlusTree storage;
  TestUpdateFalse(&storage);
}

TEST(B_Plus_Tree, Keys) {
  BPlusTree storage;
  TestKeys(&storage);
}

TEST(B_Plus_Tree, Rename_True) {
  BPlusTree storage;
  TestRenameTrue(&storage);
}

TEST(B_Plus_Tree, Rename_False) {
  BPlusTree storage;
  TestRenameFalse(&storage);
}

TEST(B_Plus_Tree, TTL_Correct) {
  BPlusTree storage;
  TestTtlCorrect(&storage);
}

TEST(B_Plus_Tree, TTL_Incorrect) {
  BPlusTree storage;
  TestTtlIncorrect(&storage);
}

TEST(B_Plus_Tree, Find) {
  BPlusTree storage;
  TestFind(&storage);
}

TEST(B_Plus_Tree, ShowAll) {
  BPlusTree storage;
  TestShowAll(&storage);
}

TEST(B_Plus_Tree, Export) {
  BPlusTree storage;
  TestExport(&storage);
}

TEST(B_Plus_Tree, Upload) {
  BPlusTree storage;
  TestUpload(&storage);
}

// ========= RED_BLACK_TREE

TEST(Self_Balancing_Binary_Search_Tree, Set_Correct) {
  SelfBalancingBinarySearchTree storage;
  TestSetCorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Set_Incorrect) {
  SelfBalancingBinarySearchTree storage;
  TestSetIncorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Get_Correct) {
  SelfBalancingBinarySearchTree storage;
  TestGetCorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Get_Incorrect) {
  SelfBalancingBinarySearchTree storage;
  TestGetIncorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Exists_True) {
  SelfBalancingBinarySearchTree storage;
  TestExistsTrue(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Exists_False) {
  SelfBalancingBinarySearchTree storage;
  TestExistsFalse(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Delete_True) {
  SelfBalancingBinarySearchTree storage;
  TestDeleteCorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Delete_False) {
  SelfBalancingBinarySearchTree storage;
  TestDeleteIncorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Update_True) {
  SelfBalancingBinarySearchTree storage;
  TestUpdateTrue(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Update_False) {
  SelfBalancingBinarySearchTree storage;
  TestUpdateFalse(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Keys) {
  SelfBalancingBinarySearchTree storage;
  TestKeys(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Rename_True) {
  SelfBalancingBinarySearchTree storage;
  TestRenameTrue(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Rename_False) {
  SelfBalancingBinarySearchTree storage;
  TestRenameFalse(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, TTL_Correct) {
  SelfBalancingBinarySearchTree storage;
  TestTtlCorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, TTL_Incorrect) {
  SelfBalancingBinarySearchTree storage;
  TestTtlIncorrect(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Find) {
  SelfBalancingBinarySearchTree storage;
  TestFind(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, ShowAll) {
  SelfBalancingBinarySearchTree storage;
  TestShowAll(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Export) {
  SelfBalancingBinarySearchTree storage;
  TestExport(&storage);
}

TEST(Self_Balancing_Binary_Search_Tree, Upload) {
  SelfBalancingBinarySearchTree storage;
  TestUpload(&storage);
}

// ========= HASH_TABLE

TEST(Hash_Table, Set_Correct) {
  HashTable storage(10);
  TestSetCorrect(&storage);
}

TEST(Hash_Table, Set_Incorrect) {
  HashTable storage(10);
  TestSetIncorrect(&storage);
}

TEST(Hash_Table, Get_Correct) {
  HashTable storage(10);
  TestGetCorrect(&storage);
}

TEST(Hash_Table, Get_Incorrect) {
  HashTable storage(10);
  TestGetIncorrect(&storage);
}

TEST(Hash_Table, Exists_True) {
  HashTable storage(10);
  TestExistsTrue(&storage);
}

TEST(Hash_Table, Exists_False) {
  HashTable storage(10);
  TestExistsFalse(&storage);
}

TEST(Hash_Table, Delete_True) {
  HashTable storage(10);
  TestDeleteCorrect(&storage);
}

TEST(Hash_Table, Delete_False) {
  HashTable storage(10);
  TestDeleteIncorrect(&storage);
}

TEST(Hash_Table, Update_True) {
  HashTable storage(10);
  TestUpdateTrue(&storage);
}

TEST(Hash_Table, Update_False) {
  HashTable storage(10);
  TestUpdateFalse(&storage);
}

TEST(Hash_Table, Keys) {
  HashTable storage(10);
  TestKeys(&storage);
}

TEST(Hash_Table, Rename_True) {
  HashTable storage(10);
  TestRenameTrue(&storage);
}

TEST(Hash_Table, Rename_False) {
  HashTable storage(10);
  TestRenameFalse(&storage);
}

TEST(Hash_Table, TTL_Correct) {
  HashTable storage(10);
  TestTtlCorrect(&storage);
}

TEST(Hash_Table, TTL_Incorrect) {
  HashTable storage(10);
  TestTtlIncorrect(&storage);
}

TEST(Hash_Table, Find) {
  HashTable storage(10);
  TestFind(&storage);
}

TEST(Hash_Table, ShowAll) {
  HashTable storage(10);
  TestShowAll(&storage);
}

TEST(Hash_Table, Export) {
  HashTable storage(10);
  TestExport(&storage);
}

TEST(Hash_Table, Upload) {
  HashTable storage(10);
  TestUpload(&storage);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
