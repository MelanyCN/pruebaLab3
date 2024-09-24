#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>
//#include <filesystem>
using namespace std;

#ifndef NODE_H
#define NODE_H

class Node {
   public:
    bool isLeaf;
    vector<int> keys;
    Node* ptr2next;              //Pointer to connect next node for leaf nodes
    union ptr {                  //to make memory efficient Node
        vector<Node*> ptr2Tree;  //Array of pointers to Children sub-trees for intermediate Nodes
        vector<FILE*> dataPtr;   // Data-Pointer for the leaf node

        ptr();   // To remove the error !?
        ~ptr();  // To remove the error !?
    } ptr2TreeOrData;

    friend class BPTree;  // to access private members of the Node and hold the encapsulation concept
   public:
    Node();
    ~Node();

    //Para el manejo de guardado del arbol
    void serialize(std::ofstream& out);
    static Node* deserialize(std::ifstream& in);
    void inorderSerialize(std::ofstream& out);
    static Node* inorderDeserialize(std::ifstream& in);

    // para graficar el arbol
    void toDot(std::ofstream& out, int& nodeCount);
};

class BPTree {
   private:
    int maxIntChildLimit;                                   //Limiting  #of children for internal Nodes!
    int maxLeafNodeLimit;                                   // Limiting #of nodes for leaf Nodes!!!
    Node* root;                                             //Pointer to the B+ Tree root
    void insertInternal(int x, Node** cursor, Node** child);  //Insert x from child in cursor(parent)
    Node** findParent(Node* cursor, Node* child);
    Node* firstLeftNode(Node* cursor);

   public:
    BPTree();
    ~BPTree();
    BPTree(int degreeInternal, int degreeLeaf);
    Node* getRoot();
    int getMaxIntChildLimit();
    int getMaxLeafNodeLimit();
    void setRoot(Node *);
    void display(Node* cursor);
    void seqDisplay(Node* cursor);
    void search(int key);
    void insert(int key, FILE* filePtr);
    void removeKey(int key);
    void removeInternal(int x, Node* cursor, Node* child);

    //Para el manejo de guardado del arbol
    void serialize(const std::string& filename);
    void deserialize(const std::string& filename);
    void inorderSerialize(const std::string& filename);
    void inorderDeserialize(const std::string& filename);

    //Para graficar el arbol
    void toDot(const std::string& filename);
};

#endif