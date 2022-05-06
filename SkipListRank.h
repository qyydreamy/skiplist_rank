#pragma once

#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <map>
#include "Node.h"


#define STORE_FILE "store/dumpFile"

std::mutex mtx;     // mutex for critical section
std::string delimiter = ":";


// Class template for Skip list
template <typename K, typename V>
class SkipList {

public:
	SkipList(int);
	~SkipList();
	int get_random_level();
	Node<K, V>* create_node(K, V, int);
	int insert_element(K, V);
	void display_list();
	bool search_element(K, V);
	void delete_element(K, V);
	int get_rank(K, V);
	void dump_file();
	void load_file();
	int size();
	void cal_max_level(int);

private:
	void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
	bool is_valid_string(const std::string& str);

private:
	// Maximum level of the skip list 
	int _max_level;

	// current level of skip list 
	int _skip_list_level;

	// pointer to header node 
	Node<K, V>* _header;

	// file operator
	std::ofstream _file_writer;
	std::ifstream _file_reader;

	// skiplist current element count
	int _element_count;

	// skip list rank
	std::map<long long, int> _rank;
};



