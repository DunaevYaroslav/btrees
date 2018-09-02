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
#include <stdexcept>

#include "btree.h"
#include "experiment.h"

const char CSV_DELIM = ';';

const std::regex csvFileNameRegex("^(\\S*)\\.csv$");

const int CSV_FILE_NAME_REGEX_BEFORE_EXTENSION = 1;

Experiment parseExperiment(const std::string& line);

BaseBTree::TreeType parseTreeType(const std::string& treeTypeString);

void makeExperiment(const Experiment& experiment, std::ofstream& outputFile);

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

    for (std::vector<Experiment>::iterator iter = experiments.begin(); iter != experiments.end(); ++iter)
        makeExperiment(*iter, outputFile);

    outputFile.close();

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

    return Experiment(treeType, treeOrder, treeKeysSize, dataFilePath);
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

void makeExperiment(const Experiment& experiment, std::ofstream& outputFile)
{
    // TODO: implement the method.
}
