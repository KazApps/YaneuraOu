/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2025 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//Definition of input features HalfKP of NNUE evaluation function

#ifndef NNUE_FEATURES_HALF_KA_V2_HM_H_INCLUDED
#define NNUE_FEATURES_HALF_KA_V2_HM_H_INCLUDED

#include <cstdint>

#include "../../../misc.h"
#include "../../../evaluate.h"
#include "../nnue_common.h"

namespace Eval::NNUE::Features {

// Feature HalfKAv2_hm: Combination of the position of own king and the
// position of pieces. Position mirrored such that king is always on e..h files.
class HalfKAv2_hm {

    // Unique number for each piece type on each square
    enum {
        PS_NONE         = 0,
        PS_B_PAWN       = 0,
        PS_W_PAWN       =  1 * SQ_NB,
        PS_B_LANCE      =  2 * SQ_NB,
        PS_W_LANCE      =  3 * SQ_NB,
        PS_B_KNIGHT     =  4 * SQ_NB,
        PS_W_KNIGHT     =  5 * SQ_NB,
        PS_B_SILVER     =  6 * SQ_NB,
        PS_W_SILVER     =  7 * SQ_NB,
        PS_B_GOLD       =  8 * SQ_NB,
        PS_W_GOLD       =  9 * SQ_NB,
        PS_B_BISHOP     = 10 * SQ_NB,
        PS_W_BISHOP     = 11 * SQ_NB,
        PS_B_ROOK       = 12 * SQ_NB,
        PS_W_ROOK       = 13 * SQ_NB,
        PS_B_PRO_PAWN   = 14 * SQ_NB,
        PS_W_PRO_PAWN   = 15 * SQ_NB,
        PS_B_PRO_LANCE  = 16 * SQ_NB,
        PS_W_PRO_LANCE  = 17 * SQ_NB,
        PS_B_PRO_KNIGHT = 18 * SQ_NB,
        PS_W_PRO_KNIGHT = 19 * SQ_NB,
        PS_B_PRO_SILVER = 20 * SQ_NB,
        PS_W_PRO_SILVER = 21 * SQ_NB,
        PS_B_HORSE      = 22 * SQ_NB,
        PS_W_HORSE      = 23 * SQ_NB,
        PS_B_DRAGON     = 24 * SQ_NB,
        PS_W_DRAGON     = 25 * SQ_NB,
        PS_KING         = 26 * SQ_NB,
        PS_NB           = 27 * SQ_NB
    };

    static constexpr IndexType PieceSquareIndex[COLOR_NB][PIECE_NB] = {
      // Convention: B - us, W - them
      // Viewed from other side, B and W are reversed
      { PS_NONE, PS_B_PAWN, PS_B_LANCE, PS_B_KNIGHT, PS_B_SILVER, PS_B_BISHOP, PS_B_ROOK, PS_B_GOLD, PS_KING, PS_B_PRO_PAWN, PS_B_PRO_LANCE, PS_B_PRO_KNIGHT, PS_B_PRO_SILVER, PS_B_HORSE, PS_B_DRAGON, PS_NONE,
        PS_NONE, PS_W_PAWN, PS_W_LANCE, PS_W_KNIGHT, PS_W_SILVER, PS_W_BISHOP, PS_W_ROOK, PS_W_GOLD, PS_KING, PS_W_PRO_PAWN, PS_W_PRO_LANCE, PS_W_PRO_KNIGHT, PS_W_PRO_SILVER, PS_W_HORSE, PS_W_DRAGON, PS_NONE },
      { PS_NONE, PS_W_PAWN, PS_W_LANCE, PS_W_KNIGHT, PS_W_SILVER, PS_W_BISHOP, PS_W_ROOK, PS_W_GOLD, PS_KING, PS_W_PRO_PAWN, PS_W_PRO_LANCE, PS_W_PRO_KNIGHT, PS_W_PRO_SILVER, PS_W_HORSE, PS_W_DRAGON, PS_NONE,
        PS_NONE, PS_B_PAWN, PS_B_LANCE, PS_B_KNIGHT, PS_B_SILVER, PS_B_BISHOP, PS_B_ROOK, PS_B_GOLD, PS_KING, PS_B_PRO_PAWN, PS_B_PRO_LANCE, PS_B_PRO_KNIGHT, PS_B_PRO_SILVER, PS_B_HORSE, PS_B_DRAGON, PS_NONE }};

   public:
    // Feature name
    static constexpr const char* Name = "HalfKAv2_hm(Friend)";

    // Hash value embedded in the evaluation file
    static constexpr std::uint32_t HashValue = 0x7f234cb8u;

    // Number of feature dimensions
    static constexpr IndexType Dimensions =
      static_cast<IndexType>(FILE_NB) * 5 * (static_cast<IndexType>(PS_NB) + 64);

#define B(v) (v * (PS_NB + 64))
    // clang-format off
    static constexpr int KingBuckets[COLOR_NB][SQ_NB] = {
      { B( 0), B( 1), B( 2), B( 3), B( 4), B( 5), B( 6), B( 7), B( 8),
        B( 9), B(10), B(11), B(12), B(13), B(14), B(15), B(16), B(17),
        B(18), B(19), B(20), B(21), B(22), B(23), B(24), B(25), B(26),
        B(27), B(28), B(29), B(30), B(31), B(32), B(33), B(34), B(35),
        B(36), B(37), B(38), B(39), B(40), B(41), B(42), B(43), B(44),
        B(27), B(28), B(29), B(30), B(31), B(32), B(33), B(34), B(35),
        B(18), B(19), B(20), B(21), B(22), B(23), B(24), B(25), B(26),
        B( 9), B(10), B(11), B(12), B(13), B(14), B(15), B(16), B(17),
        B( 0), B( 1), B( 2), B( 3), B( 4), B( 5), B( 6), B( 7), B( 8) },
      { B( 8), B( 7), B( 6), B( 5), B( 4), B( 3), B( 2), B( 1), B( 0),
        B(17), B(16), B(15), B(14), B(13), B(12), B(11), B(10), B( 9),
        B(26), B(25), B(24), B(23), B(22), B(21), B(20), B(19), B(18),
        B(35), B(34), B(33), B(32), B(31), B(30), B(29), B(28), B(27),
        B(44), B(43), B(42), B(41), B(40), B(39), B(38), B(37), B(36),
        B(35), B(34), B(33), B(32), B(31), B(30), B(29), B(28), B(27),
        B(26), B(25), B(24), B(23), B(22), B(21), B(20), B(19), B(18),
        B(17), B(16), B(15), B(14), B(13), B(12), B(11), B(10), B( 9),
        B( 8), B( 7), B( 6), B( 5), B( 4), B( 3), B( 2), B( 1), B( 0) }
    };
    // clang-format on

    // Maximum number of simultaneously active features.
    static constexpr IndexType MaxActiveDimensions = 40;
    using IndexList                                = ValueList<IndexType, MaxActiveDimensions>;

    // Index of a feature for a given king position and another piece on some square
    template<Color Perspective>
    static IndexType make_board_index(Square s, Piece pc, Square ksq);
    
    template<Color Perspective>
    static IndexType make_hand_index(int bit, Square ksq);

    // Get a list of indices for active features
    template<Color Perspective>
    static void append_active_indices(const Position& pos, IndexList& active);

    // Get a list of indices for recently changed features
    template<Color Perspective>
    static void
    append_changed_indices(Square ksq, const DirtyPiece& dp, IndexList& removed, IndexList& added);

    // Returns whether the change stored in this StateInfo means
    // that a full accumulator refresh is required.
    static bool requires_refresh(const StateInfo* st, Color perspective);
};

}  // namespace Eval::NNUE::Features

#endif  // #ifndef NNUE_FEATURES_HALF_KA_V2_HM_H_INCLUDED
