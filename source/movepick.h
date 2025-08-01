﻿#ifndef MOVEPICK_H_INCLUDED
#define MOVEPICK_H_INCLUDED

#include "config.h"
#if defined(USE_MOVE_PICKER)

#include <array>
//#include <cassert>
#include <cmath>
//#include <cstdint>
//#include <cstdlib>
#include <limits>
//#include <type_traits>

#include "history.h"
//#include "movegen.h"
#include "types.h"
#include "position.h"

// -----------------------
//   MovePicker
// -----------------------

// 指し手オーダリング器


// MovePicker class is used to pick one pseudo-legal move at a time from the
// current position. The most important method is next_move(), which returns a
// new pseudo-legal move each time it is called, until there are no moves left,
// when Move::none() is returned. In order to improve the efficiency of the
// alpha-beta algorithm, MovePicker attempts to return the moves which are most
// likely to get a cut-off first.
//
// MovePickerクラスは、現在の局面から、(呼び出し)一回につきpseudo legalな指し手を一つ取り出すのに用いる。
// 最も重要なメソッドはnext_move()であり、これは、新しいpseudo legalな指し手を呼ばれるごとに返し、
// (返すべき)指し手が無くなった場合には、Move::none()を返す。
// alpha beta探索の効率を改善するために、MovePickerは最初に(早い段階のnext_move()で)カットオフ(beta cut)が
// 最も出来そうな指し手を返そうとする。
//
class MovePicker
{
	// 生成順に次の1手を取得するのか、オーダリング上、ベストな指し手を取得するのかの定数
	// (このクラスの内部で用いる。)
	enum PickType {
		Next,
		Best
	};

public:

	// このクラスは指し手生成バッファが大きいので、コピーして使うような使い方は禁止。
	MovePicker(const MovePicker&)            = delete;
	MovePicker& operator=(const MovePicker&) = delete;

	// 通常探索(main search)と静止探索から呼び出されるとき用のコンストラクタ。
	MovePicker(const Position& pos_,
		Move ttMove_,
		Depth depth_,
		const ButterflyHistory* mh,
		const LowPlyHistory*,
		const CapturePieceToHistory* cph,
		const PieceToHistory** ch,
#if defined(ENABLE_PAWN_HISTORY)
		const PawnHistory* ph,
#endif
		int ply_
	);

	// 通常探索時にProbCutの処理から呼び出されるのコンストラクタ。
	// SEEの値がth以上となるcaptureの指してだけを生成する。
	// threshold_ = 直前に取られた駒の価値。これ以下の捕獲の指し手は生成しない。
	// capture_or_pawn_promotion()に該当する指し手しか返さない。
	MovePicker(const Position&, Move ttMove_, int threshold_, const CapturePieceToHistory*);

	// 呼び出されるごとに新しいpseudo legalな指し手をひとつ返す。
	// 指し手が尽きればMove::none()が返る。
	// 置換表の指し手(ttMove)を返したあとは、それを取り除いた指し手を返す。
	Move next_move();

	// next_move()で、quietな指し手をskipするためのフラグをセットする。
	void skip_quiet_moves();

private:
	template <PickType T, typename Pred> Move select(Pred);

	// 指し手のオーダリング用
	// GenType == CAPTURES : 捕獲する指し手のオーダリング
	// GenType == QUIETS   : 捕獲しない指し手のオーダリング
	// GenType == EVASIONS : 王手回避の指し手のオーダリング
	template<MOVE_GEN_TYPE> void score();

	// range-based forを使いたいので。
	// 現在の指し手から終端までの指し手が返る。
	ExtMove* begin() { return cur; }
	ExtMove* end() { return endMoves; }

	const Position&              pos;

	// コンストラクタで渡されたhistroyのポインタを保存しておく変数。
	const ButterflyHistory*      mainHistory;
	const LowPlyHistory*         lowPlyHistory;
	const CapturePieceToHistory* captureHistory;
	const PieceToHistory**       continuationHistory;
#if defined(ENABLE_PAWN_HISTORY)
	const PawnHistory*           pawnHistory;
#endif

	// 置換表の指し手(コンストラクタで渡される)
	Move ttMove;

	// cur            : 次に返す指し手
	// endMoves       : 生成された指し手の末尾
	// endBadCapture  : BadCaptureの終端(captureの指し手を試すフェイズでのendMovesから後方に向かって悪い捕獲の指し手を移動させていく時に用いる)
	// beginBadQuiets : あとで
	// endBadQuiets   : あとで
	ExtMove *cur, *endMoves, *endBadCaptures, *beginBadQuiets, *endBadQuiets;

	// 指し手生成の段階
	int stage;

	// ProbCut用の指し手生成に用いる、直前の指し手で捕獲された駒の価値
	Value threshold;

	// コンストラクタで渡された探索深さ
	Depth depth;

	// コンストラクタで渡されたrootからの手数
	int   ply;

	// next_move()で、quietな指し手をskipするかのフラグ
	bool  skipQuiets = false;

	// 指し手生成バッファ
	// 最大合法手の数 = 593 , これを要素数が32の倍数になるようにpaddingすると608。
	// 32byteの倍数になるようにcurを使いたいので+3(ttmoveとkillerの分)して、611。
#if !defined(USE_SUPER_SORT)
	ExtMove moves[MAX_MOVES];
#else
	ExtMove moves[611];
#endif
};

#endif // defined(USE_MOVE_PICKER)

#endif // #ifndef MOVEPICK_H_INCLUDED

