/// \file
/// \brief     The main experiments instrument program file
/// \authors   Anton Rigin
/// \version   0.1
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <stdexcept>
#include <malloc.h>

#include "btree.h"
#include "indexer.h"
#include "experiment.h"

const char CSV_DELIM = ';';

const int FULL_KEYS_COUNT = 100000;
const int PREPARATION_KEYS_COUNT = FULL_KEYS_COUNT / 2;
const int MEASURING_KEYS_COUNT = FULL_KEYS_COUNT - PREPARATION_KEYS_COUNT;

const std::regex csvFileNameRegex("^(\\S*)\\.csv$");

const int CSV_FILE_NAME_REGEX_BEFORE_EXTENSION = 1;

size_t currentUsedMemory = 0;
size_t maxUsedMemory = 0;

struct ByteComparator : public BaseBTree::IComparator {

    virtual bool compare(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz; ++i)
        {
            if (lhv[i] < rhv[i])
                return true;
        }

        return false;
    }

    virtual bool isEqual(const Byte* lhv, const Byte* rhv, UInt sz) override
    {
        for (UInt i = 0; i < sz; ++i)
        {
            if (lhv[i] != rhv[i])
                return false;
        }

        return true;
    }
}; // struct ByteComparator

class DecimalPointLoc: public std::numpunct<char>
{

public:

    typedef char char_type;

    typedef std::string string_type;

    explicit DecimalPointLoc(size_t r = 0): std::numpunct<char>(r) {}

protected:

    char do_decimal_point() const { return ','; }

};

Experiment parseExperiment(const std::string& line);

BaseBTree::TreeType parseTreeType(const std::string& treeTypeString);

void makeExperiment(const Experiment& experiment, std::ofstream& outputFile, int experimentNumber);

void writeCsvHeader(std::ofstream& outputFile);

void* operator new(size_t size);

void operator delete(void* p) noexcept;

void* operator new[](size_t size);

void operator delete[](void* p) noexcept;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "The count of the command line arguments should be equal to 1"
                  << " - it should be the name of the CSV file with the experiments scheme" << std::endl;

        return -1;
    }

    std::string inputFileName(argv[1]);

    std::smatch match;
    if (!std::regex_match(inputFileName, match, csvFileNameRegex))
    {
        std::cerr << "The incorrect CSV file name. Should be *.csv" << std::endl;

        return -1;
    }

    std::ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        std::cerr << "Cannot open the input file " << inputFileName << " for reading" << std::endl;

        return -1;
    }

    std::vector<Experiment> experiments;

    std::string line;
    std::getline(inputFile, line);
    int i = 0;
    while (std::getline(inputFile, line))
    {
        try
        {
            Experiment experiment = parseExperiment(line);
            experiments.push_back(experiment);
        }
        catch (std::invalid_argument& e)
        {
            std::cerr << "The error appeared during parsing the experiment " << i << ": " << e.what() << std::endl;
        }
        catch (std::out_of_range& e)
        {
            std::cerr << "The error appeared during parsing the experiment " << i << ": " << e.what() << std::endl;
        }
    }

    inputFile.close();

    std::string outputFileName = std::string(match[CSV_FILE_NAME_REGEX_BEFORE_EXTENSION])
            + "_results" + std::string(".csv");
    std::ofstream outputFile(outputFileName);

    if (!outputFile.is_open())
    {
        std::cerr << "Cannot open the output file " << inputFileName << " for reading" << std::endl;

        return -1;
    }

    writeCsvHeader(outputFile);

    i = 0;
    for (std::vector<Experiment>::iterator iter = experiments.begin(); iter != experiments.end(); ++iter)
    {
        std::cout << "Making the experiment " << ++i << "/" << experiments.size() << "..." << std::endl;
        try
        {
            makeExperiment(*iter, outputFile, i);
            std::cout << "The experiment " << i << "/" << experiments.size() << " is finished" << std::endl;
        }
        catch (std::invalid_argument& e)
        {
            std::cerr << "The error appeared during the making the experiment" << i << "/" << experiments.size() << ": "
                    << e.what() << std::endl;
        }
    }

    outputFile.close();

    std::cout << "The output file " << outputFileName << " has been successfully written" << std::endl;

    return 0;
}

Experiment parseExperiment(const std::string& line)
{
    std::stringstream lineStream(line);

    std::string treeTypeString;
    std::getline(lineStream, treeTypeString, CSV_DELIM);
    BaseBTree::TreeType treeType = parseTreeType(treeTypeString);

    std::string treeOrderString;
    std::getline(lineStream, treeOrderString, CSV_DELIM);
    UShort treeOrder = std::stoi(treeOrderString);

    std::string dataFilePath;
    std::getline(lineStream, dataFilePath, CSV_DELIM);

    std::string searchedName;
    std::getline(lineStream, searchedName, CSV_DELIM);

    return Experiment(treeType, treeOrder, dataFilePath, searchedName);
}

BaseBTree::TreeType parseTreeType(const std::string& treeTypeString)
{
    if (treeTypeString == "B_TREE")
        return BaseBTree::TreeType::B_TREE;
    else if (treeTypeString == "B_PLUS_TREE")
        return BaseBTree::TreeType::B_PLUS_TREE;
    else if (treeTypeString == "B_STAR_TREE")
        return BaseBTree::TreeType::B_STAR_TREE;
    else if (treeTypeString == "B_STAR_PLUS_TREE")
        return BaseBTree::TreeType::B_STAR_PLUS_TREE;
    else
        throw std::invalid_argument("Cannot parse tree type: " + treeTypeString);
}

void makeExperiment(const Experiment& experiment, std::ofstream& outputFile, int experimentNumber)
{
    ExperimentResult experimentResult(experiment);

    Indexer indexer;
    indexer.create(experiment.getTreeType(), experiment.getTreeOrder(), std::to_string(experimentNumber)
            + std::string(".xibt"));

    ByteComparator comparator;
    FileBaseBTree* tree = new FileBaseBTree(experiment.getTreeType(), experiment.getTreeOrder(), sizeof(int),
            &comparator, std::string("int_") + std::to_string(experimentNumber) + std::string(".xibt"));

    clock_t time = 0;
    size_t usedMemory = 0;
    UInt diskOperationsCount = 0;

    clock_t start = std::clock();
    for (int i = 0; i < FULL_KEYS_COUNT; ++i)
    {
        if (i >= PREPARATION_KEYS_COUNT)
        {
            maxUsedMemory = 0;
            tree->getTree()->resetDiskOperationsCount();

            tree->insert((Byte*) &i);

            usedMemory += maxUsedMemory;
            diskOperationsCount += tree->getTree()->getDiskOperationsCount();
        }
        else
            tree->insert((Byte*) &i);
    }
    clock_t end = std::clock();
    time = end - start;

    usedMemory /= MEASURING_KEYS_COUNT;
    experimentResult.setInsertionTime(time);
    experimentResult.setInsertionUsedMemory(usedMemory);
    experimentResult.setInsertionDiskOperationsCount(((double) diskOperationsCount) / MEASURING_KEYS_COUNT);

    usedMemory = 0;
    diskOperationsCount = 0;

    start = std::clock();
    for (int i = 0; i < FULL_KEYS_COUNT; ++i)
    {
        maxUsedMemory = 0;
        tree->getTree()->resetDiskOperationsCount();

        tree->search((Byte*) &i);

        usedMemory += maxUsedMemory;
        diskOperationsCount += tree->getTree()->getDiskOperationsCount();
    }
    end = std::clock();
    time += end - start;

    usedMemory /= FULL_KEYS_COUNT;
    experimentResult.setSearchTime(time);
    experimentResult.setSearchUsedMemory(usedMemory);
    experimentResult.setSearchDiskOperationsCount(((double) diskOperationsCount) / FULL_KEYS_COUNT);
    experimentResult.setMaxSearchDepth(tree->getTree()->getMaxSearchDepth());

    time = 0;
    usedMemory = 0;
    diskOperationsCount = 0;

    start = std::clock();
    for (int i = 0; i < FULL_KEYS_COUNT; ++i)
    {
        maxUsedMemory = 0;
        tree->getTree()->resetDiskOperationsCount();

        tree->remove((Byte*) &i);

        usedMemory += maxUsedMemory;
        diskOperationsCount += tree->getTree()->getDiskOperationsCount();
    }
    end = std::clock();
    time += end - start;

    usedMemory /= FULL_KEYS_COUNT;
    experimentResult.setRemovingTime(time);
    experimentResult.setRemovingUsedMemory(usedMemory);
    experimentResult.setRemovingDiskOperationsCount(((double) diskOperationsCount) / FULL_KEYS_COUNT);

    indexer.resetDiskOperationsCount();
    start = std::clock();
    indexer.indexFile(experiment.getDataFilePath());
    end = std::clock();
    time = end - start;
    usedMemory = maxUsedMemory;
    diskOperationsCount = indexer.getDiskOperationsCount();

    experimentResult.setIndexingTime(time);
    experimentResult.setIndexingUsedMemory(usedMemory);
    experimentResult.setIndexingDiskOperationsCount(diskOperationsCount);

    const std::string& searchedName = experiment.getSearchedName();

    indexer.resetDiskOperationsCount();
    start = std::clock();
    indexer.findAllOccurrences(std::wstring(searchedName.begin(), searchedName.end()),
                               experiment.getDataFilePath());
    end = std::clock();
    time = end - start;
    usedMemory = maxUsedMemory;
    diskOperationsCount = indexer.getDiskOperationsCount();
    experimentResult.setIndexSearchingTime(time);
    experimentResult.setIndexSearchingUsedMemory(usedMemory);
    experimentResult.setIndexSearchingDiskOperationsCount(diskOperationsCount);
    experimentResult.setIndexMaxSearchDepth(indexer.getMaxSearchDepth());

    DecimalPointLoc decimalPointLoc;
    outputFile.imbue(std::locale(std::locale(), &decimalPointLoc));
    outputFile.precision(3);
    outputFile << experimentNumber << CSV_DELIM
            << experimentResult.getInsertionTime() << CSV_DELIM
            << experimentResult.getSearchTime() << CSV_DELIM
            << experimentResult.getRemovingTime() << CSV_DELIM
            << experimentResult.getIndexingTime() << CSV_DELIM
            << experimentResult.getIndexSearchingTime() << CSV_DELIM
            << experimentResult.getInsertionUsedMemory() << CSV_DELIM
            << experimentResult.getSearchUsedMemory() << CSV_DELIM
            << experimentResult.getRemovingUsedMemory() << CSV_DELIM
            << experimentResult.getIndexingUsedMemory() << CSV_DELIM
            << experimentResult.getIndexSearchingUsedMemory() << CSV_DELIM
            << experimentResult.getInsertionDiskOperationsCount() << CSV_DELIM
            << experimentResult.getSearchDiskOperationsCount() << CSV_DELIM
            << experimentResult.getRemovingDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexingDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexSearchingDiskOperationsCount() << CSV_DELIM
            << experimentResult.getMaxSearchDepth() << CSV_DELIM
            << experimentResult.getIndexMaxSearchDepth() << std::endl;
}

void writeCsvHeader(std::ofstream& outputFile)
{
    outputFile << "Number;InsertionTime;SearchTime;RemovingTime;IndexingTime;IndexSearchingTime;"
            << "InsertionUsedMemory;SearchUsedMemory;RemovingUsedMemory;"
            << "IndexingUsedMemory;IndexSearchingUsedMemory;"
            << "InsertionDiskOperationsCount;SearchDiskOperationsCount;RemovingDiskOperationsCount;"
            << "IndexingDiskOperationsCount;IndexSearchingDiskOperationsCount;"
            << "MaxSearchDepth;IndexMaxSearchDepth" << std::endl;
}

void* operator new(size_t size)
{
    void* p = malloc(size);
    currentUsedMemory += _msize(p);
    if (currentUsedMemory > maxUsedMemory)
        maxUsedMemory = currentUsedMemory;
    return p;
}

void operator delete(void* p) noexcept
{
    currentUsedMemory -= _msize(p);
    free(p);
}

void* operator new[](size_t size)
{
    return ::operator new(size);
}

void operator delete[](void* p) noexcept
{
    ::operator delete(p);
}
