#include "classes.h"
extern std::string progName;
extern OPTABLE opTab;
extern REGTABLE regTab;
extern std::vector<SYMTABLE> symTabs;
extern std::vector<unsigned> lengthOfSections;
extern size_t controlSectionCount;
extern size_t ABSOLUTE_START_ADDR;

std::fstream listing_file;
std::string line;
assemblerStatus globalAssemblyStat = assemblerStatus::SUCCESS;

bool secondPass()
{
    std::fstream input_file;
    std::fstream obj_file;
    
    input_file.open("intermediate.txt", std::ios::in);
    if(!input_file.is_open())
    {
        std::cout << "Error opening the Intermediate File while performing Pass 2.\nExiting Gracefully";
        std::exit(-1);
    }

    assemblerStatus assemblyStat = assemblerStatus::SUCCESS;

    obj_file.open("output.txt", std::ios::out);
    if(!obj_file.is_open())
    {
        std::cout << "Error opening the Object File while performing Pass 2.\nExiting Gracefully";
        input_file.close();

        std::exit(-1);
    }


    listing_file.open("listing.txt", std::ios::out);
    if(!listing_file.is_open())
    {
        std::cout << "Error opening the Listing File while performing Pass 2.\nExiting Gracefully";
        input_file.close();
        obj_file.close();

        std::exit(-1);
    }

    listing_file << std::right << std::setw(6) << std::setfill('0') << "LOC_CTR\t" <<
                    std::setw(6) << std::setfill('0') << "LABEL\t " <<
                    std::setw(6) << std::setfill('0') << "OPCODE\t    " <<
                    std::setw(6) << std::setfill('0') << "OPERAND\t   " <<
                    std::setw(6) << std::setfill('0') << "OBJECT_CODE\t " << '\n';


    std::string label, opcode, operand;
    size_t START_ADDR = ABSOLUTE_START_ADDR;
    instructionType instrType;

    obj_file    << "H_" << progName + '_' << std::hex
                << std::setw(6) << std::right << std::uppercase << std::setfill('0') << ABSOLUTE_START_ADDR << '_'
                << std::setw(6) << std::right << std::uppercase << lengthOfSections[0] << '\n';

    std::cout   << "\n\nH_" << progName + '_' << std::hex
                << std::setw(6) << std::right << std::uppercase << std::setfill('0') << ABSOLUTE_START_ADDR << '_'
                << std::setw(6) << std::right << std::uppercase << lengthOfSections[0] << '\n';


    std::string loc;
    std::string put_next{};
    std::string text_record{};
    std::vector<std::string> modification_records{};
    
    bool found_first = false;
    size_t space_taken = 0;
    line.clear();

    /*
       * this flag is to allow for not initialising the text record with the START
       * and END records
       *
       * For now, I have just said "to continue" if such a record is found, but i
       * think adding a variable to keep track is much better for code maintenance,
       * so might add it up later!
   */
    controlSectionCount = 0;

    while (std::getline(input_file, line))
    {
        std::istringstream iss(line);

        if(line.empty())
            continue;

        opcode.clear(), operand.clear(), label.clear(), loc.clear(), instrType = instructionType::INVALID;

        if (iss >> loc >> label >> opcode >> operand)
        {
            if(opcode[0] == '+')
                opcode.erase(opcode.begin()), instrType = instructionType::EXTENDED;


            if( opTab.find(opcode) != opTab.end() )
            {
                unsigned OP_CODE_VAL = opTab[opcode].opcode_value;
                long long LABEL_ADDR = 0;
                unsigned OP_CODE_FORMAT = opTab[opcode].format;

                if(instrType == instructionType::EXTENDED)
                    OP_CODE_FORMAT = 4;

                if(OP_CODE_FORMAT == 3 || OP_CODE_FORMAT == 4)
                {
                    if(operand[0] == '#')
                    {
                        // we can have a mixture of format 4, and immediate addressing mode.
                        // so we need to handle that.
                        operand.erase(operand.begin());

                        if(instrType == instructionType::EXTENDED)
                            instrType = instructionType::EXTENDED_IMMEDIATE;
                        else
                            instrType = instructionType::IMMEDIATE;

                        // decimal operand, so we dont need hex conversion
                        LABEL_ADDR = std::stoll(operand);
                    }
                    else if(operand[0] == '@')
                    {
                        // indirect addressing mode
                        operand.erase(operand.begin());

                        /*
                         * I have not incorporated "Relative addressing with indirect".
                         * The book does mention that indirect addressing is done with relative addressing done in the
                         * first place for format 3 instructions, but it doesn't explain for format 4.
                         *
                         * I think that logically, format 4 instructions just take in the whole address. Like they got
                         * the space to hold the 20 bit address, so we dont really need the relative addressing mode.
                         *
                        */

                        if(symTabs[controlSectionCount].find(operand) != symTabs[controlSectionCount].end())
                        {
                            LABEL_ADDR = symTabs[controlSectionCount][operand].location;

                            if(OP_CODE_FORMAT == 3)
                            {
                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    listing_file << line << '\t' << "Cannot refer an external variable in a format 3 instruction.\n";
                                    std::cout << line << "Cannot refer an external variable in a format 3 instruction.\n";
                                    
                                    globalAssemblyStat = assemblerStatus::FAILURE;
                                    continue;
                                }
                                else
                                {
                                    auto displacement =
                                            LABEL_ADDR - (std::stoll(loc, NULL, 16) + OP_CODE_FORMAT);

                                    if(displacement >= -2047 && displacement <= 2048)
                                        instrType = instructionType::INDIRECT_PC_RELATIVE;
                                    else if(displacement >= 0 && displacement <= 4096)
                                        instrType = instructionType::INDIRECT_BASE_RELATIVE;
                                    else
                                        instrType = instructionType::INVALID;

                                    LABEL_ADDR = displacement;
                                }
                            }
                            // We dont check for `OP_CODE_FORMAT == 4` because we did that above itself.
                            // We just need to check `OP_CODE_FORMAT == 3`, and we did that above.
                            if(instrType == instructionType::EXTENDED)
                            {
                                LABEL_ADDR = symTabs[controlSectionCount][operand].location;

                                /*
                                 * Only if the instruction is referring to an external record, i add the modification record.
                                 * This should make sense because, if it was referring to a local symbol, then the LABEL_ADDR
                                 * would have been the actual address of the symbol, and not 0.
                                */

                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    std::ostringstream oss;
                                    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (std::stoll(loc, NULL, 16) + 1);
                                    modification_records.push_back("M_" + oss.str() + "_05_+" + operand);
                                }

                                instrType = instructionType::EXTENDED_INDIRECT;
                            }
                        }
                        else
                        {
                            listing_file << line << '\t' << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at location: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                        // indexed mode
                    else if(operand.substr(operand.length() - 2, operand.length()) == ",X")
                    {
                        // indexed addressing mode
                        operand.erase(operand.end() - 2, operand.end());

                        // if(instrType == instructionType::EXTENDED)
                        // 	instrType = instructionType::EXTENDED_INDEXED;
                        // else
                        // 	instrType = instructionType::INDEXED;

                        if (symTabs[controlSectionCount].find(operand) != symTabs[controlSectionCount].end())
                        {
                            LABEL_ADDR = symTabs[controlSectionCount][operand].location;

                            /*
                                  so we have to make 1 of 2 choices here:
                                  1. if we have a format 3 instruction with indexed addressing mode,
                                          we need to send in the displacement value as the "LABEL_ADDR"
                                  2. if we have a format 4 instruction with indexed addressing mode,
                                          we need to send in the address of the operand as the "LABEL_ADDR",
                                          as we are not assuming relative modes for format 4 as of now.
                            */

                            if (OP_CODE_FORMAT == 3)
                            {
                                auto displacement =
                                        LABEL_ADDR - (std::stoll(loc, NULL, 16) + OP_CODE_FORMAT);

                                if (displacement >= -2047 && displacement <= 2048)
                                    instrType = instructionType::PC_RELATIVE_INDEXED;
                                else if (displacement >= 0 && displacement <= 4096)
                                    instrType = instructionType::BASE_RELATIVE_INDEXED;
                                else
                                    instrType = instructionType::INVALID;

                                LABEL_ADDR = displacement;
                            }
                                // if i decide to include relative with indexing in format 4, then the logic will
                                // go here below. For now, it's just doing indexing. No Relative + Indexing. We dont do any
                                // modification with "LABEL_ADDR" here because we already got it above
                            else if (OP_CODE_FORMAT == 4)
                            {
                                instrType = instructionType::EXTENDED_INDEXED;

                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    std::ostringstream oss;
                                    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (std::stoll(loc, NULL, 16) + 1);
                                    modification_records.push_back("M_" + oss.str() + "_05_+" + operand);
                                }
                            }
                        }
                        else
                        {
                            listing_file << line << '\t' << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                        // this is for the case where we don't have any addressing mode specified, and we just have a label
                        // with us. like :
                        // 				- 1049 LABEL STL  RETADR"
                        //						    or
                        // 				- 1049 LABEL +STL  RETADR
                    else
                    {
                        if(OP_CODE_FORMAT == 3)
                        {
                            if(symTabs[controlSectionCount].find(operand) != symTabs[controlSectionCount].end())
                            {
                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::ABSOLUTE)
                                    LABEL_ADDR = symTabs[controlSectionCount][operand].location;
                                else if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    listing_file << line << "\t\t\t\t" << "Cannot refer an external variable in a format 3 instruction.\n";
                                    std::cout << "Cannot refer an external variable in a format 3 instruction. Bad practice\n";

                                    globalAssemblyStat = assemblerStatus::FAILURE;
                                    continue;
                                }
                            }

                            else
                            {
                                listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                                std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                                globalAssemblyStat = assemblerStatus::FAILURE;
                                continue;
                            }

                            // calculate the displacement.
                            // the problem is we need the address of the next instruction, which we don't have.
                            // but we can calculate it by adding {our location (VAR loc) } + {op_code_format}.

                            // hopefully this is correct. In theory, it looks perfectly sensible & logical.
                            auto displacement =
                                    LABEL_ADDR - (std::stoll(loc, NULL, 16) + OP_CODE_FORMAT);

                            if(displacement >= -2047 && displacement <= 2048)
                            {
                                // PC relative mode.
                                // i currently dont handle cases where we can also potentially have a
                                // format 4 instruction with relative addressing. that seems like a
                                // bit too much work for now. We will do it later.
                                instrType = instructionType::PC_RELATIVE;
                            }
                            else if(displacement >= 0 && displacement <= 4096)
                            {
                                // base relative mode
                                instrType = instructionType::BASE_RELATIVE;
                            }
                            else
                                instrType = instructionType::INVALID;

                            LABEL_ADDR = displacement;
                        }
                        else if(OP_CODE_FORMAT == 4)
                        {
                            instrType = instructionType::EXTENDED_DIRECT;

                            if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::ABSOLUTE)
                                LABEL_ADDR = symTabs[controlSectionCount][operand].location;
                            else if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                LABEL_ADDR = 0;
                            else
                            {
                                listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                                std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                                globalAssemblyStat = assemblerStatus::FAILURE;
                                continue;
                            }


                            if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                            {
                                std::ostringstream oss;
                                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (std::stoll(loc, NULL, 16) + 1);
                                modification_records.push_back("M_" + oss.str() + "_05_+" + operand);
                            }
                            // no displacement calculation here. I am not considering the case for
                            // format 4 instructions with relative addressing. too much work for now
                        }
                    }
                }
                else if(OP_CODE_FORMAT == 2)
                {
                    // we don't need to do anything here, as we don't need to calculate
                    // the operand value for format 2 instructions. They are register operations,
                    // we need to just write the register numbers to the file. We plan to create a
                    // map of register names to their numbers.

                    /*
                        so such an instruction if used, would look like:
                        {ADDRESS}	{LABEL_NAME}	CLEAR	X
                        i have not seen a label attached to format 2 instructions, but you do you.
                    */

                    if(operand.find(',') != std::string::npos)
                    {
                        /*
                              we might have:
                                      - a pair of registers (as most of instructions have that)
                                      - a register and a constant (as in SHIFTL A, 2)
                        */

                        if( regTab.find(std::string(1, operand[0])) != regTab.end() &&
                            regTab.find(std::string(1, operand[2])) != regTab.end() )
                        {
                            // we have a pair of registers. We also need to check if the instruction is even
                            // valid, because having "CLEAR A,X" gets us in here, but it is NOT VALID.

                            if( opcode == "CLEAR" || opcode == "TIXR" || opcode == "SVC" )
                            {
                                listing_file << line << "\t\t\t\t" << "Invalid Instruction format found. Cannot have 2 Registers in such an instruction.\n";
                                std::cout << "INVALID INSTRUCTION " << opcode << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                                globalAssemblyStat = assemblerStatus::FAILURE;
                                continue;
                            }
                            LABEL_ADDR = regTab[std::string(1, operand[0])] << 4;
                            LABEL_ADDR += regTab[std::string(1, operand[2])];
                        }

                        else if( ( regTab.find(std::string(1, operand[0])) != regTab.end() ) &&
                                 ( std::stoul(operand.substr(2, operand.length()) ) <= 15 ) )
                        {
                            // we have a register and a constant
                            LABEL_ADDR = 0;

                            LABEL_ADDR = regTab[std::string(1,operand[0])] << 4;
                            LABEL_ADDR += std::stoll(operand.substr(2, operand.length()));
                        }
                        else
                        {
                            listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                    else
                    {
                        // we have a single register. Only in SHIFTL and SHIFTR instructions
                        LABEL_ADDR = 0;

                        if( regTab.find(std::string(1,operand[0])) != regTab.end() )
                            LABEL_ADDR = regTab[std::string(1,operand[0])] << 4;
                        else
                        {
                            listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                }
                else if(OP_CODE_FORMAT == 1)
                {
                    // if we enter this scope, code is buggy.
                    // because at max, we can have such an instruction as:
                    // "{ADDRESS}	{LABEL_NAME}	TIO"

                    // so, we will enter some place where the operand is empty.

                    listing_file << line << "\t\t\t\t" << "No such instruction exists.\n";
                    std::cout << "Why did you enter this?\nA format 1 instruction with 4 fields "
                                 "in intermediate file? Blasphemy. It was in the instruction " << line + '\n';

                    globalAssemblyStat = assemblerStatus::FAILURE;
                    continue;
                }
//                else
//                {
//                    std::cout << "INVALID OP_CODE_FORMAT FOUND IN PASS 2. SHOULD NOT HAVE HAPPENED! "
//                                    "It was in the instruction " << line + '\n';
//                    std::exit(-1);
//                }

                // logic to write the proper operand value to the file depending on the format &
                // addressing mode used

                // std::cout << "\nI reached here?\n";
                // std::cout << opcode << '\t' << "( " << OP_CODE_FORMAT << ", " << OP_CODE_VAL << ", " << instrType
                //                     << " )"<< '\t' << operand << '\t' << LABEL_ADDR << "\n\n";

                std::string retVal = calculateOperandValue(OP_CODE_VAL, OP_CODE_FORMAT, instrType, LABEL_ADDR);

                listing_file << line << '\t' << retVal << '\n';


                if (space_taken + OP_CODE_FORMAT <= 30)
                    put_next += '_' + retVal, space_taken += OP_CODE_FORMAT;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << space_taken;
                    std::string space_taken_string = sp.str();

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    std::cout << text_record << std::endl;
                    obj_file << text_record << std::endl;

                    text_record.clear();
                    put_next.clear();

                    put_next = '_' + retVal;
                    space_taken = OP_CODE_FORMAT;
                    found_first = false;
                }

            }
            else if(opcode == "BYTE")
            {
                std::string token{};
                std::ostringstream okk;
                okk.clear(), okk.str("");

                // loop_check = true;
                if (operand[0] == 'C')
                {
                    // ex: 42069 ARRAY BYTE C'GITHUB'
                    int i = 2;
                    do {
                        okk << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(operand[i]);
                        i++;
                    } while (operand[i] != '\'');

                    // using a temp string token for much better readability, to allow
                    // checks later when space_taken checks are made

                    // "operand.length() - 3" because we need to neglect the C, ' and '
                    token = okk.str();

                    if ((space_taken + operand.length() - 3) <= 30)
                        put_next += '_' + token, space_taken += (operand.length() - 3);
                    else
                    {
                        // write the current record, and re-initialize
                        std::ostringstream sp;
                        sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                        std::string space_taken_string = sp.str();

                        text_record.append(space_taken_string);
                        text_record.append(put_next);

                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;

                        text_record.clear();
                        put_next.clear();

                        put_next = '_' + token;
                        space_taken = operand.length() - 3;
                        found_first = false;
                    }
                }
                else if(operand[0] == 'X')
                {
                    /*
                       * ex: 42069 VAL BYTE X'1b'
                       *
                       * Currently, this accepts hex consts only as a pair of numbers, i.e,
                       * the length is even. Not a big deal to change it to accept odd
                       * lengths too, so will change it later.
                    */

                    /*
                     * reuse the string created "token" in the same scope, instead of a
                     * new one. MFW I don't want to see clang-tidy warning lmao
                     */

                    token.clear();

                    // n -> NUMBER of chars to be collected, NOT the last index to be retreived
                    token = operand.substr(2, operand.length() - 3);

                    /*
                     * I am doing this below because i dont want to over-allocate to the memory.
                     *
                     * What i mean by this is, lets consider a case where we have the operand as " X'00001b' ".
                     * Now, i cant allocate 3 bytes just because the length is 3. I need to allocate only 1 byte, because
                     * the trailing zeroes mean nothing.
                     *
                     * Maybe the user wrote it as such because of some standard they were following. So, i first convert
                     * to hex, then do the width adjustment accordingly
                    */

                    size_t token_val = std::stoi(token, nullptr, 16);
                    std::ostringstream okk;
                    okk << std::uppercase << std::hex << token_val;

                    std::string token_val_str = okk.str();

                    std::ostringstream oss;
                    size_t width = (token_val_str.length() % 2 == 0) ?
                                   (token_val_str.length()) : (token_val_str.length() + 1);

                    // std::clog << "what? " << line + "\n" << width << '\n';

                    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(int(width)) << token_val;

                    // length of the hex stuff in bytes is " ( operand.length() - 3 ) / 2
                    token = oss.str();

                    if ( (space_taken + (width / 2)) <= 30)
                        put_next += '_' + token, space_taken += (width / 2);
                    else
                    {
                        // write the current record, and re-initialize
                        std::ostringstream sp;
                        sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                        std::string space_taken_string = sp.str();

                        text_record.append(space_taken_string);
                        text_record.append(put_next);

                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;

                        text_record.clear();
                        put_next.clear();

                        put_next = '_' + token;
                        space_taken = width / 2;
                        found_first = false;
                    }
                }
            }
            else if(opcode == "WORD")
            {
                std::ostringstream oss;
                oss << std::hex << std::setfill('0') << std::setw(6) << std::stoi(operand);
                std::string token = oss.str();

                if (space_taken + 3 <= 30)
                    put_next += '_' + token, space_taken += 3;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                    std::string space_taken_string = sp.str();

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    std::cout << text_record << std::endl;
                    obj_file << text_record << std::endl;

                    text_record.clear();
                    put_next.clear();

                    put_next = '_' + token;
                    space_taken = 3;
                    found_first = false;
                }
            }
            else if(opcode == "RESW" || opcode == "RESB")
            {
                // write the current record, and re-initialize
                if (space_taken == 0)
                    continue;

                std::ostringstream sp;
                sp << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << space_taken;
                std::string space_taken_string = sp.str();

                text_record.append(space_taken_string);
                text_record.append(put_next);

                std::cout << text_record << std::endl;
                obj_file << text_record << std::endl;

                text_record.clear();
                put_next.clear();

                space_taken = 0;
                found_first = false;

                /*
                 * we put continue because we want the "next" record to be taken as the first one.
                 * in other cases, we could not accomodate the current record, and hence we jumped to the below
                 * "!found_first" clause. But in this case, we want to ensure that the next record is taken as the
                 * first one, as this record is just a RESB / RESW directive.
                */
                continue;
            }
            else if (loc == "//")
            {
                /*
                 * If entered this scope, you have encountered an "ERROR". Either due to
                 * an invalid opcode, or due to a duplicate label. What do we do in such
                 * a situation?
                 *
                 * I have no idea as of now, but for now, I will just add a size of 3,
                 * and write "error" to indicate an error.
                 */

                if (space_taken + 3 <= 30)
                    put_next += "_ERROR", space_taken += 3;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                    std::string space_taken_string = sp.str();

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    std::cout << text_record << std::endl;
                    obj_file << text_record << std::endl;

                    text_record.clear();
                    put_next.clear();

                    put_next = "_ERROR";
                    space_taken = 3;
                    found_first = false;

                    /*
                     * next instruction will be taken as the first one in the next iteration. I dont want to consider
                     * this instruction as the first one, because it is an error. But we will be writing it to
                     * the object file.
                     * I think this is wrong, as we are putting an "ERROR" in the object file, but saying that the next
                     * record is to be taken as the starting point for the text record. Hopefully we don't ever enter
                     * this scope
                    */

                    continue;
                }
            }
        }
        else if(operand.empty() && opcode.empty())
        {
            /*
             * instructions that come under such scenarios are:
             *  - RSUB
             *  - FIX
             *  - FLOAT
             *  - HIO
             *  - NORM
             *  - SIO
             *  - TIO
             *
             *  Here, RSUB is the only instruction that is of format 3/4. Rest all are format 1.
             */

            // VAR loc has location, so we don't update it below

            opcode = label;
            if(opcode[0] == '+')
                opcode.erase(opcode.begin()), instrType = instructionType::EXTENDED;

            if(opTab.find(opcode) != opTab.end())
            {
                unsigned OP_CODE_VAL = opTab[opcode].opcode_value;
                unsigned OP_CODE_FORMAT = opTab[opcode].format;
                long long LABEL_ADDR = 0;

                if(instrType == instructionType::EXTENDED)
                    OP_CODE_FORMAT = 4;


                if(opcode == "RSUB")
                {
                    if(instrType == instructionType::EXTENDED)
                        instrType = instructionType::EXTENDED_DIRECT;
                    else
                        instrType = instructionType::DIRECT;
                }
                // this implies that all the other instructions are of format 1, so we dont need explicit check
                // because we already came inside this because it was a valid opcode, and we will go into the
                // else clause because it wasn't RSUB. So it has to be one of those format 1 instructions.
                else
                {
                    if(instrType == instructionType::EXTENDED)
                    {
                        listing_file << line << "\t\t\t\t" << "Assembling a format 1 instruction as a format 4 one. Not allowed.\n";
                        std::cout << "Assembling a format 1 instruction as a format 4. Bad practice\n" << line << '\n';

                        globalAssemblyStat = assemblerStatus::FAILURE;
                        continue;
                        
                    }
                    else
                        instrType = instructionType::DIRECT;
                }

                // logic to write the proper operand value to the file depending on the format &
                // addressing mode used

                // We send the LABEL_ADDR as 0 because these instructiosn don't need any of that. The hardware
                // just relies on the opcode, and works flawlessly
                std::string retVal = calculateOperandValue(OP_CODE_VAL, OP_CODE_FORMAT, instrType, LABEL_ADDR);

                listing_file << line << "\t\t\t" << retVal << '\n';


                if ( (space_taken + OP_CODE_FORMAT <= 30) )
                    put_next += '_' + retVal, space_taken += OP_CODE_FORMAT;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                    std::string space_taken_string = sp.str();

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    if(globalAssemblyStat != assemblerStatus::FAILURE)
                    {
                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;
                    }

                    text_record.clear();
                    put_next.clear();

                    put_next = '_' + retVal;
                    space_taken = OP_CODE_FORMAT;
                    found_first = false;
                }

                // std::cout << "got this: " << retVal << '\n';
            }
            else
            {
                listing_file << line << "\t\t\t\t" << "Invalid Opcode " << opcode << " found.\n";
                std::cout << "INVALID OPCODE FOUND IN PASS 2. SHOULD NOT HAVE HAPPENED! "
                             "It was in the instruction " << line + '\n' << " at line: " << loc << '\n';

                globalAssemblyStat = assemblerStatus::FAILURE;
                continue;
            }
        }
        else if(operand.empty())
        {
            /*
             * What all can be in this scope?
             *  - no "actual" label instructions ("LDA ZERO")    => ("{loc} LDA ZERO")
             *  - control Section definitions    ("RDREC CSECT") => ("{loc} RDREC CSECT")
             *
             * NOTE: Someone could also write something like "WORD 42069", meaning that it doesn't have a label at all.
             *       but c'mon, who would do that? because we need to have label to it to be able to address it.
             *       it's like, a named variable. So if you are someone who would do that, then you need a reality check.
             *
             * jesus, we could save so much boilerplate code if we just remove the label for all instructions.
             * We dont really need them anyway, so why bother?
             *
             * omw to duplicate the code. I shall do the above later.
             */

            operand = opcode;
            opcode = label;

            if(opcode[0] == '+')
                opcode.erase(opcode.begin()), instrType = instructionType::EXTENDED;


            if( opTab.find(opcode) != opTab.end() )
            {
                unsigned OP_CODE_VAL = opTab[opcode].opcode_value;
                long long LABEL_ADDR = 0;
                unsigned OP_CODE_FORMAT = opTab[opcode].format;

                if(instrType == instructionType::EXTENDED)
                    OP_CODE_FORMAT = 4;

                if(OP_CODE_FORMAT == 3 || OP_CODE_FORMAT == 4)
                {
                    if(operand[0] == '#')
                    {
                        // we can have a mixture of format 4, and immediate addressing mode.
                        // so we need to handle that.
                        operand.erase(operand.begin());

                        if(instrType == instructionType::EXTENDED)
                            instrType = instructionType::EXTENDED_IMMEDIATE;
                        else
                            instrType = instructionType::IMMEDIATE;

                        LABEL_ADDR = std::stoll(operand);
                    }
                    else if(operand[0] == '@')
                    {
                        // indirect addressing mode
                        operand.erase(operand.begin());

                        /*
                         * I have not incorporated "Relative addressing with indirect".
                         * The book does mention that indirect addressing is done with relative addressing done in the
                         * first place for format 3 instructions, but it doesn't explain for format 4.
                         *
                         * I think that logically, format 4 instructions just take in the whole address. Like they got
                         * the space to hold the 20 bit address, so we dont really need the relative addressing mode.
                         *
                        */

                        if(symTabs[controlSectionCount].find(operand) != symTabs[controlSectionCount].end())
                        {
                            LABEL_ADDR = symTabs[controlSectionCount][operand].location;

                            if(OP_CODE_FORMAT == 3)
                            {
                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    listing_file << line << "\t\t\t\t" << "Cannot refer an external variable in a format 3 instruction.\n";
                                    std::cout << "Cannot refer an external variable in a format 3 instruction. Bad practice\n";
                                }
                                else
                                {
                                    auto displacement =
                                            LABEL_ADDR - (std::stoll(loc, NULL, 16) + OP_CODE_FORMAT);

                                    if(displacement >= -2047 && displacement <= 2048)
                                        instrType = instructionType::INDIRECT_PC_RELATIVE;
                                    else if(displacement >= 0 && displacement <= 4096)
                                        instrType = instructionType::INDIRECT_BASE_RELATIVE;
                                    else
                                        instrType = instructionType::INVALID;

                                    LABEL_ADDR = displacement;
                                }
                            }
                            /*
                             * if we have a format 4 instruction, we just make the `LABEL_ADDR` to point to
                             * whatever the symTab holds. We dont need to do any displacement calculation, as i
                             * mentioned above
                            */
                            if(instrType == instructionType::EXTENDED)
                            {
                                LABEL_ADDR = symTabs[controlSectionCount][operand].location;
                                instrType = instructionType::EXTENDED_INDIRECT;

                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    std::ostringstream oss;
                                    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (std::stoll(loc, NULL, 16) + 1);
                                    modification_records.push_back("M_" + oss.str() + "_05_+" + operand);
                                }
                            }
                        }
                    }
                        // indexed mode
                    else if(operand.substr(operand.length() - 2, operand.length()) == ",X")
                    {
                        // indexed addressing mode
                        operand.erase(operand.end() - 2, operand.end());

                        // if(instrType == instructionType::EXTENDED)
                        // 	instrType = instructionType::EXTENDED_INDEXED;
                        // else
                        // 	instrType = instructionType::INDEXED;

                        if (symTabs[controlSectionCount].find(operand) != symTabs[controlSectionCount].end())
                        {
                            LABEL_ADDR = symTabs[controlSectionCount][operand].location;

                            /*
                                  so we have to make 1 of 2 choices here:
                                  1. if we have a format 3 instruction with indexed addressing mode,
                                          we need to send in the displacement value as the "LABEL_ADDR"
                                  2. if we have a format 4 instruction with indexed addressing mode,
                                          we need to send in the address of the operand as the "LABEL_ADDR",
                                          as we are not assuming relative modes for format 4 as of now.
                            */

                            if (OP_CODE_FORMAT == 3)
                            {
                                auto displacement =
                                        LABEL_ADDR - (std::stoll(loc, NULL, 16) + OP_CODE_FORMAT);

                                if (displacement >= -2047 && displacement <= 2048)
                                    instrType = instructionType::PC_RELATIVE_INDEXED;
                                else if (displacement >= 0 && displacement <= 4096)
                                    instrType = instructionType::BASE_RELATIVE_INDEXED;
                                else
                                    instrType = instructionType::INVALID;

                                LABEL_ADDR = displacement;
                            }
                                /*
                                 * if i decide to include relative with indexing in format 4, then the logic will
                                 * go here below. For now, it's just doing indexing. No Relative + Indexing. We dont do any
                                 * modification with "LABEL_ADDR" here because we already got it above
                                */
                            else if (OP_CODE_FORMAT == 4)
                            {
                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    std::ostringstream oss;
                                    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (std::stoll(loc, NULL, 16) + 1);
                                    modification_records.push_back("M_" + oss.str() + "_05_+" + operand);
                                }

                                instrType = instructionType::EXTENDED_INDEXED;
                            }
                        }
                        else
                        {
                            listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                        // this is for the case where we don't have any addressing mode specified, and we just have a label
                        // with us. like :
                        // 				- 1049 LABEL STL  RETADR"
                        //						    or
                        // 				- 1049 LABEL +STL  RETADR
                    else
                    {
                        if(OP_CODE_FORMAT == 3)
                        {
                            if(symTabs[controlSectionCount].find(operand) != symTabs[controlSectionCount].end())
                            {
                                if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::ABSOLUTE)
                                    LABEL_ADDR = symTabs[controlSectionCount][operand].location;
                                else if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                {
                                    listing_file << line << "\t\t\t\t" << "Cannot refer an external variable in a format 3 instruction.\n";
                                    std::cout << "Cannot refer an external variable in a format 3 instruction. Bad practice\n";

                                    globalAssemblyStat = assemblerStatus::FAILURE;
                                    continue;
                                }
                            }
                            else
                            {
                                listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                                std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                                globalAssemblyStat = assemblerStatus::FAILURE;
                                continue;
                            }

                            // calculate the displacement.
                            // the problem is we need the address of the next instruction, which we don't have.
                            // but we can calculate it by adding {our location (VAR loc) } + {op_code_format}.

                            // hopefully this is correct. In theory, it looks perfectly sensible & logical.
                            auto displacement =
                                    LABEL_ADDR - (std::stoll(loc, NULL, 16) + OP_CODE_FORMAT);

                            if(displacement >= -2047 && displacement <= 2048)
                            {
                                // PC relative mode.
                                // i currently dont handle cases where we can also potentially have a
                                // format 4 instruction with relative addressing. that seems like a
                                // bit too much work for now. We will do it later.
                                instrType = instructionType::PC_RELATIVE;
                            }
                            else if(displacement >= 0 && displacement <= 4096)
                            {
                                // base relative mode
                                instrType = instructionType::BASE_RELATIVE;
                            }
                            else
                                instrType = instructionType::INVALID;

                            LABEL_ADDR = displacement;
                        }
                        else if(OP_CODE_FORMAT == 4)
                        {
                            if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::ABSOLUTE)
                                LABEL_ADDR = symTabs[controlSectionCount][operand].location;
                            else if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                                LABEL_ADDR = 0;
                            else
                            {
                                listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                                std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                                globalAssemblyStat = assemblerStatus::FAILURE;
                                continue;
                            }

                            if(symTabs[controlSectionCount][operand].type == symbol_entry_details::typeOfSymbol::EXTERNAL)
                            {
                                std::ostringstream oss;
                                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (std::stoll(loc, NULL, 16) + 1);
                                modification_records.push_back("M_" + oss.str() + "_05_+" + operand);
                            }

                            instrType = instructionType::EXTENDED_DIRECT;
                            // no displacement calculation here. I am not considering the case for
                            // format 4 instructions with relative addressing. too much work for now
                        }
                    }
                }
                else if(OP_CODE_FORMAT == 2)
                {
                    /*
                     * we don't need to do anything here, as we don't need to calculate
                     * the operand value for format 2 instructions. They are register operations,
                     * we need to just write the register numbers to the file. We plan to create a
                     * map of register names to their numbers.

                     * so such an instruction if used, would look like:
                     * {ADDRESS}	{LABEL_NAME}	CLEAR	X
                     * i have not seen a label attached to format 2 instructions, but you do you.
                    */

                    // dummy value for now
                    if(operand.find(',') != std::string::npos)
                    {
                        /*
                              we might have:
                                      - a pair of registers (as most of instructions have that)
                                      - a register and a constant (as in SHIFTL A, 2)
                        */
                        
                        if( regTab.find(std::string(1, operand[0])) != regTab.end() &&
                            regTab.find(std::string(1, operand[2])) != regTab.end() )
                        {
                            // we have a pair of registers. We also need to check if the instruction is even
                            // valid, because having "CLEAR A,X" gets us in here, but it is NOT VALID.

                            if( opcode == "CLEAR" || opcode == "TIXR" || opcode == "SVC" )
                            {
                                listing_file << line << "\t\t\t\t" << "Invalid Instruction format found. Cannot have 2 Registers in such an instruction.\n";
                                std::cout << "INVALID INSTRUCTION " << opcode << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                                globalAssemblyStat = assemblerStatus::FAILURE;
                                continue;
                            }
                            LABEL_ADDR = regTab[std::string(1,operand[0])] << 4;
                            LABEL_ADDR += regTab[std::string(1,operand[2])];
                        }

                        else if( regTab.find(std::string(1, operand[0])) != regTab.end() &&
                                 std::stoul(operand.substr(2, operand.length())) <= 15 )
                        {
                            // we have a register and a constant
                            LABEL_ADDR = 0;

                            LABEL_ADDR = regTab[std::string(1, operand[0])] << 4;
                            LABEL_ADDR += std::stoll(operand.substr(2, operand.length()));
                        }
                        else
                        {
                            listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                    else
                    {
                        /*
                         * We have a single register involved here.
                         * Instructions that follow this are:
                         *      - SHIFTL and SHIFTR instructions
                         *      - CLEAR instruction
                        */
                        LABEL_ADDR = 0;

                        if( regTab.find(std::string(1, operand[0])) != regTab.end() )
                            LABEL_ADDR = regTab[std::string(1, operand[0])] << 4;
                        else
                        {
                            //std::cout << "OPERAND: " << operand.length() << '\n';
                            listing_file << line << "\t\t\t\t" << "Invalid Operand " << operand << " found.\n";
                            std::cout << "INVALID OPERAND " << operand << " FOUND IN PASS 2 at line: " << loc << ". SHOULD NOT HAVE HAPPENED!";

                            globalAssemblyStat = assemblerStatus::FAILURE;
                            continue;
                        }
                    }
                }
                else if(OP_CODE_FORMAT == 1)
                {
                    /*
                     * if we enter this scope, code is buggy.
                     * because at max, we can have such an instruction as:
                     *      "{ADDRESS}	TIO"
                     *
                     * so, we will enter some place where the operand is empty.
                    */
                    listing_file << line << "\t\t\t\t" << "No such instruction exists.\n";
                    std::cout << "Why did you enter this?\nA format 1 instruction with 3 fields "
                                 "in intermediate file? Blasphemy. It was in the instruction " << line + '\n';
                    
                    globalAssemblyStat = assemblerStatus::FAILURE;
                    continue;
                }
                //else
                //{
                //    std::cout << "INVALID OP_CODE_FORMAT FOUND IN PASS 2. SHOULD NOT HAVE HAPPENED! "
                //                    "It was in the instruction " << line + '\n';
                //    std::exit(-1);
                //}

                // logic to write the proper operand value to the file depending on the format &
                // addressing mode used

                // std::cout << "\nI reached here?\n";
                // std::cout << opcode << '\t' << "( " << OP_CODE_FORMAT << ", " << OP_CODE_VAL << ", " << instrType
                //                      << " )"<< '\t' << operand << '\t' << LABEL_ADDR << "\n\n";

                

                std::string retVal = calculateOperandValue(OP_CODE_VAL, OP_CODE_FORMAT, instrType, LABEL_ADDR);
                listing_file << line << '\t' << retVal << '\n';


                if (space_taken + OP_CODE_FORMAT <= 30)
                    put_next += '_' + retVal, space_taken += OP_CODE_FORMAT;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                    std::string space_taken_string = sp.str();

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    if(globalAssemblyStat != assemblerStatus::FAILURE)
                    {
                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;
                    }

                    text_record.clear();
                    put_next.clear();

                    put_next = '_' + retVal;
                    space_taken = OP_CODE_FORMAT;
                    found_first = false;
                }
                // std::cout << "got this: " << retVal << '\n';
            }
            else if(operand == "CSECT")
            {
                /*
                 * If we enter this, it implies that we are starting a new control section.
                 *
                 * We push an END record to the previous CS, and then start a new header / text for the new CS.
                 * Let's see how we are going to do it
                 */

                if(space_taken != 0)
                {
                    // we need to push the current text record for the previous CS to the object file
                    std::ostringstream sp;
                    sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
                    std::string space_taken_string = sp.str();

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    if(globalAssemblyStat != assemblerStatus::FAILURE)
                    {
                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;
                    }
                }

                if(controlSectionCount == 0)
                    START_ADDR = ABSOLUTE_START_ADDR;
                else
                    START_ADDR = 0;

                controlSectionCount++;

                // put the modification records now
                
                if(globalAssemblyStat != assemblerStatus::FAILURE)
                {
                    for(auto &i : modification_records)
                    {
                        std::cout << i << '\n';
                        obj_file << i << '\n';
                    }
                }

                modification_records.clear();


                if(globalAssemblyStat != assemblerStatus::FAILURE)
                {
                    // put the end records now
                    std::cout << "E_" << std::hex << std::setw(6) << std::setfill('0') << std::right << START_ADDR << "\n\n";
                    obj_file  << "E_" << std::hex << std::setw(6) << std::setfill('0') << std::right << START_ADDR << "\n\n";



                    // now we re-initialise header records
                    std::cout << "H_" << std::setw(6) << std::right << std::setfill('*') << opcode << '_' <<
                            std::setw(6) << std::setfill('0') << 0 << '_' <<
                            std::setw(6) << std::setfill('0') << std::right << std::hex << std::uppercase <<
                            lengthOfSections[controlSectionCount] << '\n';
                    obj_file  << "H_" << std::setw(6) << std::right << std::setfill('*') << opcode << '_' <<
                            std::setw(6) << std::setfill('0') << 0 << '_' <<
                            std::setw(6) << std::setfill('0') << std::right << std::hex << std::uppercase <<
                            lengthOfSections[controlSectionCount] << '\n';
                }

                space_taken = 0;
                found_first = false;
                text_record.clear();
                put_next.clear();

                // we continue because we want to consider the next instruction for the new CS as the first one
                continue;
            }
        }

            /*
             * this works flawlessly, in the sense that we have all the four variables
             * filled for "START". So we don't need to assign something else to opcode
             * before checking. Does that make sense? I hope it does.
             *
             * Do we ever land in here? because the intermediate file does not contain the
             * START record at all. So, I think we can safely remove this block.
            */
        else if (opcode == "START")
            continue;

        /*
         * This is VERY important. This is going to initialise the "T_{location}"
         * part for a new Text-Record everytime we come out with "found_first"
         * variable as "false"
         */

        if (!found_first)
        {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << std::stoll(loc, NULL, 16);
            std::string temp_loc = oss.str();

            text_record.append("T_" + temp_loc + '_');
            // obj_file << loc << '_' << 30 << '_';
            found_first = true;
        }
    }

    // write the text records for the last CS, as it didn't see a new "CSECT"

    if(space_taken != 0)
    {
        // we need to push the current text record for the previous CS to the object file
        std::ostringstream sp;
        sp << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << space_taken;
        std::string space_taken_string = sp.str();

        text_record.append(space_taken_string);
        text_record.append(put_next);

        if(globalAssemblyStat != assemblerStatus::FAILURE)
        {
            std::cout << text_record << std::endl;
            obj_file << text_record << std::endl;
        }
    }

    // put the modification records now
    if(globalAssemblyStat != assemblerStatus::FAILURE)
    {
        for(auto &i : modification_records)
        {
            std::cout << i << '\n';
            obj_file << i << '\n';
        }
    }

    modification_records.clear();


    // write the END record for the last CS
    if(globalAssemblyStat != assemblerStatus::FAILURE)
    {
        std::cout << "E_" << std::hex << std::setw(6) << std::setfill('0') << START_ADDR << "\n\n";
        obj_file << "E_" << std::hex << std::setw(6) << std::setfill('0') << START_ADDR << "\n\n";
    }

    if(globalAssemblyStat == assemblerStatus::FAILURE)
        std::clog << "\n\nSecond Pass Assembly failed, no Object File was generated.\nPlease check the Listing File for errors.\n\n";
    else
        std::clog << "\n\nSecond Pass Assembly of instructions successful.\nPlease check the Object File.\n\n";

    obj_file.close();
    input_file.close();

    return true;
}