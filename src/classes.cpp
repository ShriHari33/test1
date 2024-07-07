#include "classes.h"

extern std::fstream listing_file;
extern std::string line;
extern assemblerStatus globalAssemblyStat;

void fill_opTable(OPTABLE& opTab)
{
    opTab["ADD"] = opcode_entry_details(3U, 0x18U);
    opTab["ADDF"] = opcode_entry_details(3U, 0x58U);
    opTab["ADDR"] = opcode_entry_details(2U, 0x90U);
    opTab["AND"] = opcode_entry_details(3U, 0x40U);
    opTab["CLEAR"] = opcode_entry_details(2U, 0xB4U);
    opTab["COMP"] = opcode_entry_details(3U, 0x28U);
    opTab["COMPF"] = opcode_entry_details(3U, 0x88U);
    opTab["COMPR"] = opcode_entry_details(2U, 0xA0U);
    opTab["DIV"] = opcode_entry_details(3U, 0x24U);
    opTab["DIVF"] = opcode_entry_details(3U, 0x64U);
    opTab["DIVR"] = opcode_entry_details(2U, 0x9CU);
    opTab["FIX"] = opcode_entry_details(1U, 0xC4U);
    opTab["FLOAT"] = opcode_entry_details(1U, 0xC0U);
    opTab["HIO"] = opcode_entry_details(1U, 0xF4U);
    opTab["J"] = opcode_entry_details(3U, 0x3CU);
    opTab["JEQ"] = opcode_entry_details(3U, 0x30U);
    opTab["JGT"] = opcode_entry_details(3U, 0x34U);
    opTab["JLT"] = opcode_entry_details(3U, 0x38U);
    opTab["JSUB"] = opcode_entry_details(3U, 0x48U);
    opTab["LDA"] = opcode_entry_details(3U, 0x00U);
    opTab["LDB"] = opcode_entry_details(3U, 0x68U);
    opTab["LDCH"] = opcode_entry_details(3U, 0x50U);
    opTab["LDF"] = opcode_entry_details(3U, 0x70U);
    opTab["LDL"] = opcode_entry_details(3U, 0x08U);
    opTab["LDS"] = opcode_entry_details(3U, 0x6CU);
    opTab["LDT"] = opcode_entry_details(3U, 0x74U);
    opTab["LDX"] = opcode_entry_details(3U, 0x04U);
    opTab["LPS"] = opcode_entry_details(3U, 0xD0U);
    opTab["MUL"] = opcode_entry_details(3U, 0x20U);
    opTab["MULF"] = opcode_entry_details(3U, 0x60U);
    opTab["MULR"] = opcode_entry_details(2U, 0x98U);
    opTab["NORM"] = opcode_entry_details(1U, 0xC8U);
    opTab["OR"] = opcode_entry_details(3U, 0x44U);
    opTab["RD"] = opcode_entry_details(3U, 0xD8U);
    opTab["RMO"] = opcode_entry_details(2U, 0xACU);
    opTab["RSUB"] = opcode_entry_details(3U, 0x4CU);
    opTab["SHIFTL"] = opcode_entry_details(2U, 0xA4U);
    opTab["SHIFTR"] = opcode_entry_details(2U, 0xA8U);
    opTab["SIO"] = opcode_entry_details(1U, 0xF0U);
    opTab["SSK"] = opcode_entry_details(3U, 0xECU);
    opTab["STA"] = opcode_entry_details(3U, 0x0CU);
    opTab["STB"] = opcode_entry_details(3U, 0x78U);
    opTab["STCH"] = opcode_entry_details(3U, 0x54U);
    opTab["STF"] = opcode_entry_details(3U, 0x80U);
    opTab["STI"] = opcode_entry_details(3U, 0xD4U);
    opTab["STL"] = opcode_entry_details(3U, 0x14U);
    opTab["STS"] = opcode_entry_details(3U, 0x7CU);
    opTab["STSW"] = opcode_entry_details(3U, 0xE8U);
    opTab["STT"] = opcode_entry_details(3U, 0x84U);
    opTab["STX"] = opcode_entry_details(3U, 0x10U);
    opTab["SUB"] = opcode_entry_details(3U, 0x1CU);
    opTab["SUBF"] = opcode_entry_details(3U, 0x5CU);
    opTab["SUBR"] = opcode_entry_details(2U, 0x94U);
    opTab["SVC"] = opcode_entry_details(2U, 0xB0U);
    opTab["TD"] = opcode_entry_details(3U, 0xE0U);
    opTab["TIO"] = opcode_entry_details(1U, 0xF8U);
    opTab["TIX"] = opcode_entry_details(3U, 0x2CU);
    opTab["TIXR"] = opcode_entry_details(2U, 0xB8U);
    opTab["WD"] = opcode_entry_details(3U, 0xDCU);
}

void fill_regTable(REGTABLE& regTab)
{
    regTab["A"] = 0;
    regTab["X"] = 1;
    regTab["L"] = 2;
    regTab["B"] = 3;
    regTab["S"] = 4;
    regTab["T"] = 5;
    regTab["F"] = 6;
    regTab["PC"] = 8;
    regTab["SW"] = 9;
}

std::string calculateOperandValue(unsigned OP_CODE_VAL, unsigned OP_CODE_FORMAT, instructionType instrType, long long LABEL_ADDR)
{
    // need the "nixbpe" bits for format 3 and 4 instructions
    std::bitset<6> instructionIdentify{};
    std::ostringstream oss{};

    // we need to set the "nixbpe" bits appropriately for format 3 and 4 instructions

    if(OP_CODE_FORMAT == 3)
    {
        if(instrType == instructionType::INDIRECT_PC_RELATIVE)
            instructionIdentify = 0b100010;
        else if(instrType == instructionType::INDIRECT_BASE_RELATIVE)
            instructionIdentify = 0b100100;
        else if(instrType == instructionType::IMMEDIATE)
            instructionIdentify = 0b010000;
        else if(instrType == instructionType::INDEXED)
            instructionIdentify = 0b111000;
        else if(instrType == instructionType::BASE_RELATIVE)
            instructionIdentify = 0b110100;
        else if(instrType == instructionType::PC_RELATIVE)
            instructionIdentify = 0b110010;
        else if(instrType == instructionType::PC_RELATIVE_INDEXED)
            instructionIdentify = 0b111010;
        else if(instrType == instructionType::BASE_RELATIVE_INDEXED)
            instructionIdentify = 0b111100;
        else if(instrType == instructionType::DIRECT)
            instructionIdentify = 0b110000;
        else if(instrType == instructionType::INVALID)
        {
            listing_file << line << "\t\t\t\tTarget not reachable. Please consider format 4 instruction\n";
            std::cout << line << "\t\t\t\tTarget not reachable. Please consider format 4 instruction\n";

            globalAssemblyStat = assemblerStatus::FAILURE;
            return "";
        }

        oss.clear();
        oss.str("");
        std::bitset<8> temp1(OP_CODE_VAL);
        std::bitset<12> temp2(LABEL_ADDR);

        std::ostringstream okk;
        okk.clear();
        okk.str("");

        okk << temp1;
//        std::cout << "OSS WITH NOTHING: " << oss.str() << '\n';
//        std::cout << "OKK WITH OPCODE_VAL: " << okk.str() << '\n';

        oss << std::setfill('0') << std::setw(6) << okk.str().substr(0, 6);

//        std::cout << "PUSHING THIS AS OPCODE_VAL: " << okk.str().substr(0, 6) << '\n';
//        std::cout << "OPCODE_VAL " << oss.str() << '\n';
//        std::cout << "\n\n";
        okk.clear();
        okk.str("");

        oss << std::setfill('0') << std::setw(6) << instructionIdentify;

        okk << temp2;
//        std::cout << "LABEL_ADDR " << okk.str() << "\n";
        oss << std::setfill('0') << std::setw(12) << okk.str();

        // convert the binary string to hex
        std::string temp = oss.str();
        std::bitset<32> temp3(temp.c_str());
        oss.clear(), oss.str("");

        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << temp3.to_ulong();
    }
    else if(OP_CODE_FORMAT == 4)
    {
        if(instrType == instructionType::EXTENDED_INDIRECT)
            instructionIdentify = 0b100001;
        else if(instrType == instructionType::EXTENDED_IMMEDIATE)
            instructionIdentify = 0b010001;
        else if(instrType == instructionType::EXTENDED_INDEXED)
            instructionIdentify = 0b111001;
        else if(instrType == instructionType::EXTENDED_DIRECT)
            instructionIdentify = 0b110001;
        else
        {
            listing_file << line << "\t\t\t\tTarget not reachable. Please consider format 4 instruction\n";
            std::cout << line << "\t\t\t\tTarget not reachable. Please consider format 4 instruction\n";

            globalAssemblyStat = assemblerStatus::FAILURE;
            return "";
        }

        oss.clear();
        oss.str("");

        std::bitset<8> temp1(OP_CODE_VAL);
        std::bitset<20> temp2(LABEL_ADDR);

        std::ostringstream okk;
        okk.clear();
        okk.str("");

        okk << temp1;
        oss << std::setfill('0') << std::setw(6) << okk.str().substr(0, 6);
        okk.clear(), okk.str("");

        oss << std::setfill('0') << std::setw(6) << instructionIdentify;

        okk << temp2;
        oss << std::setfill('0') << std::setw(20) << okk.str();

        // convert the binary string to hex
        std::string temp = oss.str();
        std::bitset<32> temp3(temp.c_str());
        oss.clear();
        oss.str("");

        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << temp3.to_ulong();
    }
    else if(OP_CODE_FORMAT == 2)
    {
        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << OP_CODE_VAL;
        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << LABEL_ADDR;
    }
    else if(OP_CODE_FORMAT == 1)
    {
        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(1) << OP_CODE_VAL;
    }

    return oss.str();
}