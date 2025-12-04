#include "benchmark.h"
#include "numa.h"

#include <fstream>
#include <iostream>

// ----------------------------------
//  USI拡張コマンド "bench"(ベンチマーク)
// ----------------------------------
namespace {

// benchmark用デフォルトの局面集
const std::vector<std::string> Defaults = {
	"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
	"8l/1l+R2P3/p2pBG1pp/kps1p4/Nn1P2G2/P1P1P2PP/1PS6/1KSG3+r1/LN2+p3L w Sbgn3p 124",
	"lnsgkgsnl/1r7/p1ppp1bpp/1p3pp2/7P1/2P6/PP1PPPP1P/1B3S1R1/LNSGKG1NL b - 9",
	"l4S2l/4g1gs1/5p1p1/pr2N1pkp/4Gn3/PP3PPPP/2GPP4/1K7/L3r+s2L w BS2N5Pb 1",
	"6n1l/2+S1k4/2lp4p/1np1B2b1/3PP4/1N1S3rP/1P2+pPP+p1/1p1G5/3KG2r1 b GSN2L4Pgs2p 1",
	"l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w RGgsn5p 1",
	"l1r6/4S2k1/p2pb1gsg/5pp2/1pBnp3p/5PP2/PP1P1G1S1/6GKL/L1R5L b Ps3n5p 93",
	"5+P+B+R1/1kg2+P1+P+R/1g1s2KG1/3g4p/2p1pS3/1+p+l1s4/4B1N1P/9/4P4 b S3N3L9P 221",
	"ln3g1nl/1r1sg1sk1/p1p1ppbp1/1p1p2p1p/2P6/3P4P/PP2PPPP1/1BRS2SK1/LNG2G1NL b - 23",
	"l1+R4nk/5rgs1/3pp1gp1/p4pp1l/1p5Pp/4PSP2/P4PNG1/4G4/L5K1L w 2BP2s2n4p 88",
	"6B1+S/2gg5/4lp1+P1/6p1p/4pP1R1/Ppk1P1P1P/2+p2GK2/5S3/1+n3+r2L b B2SN2L2Pg2n4p 149",
	"7nl/3+P1kg2/4pb1ps/2r2NP1p/l1P2P1P1/s7P/PN2P4/KGB2G3/1N1R4L w G5P2sl2p 98",
	"l4Grnl/1B2+B1gk1/p1n3sp1/4ppp1p/P1S2P1P1/1PGP2P1P/3pP2g1/1K4sR1/LN6L w 3Psn 78",
	"ln6l/2gkgr1s1/1p1pp1n1p/3s1pP2/p8/1P1PBPb2/PS2P1NpP/1K1G2R2/LN1G4L w 3Psp 58",
	"ln1gk2nl/1rs3g2/p3ppspp/2pp2p2/1p5PP/2P6/PPSPPPP2/2G3SR1/LN2KG1NL b Bb 21",
	"ln7/1r2g1g2/2pspk1bn/pp1p2PB1/5pp1p/P1P2P3/1PSPP3+l/3K2S2/LN1G1G3 b Srnl3p 59",
	"4g2nl/5skn1/p1pppp1p1/6p+b1/4P4/3+R1SL1p/P3GPPP1/1+r2SS1KP/3PL2NL w GPbgn2p 128",
	"lnsgk2nl/1r4gs1/p1pppp1pp/6p2/1p5P1/2P6/PPSPPPP1P/7R1/LN1GKGSNL b Bb 13",
	"ln1g1gsnl/1r1s2k2/p1pp1p1p1/6p1p/1p7/2P5P/PPS+b1PPP1/2B3K2/LN1GRGSNL w P2p 26",
	"l2sk2nl/2g2s1g1/2n1pp1pp/pr4p2/1p6P/P2+b+RP1P1/1P2PSP2/5K3/L2G1G1NL b SPbn3p 51",
};

// speedtestコマンドでテストする用。
// やねうら王ではDefaultsと同じ局面にしておく。
const std::vector<std::vector<std::string>> BenchmarkPositions = { {Defaults} };

} // namespace

namespace YaneuraOu::Benchmark {

// Builds a list of UCI commands to be run by bench. There
// are five parameters: TT size in MB, number of search threads that
// should be used, the limit value spent for each position, a file name
// where to look for positions in FEN format, and the type of the limit:
// depth, perft, nodes and movetime (in milliseconds). Examples:
//
// bench                            : search default positions up to depth 13
// bench 64 1 15                    : search default positions up to depth 15 (TT = 64MB)
// bench 64 1 100000 default nodes  : search default positions for 100K nodes each
// bench 64 4 5000 current movetime : search current position with 4 threads for 5 sec
// bench 16 1 5 blah perft          : run a perft 5 on positions in file "blah"

// benchで実行するUCIコマンドのリストを構築します。
// 引数は以下の5つです：TTサイズ（MB単位）、使用するサーチスレッド数、
// 各局面に費やす制限値、FEN形式の局面を読み込むファイル名、
// そして制限の種類（depth、perft、nodes、movetime（ミリ秒））。
// 使用例：
// bench                            : デフォルトの局面を深さ13まで探索
// bench 64 1 15                    : デフォルトの局面を深さ15まで探索（TT=64MB）
// bench 64 1 100000 default nodes  : デフォルトの局面を各局面100,000ノードで探索
// bench 64 4 5000 current movetime : 現在の局面を4スレッドで5秒間探索
// bench 16 1 5 blah perft          : ファイル"blah"内の局面に対してperft 5を実行

/*
	📓 やねうら王では、制限の種類をデフォルトで、depthからmovetimeに変更しているので、
		上の例のようにdepthで深さ15まで探索するには

			bench 64 1 15 default depth

		のように指定する必要がある。
*/

std::vector<std::string> setup_bench(const std::string& currentFen, std::istream& is) {
	std::vector<std::string> fens, list;
	std::string              go, token;

	// Assign default values to missing arguments
	std::string ttSize    = (is >> token) ? token : "16";
	std::string threads   = (is >> token) ? token : "1";
	std::string limit     = (is >> token) ? token : "12";
	std::string fenFile   = (is >> token) ? token : "default";
	std::string limitType = (is >> token) ? token : "depth";

	go = limitType == "eval" ? "eval" : "go " + limitType + " " + limit;

	if (fenFile == "default")
		fens = Defaults;

	else if (fenFile == "current")
		fens.push_back(currentFen);

	else
	{
		std::string   fen;
		std::ifstream file(fenFile);

		if (!file.is_open())
		{
			std::cerr << "Unable to open file " << fenFile << std::endl;
			exit(EXIT_FAILURE);
		}

		while (getline(file, fen))
			if (!fen.empty())
				fens.push_back(fen);

		file.close();
	}

	list.emplace_back("setoption name Threads value " + threads);
#if STOCKFISH
	list.emplace_back("setoption name Hash value " + ttSize);
#else
    list.emplace_back("setoption name Hash value " + ttSize);
#endif
	// 🤔 どうせ内部的にしか使わない符号みたいなものなので"usinewgame"に変更しないことにする。
    list.emplace_back("ucinewgame");

	for (const std::string& fen : fens)
		if (fen.find("setoption") != std::string::npos)
			list.emplace_back(fen);
		else
		{
#if STOCKFISH
			list.emplace_back("position fen " + fen);
#else
            list.emplace_back("position sfen " + fen);
#endif
			list.emplace_back(go);
		}

	return list;
}

BenchmarkSetup setup_benchmark(std::istream& is) {

	// TT_SIZE_PER_THREAD is chosen such that roughly half of the hash is used all positions
	// for the current sequence have been searched.

	// TT_SIZE_PER_THREAD は、現在のシーケンスのすべての局面を探索した際に
	// ハッシュの約半分が使用されるように選定されています。

	static constexpr int TT_SIZE_PER_THREAD = 128;

	static constexpr int DEFAULT_DURATION_S = 150;

	BenchmarkSetup setup{};

	// Assign default values to missing arguments
	int desiredTimeS;

	if (!(is >> setup.threads))
		setup.threads = get_hardware_concurrency();
	else
		setup.originalInvocation += std::to_string(setup.threads);

	if (!(is >> setup.ttSize))
		setup.ttSize = TT_SIZE_PER_THREAD * setup.threads;
	else
		setup.originalInvocation += " " + std::to_string(setup.ttSize);

	if (!(is >> desiredTimeS))
		desiredTimeS = DEFAULT_DURATION_S;
	else
		setup.originalInvocation += " " + std::to_string(desiredTimeS);

	setup.filledInvocation += std::to_string(setup.threads) + " " + std::to_string(setup.ttSize)
		+ " " + std::to_string(desiredTimeS);

	auto getCorrectedTime = [&](int ply) {
		// time per move is fit roughly based on LTC games
		// seconds = 50/{ply+15}
		// ms = 50000/{ply+15}
		// with this fit 10th move gets 2000ms
		// adjust for desired 10th move time
		return 50000.0 / (static_cast<double>(ply) + 15.0);
		};

	float totalTime = 0;
	for (const auto& game : BenchmarkPositions)
	{
		int ply = 1;
		for (int i = 0; i < static_cast<int>(game.size()); ++i)
		{
			const float correctedTime = float(getCorrectedTime(ply));
			totalTime += correctedTime;
			ply += 1;
		}
	}

	float timeScaleFactor = static_cast<float>(desiredTimeS * 1000) / totalTime;

	for (const auto& game : BenchmarkPositions)
	{
		setup.commands.emplace_back("ucinewgame");
		int ply = 1;
		for (const std::string& fen : game)
		{
#if STOCKFISH
			setup.commands.emplace_back("position fen " + fen);
#else
            setup.commands.emplace_back("position sfen " + fen);
#endif
			const int correctedTime = static_cast<int>(getCorrectedTime(ply) * timeScaleFactor);
			setup.commands.emplace_back("go movetime " + std::to_string(correctedTime));

			ply += 1;
		}
	}

	return setup;
}

} // namespace YaneuraOu
