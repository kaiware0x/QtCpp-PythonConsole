#ifndef CADFILEREADER_H
#define CADFILEREADER_H

#include <string>

class CadFileReader {
public:
    CadFileReader();

    bool readStep(const std::string& step_file_path) const;
    bool readStl(const std::string& stl_file_path) const;
};

#endif    // CADFILEREADER_H
