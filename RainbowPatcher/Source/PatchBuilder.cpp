#include <RainbowPatcher/PatchBuilder.h>

#include <array>
#include <fstream>
#include <limits>

namespace Patcher
{
  namespace
  {
    const uint64_t SECTOR_SIZE = 64 * 1024;
    const uint64_t TRAINER_SIZE = 512;
    const uint16_t MAX_SECTORS_PER_ROM = 128;

    struct NesRom
    {
      std::vector<uint8_t> prg_rom;
      std::vector<uint8_t> chr_rom;
    };

    /**
     * @brief Clears a comparison result and assigns an error.
     * @param result Result to clear and update.
     * @param error Error value to assign.
     * @param message User-facing error message.
     * @return Always false, for direct use in error paths.
     */
    bool set_error(RomDiff& result, Error error, const std::string& message)
    {
      result.clear();
      result.error = error;
      result.error_message = message;
      return false;
    }

    /**
     * @brief Decodes a NES 2.0 PRG-ROM or CHR-ROM size field.
     * @param lsb Least-significant size byte.
     * @param msb Most-significant size nibble.
     * @param unit Linear encoding size unit.
     * @param size Receives the decoded byte size.
     * @return true when the decoded size fits in a 64-bit value.
     */
    bool decode_nes2_size(uint8_t lsb, uint8_t msb, uint64_t unit, uint64_t& size)
    {
      if(msb != 0x0F) {
        size = (((uint64_t)msb << 8) | lsb) * unit;
        return true;
      }

      uint8_t exponent = lsb >> 2;
      uint8_t multiplier = lsb & 0x03;
      if(exponent >= 63) {
        return false;
      }

      uint64_t base = (uint64_t)1 << exponent;
      uint64_t factor = multiplier * 2 + 1;
      if(base > std::numeric_limits<uint64_t>::max() / factor) {
        return false;
      }

      size = base * factor;
      return true;
    }

    /**
     * @brief Reads one complete ROM region from a file.
     * @param file Input ROM stream.
     * @param offset Region offset in the file.
     * @param size Region size in bytes.
     * @param data Receives the region data.
     * @return true when all requested bytes were read.
     */
    bool read_region(std::ifstream& file, uint64_t offset, uint64_t size, std::vector<uint8_t>& data)
    {
      if(size > std::numeric_limits<size_t>::max()) {
        return false;
      }

      data.resize((size_t)size);
      if(size == 0) {
        return true;
      }

      file.seekg((std::streamoff)offset, std::ios::beg);
      file.read((char*)data.data(), (std::streamsize)size);
      return file.gcount() == (std::streamsize)size;
    }

    /**
     * @brief Loads and validates the patchable regions of a NES 2.0 ROM.
     * @param path ROM file path.
     * @param rom Receives the PRG-ROM and CHR-ROM data.
     * @param result Receives validation errors.
     * @param name ROM label used in error messages.
     * @return true when the ROM is valid and was completely loaded.
     */
    bool load_rom(const std::filesystem::path& path, NesRom& rom, RomDiff& result, const char* name)
    {
      if(path.empty() || !std::filesystem::exists(path)) {
        return set_error(result, Error::MissingFile, std::string("ROM ") + name + " not found.");
      }

      std::ifstream file(path, std::ios::binary);
      if(!file.is_open()) {
        return set_error(result, Error::FailedToOpenFile, std::string("Failed to open ROM ") + name + ".");
      }

      std::array<uint8_t, 16> header = {};
      file.read((char*)header.data(), header.size());
      if(file.gcount() != (std::streamsize)header.size()) {
        return set_error(result, Error::InvalidNesHeader, std::string("ROM ") + name + " has a truncated NES header.");
      }

      if(header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A) {
        return set_error(result, Error::InvalidNesHeader, std::string("ROM ") + name + " is not a NES ROM.");
      }

      if((header[7] & 0x0C) != 0x08) {
        return set_error(result, Error::UnsupportedNesVersion, std::string("ROM ") + name + " is not using a NES 2.0 header.");
      }

      uint64_t prg_size = 0;
      uint64_t chr_size = 0;
      if(!decode_nes2_size(header[4], header[9] & 0x0F, 16 * 1024, prg_size) || !decode_nes2_size(header[5], header[9] >> 4, 8 * 1024, chr_size)) {
        return set_error(result, Error::InvalidRomSize, std::string("ROM ") + name + " has an invalid NES 2.0 ROM size.");
      }

      if(prg_size == 0 || prg_size % SECTOR_SIZE != 0 || chr_size % SECTOR_SIZE != 0) {
        return set_error(result, Error::InvalidRomSize, std::string("ROM ") + name + " PRG-ROM and CHR-ROM sizes must be multiples of 64 KiB.");
      }

      if(prg_size / SECTOR_SIZE > MAX_SECTORS_PER_ROM || chr_size / SECTOR_SIZE > MAX_SECTORS_PER_ROM) {
        return set_error(result, Error::TooManySectors, std::string("ROM ") + name + " exceeds 128 sectors for PRG-ROM or CHR-ROM.");
      }

      uint64_t data_offset = 16;
      if(header[6] & 0x04) {
        data_offset += TRAINER_SIZE;
      }

      file.seekg(0, std::ios::end);
      std::streamoff file_size = file.tellg();
      if(file_size < 0 || (uint64_t)file_size < data_offset + prg_size + chr_size) {
        return set_error(result, Error::InvalidRomSize, std::string("ROM ") + name + " is truncated.");
      }

      if(!read_region(file, data_offset, prg_size, rom.prg_rom) || !read_region(file, data_offset + prg_size, chr_size, rom.chr_rom)) {
        return set_error(result, Error::FailedToOpenFile, std::string("Failed to read ROM ") + name + ".");
      }

      return true;
    }

    /**
     * @brief Adds all changed 64 KiB sectors from one ROM region.
     * @param data_a Original ROM region.
     * @param data_b New ROM region.
     * @param destination Patch destination for the region.
     * @param result Comparison result to update.
     */
    void compare_region(const std::vector<uint8_t>& data_a,
      const std::vector<uint8_t>& data_b,
      Destination destination,
      RomDiff& result)
    {
      size_t sector_count = data_b.size() / (size_t)SECTOR_SIZE;
      for(size_t sector = 0; sector < sector_count; sector++) {
        size_t offset = sector * (size_t)SECTOR_SIZE;
        bool different = false;
        for(size_t byte = 0; byte < (size_t)SECTOR_SIZE; byte++) {
          if(data_a[offset + byte] != data_b[offset + byte]) {
            different = true;
            break;
          }
        }

        if(different) {
          SectorDifference difference;
          difference.destination = destination;
          difference.index = (uint8_t)sector;
          result.sectors.push_back(difference);
          if(destination == Destination::PrgRom) {
            result.prg_sector_count++;
          } else {
            result.chr_sector_count++;
          }
        }
      }
    }

    /**
     * @brief Writes a complete byte range to a patch stream.
     * @param file Output patch stream.
     * @param data Data to write.
     * @param size Data size in bytes.
     * @return true when the stream remains valid.
     */
    bool write_all(std::ofstream& file, const void* data, size_t size)
    {
      file.write((const char*)data, (std::streamsize)size);
      return file.good();
    }
  }

  /**
   * @brief Returns whether the comparison completed without error.
   * @return true when no error was recorded.
   */
  bool RomDiff::is_valid() const
  {
    return error == Error::None;
  }

  /**
   * @brief Returns the total number of changed sectors.
   * @return Combined PRG-ROM and CHR-ROM sector count.
   */
  uint16_t RomDiff::sector_count() const
  {
    return (uint16_t)(prg_sector_count + chr_sector_count);
  }

  /** @brief Resets the error, message, counters and changed-sector list. */
  void RomDiff::clear()
  {
    error = Error::None;
    error_message.clear();
    prg_sector_count = 0;
    chr_sector_count = 0;
    sectors.clear();
  }

  /**
   * @brief Compares two NES 2.0 ROM files sector by sector.
   * @param file_a Currently installed ROM file.
   * @param file_b New ROM file.
   * @return Comparison result and changed-sector list.
   */
  RomDiff compare(const std::filesystem::path& file_a, const std::filesystem::path& file_b)
  {
    RomDiff result;
    NesRom rom_a;
    NesRom rom_b;

    if(!load_rom(file_a, rom_a, result, "A") || !load_rom(file_b, rom_b, result, "B")) {
      return result;
    }

    if(rom_a.prg_rom.size() != rom_b.prg_rom.size() || rom_a.chr_rom.size() != rom_b.chr_rom.size()) {
      set_error(result, Error::DifferentRomLayout, "ROM A and ROM B do not have the same PRG-ROM and CHR-ROM sizes.");
      return result;
    }

    compare_region(rom_a.prg_rom, rom_b.prg_rom, Destination::PrgRom, result);
    compare_region(rom_a.chr_rom, rom_b.chr_rom, Destination::ChrRom, result);

    return result;
  }

  /**
   * @brief Creates a Rainbow patch that transforms ROM A into ROM B.
   * @param file_a Currently installed ROM file.
   * @param file_b New ROM file.
   * @param patch_file Destination patch file.
   * @param comparison Receives the final comparison result.
   * @return true when the patch was successfully created.
   */
  bool create_patch(const std::filesystem::path& file_a, const std::filesystem::path& file_b, const std::filesystem::path& patch_file, RomDiff& romDiff)
  {
    romDiff = compare(file_a, file_b);
    if(!romDiff.is_valid()) {
      return false;
    }
    if(romDiff.sectors.empty()) {
      return set_error(romDiff, Error::NoDifference, "ROM A and ROM B are identical.");
    }

    NesRom rom_b;
    RomDiff load_result;
    if(!load_rom(file_b, rom_b, load_result, "B")) {
      romDiff = load_result;
      return false;
    }

    if(patch_file.empty()) {
      return set_error(romDiff, Error::FailedToWritePatch, "Patch output path is empty.");
    }

    std::filesystem::path temporary_path = patch_file;
    temporary_path += ".tmp";
    std::ofstream output(temporary_path, std::ios::binary | std::ios::trunc);
    if(!output.is_open()) {
      return set_error(romDiff, Error::FailedToWritePatch, "Failed to create the patch file.");
    }

    uint16_t block_count = romDiff.sector_count();
    uint8_t header[16] = { 'R', 'N', 'B', 'W', 0x1A, 0 };
    header[6] = (uint8_t)(block_count >> 8);
    header[7] = (uint8_t)(block_count & 0xFF);

    bool success = write_all(output, header, sizeof(header));
    for(size_t i = 0; success && i < romDiff.sectors.size(); i++) {
      const SectorDifference& difference = romDiff.sectors[i];
      uint8_t sector_header[16] = {};
      sector_header[0] = (uint8_t)difference.destination;
      sector_header[1] = difference.index;
      success = write_all(output, sector_header, sizeof(sector_header));

      const std::vector<uint8_t>& data = difference.destination == Destination::PrgRom ? rom_b.prg_rom : rom_b.chr_rom;
      size_t offset = (size_t)difference.index * (size_t)SECTOR_SIZE;
      if(success) {
        success = write_all(output, data.data() + offset, (size_t)SECTOR_SIZE);
      }
    }

    output.close();
    if(!success || output.fail()) {
      std::error_code ignored;
      std::filesystem::remove(temporary_path, ignored);
      return set_error(romDiff, Error::FailedToWritePatch, "Failed while writing the patch file.");
    }

    std::error_code error;
    if(std::filesystem::exists(patch_file)) {
      std::filesystem::remove(patch_file, error);
      if(error) {
        std::filesystem::remove(temporary_path, error);
        return set_error(romDiff, Error::FailedToWritePatch, "Failed to replace the existing patch file.");
      }
    }

    std::filesystem::rename(temporary_path, patch_file, error);
    if(error) {
      std::filesystem::remove(temporary_path, error);
      return set_error(romDiff, Error::FailedToWritePatch, "Failed to finalize the patch file.");
    }

    return true;
  }
}
