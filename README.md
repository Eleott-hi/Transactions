# Transactions

Implementation of in-memory key-value store for storing various financial transaction data with console interface.

## Contents

1. [Chapter I](#chapter-i) \
   1.1. [Tests](#tests) \
   1.2. [Building](#building)
2. [Chapter II](#chapter-ii) \
   2.1. [Information](#information)
3. [Chapter III](#chapter-iii) \
   3.1. [Research](#research)

## Chapter I

## Tests
All tests are written in gtest module. Gtest have to be install in your environment.

To run test enter project directory in the terminal and input `make tests`. After that building and running tests will follow. 
```
> make tests
[==========] Running 57 tests from 3 test suites.
[----------] Global test environment set-up.
[----------] 19 tests from B_Plus_Tree
[ RUN      ] B_Plus_Tree.Set_Correct
...
[       OK ] Hash_Table.Upload (305 ms)
[----------] 19 tests from Hash_Table (712 ms total)

[----------] Global test environment tear-down
[==========] 57 tests from 3 test suites ran. (4942 ms total)
[  PASSED  ] 57 tests.
```

## Building

To build project enter project directory in the terminal and input `make`. It'll start project building in `build` directory and run the program (`build/program.out`) after the building is finished. 
```
> make
Enter mode: [1 - HashTable, 2 - B+ Tree, 3 - RB Tree]
> 
```
If you see the output below - everything is correct and the program works just fine.

You can choose any storage implementation (Hashtable, b+tree, red-black tree) and start to insert your data.


## Chapter II
## Information

### Data structure description

In the key-value store the key will be **strings** and values (data) will be records of persons in the
following form:

* Last name (string)
* First name (string)
* Year of birth (int)
* City (string)
* Number of current coins (int)

### Description of key-value store functions

### Q or QUIT
Enter this comand to quit the program.

### SET

This command is used to set the key and its value. In the example below, the key is the string `foo`, and the value is
the structure described above. The values of the new record fields are entered in the order they are described in the
structure. `EX` is used as an optional parameter to specify the lifetime of the record you are creating. If the optional
field is not specified, the record lifetime is not limited by default.

Description of the `SET` command parameters:

```
SET <key> <Last name> <First name> <Year of birth> <City> <Number of current coins> [EX <time in seconds>]
```

An example of using the `SET` command to create a record with no time limit:

```
SET foo Vasilev Ivan 2000 Moscow 55 
> OK
SET foo Vasilev Ivan 2000 Moscow 55 
[ERROR] - key exists
SET foo Vasilev 123 aaaaa Moscow 55 
[ERROR] - invalid input
```

An example of using the `SET` command to create a record with a time limit. The record will exist for 10 seconds, and
then it will be automatically deleted:

```
SET foo Vasilev Ivan 2000 Moscow 55 EX 10 
> OK
TTL foo
> 8
```

### GET

This command is used to get the value associated with the key. If there is no such record, `(null)` will be returned:

```
GET foo
> Vasilev Ivan 2000  Moscow   55 
GET unknownkey
> (null)
```

### EXISTS

This command checks if a record with the given key exists. It returns `true` if the object exists or `false` if it
doesn't:

```
EXISTS foo
> true
```

### DEL

This command deletes the key and the corresponding value, then returns `true` if the record was successfully deleted,
otherwise `false`:

```
DEL foo
> true
DEL unknownkey
> false
```

### UPDATE

This command updates the value by the corresponding key if such a key exists:

```
SET foo Vas I 20 Mos 5 
> OK
UPDATE foo Vasilev Ivan 2000 Moscow 55 
> OK

GET foo
> Vasilev Ivan 2000 Moscow 55
```

If there is a field that is not planned to change, it is replaced by a dash "-":

```
SET foo Vas I 20 Mos 5 
> OK
UPDATE foo Vasilev - - - 55
> OK

GET foo
> Vasilev I 20 Mos 55 
```

### KEYS

Returns all the keys that are in the store:

```
KEYS
1) boo
2) foo
3) bar
```

### RENAME

This command is used to rename keys:

```
RENAME foo foo2
> OK

GET foo
> (null)

GET foo2
> Vasilev I 20 Mos 55
```

### TTL

When the key is set with the time limit, this command can be used to view the remaining time. If there is no record with
the given key, `(null)` will be returned:

```
SET foo Vasilev Ivan 2000 Moscow 55 EX 10
> OK
TTL foo
> 6
TTL foo
> 5
TTL foo
> 4
TTL foo
> (null)

SET foo Vasilev Ivan 2000 Moscow 55
TTL foo
> (unlimited)
```

### FIND

This command is used to restore the key (or keys) according to a given value. Similarly to the `UPDATE` command, you
don’t have to specify all the values from the structure of the School 21 students. If any fields will not be searched,
it is replaced by a dash "-".

An example of using the `FIND` command to search through all fields of a student structure:

```
FIND Vasilev Ivan 2000 Moscow 55 
> 1) foo
FIND Vasilev Anton 1997 Tver 55
> 1) boo
```

An example of using the `FIND` command to search by last name and number of coins:

```
FIND Vasilev - - - 55
> 1) foo
> 2) boo
```

### SHOWALL

This command is used for getting all records that are in the key-value store at the moment:

```
SHOWALL
> № | Last name |   First name   | Year |  City   | Number of coins |
> 1   "Vasilev"       "Ivan"       2000  "Moscow"         55 
> 2   "Ivanov"       "Vasily"      2000  "Moscow"         55 
```

### UPLOAD

This command is used to upload data from a file. The file contains a list of uploaded data in the format:

```
key1 "Vasilev" "Ivan" 2001 " Rostov" 55
key2 "Ivanov" "Vasiliy" 2000 "Москва" 55 
...
key101 " Sidorov" "Sergei" 1847 "Suzdal" 12312313 
```

Command call:

```
UPLOAD ~/Desktop/TestData/file.dat
> OK 101
```

After the `OK` the number of strings uploaded from the file is displayed.

### EXPORT

This command is used to export the data that are currently in the key-value store to a file. The output of the file must
contain a list of data in the format:

```
key1 "Vasilev" "Ivan" 2001 " Rostov" 55
key2 "Ivanov" "Vasiliy" 2000 "Москва" 55 
...
key101 " Sidorov" "Sergei" 1847 "Suzdal" 12312313 
```

Command call:

```
EXPORT ~/Desktop/TestData/export.dat
> OK 101
```

After the `OK` the number of strings exported from the file is displayed.

## Chapter III

## Research

Research on temporal characteristics of in-memory key-value store implementations

To run research enter project directory in the terminal and input `make research`. It'll start building in `build` directory and run the program (`build/research.out`) after the building is finished. 

```
> make research
Number of items in the store: 
```

If you see the output below - everything is correct and the program works just fine.

- Set the number of items in the store which will be randomly generated
- Set the number of iterations of one operation

The reasearch present the time which certain operations (GET, SET, DEL, SHOWALL, FIND) take for each storage implamantation (BinaryTree(Red-Black Tree), HashTable, BPlusTree).

```
> make research
Number of items in the store: 100
Number of iterations of one operation: 1
       Research  BinaryTree[ns]   HashTable[ns]   BPlusTree[ns]
            Set           25000            1000            1000
            Get            1000            1000            1000
         Delete            2000            2000            2000
           Find           18000           70000           12000
        ShowAll           44000          131000           47000
```
