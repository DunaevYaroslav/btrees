/// \file
/// \brief     B-tree / B+-tree / B*-tree / B*+-tree file indexer.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      01.05.2017 -- 02.04.2018
///            The course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include "indexer.h"

namespace btree
{

Indexer::Key::Key(const std::wstring& nam) : offset(0)
{
    int len = nam.length() + 1;
    len = len > NAME_LENGTH ? NAME_LENGTH : len;
    memcpy(name, nam.c_str(), len * 2);
}

bool Indexer::NameComparator::compare(const Byte* lhv, const Byte* rhv, UInt sz)
{
    if(lhv == nullptr)
        throw std::invalid_argument("lhv was nullptr");

    if(rhv == nullptr)
        throw std::invalid_argument("rhv was nullptr");

    int i = 0;

    for( ; i < NAME_LENGTH; i += 2)
    {
        wchar_t lchar = lhv[i];
        wchar_t rchar = rhv[i];

        if(lchar == 0 || rchar == 0)
            break;

        if(lchar < rchar)
            return true;

        if(lchar > rchar)
            return false;
    }

    if(i < NAME_LENGTH)
    {
        wchar_t lchar = lhv[i];
        wchar_t rchar = rhv[i];

        if(lchar == 0 && rchar != 0)
            return true;
    }

    return false;
}

bool Indexer::NameComparator::isEqual(const Byte* lhv, const Byte* rhv, UInt sz)
{
    if(lhv == nullptr)
        throw std::invalid_argument("lhv was nullptr");

    if(rhv == nullptr)
        throw std::invalid_argument("rhv was nullptr");

    int i = 0;

    for( ; i < NAME_LENGTH; i += 2)
    {
        wchar_t lchar = lhv[i];
        wchar_t rchar = rhv[i];

        if(lchar == 0 || rchar == 0)
            break;

        if(lchar != rchar)
            return false;
    }

    if(i < NAME_LENGTH)
    {
        wchar_t lchar = lhv[i];
        wchar_t rchar = rhv[i];
        if(lchar != rchar)
            return false;
    }

    return true;
}

Indexer::~Indexer()
{
    close();
}

void Indexer::create(BaseBTree::TreeType treeType, UShort order, const std::string& treeFileName)
{
    if(_bt != nullptr)
        delete _bt;

    _bt = new FileBaseBTree(treeType, order, sizeof(Key), &_comparator, treeFileName);

    lastFileName = "";
}

void Indexer::open(BaseBTree::TreeType treeType, const std::string& treeFileName)
{
    if(_bt != nullptr)
        delete _bt;

    _bt = new FileBaseBTree(treeType, treeFileName, &_comparator);

    lastFileName = "";
}

void Indexer::close()
{
    if(_bt != nullptr)
        delete _bt;

    lastFileName = "";
}

void Indexer::indexFile(const std::string& fileName)
{
    if(_bt == nullptr)
        throw std::logic_error("You should create or open B-tree firstly.\n");

    std::wifstream file;
    file.open(fileName, std::ios_base::binary);

    if(!file.is_open())
        throw std::logic_error("Cannot open file for indexing.\n");

    std::wstring line;

    ULong offset = file.tellg();

    while(std::getline(file, line))
    {
        std::wstringstream sstream(line);

        std::wstring name;
        std::getline(sstream, name, L';');

        Key key(name, offset);

        _bt->getTree()->insert((Byte*) &key);

        offset = file.tellg();
    }

    file.close();

    lastFileName = fileName;
}

std::list<std::wstring> Indexer::findAllOccurrences(const std::wstring& name, const std::string& fileName)
{
    if(_bt == nullptr)
        throw std::logic_error("You should create or open B-tree firstly.\n");

    if(fileName != lastFileName)
        throw std::logic_error("You should index the file firstly.\n");

    std::wifstream file(fileName);

    if(!file.is_open())
        throw std::logic_error("Cannot open file for indexing.\n");

    Key nameKey(name);
    std::list<Byte*> occurrences;
    _bt->getTree()->searchAll((Byte*) &nameKey, occurrences);

    std::list<std::wstring> occurrencesStrings;

    for(std::list<Byte*>::iterator iter = occurrences.begin(); iter != occurrences.end(); ++iter)
    {
        Key* occurrenceKey = (Key*) *iter;

        file.seekg(occurrenceKey->offset, std::ios_base::beg);
        std::wstring occurrenceString;
        std::getline(file, occurrenceString);
        occurrencesStrings.push_back(occurrenceString);

        delete *iter;
    }

    file.close();

    return occurrencesStrings;
}

std::string Indexer::getLine(std::ifstream& file)
{
    std::string line;
    char next;
    int a = file.tellg();
    while(file.read(&next, 1) && next != '\n')
    {
        line.push_back(next);
        file.clear();
        a = file.tellg();
    }


    return line;
}

} // namespace btree
