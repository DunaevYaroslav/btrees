/**
 * (c) Anton Rigin, group BPI154(2), 2017.
 */

#ifndef BTREEBASEDINDEX_INDEXER_H
#define BTREEBASEDINDEX_INDEXER_H

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <wchar.h>

#include "btree.h"
#include "utils.h"

namespace xi
{

/**
 * Represents the B-tree based indexer of the file records.
 */
class Indexer
{

public:

    /**
     * The max length of the stored name.
     */
    static const int NAME_LENGTH = 42;

#pragma pack(push, 1)
    /**
     * Represents the key for storing and searching.
     */
    struct Key
    {
        /**
         * Default constructor.
         */
        Key() : name(L""), offset(0) { }

        /**
         * Constructor.
         *
         * Assigns 0 to offset.
         * @param nam The name for storing and searching.
         */
        Key(const std::wstring& nam);

        /**
         * Constructor.
         *
         * @param nam The name for storing and searching.
         * @param ofs The offset of record from the file's begin.
         */
        Key(const std::wstring& nam, ULong ofs) : Key(nam) { offset = ofs; }

        /**
         * The name for storing and searching.
         */
        wchar_t name[NAME_LENGTH];

        /**
         * The offset of record from the file's begin.
         */
        ULong offset;
    }; // struct Key
#pragma pack(pop)

    /**
     * Represents the comparator of name part of the key.
     */
    struct NameComparator : public FileBaseBTree::IComparator
    {
        /**
         * Compares two name keys, returns true if the first name is lexicographically less than the second name,
         * false otherwise.
         *
         * @param lhv The first name key's byte array.
         * @param rhv The second name key's byte array.
         * @param sz The key's size.
         * @return true if the first name is lexicographically less than the second name, false otherwise.
         */
        virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz);

        /**
         * Compares two name keys, returns true if they are equal, false otherwise.
         *
         * @param lhv The first name key's byte array.
         * @param rhv The second name key's byte array.
         * @param sz The key's size.
         * @return true if the two name keys are equal, false otherwise.
         */
        virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz);
    }; // struct NameComparator

public:

    /**
     * Destructor.
     *
     * Closes the tree and clears its memory.
     */
    ~Indexer();

public:

    /**
     * Creates the B-tree with given order.
     *
     * @param order The order for creating the B-tree.
     * @param treeFileName Name of the file for storing the tree.
     */
    void create(UShort order, const std::string& treeFileName);

    /**
     * Opens the stored B-tree.
     *
     * @param treeFileName Name of the file where the tree is stored.
     */
    void open(const std::string& treeFileName);

    /**
     * Closes the tree and clears its memory.
     */
    void close();

    /**
     * Performs indexing of the file: all the file's records are being stored in the B-tree during this process.
     *
     * @param fileName Name of the file for indexing.
     * @throws std::logic_error If the B-tree was not created or opened yet.
     * @throws Other exception if some special problem during reading the file appeared.
     */
    void indexFile(const std::string& fileName);

    /**
     * Finds all the occurrences of the given name in the given file using the B-tree.
     *
     * File should be indexed firstly.
     * @param name Name for searching its occurrences.
     * @param fileName Name of the file which records are being found.
     * @return List of the occurrences' strings.
     * @throws std::logic_error If the B-tree was not created or opened or given file was not indexed yet.
     * @throws Other exception if some special problem during reading the file appeared.
     */
    std::list<std::wstring> findAllOccurrences(const std::wstring& name, const std::string& fileName);

    /**
     * Returns tree's max depth reached during searching process.
     * @return Tree's max depth reached during searching process.
     */
    UInt getMaxSearchDepth() { return _bt != nullptr ? _bt->getMaxSearchDepth() : 0; }

private:

    std::string getLine(std::ifstream& file);

private:

    /**
     * The B-tree.
     */
    FileBaseBTree* _bt = nullptr;

    /**
     * The comparator of name part of the key.
     */
    NameComparator _comparator;

    /**
     * The name of the last indexed file.
     *
     * Used for checking if the file with given name was indexed.
     */
    std::string lastFileName;

}; // class Indexer

} // namespace xi

#endif // BTREEBASEDINDEX_INDEXER_H
