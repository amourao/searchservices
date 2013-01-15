#pragma once
#include "IDataModel.h"

using namespace std;

class NRoi : public IDataModel{

public:

	NRoi(string SQLTable, map<string,region>* value);
	~NRoi();

	bool storeSQL();
	bool loadSQL(int mediaId);


private:

	 map<string,region>* value;
	string SQLTable;
	size_t rawSize;

};
/*
namespace Poco {
namespace Data {

template <>
class TypeHandler<class NTag>
{
public:
    static std::size_t size()
    {
        return 3; // we handle three columns of the Table!
    }

   static void bind(std::size_t pos, const NTag& obj, AbstractBinder* pBinder, AbstractBinder::Direction dir)
    {
        poco_assert_dbg (pBinder != 0);
        // the table is defined as Person (FirstName VARCHAR(30), lastName VARCHAR, SocialSecNr INTEGER(3))
        // Note that we advance pos by the number of columns the datatype uses! For string/int this is one.
        pos++;
        TypeHandler<map<string,float> >::bind(pos++, *obj.value, pBinder, dir);
    }

    static void prepare(std::size_t pos, const NTag& obj, AbstractPreparator* pPrepare)
    {
        poco_assert_dbg (pBinder != 0);
        // the table is defined as Person (FirstName VARCHAR(30), lastName VARCHAR, SocialSecNr INTEGER(3))
        // Note that we advance pos by the number of columns the datatype uses! For string/int this is one.
        TypeHandler<map<string,float> >::prepare(pos++, *obj.value, pPrepare);
    }

    static void extract(std::size_t pos, NTag& obj, const  NTag& defVal, AbstractExtractor* pExt)
        /// obj will contain the result, defVal contains values we should use when one column is NULL
    {
    	map<string,float> value;
    	TypeHandler<map<string,float> >::extract(pos++, value, *defVal.value, pExt);
    	obj.value = value;
    }

};
}
}*/
