//#include "pch.h"

#include "SkipListRank.h"

#include <string>
#include <math.h>
#include <algorithm>
#include <chrono>

void test_construct_list(SkipList<int, long long>& skiplist, int element_count)
{
	std::map<long long, int> insert_data;
	long long guid = 0;
	int score = 0;
	for (int i = 0; i < element_count; ++i)
	{
		long long r = rand();
		long long r_g = r % 2147483647;
		guid = (r_g <<32) | i;
		score = r % 10000;
		insert_data.insert(std::make_pair(guid, score));
	}

	//test
	auto cstart = std::chrono::steady_clock::now();
	for (std::map<long long, int>::iterator it = insert_data.begin();
		it != insert_data.end(); ++it)
	{
		skiplist.insert_element(it->second, it->first);
	}
	auto cend = std::chrono::steady_clock::now();
	auto cdiff = cend - cstart;
	std::cout << "element_count: " << element_count
		<< "time: " << std::chrono::duration<double, std::milli>(cdiff).count() << std::endl;
}

void test_insert(SkipList<int, long long>& skiplist, int score, long long guid)
{
	skiplist.insert_element(score, guid);
}

void test_delete(SkipList<int, long long>& skiplist, int score, long long guid)
{
	skiplist.delete_element(score, guid);
}

void test_search(SkipList<int, long long>& skiplist, int score, long long guid)
{
	skiplist.search_element(score, guid);
}

int test_get_rank(SkipList<int, long long>& skiplist, int score, long long guid)
{
	return skiplist.get_rank(score, guid);
}

int main()
{
	//input
//	int test_type = 0;
//	std::cin >> test_type;
	int element_count = 1000;
	std::cin >> element_count;

	SkipList<int, long long> skipList(element_count);
	skipList.cal_max_level(element_count);
	test_construct_list(skipList, element_count);
	
//	test_insert(skipList, 4095, 139534291922069263);
//	test_delete(skipList, 4095, 139534291922069263);
//	test_search(skipList, 4095, 139534291922069263);
//	test_get_rank(skipList, 4095, 139534291922069263);

/*
	auto start = std::chrono::steady_clock::now();
	if (test_type == 1)
	{
		test_insert(skipList, 4095, 139534291922069263);
	}
	else if (test_type == 2)
	{
		test_insert(skipList, 4095, 139534291922069263);
		test_delete(skipList, 4095, 139534291922069263);
	}
	else if (test_type == 3)
	{
		test_insert(skipList, 4095, 139534291922069263);
		test_get_rank(skipList, 4095, 139534291922069263);
	}
	else
	{
		return 0;
	}
	auto end = std::chrono::steady_clock::now();
	auto diff = end - start;
	std::cout << "steady_clock: " << std::chrono::duration<double, std::milli>(diff).count() << std::endl;

	return 1;
*/
/*
	SkipList<int, long long> skipList(16);
	int element_count = 100;
	skipList.cal_max_level(element_count);
	skipList.insert_element(3, 1);
	skipList.insert_element(11, 2);
	skipList.insert_element(28, 3);
	skipList.insert_element(56, 4);
	skipList.insert_element(66, 5);
	skipList.insert_element(81, 6);
	skipList.insert_element(90, 7);
	int rank = test_get_rank(skipList, 3, 1);
	std::cout << "rank: " << rank << std::endl;

	return 1;
	*/
}

// create new node 
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
	Node<K, V>* n = new Node<K, V>(k, v, level);
	return n;
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {

	mtx.lock();
	Node<K, V>* current = this->_header;

	// create update array and initialize it 
	// update is array which put node that the node->forward[i] should be operated later
	Node<K, V>** update = new Node<K, V>*[_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));

	int* rank = new int[_max_level + 1];
	memset(rank, 0, sizeof(int)*(_max_level + 1));

	// start form highest level of skip list 
	for (int i = _skip_list_level; i >= 0; i--) {
		rank[i] = i == (_skip_list_level) ? 0 : rank[i + 1];
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			rank[i] += current->span;
			current = current->forward[i];
		}
		update[i] = current;
	}

	// reached level 0 and forward pointer to right node, which is desired to insert key.
	current = current->forward[0];

	// if current node have key equal to searched key, we get it
	if (current != NULL && current->get_key() == key && current->get_value() == value) {
		std::cout << "key: " << key << ", exists" << std::endl;
		mtx.unlock();
		return 1;
	}

	// if current is NULL that means we have reached to end of the level 
	// if current's key is not equal to key that means we have to insert node between update[0] and current node 
	if (current == NULL || (current->get_key() != key && current->get_value() != value)) {

		// Generate a random level for node
		int random_level = get_random_level();

		// If random level is greater thar skip list's current level, initialize update value with pointer to header
		if (random_level > _skip_list_level) {
			for (int i = _skip_list_level + 1; i < random_level + 1; i++) {
				update[i] = _header;
			}
			_skip_list_level = random_level;
		}

		// create new node with random level generated 
		Node<K, V>* inserted_node = create_node(key, value, random_level);

		// insert node 
		for (int i = 0; i <= random_level; i++) {
			inserted_node->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = inserted_node;
			inserted_node->span = update[i]->span - (rank[0] - rank[i]);
			update[i]->span = (rank[0] - rank[i]) + 1;
		}

		for (int i = random_level; i < _skip_list_level; i++) {
			update[i]->span++;
		}

//		std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
		_element_count++;
	}
	mtx.unlock();
	return 0;
}

// Display skip list 
template<typename K, typename V>
void SkipList<K, V>::display_list() {

	std::cout << "\n*****Skip List*****" << "\n";
	for (int i = 0; i <= _skip_list_level; i++) {
		Node<K, V>* node = this->_header->forward[i];
		std::cout << "Level " << i << ": ";
		while (node != NULL) {
			std::cout << node->get_key() << ":" << node->get_value() << ";";
			node = node->forward[i];
		}
		std::cout << std::endl;
	}
}

// Dump data in memory to file 
template<typename K, typename V>
void SkipList<K, V>::dump_file() {

	std::cout << "dump_file-----------------" << std::endl;
	_file_writer.open(STORE_FILE);
	Node<K, V>* node = this->_header->forward[0];

	while (node != NULL) {
		_file_writer << node->get_key() << ":" << node->get_value() << "\n";
		std::cout << node->get_key() << ":" << node->get_value() << ";\n";
		node = node->forward[0];
	}

	_file_writer.flush();
	_file_writer.close();
	return;
}

// Load data from disk
template<typename K, typename V>
void SkipList<K, V>::load_file() {

	_file_reader.open(STORE_FILE);
	std::cout << "load_file-----------------" << std::endl;
	std::string line;
	std::string* key = new std::string();
	std::string* value = new std::string();
	while (getline(_file_reader, line)) {
		get_key_value_from_string(line, key, value);
		if (key->empty() || value->empty()) {
			continue;
		}
		insert_element(*key, *value);
		std::cout << "key:" << *key << "value:" << *value << std::endl;
	}
	_file_reader.close();
}

// Get current SkipList size 
template<typename K, typename V>
int SkipList<K, V>::size() {
	return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::cal_max_level(int predict_count) {
	float p = 0.25;
	int e = int(1 / p);
	this->_max_level = (int)(log(predict_count) / log(e));
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

	if (!is_valid_string(str)) {
		return;
	}
	*key = str.substr(0, str.find(delimiter));
	*value = str.substr(str.find(delimiter) + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

	if (str.empty()) {
		return false;
	}
	if (str.find(delimiter) == std::string::npos) {
		return false;
	}
	return true;
}

// Delete element from skip list 
template<typename K, typename V>
void SkipList<K, V>::delete_element(K key, V value) {

	mtx.lock();
	Node<K, V>* current = this->_header;
	Node<K, V>** update = new Node<K, V>*[_max_level+1];
	memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));

	// start from highest level of skip list
	for (int i = _skip_list_level; i >= 0; i--) {
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		update[i] = current;
	}

	current = current->forward[0];
	if (current != NULL && current->get_key() == key && current->get_value() == value) {

		// start for lowest level and delete the current node of each level
		for (int i = 0; i <= _skip_list_level; i++) {

			// if at level i, next node is not target node, break the loop.
			if (update[i]->forward[i] != current)
			{
				update[i]->span -= 1;
				break;
			}

			update[i]->span += current->span - 1;
			update[i]->forward[i] = current->forward[i];
		}

		// Remove levels which have no elements
		while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
			_skip_list_level--;
		}

//		std::cout << "Successfully deleted key " << key << std::endl;
		_element_count--;
	}
	mtx.unlock();
	return;
}

// Get rank of skip list 
template<typename K, typename V>
int SkipList<K, V>::get_rank(K key, V value) {

	Node<K, V>* current = this->_header;
	int rank = 0;

	// start form highest level of skip list 
	for (int i = _skip_list_level; i >= 0; i--) {
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			rank += current->span;
			current = current->forward[i];
		}
		if (current->forward[i] != NULL && current->forward[i]->get_key() == key 
			&& current->forward[i]->get_value() == value)
		{
			return (_element_count - rank);
		}
	}
	return 0;
}


template<typename K, typename V>
bool SkipList<K, V>::search_element(K key, V value) {

	std::cout << "search_element-----------------" << std::endl;
	Node<K, V>* current = _header;

	// start from highest level of skip list
	for (int i = _skip_list_level; i >= 0; i--) {
		while (current->forward[i] && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
	}

	//reached level 0 and advance pointer to right node, which we search
	current = current->forward[0];

	// if current node have key equal to searched key, we get it
	if (current && current->get_key() == key && current->get_value() == value) {
		std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
		return true;
	}

	std::cout << "Not Found Key:" << key << std::endl;
	return false;
}

// construct skip list
template<typename K, typename V>
SkipList<K, V>::SkipList(int predict_count) {

	this->_max_level = 16;
	this->_skip_list_level = 0;
	this->_element_count = 0;

	// create header node and initialize key and value to null
	K k(0);
	V v(0);
	this->_header = new Node<K, V>(k, v, _max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() {

	if (_file_writer.is_open()) {
		_file_writer.close();
	}
	if (_file_reader.is_open()) {
		_file_reader.close();
	}
	delete _header;
}

// template<typename K, typename V>
// int SkipList<K, V>::get_random_level() {
// 
// 	int k = 1;
// 	while (rand() % 2) {
// 		k++;
// 	}
// 	k = (k < _max_level) ? k : _max_level;
// 	return k;
// };

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;
	while ((rand() % 100) < 25) {
		k++;
	}
	return std::min(k, _max_level);
};
