#include <iostream>
#include <limits>
#include <sstream>

#include "B+ Tree.h"

Node* parent = NULL;

Node::ptr::ptr() {}

Node::ptr::~ptr() {}

Node::Node() {
    this->isLeaf = false;
    this->ptr2next = NULL;
}

Node::~Node() {
    if (!isLeaf) {
        for (Node* child : ptr2TreeOrData.ptr2Tree) {
            delete child;
        }
    }
}

void Node::serialize(std::ofstream& out) {
    out << isLeaf << " " << keys.size() << " ";
    for (int key : keys) {
        out << key << " ";
    }
    if (!isLeaf) {
        out << ptr2TreeOrData.ptr2Tree.size() << " ";
        for (Node* child : ptr2TreeOrData.ptr2Tree) {
            child->serialize(out);
        }
    } else {
        out << ptr2TreeOrData.dataPtr.size() << " ";
        for (FILE* data : ptr2TreeOrData.dataPtr) {
            out << "data_file_placeholder ";
        }
    }
}

Node* Node::deserialize(std::ifstream& in) {
    Node* node = new Node();
    size_t keysSize, childrenSize;
    in >> node->isLeaf >> keysSize;
    node->keys.resize(keysSize);
    for (size_t i = 0; i < keysSize; ++i) {
        in >> node->keys[i];
    }
    if (!node->isLeaf) {
        in >> childrenSize;
        node->ptr2TreeOrData.ptr2Tree.resize(childrenSize);
        for (size_t i = 0; i < childrenSize; ++i) {
            node->ptr2TreeOrData.ptr2Tree[i] = Node::deserialize(in);
        }
    } else {
        in >> childrenSize;
        node->ptr2TreeOrData.dataPtr.resize(childrenSize);
        for (size_t i = 0; i < childrenSize; ++i) {
            std::string placeholder;
            in >> placeholder;
            node->ptr2TreeOrData.dataPtr[i] = nullptr;
        }
    }
    return node;
}

// Inorder serialize Node
void Node::inorderSerialize(std::ofstream& out) {
    if (isLeaf) {
        out << "N_hoja ";
    } else {
        out << "N_interno ";
    }

    for (int key : keys) {
        out << key << " ";
    }

    if (isLeaf) {
        for (const auto& dataPtr : ptr2TreeOrData.dataPtr) {
            // Serializar la ruta del archivo o alguna representación del FILE*
            out << "data_file_placeholder ";
        }
    }

    out << "\n";

    if (!isLeaf) {
        for (Node* child : ptr2TreeOrData.ptr2Tree) {
            if (child) {
                child->inorderSerialize(out);
            }
        }
    }
}

Node* Node::inorderDeserialize(std::ifstream& in) {
    std::string line;
    std::getline(in, line);  // Leer la línea completa

    if (line.empty() || in.eof()) {
        std::cerr << "End of file reached prematurely or empty line." << std::endl;
        return nullptr;  // Reached end of file or empty line
    }

    std::istringstream iss(line);
    std::string type;
    iss >> type;

    Node* node = new Node();
    node->isLeaf = (type == "N_hoja");

    int key;
    while (iss >> key) {
        node->keys.push_back(key);
    }

    if (!node->isLeaf) {
        for (int i = 0; i <= node->keys.size(); ++i) {
            Node* child = inorderDeserialize(in);
            if (child != nullptr) {
                node->ptr2TreeOrData.ptr2Tree.push_back(child);
            }
        }
    }

    /*
    if (node->isLeaf) {
        for (int i = 0; i < node->keys.size(); ++i) {
            std::string placeholder;
            if (iss >> placeholder) {
                node->ptr2TreeOrData.dataPtr.push_back(nullptr); // Placeholder for FILE*
            }
        }
    } else {
        for (int i = 0; i < node->keys.size() + 1; ++i) {
            Node* child = inorderDeserialize(in);
            if (child != nullptr) {
                node->ptr2TreeOrData.ptr2Tree.push_back(child);
            }
        }
    }
    */

    return node;
}

// Serialize Node to DOT format
void Node::toDot(std::ofstream& out, int& nodeCount) {
    int currentNode = nodeCount++;
    out << "node" << currentNode << " [label=\"";

    for (int key : keys) {
        out << "<f" << key << "> " << key << " |";
    }
    out.seekp(-1, std::ios_base::end); // Remove the last '|'
    out << "\"];\n";

    if (!isLeaf) {
        for (size_t i = 0; i < ptr2TreeOrData.ptr2Tree.size(); ++i) {
            Node* child = ptr2TreeOrData.ptr2Tree[i];
            if (child) {
                int childNode = nodeCount;
                child->toDot(out, nodeCount);
                out << "\"node" << currentNode << "\":f" << i << " -> \"node" << childNode << "\";\n";
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------

BPTree::BPTree() {
    this->maxIntChildLimit = 4;
    this->maxLeafNodeLimit = 3;
    this->root = NULL;
}

BPTree::~BPTree() {
    delete root;  // Asegúrate de que esto maneje correctamente la liberación de todo el árbol
}

BPTree::BPTree(int degreeInternal, int degreeLeaf) {
    this->maxIntChildLimit = degreeInternal;
    this->maxLeafNodeLimit = degreeLeaf;
    this->root = NULL;
}

int BPTree::getMaxIntChildLimit() {
    return maxIntChildLimit;
}

int BPTree::getMaxLeafNodeLimit() {
    return maxLeafNodeLimit;
}

Node* BPTree::getRoot() {
    return this->root;
}

void BPTree::setRoot(Node *ptr) {
    this->root = ptr;
}

Node* BPTree::firstLeftNode(Node* cursor) {
    if (cursor->isLeaf)
        return cursor;
    for (int i = 0; i < cursor->ptr2TreeOrData.ptr2Tree.size(); i++)
        if (cursor->ptr2TreeOrData.ptr2Tree[i] != NULL)
            return firstLeftNode(cursor->ptr2TreeOrData.ptr2Tree[i]);

    return NULL;
}

Node** BPTree::findParent(Node* cursor, Node* child) {
    /*
		Finds parent using depth first traversal and ignores leaf nodes as they cannot be parents
		also ignores second last level because we will never find parent of a leaf node during insertion using this function
	*/

    if (cursor->isLeaf || cursor->ptr2TreeOrData.ptr2Tree[0]->isLeaf)
        return NULL;

    for (int i = 0; i < cursor->ptr2TreeOrData.ptr2Tree.size(); i++) {
        if (cursor->ptr2TreeOrData.ptr2Tree[i] == child) {
            parent = cursor;
        } else {
            //Commenting To Remove vector out of bound Error: 
            //new (&cursor->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
            Node* tmpCursor = cursor->ptr2TreeOrData.ptr2Tree[i];
            findParent(tmpCursor, child);
        }
    }

    return &parent;
}

void BPTree::serialize(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file for serialization: " << filename << std::endl;
        return;
    }
    if (root) {
        root->serialize(out);
    }
    out.close();
}

void BPTree::deserialize(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Error opening file for deserialization: " << filename << std::endl;
        return;
    }
    root = Node::deserialize(in);
    in.close();
}

void BPTree::inorderSerialize(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file for serialization: " << filename << std::endl;
        return;
    }
    if (root) {
        root->inorderSerialize(out);
    }
    out.close();
}

void BPTree::inorderDeserialize(const std::string& filename) {
    std::ifstream in(filename);
    std::cout << filename << endl;
    if (!in) {
        std::cerr << "Error opening file for deserialization: " << filename << std::endl;
        return;
    }
    std::cout << "entra a desiralizar "<< endl;
    root = Node::inorderDeserialize(in);
    std::cout << "sale desiralizar "<< endl;
    if (!root) {
        std::cerr << "Error: Failed to deserialize the tree." << std::endl;
    }
    in.close();
}

void BPTree::toDot(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file for DOT output: " << filename << std::endl;
        return;
    }

    out << "digraph BPTree {\n";
    out << "node [shape=record];\n";
    int nodeCount = 0;
    if (root) {
        root->toDot(out, nodeCount);
    }
    out << "}\n";
    out.close();
}