#ifndef scoFile_h
#define scoFile_h

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
//#include <rw/cstring.h>
//#include <rw/hashdict.h>
//#include "generVc.h"



#if 0
#if defined(_MSC_VER)
#  if defined(SCO_DLLEXPORT)
#    define SCO_DECLSPEC __declspec(dllexport)
#  else
#    define SCO_DECLSPEC  __declspec(dllimport)
#  endif
#  define SCO_STDCALL __stdcall
#else
#  define SCO_DECLSPEC
#  define SCO_STDCALL
#endif
#else
#  define SCO_DECLSPEC
#  define SCO_STDCALL
#endif

// Macros for defining method implementatons
// getField
#define getField(className,field) \
std::string \
  className::##field ( unsigned int recNum ) const \
{ \
  return fieldAsString( recNum, #field ); \
} \
std::string \
  className::##field ( std::string key ) const \
{ \
  return field( recordIndex( key ) ); \
}
#define getFieldH(field) \
std::string field( unsigned int recNum ) const { return fieldAsString( recNum, #field ); }\
std::string field( const std::string & key ) const { return field( recordIndex( key ) ); }

// getFieldFileWithIterator
#define getFieldFileWithIterator(className,field) \
getField(className,field) \
std::string \
  className##Iterator::##field () const \
{ \
  return fieldOnRecord( #field ); \
}
#define getFieldFileWithIteratorH(field) \
std::string field () const { return fieldOnRecord( #field ); }

// getFloatField
#define getFloatField(className,field) \
getField(className,field) \
float \
  className::##field##AsFloat ( unsigned int recNum ) const \
{ \
  return fieldAsFloat( recNum, #field ); \
} \
float \
  className::##field##AsFloat ( std::string key ) const \
{ \
  return field##AsFloat( recordIndex( key ) ); \
}
#define getFloatFieldH(field) \
getFieldH(field) \
float field##AsFloat(unsigned int recNum) const { return fieldAsFloat( recNum, #field ); } \
float field##AsFloat( const std::string & key ) const { return field##AsFloat( recordIndex( key ) ); }

// getFloatFieldFileWithIterator
#define getFloatFieldFileWithIterator(className,field) \
getFloatField(className,field) \
std::string \
  className##Iterator::##field () const \
{ \
  return fieldOnRecord( #field ); \
}\
float \
  className##Iterator::##field##AsFloat () const \
{ \
  return fieldOnRecordAsFloat( #field ); \
}
#define getFloatFieldFileWithIteratorH(field) \
std::string field () const { return fieldOnRecord( #field ); }; \
float field##AsFloat () const { return fieldOnRecordAsFloat( #field ); }

// getIntField
#define getIntField(className,field) \
getField(className,field) \
int \
  className::##field##AsInt ( unsigned int recNum ) const \
{ \
  return fieldAsInt( recNum, #field ); \
} \
int \
  className::##field##AsInt ( std::string key ) const \
{ \
  return field##AsInt( recordIndex( key ) ); \
}
#define getIntFieldH(field) \
getFieldH(field) \
int field##AsInt(unsigned int recNum) const { return fieldAsInt( recNum, #field ); } \
int field##AsInt(const std::string & key ) const { return field##AsInt( recordIndex( key ) ); }


// getIntFieldFileWithIterator
#define getIntFieldFileWithIterator(className,field) \
getIntField(className,field) \
std::string \
  className##Iterator::##field () const \
{ \
  return fieldOnRecord( #field ); \
}\
int \
  className##Iterator::##field##AsInt () const \
{ \
  return fieldOnRecordAsInt( #field ); \
}
#define getIntFieldFileWithIteratorH(field) \
std::string field() const { return fieldOnRecord( #field ); } \
int field##AsInt() const { return fieldOnRecordAsInt( #field ); }

// Forward declarations
class SCO_DECLSPEC SCOfile;

class SCO_DECLSPEC SCOfileIterator {

public:
  
  //Advances the iterator to the next record a pointer to 
  //the record. Returns nil if the cursor is at the end of the 
  //collection. 
  virtual const std::string * operator()(); 

  // Return current record number
  unsigned int recNum() const;

   
  // Return refernce to file being iterated on
  const SCOfile & scoFile() const;

  // Return current record iterator is pointing to.
  std::string record() const;

    
  std::string
    fieldOnRecord ( std::string fieldName )
      const;

  float 
    fieldOnRecordAsFloat ( 
      std::string fieldName ) 
      const;

  int
    fieldOnRecordAsInt (  
    std::string fieldName  )
    const;
  
  // preferred constructor
  // Construct an iterator for a SCOfile to allow sequential file access.
  // Immediately after construction, the position of the 
  // iterator is undefined until positioned.
  SCOfileIterator(const SCOfile&);
  
  // destructor
  virtual
    ~SCOfileIterator();
  
  // Self-test
  //static void test();

private:

  unsigned int recNumZeroBased() const;

  virtual void openFile();
  
  // default constructor -- don't use!
  SCOfileIterator();
  
  // copy constructor  -- don't use!
  SCOfileIterator( const SCOfileIterator& source );
  
  // assignment operator -- don't use!
  SCOfileIterator&
  operator=(const SCOfileIterator& rhs);

  // Member data
  FILE * filePtr_;
  const SCOfile * scoFilePtr_;
  unsigned int recNum_;
  std::string * recordPtr_;
  
};

//---------------------------------------------------------------

class SCOfile {

  friend class SCOfileIterator;

public:
  
  // return number of records in file.
  // If removeDuplicateRecords is TRUE then then number
  // of unique records is returned.
  virtual unsigned int numRecords() const;

  std::string filename() const;

  // removeDuplicateRecords flag.
  // If true then only one copy of a record will be returned when
  // accessing the file by record number. The duplicates are
  // skipped over.
  // This field must be set when to fileDataInitialized() is false.
  // If this condition is not met, then an error condition is raised.
  bool removeDuplicateRecords() const;
  void removeDuplicateRecords(bool);

  // commentRecords flag.
  // If true then records which are comments are skipped over as if they were not present.
  // This field must be set when to fileDataInitialized() is false.
  // If this condition is not met, then an error condition is raised.
  bool commentRecords() const;
  void commentRecords(bool);



  void initialize();
 
  virtual std::string
    fieldAsString ( unsigned int recNum, std::string fieldName )
      const;
  virtual std::string
    fieldAsString ( std::string key, std::string fieldName )
      const;  
  virtual int
    fieldAsInt ( unsigned int recNum, std::string fieldName )
      const;
  virtual int
    fieldAsInt ( std::string key, std::string fieldName )
      const;
  virtual float
    fieldAsFloat ( unsigned int recNum, std::string fieldName )
      const;
  virtual float
    fieldAsFloat ( std::string key, std::string fieldName )
      const;
  
  // Return true if record with specified key exists
  bool
    keyExists( const std::string & key )
    const;
  
  virtual void initializeFileData();

protected:
  virtual void initializeFileDict();
  virtual void initializeFieldInformation() = 0;

  
  virtual bool hasHeaderRecord() const {return false;};

  // stripBlanks flag.
  bool strip() const;
  void strip(bool);

  // test to see if record is a comment
  bool commentRecord(std::string rec,int physicalRecordNumber ) const;

  // Implement roguewave string strip.
  // needed when converting to stl
  std::string stripString( const std::string & orig ) const;

  // attempt to deal with dos to unix and unix to dos eol issues
  static std::string trimEolCharacters(const std::string & inputRecord);

  // return spefific record in file
  std::string record( unsigned int recNo ) const;
  std::string record( std::string key ) const;

  // Return index of record with specfied key
  int recordIndex ( std::string key ) const;

  // return named field on specified record  
  virtual std::string
    fieldOnRecord ( const std::string & record, std::string fieldName )
      const=0;
  
  virtual float
    fieldToFloat( 
      std::string fieldValue,  
      unsigned int recNum,
      std::string fieldName )
      const;
  float 
    fieldOnRecordAsFloat (
      std::string record,
      unsigned int recNum,  
      std::string fieldName ) 
      const;
  
 
  virtual int
    fieldToInt( 
      std::string fieldValue, 
      unsigned int recNum,
      std::string fieldName )
      const; 
  int
    fieldOnRecordAsInt (
      std::string record,
      unsigned int recNum,  
      std::string fieldName  )
      const;
  // compute key for storing record in dictionary
  virtual
    std::string 
      key( unsigned int ) const = 0;

  // resize vector that contains records
  void resize(size_t newSize);

  bool fileDataInitialized() const;
  void fileDataInitialized(bool val){fileDataInitialized_=val;};
  bool fileDictInitialized() const;

  int recordNumberWithoutComments(int recNum) const;

  
  virtual void testRecordFormat(std::string rec, unsigned int recNum ) const =0;

  // preferred constructor
  SCOfile(std::string filename);
  
  // destructor
  virtual
    ~SCOfile();
  
  // Self-test
  //static void test();

  
private:
  
  
  // default constructor -- don't use!
  SCOfile();
  
  // copy constructor  -- don't use!
  SCOfile( const SCOfile& source );
  
  // assignment operator -- don't use!
  SCOfile&
  operator=(const SCOfile& rhs);

  // Member data
  bool removeDuplicateRecords_;
  bool commentRecords_;
  std::vector<std::string> fileData_;
  bool fileDataInitialized_;
  std::map<std::string,int> fileDict_; // record number skipping comments
  std::map<int,int> recNumWc_; // record number with comments
  bool fileDictInitialized_;
  std::string filename_; 
  bool strip_;
  
};

//---------------------------------------------------------------




class SCO_DECLSPEC SCOfixedFormatFile : public SCOfile {
public:

  // return record length
  // since these are files with fixed length fields all records are
  // the same length.
  int recLength() const;

  // return named field on specified record  
  virtual std::string
    fieldOnRecord ( std::string record, std::string fieldName )
      const;
  
protected:
  
  virtual void initializeFieldInformation();
  virtual void testRecordFormat(std::string rec, unsigned int recNum) const;

  // insert field name and length into file description
  void
    insertField( std::string fieldName, int fieldLen );

  // Given a field name return its lenght on a record
  size_t
    fieldLen( std::string fieldNam )
      const;

  // Given a field name return its starting position on a record
  size_t
    fieldPos( std::string fieldNam )
      const;

  // preferred constructor
  SCOfixedFormatFile(std::string filename);
  
  // destructor
  virtual
    ~SCOfixedFormatFile();
  
  // Self-test
  //static void test();

  
private:
  
  
  // default constructor -- don't use!
  SCOfixedFormatFile();
  
  // copy constructor  -- don't use!
  SCOfixedFormatFile( const SCOfixedFormatFile& source );
  
  // assignment operator -- don't use!
  SCOfixedFormatFile&
  operator=(const SCOfixedFormatFile& rhs);

  // Member data
  std::vector<std::string> fields_;
  std::vector<int>    fieldLens_; 

  std::map<std::string,int> fieldLenDict_;
  std::map<std::string,int> fieldPosDict_;
  
};


class SCO_DECLSPEC SCOcsvFormatFile : public SCOfile {
  
public:

  // return number of fields on a record
  int numberOfFields() const;

  // return named field on specified record  
  virtual std::string
    fieldOnRecord ( const std::string & record, std::string fieldName )
      const;
  std::string 
    fieldOnRecord( const std::string & record, int desiredFieldNumber )
    const;
  
protected:
  
  virtual void initializeFieldInformation();
  virtual void testRecordFormat(std::string rec, unsigned int recNum) const;

  // insert field name and length into file description
  void
    insertField( std::string fieldName );

  // return true if field with specified name exists
  //bool fieldExists(std::string & fileName) const;

  
  // Given a field name return its field column number
  size_t
    fieldPos( const std::string & fieldNam )
      const;

  void fieldSeperator( char );
  char fieldSeperator() const;

  void hasHeaderRecord( int numberOfFields );
  virtual bool hasHeaderRecord() const;

  // preferred constructor
  SCOcsvFormatFile(std::string filename);
  
  // destructor
  virtual
    ~SCOcsvFormatFile();
  
  // Self-test
  //static void test();

  
private:
  
  
  // default constructor -- don't use!
  SCOcsvFormatFile();
  
  // copy constructor  -- don't use!
  SCOcsvFormatFile( const SCOcsvFormatFile& source );
  
  // assignment operator -- don't use!
  SCOcsvFormatFile&
  operator=(const SCOcsvFormatFile& rhs);

  // Member data
  std::vector<std::string> fields_;

  std::map<std::string,int> fieldPosDict_;

  char fieldSeperator_;
  bool hasHeaderRecord_;
  
};

#endif





