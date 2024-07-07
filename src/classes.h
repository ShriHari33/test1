#pragma once

#include "iostream"
#include "string"
#include "vector"
#include "map"
#include "algorithm"
#include "fstream"
#include "sstream"
#include "unordered_map"
#include "iomanip"
#include "bitset"

struct opcode_entry_details
{
    unsigned format;
    unsigned opcode_value;

    opcode_entry_details(unsigned int f, unsigned int val)
            : format{f}, opcode_value{val} {}

    opcode_entry_details() = default;
};

struct symbol_entry_details
{
    enum typeOfSymbol
    {
        ABSOLUTE,
        EXTERNAL
    };

    unsigned location;
    typeOfSymbol type;


    symbol_entry_details(unsigned int loc, typeOfSymbol t)
            : location{loc}, type{t} {}


    symbol_entry_details() = default;
};


// ease of use declaration, also easier to read to the eye
using OPTABLE = std::unordered_map<std::string, opcode_entry_details>;
using SYMTABLE = std::unordered_map<std::string, symbol_entry_details>;
using REGTABLE = std::unordered_map<std::string, unsigned>;




// enum to handle proper instruction format & generate the nixbpe bits
enum instructionType
{
    DIRECT = 0,
    INDIRECT_PC_RELATIVE,
    INDIRECT_BASE_RELATIVE,
    IMMEDIATE,
    INDEXED,
    PC_RELATIVE,
    PC_RELATIVE_INDEXED,
    BASE_RELATIVE,
    BASE_RELATIVE_INDEXED,
    EXTENDED,
    EXTENDED_IMMEDIATE,
    EXTENDED_INDIRECT,
    EXTENDED_INDEXED,
    EXTENDED_DIRECT,
    INVALID
};

// need this enum for error handling
enum assemblerStatus
{
    SUCCESS,
    FAILURE
};

// function declarations
void fill_opTable(OPTABLE& opTab);
void fill_regTable(REGTABLE& regTab);
std::string calculateOperandValue(unsigned OP_CODE_VAL, unsigned OP_CODE_FORMAT, instructionType instrType, long long LABEL_ADDR);
