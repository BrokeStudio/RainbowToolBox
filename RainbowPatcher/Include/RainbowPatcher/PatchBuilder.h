#pragma once
#ifndef PATCH_BUILDER_H
  #define PATCH_BUILDER_H

  #include <filesystem>
  #include <stdint.h>
  #include <string>
  #include <vector>

namespace Patcher
{
  enum class Error
  {
    None,
    MissingFile,
    FailedToOpenFile,
    InvalidNesHeader,
    UnsupportedNesVersion,
    InvalidRomSize,
    DifferentRomLayout,
    TooManySectors,
    NoDifference,
    FailedToWritePatch,
  };

  enum class Destination : uint8_t
  {
    PrgRom = 0,
    ChrRom = 1,
  };

  struct SectorDifference
  {
    Destination destination;
    uint8_t index;
  };

  struct RomDiff
  {
    Error error = Error::None;
    std::string error_message;
    uint16_t prg_sector_count = 0;
    uint16_t chr_sector_count = 0;
    std::vector<SectorDifference> sectors;

    bool is_valid() const;
    uint16_t sector_count() const;
    void clear();
  };

  RomDiff compare(const std::filesystem::path& file_a, const std::filesystem::path& file_b);
  bool create_patch(const std::filesystem::path& file_a, const std::filesystem::path& file_b, const std::filesystem::path& patch_file, RomDiff& comparison);
}

#endif
