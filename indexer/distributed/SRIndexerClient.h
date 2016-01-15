#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>

#include "IIndexer.h"
#include "FactoryIndexer.h"

#include "../commons/FileDownloader.h"
#include "../analyser/tools/MatrixTools.h"

using namespace std;

class SRIndexerClient {

public:

    SRIndexerClient();
    SRIndexerClient(string& type, map<string,string>& params);
    ~SRIndexerClient();

    void index(arma::fmat& features,vector<float>& coeffs, vector<unsigned long int>& indexes);

    std::pair<vector<unsigned long>,vector<float> > knnSearchIdLong(arma::fmat& name, vector<int> buckets, int n, double search_limit);

    bool save(string basePath);
    bool load(string basePath);

private:

    struct Coefficient {
        long vector_pos;
        long original_id;
        float value;

        Coefficient(long vp,long id, float v):
            vector_pos(vp),
            original_id(id),
            value(v)
        {}

        bool operator<(const Coefficient& other) const
        {
            return value < other.value;
        }

        bool operator==(const Coefficient& other) const
        {
            return vector_pos == other.vector_pos;
        }
    };

    std::vector<std::vector<Coefficient>> indexData;
    arma::fmat data;
};
