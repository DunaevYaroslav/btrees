/// \file
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
    _rootPage.insertNonFull(k);
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
        delete[] _data;

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

void BaseBTree::PageWrapper::insertNonFull(const Byte* k)
{
    if (isFull())
        throw std::domain_error("Node is full. Can't insert");

    IComparator* c = _tree->getComparator();
    if (!c)
        throw std::runtime_error("Comparator not set. Can't insert");

    // Getting the current pages' keys number (it will be used several times in this method).
    UShort keysNum = getKeysNum();

    int i = keysNum - 1;

    // If the current page is leaf, we can simply insert the key into the page.
    if(isLeaf())
    {
        // Increasing number of keys in the node for inserting the key.
        setKeyNum(keysNum + 1);

        // Shifting keys in the node to the right.
        for( ; i >= 0 && c->compare(k, getKey(i), _tree->_recSize); --i)
            copyKey(getKey(i + 1), getKey(i));

        // Inserting the key to the node.
        copyKey(getKey(i + 1), k);

        // Writing the page to the disk.
        writePage();
    }
    else // If the current page is not leaf, we should check if we should split one of the children and recursively call the method on the child.
    {
        // Searching necessary child.
        for( ; i >= 0 && c->compare(k, getKey(i), _tree->_recSize); --i) ;
        ++i;

        // Reading the child from disk.
        PageWrapper child(_tree);
        child.readPageFromChild(*this, i);

        // If the child is full, we should split it and recursively call the method on the child or on the new created child.
        if(child.isFull())
        {
            PageWrapper newChild(_tree);
            _tree->splitChild(*this, i, child, newChild);
            if(c->compare(getKey(i), k, _tree->_recSize))
                newChild.insertNonFull(k);
            else
                child.insertNonFull(k);
        }
        else // If the child is not full, we can simply recursively call the method on it.
            child.insertNonFull(k);
    }
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
        BaseBTree::splitChild(node, iChild, leftChild, rightChild);

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
        if(i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) // Cases 1 and 2: if the key has
                                                                                    // been found, removes it recursively.
        {
            if(currentPage.isRoot())
                return removeByKeyNum(i, _rootPage);
            else
                return removeByKeyNum(i, currentPage);
        }
        else // If the key has not been found and the current page is a leaf, there is not the key in the tree.
            return false;
    }

    // Case 3.
    PageWrapper child(this);
    PageWrapper leftNeighbour(this);
    PageWrapper rightNeighbour(this);
    if(prepareSubtree(i, currentPage, child, leftNeighbour, rightNeighbour))
        return remove(k, leftNeighbour);
    else
        return remove(k, child);
}

int BaseBPlusTree::removeAll(const Byte* k, BaseBTree::PageWrapper& currentPage)
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
        if(isLeaf && i < keysNum && _comparator->isEqual(k, currentPage.getKey(i), _recSize)) // Cases 1 and 2: if the key
                                                                                              // has been found, removes it recursively.
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

void BaseBPlusTree::mergeChildren(BaseBTree::PageWrapper& leftChild, BaseBTree::PageWrapper& rightChild,
        BaseBTree::PageWrapper& currentPage, UShort medianNum)
{
    if(!leftChild.isLeaf())
        BaseBTree::mergeChildren(leftChild, rightChild, currentPage, medianNum);

    UShort keysNum = currentPage.getKeysNum();

    leftChild.setKeyNum(getMaxLeafKeys());

    // Moving keys from the right child to the left child.
    leftChild.copyKeys(leftChild.getKey(getMinLeafKeys()), rightChild.getKey(0), getMinLeafKeys());

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

void BaseBPlusTree::setOrder(UShort order, UShort recSize)
{
    _order = order;
    _recSize = recSize;

    _minKeys = _order - 1;
    _maxKeys = 2 * _order - 1;

    _minLeafKeys = _minKeys + 1;
    _maxLeafKeys = _maxKeys + 1;

    if (_maxKeys > MAX_KEYS_NUM)
        throw std::invalid_argument("For a given B-tree order, there is an excess of the maximum number of keys");

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
//        case BaseBTree::TreeType::B_STAR_TREE: _tree = new BaseBSTree(0, 0, nullptr, nullptr); break;
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
