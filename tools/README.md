# Tools

This directory contains stand alone tools used to generate certain constant (arrays) used in core/source/moveConstants.h.

## Common directory

Since many of the tools need to do similar things these have been implemented in the common directory so each tool can use the same code.

### generateKnightMoves.cpp

This tool is used to generate a 64 long bitboard array.
