/**
 * (c) Anton Rigin, group BPI154(2), 2017.
 */

#include <gtest-fus/gtest.h>

#include <string>
#include <ctime>

#include "individual.h"
#include "indexer.h"

using namespace xi;

static const int ORDERS[] = { 2, 4, 6, 8, 10, 20, 30, 40, 50, 250, 450, 650, 850, 1050, 1250, 1450, 1650, 1850, 2050 };

static const int ORDERS_LENGTH = sizeof(ORDERS) / sizeof(int);

/**
 * Represents some methods for testing.
 */
class BPlusTreeBasedIndexTest : public ::testing::Test
{

public:

    /**
     * The B-tree's order.
     */
    static const int ORDER = 50;

public:

    /**
     * Constructor. Sets the Russian locale.
     */
    BPlusTreeBasedIndexTest() { setlocale(LC_ALL, "Russian"); }

public:

    /**
     * Tests index searching, counts time.
     *
     * @param fileName Name of file where searching is being performed.
     * @param treeFileName Name of file where BTree will be stored.
     * @param indexOutFileName Name of file where found occurrences (during index searching) will be stored.
     * @param linearOutFileName Name of file where found occurrences (during linear searching) will be stored.
     * @param name Name for searching in the file.
     * @param expectedCount Expected count of occurrences in the file.
     */
    void testIndex(std::string fileName, std::string treeFileName,
            std::string indexOutFileName, std::string linearOutFileName,
            std::wstring name, int expectedCount);

    void testIndexWithDifferentParams(std::string fileName, std::string treeFileName,
            std::string indexOutFileName, std::string outputCsvFileName, std::wstring name, int expectedCount);

private:

    /**
     * Performs linear searching in the file.
     *
     * @param name Name for searching in the file.
     * @param fileName Name of file where searching is being performed.
     * @return List of the occurrences' strings.
     */
    std::list<std::wstring> searchLinearly(std::wstring& name, std::string fileName);

    /**
     * Writes found occurrences to file with given name.
     *
     * @param occurrences List of the occurrences' strings.
     * @param fileName Name of file where found occurrences will be stored.
     */
    void writeOccurrencesToFile(std::list<std::wstring>& occurrences, std::string& fileName);

    /**
     * Gets time between begin time and end time in seconds.
     *
     * @param begin Begin time.
     * @param end End time.
     * @return Time between begin time and end time in seconds.
     */
    double getTimeInSecs(clock_t begin, clock_t end);

};

void BPlusTreeBasedIndexTest::testIndex(std::string fileName, std::string treeFileName,
        std::string indexOutFileName, std::string linearOutFileName,
        std::wstring name, int expectedCount)
{
    std::cout << "File " << fileName << std::endl;

    fileName = TEST_FILES_PATH + fileName;
    treeFileName = TEST_FILES_PATH + treeFileName;
    indexOutFileName = TEST_FILES_PATH + indexOutFileName;
    linearOutFileName = TEST_FILES_PATH + linearOutFileName;

    xi::Indexer indexer;
    indexer.create(BaseBTree::TreeType::B_PLUS_TREE, ORDER, treeFileName);

    std::cout << "Indexing..." << std::endl;
    clock_t begin = clock();
    indexer.indexFile(fileName);
    clock_t end = clock();
    std::cout << "Time of indexing: " << getTimeInSecs(begin, end) << " s" << std::endl;

    std::cout << "Index searching..." << std::endl;
    begin = clock();
    std::list<std::wstring> occurrences = indexer.findAllOccurrences(name, fileName);
    end = clock();
    std::cout << "Time of searching using index: " << getTimeInSecs(begin, end) << " s" << std::endl;
    EXPECT_EQ(expectedCount, occurrences.size());
    std::wcout << "Found: " << occurrences.size() << std::endl;
    std::wcout << "Tree's max search depth: " << indexer.getMaxSearchDepth() << std::endl;
    writeOccurrencesToFile(occurrences, indexOutFileName);

    std::cout << "Linear searching..." << std::endl;
    begin = clock();
    occurrences = searchLinearly(name, fileName);
    end = clock();
    std::cout << "Time of linear searching: " << getTimeInSecs(begin, end) << " s" << std::endl;
    EXPECT_EQ(expectedCount, occurrences.size());
    std::wcout << "Found: " << occurrences.size() << std::endl;
    writeOccurrencesToFile(occurrences, linearOutFileName);
}

void BPlusTreeBasedIndexTest::testIndexWithDifferentParams(std::string fileName, std::string treeFileName,
        std::string indexOutFileName, std::string outputCsvFileName, std::wstring name, int expectedCount)
{
    std::cout << "File " << fileName << std::endl;

    fileName = TEST_FILES_PATH + fileName;
    treeFileName = TEST_FILES_PATH + treeFileName;
    indexOutFileName = TEST_FILES_PATH + indexOutFileName;
    outputCsvFileName = TEST_FILES_PATH + outputCsvFileName;

    std::ofstream outputCsvFile(outputCsvFileName);

    if (!outputCsvFile.is_open())
        throw std::invalid_argument("Cannot open output CSV file for writing");

    outputCsvFile << "Tree order;Time of indexing;Time of searching;Tree's max search depth" << std::endl;

    for (int i = 0; i < ORDERS_LENGTH; ++i)
    {
        std::cout << "Tree order " << ORDERS[i] << std::endl;

        xi::Indexer indexer;
        indexer.create(ORDERS[i], treeFileName);

        std::cout << "Indexing..." << std::endl;
        clock_t begin = clock();
        indexer.indexFile(fileName);
        clock_t end = clock();
        double indexingTime = getTimeInSecs(begin, end);
        std::cout << "Time of indexing: " << indexingTime << " s" << std::endl;

        std::cout << "Index searching..." << std::endl;
        begin = clock();
        std::list<std::wstring> occurrences = indexer.findAllOccurrences(name, fileName);
        end = clock();

        double searchingTime = getTimeInSecs(begin, end);
        std::cout << "Time of searching using index: " << searchingTime << " s" << std::endl;

        EXPECT_EQ(expectedCount, occurrences.size());
        std::wcout << "Found: " << occurrences.size() << std::endl;

        int treeMaxSearchDepth = indexer.getMaxSearchDepth();
        std::wcout << "Tree's max search depth: " << treeMaxSearchDepth << std::endl;
        writeOccurrencesToFile(occurrences, indexOutFileName);

        outputCsvFile << ORDERS[i] << ";" << indexingTime << ";"
                      << searchingTime<< ";" << treeMaxSearchDepth << std::endl;
    }

    outputCsvFile.close();
}

std::list<std::wstring> BPlusTreeBasedIndexTest::searchLinearly(std::wstring& name, std::string fileName)
{
    std::wifstream file(fileName);

    if(!file.is_open())
        throw std::logic_error("Cannot open file for linear searching.\n");

    std::list<std::wstring> occurrences;

    std::wstring line;

    while(std::getline(file, line))
    {
        std::wstringstream sstream(line);

        std::wstring lineName;
        std::getline(sstream, lineName, L';');

        if(name == lineName)
            occurrences.push_back(line);
    }

    return occurrences;
}

void BPlusTreeBasedIndexTest::writeOccurrencesToFile(std::list<std::wstring>& occurrences, std::string& fileName)
{
    std::wofstream file(fileName);

    if(!file.is_open())
    {
        std::cout << "Can't open file " << fileName << " for writing." << std::endl;
        return;
    }

    for(std::list<std::wstring>::iterator iter = occurrences.begin(); iter != occurrences.end(); ++iter)
        file << *iter << std::endl;

    file.close();

    std::cout << "File " << fileName << " has been successfully written." << std::endl;
}

double BPlusTreeBasedIndexTest::getTimeInSecs(clock_t begin, clock_t end)
{
    return ((double)(end - begin)) / CLOCKS_PER_SEC;
}



TEST_F(BPlusTreeBasedIndexTest, IndexerTest1)
{
    testIndex("University_and_books.csv", "BTree_University_and_books.xibt",
              "Occurrences_index_University_and_books.txt", "Occurrences_linear_University_and_books.txt",
              L"Подбельский", 3);
}

TEST_F(BPlusTreeBasedIndexTest, IndexerTest2)
{
    testIndex("Hospital_log.csv", "BTree_Hospital_log.xibt",
              "Occurrences_index_Hospital_log.txt", "Occurrences_linear_Hospital_log.txt",
              L"1e consult poliklinisch", 1136);
}

TEST_F(BPlusTreeBasedIndexTest, IndexerWithDifferentParamsTest1)
{
    testIndexWithDifferentParams(
            "University_and_books.csv", "BTree_University_and_books.xibt",
            "Occurrences_index_University_and_books.txt", "Results_BPlusTree_University_and_books.csv",
            L"Подбельский", 3
    );
}

TEST_F(BPlusTreeBasedIndexTest, IndexerWithDifferentParamsTest2)
{
    testIndexWithDifferentParams(
            "Hospital_log.csv", "BTree_Hospital_log.xibt",
            "Occurrences_index_Hospital_log.txt", "Results_BPlusTree_Hospital_log.csv",
            L"1e consult poliklinisch", 1136
    );
}
