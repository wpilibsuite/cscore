/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "JpegUtil.h"

#include "support/raw_istream.h"

namespace cs {

// DHT data for MJPEG images that don't have it.
static const unsigned char dhtData[] = {
    0xff, 0xc4, 0x01, 0xa2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x01, 0x00, 0x03,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
    0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04,
    0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22,
    0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15,
    0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
    0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
    0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5,
    0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
    0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11, 0x04,
    0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22,
    0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33,
    0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25,
    0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
    0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
    0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
    0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};

bool IsJpeg(llvm::StringRef data) {
  if (data.size() < 11) return false;

  // Check for valid SOI
  auto bytes = data.bytes_begin();
  if (bytes[0] != 0xff || bytes[1] != 0xd8)
    return false;

  // Check for valid JPEG header (null terminated JFIF)
  if (bytes[2] == 0xff && bytes[3] == 0xe0) {
    if (data.substr(6, 4) != "JFIF" || bytes[10] != 0x00)
    return false;
  }

  return true;
}

bool GetJpegSize(llvm::StringRef data, int* width, int* height) {
  if (!IsJpeg(data)) return false;

  auto bytes = data.bytes_begin();
  size_t blockLength = bytes[4] * 256 + bytes[5] + 4;
  for (;;) {
    data = data.substr(blockLength);    // Get to the next block
    if (data.size() < 4) return false;  // EOF
    bytes = data.bytes_begin();
    if (bytes[0] != 0xff) return false;  // not a tag
    if (bytes[1] == 0xc0) {
      // SOF contains the file size
      if (data.size() < 9) return false;
      *height = bytes[5] * 256 + bytes[6];
      *width = bytes[7] * 256 + bytes[8];
      return true;
    }
    // Go to the next block
    blockLength = bytes[2] * 256 + bytes[3] + 2;
  }
}

bool JpegNeedsDHT(const char* data, std::size_t* size, std::size_t* locSOF) {
  llvm::StringRef sdata(data, *size);
  if (!IsJpeg(sdata)) return false;

  *locSOF = *size;

  // Search until SOS for DHT tag
  auto bytes = sdata.bytes_begin();
  size_t blockLength = bytes[4] * 256 + bytes[5] + 4;
  for (;;) {
    sdata = sdata.substr(blockLength);   // Get to the next block
    if (sdata.size() < 4) return false;  // EOF
    bytes = sdata.bytes_begin();
    if (bytes[0] != 0xff) return false;                   // not a tag
    if (bytes[1] == 0xda) break;                          // SOS
    if (bytes[1] == 0xc4) return false;                   // DHT
    if (bytes[1] == 0xc0) *locSOF = sdata.data() - data;  // SOF
    // Go to the next block
    blockLength = bytes[2] * 256 + bytes[3] + 2;
  }

  // Only add DHT if we also found SOF (insertion point)
  if (*locSOF != *size) {
    *size += sizeof(dhtData);
    return true;
  }
  return false;
}

llvm::StringRef JpegGetDHT() {
  return llvm::StringRef(reinterpret_cast<const char*>(dhtData),
                         sizeof(dhtData));
}

static inline void ReadInto(wpi::raw_istream& is, std::string& buf,
                            size_t len) {
  size_t oldSize = buf.size();
  buf.resize(oldSize + len);
  is.read(&(*buf.begin()) + oldSize, len);
}

bool ReadJpeg(wpi::raw_istream& is, std::string& buf, int* width,
              int* height) {
  // in case we don't get a SOF
  *width = 0;
  *height = 0;

  // read the header
  buf.resize(4 + 7);
  is.read(&(*buf.begin()), 4 + 7);
  if (is.has_error()) return false;
  if (!IsJpeg(buf)) return false;

  auto bytes = reinterpret_cast<const unsigned char*>(buf.data());
  size_t pos = 4;
  size_t blockLength = bytes[4] * 256 + bytes[5];
  bool sofBlock = false;
  for (;;) {
    // Read to the next block + marker
    ReadInto(is, buf, blockLength + 2);
    if (is.has_error()) return false;

    // Process any blocks we just read
    if (sofBlock) {
      // SOF contains the file size; make sure we actually read enough bytes
      if (blockLength >= 7) {
        *height = bytes[5] * 256 + bytes[6];
        *width = bytes[7] * 256 + bytes[8];
      }
      sofBlock = false;
    }

    pos += blockLength;
    bytes = reinterpret_cast<const unsigned char*>(buf.data() + pos);
    if (bytes[0] != 0xff) return false;     // not a tag
    if (bytes[1] == 0xd9) return true;      // EOI, we're done
    if (bytes[1] == 0xc0) sofBlock = true;  // SOF

    // Go to the next block
    ReadInto(is, buf, 2);  // read length
    if (is.has_error()) return false;
    pos += 2;
    blockLength = bytes[2] * 256 + bytes[3];
  }
}

}  // namespace cs