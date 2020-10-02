#include <chrono>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <string>

#include "libchess.h"
#include "move.h"
#include "movepicker.h"
#include "nnue.h"
#include "position.h"
#include "searchLimits.h"
#include "searchResult.h"
#include "thread.h"
#include "transposition.h"
#include "uciOutput.h"


class Fen {
public:
    Fen(std::istringstream& ss) {
        std::string piece;
        std::string side;
        std::string castling;
        std::string enpassant;
        ss >> piece >> side >> castling >> enpassant;
        _fen = piece + " " + side + " " + castling + " " + enpassant;  
    }
    const std::string& get() const {return _fen;} 
private:
    std::string _fen;   
};

class Operations{
    using operations_t = std::map<std::string, std::string>;
public:
    using iterator = operations_t::iterator;
    using const_iterator = operations_t::const_iterator;
public:
    Operations(std::istringstream& ss) {
        std::string key;
        std::string value;
        while (ss >> key) {
            std::getline(ss, value, ';');
            _operations[key] = value.substr(1);
        }
    }

    void print() const {
        for(const auto& op: _operations) {
            std::cout<<op.first << ": '" << op.second << "'" << std::endl;
        }
    }

    std::string get(const std::string& key) const {
        // todo return the value only if exist...
        // std::optional??
        return _operations.at(key);
    }

    bool contains(const std::string& key) const {
        auto it = _operations.find(key);
        return it != _operations.end();
    }

    iterator begin() { return _operations.begin(); }
    iterator end() { return _operations.end(); }
    const_iterator begin() const { return _operations.begin(); }
    const_iterator end() const { return _operations.end(); }
    const_iterator cbegin() const { return _operations.cbegin(); }
    const_iterator cend() const { return _operations.cend(); }

private:
    operations_t _operations;
};

class EpdRecord {
public:
    EpdRecord(std::istringstream& ss): _fen(ss), _operations(ss){}

    void print() const {
        std::cout<<_fen.get()<< std::endl;
        _operations.print();
    }

    const Fen& getFen() const {return _fen;}
    const Operations& getOperations() const {return _operations;}
private:
    Fen _fen;
    Operations _operations;
};

class Epd{
    using epds_t = std::list<EpdRecord>;
public:
    using iterator = epds_t::iterator;
    using const_iterator = epds_t::const_iterator;

    Epd(const std::string& filename)
    {
        std::ifstream myfile (filename);
        std::string line;
        while (std::getline (myfile, line))
        {
		    std::istringstream is(line);
		    is >> std::skipws;
            _records.emplace_back(is);
        }
        //std::cout << "Parsed " << _records.size() << " records" << std::endl;
    }

    void print() const {
        for(const auto & r: _records) {
            r.print();
        }
    }

    iterator begin() { return _records.begin(); }
    iterator end() { return _records.end(); }
    const_iterator begin() const { return _records.begin(); }
    const_iterator end() const { return _records.end(); }
    const_iterator cbegin() const { return _records.cbegin(); }
    const_iterator cend() const { return _records.cend(); }
    unsigned int getSize() const { return _records.size(); };
private:
    epds_t _records;
};

class Movelist {
public:
    Movelist(const Position& pos, const std::string& mm) {
        std::istringstream is(mm);
        is >> std::skipws;
        std::string ms;
        while (is >> ms) {
            Move m;
            MovePicker mp(pos);
            bool found = false;
            while ((m = mp.getNextMove()))
            {
                if (ms == UciOutput::displayMove(pos, m))
                {
                    _moveList.push_back(m);
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout<<"MOVE NOT FOUND "<<ms<<std::endl;
            }
        }
    }

    bool contains(const Move& m) const{
        return std::find(_moveList.begin(), _moveList.end(), m) != _moveList.end();
    }

    std::string getstring() const {
        std::string s = "";
        for (const auto& m: _moveList) {
            s += UciOutput::displayUci(m, false) + " ";
        }
        return s;
    }
private:
    std::list<Move> _moveList;
};

class PointList {
public:
    struct Point {
        Move m;
        std::string MoveString;
        int value;
    };

    static bool containPointList(const EpdRecord& r) {
        return r.getOperations().contains("c7") && r.getOperations().contains("c8") && r.getOperations().contains("c9");
    }

    PointList(const Position& pos, const EpdRecord& r) {
        std::string c7 = r.getOperations().get("c7");
        std::string c8 = r.getOperations().get("c8");
        std::string c9 = r.getOperations().get("c9");
        c7 = c7.substr(1, c7.size() - 2);
        c8 = c8.substr(1, c8.size() - 2);
        c9 = c9.substr(1, c9.size() - 2);

        std::istringstream ic7(c7);
        ic7 >> std::skipws;

        std::istringstream ic8(c8);
        ic8 >> std::skipws;

        std::istringstream ic9(c9);
        ic9 >> std::skipws;

        std::string ms;
        std::string ms2;
        std::string mv;
        while (ic7 >> ms && ic8 >> mv && ic9 >> ms2) {
            Move m;
            MovePicker mp(pos);
            bool found = false;
            while ((m = mp.getNextMove()))
            {
                if (ms == UciOutput::displayMove(pos, m))
                {
                    _pointList.push_back(Point{m, ms2, std::stoi(mv)});
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout<<"MOVE NOT FOUND "<<ms<<std::endl;
                exit(0);
            }
        }
    }

    std::string getMoves() const {
        std::string s = "";
        for (const auto& p: _pointList) {
            s += p.MoveString + " ";
        }
        return s;
    }

    int getPoints(const Move m) const {
        for (const auto& p: _pointList) {
            if(p.MoveString == UciOutput::displayUci(m, false)) {
                if (p.m != m) {
                    std::cout<<"error in move"<<std::endl;
                }
                return p.value;
            }
        }
        return 0;
    }
private:
    std::list<Point> _pointList;
};

class Result {
public:
    Result(unsigned int total): _total(total){}
    void addResult(unsigned int x) {
        _partial += x;
    }

    std::string getStringResult() const {
        std::string s;
        s += std::to_string(getPercentual()) + "% (" + std::to_string(_partial) + "/" + std::to_string(_total) +  ")";
        return s;
    }

    unsigned int getTotal() const { return _total; }
    unsigned int getPartial() const { return _partial; }
    double getPercentual() const { return double(_partial) / _total * 100.0; }

private:
    unsigned int _total;
    unsigned int _partial = 0;

};

Result test(const std::string& testName, my_thread& thr, Position& pos, SearchLimits& sl) {
    Epd epd(testName);

    bool usePointSystem = PointList::containPointList(*epd.cbegin());

    Result results(epd.getSize() * (usePointSystem ? 10 : 1));
    for (const auto & record: epd) {
        
        if (PointList::containPointList(record) != usePointSystem) {
            std::cout<<"warning mixed point system"<<std::endl;
            exit(0);
        }

        pos.setupFromFen(record.getFen().get());
        auto r =  thr.synchronousSearch(pos, sl);

        if (usePointSystem) {
            PointList point(pos, record);
            auto p = point.getPoints(r.PV.getMove(0));
            results.addResult(p);
        }
        else if (Movelist(pos, record.getOperations().get("bm")).contains(r.PV.getMove(0))) {
            results.addResult(1);
        }
    }
    return results; 
}

void doTest(const std::string& testName, my_thread& thr, Position& pos, SearchLimits& sl) {
    const auto res = test(testName, thr, pos, sl);
    std::cout<<testName<<": "<<res.getPercentual()<<"% ("<<res.getPartial()<<"/"<<res.getTotal()<<")"<<std::endl;
}

int main(int argc, char ** argv) {
    unsigned int time = 100;
    if( argc == 2) {
        time = atoi(argv[1]);
    }
    
    std::cout<<"time for each position: "<<time<<" ms"<<std::endl;
	
    libChessInit();

    my_thread thr;
    thr.setMute(true);
	thr.getTT().setSize(64);

    Position pos(Position::pawnHash::off);
    pos.nnue().load("nnue.par");

    SearchLimits sl;
    sl.setMoveTime(time);

    doTest("STS1-STS15_LAN_v3.epd", thr, pos, sl);
    doTest("wacnew.epd", thr, pos, sl);
}
