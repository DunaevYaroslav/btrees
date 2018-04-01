﻿/// \file
/// \brief     B-tree, B+-tree and B*-tree classes
/// \authors   Sergey Shershakov, Anton Rigin
/// \version   0.1.0
/// \date      01.05.2017 -- 04.02.2018
///            This is a part of the course "Algorithms and Data Structures"
///            provided by the School of Software Engineering of the Faculty
///            of Computer Science at the Higher School of Economics
///            and of the course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include "btree.h"

#include <stdexcept>        // std::invalid_argument
#include <cstring>          // memset

namespace xi {

//==============================================================================
// class BaseBTree
//==============================================================================

bool BaseBTree::Header::checkIntegrity()
{
    return (sign == VALID_SIGN) && (order >= 1) && (recSize > 0);
}

BaseBTree::BaseBTree(UShort order, UShort recSize, IComparator* comparator, std::iostream* stream)
    : _order(order), 
    _recSize(recSize), 
    _comparator(comparator),
    _stream(stream), 
    _lastPageNum(0),
    _rootPageNum(0),
    _maxSearchDepth(0),
    _rootPage(this)
#ifdef BTREE_WITH_REUSING_FREE_PAGES
  , _freePagesCounter(0)
#endif
{
}

BaseBTree::BaseBTree(IComparator* comparator, std::iostream* stream):
    BaseBTree(
        0,
        0,
        comparator, stream)
{

}

BaseBTree::~BaseBTree()
{
}

void BaseBTree::resetBTree()
{
    _order = 0;
    _recSize = 0;
    _stream = nullptr;
    _comparator = nullptr;
}

void BaseBTree::readPage(UInt pnum, Byte* dst)
{    
    checkForOpenStream();
    if (pnum == 0 || pnum > getLastPageNum())
        throw std::invalid_argument("Can't read a non-existing page");

    readPageInternal(pnum, dst);
}

void BaseBTree::writePage(UInt pnum, const Byte* dst)
{
    checkForOpenStream();

    if (pnum == 0 || pnum > getLastPageNum())
        throw std::invalid_argument("Can't write a non-existing page");

    writePageInternal(pnum, dst);

}

bool BaseBTree::checkKeysNumber(UShort keysNum, bool isRoot) // TODO: fix.
{
//    if (keysNum > getMaxKeys())
//        return false;
//
//    if (isRoot)
//        return true;
//
//    return (keysNum >= getMinKeys());

    return true;
}

void BaseBTree::checkKeysNumberExc(UShort keysNum, bool isRoot)
{
    if (!checkKeysNumber(keysNum,  isRoot))
        throw std::invalid_argument("Invalid number of keys for a node");
}

UInt BaseBTree::allocPage(PageWrapper& pw, UShort keysNum, bool isLeaf /*= false*/)
{
    checkForOpenStream();
    checkKeysNumberExc(keysNum, pw.isRoot());

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    return allocPageUsingFreePages(pw, keysNum, pw.isRoot(), isLeaf);

#else

    return allocPageInternal(pw, keysNum, pw.isRoot(),  isLeaf);

#endif

}

xi::UInt BaseBTree::allocNewRootPage(PageWrapper& pw)
{
    checkForOpenStream();

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    return allocPageUsingFreePages(pw, 0, true, false);

#else

    return allocPageInternal(pw, 0, true, false);

#endif

}

void BaseBTree::insert(const Byte* k)
{
    if(_rootPage.isFull())
    {
        UInt prevRootPageNum = _rootPageNum;
        _rootPage.allocNewRootPage();
        _rootPage.setCursor(0, prevRootPageNum);
        _rootPage.setAsRoot(true);
        _rootPage.splitChild(0);
    }
    insertNonFull(k, _rootPage);
}

void BaseBTree::insertNonFull(const Byte* k, PageWrapper& currentNode)
{
    if (currentNode.isFull())
        throw std::domain_error("Node is full. Can't insert");

    IComparator* c = getComparator();
    if (!c)
        throw std::runtime_error("Comparator not set. Can't insert");

    // Getting the current pages' keys number (it will be used several times in this method).
    UShort keysNum = currentNode.getKeysNum();

    int i = keysNum - 1;

    // If the current page is leaf, we can simply insert the key into the page.
    if(currentNode.isLeaf())
    {
        // Increasing number of keys in the node for inserting the key.
        currentNode.setKeyNum(keysNum + 1);

        // Shifting keys in the node to the right.
        for( ; i >= 0 && c->compare(k, currentNode.getKey(i), getRecSize()); --i)
            currentNode.copyKey(currentNode.getKey(i + 1), currentNode.getKey(i));

        // Inserting the key to the node.
        currentNode.copyKey(currentNode.getKey(i + 1), k);

        // Writing the page to the disk.
        currentNode.writePage();
    }
    else // If the current page is not leaf, we should check if we should split one of the children and recursively call the method on the child.
    {
        // Searching necessary child.
        for( ; i >= 0 && c->compare(k, currentNode.getKey(i), getRecSize()); --i) ;
        ++i;

        // Reading the child from disk.
        PageWrapper child(this);
        child.readPageFromChild(currentNode, i);

        // If the child is full, we should split it and recursively call the method on the child or on the new created child.
        if(child.isFull())
        {
            PageWrapper newChild(this);
            splitChild(currentNode, i, child, newChild);
            if(c->compare(currentNode.getKey(i), k, getRecSize()))
                insertNonFull(k, newChild);
            else
                insertNonFull(k, child);
        }
        else // If the child is not full, we can simply recursively call the method on it.
            insertNonFull(k, child);
    }
}

Byte* BaseBTree::search(const Byte* k)
{
    if (_comparator == nullptr)
        throw std::runtime_error("Comparator not set. Can't search");

    _maxSearchDepth = 0;

    return search(k, _rootPage, 1);
}

Byte* BaseBTree::search(const Byte* k, PageWrapper& currentPage, UInt currentDepth)
{
    int i;
    UShort keysNum = currentPage.getKeysNum();
    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    if(i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) // If the key has been found.
    {
        Byte* result = new Byte[_recSize];
        currentPage.copyKey(result, currentPage.getKey(i));
        return result;
    }
    else if(currentPage.isLeaf())  // If the key has not been found and the current page is a leaf, there is not the key in the tree.
        return nullptr;
    else // If the current page is not a leaf and does not contain the key.
    {
        PageWrapper nextPage(this);
        nextPage.readPageFromChild(currentPage, i);
        return search(k, nextPage, currentDepth + 1);
    }
}

int BaseBTree::searchAll(const Byte* k, std::list<Byte*>& keys)
{
    if (_comparator == nullptr)
        throw std::runtime_error("Comparator not set. Can't search");

    _maxSearchDepth = 0;

    return searchAll(k, keys, _rootPage, 1);
}

int BaseBTree::searchAll(const Byte* k, std::list<Byte*>& keys, PageWrapper& currentPage, UInt currentDepth)
{
    if(currentDepth > _maxSearchDepth)
        _maxSearchDepth = currentDepth;

    int amount = 0;
    int i;
    UShort keysNum = currentPage.getKeysNum();
    bool isLeaf = currentPage.isLeaf();

    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    int first = i;

    PageWrapper nextPage(this);
    for( ; i < keysNum && (i == first || _comparator->isEqual(k, currentPage.getKey(i), _recSize)); ++i)
    {
        if(_comparator->isEqual(k, currentPage.getKey(i), _recSize)) // If the key has been found.
        {
            Byte* result = new Byte[_recSize];
            currentPage.copyKey(result, currentPage.getKey(i));
            keys.push_back(result);
            ++amount;
        }

        if(!isLeaf) // If the current page is not a leaf.
        {
            nextPage.readPageFromChild(currentPage, i);
            amount += searchAll(k, keys, nextPage, currentDepth + 1);
        }
    }

    if(!isLeaf) // If the current page is not a leaf.
    {
        nextPage.readPageFromChild(currentPage, i);
        amount += searchAll(k, keys, nextPage, currentDepth + 1);
    }

    return amount;
}

void BaseBTree::splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild)
{
    if (node.isFull())
        throw std::domain_error("A parent node is full, so its child can't be splitted");

    if (iChild > node.getKeysNum())
        throw std::invalid_argument("Cursor not exists");

    if (leftChild.getPageNum() == 0)
        leftChild.readPageFromChild(node, iChild);

    // Allocating page for the right child which is new created child.
    rightChild.allocPage(_minKeys, leftChild.isLeaf());

    // Copying keys and cursors from the right half of the left child to the right child.
    rightChild.copyKeys(rightChild.getKey(0), leftChild.getKey(_minKeys + 1), _minKeys);
    if(!leftChild.isLeaf())
        node.copyCursors(rightChild.getCursorPtr(0), leftChild.getCursorPtr(_minKeys + 1), _minKeys + 1);

    // Increasing number of keys num in the parent node for inserting median.
    UShort keysNum = node.getKeysNum() + 1;
    node.setKeyNum(keysNum);

    // Shifting coursors in the parent to the right.
    for(int j = keysNum - 1; j > iChild; --j)
        node.copyCursors(node.getCursorPtr(j + 1), node.getCursorPtr(j), 1);

    // Setting coursor in the parent to the right child.
    node.setCursor(iChild + 1, rightChild.getPageNum());

    // Shifting coursors in the parent to the right.
    for(int j = keysNum - 2; j >= iChild; --j)
        node.copyKey(node.getKey(j + 1), node.getKey(j));

    // Moving median to the parent.
    node.copyKey(node.getKey(iChild), leftChild.getKey(_minKeys));
    leftChild.setKeyNum(_minKeys);

    // Writing the pages to the disk.
    leftChild.writePage();
    rightChild.writePage();
    node.writePage();
}

#ifdef BTREE_WITH_DELETION

bool BaseBTree::remove(const Byte* k)
{
    if (_comparator == nullptr)
        throw std::runtime_error("Comparator not set. Can't remove");

    return remove(k, _rootPage);
}

bool BaseBTree::remove(const Byte* k, PageWrapper& currentPage)
{
    int i;
    UShort keysNum = currentPage.getKeysNum();

    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    if(i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) // Cases 1 and 2: if the key has been found, removes it recursively.
    {
        if(currentPage.isRoot())
            return removeByKeyNum(i, _rootPage);
        else
            return removeByKeyNum(i, currentPage);
    }

    else if(currentPage.isLeaf()) // If the key has not been found and the current page is a leaf, there is not the key in the tree.
        return false;

    // Case 3.
    PageWrapper child(this);
    PageWrapper leftNeighbour(this);
    PageWrapper rightNeighbour(this);
    if(prepareSubtree(i, currentPage, child, leftNeighbour, rightNeighbour))
        return remove(k, leftNeighbour);
    else
        return remove(k, child);
}

int BaseBTree::removeAll(const Byte* k)
{
    if (_comparator == nullptr)
        throw std::runtime_error("Comparator not set. Can't remove");

    return removeAll(k, _rootPage);
}

int BaseBTree::removeAll(const Byte* k, PageWrapper& currentPage)
{
    if(currentPage.getKeysNum() == 0)
        return 0;

    int amount = 0;
    int i;
    UShort keysNum = currentPage.getKeysNum();
    bool isLeaf = currentPage.isLeaf();
    bool isRoot = currentPage.isRoot();

    PageWrapper child(this);
    PageWrapper leftNeighbour(this);
    PageWrapper rightNeighbour(this);

    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    int first = i;

    for( ; i <= keysNum && (i == first ||
            (i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) ||
            (i > first && _comparator->isEqual(k, currentPage.getKey(i - 1), _recSize))); ++i)
    {
        if(i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) // Cases 1 and 2: if the key has been found, removes it recursively.
        {
            removeByKeyNum(i, currentPage);

            if(isRoot && !currentPage.isRoot())
            {
                currentPage.readPage(_rootPageNum);
                keysNum = currentPage.getKeysNum();
                i = -1;
                continue;
            }

            ++amount;

            keysNum = currentPage.getKeysNum();
            --i;

            if(!currentPage.isRoot() && keysNum <= _minKeys)
                return amount;

            continue;
        }

        if(!isLeaf) // Case 3.
        {
            if(prepareSubtree(i, currentPage, child, leftNeighbour, rightNeighbour))
            {
                amount += removeAll(k, leftNeighbour);
                --i;
                if (leftNeighbour.getKeysNum() <= _minKeys)
                    --i;
            }
            else
            {
                amount += removeAll(k, child);
                if(child.getKeysNum() <= _minKeys)
                    --i;
            }

            if(isRoot && !currentPage.isRoot() || currentPage.getKeysNum() > keysNum)
            {
                currentPage.readPage(_rootPageNum);
                i = -1;
                isLeaf = true;
            }

            if(currentPage.getKeysNum() < keysNum)
                --i;

            keysNum = currentPage.getKeysNum();
        }

    }

    if(isRoot && currentPage.getData() != _rootPage.getData())
        loadRootPage();

    return amount;
}

bool BaseBTree::removeByKeyNum(UShort keyNum, PageWrapper& currentPage)
{
    UShort keysNum = currentPage.getKeysNum();
    Byte* k = currentPage.getKey(keyNum);

    if(currentPage.isLeaf()) // Case 1: we can simply remove the key from the node.
    {
        for(int j = keyNum; j < keysNum - 1; ++j)
            currentPage.copyKey(currentPage.getKey(j), currentPage.getKey(j + 1));

        currentPage.setKeyNum(keysNum - 1);

        currentPage.writePage();

        return true;
    }

    // Case 2:

    const Byte* replace = nullptr;

    PageWrapper leftChild(this), rightChild(this);
    leftChild.readPageFromChild(currentPage, keyNum);
    if(leftChild.getKeysNum() >= _minKeys + 1) // If the left child has not less than _order elements, we should get and recursively remove
                                               // predecessor of the removed key, and replace the removed key by this predecessor (case 2a).
        replace = getAndRemoveMaxKey(leftChild);

    if(replace == nullptr)
    {
        rightChild.readPageFromChild(currentPage, keyNum + 1);
        if(rightChild.getKeysNum() >= _minKeys + 1) // If the right child has not less than _order elements, we should get and recursively remove
                                                    // successor of the removed key, and replace the removed key by this predecessor (case 2b).
            replace = getAndRemoveMinKey(rightChild);
    }

    // Replacing the removed key by predecessor or successor.
    if(replace != nullptr)
    {
        currentPage.copyKey(currentPage.getKey(keyNum), replace);
        delete[] replace;

        currentPage.writePage();

        return true;
    }

    // Case 2c: we should merge the left child and the right child and use the removed key as the median in the new node.
    // After it, we should recursively remove the removed key from the new node.

    mergeChildren(leftChild, rightChild, currentPage, keyNum);

    if(leftChild.isRoot())
        removeByKeyNum(_maxKeys / 2, _rootPage);
    else
        removeByKeyNum(_maxKeys / 2, leftChild);

    return true;
}

bool BaseBTree::prepareSubtree(UShort cursorNum, PageWrapper& currentPage, PageWrapper& child, PageWrapper& leftNeighbour, PageWrapper& rightNeighbour)
{
    UShort keysNum = currentPage.getKeysNum();

    // Reading the child from disk.
    child.readPageFromChild(currentPage, cursorNum);
    UShort childKeysNum = child.getKeysNum();
    if(childKeysNum <= _minKeys) // If the number of keys in child is less than _order, we should consider cases 3a 3b.
    {
        if(cursorNum >= 1) // Checking case 3a for the left neighbour, if the cursor is not the first one.
        {
            leftNeighbour.readPageFromChild(currentPage, cursorNum - 1);
            UShort neighbourKeysNum = leftNeighbour.getKeysNum();
            if(neighbourKeysNum >= _minKeys + 1) // Case 3a for the left neighbour.
            {
                child.setKeyNum(++childKeysNum);
                child.copyCursors(child.getCursorPtr(childKeysNum), child.getCursorPtr(childKeysNum - 1), 1);
                for(int j = childKeysNum - 2; j >= 0; --j)
                {
                    child.copyKey(child.getKey(j + 1), child.getKey(j));
                    child.copyCursors(child.getCursorPtr(j + 1), child.getCursorPtr(j), 1);
                }

                // Moving separator to the child.
                child.copyKey(child.getKey(0), currentPage.getKey(cursorNum - 1));

                // Replacing separator in the current page by the border key of the neighbour.
                currentPage.copyKey(currentPage.getKey(cursorNum - 1), leftNeighbour.getKey(neighbourKeysNum - 1));

                // Moving the border cursor from the neighbour to the child.
                child.copyCursors(child.getCursorPtr(0), leftNeighbour.getCursorPtr(neighbourKeysNum), 1);

                leftNeighbour.setKeyNum(--neighbourKeysNum);

                child.writePage();
                leftNeighbour.writePage();
                currentPage.writePage();

                return false;
            }
        }

        if(cursorNum < keysNum) // Checking case 3b for the right neighbour, if the cursor is not the last one.
        {
            rightNeighbour.readPageFromChild(currentPage, cursorNum + 1);
            UShort neighbourKeysNum = rightNeighbour.getKeysNum();
            if(neighbourKeysNum >= _minKeys + 1) // Case 3a for the right neighbour.
            {
                child.setKeyNum(++childKeysNum);

                // Moving separator to the child.
                child.copyKey(child.getKey(childKeysNum - 1), currentPage.getKey(cursorNum));

                // Replacing separator in the current page by the border key of the neighbour.
                currentPage.copyKey(currentPage.getKey(cursorNum), rightNeighbour.getKey(0));

                // Moving the border cursor from the neighbour to the child.
                child.copyCursors(child.getCursorPtr(childKeysNum), rightNeighbour.getCursorPtr(0), 1);

                for(int j = 0; j < neighbourKeysNum - 1; ++j)
                {
                    rightNeighbour.copyKey(rightNeighbour.getKey(j), rightNeighbour.getKey(j + 1));
                    rightNeighbour.copyCursors(rightNeighbour.getCursorPtr(j), rightNeighbour.getCursorPtr(j + 1), 1);
                }
                rightNeighbour.copyCursors(rightNeighbour.getCursorPtr(neighbourKeysNum - 1), rightNeighbour.getCursorPtr(neighbourKeysNum), 1);
                rightNeighbour.setKeyNum(--neighbourKeysNum);

                child.writePage();
                rightNeighbour.writePage();
                currentPage.writePage();

                return false;
            }
        }

        // Case 3b: merging the child with the neighbour.

        if(cursorNum >= 1)
        {
            mergeChildren(leftNeighbour, child, currentPage, cursorNum - 1);

            return true;
        }

        mergeChildren(child, rightNeighbour, currentPage, cursorNum);

        return false;
    }

    return false;
}

const Byte* BaseBTree::getAndRemoveMaxKey(PageWrapper& pw)
{
    // Only cases 1 and 3 are relevant for this task (because the max key will be in the leaf).

    if(pw.isLeaf()) // Case 1.
    {
        Byte* maxKey = new Byte[_recSize];
        pw.copyKey(maxKey, pw.getKey(pw.getKeysNum() - 1));

        pw.setKeyNum(pw.getKeysNum() - 1);

        pw.writePage();

        return maxKey;
    }

    // Case 3.
    PageWrapper child(this);
    PageWrapper leftNeighbour(this);
    PageWrapper rightNeighbour(this);
    if(prepareSubtree(pw.getKeysNum(), pw, child, leftNeighbour, rightNeighbour))
        return getAndRemoveMaxKey(leftNeighbour);
    else
        return getAndRemoveMaxKey(child);
}

const Byte* BaseBTree::getAndRemoveMinKey(PageWrapper& pw)
{
    // Only cases 1 and 3 are relevant for this task (because the min key will be in the leaf).

    if(pw.isLeaf()) // Case 1.
    {
        Byte* minKey = new Byte[_recSize];
        pw.copyKey(minKey, pw.getKey(0));

        UShort pwKeysNum = pw.getKeysNum();
        for(int j = 0; j < pwKeysNum - 1; ++j)
            pw.copyKey(pw.getKey(j), pw.getKey(j + 1));

        pw.setKeyNum(pw.getKeysNum() - 1);

        pw.writePage();

        return minKey;
    }

    // Case 3.
    PageWrapper child(this);
    PageWrapper leftNeighbour(this);
    PageWrapper rightNeighbour(this);
    if(prepareSubtree(pw.getKeysNum(), pw, child, leftNeighbour, rightNeighbour))
        return getAndRemoveMinKey(leftNeighbour);
    else
        return getAndRemoveMinKey(child);
}

void BaseBTree::mergeChildren(PageWrapper& leftChild, PageWrapper& rightChild, PageWrapper& currentPage, UShort medianNum)
{
    UShort keysNum = currentPage.getKeysNum();
    Byte* median = currentPage.getKey(medianNum);

    leftChild.setKeyNum(_maxKeys);

    // Moving median to the left child.
    leftChild.copyKey(leftChild.getKey(_minKeys), median);

    // Moving keys and cursors from the right child to the left child.
    leftChild.copyKeys(leftChild.getKey(_minKeys + 1), rightChild.getKey(0), _minKeys);
    leftChild.copyCursors(leftChild.getCursorPtr(_minKeys + 1), rightChild.getCursorPtr(0), _minKeys + 1);

    // Shifting keys and cursors in the current page to the left, after removing the median from the current page.
    for(int j = medianNum; j < keysNum - 1; ++j)
    {
        currentPage.copyKey(currentPage.getKey(j), currentPage.getKey(j + 1));
        currentPage.copyCursors(currentPage.getCursorPtr(j + 1), currentPage.getCursorPtr(j + 2), 1);
    }

    leftChild.writePage();

    if(currentPage.getKeysNum() == 1) // In the case when the current page was root and becomes empty.
    {
        leftChild.setAsRoot(true);

#ifdef BTREE_WITH_REUSING_FREE_PAGES

        // Making the page free for reusing.
        markPageFree(currentPage.getPageNum());

#endif

        loadRootPage();
    }
    else
    {
        currentPage.setKeyNum(keysNum - 1);
        currentPage.writePage();
    }


#ifdef BTREE_WITH_REUSING_FREE_PAGES

    // Making the page free for reusing.
    markPageFree(rightChild.getPageNum());

#endif

}

#endif // BTREE_WITH_DELETION

#ifdef BTREE_WITH_REUSING_FREE_PAGES

UInt BaseBTree::allocPageUsingFreePages(PageWrapper& pw, UShort keysNum, bool isRoot, bool isLeaf)
{
    if(_freePagesCounter == 0) // If the count of the free pages equals 0, we should write new page to the end
                               // and move the free pages counter to the end of the file.
    {
        allocPageInternal(pw, keysNum, isRoot, isLeaf);
        writeFreePagesCounter();
        return _lastPageNum;
    }
    else // If the count of the free pages does not equal 0, we should find the free page in the file and rewrite it using the LIFO rule.
    {
        UInt lastFree = getLastFreePageNum();
        allocPageUsingFreePagesInternal(pw, keysNum, isRoot, isLeaf, lastFree);
        --_freePagesCounter;
        writeFreePagesCounter();
        return lastFree;
    }
}

void BaseBTree::loadFreePagesCounter()
{
    _stream->seekg(getFreePagesInfoAreaOfs(), std::ios_base::beg);
    _stream->read((char*)&_freePagesCounter, FREE_PAGES_COUNTER_SZ);
}

void BaseBTree::writeFreePagesCounter()
{
    _stream->seekg(getFreePagesInfoAreaOfs(), std::ios_base::beg);
    _stream->write((const char*)&_freePagesCounter, FREE_PAGES_COUNTER_SZ);
}

UInt BaseBTree::getLastFreePageNum()
{
    UInt result;
    _stream->seekg(getLastFreePageNumOfs(), std::ios_base::beg);
    _stream->read((char*)&result, FREE_PAGE_NUM_SZ);
    return result;
}

void BaseBTree::allocPageUsingFreePagesInternal(PageWrapper& pw, UShort keysNum, bool isRoot, bool isLeaf, UInt freePageNum)
{
    pw.clear();
    pw.setKeyNumLeaf(keysNum, isRoot, isLeaf);

    _stream->seekg(getPageOfs(freePageNum), std::ios_base::beg);
    _stream->write((const char*)pw.getData(), getNodePageSize());
}

ULong BaseBTree::getPageOfs(UInt pageNum)
{
    return (ULong)FIRST_PAGE_OFS + (pageNum - 1) * _nodePageSize;
}

ULong BaseBTree::getFreePagesInfoAreaOfs()
{
    return getPageOfs(_lastPageNum + 1);
}

ULong BaseBTree::getLastFreePageNumOfs()
{
    return getFreePagesInfoAreaOfs() + FREE_PAGES_COUNTER_SZ + (_freePagesCounter - 1) * FREE_PAGE_NUM_SZ;
}

void BaseBTree::markPageFree(UInt pageNum)
{
    if(pageNum > _lastPageNum)
        throw std::invalid_argument("No page with a such number");

    _stream->seekg(getLastFreePageNumOfs() + FREE_PAGE_NUM_SZ, std::ios_base::beg);
    _stream->write((const char*)&pageNum, FREE_PAGE_NUM_SZ);

    ++_freePagesCounter;
    writeFreePagesCounter();
}

#endif // BTREE_WITH_REUSING_FREE_PAGES

UInt BaseBTree::allocPageInternal(PageWrapper& pw, UShort keysNum, bool isRoot, bool isLeaf)
{
    pw.clear();
    pw.setKeyNumLeaf(keysNum, isRoot, isLeaf);

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    _stream->seekg(getFreePagesInfoAreaOfs(), std::ios_base::beg);

#else

    _stream->seekg(0, std::ios_base::end);

#endif

    _stream->write((const char*)pw.getData(), getNodePageSize());

    ++_lastPageNum;
    writePageCounter();

    return _lastPageNum;
}

void BaseBTree::readPageInternal(UInt pnum, Byte* dst)
{
    gotoPage(pnum);
    _stream->read((char*)dst, getNodePageSize());
}

void BaseBTree::writePageInternal(UInt pnum, const Byte* dst)
{
    gotoPage(pnum);
    _stream->write((const char*)dst, getNodePageSize());
}

void BaseBTree::gotoPage(UInt pnum)
{
    UInt pageOfs = FIRST_PAGE_OFS + getNodePageSize() * (pnum - 1);
    _stream->seekg(pageOfs, std::ios_base::beg);
}

void BaseBTree::loadTree()
{
    Header hdr;
    readHeader(hdr);

    if (_stream->fail())
    {
        throw std::runtime_error("Can't read header");
    }

    if (!hdr.checkIntegrity())
    {
        throw std::runtime_error("Stream is not a valid xi B-tree file");
    }

    setOrder(hdr.order, hdr.recSize);

    readPageCounter();
    readRootPageNum();

    if (_stream->fail())
    {
        throw std::runtime_error("Can't read necessary fields. File corrupted");
    }

    loadRootPage();

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    loadFreePagesCounter();

#endif

}

bool BaseBTree::isFull(const PageWrapper& page) const
{
    return page.getKeysNum() == getMaxKeys();
}

void BaseBTree::loadRootPage()
{
    if (getRootPageNum() == 0)
        throw std::runtime_error("Root page is not defined");

    _rootPage.readPage(getRootPageNum());
    _rootPage.setAsRoot(false);

}

void BaseBTree::createTree(UShort order, UShort recSize)
{
    setOrder(order, recSize);

    writeHeader();
    writePageCounter();
    writeRootPageNum();

    createRootPage();

#ifdef BTREE_WITH_REUSING_FREE_PAGES

    writeFreePagesCounter();

#endif

}

void BaseBTree::createRootPage()
{
    _rootPage.allocPage(0, true);
    _rootPage.setAsRoot();
}

void BaseBTree::checkForOpenStream()
{
    if (!isOpened())
        throw std::runtime_error("Stream is not ready");
}

void BaseBTree::writeHeader()
{    
    Header hdr(_order, _recSize);    
    _stream->write((const char*)(void*)&hdr, HEADER_SIZE);
}

void BaseBTree::readHeader(Header& hdr)
{
    _stream->seekg(HEADER_OFS, std::ios_base::beg);
    _stream->read((char*)&hdr, HEADER_SIZE);
}

void BaseBTree::writePageCounter()
{
    _stream->seekg(PAGE_COUNTER_OFS, std::ios_base::beg);
    _stream->write((const char*)&_lastPageNum, PAGE_COUNTER_SZ);
}

void BaseBTree::readPageCounter()
{
    _stream->seekg(PAGE_COUNTER_OFS, std::ios_base::beg);    
    _stream->read((char*)&_lastPageNum, PAGE_COUNTER_SZ);
}

void BaseBTree::writeRootPageNum()
{
    _stream->seekg(ROOT_PAGE_NUM_OFS, std::ios_base::beg);
    _stream->write((const char*)&_rootPageNum, ROOT_PAGE_NUM_SZ);

}

void BaseBTree::readRootPageNum()
{
    _stream->seekg(ROOT_PAGE_NUM_OFS, std::ios_base::beg);
    _stream->read((char*)&_rootPageNum, ROOT_PAGE_NUM_SZ);
}

void BaseBTree::setRootPageNum(UInt pnum, bool writeFlag /*= true*/)
{
    _rootPageNum = pnum;
    if (writeFlag)
        writeRootPageNum();
}

void BaseBTree::setOrder(UShort order, UShort recSize)
{
    _order = order;
    _recSize = recSize;

    _minKeys = _order - 1;
    _maxKeys = 2 * _order - 1;

    if (_maxKeys > MAX_KEYS_NUM)
        throw std::invalid_argument("For a given B-tree order, there is an excess of the maximum number of keys");

    _keysSize = _recSize * _maxKeys;
    _cursorsOfs = _keysSize + KEYS_OFS;
    _nodePageSize = _cursorsOfs + CURSOR_SZ * (_maxKeys + 1);

    reallocWorkPages();
}

void BaseBTree::reallocWorkPages()
{
    _rootPage.reallocData(_nodePageSize);
}

//==============================================================================
// class BaseBTree::PageWrapper
//==============================================================================

BaseBTree::PageWrapper::PageWrapper(BaseBTree* tr) :
    _data(nullptr)
    , _tree(tr)
    , _pageNum(0)
{
    if (_tree->isOpened())
        reallocData(_tree->getNodePageSize());
}

BaseBTree::PageWrapper::~PageWrapper()
{
    reallocData(0);
}

void BaseBTree::PageWrapper::reallocData(UInt sz)
{
    if (_data)
    {
        delete[] _data;
        _data = nullptr;
    }

    if (sz)
        _data = new Byte[sz];
        
}

void BaseBTree::PageWrapper::clear()
{
    if (!_data)
        return;

    memset(_data, 0, _tree->getNodePageSize());
}

void BaseBTree::PageWrapper::setKeyNumLeaf(UShort keysNum, bool isRoot, bool isLeaf) //NodeType nt)
{
    _tree->checkKeysNumberExc(keysNum, isRoot);

    if (isLeaf)
        keysNum |= LEAF_NODE_MASK; // 0x8000;

    *((UShort*)&_data[0]) = keysNum;
}

void BaseBTree::PageWrapper::setKeyNum(UShort keysNum, bool isRoot) //NodeType nt)
{
    _tree->checkKeysNumberExc(keysNum, isRoot);


    UShort kldata = *((UShort*)&_data[0]);
    kldata &= LEAF_NODE_MASK;
    kldata |= keysNum;

    *((UShort*)&_data[0]) = kldata;
}

void BaseBTree::PageWrapper::setLeaf(bool isLeaf)
{
    UShort kldata = *((UShort*)&_data[0]);
    kldata &= ~LEAF_NODE_MASK;
    if (isLeaf)
        kldata |= LEAF_NODE_MASK;   // 0x8000;

    *((UShort*)&_data[0]) = kldata;
}

bool BaseBTree::PageWrapper::isLeaf() const
{
    UShort info = *((UShort*)&_data[0]);
    bool isLeaf = (info & LEAF_NODE_MASK) != 0;
    return isLeaf;
}

UShort BaseBTree::PageWrapper::getKeysNum() const
{
    UShort info = *((UShort*)&_data[0]);
    UShort keysNum = (info & ~LEAF_NODE_MASK);
    return keysNum;
}

Byte* BaseBTree::PageWrapper::getKey(UShort num)
{
    int kofst = getKeyOfs(num);
    if (kofst == -1)
        return nullptr;

    Byte* key = _data + kofst;
    return key;
}

const Byte* BaseBTree::PageWrapper::getKey(UShort num) const
{
    int kofst = getKeyOfs(num);
    if (kofst == -1)
        return nullptr;

    const Byte* key = _data + kofst;
    return key;
}

void BaseBTree::PageWrapper::copyKey(Byte* dst, const Byte* src)
{
    memcpy(
        dst,
        src,
        _tree->getRecSize());
}

void BaseBTree::PageWrapper::copyKeys(Byte* dst, const Byte* src, UShort num)
{
    memcpy(
        dst,
        src,
        _tree->getRecSize() * num
        );

}

void BaseBTree::PageWrapper::copyCursors(Byte* dst, const Byte* src, UShort num)
{
    memcpy(
        dst,
        src,
        num * CURSOR_SZ
        );
}

UInt BaseBTree::PageWrapper::getCursor(UShort cnum)
{
    int curOfs = getCursorOfs(cnum);
    if (curOfs == -1)
        throw std::invalid_argument("Wrong cursor number");

    return *((const UInt*)(_data + curOfs));
}

Byte* BaseBTree::PageWrapper::getCursorPtr(UShort cnum)
{
    int curOfs = getCursorOfs(cnum);
    if (curOfs == -1)
        throw std::invalid_argument("Wrong cursor number");

    return (_data + curOfs);

}

void BaseBTree::PageWrapper::setCursor(UShort cnum, UInt cval)
{
    int curOfs = getCursorOfs(cnum);
    if (curOfs == -1)
        throw std::invalid_argument("Wrong cursor number");

    *((UInt*)(_data + curOfs)) = cval;
}

int BaseBTree::PageWrapper::getCursorOfs(UShort cnum) const
{
    if (cnum > getKeysNum())
        return -1;

    return _tree->getCursorsOfs() + CURSOR_SZ * cnum;
}

int BaseBTree::PageWrapper::getKeyOfs(UShort num) const
{
    if (num >= getKeysNum())
        return -1;

    return KEYS_OFS + _tree->getRecSize() * num;
}

void BaseBTree::PageWrapper::setAsRoot(bool writeFlag /* = true */)
{
    _tree->_rootPageNum = _pageNum;

    if (!writeFlag)
        return;

    if (_pageNum == 0)
        throw std::runtime_error("Can't set a page as root until allocate a page in a file");

    _tree->setRootPageNum(_pageNum, true);
}

void BaseBTree::PageWrapper::readPageFromChild(PageWrapper& pw, UShort chNum)
{
    UInt cur = pw.getCursor(chNum);
    if (cur == 0)
    {
        throw std::invalid_argument("Cursor does not point to a existing node/page");
    }
    
    readPage(cur);
}

void BaseBTree::PageWrapper::writePage()
{
    if (getPageNum() == 0)
        throw std::runtime_error("Page number not set. Can't write");

    _tree->writePage(getPageNum(), _data);
}

void BaseBTree::PageWrapper::splitChild(UShort iChild)
{
    PageWrapper leftChild(_tree);
    PageWrapper rightChild(_tree);
    _tree->splitChild(*this, iChild, leftChild, rightChild);
}

void BaseBPlusTree::splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild)
{
    if (node.isFull())
        throw std::domain_error("A parent node is full, so its child can't be splitted");

    if (iChild > node.getKeysNum())
        throw std::invalid_argument("Cursor not exists");

    if (leftChild.getPageNum() == 0)
        leftChild.readPageFromChild(node, iChild);

    if (!leftChild.isLeaf())
    {
        BaseBTree::splitChild(node, iChild, leftChild, rightChild);
        return;
    }

    // Allocating page for the right child which is new created child.
    rightChild.allocPage(getMinLeafKeys(), leftChild.isLeaf());

    // Copying keys and cursors from the right half of the left child to the right child.
    rightChild.copyKeys(rightChild.getKey(0), leftChild.getKey(getMinLeafKeys()), getMinLeafKeys());

    // Increasing number of keys num in the parent node for inserting median.
    UShort keysNum = node.getKeysNum() + 1;
    node.setKeyNum(keysNum);

    // Shifting coursors in the parent to the right.
    for(int j = keysNum - 1; j > iChild; --j)
        node.copyCursors(node.getCursorPtr(j + 1), node.getCursorPtr(j), 1);

    // Setting coursor in the parent to the right child.
    node.setCursor(iChild + 1, rightChild.getPageNum());

    // Shifting coursors in the parent to the right.
    for(int j = keysNum - 2; j >= iChild; --j)
        node.copyKey(node.getKey(j + 1), node.getKey(j));

    // Copying median to the parent.
    node.copyKey(node.getKey(iChild), leftChild.getKey(getMinLeafKeys() - 1));
    leftChild.setKeyNum(getMinLeafKeys());

    // Writing the pages to the disk.
    leftChild.writePage();
    rightChild.writePage();
    node.writePage();
}

Byte* BaseBPlusTree::search(const Byte* k, BaseBTree::PageWrapper& currentPage, UInt currentDepth)
{
    int i;
    UShort keysNum = currentPage.getKeysNum();
    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    if(currentPage.isLeaf())
    {
        // If the key has been found.
        if(i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize))
        {
            Byte* result = new Byte[_recSize];
            currentPage.copyKey(result, currentPage.getKey(i));
            return result;
        }
        else // If the key has not been found and the current page is a leaf, there is not the key in the tree.
            return nullptr;
    }
    else // If the current page is not a leaf.
    {
        PageWrapper nextPage(this);
        nextPage.readPageFromChild(currentPage, i);
        return search(k, nextPage, currentDepth + 1);
    }
}

int BaseBPlusTree::searchAll(const Byte* k, std::list<Byte*>& keys, BaseBTree::PageWrapper& currentPage,
                UInt currentDepth)
{
    if(currentDepth > _maxSearchDepth)
        _maxSearchDepth = currentDepth;

    int amount = 0;
    int i;
    UShort keysNum = currentPage.getKeysNum();
    bool isLeaf = currentPage.isLeaf();

    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    int first = i;

    PageWrapper nextPage(this);
    for( ; i < keysNum && (i == first || _comparator->isEqual(k, currentPage.getKey(i), _recSize)); ++i)
    {
        if(isLeaf && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) // If the key has been found and
                                                                               // and the current page is a leaf.
        {
            Byte* result = new Byte[_recSize];
            currentPage.copyKey(result, currentPage.getKey(i));
            keys.push_back(result);
            ++amount;
        }

        if(!isLeaf) // If the current page is not a leaf.
        {
            nextPage.readPageFromChild(currentPage, i);
            amount += searchAll(k, keys, nextPage, currentDepth + 1);
        }
    }

    if(!isLeaf) // If the current page is not a leaf.
    {
        nextPage.readPageFromChild(currentPage, i);
        amount += searchAll(k, keys, nextPage, currentDepth + 1);
    }

    return amount;
}

bool BaseBPlusTree::remove(const Byte* k, BaseBTree::PageWrapper& currentPage)
{
    int i;
    UShort keysNum = currentPage.getKeysNum();
    for(i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;

    if(currentPage.isLeaf())
    {
        // If the key has been found.
        if(i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize))
        {
            UShort keysNum = currentPage.getKeysNum();

            for (int j = i; j < keysNum - 1; ++j)
                currentPage.copyKey(currentPage.getKey(j), currentPage.getKey(j + 1));

            currentPage.setKeyNum(keysNum - 1);

            currentPage.writePage();

            return true;
        }
        else // If the key has not been found and the current page is a leaf, there is not the key in the tree.
            return false;
    }
    else // If the current page is not a leaf.
    {
        PageWrapper nextPage(this);
        nextPage.readPageFromChild(currentPage, i);
        if (!nextPage.isRoot() && nextPage.isLeaf() && nextPage.getKeysNum() == getMinLeafKeys())
        {
            PageWrapper leftSibling(this);
            PageWrapper rightSibling(this);

            if (i > 0)
            {
                leftSibling.readPageFromChild(currentPage, i - 1);
                if (leftSibling.getKeysNum() > getMinLeafKeys())
                {
                    nextPage.setKeyNum(getMinLeafKeys() + 1);
                    for (int j = getMinLeafKeys(); j > 0; --j)
                        currentPage.copyKey(nextPage.getKey(j), nextPage.getKey(j - 1));

                    currentPage.copyKey(nextPage.getKey(0), leftSibling.getKey(leftSibling.getKeysNum() - 1));
                    leftSibling.setKeyNum(leftSibling.getKeysNum() - 1);

                    leftSibling.writePage();
                    nextPage.writePage();

                    return remove(k, nextPage);
                }
            }

            if (i < keysNum)
            {
                rightSibling.readPageFromChild(currentPage, i + 1);
                if (rightSibling.getKeysNum() > getMinLeafKeys())
                {
                    nextPage.setKeyNum(getMinLeafKeys() + 1);

                    currentPage.copyKey(nextPage.getKey(getMinLeafKeys()), rightSibling.getKey(0));
                    currentPage.copyKey(currentPage.getKey(i), rightSibling.getKey(0));
                    int rightSiblingKeysNum = rightSibling.getKeysNum();
                    for (int j = 0; j < rightSiblingKeysNum - 1; ++j)
                        currentPage.copyKey(rightSibling.getKey(j), rightSibling.getKey(j + 1));
                    rightSibling.setKeyNum(rightSiblingKeysNum - 1);

                    nextPage.writePage();
                    rightSibling.writePage();

                    return remove(k, nextPage);
                }
            }

            if (i > 0)
            {
                if (leftSibling.getKeysNum() == getMinLeafKeys())
                {
                    mergeChildren(leftSibling, nextPage, currentPage, i - 1);
                    return remove(k, leftSibling);
                }
            }

            mergeChildren(nextPage, rightSibling, currentPage, i);
        }

        return nextPage.isRoot() ? remove(k, _rootPage) : remove(k, nextPage);
    }
}

int BaseBPlusTree::removeAll(const Byte* k, BaseBTree::PageWrapper& currentPage)
{
//    if (currentPage.getKeysNum() == 0)
//        return 0;
//
//    int amount = 0;
//    int i;
//    UShort keysNum = currentPage.getKeysNum();
//    bool isLeaf = currentPage.isLeaf();
//    bool isRoot = currentPage.isRoot();
//
//    PageWrapper child(this);
//    PageWrapper leftNeighbour(this);
//    PageWrapper rightNeighbour(this);
//
//    for (i = 0; i < keysNum && _comparator->compare(currentPage.getKey(i), k, _recSize); ++i) ;
//
//    int first = i;
//
//    for ( ; i <= keysNum && (i == first ||
//            (i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) ||
//            (i > first && _comparator->isEqual(k, currentPage.getKey(i - 1), _recSize))); ++i)
//    {
//        if (currentPage.isLeaf())
//        {
//            if (i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize) &&
//                    (currentPage.isRoot() || currentPage.getKeysNum() > getMinLeafKeys()))
//            {
//                remove(k, currentPage);
//                ++amount;
//                --i;
//            }
//            else
//                return amount;
//        }
//        else
//        {
//            PageWrapper nextPage(this);
//            nextPage.readPageFromChild(currentPage, i);
//            int nextPageAmount = 0;
//            do
//            {
//                nextPageAmount = removeAll(k, nextPage);
//                amount += nextPageAmount;
//                if (nextPage.isLeaf() && nextPage.getKeysNum() == getMinLeafKeys())
//                {
//                    remove(k, currentPage);
//
//                    if (isRoot)
//                    {
//                        if (currentPage.getData() != _rootPage.getData())
//                            loadRootPage();
//
//                        continue;
//                    }
//
//                    break;
//                }
//            }
//            while (nextPageAmount != 0);
//        }
//    }
//
//    if(isRoot && currentPage.getData() != _rootPage.getData())
//        loadRootPage();
//
//    return amount;

    // TODO: make effectively.

    int amount = 0;

    while (BaseBTree::remove(k))
        ++amount;

    return amount;
}

void BaseBPlusTree::mergeChildren(BaseBTree::PageWrapper& leftChild, BaseBTree::PageWrapper& rightChild,
        BaseBTree::PageWrapper& currentPage, UShort medianNum)
{
    if (!leftChild.isLeaf() || !rightChild.isLeaf())
        throw std::invalid_argument("In B+tree only leafs merging is allowed");

    UShort keysNum = currentPage.getKeysNum();

    leftChild.setKeyNum(getMaxLeafKeys());

    // Moving keys from the right child to the left child.
    leftChild.copyKeys(leftChild.getKey(getMinLeafKeys()), rightChild.getKey(0), getMinLeafKeys());

    // Shifting keys in the current page to the left, after removing the median from the current page.
    for(int j = medianNum; j < keysNum - 1; ++j)
        currentPage.copyKey(currentPage.getKey(j), currentPage.getKey(j + 1));

    leftChild.writePage();

    if(currentPage.getKeysNum() == 1) // In the case when the current page was root and becomes empty.
    {
        leftChild.setAsRoot(true);

#ifdef BTREE_WITH_REUSING_FREE_PAGES

        // Making the page free for reusing.
        markPageFree(currentPage.getPageNum());

#endif

        loadRootPage();
    }
    else
    {
        currentPage.setKeyNum(keysNum - 1);
        currentPage.writePage();
    }


#ifdef BTREE_WITH_REUSING_FREE_PAGES

    // Making the page free for reusing.
    markPageFree(rightChild.getPageNum());

#endif
}

void BaseBPlusTree::setOrder(UShort order, UShort recSize)
{
    _order = order;
    _recSize = recSize;

    _minKeys = _order - 1;
    _maxKeys = 2 * _order - 1;

    _minLeafKeys = _minKeys + 1;
    _maxLeafKeys = _maxKeys + 1;

    if (_maxLeafKeys > MAX_KEYS_NUM)
        throw std::invalid_argument("For a given B+-tree order, there is an excess of the maximum number of keys");

    _keysSize = _recSize * _maxLeafKeys;
    _cursorsOfs = _keysSize + KEYS_OFS;
    _nodePageSize = _cursorsOfs + CURSOR_SZ * (_maxLeafKeys + 1);

    reallocWorkPages();
}

bool BaseBPlusTree::isFull(const PageWrapper& page) const
{
    return (!page.isLeaf() && BaseBTree::isFull(page)) || (page.isLeaf() && page.getKeysNum() == getMaxLeafKeys());
}

//==============================================================================
// class BaseBStarTree
//==============================================================================

void BaseBStarTree::insert(const Byte* k)
{
    if(_rootPage.isFull())
    {
        UInt prevRootPageNum = _rootPageNum;
        _rootPage.allocNewRootPage();
        _rootPage.setCursor(0, prevRootPageNum);
        _rootPage.setAsRoot(true);
        _rootPage.splitChild(0);
    }
    insertNonFull(k, _rootPage);
}

void BaseBStarTree::insertNonFull(const Byte* k, PageWrapper& currentNode)
{
    if (currentNode.isFull())
        throw std::domain_error("Node is full. Can't insert");

    IComparator* c = getComparator();
    if (!c)
        throw std::runtime_error("Comparator not set. Can't insert");

    // Getting the current pages' keys number (it will be used several times in this method).
    UShort keysNum = currentNode.getKeysNum();

    int i = keysNum - 1;

    // If the current page is leaf, we can simply insert the key into the page.
    if(currentNode.isLeaf())
    {
        // Increasing number of keys in the node for inserting the key.
        currentNode.setKeyNum(keysNum + 1);

        // Shifting keys in the node to the right.
        for( ; i >= 0 && c->compare(k, currentNode.getKey(i), getRecSize()); --i)
            currentNode.copyKey(currentNode.getKey(i + 1), currentNode.getKey(i));

        // Inserting the key to the node.
        currentNode.copyKey(currentNode.getKey(i + 1), k);

        // Writing the page to the disk.
        currentNode.writePage();
    }
    else
    {
        // Searching necessary child.
        for( ; i >= 0 && c->compare(k, currentNode.getKey(i), getRecSize()); --i) ;
        ++i;

        // Reading the child from disk.
        PageWrapper child(this);
        child.readPageFromChild(currentNode, i);

        if(child.isFull())
        {
//            UShort childKeysNum = child.getKeysNum();
//            bool isChildLeaf = child.isLeaf();

            PageWrapper leftSibling(this);
            PageWrapper rightSibling(this);

            if (i > 0)
            {
                leftSibling.readPageFromChild(currentNode, i - 1);
                UShort leftSiblingKeysNum = leftSibling.getKeysNum();

                if (!leftSibling.isFull())
                {
//                    UShort sum = childKeysNum + leftSiblingKeysNum;
//                    UShort newLeftSiblingKeysNum = sum / 2 + (sum % 2 == 1 ? 1 : 0);
//                    UShort movedKeys = newLeftSiblingKeysNum - leftSiblingKeysNum;
//                    UShort childLeftKeys = childKeysNum - movedKeys;
//
//                    int insertionPlace = -1;
//
//                    for (int j = 0; j < leftSiblingKeysNum; ++j)
//                    {
//                        if (c->compare(leftSibling.getKey(j), k, getRecSize()))
//                            insertionPlace = j;
//                    }
//
//                    if (insertionPlace == -1 && c->compare(currentNode.getKey(i), currentNode.getKey(i - 1),
//                            getRecSize()))
//                        insertionPlace = leftSiblingKeysNum;
//
//                    if (insertionPlace == -1)
//                    {
//                        for (int j = 0; j < childKeysNum; ++j)
//                            insertionPlace = leftSiblingKeysNum + j;
//                    }
//
//                    if (insertionPlace == -1)
//                        insertionPlace = leftSiblingKeysNum + childKeysNum;
//
//                    leftSibling.setKeyNum(newLeftSiblingKeysNum);
//
//                    currentNode.copyKey(leftSibling.getKey(leftSiblingKeysNum), currentNode.getKey(i - 1));
//                    currentNode.copyKeys(leftSibling.getKey(leftSiblingKeysNum + 1), child.getKey(0),
//                            movedKeys - 1);
//                    currentNode.copyKey(currentNode.getKey(i - 1),
//                            child.getKey(movedKeys - 1));
//
//                    if (!isChildLeaf)
//                        currentNode.copyCursors(leftSibling.getCursorPtr(leftSiblingKeysNum + 1), child.getCursorPtr(0),
//                                movedKeys);
//
//                    for (int j = 0; j < childLeftKeys; ++j)
//                        currentNode.copyKey(child.getKey(j), child.getKey(j + movedKeys));
//
//                    if (!isChildLeaf)
//                    {
//                        for (int j = 0; j <= childLeftKeys; ++j)
//                            currentNode.copyCursors(child.getCursorPtr(j),
//                                    child.getCursorPtr(j + movedKeys), 1);
//                    }
//
//                    child.setKeyNum(childLeftKeys);
//
//                    leftSibling.writePage();
//                    child.writePage();
//                    currentNode.writePage();

                    shareKeysWithLeftChildAndInsert(k, currentNode, i, child, leftSibling);
                    return;
                }
            }

            if (i < keysNum)
            {
                rightSibling.readPageFromChild(currentNode, i + 1);
                UShort rightSiblingKeysNum = rightSibling.getKeysNum();

                if (!rightSibling.isFull())
                {
//                    UShort sum = childKeysNum + rightSiblingKeysNum;
//                    UShort newRightSiblingKeysNum = sum / 2 + (sum % 2 == 1 ? 1 : 0);
//                    UShort movedKeys = newRightSiblingKeysNum - rightSiblingKeysNum;
//                    UShort childLeftKeys = childKeysNum - movedKeys;
//
//                    rightSibling.setKeyNum(newRightSiblingKeysNum);
//
//                    for (int j = newRightSiblingKeysNum - 1; j >= movedKeys; --j)
//                        currentNode.copyKey(rightSibling.getKey(j), rightSibling.getKey(j - movedKeys));
//
//                    if (!isChildLeaf)
//                    {
//                        for (int j = newRightSiblingKeysNum; j >= movedKeys; --j)
//                            currentNode.copyCursors(rightSibling.getCursorPtr(j),
//                                    rightSibling.getCursorPtr(j - movedKeys), 1);
//                    }
//
//                    currentNode.copyKey(rightSibling.getKey(movedKeys - 1), currentNode.getKey(i));
//                    currentNode.copyKeys(rightSibling.getKey(0), child.getKey(childLeftKeys + 1), movedKeys - 1);
//                    if (!isChildLeaf)
//                        currentNode.copyCursors(rightSibling.getCursorPtr(0), child.getCursorPtr(childLeftKeys + 1),
//                                movedKeys);
//                    currentNode.copyKey(currentNode.getKey(i), child.getKey(childLeftKeys));
//
//                    child.setKeyNum(childLeftKeys);
//
//                    child.writePage();
//                    rightSibling.writePage();
//                    currentNode.writePage();

                    shareKeysWithRightChildAndInsert(k, currentNode, i, child, rightSibling);
                    return;
                }
            }

            PageWrapper middle(this);

            if (i > 0)
            {
                splitChildren(currentNode, i - 1, leftSibling, middle, child);
                if (c->compare(currentNode.getKey(i), k, getRecSize()))
                    insertNonFull(k, child);
                else if (c->compare(currentNode.getKey(i - 1), k, getRecSize()))
                    insertNonFull(k, middle);
                else
                    insertNonFull(k, leftSibling);
            }
            else
            {
                splitChildren(currentNode, i, child, middle, rightSibling);
                if (c->compare(currentNode.getKey(i + 1), k, getRecSize()))
                    insertNonFull(k, rightSibling);
                else if (c->compare(currentNode.getKey(i), k, getRecSize()))
                    insertNonFull(k, middle);
                else
                    insertNonFull(k, child);
            }
        }
        else // If the child is not full, we can simply recursively call the method on it.
            insertNonFull(k, child);
    }
}

void BaseBStarTree::splitChild(PageWrapper& node, UShort iChild, PageWrapper& leftChild, PageWrapper& rightChild)
{
    if (node.isFull())
        throw std::domain_error("A parent node is full, so its child can't be splitted");

    if (iChild > node.getKeysNum())
        throw std::invalid_argument("Cursor not exists");

    if (leftChild.getPageNum() == 0)
        leftChild.readPageFromChild(node, iChild);

    UShort rightChildKeysNum = leftChild.getKeysNum() / 2;
    UShort leftChildKeysNum = leftChild.getKeysNum() - rightChildKeysNum - 1;

    // Allocating page for the right child which is new created child.
    rightChild.allocPage(rightChildKeysNum, leftChild.isLeaf());

    // Copying keys and cursors from the right half of the left child to the right child.
    rightChild.copyKeys(rightChild.getKey(0), leftChild.getKey(leftChildKeysNum + 1), rightChildKeysNum);
    if (!leftChild.isLeaf())
        node.copyCursors(rightChild.getCursorPtr(0), leftChild.getCursorPtr(leftChildKeysNum + 1),
                rightChildKeysNum + 1);

    // Increasing number of keys num in the parent node for inserting median.
    UShort keysNum = node.getKeysNum() + 1;
    node.setKeyNum(keysNum);

    // Shifting coursors in the parent to the right.
    for(int j = keysNum - 1; j > iChild; --j)
        node.copyCursors(node.getCursorPtr(j + 1), node.getCursorPtr(j), 1);

    // Setting coursor in the parent to the right child.
    node.setCursor(iChild + 1, rightChild.getPageNum());

    // Shifting keys in the parent to the right.
    for(int j = keysNum - 2; j >= iChild; --j)
        node.copyKey(node.getKey(j + 1), node.getKey(j));

    // Moving median to the parent.
    node.copyKey(node.getKey(iChild), leftChild.getKey(leftChildKeysNum));
    leftChild.setKeyNum(leftChildKeysNum);

    // Writing the pages to the disk.
    leftChild.writePage();
    rightChild.writePage();
    node.writePage();
}

void BaseBStarTree::splitChildren(PageWrapper& node, UShort iLeft,
        PageWrapper& left, PageWrapper& middle, PageWrapper& right)
{
    if (node.isFull())
        throw std::domain_error("A parent node is full, so its children can't be splitted");

    if (iLeft >= node.getKeysNum())
        throw std::invalid_argument("Left and/or cursors do not exist");

    if (left.getPageNum() == 0)
        left.readPageFromChild(node, iLeft);

    if (right.getPageNum() == 0)
        right.readPageFromChild(node, iLeft + 1);

    bool isLeaf = left.isLeaf();

    UShort iRight = iLeft + 1;

    middle.allocPage(getMiddleSplitProductKeys(), isLeaf);

    UShort keysNum = left.getKeysNum() + right.getKeysNum() + 1;
    Byte* keys = new Byte[keysNum * _recSize];
    Byte* cursors = isLeaf ? nullptr : new Byte[(keysNum + 1) * CURSOR_SZ];

    node.copyKeys(&keys[0], left.getKey(0), left.getKeysNum());
    if (!isLeaf)
        node.copyCursors(&cursors[0], left.getCursorPtr(0), left.getKeysNum() + 1);

    node.copyKey(&keys[left.getKeysNum() * _recSize], node.getKey(iLeft));

    node.copyKeys(&keys[(left.getKeysNum() + 1) * _recSize], right.getKey(0), right.getKeysNum());
    if (!isLeaf)
        node.copyCursors(&cursors[(left.getKeysNum() + 1) * CURSOR_SZ], right.getCursorPtr(0), right.getKeysNum() + 1);

//    int i;
//    for (i = 0; i < keysNum - 1 && _comparator->compare(&keys[i], k, _recSize); ++i) ;
//
//    for (int j = keysNum - 1; j > i; ++j)
//        node.copyKey(&keys[j], &keys[j - 1]);
//
//    node.copyKey(&keys[i], k);

    left.setKeyNum(getLeftSplitProductKeys());

    node.copyKeys(left.getKey(0), &keys[0], getLeftSplitProductKeys());
    if (!isLeaf)
        node.copyCursors(left.getCursorPtr(0), &cursors[0], getLeftSplitProductKeys() + 1);

    node.copyKeys(middle.getKey(0), &keys[(getLeftSplitProductKeys() + 1) * _recSize], getMiddleSplitProductKeys());
    if (!isLeaf)
        node.copyCursors(middle.getCursorPtr(0), &cursors[(getLeftSplitProductKeys() + 1) * CURSOR_SZ],
                getMiddleSplitProductKeys() + 1);

    right.setKeyNum(getRightSplitProductKeys());
    node.copyKeys(right.getKey(0), &keys[(getLeftSplitProductKeys() + getMiddleSplitProductKeys() + 2) * _recSize],
            getRightSplitProductKeys());
    if (!isLeaf)
        node.copyCursors(right.getCursorPtr(0),
                &cursors[(getLeftSplitProductKeys() + getMiddleSplitProductKeys() + 2) * CURSOR_SZ],
                getRightSplitProductKeys() + 1);

    node.copyKey(node.getKey(iLeft), &keys[getLeftSplitProductKeys() * _recSize]);

    UShort parentKeysNum = node.getKeysNum() + 1;
    node.setKeyNum(parentKeysNum);

    for (int i = parentKeysNum - 1; i > iLeft; --i)
        node.copyKey(node.getKey(i), node.getKey(i - 1));

    node.copyKey(node.getKey(iRight), &keys[(getLeftSplitProductKeys() + getMiddleSplitProductKeys() + 1) * _recSize]);

    for (int i = parentKeysNum; i > iRight; --i)
        node.copyCursors(node.getCursorPtr(i), node.getCursorPtr(i - 1), 1);

    node.setCursor(iRight, middle.getPageNum());

    left.writePage();
    middle.writePage();
    right.writePage();
    node.writePage();

    delete[] keys;
    if (cursors != nullptr)
        delete[] cursors;
}

void BaseBStarTree::shareKeysWithLeftChildAndInsert(const Byte* k, PageWrapper& node, UShort iChild,
        PageWrapper& child, PageWrapper& left)
{
    if (!child.isFull())
        throw std::invalid_argument("Child that shares keys should be full");

    if (left.isFull())
        throw std::invalid_argument("Left sibling should not be full");

    if (iChild > node.getKeysNum())
        throw std::invalid_argument("Cursor not exists");

    IComparator* c = getComparator();
    if (!c)
        throw std::runtime_error("Comparator not set. Can't insert");

    bool isChildLeaf = child.isLeaf();

    UShort childKeysNum = child.getKeysNum();
    UShort leftSiblingKeysNum = left.getKeysNum();

    UShort sum = childKeysNum + leftSiblingKeysNum;
    UShort newLeftSiblingKeysNum = sum / 2 + (sum % 2 == 1 ? 1 : 0);
    UShort movedKeys = newLeftSiblingKeysNum - leftSiblingKeysNum;
    UShort childLeftKeys = childKeysNum - movedKeys;

    if (newLeftSiblingKeysNum == getMaxKeys() && movedKeys == 1 && c->compare(k, child.getKey(0), _recSize))
    {
        insertNonFull(node.getKey(iChild - 1), left);
        node.copyKey(node.getKey(iChild - 1), k);
        return;
    }

//    int insertionPlace = -1;
//
//    for (int j = 0; j < leftSiblingKeysNum; ++j)
//    {
//        if (c->compare(left.getKey(j), k, getRecSize()))
//            insertionPlace = j;
//    }
//
//    if (insertionPlace == -1 && c->compare(node.getKey(iChild), node.getKey(iChild - 1), getRecSize()))
//        insertionPlace = leftSiblingKeysNum;
//
//    if (insertionPlace == -1)
//    {
//        for (int j = 0; j < childKeysNum; ++j)
//            insertionPlace = leftSiblingKeysNum + j;
//    }
//
//    if (insertionPlace == -1)
//        insertionPlace = leftSiblingKeysNum + childKeysNum;

    left.setKeyNum(newLeftSiblingKeysNum);

//    if (insertionPlace == 0)
//    {
//        node.copyKey(left.getKey(leftSiblingKeysNum), k);
//        node.copyKey(left.getKey(leftSiblingKeysNum + 1), node.getKey(iChild - 1));
//        node.copyKeys(left.getKey(leftSiblingKeysNum + 2), )
//    }
//    else
//        node.copyKey(left.getKey(leftSiblingKeysNum), node.getKey(iChild - 1));

    node.copyKey(left.getKey(leftSiblingKeysNum), node.getKey(iChild - 1));
    node.copyKeys(left.getKey(leftSiblingKeysNum + 1), child.getKey(0), movedKeys - 1);
    node.copyKey(node.getKey(iChild - 1), child.getKey(movedKeys - 1));

    if (!isChildLeaf)
        node.copyCursors(left.getCursorPtr(leftSiblingKeysNum + 1), child.getCursorPtr(0), movedKeys);

    for (int j = 0; j < childLeftKeys; ++j)
        node.copyKey(child.getKey(j), child.getKey(j + movedKeys));

    if (!isChildLeaf)
    {
        for (int j = 0; j <= childLeftKeys; ++j)
            node.copyCursors(child.getCursorPtr(j), child.getCursorPtr(j + movedKeys), 1);
    }

    child.setKeyNum(childLeftKeys);

    left.writePage();
    child.writePage();
    node.writePage();

    if (c->compare(k, node.getKey(iChild - 1), getRecSize()))
        insertNonFull(k, left);
    else
        insertNonFull(k, child);
}

void BaseBStarTree::shareKeysWithRightChildAndInsert(const Byte* k, PageWrapper& node, UShort iChild,
        PageWrapper& child, PageWrapper& right)
{
    if (!child.isFull())
        throw std::invalid_argument("Child that shares keys should be full");

    if (right.isFull())
        throw std::invalid_argument("Right sibling should not be full");

    if (iChild >= node.getKeysNum())
        throw std::invalid_argument("Cursor and/or right sibling not exists");

    IComparator* c = getComparator();
    if (!c)
        throw std::runtime_error("Comparator not set. Can't insert");

    bool isChildLeaf = child.isLeaf();

    UShort childKeysNum = child.getKeysNum();
    UShort rightSiblingKeysNum = right.getKeysNum();

    UShort sum = childKeysNum + rightSiblingKeysNum;
    UShort newRightSiblingKeysNum = sum / 2 + (sum % 2 == 1 ? 1 : 0);
    UShort movedKeys = newRightSiblingKeysNum - rightSiblingKeysNum;
    UShort childLeftKeys = childKeysNum - movedKeys;

    if (newRightSiblingKeysNum == getMaxKeys() && movedKeys == 1
            && c->compare(child.getKey(childKeysNum - 1), k, _recSize))
    {
        insertNonFull(node.getKey(iChild), right);
        node.copyKey(node.getKey(iChild), k);
        return;
    }

    right.setKeyNum(newRightSiblingKeysNum);

    for (int j = newRightSiblingKeysNum - 1; j >= movedKeys; --j)
        node.copyKey(right.getKey(j), right.getKey(j - movedKeys));

    if (!isChildLeaf)
    {
        for (int j = newRightSiblingKeysNum; j >= movedKeys; --j)
            node.copyCursors(right.getCursorPtr(j), right.getCursorPtr(j - movedKeys), 1);
    }

    node.copyKey(right.getKey(movedKeys - 1), node.getKey(iChild));
    node.copyKeys(right.getKey(0), child.getKey(childLeftKeys + 1), movedKeys - 1);
    if (!isChildLeaf)
        node.copyCursors(right.getCursorPtr(0), child.getCursorPtr(childLeftKeys + 1), movedKeys);
    node.copyKey(node.getKey(iChild), child.getKey(childLeftKeys));

    child.setKeyNum(childLeftKeys);

    child.writePage();
    right.writePage();
    node.writePage();

    if (c->compare(node.getKey(iChild), k, getRecSize()))
        insertNonFull(k, right);
    else
        insertNonFull(k, child);
}

void BaseBStarTree::setOrder(UShort order, UShort recSize)
{
    _order = order;
    _recSize = recSize;

    _minKeys = (2 * _order - 2) / 3;
    _maxKeys = _order;

    _maxRootKeys = 2 * _minKeys;

    _leftSplitProductKeys = (2 * _order - 1) / 3;
    _middleSplitProductKeys = 2 * _order / 3;
    _rightSplitProductKeys = (2 * _order + 1) / 3;

    UInt maxPossibleNodeKeys = std::max(_maxKeys, _maxRootKeys);

    if (maxPossibleNodeKeys > MAX_KEYS_NUM)
        throw std::invalid_argument("For a given B*-tree order, there is an excess of the maximum number of keys");

    _keysSize = _recSize * maxPossibleNodeKeys;
    _cursorsOfs = _keysSize + KEYS_OFS;
    _nodePageSize = _cursorsOfs + CURSOR_SZ * (maxPossibleNodeKeys + 1);

    reallocWorkPages();
}

bool BaseBStarTree::isFull(const PageWrapper& page) const
{
    return (!page.isRoot() && BaseBTree::isFull(page)) || (page.isRoot() && page.getKeysNum() == getMaxRootKeys());
}

//==============================================================================
// class FileBaseBTree
//==============================================================================

FileBaseBTree::FileBaseBTree(BaseBTree::TreeType treeType, UShort order, UShort recSize, BaseBTree::IComparator* comparator,
    const std::string& fileName)
    : FileBaseBTree(treeType)
{
    _tree->setComparator(comparator);

    checkTreeParams(order, recSize);
    createInternal(order, recSize, fileName);
}

FileBaseBTree::FileBaseBTree(BaseBTree::TreeType treeType, const std::string& fileName, BaseBTree::IComparator* comparator)
    : FileBaseBTree(treeType)
{
    _tree->setComparator(comparator);
    loadInternal(fileName);
}

FileBaseBTree::FileBaseBTree(BaseBTree::TreeType treeType)
{
    switch (treeType)
    {
        case BaseBTree::TreeType::B_TREE: _tree = new BaseBTree(0, 0, nullptr, nullptr); break;
        case BaseBTree::TreeType::B_PLUS_TREE: _tree = new BaseBPlusTree(0, 0, nullptr, nullptr); break;
        case BaseBTree::TreeType::B_STAR_TREE: _tree = new BaseBStarTree(0, 0, nullptr, nullptr); break;
    }

    isComposition = true;
}

FileBaseBTree::~FileBaseBTree()
{
    close();

    if (isComposition)
        delete _tree;
}

void FileBaseBTree::create(UShort order, UShort recSize,
    const std::string& fileName)
{
    if (isOpen())
        throw std::runtime_error("B-tree file is already open");

    checkTreeParams(order, recSize);
    createInternal(order, recSize, fileName);
}

void FileBaseBTree::createInternal(UShort order, UShort recSize,
    const std::string& fileName)
{
    _fileStream.open(fileName, 
        std::fstream::in | std::fstream::out |
        std::fstream::trunc |
        std::fstream::binary);

    if (_fileStream.fail())
    {
        _fileStream.close();
        throw std::runtime_error("Can't open file for writing");
    }

    _fileName = fileName;
    _tree->setStream(&_fileStream);

    _tree->createTree(order, recSize);
}

void FileBaseBTree::open(const std::string& fileName)
{
    if (isOpen())
        throw std::runtime_error("Tree file is already open");

    loadInternal(fileName);
}

void FileBaseBTree::loadInternal(const std::string& fileName)
{
    _fileStream.open(fileName,
        std::fstream::in | std::fstream::out |
        std::fstream::binary);

    if (_fileStream.fail())
    {
        _fileStream.close();
        throw std::runtime_error("Can't open file for reading");
    }

    _fileName = fileName;
    _tree->setStream(&_fileStream);


    try {
        _tree->loadTree();
    }
    catch (std::exception& e)
    {
        _fileStream.close();
        throw e;
    }
    catch (...)
    {
        _fileStream.close();
        throw std::runtime_error("Error when loading btree");
    }
}

void FileBaseBTree::close()
{
    if (!isOpen())
        return;

    closeInternal();
}

void FileBaseBTree::closeInternal()
{
    _fileStream.close();
    _tree->resetBTree();
}

void FileBaseBTree::checkTreeParams(UShort order, UShort recSize)
{
    if (order < 1 || recSize == 0)
        throw std::invalid_argument("Tree order can't be less than 1 and record siaze can't be 0");

}

bool FileBaseBTree::isOpen() const
{
    return (_fileStream.is_open());
}

} // namespace xi
