#pragma once
#include <cstring>

struct Coefficient {
    unsigned long vector_pos;
    unsigned long original_id;
    float value;

    Coefficient(unsigned long vp, unsigned long id, float v):
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

struct QueryStructReq {
    std::vector<float> query;
    std::vector<int> buckets;
    std::vector<float> coeffs;
    std::vector<float> parameters;
    std::vector<unsigned long> indexes;

    char operation;
    uint totalByteSize;

    uint computeTotalByteSize(){
        uint qBufferLen = query.size()*sizeof(float);
        uint bBufferLen = buckets.size()*sizeof(int);
        uint cBufferLen = coeffs.size()*sizeof(float);
        uint iBufferLen = indexes.size()*sizeof(unsigned long);
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
        uint iBufferLen = indexes.size()*sizeof(unsigned long);
        uint pBufferLen = parameters.size()*sizeof(float);

        totalByteSize = qBufferLen + bBufferLen + cBufferLen + iBufferLen + pBufferLen + sizeof(uint)*5 + 1/*operationType*/ + sizeof(uint) /*totalByteSize*/;

        char* outBuffer = new char[totalByteSize];

        memcpy(&outBuffer[0],&totalByteSize,sizeof(uint));

        outBuffer[sizeof(uint)] = operation; //operation type (query)

        memcpy(&outBuffer[sizeof(uint)+1],&qBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+sizeof(uint)],&bBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+sizeof(uint)*2],&cBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+cBufferLen+sizeof(uint)*3],&iBufferLen,sizeof(uint));
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+cBufferLen+iBufferLen+sizeof(uint)*4],&pBufferLen,sizeof(uint));

        memcpy(&outBuffer[sizeof(uint)+1+sizeof(uint)],qBuffer,qBufferLen);
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+sizeof(uint)*2],bBuffer,bBufferLen);
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+sizeof(uint)*3],cBuffer,cBufferLen);
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+pBufferLen+sizeof(uint)*4],iBuffer,iBufferLen);
        memcpy(&outBuffer[sizeof(uint)+1+qBufferLen+bBufferLen+pBufferLen+iBufferLen+sizeof(uint)*5],pBuffer,pBufferLen);

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
        unsigned long* iBuffer = reinterpret_cast<unsigned long*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)+cBufferLen+sizeof(uint)]);
        float* pBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+qBufferLen+sizeof(uint)+bBufferLen+sizeof(uint)+cBufferLen+sizeof(uint)+iBufferLen+sizeof(uint)]);

        query.insert(query.end(), qBuffer, qBuffer+(uint)qBufferLen/sizeof(float));
        buckets.insert(buckets.end(), bBuffer, bBuffer+(uint)bBufferLen/sizeof(uint));
        coeffs.insert(coeffs.end(), cBuffer, cBuffer+(uint)cBufferLen/sizeof(float));
        indexes.insert(indexes.end(), iBuffer, iBuffer+(uint)iBufferLen/sizeof(unsigned long));
        parameters.insert(parameters.end(), pBuffer, pBuffer+(uint)pBufferLen/sizeof(float));
    }
};

struct QueryStructRsp {
    std::vector<unsigned long> indexes;
    std::vector<float> dists;
    std::vector<float> parameters;
    char operation;

    uint totalByteSize;

    uint computeTotalByteSize(){
        uint iBufferLen = indexes.size()*sizeof(unsigned long);
        uint dBufferLen = dists.size()*sizeof(float);
        uint pBufferLen = parameters.size()*sizeof(float);

        return iBufferLen + dBufferLen+ pBufferLen + sizeof(uint)*3 + 1/*operation type*/ + sizeof(uint) /*totalByteSize*/;
    }

    char* toBytes(){
        char* iBuffer = reinterpret_cast<char*>(&indexes[0]);
        char* dBuffer = reinterpret_cast<char*>(&dists[0]);
        char* pBuffer = reinterpret_cast<char*>(&parameters[0]);

        uint iBufferLen = indexes.size()*sizeof(unsigned long);
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

        unsigned long* iBuffer = reinterpret_cast<unsigned long*>(&c[sizeof(uint)+1+sizeof(uint)]);
        float* dBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+iBufferLen+sizeof(uint)]);
        float* pBuffer = reinterpret_cast<float*>(&c[sizeof(uint)+1+sizeof(uint)+iBufferLen+sizeof(uint)+dBufferLen+sizeof(uint)]);

        indexes.insert(indexes.end(), iBuffer, iBuffer+(uint)iBufferLen/sizeof(unsigned long));
        dists.insert(dists.end(), dBuffer, dBuffer+(uint)dBufferLen/sizeof(float));
        parameters.insert(parameters.end(), pBuffer, pBuffer+(uint)pBufferLen/sizeof(float));
    }
};