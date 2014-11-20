#include "ShotDetector.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]){

    try {
        po::options_description generic("Command line options");
        generic.add_options()
            ("help,h", "produce help message")
            ("step,s", po::value<int>()->default_value(5), "set frame analysis step")
            ("threshold,t", po::value<double>()->default_value(3), "set peak detection threshold multiplier")
            ("min_duration,m", po::value<int>()->default_value(60), "set minimum scene frame count")
            ("save-all,a", "save begin, middle and end scene frames (by default, it only saves middle)")
            ("benchmark,b", "do not save or output. For benchmarking only")
        ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("filename", po::value<string>(), "set video filename")
        ;

        po::positional_options_description p;
        p.add("filename", -1);

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);


        if (vm.count("help")) {
            cout << visible << endl;
            return 1;
        }

        if (!vm.count("filename")) {
            cout << "Filename was not set" << endl;
            return 1;
        }

        string filename = vm["filename"].as<string>();
        int step = vm["step"].as<int>();
        double threshold = vm["threshold"].as<double>();
        int min_duration = vm["min_duration"].as<int>();
        bool saveFrames = (1 - vm.count("benchmark")) > 0;
        bool saveOutput = (1 - vm.count("benchmark")) > 0;
        bool saveMiddleOnly = (vm.count("save-all") == 0);

        ShotDetector::processOneVideo(filename,step,threshold,min_duration,saveFrames,saveOutput,saveMiddleOnly);

    }
    catch(exception& e) {
        cerr << "error: " << e.what() << endl;
        return 1;
    } catch(...) {
        cerr << "Unknown exception!" << endl;
    }

    return 0;

}
