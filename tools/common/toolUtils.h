#pragma once

#include "bitBoard.h"
#include <vector>
#include <string>

void writeArray(std::vector<bitboard> array, const std::string &outFile, std::string_view arrName);

void clearFile(const std::string &outFile);