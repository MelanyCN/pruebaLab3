#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include "B+ Tree.h"

using namespace std;

void BPTree::insert(int key, FILE* filePtr) { 
    if (root == NULL) {
        root = new Node;
        root->isLeaf = true;
        root->keys.push_back(key);
        new (&root->ptr2TreeOrData.dataPtr) std::vector<FILE*>; // -----------------------------------------------------------------------
        root->ptr2TreeOrData.dataPtr.push_back(filePtr);

        //cout << key << " es la raiz!!" << endl;
        return;
    } else {
        Node* cursor = root;
        Node* parent = NULL;

        // searching for the possible position for the given key by doing the same procedure we did in search
        while (cursor->isLeaf == false) {
            parent = cursor;
            int idx = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
            cursor = cursor->ptr2TreeOrData.ptr2Tree[idx];
        }

        // now cursor is the leaf node in which we'll insert the new key
        if (cursor->keys.size() < maxLeafNodeLimit) {
            /*
                If current leaf Node is not FULL, find the correct position for the new key and insert!
            */
            int i = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
            cursor->keys.push_back(key);
            cursor->ptr2TreeOrData.dataPtr.push_back(filePtr);

            if (i != cursor->keys.size() - 1) {
                for (int j = cursor->keys.size() - 1; j > i; j--) {  // shifting the position for keys and datapointer
                    cursor->keys[j] = cursor->keys[j - 1];
                    cursor->ptr2TreeOrData.dataPtr[j] = cursor->ptr2TreeOrData.dataPtr[j - 1];
                }

                // since earlier step was just to inc. the size of vectors and making space, now we are simply inserting
                cursor->keys[i] = key;
                cursor->ptr2TreeOrData.dataPtr[i] = filePtr;
            }
            cout << "Insertado optimamente el key: " << key << endl;
        } else {
            /*
                DAMN!! Node Overflowed :(
                HAIYYA! Splitting the Node .
            */
            vector<int> virtualNode(cursor->keys);
            vector<FILE*> virtualDataNode(cursor->ptr2TreeOrData.dataPtr);

            // finding the probable place to insert the key
            int i = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();

            virtualNode.push_back(key);          // to create space
            virtualDataNode.push_back(filePtr);  // to create space

            if (i != virtualNode.size() - 1) {
                for (int j = virtualNode.size() - 1; j > i; j--) {  // shifting the position for keys and datapointer
                    virtualNode[j] = virtualNode[j - 1];
                    virtualDataNode[j] = virtualDataNode[j - 1];
                }

                // inserting
                virtualNode[i] = key;
                virtualDataNode[i] = filePtr;
            }
            /*
                BAZINGA! I have the power to create new Leaf :)
            */

            Node* newLeaf = new Node;
            newLeaf->isLeaf = true;
            new (&newLeaf->ptr2TreeOrData.dataPtr) std::vector<FILE*>;
            //// now, newLeaf->ptr2TreeOrData.ptr2Tree is the active member of the union

            // swapping the next ptr
            Node* temp = cursor->ptr2next;
            cursor->ptr2next = newLeaf;
            newLeaf->ptr2next = temp;

            // resizing and copying the keys & dataPtr to OldNode
            int partitionIdx = maxLeafNodeLimit / 2;
            cursor->keys.resize(partitionIdx);
            cursor->ptr2TreeOrData.dataPtr.resize(partitionIdx);
            for (int i = 0; i < partitionIdx; i++) {
                cursor->keys[i] = virtualNode[i];
                cursor->ptr2TreeOrData.dataPtr[i] = virtualDataNode[i];
            }

            // Pushing new keys & dataPtr to NewNode
            for (int i = partitionIdx; i < virtualNode.size(); i++) {
                newLeaf->keys.push_back(virtualNode[i]);
                newLeaf->ptr2TreeOrData.dataPtr.push_back(virtualDataNode[i]);
            }

            if (cursor == root) {
                Node* newRoot = new Node;
                newRoot->keys.push_back(newLeaf->keys[0]);
                new (&newRoot->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
                newRoot->ptr2TreeOrData.ptr2Tree.push_back(cursor);
                newRoot->ptr2TreeOrData.ptr2Tree.push_back(newLeaf);
                root = newRoot;
                cout << "Se modifico la raiz!" << endl;
            } else {
                // Insert new key in the parent
                insertInternal(newLeaf->keys[0], &parent, &newLeaf);
            }
        }
    }
}

void BPTree::insertInternal(int x, Node** cursor, Node** child) {  // in Internal Nodes
    if ((*cursor)->keys.size() < maxIntChildLimit - 1) {
        /*
            If cursor is not full find the position for the new key.
        */
        int i = std::upper_bound((*cursor)->keys.begin(), (*cursor)->keys.end(), x) - (*cursor)->keys.begin();
        (*cursor)->keys.push_back(x);
        // new (&(cursor)->ptr2TreeOrData.ptr2Tree) std::vector<Node>;
        //// now, root->ptr2TreeOrData.ptr2Tree is the active member of the union
        (*cursor)->ptr2TreeOrData.ptr2Tree.push_back(*child);

        if (i != (*cursor)->keys.size() - 1) {  // if there are more than one element
            // Different loops because size is different for both (i.e. diff of one)

            for (int j = (*cursor)->keys.size() - 1; j > i; j--) {  // shifting the position for keys and datapointer
                (*cursor)->keys[j] = (*cursor)->keys[j - 1];
            }

            for (int j = (*cursor)->ptr2TreeOrData.ptr2Tree.size() - 1; j > (i + 1); j--) {
                (*cursor)->ptr2TreeOrData.ptr2Tree[j] = (*cursor)->ptr2TreeOrData.ptr2Tree[j - 1];
            }

            (*cursor)->keys[i] = x;
            (*cursor)->ptr2TreeOrData.ptr2Tree[i + 1] = *child;
        }
        cout << "Insertado optimamente el key" << endl;
    } else {
        vector<int> virtualKeyNode((*cursor)->keys);
        vector<Node*> virtualTreePtrNode((*cursor)->ptr2TreeOrData.ptr2Tree);

        int i = std::upper_bound((*cursor)->keys.begin(), (*cursor)->keys.end(), x) - (*cursor)->keys.begin();  // finding the position for x
        virtualKeyNode.push_back(x);                                                                    // to create space
        virtualTreePtrNode.push_back(*child);                                                            // to create space

        if (i != virtualKeyNode.size() - 1) {
            for (int j = virtualKeyNode.size() - 1; j > i; j--) {  // shifting the position for keys and datapointer
                virtualKeyNode[j] = virtualKeyNode[j - 1];
            }

            for (int j = virtualTreePtrNode.size() - 1; j > (i + 1); j--) {
                virtualTreePtrNode[j] = virtualTreePtrNode[j - 1];
            }

            virtualKeyNode[i] = x;
            virtualTreePtrNode[i + 1] = *child;
        }

        int partitionKey = virtualKeyNode[(virtualKeyNode.size() / 2) - 1];  // exclude middle element while splitting
        int partitionIdx = (virtualKeyNode.size() / 2) - 1;

        // resizing and copying the keys & TreePtr to OldNode
        (*cursor)->keys.resize(partitionIdx);
        (*cursor)->ptr2TreeOrData.ptr2Tree.resize(partitionIdx + 1);
        for (int i = 0; i < partitionIdx; i++) {
            (*cursor)->keys[i] = virtualKeyNode[i];
        }

        for (int i = 0; i < partitionIdx + 1; i++) {
            (*cursor)->ptr2TreeOrData.ptr2Tree[i] = virtualTreePtrNode[i];
        }

        Node* newInternalNode = new Node;
        new (&newInternalNode->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
        // Pushing new keys & TreePtr to NewNode

        for (int i = partitionIdx + 1; i < virtualKeyNode.size(); i++) {
            newInternalNode->keys.push_back(virtualKeyNode[i]);
        }

        for (int i = partitionIdx + 1; i < virtualTreePtrNode.size(); i++) {  // because only key is excluded not the pointer
            newInternalNode->ptr2TreeOrData.ptr2Tree.push_back(virtualTreePtrNode[i]);
        }

        if ((*cursor) == root) {
            /*
                If cursor is a root we create a new Node
            */
            Node* newRoot = new Node;
            newRoot->keys.push_back(partitionKey);
            new (&newRoot->ptr2TreeOrData.ptr2Tree) std::vector<Node*>;
            newRoot->ptr2TreeOrData.ptr2Tree.push_back(*cursor);
            //// now, newRoot->ptr2TreeOrData.ptr2Tree is the active member of the union
            newRoot->ptr2TreeOrData.ptr2Tree.push_back(newInternalNode);

            root = newRoot;
            cout << "Se creo una nueva raiz!" << endl;
        } else {
            /*
                ::Recursion::
            */
            insertInternal(partitionKey, findParent(root, *cursor), &newInternalNode);
        }
    }
}