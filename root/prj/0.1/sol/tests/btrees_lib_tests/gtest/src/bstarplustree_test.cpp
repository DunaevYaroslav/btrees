/// \file
/// \brief     B*+-tree test.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include <gtest-fus/gtest.h>


#include "individual.h"
#include "btree.h"

using namespace xi;

/** \brief Тестовый класс для тестирования открытых интерфейсов B*+-tree. */
class BStarPlusTreeTest : public ::testing::Test {

public:

    /**
     * The B*+-tree's order.
     */
    static const int ORDER = 4;

public:
    //BTreeTest()
    //    : _dumper(DUMP_EVENTLOG_PUB_FN, DUMP_IMGS_PUB_PATH)
    //{
    //}

public:
    //static const int STRUCT2_SEQ[];
    //static const int STRUCT2_SEQ_NUM;
    std::string& getFn(const char* fn)
    {
        _fn = TEST_FILES_PATH;
        _fn.append(fn);
        return _fn;
    }

    void clearKeysList(std::list<Byte*>& keys)
    {
        for(std::list<Byte*>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
            delete[] *iter;

        keys.clear();
    }

protected:
    std::string _fn;        ///< Имя файла
    //RBTreeDefDumper<int, std::less<int>> _dumper;

    ///** \brief Выводить в формате GraphViz. */
    //RBTreeGvDumper<int, std::less<int>> _gvDumper;
}; // class RBTreePubTest

struct ByteComparator : public BaseBTree::IComparator {
    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        if (*lhv < *rhv)
            return true;
        return false;
    }

    // простейшая реализация — побайтное сравнение
    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz; ++i)
            if (*lhv != *rhv)
                return false;

        return true;
    }



};

TEST_F(BStarPlusTreeTest, InsertS1)
{
    std::string& fn = getFn("InsertS1.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    Byte k = 0x03;
    bt.insert(&k);
    Byte* searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    std::list<Byte*> keys;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x02;
    bt.insert(&k);
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x01;
    bt.insert(&k);
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x03;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x02;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x01;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);
}

TEST_F(BStarPlusTreeTest, InsertS2)
{
    std::string& fn = getFn("InsertS2.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    Byte k = 0x03;
    bt.insert(&k);
    Byte* searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    std::list<Byte*> keys;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x02;
    bt.insert(&k);
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x01;
    bt.insert(&k);
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x04;
    bt.insert(&k);
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    Byte kek = *keys.back();
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x03;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x02;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x01;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    k = 0x04;
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);
}


TEST_F(BStarPlusTreeTest, InsertS3)
{
    std::string& fn = getFn("InsertS3.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }
}

TEST_F(BStarPlusTreeTest, InsertS4)
{
    std::string& fn = getFn("InsertS4.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03, 0x06, 0x04, 0x10, 0x08, 0x12 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }
}

TEST_F(BStarPlusTreeTest, InsertS5)
{
    std::string& fn = getFn("InsertS5.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    for (Byte i = 0; i < 100; ++i)
    {
        bt.insert(&i);
        Byte* searched = bt.search(&i);
        EXPECT_TRUE(searched != nullptr);
    }

    for (Byte i = 0; i < 100; ++i)
    {
        Byte* searched = bt.search(&i);
        EXPECT_TRUE(searched != nullptr);
    }
}

TEST_F(BStarPlusTreeTest, InsertS6)
{
    std::string& fn = getFn("InsertS6.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    for (Byte i = 100; i > 0; --i)
    {
        bt.insert(&i);
        Byte* searched = bt.search(&i);
        EXPECT_TRUE(searched != nullptr);
    }

    for (Byte i = 100; i > 0; --i)
    {
        Byte* searched = bt.search(&i);
        EXPECT_TRUE(searched != nullptr);
    }
}

TEST_F(BStarPlusTreeTest, InsertS7)
{
    std::string& fn = getFn("InsertS7.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    for (Byte i = 0; i <= 50; ++i)
    {
        Byte values[] = {i, (Byte) (100 - i)};

        for (Byte j : values)
        {
            for (Byte k = 0; k <= j; ++k)
            {
                bt.insert(&j);
                Byte* searched = bt.search(&j);
                EXPECT_TRUE(searched != nullptr);
            }

            if (j == 50)
                break;
        }
    }

    for (Byte i = 0; i <= 100; ++i)
    {
        std::list<Byte*> keys;

        EXPECT_EQ(i + 1, bt.searchAll(&i, keys));
        EXPECT_EQ(i, *keys.back());

        clearKeysList(keys);
    }
}

TEST_F(BStarPlusTreeTest, InsertS8)
{
    std::string& fn = getFn("InsertS8.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER + 1, 1, &comparator, fn);

    for (Byte i = 0; i <= 50; ++i)
    {
        Byte values[] = {i, (Byte) (100 - i)};

        for (Byte j : values)
        {
            for (Byte k = 0; k <= j; ++k)
            {
                bt.insert(&j);
                Byte* searched = bt.search(&j);
                EXPECT_TRUE(searched != nullptr);
            }

            if (j == 50)
                break;
        }
    }

    for (Byte i = 0; i <= 100; ++i)
    {
        std::list<Byte*> keys;

        EXPECT_EQ(i + 1, bt.searchAll(&i, keys));
        EXPECT_EQ(i, *keys.back());

        clearKeysList(keys);
    }
}

#ifdef BTREE_WITH_REUSING_FREE_PAGES

TEST_F(BStarPlusTreeTest, Reusing1)
{
    std::string& fn = getFn("Reusing1.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);
    BaseBTree::PageWrapper wp(bt.getTree());

    wp.allocPage(3, false);
    EXPECT_EQ(2, wp.getPageNum());

    wp.makePageFree();
    wp.allocPage(3, false);
    EXPECT_EQ(2, wp.getPageNum());

    wp.makePageFree();
}

TEST_F(BStarPlusTreeTest, Reusing2)
{
    std::string& fn = getFn("Reusing2.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);
    BaseBTree::PageWrapper wp(bt.getTree());

    wp.allocPage(3, false);
    EXPECT_EQ(2, wp.getPageNum());

    // Trying to allocate more pages than the free pages count.
    wp.makePageFree();
    wp.allocPage(3, false);
    EXPECT_EQ(2, wp.getPageNum());
    wp.allocPage(3, false);
    EXPECT_EQ(3, wp.getPageNum());

    bt.getTree()->markPageFree(3);
    wp.allocPage(3, false);
    EXPECT_EQ(3, wp.getPageNum());

    wp.makePageFree();
    bt.getTree()->markPageFree(2);
}

TEST_F(BStarPlusTreeTest, Reusing3)
{
    std::string& fn = getFn("Reusing3.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);
    BaseBTree::PageWrapper wp(bt.getTree());

    wp.allocNewRootPage();
    EXPECT_EQ(2, wp.getPageNum());
}

TEST_F(BStarPlusTreeTest, Reusing4)
{
    std::string& fn = getFn("Reusing3.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);
    BaseBTree::PageWrapper wp(bt.getTree());

    wp.allocPage(3, false);
    EXPECT_EQ(2, wp.getPageNum());

    wp.makePageFree();
    wp.allocNewRootPage();
    EXPECT_EQ(2, wp.getPageNum());
}

#endif

#ifdef BTREE_WITH_DELETION

TEST_F(BStarPlusTreeTest, Remove1)
{
    std::string& fn = getFn("Remove1.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        EXPECT_TRUE(bt.remove(&el));

        if(i != 5)
        {
            Byte* searched = bt.search(&el);
            EXPECT_TRUE(searched == nullptr);
            delete[] searched;
        }
    }
}

TEST_F(BStarPlusTreeTest, Remove2)
{
    std::string& fn = getFn("Remove2.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    // Removing in the reversed order.
    for (int i = sizeof(els) / sizeof(els[0]) - 1; i >= 0; --i)
    {
        Byte& el = els[i];
        EXPECT_TRUE(bt.remove(&el));

        if(i != 6)
        {
            Byte* searched = bt.search(&el);
            EXPECT_TRUE(searched == nullptr);
            delete[] searched;
        }
    }
}

TEST_F(BStarPlusTreeTest, Remove3)
{
    std::string& fn = getFn("Remove3.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    Byte& el = els[5];

    EXPECT_EQ(2, bt.removeAll(&el));

    Byte* searched = bt.search(&el);
    EXPECT_TRUE(searched == nullptr);
    delete[] searched;

    for(int i = 0; i < sizeof(els) / sizeof(els[0]) - 2; ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        EXPECT_EQ(1, bt.searchAll(&el, keys));
        EXPECT_EQ(el, *keys.back());
        clearKeysList(keys);
    }
}

TEST_F(BStarPlusTreeTest, Remove4)
{
    std::string& fn = getFn("Remove4.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte k = 0x03;
    bt.insert(&k);
    Byte* searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    std::list<Byte*> keys;
    EXPECT_EQ(1, bt.searchAll(&k, keys));
    EXPECT_EQ(1, keys.size());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    bt.insert(&k);
    searched = bt.search(&k);
    EXPECT_TRUE(searched != nullptr);
    delete[] searched;
    EXPECT_EQ(2, bt.searchAll(&k, keys));
    EXPECT_EQ(2, keys.size());
    EXPECT_EQ(k, *keys.front());
    EXPECT_EQ(k, *keys.back());
    clearKeysList(keys);

    EXPECT_EQ(2, bt.removeAll(&k));

    searched = bt.search(&k);
    EXPECT_TRUE(searched == nullptr);
    delete[] searched;
}

TEST_F(BStarPlusTreeTest, Remove5)
{
    std::string& fn = getFn("Remove5.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]) - 1; ++i)
    {
        Byte& el = els[i];

        if(i == sizeof(els) / sizeof(els[0]) - 2)
            EXPECT_EQ(2, bt.removeAll(&el));
        else
            EXPECT_EQ(1, bt.removeAll(&el));

        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]) - 1; ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }
}

TEST_F(BStarPlusTreeTest, Remove6)
{
    std::string& fn = getFn("Remove6.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for(int i = sizeof(els) / sizeof(els[0]) - 2; i >= 0; --i)
    {
        Byte& el = els[i];

        if(i == sizeof(els) / sizeof(els[0]) - 2)
            EXPECT_EQ(2, bt.removeAll(&el));
        else
            EXPECT_EQ(1, bt.removeAll(&el));

        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]) - 1; ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }
}

TEST_F(BStarPlusTreeTest, Remove7)
{
    std::string& fn = getFn("Remove7.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0xAB, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(5, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for(int i = sizeof(els) / sizeof(els[0]) - 5; i >= 0; --i)
    {
        Byte& el = els[i];

        if(i == sizeof(els) / sizeof(els[0]) - 5)
            EXPECT_EQ(5, bt.removeAll(&el));
        else
            EXPECT_EQ(1, bt.removeAll(&el));

        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]) - 1; ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }
}

TEST_F(BStarPlusTreeTest, Remove8)
{
    std::string& fn = getFn("Remove8.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, 5, 1, &comparator, fn);

    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0xAB, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(5, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for(int i = sizeof(els) / sizeof(els[0]) - 5; i >= 0; --i)
    {
        Byte& el = els[i];

        if(i == sizeof(els) / sizeof(els[0]) - 5)
            EXPECT_EQ(5, bt.removeAll(&el));
        else
            EXPECT_EQ(1, bt.removeAll(&el));

        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]) - 1; ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched == nullptr);
        delete[] searched;
    }
}

#ifdef BTREE_WITH_REUSING_FREE_PAGES

TEST_F(BStarPlusTreeTest, RemoveAndReuse1)
{
    std::string& fn = getFn("RemoveAndReuse1.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        EXPECT_TRUE(bt.remove(&el));

        if(i != 5)
        {
            Byte* searched = bt.search(&el);
            EXPECT_TRUE(searched == nullptr);
            delete[] searched;
        }
    }

    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        EXPECT_TRUE(bt.remove(&el));

        if(i != 5)
        {
            Byte* searched = bt.search(&el);
            EXPECT_TRUE(searched == nullptr);
            delete[] searched;
        }
    }
}

TEST_F(BStarPlusTreeTest, RemoveAndReuse2)
{
    std::string& fn = getFn("RemoveAndReuse2.xibt");

    ByteComparator comparator;
    FileBaseBTree bt(BaseBTree::TreeType::B_STAR_PLUS_TREE, ORDER, 1, &comparator, fn);


    Byte els[] = { 0x01, 0x11, 0x09, 0x05, 0x07, 0x03, 0x03 };
    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    // Removing in the reversed order.
    for (int i = sizeof(els) / sizeof(els[0]) - 1; i >= 0; --i)
    {
        Byte& el = els[i];
        EXPECT_TRUE(bt.remove(&el));

        if(i != 6)
        {
            Byte* searched = bt.search(&el);
            EXPECT_TRUE(searched == nullptr);
            delete[] searched;
        }
    }

    for (int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        bt.insert(&el);
    }

    for(int i = 0; i < sizeof(els) / sizeof(els[0]); ++i)
    {
        Byte& el = els[i];
        Byte* searched = bt.search(&el);
        EXPECT_TRUE(searched != nullptr);
        delete[] searched;

        std::list<Byte*> keys;
        if(els[i] == 0x03)
        {
            EXPECT_EQ(2, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.front());
            EXPECT_EQ(el, *keys.back());
        }
        else
        {
            EXPECT_EQ(1, bt.searchAll(&el, keys));
            EXPECT_EQ(el, *keys.back());
        }

        clearKeysList(keys);
    }

    // Removing in the reversed order.
    for (int i = sizeof(els) / sizeof(els[0]) - 1; i >= 0; --i)
    {
        Byte& el = els[i];
        EXPECT_TRUE(bt.remove(&el));

        if(i != 6)
        {
            Byte* searched = bt.search(&el);
            EXPECT_TRUE(searched == nullptr);
            delete[] searched;
        }
    }
}

#endif // BTREE_WITH_REUSING_FREE_PAGES

#endif // BTREE_WITH_DELETION
