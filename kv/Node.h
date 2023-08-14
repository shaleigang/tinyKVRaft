//
// Created by slg on 8/10/23.
//

#ifndef TINYKVRAFT_NODE_H
#define TINYKVRAFT_NODE_H

#include <cstring>

namespace kv {

template<typename K, typename V>
class Node {
public:
    Node () {}
    Node(K k, V v, int level);
    ~Node();

    K getKey() const;
    V getValue() const;

    void setValue(V value);

    Node<K,V>** forward;

    int nodeLevel;

private:
    K key_;
    V value_;
};


template<typename K, typename V>
Node<K,V>::Node(K k, V v, int level)
  : key_(k),
    value_(v),
    nodeLevel(level),
    forward(new Node<K,V>* [level + 1]){
    memset(forward, 0, sizeof(Node<K,V>*) * (level + 1));
}

template<typename K, typename V>
Node<K,V>::~Node() {
    delete [] forward;
}

template<typename K, typename V>
K Node<K, V>::getKey() const {
    return key_;
}

template<typename K, typename V>
V Node<K,V>::getValue() const {
    return value_;
}

template<typename K, typename V>
void Node<K,V>::setValue(V value) {
    value_ = value;
}



}


#endif //TINYKVRAFT_NODE_H
