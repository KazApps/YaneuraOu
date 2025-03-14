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

//Definition of input features HalfKAv2_hm of NNUE evaluation function

#include "half_ka_v2_hm.h"

#include "../../../bitboard.h"
#include "../../../position.h"
#include "../../../types.h"
#include "../nnue_accumulator.h"

namespace Eval::NNUE::Features {

template<Color Perspective>
inline IndexType orient(Square s, Square ksq) {
    if constexpr (Perspective == BLACK)
        return static_cast<IndexType>(file_of(ksq) <= FILE_4 ? horizontal_flip(s) : s);

    if constexpr (Perspective == WHITE)
        s = Flip(s);
    
	return static_cast<IndexType>(file_of(ksq) >= FILE_6 ? horizontal_flip(s) : s);
}

template<Color Perspective>
inline IndexType orient(int bit) {
    return Perspective == BLACK ? bit : (bit + 32 - 64 * (bit >= 32));
}

// Index of a feature for a given king position and another piece on some square
template<Color Perspective>
inline IndexType HalfKAv2_hm::make_board_index(Square s, Piece pc, Square ksq) {
    return IndexType(orient<Perspective>(s, ksq) + PieceSquareIndex[Perspective][pc]
                     + KingBuckets[Perspective][ksq]);
}

template<Color Perspective>
inline IndexType HalfKAv2_hm::make_hand_index(int bit, Square ksq) {
    return IndexType(orient<Perspective>(bit) + PS_NB + KingBuckets[Perspective][ksq]);
}

// Get a list of indices for active features
template<Color Perspective>
void HalfKAv2_hm::append_active_indices(const Position& pos, IndexList& active) {
    Square   ksq       = pos.king_square<Perspective>();
    Bitboard bb        = pos.pieces();
    u64      hand_bits = pos.hand_bits();

    while (bb)
    {
        Square s = bb.pop();
        active.push_back(make_board_index<Perspective>(s, pos.piece_on(s), ksq));
    }

    while (hand_bits)
        active.push_back(make_hand_index<Perspective>(pop_lsb(hand_bits), ksq));
}

// Explicit template instantiations
template void HalfKAv2_hm::append_active_indices<BLACK>(const Position& pos, IndexList& active);
template void HalfKAv2_hm::append_active_indices<WHITE>(const Position& pos, IndexList& active);
template IndexType HalfKAv2_hm::make_board_index<BLACK>(Square s, Piece pc, Square ksq);
template IndexType HalfKAv2_hm::make_board_index<WHITE>(Square s, Piece pc, Square ksq);
template IndexType HalfKAv2_hm::make_hand_index<BLACK>(int bit, Square ksq);
template IndexType HalfKAv2_hm::make_hand_index<WHITE>(int bit, Square ksq);

// Get a list of indices for recently changed features
template<Color Perspective>
void HalfKAv2_hm::append_changed_indices(Square            ksq,
                                         const DirtyPiece& dp,
                                         IndexList&        removed,
                                         IndexList&        added) {
    for (int i = 0; i < dp.dirty_num; ++i)
    {
        if (dp.from[i] == SQ_NB && dp.hand_bit[i] != -1)
            removed.push_back(make_hand_index<Perspective>(dp.hand_bit[i], ksq));
        else
            removed.push_back(make_board_index<Perspective>(dp.from[i], dp.piece[i], ksq));
        
        if (dp.to[i] == SQ_NB && dp.hand_bit[i] != -1)
            added.push_back(make_hand_index<Perspective>(dp.hand_bit[i], ksq));
        else
            if (dp.promote)
                added.push_back(make_board_index<Perspective>(dp.to[i], make_promoted_piece(dp.piece[i]), ksq));
            else
                added.push_back(make_board_index<Perspective>(dp.to[i], dp.piece[i], ksq));
    }
}

// Explicit template instantiations
template void HalfKAv2_hm::append_changed_indices<BLACK>(Square            ksq,
                                                         const DirtyPiece& dp,
                                                         IndexList&        removed,
                                                         IndexList&        added);
template void HalfKAv2_hm::append_changed_indices<WHITE>(Square            ksq,
                                                         const DirtyPiece& dp,
                                                         IndexList&        removed,
                                                         IndexList&        added);

bool HalfKAv2_hm::requires_refresh(const StateInfo* st, Color perspective) {
    return st->dirtyPiece.piece[0] == make_piece(perspective, KING);
}

}  // namespace Eval::NNUE::Features
