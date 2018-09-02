/// \file
/// \brief     The Experiment class.
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

#include "btree.h"

using namespace btree;

class Experiment
{

public:

    Experiment(BaseBTree::TreeType treeType, UShort treeOrder, UInt treeKeysSize, std::string dataFilePath)
            : _treeType(treeType), _treeOrder(treeOrder), _treeKeysSize(treeKeysSize),
            _dataFilePath(std::move(dataFilePath)) {}

    Experiment(const Experiment& other) = default;

    Experiment& operator=(Experiment&) = default;

    ~Experiment() = default;

    BaseBTree::TreeType getTreeType() const
    {
        return _treeType;
    }

    UShort getTreeOrder() const
    {
        return _treeOrder;
    }

    UInt getTreeKeysSize() const
    {
        return _treeKeysSize;
    }

    const std::string& getDataFilePath() const
    {
        return _dataFilePath;
    }

private:

    BaseBTree::TreeType _treeType;

    UShort _treeOrder;

    UInt _treeKeysSize;

    std::string _dataFilePath;

};

#endif //BTREES_EXPERIMENT_H
