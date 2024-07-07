#include "classes.h"
#include "utilities.h"
#include "cstring"


bool secondPass();

int main()
{
    std::cout.setstate(std::ios::failbit);


    fill_opTable(opTab);
	fill_regTable(regTab);


    std::fstream input_file("input.txt", std::ios::in);
    std::fstream intermediate_file("intermediate.txt", std::ios::out);


    assemblerStatus assemblyStat = assemblerStatus::SUCCESS;
    assemblerStatus globalAssemblyStat = assemblerStatus::SUCCESS;

    
    if (!input_file.is_open())
    {
        std::cerr << "error opening the Input File during assembly\n";
        std::exit(0);
    }

    if (!intermediate_file.is_open())
    {
        std::cerr << "error opening the Intermediate File during assembly\n";
        std::exit(0);
    }

    unsigned temp_LOC_CTR = 0;
    size_t START_ADDR = 0;
    
    std::string line;
    std::string label, opcode, operand;
    size_t add_to_loc = 0;
    instructionType instrType = instructionType::INVALID;
    symTabs.resize(1);

    std::cout << "LOC_CTR\t LABEL\t OPCODE\t OPERAND\n\n";

    while (std::getline(input_file, line))
    {
        label.clear(), opcode.clear(), operand.clear();
        add_to_loc = 0;
        instrType = instructionType::INVALID;
        assemblyStat = assemblerStatus::SUCCESS;
        
        std::istringstream iss(line);
        
        iss >> std::skipws;
        iss >> label;

        if(label == "EXTREF")
        {
            std::string temp;
            while(std::getline(iss >> std::ws, temp, ','))
                symTabs[controlSectionCount].insert({temp, {0, symbol_entry_details::EXTERNAL}});
            
            continue;
        }
        else if(label == "EXTDEF")
            continue;
        else if(label[0] == '.') // this indicates a comment
            continue;
        /*
            will not worry about this now. This case handling is like a lot of work because 
            we need to basically check if "the other sections" actually ALLOW us to use these
            variables as external references. So, we will do this later. Pain.
            else if(label == "EXTDEF")
            {
                std::string temp;
                while(iss >> temp)
                {
                    symTabs[controlSectionCount].insert({temp, {0, symbol_entry_details::EXTERNAL}});
                }
                continue;
            }
        */

        // if there are  "LABEL, OPCODE, OPERAND", only then we go into this 'if'
        if (iss >> opcode >> operand)
        {
            if(opcode[0] == '+')
                opcode.erase(opcode.begin()), instrType = instructionType::EXTENDED;
            

            bool valid_opcode = (opTab.find(opcode) != opTab.end());
            
            if (valid_opcode)
            {
                auto it = opTab.find(opcode);

                if(instrType != instructionType::EXTENDED)
                    add_to_loc = it->second.format;
                else
                    add_to_loc = 4, opcode.insert(0, "+");
            }
            // below processing is for assembly directives. We should have been more careful with
            // using the name as "opcode", but we shall fix it later.
            else if (opcode == "START")
            {
                // i dont handle cases where the START directive doesn't have a label.
                // i realise that it is not a good practice, but i have a deadline
                progName = label;

                while(progName.length() < 6)
                    progName = '*' + progName;

                add_to_loc = 0;

                std::istringstream hex(operand);
                hex >> std::hex >> temp_LOC_CTR;
                START_ADDR = temp_LOC_CTR;
                ABSOLUTE_START_ADDR = temp_LOC_CTR;
            }
            else if (opcode == "WORD")
            {
                add_to_loc = 3;
            }
            else if (opcode == "BYTE")
            {
                if (operand[0] == 'C')
                {
                    add_to_loc = operand.length() - 3;
                }
                else if (operand[0] == 'X')
                {
                    add_to_loc = (operand.length() - 3) / 2;
                }
            }
            else if (opcode == "RESW")
            {
                // this is in decimal, not hex
                add_to_loc = std::stoul(operand) * 3;
            }
            else if (opcode == "RESB")
            {
                // this is in decimal, not hex
                add_to_loc = std::stoul(operand);
            }
            else
            {
                // check this for error handling
                std::cout << "Invalid opcode / directive: " << opcode << "found in the instruction: " << line << '\n';
                intermediate_file << "Invalid opcode / directive: " << opcode << "found in the instruction: " << line << '\n';
                
                assemblyStat = assemblerStatus::FAILURE;
                globalAssemblyStat = assemblerStatus::FAILURE;
                add_to_loc = 0;
                continue;
            }


            // checking for duplicate label
            bool dup_label = ( symTabs[controlSectionCount].find(label) !=
                                        symTabs[controlSectionCount].end() );

            if (!dup_label)
                symTabs[controlSectionCount].insert({label, {temp_LOC_CTR, symbol_entry_details::ABSOLUTE}});
            else
            {
                intermediate_file << std::setfill('X') << std::right << std::setw(6) << "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << label <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << operand << 
                                '\t' << "Duplicate label \"" << label << "\" found. Previously declared at: " << 
                                std::hex << std::right << std::setw(6) << std::setfill('0') << 
                                symTabs[controlSectionCount][label].location << '\n';

                std::cout << std::setfill('X') << std::right << std::setw(6) <<  "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << label <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << operand << 
                                '\t' << "Duplicate label \"" << label << "\" found. Previously declared at: " << 
                                std::hex << std::right << std::setw(6) << std::setfill('0') << 
                                symTabs[controlSectionCount][label].location << '\n';

                assemblyStat = assemblerStatus::FAILURE;
                globalAssemblyStat = assemblerStatus::FAILURE;
                add_to_loc = 0;
                continue;
            }
            
            // printing the line to the intermediate file. We don't print the line if it is a START
            if (opcode != "START" && assemblyStat != assemblerStatus::FAILURE)
            {
                std::cout << std::hex << std::right << std::setfill('0') << std::setw(6) << temp_LOC_CTR << '\t' <<
                        std::left << std::setw(10) << std::setfill(' ') << label <<
                        std::left << std::setw(10) << std::setfill(' ') << opcode <<
                        std::left << std::setw(10) << std::setfill(' ') << operand << std::endl;

                intermediate_file << std::hex << std::right << std::setfill('0') << std::setw(6) << temp_LOC_CTR << '\t' <<
                        std::left << std::setw(10) << std::setfill(' ') << label <<
                        std::left << std::setw(10) << std::setfill(' ') << opcode <<
                        std::left << std::setw(10) << std::setfill(' ') << operand << std::endl;
            }
        }
        // this implies a line with whitspaces. So we skip it.
        else if (opcode.empty() && operand.empty() && label.empty())
            continue;
        else if (opcode.empty() && operand.empty())
        {
            /*
             * only such instruction in SIC is "RSUB". But we also enter this
             * conditional if we had "END" parsed in this line. So we need to take
             * care of both.
             * 
             * PS: We would also have format 1 instructions that we need to handle here
             */
            
            opcode = label;

            if(opcode[0] == '+')
                opcode.erase(opcode.begin()), instrType = instructionType::EXTENDED;
            
            /*
             * for faster checks, I don't do `if(op_tab.find(opcode))` because as said
             * above, the ONLY instruction that SIC provides in this case is "RUSB".
             */

            if (opcode == "RSUB")
            {
                std::cout << std::hex << std::right << std::setw(6) << std::setfill('0') << temp_LOC_CTR << '\t' <<
                            std::left << std::setw(10) << std::setfill(' ') << "" <<
                            std::left << std::setw(10) << std::setfill(' ') << opcode << std::endl;

                intermediate_file << std::hex << std::right << std::setw(6) << std::setfill('0') << temp_LOC_CTR << '\t' <<
                            std::left << std::setw(10) << std::setfill(' ') << "" <<
                            std::left << std::setw(10) << std::setfill(' ') << opcode << std::endl;

                if(instrType != instructionType::EXTENDED)
                    add_to_loc = 3;
                else
                    add_to_loc = 4, opcode.insert(0, "+");
            }
            // I only read our file until I find the "END". I don't care what happens
            // next, I just stop.
            else if (opcode == "END")
            {
                lengthOfSections.push_back(temp_LOC_CTR - START_ADDR);
                //add_to_loc = 1;
                //temp_LOC_CTR += add_to_loc;
                break;
            }
            else if(opTab.find(opcode) != opTab.end())
            {
                // these are format 1 instructions
                if(instrType == instructionType::EXTENDED)
                {
                    // cannot have a format 1 instruction with extended addressing

                    intermediate_file << std::setfill('X') << std::right << std::setw(6) << "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << "" <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << "" << 
                                '\t' << "Invalid Opcode \"" << opcode << "\" found\n";

                    std::cout << std::setfill('X') << std::right << std::setw(6) << "" << '\t' <<
                                std::left << std::setw(10) << std::setfill(' ') << "" <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << "" << 
                                '\t' << "Invalid Opcode \"" << opcode << "\" found\n"; 
                    
                    globalAssemblyStat = assemblerStatus::FAILURE;
                    add_to_loc = 0;
                }
                else
                {
                    // a non-extended, valid format 3 instruction
                    std::cout << std::hex << std::right << std::setw(6) << std::setfill('0') << temp_LOC_CTR << '\t' <<
                            std::left << std::setw(10) << std::setfill(' ') << "" <<
                            std::left << std::setw(10) << std::setfill(' ') << opcode << std::endl;

                    intermediate_file << std::hex << std::right << std::setw(6) << std::setfill('0') << temp_LOC_CTR << '\t' <<
                            std::left << std::setw(10) << std::setfill(' ') << "" <<
                            std::left << std::setw(10) << std::setfill(' ') << opcode << std::endl;

                    add_to_loc = 1;
                }
            }
            // if it was not "RSUB" or "END", then it is an error.
            else
            {
                intermediate_file << std::setfill('X') << std::right << std::setw(6) << "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << "" <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << "" << 
                                '\t' << "Invalid Opcode \"" << label << "\" found\n";

                std::cout << std::setfill('X') << std::right << std::setw(6) << "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << "" <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << "" << 
                                '\t' << "Invalid Opcode \"" << label << "\" found\n";
                

                globalAssemblyStat = assemblerStatus::FAILURE;
                add_to_loc = 0;
            }
        }

        // we only have "OPCODE, OPERAND" present (ex: LDA ZERO) &
        // where we can have CSECT defines like "RDREC CSECT"
        else if (operand.empty())
        {
            /*
             * There is also a weird possibility of having an instruction of the kind
             * "LEX RSUB", which is technically okay to use, but if you actually use
             * it, I think you are not functioning properly. Because who the hell
             * would use a label for an RSUB?
             *
             * So, I have not included bug fixing for this issue as of now.
             */
            if(opcode == "CSECT")
            {
                // length of the current section appended
                lengthOfSections.push_back(temp_LOC_CTR - START_ADDR);
                // new control section will start from 0 (assumption)
                temp_LOC_CTR = 0;
                START_ADDR = 0;
                add_to_loc = 0;
                controlSectionCount++;
                symTabs.resize(controlSectionCount + 1);

                // currently appending 0 as the location for the control section
                // records. Maybe we can see later if we can do something better.
                std::cout << '\n' << std::right << std::setw(6) << std::setfill('0') << 0 << '\t' <<
                std::left << std::setw(10) << std::setfill(' ') << "" <<
                std::left << std::setw(10) << std::setfill(' ') << label <<
                std::left << std::setw(10) << std::setfill(' ') << opcode << std::endl;

                intermediate_file << '\n' << std::right << std::setw(6) << std::setfill('0') << 0 << '\t' <<
                std::left << std::setw(10) << std::setfill(' ') << "" <<
                std::left << std::setw(10) << std::setfill(' ') << label <<
                std::left << std::setw(10) << std::setfill(' ') << opcode << std::endl;

                continue;
            }
            else
            {
                // This is the case where we have "LDA ZERO" kind of instructions. 
                // No labels basically
                operand = opcode;
                opcode = label;

                if(opcode[0] == '+')
                    opcode.erase(opcode.begin()), instrType = instructionType::EXTENDED;
                
                bool valid_op_code = (opTab.find(opcode) != opTab.end());
                if (valid_op_code)
                {
                    if(instrType != instructionType::EXTENDED)
                        add_to_loc = opTab[opcode].format;
                    else
                        add_to_loc = 4, opcode.insert(0, "+");


                    std::cout << std::hex << std::right << std::setfill('0') << std::setw(6) << temp_LOC_CTR << '\t' <<
                              std::left << std::setw(10) << std::setfill(' ') << "" <<
                              std::left << std::setw(10) << std::setfill(' ') << opcode <<
                              std::left << std::setw(10) << std::setfill(' ') << operand << '\n';


                    intermediate_file << std::hex << std::right << std::setfill('0') << std::setw(6) << temp_LOC_CTR << '\t' <<
                              std::left << std::setw(10) << std::setfill(' ') << "" <<
                              std::left << std::setw(10) << std::setfill(' ') << opcode <<
                              std::left << std::setw(10) << std::setfill(' ') << operand << '\n';
                }
                else
                {
                    intermediate_file << std::setfill('X') << std::right << std::setw(6) << "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << "" <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << operand << 
                                '\t' << "Invalid Opcode \"" << opcode << "\" found\n";

                    std::cout << std::setfill('X') << std::right << std::setw(6) << "" << '\t' << 
                                std::left << std::setw(10) << std::setfill(' ') << "" <<
                                std::left << std::setw(10) << std::setfill(' ') << opcode <<
                                std::left << std::setw(10) << std::setfill(' ') << operand << 
                                '\t' << "Invalid Opcode \"" << opcode << "\" found\n";

                    
                    globalAssemblyStat = assemblerStatus::FAILURE;
                    add_to_loc = 0;
                }
            }
        }
        
        temp_LOC_CTR += add_to_loc;
    }

    /*
        printing the length of the sections

        std::cout << "\n\n";
        for(auto i : lengthOfSections)
            std::cout << i << '\n';
    */
    
    /*
        printing the symbol table

        for(auto sym : symTabs)
        {
            std::cout << "\n\n";
            for(auto i : sym)
                std::cout << i.first << '\t' << std::dec << i.second.location << '\t' << i.second.type << '\n';
        }
    */

    intermediate_file.close();
    input_file.close();

    if(globalAssemblyStat != assemblerStatus::FAILURE)
    {
        std::clog << "\n\nFirst Pass Assembly successful. Intermediate file generated.\nNow Performing Second Pass....\n\n";
        secondPass();
    }
    else
    {
        std::clog << "\n\nFirst Pass Assembly failed. No object file was generated.\nErrors are show-cased in the intermediate file.\n\n";
        std::exit(1);
    }

    return 0;
}