#include <any>
#include <cstring>
#include <iostream>
#include <netinet/in.h> // htonl, htons, ntohl, ntohs
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

class Hoffer {
public:
  Hoffer() : readOffset(0) {}

  // Veri yazma metodları
  void putInt32(int32_t value) {
    value = htonl(value); // Convert to network byte order
    appendToBuffer(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  void putString(const std::string &value) {
    int32_t length = htonl(static_cast<int32_t>(value.size()));
    appendToBuffer(reinterpret_cast<const char *>(&length), sizeof(length));
    appendToBuffer(value.data(), value.size());
  }

  void putDouble(double value) {
    appendToBuffer(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  void putFloat(float value) {
    appendToBuffer(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  void putByte(uint8_t value) {
    appendToBuffer(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  void putByteArray(const std::vector<uint8_t> &value) {
    int32_t length = htonl(static_cast<int32_t>(value.size()));
    appendToBuffer(reinterpret_cast<const char *>(&length), sizeof(length));
    appendToBuffer(reinterpret_cast<const char *>(value.data()), value.size());
  }

  template <typename T> void putCustom(const T &value) {
    appendToBuffer(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  // Veri okuma metodları
  int32_t getInt32() {
    int32_t value;
    readFromBuffer(reinterpret_cast<char *>(&value), sizeof(value));
    return ntohl(value); // Convert to host byte order
  }

  std::string getString() {
    int32_t length;
    readFromBuffer(reinterpret_cast<char *>(&length), sizeof(length));
    length = ntohl(length); // Convert to host byte order
    std::string value(length, '\0');
    readFromBuffer(value.data(), length);
    return value;
  }

  double getDouble() {
    double value;
    readFromBuffer(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
  }

  float getFloat() {
    float value;
    readFromBuffer(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
  }

  uint8_t getByte() {
    uint8_t value;
    readFromBuffer(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
  }

  std::vector<uint8_t> getByteArray() {
    int32_t length;
    readFromBuffer(reinterpret_cast<char *>(&length), sizeof(length));
    length = ntohl(length); // Convert to host byte order
    std::vector<uint8_t> value(length);
    readFromBuffer(reinterpret_cast<char *>(value.data()), length);
    return value;
  }

  template <typename T> T getCustom() {
    T value;
    readFromBuffer(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
  }

  // Buffer'ı yeniden kullanmak için sıfırlama
  void reset() {
    buffer.clear();
    readOffset = 0;
  }

  // Data byte dizisini almak için
  const std::vector<uint8_t> &getData() const { return buffer; }

  // Data byte dizisini ayarlamak için
  void setData(const std::vector<uint8_t> &data) {
    buffer = data;
    readOffset = 0;
  }

  // Veriyi socket üzerinden göndermek için (example using POSIX sockets)
  void sendData(int socket) const {
    ::send(socket, buffer.data(), buffer.size(), 0);
  }

private:
  std::vector<uint8_t> buffer;
  size_t readOffset;

  void appendToBuffer(const char *data, size_t size) {
    buffer.insert(buffer.end(), data, data + size);
  }

  void readFromBuffer(const char *data, size_t size) {
    if (readOffset + size > buffer.size()) {
      throw std::runtime_error("Attempt to read beyond buffer size");
    }
    std::memcpy(const_cast<char *>(data), buffer.data() + readOffset, size);
    readOffset += size;
  }
};