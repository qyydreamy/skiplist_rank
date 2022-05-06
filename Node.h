#pragma once

//Class template to implement node
template<typename K, typename V>
class Node {

public:

	Node();

	Node(K k, V v, int);

	~Node();

	K get_key() const;

	V get_value() const;

	void set_value(V);

	// Linear array to hold pointers to next node of different level
	Node<K, V> **forward;

	int node_level;
	int span;

private:
	K key;
	V value;
};

template<typename K, typename V>
Node<K, V>::Node() {
	this->span = 0;
}

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
	this->key = k;
	this->value = v;
	this->node_level = level;
	this->span = 0;

	// level + 1, because array index is from 0 - level
	this->forward = new Node<K, V>*[level + 1];

	// Fill forward array with 0(NULL) 
	memset(this->forward, 0, sizeof(Node<K, V>*)*(level + 1));
};

template<typename K, typename V>
Node<K, V>::~Node() {
	delete[]forward;
};

template<typename K, typename V>
K Node<K, V>::get_key() const {
	return key;
};

template<typename K, typename V>
V Node<K, V>::get_value() const {
	return value;
};
template<typename K, typename V>
void Node<K, V>::set_value(V value) {
	this->value = value;
};
