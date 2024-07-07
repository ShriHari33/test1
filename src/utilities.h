#pragma once
#include "classes.h"

OPTABLE opTab;
REGTABLE regTab;
std::vector<SYMTABLE> symTabs;
std::vector<unsigned> lengthOfSections;
std::string progName{};
std::vector<std::string> sectionNames;
size_t ABSOLUTE_START_ADDR = 0;
size_t controlSectionCount = 0;