// Most of this was copy-pasted from Ryan Davis's PADI software:
// https://github.com/rsdavis/ParallelDiffuseInterface-PADI
// It provides utilities for reading input parameters from a text file.
// The logging code has been removed.

#include "IO_tools.hpp"

// utility function for appending numbers to hdf5 filenames, if there is a series
std::string number_filename(std::string base_filename, int counter)
{
    std::stringstream index_string;
    index_string.str("");
    index_string.clear();
    index_string << std::setfill('0') << std::setw(3) << counter; // %03d formatting
    std::string complete_filename = base_filename + "_" + index_string.str() + ".h5";
    return complete_filename;
}

template <typename T> // primary template
void unpack(std::map<std::string, std::string> params, std::string name, T &parameter)
{
    MPI_Abort(PETSC_COMM_WORLD, 0);
}

template <> // explicit specialization for T = double
void unpack(std::map<std::string, std::string> hash, std::string name, double & parameter)
{
    std::map<std::string, std::string>::iterator it;
    it = hash.find(name);
    if (it==hash.end()) { // parameter not found
        PetscPrintf(PETSC_COMM_WORLD, "Parameter %s not found\n", name.c_str());
        MPI_Abort(PETSC_COMM_WORLD, 0);
    } else { // parameter found
        parameter = std::stod(it->second);
    }
}

template <> // explicit specialization for T = int
void unpack(std::map<std::string, std::string> hash, std::string name, int & parameter)
{
    std::map<std::string, std::string>::iterator it;
    it = hash.find(name);
    if (it==hash.end()) { // parameter not found
        PetscPrintf(PETSC_COMM_WORLD, "Parameter %s not found\n", name.c_str());
        MPI_Abort(PETSC_COMM_WORLD, 0);
    } else { // parameter found
        parameter = std::stoi(it->second);
    }
}

BC_type convert_to_BCtype(std::string name)
{
    if (name == "periodic")
    {
        return periodicBC;
    }
    else if (name == "derivative")
    {
        return derivativeBC;
    }
    else if (name == "constant")
    {
        return constantBC;
    }
    else
    {
        // XXX need better error handling
        PetscPrintf(PETSC_COMM_WORLD, "Warning: %s not valid boundary condition type; defaulting to derivative\n");
        return derivativeBC;
    }
}

template <> // explicit specialization for T = BC
void unpack(std::map<std::string, std::string> hash, std::string name, BC & parameter)
{
    std::map<std::string, std::string>::iterator it;
    it = hash.find(name);
    if (it==hash.end()) { // parameter not found
        PetscPrintf(PETSC_COMM_WORLD, "Parameter %s not found\n", name.c_str());
        MPI_Abort(PETSC_COMM_WORLD, 0);
    } else { // parameter found
        // Split line using commas
        std::istringstream ss(it->second);
        std::string token;
        int counter = 0;
        while (std::getline(ss, token, ','))
        {
            if (counter == 0)
            {
                parameter.lower_BC_type = convert_to_BCtype(token);
            }
            else if (counter == 1)
            {
                parameter.lower_BC_val = std::stod(token);
            }
            else if (counter == 2)
            {
                parameter.upper_BC_type = convert_to_BCtype(token);
            }
            else if (counter == 3)
            {
                parameter.upper_BC_val = std::stod(token);
            }
            else
            {
                PetscPrintf(PETSC_COMM_WORLD, "Boundary condition format should be"
                    "type,val,type,val\n");
            }
            counter++;
        }
    }
}

void unpack(std::map<std::string, int> name_index, std::string name, int &index)
{
    std::map<std::string, int>::iterator it;
    it = name_index.find(name);
    if (it==name_index.end()) { // parameter not found
        PetscPrintf(PETSC_COMM_WORLD, "Parameter %s not found\n", name.c_str());
        MPI_Abort(PETSC_COMM_WORLD, 0);
    } else { // parameter found
        index = it->second;
    }
}

void read_parameters(std::map<std::string, std::string> &params, std::string input_file)
{

    // This function parses the input file.
    // It reads a key-value pair from any line involving an "=" sign.
    // All key-value are stored as string in params for use later.
    // Comment lines begin with "#"
    
    std::ifstream input(input_file);
    std::string line;
    
    while (std::getline(input, line))
    {
        size_t pos;
        std::string key, value;
        
        // remove whitespace before parsing
        line.erase(std::remove(line.begin(),line.end(),' '), line.end());
        line.erase(std::remove(line.begin(),line.end(),'\t'), line.end());
        line.erase(std::remove(line.begin(),line.end(),'\n'), line.end());
        line.erase(std::remove(line.begin(),line.end(),'\r'), line.end());
        line.erase(std::remove(line.begin(),line.end(),'\v'), line.end());
        
        // skip empty lines
        if (line.empty()) continue;
        
        // skip comments
        if (line[0] == '#') continue;
        
        // parse line using the equal sign
        pos = line.find("=");
        key = line.substr(0, pos);
        value = line.substr(pos+1, line.size());
        
        params[key] = value;
    }
    
    input.close();
}