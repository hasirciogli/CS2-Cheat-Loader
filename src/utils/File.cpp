#include "File.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// Dosyayı byte dizisine çevirme fonksiyonu
std::vector<unsigned char>
File::readFileToByteArray(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    std::cerr << "Dosya açılamadı: " << filePath << std::endl;
    return {};
  }

  // Dosyanın boyutunu al
  file.seekg(0, std::ios::end);
  std::streampos fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  // Byte dizisi için yer ayır
  std::vector<unsigned char> bytes(fileSize);

  // Dosyayı oku ve byte dizisine kopyala
  file.read(reinterpret_cast<char *>(bytes.data()), fileSize);
  file.close();

  return bytes;
}

// Byte dizisini C++ dosyasına kaydetme fonksiyonu
void File::saveByteArrayToFile(const std::string &fileName,
                               const std::vector<unsigned char> &byteData,
                               const std::string &variableName) {
  std::ofstream file(fileName);
  if (!file) {
    std::cerr << "Dosya açılamadı: " << fileName << std::endl;
    return;
  }

  // C++ kodu olarak yazma
  file << "#include <cstddef>\n\nnamespace FontBytes {\n";
  file << "unsigned char " << variableName << "[" << byteData.size()
       << "] = {\n";
  for (size_t i = 0; i < byteData.size(); ++i) {
    file << "0x" << std::hex << std::setw(2) << std::setfill('0')
         << static_cast<int>(byteData[i]);
    if (i != byteData.size() - 1) {
      file << ", ";
    }
    if ((i + 1) % 12 == 0) { // 12 byte yazdıktan sonra yeni satıra geç
      file << "\n";
    }
  }
  file << "\n};\n\n";
  file << "const size_t " << variableName << "_size = " << std::dec
       << byteData.size() << ";\n";
  file << "}\n";

  file.close();
}

// File sınıfının kurucu fonksiyonu
File::File(const char *filename) : filename(filename) {
  file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
}

// File sınıfının yıkıcı fonksiyonu
File::~File() {
  if (file.is_open()) {
    file.close();
  }
}

// Dosyaya yazma fonksiyonu
void File::write(const char *data) {
  if (file.is_open()) {
    file.write(data, std::strlen(data));
  }
}

// Dosyadan okuma fonksiyonu
void File::read(char *buffer, int size) {
  if (file.is_open()) {
    file.read(buffer, size);
  }
}

// Dosyayı kapatma fonksiyonu
void File::close() {
  if (file.is_open()) {
    file.close();
  }
}
