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

    Experiment(BaseBTree::TreeType treeType, UShort treeOrder, UInt treeKeysSize,
               std::string dataFilePath, std::string searchedName)
            : _treeType(treeType), _treeOrder(treeOrder), _treeKeysSize(treeKeysSize),
            _dataFilePath(std::move(dataFilePath)), _searchedName(std::move(searchedName)) { }

    Experiment(const Experiment& other) = default;

    Experiment& operator=(Experiment&) = default;

    ~Experiment() = default;

public:

    BaseBTree::TreeType getTreeType() const { return _treeType; }

    UShort getTreeOrder() const { return _treeOrder; }

    UInt getTreeKeysSize() const { return _treeKeysSize; }

    const std::string& getDataFilePath() const { return _dataFilePath; }

    const std::string& getSearchedName() const { return _searchedName; }

private:

    BaseBTree::TreeType _treeType;

    UShort _treeOrder;

    UInt _treeKeysSize;

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

    void setInsertionTime(clock_t _insertionTime) { _insertionTime = _insertionTime; }

    clock_t getSearchTime() const { return _searchTime; }

    void setSearchTime(clock_t _searchTime) { _searchTime = _searchTime; }

    clock_t getRemovingTime() const { return _removingTime; }

    void setRemovingTime(clock_t _removingTime) { _removingTime = _removingTime; }

    clock_t getIndexingTime() const { return _indexingTime; }

    void setIndexingTime(clock_t _indexingTime) { _indexingTime = _indexingTime; }

    clock_t getIndexSearchingTime() const { return _indexSearchingTime; }

    void setIndexSearchingTime(clock_t _indexSearchingTime) { _indexSearchingTime = _indexSearchingTime; }

    UInt getInsertionUsedMemory() const { return _insertionUsedMemory; }

    void setInsertionUsedMemory(UInt _insertionUsedMemory) { _insertionUsedMemory = _insertionUsedMemory; }

    UInt getSearchUsedMemory() const { return _searchUsedMemory; }

    void setSearchUsedMemory(UInt _searchUsedMemory) { _searchUsedMemory = _searchUsedMemory; }

    UInt getRemovingUsedMemory() const { return _removingUsedMemory; }

    void setRemovingUsedMemory(UInt _removingUsedMemory) { _removingUsedMemory = _removingUsedMemory; }

    UInt getIndexingUsedMemory() const { return _indexingUsedMemory; }

    void setIndexingUsedMemory(UInt _indexingUsedMemory) { _indexingUsedMemory = _indexingUsedMemory; }

    UInt getIndexSearchingUsedMemory() const { return _indexSearchingUsedMemory; }

    void setIndexSearchingUsedMemory(UInt _indexSearchingUsedMemory)
            { _indexSearchingUsedMemory = _indexSearchingUsedMemory; }

    UInt getInsertionReadDiskOperationsCount() const { return _insertionReadDiskOperationsCount; }

    void setInsertionReadDiskOperationsCount(UInt _insertionReadDiskOperationsCount)
            { _insertionReadDiskOperationsCount = _insertionReadDiskOperationsCount; }

    UInt getSearchReadDiskOperationsCount() const { return _searchReadDiskOperationsCount; }

    void setSearchReadDiskOperationsCount(UInt _searchReadDiskOperationsCount)
            { _searchReadDiskOperationsCount = _searchReadDiskOperationsCount; }

    UInt getRemovingReadDiskOperationsCount() const { return _removingReadDiskOperationsCount; }

    void setRemovingReadDiskOperationsCount(UInt _removingReadDiskOperationsCount)
            { _removingReadDiskOperationsCount = _removingReadDiskOperationsCount; }

    UInt getIndexingReadDiskOperationsCount() const { return _indexingReadDiskOperationsCount; }

    void setIndexingReadDiskOperationsCount(UInt _indexingReadDiskOperationsCount)
            { _indexingReadDiskOperationsCount = _indexingReadDiskOperationsCount; }

    UInt getIndexSearchingReadDiskOperationsCount() const { return _indexSearchingReadDiskOperationsCount; }

    void setIndexSearchingReadDiskOperationsCount(UInt _indexSearchingReadDiskOperationsCount)
            { _indexSearchingReadDiskOperationsCount = _indexSearchingReadDiskOperationsCount; }

    UInt getInsertionWriteDiskOperationsCount() const { return _insertionWriteDiskOperationsCount; }

    void setInsertionWriteDiskOperationsCount(UInt _insertionWriteDiskOperationsCount)
            { _insertionWriteDiskOperationsCount = _insertionWriteDiskOperationsCount; }

    UInt getSearchWriteDiskOperationsCount() const { return _searchWriteDiskOperationsCount; }

    void setSearchWriteDiskOperationsCount(UInt _searchWriteDiskOperationsCount)
            { _searchWriteDiskOperationsCount = _searchWriteDiskOperationsCount; }

    UInt getRemovingWriteDiskOperationsCount() const { return _removingWriteDiskOperationsCount; }

    void setRemovingWriteDiskOperationsCount(UInt _removingWriteDiskOperationsCount)
            { _removingWriteDiskOperationsCount = _removingWriteDiskOperationsCount; }

    UInt getIndexingWriteDiskOperationsCount() const { return _indexingWriteDiskOperationsCount; }

    void setIndexingWriteDiskOperationsCount(UInt _indexingWriteDiskOperationsCount)
            { _indexingWriteDiskOperationsCount = _indexingWriteDiskOperationsCount; }

    UInt getIndexSearchingWriteDiskOperationsCount() const { return _indexSearchingWriteDiskOperationsCount; }

    void setIndexSearchingWriteDiskOperationsCount(UInt _indexSearchingWriteDiskOperationsCount)
            { _indexSearchingWriteDiskOperationsCount = _indexSearchingWriteDiskOperationsCount; }

    UInt getInsertionSeekDiskOperationsCount() const { return _insertionSeekDiskOperationsCount; }

    void setInsertionSeekDiskOperationsCount(UInt _insertionSeekDiskOperationsCount)
            { _insertionSeekDiskOperationsCount = _insertionSeekDiskOperationsCount; }

    UInt getSearchSeekDiskOperationsCount() const { return _searchSeekDiskOperationsCount; }

    void setSearchSeekDiskOperationsCount(UInt _searchSeekDiskOperationsCount)
            { _searchSeekDiskOperationsCount = _searchSeekDiskOperationsCount; }

    UInt getRemovingSeekDiskOperationsCount() const { return _removingSeekDiskOperationsCount; }

    void setRemovingSeekDiskOperationsCount(UInt _removingSeekDiskOperationsCount)
            { _removingSeekDiskOperationsCount = _removingSeekDiskOperationsCount; }

    UInt getIndexingSeekDiskOperationsCount() const { return _indexingSeekDiskOperationsCount; }

    void setIndexingSeekDiskOperationsCount(UInt _indexingSeekDiskOperationsCount)
            { _indexingSeekDiskOperationsCount = _indexingSeekDiskOperationsCount; }

    UInt getIndexSearchingSeekDiskOperationsCount() const { return _indexSearchingSeekDiskOperationsCount; }

    void setIndexSearchingSeekDiskOperationsCount(UInt _indexSearchingSeekDiskOperationsCount)
            { _indexSearchingSeekDiskOperationsCount = _indexSearchingSeekDiskOperationsCount; }

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

    UInt _insertionReadDiskOperationsCount = 0;
    UInt _searchReadDiskOperationsCount = 0;
    UInt _removingReadDiskOperationsCount = 0;
    UInt _indexingReadDiskOperationsCount = 0;
    UInt _indexSearchingReadDiskOperationsCount = 0;

    UInt _insertionWriteDiskOperationsCount = 0;
    UInt _searchWriteDiskOperationsCount = 0;
    UInt _removingWriteDiskOperationsCount = 0;
    UInt _indexingWriteDiskOperationsCount = 0;
    UInt _indexSearchingWriteDiskOperationsCount = 0;

    UInt _insertionSeekDiskOperationsCount = 0;
    UInt _searchSeekDiskOperationsCount = 0;
    UInt _removingSeekDiskOperationsCount = 0;
    UInt _indexingSeekDiskOperationsCount = 0;
    UInt _indexSearchingSeekDiskOperationsCount = 0;
};

#endif //BTREES_EXPERIMENT_H
