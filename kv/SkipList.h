//
// Created by slg on 8/10/23.
//

#ifndef TINYKVRAFT_SKIPLIST_H
#define TINYKVRAFT_SKIPLIST_H

#include <fstream>
#include <iostream>
#include <mutex>

#include "Node.h"

#define STORE_FILE "store/dumpFile"
std::string delimiter = ":";

namespace kv {

template<typename K, typename V>
class SkipList {
public:
    SkipList(int maxLevel);
    ~SkipList();

    int getRandomLevel();
    Node<K,V>* createNode(K key, V value, int level);
    int insertNode(const K key, const V value);
    bool searchNode(K key);
    void deleteNode(K key);

    void dumpFile();
    void loadFile();

    int size();

private:
    void getKeyValueFromString(const std::string& str, std::string* key, std::string* value);
    bool isValidString(const std::string& str);

private:
    int maxLevel_;
    int curLevel_;

    std::mutex mutex_;
    Node<K,V>* header_;

    int elementCount_;

};

template<typename K, typename V>
Node<K,V>* SkipList<K,V>::createNode(K key, V value, int level) {
    Node<K,V>* n = new Node<K,V>(key, value, level);
    return n;
}

template<typename K, typename V>
int SkipList<K,V>::insertNode(const K key, const V value) {
    std::unique_lock<std::mutex> lock(mutex_);
    Node<K,V>* current = header_;

    Node<K,V>* update[maxLevel_+1];
    memset(update, sizeof(Node<K,V>*) * (maxLevel_ + 1));

    for(int i = curLevel_; i >= 0; i--) {
        while(current->forward[i] != NULL && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if(current != NULL && current->getKey() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        return 1;
    }

    if(current == NULL || current->getKey() != key) {
        int randomLevel = getRandomLevel();
        if(randomLevel > curLevel_) {
            for(int i = curLevel_ + 1; i < randomLevel + 1; i++) {
                update[i] = header_;
            }
            curLevel_ = randomLevel;
        }

        Node<K,V>* insertedNode = createNode(key, value, randomLevel);

        for(int i = 0; i <= randomLevel; ++i) {
            insertedNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = insertedNode;
        }
        std::cout << "Successful insert key: " << key << ", value: " << value << std::endl;
        ++elementCount_;
    }

    return 0;

}

template<typename K, typename V>
int SkipList<K, V>::getRandomLevel() {
    int level = 1;
    while ((random() & 0xFFFF) < (0.25 * 0xFFFF))
        level += 1;
    return (level < maxLevel_) ? level : maxLevel_;
}

template<typename K, typename V>
int SkipList<K, V>::size() {
    return elementCount_;
}

template<typename K, typename V>
void SkipList<K, V>::getKeyValueFromString(const std::string &str, std::string *key, std::string *value) {

    if(!isValidString(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::isValidString(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

template<typename K, typename V>
void SkipList<K, V>::deleteNode(K key) {

    std::unique_lock<std::mutex> lock(mutex_);
    Node<K, V> *current = this->_header;
    Node<K, V> *update[maxLevel_+1];
    memset(update, 0, sizeof(Node<K, V>*)*(maxLevel_+1));

    for (int i = curLevel_; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {

        for (int i = 0; i <= curLevel_; i++) {

            if (update[i]->forward[i] != current)
                break;

            update[i]->forward[i] = current->forward[i];
        }

        while (curLevel_ > 0 && header_->forward[curLevel_]= 0) {
            curLevel_--;
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        elementCount_--;
    }
    return;
}

template<typename K, typename V>
bool SkipList<K, V>::searchNode(K key) {

    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = header_;

    for (int i = curLevel_; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

template<typename K, typename V>
SkipList<K, V>::SkipList(int maxLevel)
  : maxLevel_(maxLevel),
    curLevel_(0),
    elementCount_(0) {
    K k;
    V v;
    header_ = new Node<K, V>(k, v, maxLevel_);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() {

    delete header_;
}

template<typename K, typename V>
void SkipList<K, V>::dumpFile() {

    std::cout << "dump_file-----------------" << std::endl;
    std::ofstream fileWriter;
    fileWriter.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];

    while (node != NULL) {
        fileWriter << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    fileWriter.flush();
    fileWriter.close();
}

template<typename K, typename V>
void SkipList<K, V>::loadFile() {
    std::ifstream fileReader;
    fileReader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(fileReader, line)) {
        getKeyValueFromString(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insertNode(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    fileReader.close();
}




}

#endif //TINYKVRAFT_SKIPLIST_H
