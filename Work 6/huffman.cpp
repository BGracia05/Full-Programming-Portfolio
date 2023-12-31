#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "testing/SimpleTest.h"
using namespace std;

/**
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, decode the bits back to the original message text.
 *
 * You can assume that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 *
 * Your implementation may change the messageBits queue however you like. There
 * are no requirements about what it should look like after this function
 * returns. The encoding tree should be unchanged.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    EncodingTreeNode* cur = tree;
    EncodingTreeNode* root = tree;
    string result = "";

        while (!messageBits.isEmpty()) {
            Bit bit = messageBits.dequeue();
            if (bit == 0) {
                cur = cur->zero;
            } else {
                cur = cur->one;
            }
            if (cur->isLeaf()) {
                result += cur->ch;
                cur = root;
            }
        }
        return result;
}

/**
 * Reconstruct encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * You can assume that the queues are well-formed and represent
 * a valid encoding tree.
 *
 * Your implementation may change the queue parameters however you like. There
 * are no requirements about what they should look like after this function
 * returns.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if (treeShape.isEmpty()) {
        return nullptr;
    }
    Bit dir = treeShape.dequeue();
    if (dir == 0) {
        char leafChar = treeLeaves.dequeue();
        return new EncodingTreeNode(leafChar);
    } else {
        EncodingTreeNode* left = unflattenTree(treeShape, treeLeaves);
        EncodingTreeNode* right = unflattenTree(treeShape, treeLeaves);
        EncodingTreeNode* internalNode = new EncodingTreeNode(left, right);
        return internalNode;
    }
}

/**
 * Decompress the given EncodedData and return the original text.
 *
 * You can assume the input data is well-formed and was created by a correct
 * implementation of compress.
 *
 * Your implementation may change the data parameter however you like. There
 * are no requirements about what it should look like after this function
 * returns.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
string decompress(EncodedData& data) {
    EncodingTreeNode* tree = unflattenTree(data.treeShape, data.treeLeaves);
    string result = decodeText(tree, data.messageBits);
    deallocateTree(tree);
    return result;
}

/**
 * Constructs an optimal Huffman coding tree for the given text, using
 * the algorithm described in lecture.
 *
 * Reports an error if the input text does not contain at least
 * two distinct characters.
 *
 * When assembling larger trees out of smaller ones, make sure to set the first
 * tree dequeued from the queue to be the zero subtree of the new tree and the
 * second tree as the one subtree.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    Map<char, int> charFreq;
    for (char ch : text) {
        charFreq[ch]++;
    }
    PriorityQueue<EncodingTreeNode*> pq;
    for (char ch : charFreq) {
        EncodingTreeNode* leaf = new EncodingTreeNode(ch);
        pq.enqueue(leaf, charFreq[ch]);
    }
    while (pq.size() > 1) {
        EncodingTreeNode* right = pq.dequeue();
        EncodingTreeNode* left = pq.dequeue();
        pq.enqueue(new EncodingTreeNode(left, right), left->ch + right->ch);
    }
    EncodingTreeNode* huffmanTree = pq.dequeue();
    return huffmanTree;
}

/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    Queue<Bit> encodedBits;

    // Map to store character-to-bit mappings
    Map<char, Queue<Bit>> charToBits;

    // Helper function to build character-to-bit mappings using DFS
    function<void(EncodingTreeNode*, Queue<Bit>&)> buildCharToBitsMap = [&](EncodingTreeNode* node, Queue<Bit>& bits) {
        if (node->isLeaf()) {
            charToBits[node->getChar()] = bits;
            return;
        }
        if (node->zero) {
            bits.enqueue(0);
            buildCharToBitsMap(node->zero, bits);
            bits.dequeue();
        }
        if (node->one) {
            bits.enqueue(1);
            buildCharToBitsMap(node->one, bits);
            bits.dequeue();
        }
    };

    // Build character-to-bit mappings
    Queue<Bit> bitsSoFar;
    buildCharToBitsMap(tree, bitsSoFar);

    // Encode the input text
    for (char ch : text) {
        Queue<Bit> encodedCharBits = charToBits[ch];
        while (!encodedCharBits.isEmpty()) {
                encodedBits.enqueue(encodedCharBits.dequeue());
        }
    }

    return encodedBits;
}


/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a valid well-formed encoding tree.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if (tree->isLeaf()) {
        treeShape.enqueue(1);
        treeLeaves.enqueue(tree->ch);
    } else {
        treeShape.enqueue(0);
        flattenTree(tree->zero, treeShape, treeLeaves);
        flattenTree(tree->one, treeShape, treeLeaves);
    }
}



/**
 * Compress the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 *
 * TODO: Add any additional information to this comment that is necessary to describe
 * your implementation.
 */
EncodedData compress(string messageText) {
    EncodingTreeNode* tree = buildHuffmanTree(messageText);
    if (tree == nullptr) {
        error("text is empty");
    }
    Queue<Bit> eMessage = encodeText(tree, messageText);
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    flattenTree(tree, treeShape, treeLeaves);
    delete tree;

    EncodedData data;
    data.treeShape = treeShape;
    data.treeLeaves = treeLeaves;
    data.messageBits = eMessage;

    return data;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {
    /* Example encoding tree used in multiple test cases:
     *                *
     *              /   \
     *             T     *
     *                  / \
     *                 *   E
     *                / \
     *               R   S
     */
    // Leaf nodes: T, R, S, and E
    EncodingTreeNode* leafT = new EncodingTreeNode('T');
    EncodingTreeNode* leafR = new EncodingTreeNode('R');
    EncodingTreeNode* leafS = new EncodingTreeNode('S');
    EncodingTreeNode* leafE = new EncodingTreeNode('E');

    // Intermediate nodes
    EncodingTreeNode* RSRoot = new EncodingTreeNode(leafR, leafS);
    EncodingTreeNode* ERoot = new EncodingTreeNode(RSRoot, leafE);
    EncodingTreeNode* root = new EncodingTreeNode(leafT, leafE);

    return root;
}


void deallocateTree(EncodingTreeNode* t) {
    if (t == nullptr) {
        return;
    }
    if(!t->isLeaf()) {
        deallocateTree(t->zero);
        deallocateTree(t->one);
    }
    delete t;
}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {
    if (a == nullptr && b == nullptr) {
        return true;
    }
    if (a == nullptr || b == nullptr) {
        return false;
    }

    if (a->isLeaf() && b->isLeaf()) {
        return a->getChar() == b->getChar();
    }

    return areEqual(a->zero, b->zero) && areEqual(a->one, b->one);
}

/* * * * * * Test Cases Below This Point * * * * * */

/* TODO: Write your own student tests. */









/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

PROVIDED_TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

PROVIDED_TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

PROVIDED_TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

PROVIDED_TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

PROVIDED_TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

PROVIDED_TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

PROVIDED_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman"
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
