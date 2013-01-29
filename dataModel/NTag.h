#pragma once
#include "IDataModel.h"

using namespace std;

class NTag : public IDataModel{

public:

	NTag(string url, map<string,float> value);
	NTag();

	void* getValue();
	bool storeSQL();
	bool loadSQL(int mediaId);


private:

	map<string,float> value;

};
/*
namespace Poco { namespace Data {

template <>
class TypeHandler<NTag>
{

public:

    static std::size_t size()
    {
        return 10; // Ten columns
    }

    static void bind(std::size_t pos, const NTag& obj, Poco::Data::AbstractBinder* pBinder, Poco::Data::AbstractBinder::Direction dir)
    {
        // the table is defined as Person (LastName VARCHAR(30), FirstName VARCHAR, Age INTEGER(3))
        // Note that we advance pos by the number of columns the datatype uses! For string/int this is one.
        poco_assert_dbg (pBinder != 0);
    	map<string,float> value = obj->value;
    	map<string,float>::iterator iter;
    	for (iter = value.begin(); iter != value.end(); iter++)
    	{
    		 TypeHandler<int>::bind(pos++, (const int &)iter->first, pBinder, dir);
    		 TypeHandler<int>::bind(pos++, (const int &)iter->first, pBinder, dir);
    	}
        TypeHandler<int>::bind(pos++, (const int &)obj.PK, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.GAME_ID, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.ROUND_ID, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.USER_ID, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.TIME_ID, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.ROUND_AUDIENCE, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.ROUND_EXPRESS_ID, pBinder, dir);
        TypeHandler<float>::bind(pos++, (const float &)obj.SCORE, pBinder, dir);
        TypeHandler<int>::bind(pos++, (const int &)obj.KSVM, pBinder, dir);
        TypeHandler<std::string>::bind(pos++, obj.USERNAME, pBinder, dir);
    }

    static void prepare(std::size_t pos, const bits::GameImage& obj,  Poco::Data::AbstractPreparator *pPrepare)
    {
        // the table is defined as Person (LastName VARCHAR(30), FirstName VARCHAR, Age INTEGER(3))
        poco_assert_dbg (pPrepare != 0);

                pPrepare->prepare(pos++, (int &)obj.PK);
        pPrepare->prepare(pos++, (int &)obj.GAME_ID);
        pPrepare->prepare(pos++, (int &)obj.ROUND_ID);
        pPrepare->prepare(pos++, (int &)obj.USER_ID);
        pPrepare->prepare(pos++, (int &)obj.TIME_ID);
        pPrepare->prepare(pos++, (int &)obj.ROUND_AUDIENCE);
        pPrepare->prepare(pos++, (int &)obj.ROUND_EXPRESS_ID);
        pPrepare->prepare(pos++, (float &)obj.SCORE);
        pPrepare->prepare(pos++, (int &)obj.KSVM);
        pPrepare->prepare(pos++, obj.USERNAME);
    }

    static void extract(std::size_t pos, bits::GameImage& obj, const bits::GameImage& defVal, Poco::Data::AbstractExtractor* pExt)
    {
        // defVal is the default person we should use if we encunter NULL entries, so we take the individual fields
        // as defaults. You can do more complex checking, ie return defVal if only one single entry of the fields is null etc...

                // Poco 1.4

};
}
}*/
