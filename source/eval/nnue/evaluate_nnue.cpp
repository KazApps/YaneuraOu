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

#include "evaluate_nnue.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>

#include "network.h"
#include "nnue_misc.h"
#include "../../position.h"
#include "../../types.h"
#include "../../usi.h"
#include "../../thread.h"
#include "nnue_accumulator.h"


namespace Eval { namespace NNUE {

Network network({EvalFileDefaultName, "None", ""});

}}


void Eval::init() {}

void Eval::load_eval() {
    const std::string dir_name = Options["EvalDir"];
    const std::string file_name = EvalFileDefaultName;

    NNUE::network.load(dir_name, file_name);

    for (auto th : Threads)
        th->refreshTable.clear(NNUE::network);
}

// Evaluate is the evaluator for the outer world. It returns a static evaluation
// of the position from the point of view of the side to move.
Value Eval::evaluate(const Position&               pos,
                     Eval::NNUE::AccumulatorCache& cache,
                     int                           optimism) {

    assert(!pos.checkers());

    auto [psqt, positional] = NNUE::network.evaluate(pos, &cache);

    Value nnue = (125 * psqt + 131 * positional) / 128;

    // Blend optimism and eval with nnue complexity
    int nnueComplexity = std::abs(psqt - positional);
    optimism += optimism * nnueComplexity / 468;
    nnue -= nnue * nnueComplexity / 18000;

    int v = nnue
#if defined(USE_OPTIMISM)
    + optimism
#endif
    ;

    // Guarantee evaluation does not hit the tablebase range
    v = std::clamp(v, VALUE_TB_LOSS_IN_MAX_PLY + 1, VALUE_TB_WIN_IN_MAX_PLY - 1);

    return v;
}

// Like evaluate(), but instead of returning a value, it returns
// a string (suitable for outputting to stdout) that contains the detailed
// descriptions and values of each evaluation term. Useful for debugging.
// Trace scores are from white's point of view
std::string Eval::trace(Position& pos) {

    if (pos.checkers())
        return "Final evaluation: none (in check)";

    auto cache = std::make_unique<Eval::NNUE::AccumulatorCache>(NNUE::network);

    std::stringstream ss;
    ss << std::showpoint << std::noshowpos << std::fixed << std::setprecision(2);
    ss << '\n' << NNUE::trace(pos, NNUE::network, *cache) << '\n';

    ss << std::showpoint << std::showpos << std::fixed << std::setprecision(2) << std::setw(15);

    auto [psqt, positional] = NNUE::network.evaluate(pos, cache.get());
    Value v                 = psqt + positional;
    v                       = pos.side_to_move() == BLACK ? v : -v;
    ss << "NNUE evaluation        " << 0.01 * USI::to_cp(v) << " (black side)\n";

    v = evaluate(pos, *cache, VALUE_ZERO);
    v = pos.side_to_move() == BLACK ? v : -v;
    ss << "Final evaluation       " << 0.01 * USI::to_cp(v) << " (black side)";
    ss << " [with scaled NNUE, ...]";
    ss << "\n";

    return ss.str();
}
