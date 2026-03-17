
#ifndef CRTPARSER_H
#define CRTPARSER_H

#include <vector>

class CRTParser {
public:
   CRTParser(IDataReader& reader) : reader(reader) {}

   bool parse() {
      if (!parseHeader()) 
         return false;

      //while (parseChipBlock()) {
      //}
      return true;
   }

   //const std::vector<ChipBlock>& getChips() const { return chips; }

private:
   IDataReader& reader;
   //std::vector<ChipBlock> chips;

   bool parseHeader() {
   uint8_t signature[16];
      if (!reader.read(signature, 16)) return false;
      if (memcmp(signature, "C64 CARTRIDGE   ", 16) != 0) return false;

      // Salta versione, hardware type ecc (6 byte)
      return reader.seek(reader.tell() + 6);
   }

    /*
    bool parseChipBlock() {
        if (reader.eof()) return false;

        uint8_t header[6];
        if (!reader.read(header, 6)) return false;

        uint16_t chipType   = header[0] | (header[1] << 8);
        uint16_t startAddr  = header[2] | (header[3] << 8);
        uint16_t blockLen   = header[4] | (header[5] << 8);

        if (blockLen == 0) return false;

        ChipBlock chip;
        chip.startAddress = startAddr;
        chip.length = blockLen;
        chip.data.resize(blockLen);

        if (!reader.read(chip.data.data(), blockLen)) return false;

        chips.push_back(chip);
        return true;
    }
    */
};

#endif
