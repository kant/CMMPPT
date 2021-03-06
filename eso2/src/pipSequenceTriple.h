#ifndef ESO2pipSequenceTriple_h
#define ESO2pipSequenceTriple_h

#include <string>

class ESO2pipSequenceTriple   {
public:
  
  void setDemandName(const std::string & dname );
  void setShipPeriod(int period );
  void setIncShipVol(float incShipVol);
  void setValue(float v);

  std::string getDemandName()const;
  int getShipPeriod()const;
  float getIncShipVol()const;
  float getValue()const;
    
  // default constructor
  ESO2pipSequenceTriple();

  // Typical Constructor
  ESO2pipSequenceTriple(const std::string & dname,int period,float incShipVol,float value);
  
  // destructor
  virtual ~ESO2pipSequenceTriple();
  
  // copy constructor
  ESO2pipSequenceTriple( const ESO2pipSequenceTriple& source );
  
  // assignment operator
  ESO2pipSequenceTriple&
    operator=(const ESO2pipSequenceTriple& rhs);


  // Self-test
  static void test();

private:

  std::string demandName_;
  int shipPeriod_;
  float incShipVol_;
  float value_;
  
};

// ---------
// function operator for sorting Pip Sequence Triple
// ---------
struct ESO2sortPipSequenceTriple
{
  bool operator()( 
    const ESO2pipSequenceTriple & p1, 
    const ESO2pipSequenceTriple & p2) 
    const;
};


#endif
