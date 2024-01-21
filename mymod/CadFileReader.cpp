#include "CadFileReader.h"

#include <QDebug>

#include <STEPControl_Reader.hxx>
#include <StlAPI_Reader.hxx>

CadFileReader::CadFileReader() {}

bool CadFileReader::readStep(const std::string &step_file_path) const
{
    qDebug() << "Reading STEP..." << step_file_path;

    STEPControl_Reader          aReader;
    const IFSelect_ReturnStatus aStatus = aReader.ReadFile(step_file_path.c_str());

    if (aStatus == IFSelect_RetDone) {
        qDebug() << "A STEP file was read successfully" << step_file_path;
        return true;
    }
    else {
        qDebug() << "Cannot read the step file." << step_file_path;
        return false;
    }
}

bool CadFileReader::readStl(const std::string &stl_file_path) const
{
    qDebug() << "Reading STL..." << stl_file_path;

    StlAPI_Reader aReader_Stl;
    TopoDS_Shape  shape_Stl;
    if (aReader_Stl.Read(shape_Stl, stl_file_path.c_str())) {
        qDebug() << "A STL file was read successfully" << stl_file_path;
        return true;
    }
    else {
        qDebug() << "Cannot read the STL file." << stl_file_path;
        return false;
    }
}
