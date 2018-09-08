/// \file
/// \brief     The Experiment and ExperimentResult classes.
/// \authors   Anton Rigin
/// \version   0.1
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREES_EXPERIMENT_H
#define BTREES_EXPERIMENT_H

#include <utility>
#include <ctime>

#include "btree.h"

using namespace btree;

class Experiment
{

public:

    Experiment(BaseBTree::TreeType treeType, UShort treeOrder, std::string dataFilePath, std::string searchedName)
            : _treeType(treeType), _treeOrder(treeOrder),
            _dataFilePath(std::move(dataFilePath)), _searchedName(std::move(searchedName)) { }

    Experiment(const Experiment& other) = default;

    Experiment& operator=(Experiment&) = default;

    ~Experiment() = default;

public:

    BaseBTree::TreeType getTreeType() const { return _treeType; }

    UShort getTreeOrder() const { return _treeOrder; }

    const std::string& getDataFilePath() const { return _dataFilePath; }

    const std::string& getSearchedName() const { return _searchedName; }

private:

    BaseBTree::TreeType _treeType;

    UShort _treeOrder;

    std::string _dataFilePath;

    std::string _searchedName;

};

class ExperimentResult
{

public:

    ExperimentResult(const Experiment& experiment) : _experiment(experiment) { }

    ExperimentResult(const ExperimentResult& other) = default;

    ExperimentResult& operator=(ExperimentResult&) = default;

    ~ExperimentResult() = default;

public:

    const Experiment& getExperiment() const { return _experiment; }

    clock_t getInsertionTime() const { return _insertionTime; }

    void setInsertionTime(clock_t insertionTime) { _insertionTime = insertionTime; }

    clock_t getSearchTime() const { return _searchTime; }

    void setSearchTime(clock_t searchTime) { _searchTime = searchTime; }

    clock_t getRemovingTime() const { return _removingTime; }

    void setRemovingTime(clock_t removingTime) { _removingTime = removingTime; }

    clock_t getIndexingTime() const { return _indexingTime; }

    void setIndexingTime(clock_t indexingTime) { _indexingTime = indexingTime; }

    clock_t getIndexSearchingTime() const { return _indexSearchingTime; }

    void setIndexSearchingTime(clock_t indexSearchingTime) { _indexSearchingTime = indexSearchingTime; }

    UInt getInsertionUsedMemory() const { return _insertionUsedMemory; }

    void setInsertionUsedMemory(UInt insertionUsedMemory) { _insertionUsedMemory = insertionUsedMemory; }

    UInt getSearchUsedMemory() const { return _searchUsedMemory; }

    void setSearchUsedMemory(UInt searchUsedMemory) { _searchUsedMemory = searchUsedMemory; }

    UInt getRemovingUsedMemory() const { return _removingUsedMemory; }

    void setRemovingUsedMemory(UInt removingUsedMemory) { _removingUsedMemory = removingUsedMemory; }

    UInt getIndexingUsedMemory() const { return _indexingUsedMemory; }

    void setIndexingUsedMemory(UInt indexingUsedMemory) { _indexingUsedMemory = indexingUsedMemory; }

    UInt getIndexSearchingUsedMemory() const { return _indexSearchingUsedMemory; }

    void setIndexSearchingUsedMemory(UInt indexSearchingUsedMemory)
            { _indexSearchingUsedMemory = indexSearchingUsedMemory; }

    double getInsertionDiskOperationsCount() const { return _insertionDiskOperationsCount; }

    void setInsertionDiskOperationsCount(double insertionDiskOperationsCount)
            { _insertionDiskOperationsCount = insertionDiskOperationsCount; }

    double getSearchDiskOperationsCount() const { return _searchDiskOperationsCount; }

    void setSearchDiskOperationsCount(double searchDiskOperationsCount)
            { _searchDiskOperationsCount = searchDiskOperationsCount; }

    double getRemovingDiskOperationsCount() const { return _removingDiskOperationsCount; }

    void setRemovingDiskOperationsCount(double removingDiskOperationsCount)
            { _removingDiskOperationsCount = removingDiskOperationsCount; }

    double getIndexingDiskOperationsCount() const { return _indexingDiskOperationsCount; }

    void setIndexingDiskOperationsCount(double indexingDiskOperationsCount)
            { _indexingDiskOperationsCount = indexingDiskOperationsCount; }

    double getIndexSearchingDiskOperationsCount() const { return _indexSearchingDiskOperationsCount; }

    void setIndexSearchingDiskOperationsCount(double indexSearchingDiskOperationsCount)
            { _indexSearchingDiskOperationsCount = indexSearchingDiskOperationsCount; }

    UInt getMaxSearchDepth() const { return _maxSearchDepth; }

    UInt setMaxSearchDepth(UInt maxSearchDepth) { _maxSearchDepth = maxSearchDepth; }

    UInt getIndexMaxSearchDepth() const { return _indexMaxSearchDepth; }

    UInt setIndexMaxSearchDepth(UInt indexMaxSearchDepth) { _indexMaxSearchDepth = indexMaxSearchDepth; }

private:

    Experiment _experiment;

    clock_t _insertionTime = 0;
    clock_t _searchTime = 0;
    clock_t _removingTime = 0;
    clock_t _indexingTime = 0;
    clock_t _indexSearchingTime = 0;

    UInt _insertionUsedMemory = 0;
    UInt _searchUsedMemory = 0;
    UInt _removingUsedMemory = 0;
    UInt _indexingUsedMemory = 0;
    UInt _indexSearchingUsedMemory = 0;

    double _insertionDiskOperationsCount = 0;
    double _searchDiskOperationsCount = 0;
    double _removingDiskOperationsCount = 0;
    double _indexingDiskOperationsCount = 0;
    double _indexSearchingDiskOperationsCount = 0;

    UInt _maxSearchDepth;
    UInt _indexMaxSearchDepth;
};

#endif //BTREES_EXPERIMENT_H
