/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#include "movepicker.h"
#include "position.h"
#include "searchData.h"
// cppcheck-suppress uninitMemberVar symbolName=MovePicker::_killerPos
// cppcheck-suppress uninitMemberVar symbolName=MovePicker::_captureThreshold
MovePicker::MovePicker( const Position& p, const SearchData& sd, unsigned int ply, const Move& ttm ): _pos(p), _mg(p.getMoveGen()), _sd(sd), _ply(ply), _ttMove(ttm)
{
	if( _pos.isInCheck() )
	{
		_stagedGeneratorState = eStagedGeneratorState::getTTevasion;
	}
	else
	{
		_stagedGeneratorState = eStagedGeneratorState::getTT;
	}
}


Move MovePicker::getNextMove()
{
	while( true )
	{
		switch( _stagedGeneratorState )
		{
		case eStagedGeneratorState::generateCaptureMoves:
		case eStagedGeneratorState::generateQuiescentMoves:
		case eStagedGeneratorState::generateQuiescentCaptures:
		case eStagedGeneratorState::generateProbCutCaptures:

			_mg.generateMoves<Movegen::genType::captureMg>( _moveList );
			
			_moveList.ignoreMove( _ttMove );
			
			_scoreCaptureMoves();
			
			++_stagedGeneratorState;
			break;

		case eStagedGeneratorState::generateQuietMoves:

			_moveList.reset();

			_mg.generateMoves<Movegen::genType::quietMg>( _moveList );
			
			_moveList.ignoreMove( _ttMove);
			_moveList.ignoreMove( _killerMoves[0] );
			_moveList.ignoreMove( _killerMoves[1] );
			_moveList.ignoreMove( _counterMoves[0] );
			_moveList.ignoreMove( _counterMoves[1] );

			_scoreQuietMoves();

			++_stagedGeneratorState;
			break;

		case eStagedGeneratorState::generateCaptureEvasionMoves:

			_mg.generateMoves<Movegen::genType::captureEvasionMg>( _moveList );
			_moveList.ignoreMove( _ttMove );

			// non usate dalla generazione delle mosse, ma usate dalla ricerca!!
			_killerMoves[0] = _sd.getKillers( _ply, 0 );
			_killerMoves[1] = _sd.getKillers( _ply, 1 );

			_scoreCaptureMoves();

			++_stagedGeneratorState;
			break;

		case eStagedGeneratorState::generateQuietEvasionMoves:

			_mg.generateMoves<Movegen::genType::quietEvasionMg>( _moveList );
			_moveList.ignoreMove( _ttMove );

			_scoreQuietEvasion();
			
			++_stagedGeneratorState;
			break;

		case eStagedGeneratorState::generateQuietChecks:

			_moveList.reset();
			_mg.generateMoves<Movegen::genType::quietChecksMg>( _moveList );
			_moveList.ignoreMove( _ttMove );

			_scoreQuietMoves();

			++_stagedGeneratorState;
			break;

		case eStagedGeneratorState::iterateQuietMoves:
		case eStagedGeneratorState::iterateQuiescentCaptures:
		case eStagedGeneratorState::iterateCaptureEvasionMoves:
		case eStagedGeneratorState::iterateQuiescentMoves:
		case eStagedGeneratorState::iterateQuietChecks:
		case eStagedGeneratorState::iterateQuietEvasionMoves:

			if( Move mm; ( mm = _moveList.findNextBestMove() ) )
			{
				return mm;
			}
			else
			{
				++_stagedGeneratorState;
			}
			break;
			
		case eStagedGeneratorState::iterateGoodCaptureMoves:

			if( Move mm; ( mm = _moveList.findNextBestMove() ) )
			{
				if( ( _pos.seeSign( mm ) >= 0 ) || _pos.moveGivesSafeDoubleCheck(mm) )
				{
					return mm;
				}
				else
				{
					_badCaptureList.insert( mm );
				}

			}
			else
			{
				_killerMoves[0] = _sd.getKillers( _ply, 0 );
				_killerMoves[1] = _sd.getKillers( _ply, 1 );

				if( const Move& previousMove = _pos.getActualState().getCurrentMove() )
				{
					_counterMoves[0] = _sd.getCounterMove().getMove( _pos.getPieceAt( previousMove.getTo()), previousMove.getTo(), 0);
					_counterMoves[1] = _sd.getCounterMove().getMove( _pos.getPieceAt( previousMove.getTo()), previousMove.getTo(), 1);
				}
				else
				{
					_counterMoves[0] = Move::NOMOVE;
					_counterMoves[1] = Move::NOMOVE;
				}

				_killerPos = 0;
				++_stagedGeneratorState;
			}
			break;
			
		case eStagedGeneratorState::iterateProbCutCaptures:
			if( Move mm; ( mm = _moveList.findNextBestMove() ) )
			{
				if( _pos.see( mm ) >= _captureThreshold)
				{
					return mm;
				}
			}
			else
			{
				return Move::NOMOVE;
			}
			break;
			
		case eStagedGeneratorState::iterateBadCaptureMoves:
			return _badCaptureList.getNextMove();
			break;
			
		case eStagedGeneratorState::getKillers:
			if( _killerPos < 2 )
			{
				if( Move& t = _killerMoves[ _killerPos++ ]; ( t != _ttMove) && !_pos.isCaptureMove( t ) && _pos.isMoveLegal( t ) )
				{
					return t;
				}
			}
			else
			{
				_killerPos = 0;
				++_stagedGeneratorState;
			}
			break;
			
		case eStagedGeneratorState::getCounters:
			if( _killerPos < 2 )
			{
				if( Move& t = _counterMoves[ _killerPos++ ]; ( t != _ttMove ) && !isKillerMove(t) && !_pos.isCaptureMove( t ) && _pos.isMoveLegal( t ) )
				{
					return t;
				}
			}
			else
			{
				++_stagedGeneratorState;
			}
			break;
			
		case eStagedGeneratorState::getTT:
		case eStagedGeneratorState::getTTevasion:
		case eStagedGeneratorState::getQsearchTT:
		case eStagedGeneratorState::getQsearchTTquiet:
		case eStagedGeneratorState::getProbCutTT:
			++_stagedGeneratorState;
			if( _pos.isMoveLegal( _ttMove ) )
			{
				return _ttMove;
			}
			break;
			
		default:
			return Move::NOMOVE;
			break;
		}
	}

	return Move::NOMOVE;
}

inline void MovePicker::_scoreCaptureMoves()
{
	for( auto& m : _moveList )
	{
		Score s = _pos.getMvvLvaScore( m )
				+ _sd.getCaptureHistory().getValue( _pos.getPieceAt( m.getFrom()) , m , _pos.getPieceAt( m.getTo() ) ) * 50; // history of capture
		m.setScore( s );
	}

}

inline void MovePicker::_scoreQuietMoves()
{
	for( auto& m : _moveList )
	{
		m.setScore( _sd.getHistory().getValue( Color( _pos.isBlackTurn() ), m ) );
	}
}

inline void MovePicker::_scoreQuietEvasion()
{
	for( auto& m : _moveList )
	{
		Score s = ( _pos.getPieceAt( m.getFrom() ) );
		if( _pos.getPieceTypeAt( m.getFrom() ) == King )
		{
			s = 20;
		}
		s *= 500000;
		s += _sd.getHistory().getValue( Color( _pos.isBlackTurn() ), m );
		m.setScore( s );
	}
}

bool MovePicker::isKillerMove(Move &m) const
{
	return m == _killerMoves[0] || m == _killerMoves[1];
}

short int MovePicker::setupQuiescentSearch( const bool inCheck, const int depth )
{
	if( inCheck)
	{
		_stagedGeneratorState = eStagedGeneratorState::getTTevasion;
		return -1;
	}
	else
	{
		if( depth >= 0 )
		{
			_stagedGeneratorState = eStagedGeneratorState::getQsearchTTquiet;
			return -1;
		}
		else
		{
			_stagedGeneratorState = eStagedGeneratorState::getQsearchTT;
			return -2;
		}
	}
}

void MovePicker::setupProbCutSearch( const bitboardIndex capturePiece )
{
	//if( _pos.isInCheck() )
	//{
	//	_stagedGeneratorState = eStagedGeneratorState::getTTevasion;
	//}
	//else
	//{
		_stagedGeneratorState = eStagedGeneratorState::getProbCutTT;
	//}

	_captureThreshold = _pos.getPieceValue(capturePiece)[0];
	if( _pos.isMoveLegal( _ttMove) && ( ( !_pos.isCaptureMove( _ttMove ) ) || ( _pos.see( _ttMove ) < _captureThreshold ) ) )
	{
		_ttMove = Move::NOMOVE;
	}
}
