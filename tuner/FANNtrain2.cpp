#include <iostream>
#include <sstream>
#include <fstream>

#include "position.h"
#include "nnue.h"
#include "model.h"
#include "featureList.h"

int main()
{
    Model model;
    model.init();
    FeatureList fl;
    std::string line;

    while(true) {
        std::ifstream myfile ("fen.data");
        unsigned long long counter = 0;
        double mse = 0;
        if (myfile.is_open())
        {
            while(getline(myfile,line) )
            {
                fl.clear();
                double dval;
                auto sep = line.find_first_of(';');
                if(sep != std::string::npos) {
                    auto feats = line.substr(0, sep);
                    std::istringstream  ss(feats);
                    std::string str;
                    while (getline(ss, str, ' ')) {
                        fl.add(std::stoi(str));
                        //std::cout <<"feat " <<std::stoi(str)<<std::endl;
                    }
                    auto val = line.substr(sep+1);
                    dval = std::stod(val);

                    auto out = model.forwardPass(fl);
                    mse += (out - dval) * (out - dval);
                    ++counter;
                    //std::cout <<dval<<std::endl;
                }





            }
        }
        std::cout<<"RMS "<< mse/counter<<std::endl;
    }
    return 0;
}
