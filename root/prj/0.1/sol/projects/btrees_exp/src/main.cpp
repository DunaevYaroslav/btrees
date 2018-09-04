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

#include "btree.h"
#include "indexer.h"
#include "experiment.h"

const char CSV_DELIM = ';';

const int FULL_KEYS_COUNT = 100000;
const int PREPARATION_KEYS_COUNT = FULL_KEYS_COUNT / 2;
const int MEASURING_KEYS_COUNT = FULL_KEYS_COUNT - PREPARATION_KEYS_COUNT;

const std::regex csvFileNameRegex("^(\\S*)\\.csv$");

const int CSV_FILE_NAME_REGEX_BEFORE_EXTENSION = 1;

Experiment parseExperiment(const std::string& line);

BaseBTree::TreeType parseTreeType(const std::string& treeTypeString);

void makeExperiment(const Experiment& experiment, std::ofstream& outputFile, int experimentNumber);

void writeCsvHeader(std::ofstream& outputFile);

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
    while (std::getline(inputFile, line))
    {
        Experiment experiment = parseExperiment(line);
        experiments.push_back(experiment);
    }

    inputFile.close();

    std::string outputFileName = std::string(match[CSV_FILE_NAME_REGEX_BEFORE_EXTENSION]) + std::string(".csv");
    std::ofstream outputFile(outputFileName);

    if (!outputFile.is_open())
    {
        std::cerr << "Cannot open the output file " << inputFileName << " for reading" << std::endl;

        return -1;
    }

    writeCsvHeader(outputFile);

    int i = 0;
    for (std::vector<Experiment>::iterator iter = experiments.begin(); iter != experiments.end(); ++iter)
        makeExperiment(*iter, outputFile, ++i);

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

    std::string treeKeysSizeString;
    std::getline(lineStream, treeKeysSizeString, CSV_DELIM);
    UInt treeKeysSize = std::stoi(treeKeysSizeString);

    std::string dataFilePath;
    std::getline(lineStream, dataFilePath, CSV_DELIM);

    std::string searchedName;
    std::getline(lineStream, searchedName, CSV_DELIM);

    return Experiment(treeType, treeOrder, treeKeysSize, dataFilePath, searchedName);
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

    FileBaseBTree* tree = indexer.getTree();

    clock_t time = 0;
    UInt usedMemory = 0;
    UInt readDiskOperationsCount = 0;
    UInt writeDiskOperationsCount = 0;
    UInt seekDiskOperationsCount = 0;

    for (int i = 0; i < FULL_KEYS_COUNT; ++i)
    {
        if (i >= PREPARATION_KEYS_COUNT)
        {
            clock_t start = std::clock();
            tree->insert((Byte*) &i);
            clock_t end = std::clock();
            time += end - start;
            readDiskOperationsCount += tree->getTree()->getReadDiskOperationsCount();
            writeDiskOperationsCount += tree->getTree()->getWriteDiskOperationsCount();
            seekDiskOperationsCount += tree->getTree()->getSeekDiskOperationsCount();
        }
    }

    time /= MEASURING_KEYS_COUNT;
    usedMemory /= MEASURING_KEYS_COUNT;
    readDiskOperationsCount /= MEASURING_KEYS_COUNT;
    writeDiskOperationsCount /= MEASURING_KEYS_COUNT;
    seekDiskOperationsCount /= MEASURING_KEYS_COUNT;
    experimentResult.setInsertionTime(time);
    experimentResult.setInsertionUsedMemory(usedMemory);
    experimentResult.setInsertionReadDiskOperationsCount(readDiskOperationsCount);
    experimentResult.setInsertionWriteDiskOperationsCount(writeDiskOperationsCount);
    experimentResult.setInsertionSeekDiskOperationsCount(seekDiskOperationsCount);

    time = 0;
    usedMemory = 0;
    readDiskOperationsCount = writeDiskOperationsCount = seekDiskOperationsCount = 0;

    for (int i = 0; i < FULL_KEYS_COUNT; ++i)
    {
        clock_t start = std::clock();
        tree->search((Byte*) &i);
        clock_t end = std::clock();
        time += end - start;
        readDiskOperationsCount += tree->getTree()->getReadDiskOperationsCount();
        writeDiskOperationsCount += tree->getTree()->getWriteDiskOperationsCount();
        seekDiskOperationsCount += tree->getTree()->getSeekDiskOperationsCount();
    }

    time /= FULL_KEYS_COUNT;
    usedMemory /= FULL_KEYS_COUNT;
    readDiskOperationsCount /= FULL_KEYS_COUNT;
    writeDiskOperationsCount /= FULL_KEYS_COUNT;
    seekDiskOperationsCount /= FULL_KEYS_COUNT;
    experimentResult.setSearchTime(time);
    experimentResult.setSearchUsedMemory(usedMemory);
    experimentResult.setSearchReadDiskOperationsCount(readDiskOperationsCount);
    experimentResult.setSearchWriteDiskOperationsCount(writeDiskOperationsCount);
    experimentResult.setSearchSeekDiskOperationsCount(seekDiskOperationsCount);

    time = 0;
    usedMemory = 0;
    readDiskOperationsCount = writeDiskOperationsCount = seekDiskOperationsCount = 0;

    for (int i = 0; i < FULL_KEYS_COUNT; ++i)
    {
        clock_t start = std::clock();
        tree->remove((Byte*) &i);
        clock_t end = std::clock();
        time += end - start;
        readDiskOperationsCount += tree->getTree()->getReadDiskOperationsCount();
        writeDiskOperationsCount += tree->getTree()->getWriteDiskOperationsCount();
        seekDiskOperationsCount += tree->getTree()->getSeekDiskOperationsCount();
    }

    time /= FULL_KEYS_COUNT;
    usedMemory /= FULL_KEYS_COUNT;
    readDiskOperationsCount /= FULL_KEYS_COUNT;
    writeDiskOperationsCount /= FULL_KEYS_COUNT;
    seekDiskOperationsCount /= FULL_KEYS_COUNT;
    experimentResult.setRemovingTime(time);
    experimentResult.setRemovingUsedMemory(usedMemory);
    experimentResult.setRemovingReadDiskOperationsCount(readDiskOperationsCount);
    experimentResult.setRemovingWriteDiskOperationsCount(writeDiskOperationsCount);
    experimentResult.setRemovingSeekDiskOperationsCount(seekDiskOperationsCount);

    clock_t start = std::clock();
    indexer.indexFile(experiment.getDataFilePath());
    clock_t end = std::clock();
    time = end - start;
    readDiskOperationsCount = indexer.getReadDiskOperationsCount();
    writeDiskOperationsCount = indexer.getWriteDiskOperationsCount();
    seekDiskOperationsCount = indexer.getSeekDiskOperationsCount();

    experimentResult.setIndexingTime(time);
    experimentResult.setIndexingUsedMemory(usedMemory);
    experimentResult.setIndexingReadDiskOperationsCount(readDiskOperationsCount);
    experimentResult.setIndexingWriteDiskOperationsCount(writeDiskOperationsCount);
    experimentResult.setIndexingSeekDiskOperationsCount(seekDiskOperationsCount);

    const std::string& searchedName = experiment.getSearchedName();

    start = std::clock();
    indexer.findAllOccurrences(std::wstring(searchedName.begin(), searchedName.end()),
                               experiment.getDataFilePath());
    end = std::clock();
    time = end - start;
    readDiskOperationsCount = indexer.getReadDiskOperationsCount();
    writeDiskOperationsCount = indexer.getWriteDiskOperationsCount();
    seekDiskOperationsCount = indexer.getSeekDiskOperationsCount();
    experimentResult.setIndexSearchingTime(time);
    experimentResult.setIndexSearchingUsedMemory(usedMemory);
    experimentResult.setIndexSearchingReadDiskOperationsCount(readDiskOperationsCount);
    experimentResult.setIndexSearchingWriteDiskOperationsCount(writeDiskOperationsCount);
    experimentResult.setIndexingSeekDiskOperationsCount(seekDiskOperationsCount);

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
            << experimentResult.getInsertionReadDiskOperationsCount() << CSV_DELIM
            << experimentResult.getSearchReadDiskOperationsCount() << CSV_DELIM
            << experimentResult.getRemovingReadDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexingReadDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexSearchingReadDiskOperationsCount() << CSV_DELIM
            << experimentResult.getInsertionWriteDiskOperationsCount() << CSV_DELIM
            << experimentResult.getSearchWriteDiskOperationsCount() << CSV_DELIM
            << experimentResult.getRemovingWriteDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexingWriteDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexSearchingWriteDiskOperationsCount() << CSV_DELIM
            << experimentResult.getInsertionSeekDiskOperationsCount() << CSV_DELIM
            << experimentResult.getSearchSeekDiskOperationsCount() << CSV_DELIM
            << experimentResult.getRemovingSeekDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexingSeekDiskOperationsCount() << CSV_DELIM
            << experimentResult.getIndexSearchingSeekDiskOperationsCount() << std::endl;
}

void writeCsvHeader(std::ofstream& outputFile)
{
    outputFile << "InsertionTime;SearchTime;RemovingTime;IndexingTime;IndexSearchingTime" << std::endl
            << "InsertionUsedMemory;SearchUsedMemory;RemovingUsedMemory;"
            << "IndexingUsedMemory;IndexSearchingUsedMemory" << std::endl
            << "InsertionReadDiskOperationsCount;SearchReadDiskOperationsCount;RemovingReadDiskOperationsCount;"
            << "IndexingReadDiskOperationsCount;IndexSearchingReadDiskOperationsCount" << std::endl
            << "InsertionWriteDiskOperationsCount;SearchWriteDiskOperationsCount;RemovingWriteDiskOperationsCount;"
            << "IndexingWriteDiskOperationsCount;IndexSearchingWriteDiskOperationsCount" << std::endl
            << "InsertionSeekDiskOperationsCount;SearchSeekDiskOperationsCount;RemovingSeekDiskOperationsCount;"
            << "IndexingSeekDiskOperationsCount;IndexSearchingSeekDiskOperationsCount" << std::endl;
}
