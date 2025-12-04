// Input features and network structure used in NNUE evaluation function
// NNUE評価関数で用いる入力特徴量とネットワーク構造

#ifndef CLASSIC_NNUE_ARCHITECTURE_H_INCLUDED
#define CLASSIC_NNUE_ARCHITECTURE_H_INCLUDED

#include "../../config.h"

#if defined(EVAL_NNUE)

// Defines the network structure
// 入力特徴量とネットワーク構造が定義されたヘッダをincludeする

#include "architectures/halfkp_1024x2-8-96.h"

namespace YaneuraOu {
namespace Eval::NNUE {

	static_assert(kTransformedFeatureDimensions % kMaxSimdWidth == 0, "");
	static_assert(Network::kOutputDimensions == 1, "");
	static_assert(std::is_same<Network::OutputType, std::int32_t>::value, "");

	// Trigger for full calculation instead of difference calculation
	// 差分計算の代わりに全計算を行うタイミングのリスト
	constexpr auto kRefreshTriggers = RawFeatures::kRefreshTriggers;

} // namespace Eval::NNUE
} // namespace YaneuraOu

#endif  // defined(EVAL_NNUE)

#endif // #ifndef NNUE_ARCHITECTURE_H_INCLUDED
