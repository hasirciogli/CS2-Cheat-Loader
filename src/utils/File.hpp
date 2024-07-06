#include <fstream> // Add this line to include the necessary header file
#include <iostream>

#ifndef FILEMM_HPP
#define FILEMM_HPP

class File {
public:
  File(const char *filename);
  ~File();
  void write(const char *data);
  void read(char *buffer, int size);
  void close();
  std::vector<unsigned char> readFileToByteArray(const std::string &filePath);
  void saveByteArrayToFile(const std::string& fileName, const std::vector<unsigned char>& byteData, const std::string& variableName);

private:
  const char *filename;
  std::fstream file;
};

#endif