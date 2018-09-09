/// \file
/// \brief     The CSV file generator for B-tree experiments
/// \authors   Anton Rigin
/// \version   0.1
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>

const char CSV_DELIM = ';';

const int CSV_CELL_SIZE = 42;
const int CSV_COLUMNS = 10;
const int CSV_DEFAULT_ROWS = 100000;

const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

const int alphanumSize = sizeof(alphanum) / sizeof(alphanum[0]);

void generateFileWithEqualNames(const int csvRows);

void generateFileWithPartiallyEqualNames(const int csvRows);

void generateFileWithDifferentNames(const int csvRows);

std::string generateCell();

int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        std::cerr << "The count of the command line arguments should be not more than 1"
                  << " - it should be the name of the CSV file with the experiments scheme" << std::endl;

        return -1;
    }

    int csvRows = 0;
    if (argc == 1 || (csvRows = std::atoi(argv[1])) == 0)
    {
        csvRows = CSV_DEFAULT_ROWS;
        std::cout << "Cannot parse the argument or it does not exist, using default CSV rows count: "
                << CSV_DEFAULT_ROWS << std::endl;
    }

    srand(time(NULL));

    generateFileWithEqualNames(csvRows);
    generateFileWithPartiallyEqualNames(csvRows);
    generateFileWithDifferentNames(csvRows);

    std::cout << "Done." << std::endl;

    return 0;
}

void generateFileWithEqualNames(const int csvRows)
{
    std::cout << "Generating CSV file with equal names..." << std::endl;

    std::ofstream csv(std::to_string(csvRows) + std::string("_equal_names.csv"));

    if (!csv.is_open())
    {
        std::cerr << "Cannot open the output file equal_names.csv for writing" << std::endl;

        return;
    }

    std::string name = generateCell();

    for (int i = 0; i < csvRows; ++i)
    {
        csv << name;

        for (int j = 1; j < CSV_COLUMNS; ++j)
            csv << ";" << generateCell();

        csv << std::endl;
    }

    csv.close();
}

void generateFileWithPartiallyEqualNames(const int csvRows)
{
    std::cout << "Generating CSV file with partially equal names..." << std::endl;

    std::ofstream csv(std::to_string(csvRows) + std::string("_partially_equal_names.csv"));

    if (!csv.is_open())
    {
        std::cerr << "Cannot open the output file partially_equal_names.csv for writing" << std::endl;

        return;
    }

    int namesCount = csvRows / 1000;
    std::vector<std::string> names;
    names.reserve(namesCount);

    for (int i = 0; i < namesCount; ++i)
        names.push_back(generateCell());

    for (int i = 0; i < csvRows; ++i)
    {
        csv << names[rand() % names.size()];

        for (int j = 1; j < CSV_COLUMNS; ++j)
            csv << ";" << generateCell();

        csv << std::endl;
    }

    csv.close();
}

void generateFileWithDifferentNames(const int csvRows)
{
    std::cout << "Generating CSV file with different names..." << std::endl;

    std::ofstream csv(std::to_string(csvRows) + std::string("_different_names.csv"));

    if (!csv.is_open())
    {
        std::cerr << "Cannot open the output file different_names.csv for writing" << std::endl;

        return;
    }

    for (int i = 0; i < csvRows; ++i)
    {
        csv << generateCell();

        for (int j = 1; j < CSV_COLUMNS; ++j)
            csv << ";" << generateCell();

        csv << std::endl;
    }

    csv.close();
}

std::string generateCell()
{
    std::string cell;

    for (int i = 0; i < CSV_CELL_SIZE; ++i)
        cell += std::string(1, alphanum[rand() % alphanumSize]);

    return cell;
}
