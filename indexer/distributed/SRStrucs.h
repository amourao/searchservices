#pragma once
#include <cstring>
#include <chrono>
#include <armadillo>


#define MEASURE_TIME
typedef std::chrono::high_resolution_clock::time_point tp;
typedef uint uindex;
inline tp NOW() {return std::chrono::high_resolution_clock::now();}
inline long ELAPSED(tp start) {return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-start).count();}
inline long TO_LONG(tp start) {return std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch()).count();}


inline float myNorm(arma::fmat matrix1){
    return arma::norm<arma::Mat<float>>(matrix1,2);
}

template <typename T>
inline float myNorm(arma::Mat<T> matrix1){
    return arma::norm<arma::Mat<float>>(arma::conv_to<arma::Mat<float>>::from(matrix1),2);
}

template <typename T>
inline float myNorm(arma::Mat<T>& matrix1, arma::Mat<T>& matrix2){
    return myNorm(arma::conv_to<arma::Mat<float>>::from(matrix1)-arma::conv_to<arma::Mat<float>>::from(matrix2));
}

template <typename T>
inline float myNorm2(arma::Mat<T>& matrix){
    return sqrt(arma::dot(matrix,matrix));
}

template <typename T>
inline void normalizeColumns(arma::Mat<T>& matrix) {
    for (uint i = 0; i < matrix.n_cols; ++i) {
        arma::Mat<T> col = matrix.col(i);
        matrix.col(i) /= myNorm(col);
    }
}

struct Coefficient {
    uindex vector_pos;
    float value;

    Coefficient():
        vector_pos(0),
        value(0)
    {}

    Coefficient(uindex vp, float v):
        vector_pos(vp),
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

struct QueryStructReq {
    std::vector<float> query;
    std::vector<int> buckets;
    std::vector<float> coeffs;
    std::vector<uindex> indexes;
    std::vector<float> parameters;

    char operation;
    uint totalByteSize;

    uint computeTotalByteSize(){
        uint qBufferLen = query.size()*sizeof(float);
        uint bBufferLen = buckets.size()*sizeof(int);
        uint cBufferLen = coeffs.size()*sizeof(float);
        uint iBufferLen = indexes.size()*sizeof(uindex);
        uint pBufferLen = parameters.size()*sizeof(float);

        return qBufferLen + bBufferLen + cBufferLen + iBufferLen + pBufferLen + sizeof(uint)*5 + 1/*operationType*/ + sizeof(uint) /*totalByteSize*/;
    }

    char* toBytes(){
        char* qBuffer = reinterpret_cast<char*>(&query[0]);
        char* bBuffer = reinterpret_cast<char*>(&buckets[0]);
        char* cBuffer = reinterpret_cast<char*>(&coeffs[0]);
        char* iBuffer = reinterpret_cast<char*>(&indexes[0]);
        char* pBuffer = reinterpret_cast<char*>(&parameters[0]);

        uint qBufferLen = query.size()*sizeof(float);
        uint bBufferLen = buckets.size()*sizeof(int);
        uint cBufferLen = coeffs.size()*sizeof(float);
        uint iBufferLen = indexes.size()*sizeof(uindex);
        uint pBufferLen = parameters.size()*sizeof(float);

        totalByteSize = qBufferLen + bBufferLen + cBufferLen + iBufferLen + pBufferLen + sizeof(uint)*5 + 1/*operationType*/ + sizeof(uint) /*totalByteSize*/;

        char* outBuffer = new char[totalByteSize];

        memcpy(&outBuffer[0],&totalByteSize,sizeof(uint));

        outBuffer[sizeof(uint)] = operation; //operation type (query)

        memcpy(&outBuffer[sizeof(uint)+1],&qBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+sizeof(uint)],qBuffer,qBufferLen);

        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+sizeof(uint)],&bBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+sizeof(uint)*2],bBuffer,bBufferLen);

        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+sizeof(uint)*2],&cBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+sizeof(uint)*3],cBuffer,cBufferLen);

        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+cBufferLen+sizeof(uint)*3],&iBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+cBufferLen+sizeof(uint)*4],iBuffer,iBufferLen);

        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+cBufferLen+iBufferLen+sizeof(uint)*4],&pBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+cBufferLen+iBufferLen+sizeof(uint)*5],pBuffer,pBufferLen);

        return outBuffer;
    }

    void toQueryStructReq(char* c){
        totalByteSize = *reinterpret_cast<uint*>(&c[0]);

        operation = c[sizeof(uint)];

        uint qBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1]);
        uint bBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen]);
        uint cBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen]);
        uint iBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)+cBufferLen]);
        uint pBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)+cBufferLen+sizeof(uint)+iBufferLen]);
        //totalByteSize = qBufferLen + bBufferLen + cBufferLen + pBufferLen + sizeof(uint)*4 + 1/*operationType*/;

        float* qBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)]);
        int* bBuffer = reinterpret_cast<int*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)]);
        float* cBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)]);
        uindex* iBuffer = reinterpret_cast<uindex*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)+cBufferLen+sizeof(uint)]);
        float* pBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)+cBufferLen+sizeof(uint)+iBufferLen+sizeof(uint)]);

        query.insert(query.end(), qBuffer, qBuffer+(uint)qBufferLen/sizeof(float));
        buckets.insert(buckets.end(), bBuffer, bBuffer+(uint)bBufferLen/sizeof(uint));
        coeffs.insert(coeffs.end(), cBuffer, cBuffer+(uint)cBufferLen/sizeof(float));
        indexes.insert(indexes.end(), iBuffer, iBuffer+(uint)iBufferLen/sizeof(uindex));
        parameters.insert(parameters.end(), pBuffer, pBuffer+(uint)pBufferLen/sizeof(float));
    }
};

struct QueryStructRsp {
    std::vector<uindex> indexes;
    std::vector<float> dists;
    std::vector<float> parameters;
    char operation;

    uint totalByteSize;

    uint computeTotalByteSize(){
        uint iBufferLen = indexes.size()*sizeof(uindex);
        uint dBufferLen = dists.size()*sizeof(float);
        uint pBufferLen = parameters.size()*sizeof(float);

        return iBufferLen + dBufferLen+ pBufferLen + sizeof(uint)*3 + 1/*operation type*/ + sizeof(uint) /*totalByteSize*/;
    }

    char* toBytes(){
        char* iBuffer = reinterpret_cast<char*>(&indexes[0]);
        char* dBuffer = reinterpret_cast<char*>(&dists[0]);
        char* pBuffer = reinterpret_cast<char*>(&parameters[0]);

        uint iBufferLen = indexes.size()*sizeof(uint);
        uint dBufferLen = dists.size()*sizeof(float);
        uint pBufferLen = parameters.size()*sizeof(float);

        totalByteSize = iBufferLen + dBufferLen+ pBufferLen + sizeof(uint)*3 + 1/*operation type*/ + sizeof(uint) /*totalByteSize*/;


        char* outBuffer = new char[totalByteSize];

        memcpy(&outBuffer[0],&totalByteSize,sizeof(uint));
        outBuffer[sizeof(uint)] = operation; //operation type (query)

        memcpy(&outBuffer[sizeof(uint)+1],&iBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+iBufferLen+sizeof(uint)],&dBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+iBufferLen+sizeof(uint)+dBufferLen+sizeof(uint)],&pBufferLen,sizeof(uint));

        memcpy(&outBuffer[sizeof(uint)+1+sizeof(uint)],iBuffer,iBufferLen);
        memcpy(&outBuffer[sizeof(uint)+1+iBufferLen+sizeof(uint)*2],dBuffer,dBufferLen);
        memcpy(&outBuffer[sizeof(uint)+1+iBufferLen+dBufferLen+sizeof(uint)*3],pBuffer,pBufferLen);

        return outBuffer;
    }

    void toQueryStructRsp(char* c){
        totalByteSize = *reinterpret_cast<uint*>(&c[0]);

        operation = c[sizeof(uint)];

        uint iBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1]);
        uint dBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1+sizeof(uint)+iBufferLen]);
        uint pBufferLen = *reinterpret_cast<uint*>(&c[sizeof(uint)+1+sizeof(uint)+iBufferLen+sizeof(uint)+dBufferLen]);

        uint* iBuffer = reinterpret_cast<uindex*>(&c[sizeof(uint)+1+sizeof(uint)]);
        float* dBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+iBufferLen+sizeof(uint)]);
        float* pBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+iBufferLen+sizeof(uint)+dBufferLen+sizeof(uint)]);

        indexes.insert(indexes.end(), iBuffer, iBuffer+(uint)iBufferLen/sizeof(uindex));
        dists.insert(dists.end(), dBuffer, dBuffer+(uint)dBufferLen/sizeof(float));
        parameters.insert(parameters.end(), pBuffer, pBuffer+(uint)pBufferLen/sizeof(float));
    }
};
