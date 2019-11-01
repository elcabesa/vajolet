#include <list>
#include "gtest/gtest.h"
#include "parameters.h"
#include "position.h"

struct positions
{
	const std::string Fen;
	const Move m;
	const Score score;
	
	positions( const std::string& f, const Move& mo, const Score s ): Fen(f), m(mo), score(s){}
	 
};




TEST(seeTest, see)
{

	const Score P = initialPieceValue[Pawns][0];
	const Score N = initialPieceValue[Knights][0];
	const Score B = initialPieceValue[Bishops][0];
	const Score R= initialPieceValue[Rooks][0];
	const Score Q = initialPieceValue[Queens][0];
	//const Score K = initialPieceValue[King][0];
	/* manythanks to Fabio Gobbato for this list of fen Tests*/
	std::list<positions> posList;
		/* capture initial move */
	posList.push_back( positions("3r3k/3r4/2n1n3/8/3p4/2PR4/1B1Q4/3R3K w - - 0 1", 							Move(D3,D4), P - R + N - P ));
	posList.push_back( positions("1k1r4/1ppn3p/p4b2/4n3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1",					Move(D3,E5), N - N + B - R + N ));
	posList.push_back( positions("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1",				Move(D3,E5), P - N ));
	posList.push_back( positions("rnb2b1r/ppp2kpp/5n2/4P3/q2P3B/5R2/PPP2PPP/RN1QKB2 w Q - 1 1",				Move(H4,F6), N - B + P ));
	posList.push_back( positions("r2q1rk1/2p1bppp/p2p1n2/1p2P3/4P1b1/1nP1BN2/PP3PPP/RN1QR1K1 b - - 1 1",	Move(G4,F3), N - B ));
	posList.push_back( positions("r1bqkb1r/2pp1ppp/p1n5/1p2p3/3Pn3/1B3N2/PPP2PPP/RNBQ1RK1 b kq - 2 1",		Move(C6,D4), P - N + N - P));
	posList.push_back( positions("r1bq1r2/pp1ppkbp/4N1p1/n3P1B1/8/2N5/PPP2PPP/R2QK2R w KQ - 2 1",			Move(E6,G7), B - N ));
	posList.push_back( positions("r1bq1r2/pp1ppkbp/4N1pB/n3P3/8/2N5/PPP2PPP/R2QK2R w KQ - 2 1",				Move(E6,G7), B ));			// add by me
	posList.push_back( positions("rnq1k2r/1b3ppp/p2bpn2/1p1p4/3N4/1BN1P3/PPP2PPP/R1BQR1K1 b kq - 0 1",		Move(D6,H2), P - B ));
	posList.push_back( positions("rn2k2r/1bq2ppp/p2bpn2/1p1p4/3N4/1BN1P3/PPP2PPP/R1BQR1K1 b kq - 5 1",		Move(D6,H2), P ));
	posList.push_back( positions("r2qkbn1/ppp1pp1p/3p1rp1/3Pn3/4P1b1/2N2N2/PPP2PPP/R1BQKB1R b KQq - 2 1",	Move(G4,F3), N - B + P ));
	posList.push_back( positions("rnbq1rk1/pppp1ppp/4pn2/8/1bPP4/P1N5/1PQ1PPPP/R1B1KBNR b KQ - 1 1",		Move(B4,C3), N - B ));
	posList.push_back( positions("r4rk1/3nppbp/bq1p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - - 1 1",			Move(B6,B2), P - Q ));
	posList.push_back( positions("r4rk1/1q1nppbp/b2p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - - 1 1",			Move(F6,D5), P - N ));
	posList.push_back( positions("1r3r2/5p2/4p2p/2k1n1P1/2PN1nP1/1P3P2/8/2KR1B1R b - - 0 29",				Move(B8,B3), P - R ));
	posList.push_back( positions("1r3r2/5p2/4p2p/4n1P1/kPPN1nP1/5P2/8/2KR1B1R b - - 0 1",					Move(B8,B4), P ));
	posList.push_back( positions("2r2rk1/5pp1/pp5p/q2p4/P3n3/1Q3NP1/1P2PP1P/2RR2K1 b - - 1 22",				Move(C8,C1), R - R ));
	posList.push_back( positions("1r3r1k/p4pp1/2p1p2p/qpQP3P/2P5/3R4/PP3PP1/1K1R4 b - - 0 1",				Move(A5,A2), P - Q ));
	posList.push_back( positions("1r5k/p4pp1/2p1p2p/qpQP3P/2P2P2/1P1R4/P4rP1/1K1R4 b - - 0 1",				Move(A5,A2), P ));
	posList.push_back( positions("r2q1rk1/1b2bppp/p2p1n2/1ppNp3/3nP3/P2P1N1P/BPP2PP1/R1BQR1K1 w - - 4 14",	Move(D5,E7), B - N ));
	posList.push_back( positions("rnbqrbn1/pp3ppp/3p4/2p2k2/4p3/3B1K2/PPP2PPP/RNB1Q1NR w - - 0 1",			Move(D3,E4), P ));
		/* non capture initial move */
	posList.push_back( positions("rnb1k2r/p3p1pp/1p3p1b/7n/1N2N3/3P1PB1/PPP1P1PP/R2QKB1R w KQkq - 0 1",		Move(E4,D6), 0 - N + P ));
	posList.push_back( positions("r1b1k2r/p4npp/1pp2p1b/7n/1N2N3/3P1PB1/PPP1P1PP/R2QKB1R w KQkq - 0 1",		Move(E4,D6), 0 - N + N ));
	posList.push_back( positions("2r1k2r/pb4pp/5p1b/2KB3n/4N3/2NP1PB1/PPP1P1PP/R2Q3R w k - 0 1",			Move(D5,C6), 0 - B ));
	posList.push_back( positions("2r1k2r/pb4pp/5p1b/2KB3n/1N2N3/3P1PB1/PPP1P1PP/R2Q3R w k - 0 1",			Move(D5,C6), 0 - B + B ));
	posList.push_back( positions("2r1k3/pbr3pp/5p1b/2KB3n/1N2N3/3P1PB1/PPP1P1PP/R2Q3R w - - 0 1",			Move(D5,C6), 0 - B + B - N ));
		/* initial move promotion */
	posList.push_back( positions("5k2/p2P2pp/8/1pb5/1Nn1P1n1/5Q2/PPP4P/R3K1NR w KQ - 0 1",					Move(D7,D8, Move::fpromotion, Move::promQueen ), 0 + ( -P + Q ) )); 	// add by me
	posList.push_back( positions("r4k2/p2P2pp/8/1pb5/1Nn1P1n1/5Q2/PPP4P/R3K1NR w KQ - 0 1",					Move(D7,D8, Move::fpromotion, Move::promQueen ), 0 + ( -P + Q ) - Q ));
	posList.push_back( positions("5k2/p2P2pp/1b6/1p6/1Nn1P1n1/8/PPP4P/R2QK1NR w KQ - 0 1",					Move(D7,D8, Move::fpromotion, Move::promQueen ), 0 + ( -P + Q ) - Q + B ));
	posList.push_back( positions("4kbnr/p1P1pppp/b7/4q3/7n/8/PP1PPPPP/RNBQKBNR w KQk - 0 1",				Move(C7,C8, Move::fpromotion, Move::promQueen ), 0 + ( -P + Q ) - Q ));
	posList.push_back( positions("4kbnr/p1P1pppp/b7/4q3/7n/8/PPQPPPPP/RNB1KBNR w KQk - 0 1",				Move(C7,C8, Move::fpromotion, Move::promQueen ), 0 + ( -P + Q ) - Q + B ));
	posList.push_back( positions("4kbnr/p1P1pppp/b7/4q3/7n/8/PPQPPPPP/RNB1KBNR w KQk - 0 1",				Move(C7,C8, Move::fpromotion, Move::promKnight ), 0 + ( -P + N ) ));
		/* initial move En Passant */
	posList.push_back( positions("4kbnr/p1P4p/b1q5/5pP1/4n3/5Q2/PP1PPP1P/RNB1KBNR w KQk f6 0 2",			Move(G5,F6, Move::fenpassant), P - P ));
	posList.push_back( positions("4kbnr/p1P4p/b1q5/5pP1/4n2Q/8/PP1PPP1P/RNB1KBNR w KQk f6 0 2",				Move(G5,F6, Move::fenpassant), P - P ));
	posList.push_back( positions("4kb1r/p1P4p/b4Q2/8/8/8/PP1PPP1P/RNB1KBNR b KQk - 0 3 ",					Move(G5,F6, Move::fenpassant), P ));	// add by me to verify removal of P from f5
		/* initial move capture promotion */
	posList.push_back( positions("1n2kb1r/p1P4p/2qb4/5pP1/4n2Q/8/PP1PPP1P/RNB1KBNR w KQk - 0 1",			Move(C7,B8, Move::fpromotion, Move::promQueen ), N + ( -P + Q ) -Q ));

	posList.push_back( positions("rnbqk2r/pp3ppp/2p1pn2/3p4/3P4/N1P1BN2/PPB1PPPb/R2Q1RK1 w kq - 0 1",		Move(G1,H2), B ));

	posList.push_back( positions("3N4/2K5/2n1n3/1k6/8/8/8/8 b - - 0 1",										Move(C6,D8), N )); // add by me
	posList.push_back( positions("3N4/2K5/2n5/1k6/8/8/8/8 b - - 0 1",										Move(C6,D8), 0 )); // add by me


		/* promotion inside the loop */
	posList.push_back( positions("3N4/2P5/2n5/1k6/8/8/8/4K3 b - - 0 1",										Move(C6,D8), N - ( N - P + Q ) ));
	posList.push_back( positions("3N4/2P5/2n5/1k6/8/8/8/4K3 b - - 0 1",										Move(C6,B8), 0 - ( N - P + Q ) )); // add by me
	posList.push_back( positions("3n3r/2P5/8/1k6/8/8/3Q4/4K3 w - - 0 1",									Move(D2,D8), N ));
	posList.push_back( positions("3n3r/2P5/8/1k6/8/8/3Q4/4K3 w - - 0 1",									Move(C7,D8, Move::fpromotion, Move::promQueen), ( N - P + Q ) - Q + R ));
		/* double promotion inside the loop */
	posList.push_back( positions("r2n3r/2P1P3/4N3/1k6/8/8/8/4K3 w - - 0 1",									Move(E6,D8), N ));
	posList.push_back( positions("8/8/8/1k6/6b1/4N3/2p3K1/3n4 w - - 0 1",									Move(E3,D1), N - ( N - P + Q ) ));
	posList.push_back( positions("8/8/1k6/8/8/2N1N3/2p1p1K1/3n4 w - - 0 1",									Move(C3,D1), N - ( N - P + Q ) ));

	posList.push_back( positions("8/8/1k6/8/8/2N1N3/4p1K1/3n4 w - - 0 1",									Move(C3,D1), N - (N - P + Q ) + Q ));

	posList.push_back( positions("r1bqk1nr/pppp1ppp/2n5/1B2p3/1b2P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",		Move(E1,G1, Move::fcastle) , 0 ));


	// todo mossa castle
	// cattura diretta del re??
	
	Position pos;
	for (auto & p : posList)
	{
		pos.setupFromFen(p.Fen); 
		Score s = pos.see(p.m);
		
		EXPECT_EQ(s, p.score);
	}
}


