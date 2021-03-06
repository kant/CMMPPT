#if defined(_MSC_VER)
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "mtmDemanBomAppData.h"

  

void ESO2mtmDemanBomAppData::setAttachRate(int period, float attachRate )
{
  attachRateVec_[period] = attachRate;
}
void ESO2mtmDemanBomAppData::setAttachRate(std::vector<float> attachRateVec )
{
  attachRateVec_ = attachRateVec;
}
float ESO2mtmDemanBomAppData::getAttachRate(int period ) const
{
  return attachRateVec_[period];
}
std::vector<float> ESO2mtmDemanBomAppData::getAttachRate() const
{
  return attachRateVec_;
}

// default constructor
ESO2mtmDemanBomAppData::ESO2mtmDemanBomAppData()
:
ESO2appData(),attachRateVec_()
{}

ESO2mtmDemanBomAppData::ESO2mtmDemanBomAppData(int nPeriods)
:
ESO2appData(),attachRateVec_()
{
  setVector(nPeriods,0.f,attachRateVec_);
}

// destructor

ESO2mtmDemanBomAppData::~ESO2mtmDemanBomAppData()
{}

// copy constructor
ESO2mtmDemanBomAppData::ESO2mtmDemanBomAppData( const ESO2mtmDemanBomAppData& source ):
ESO2appData(source),
attachRateVec_(source.attachRateVec_)
{};

// assignment operator
ESO2mtmDemanBomAppData&
ESO2mtmDemanBomAppData::operator=(const ESO2mtmDemanBomAppData& rhs)
{  
  if (this != &rhs) {        
    ESO2appData::operator=(rhs);
    attachRateVec_ = rhs.attachRateVec_;
  }
  return *this;
}

ESO2appData * ESO2mtmDemanBomAppData::clone() const
{ return (new ESO2mtmDemanBomAppData(*this));}

// Self-test
void ESO2mtmDemanBomAppData::test()
{

  ESO2mtmDemanBomAppData ad1;
  std::vector<float> v;

  {
  ESO2mtmDemanBomAppData ad0(3);
  assert(ad0.getAttachRate(0)==0.f);
  assert(ad0.getAttachRate(1)==0.f);
  assert(ad0.getAttachRate(2)==0.f);
  ad0.setAttachRate(1,2.f);
  v = ad0.getAttachRate();
  assert(v[0]==0.f);
  assert(v[1]==2.f);
  assert(v[2]==0.f);
  assert(ad0.getAttachRate(0)==0.f);
  assert(ad0.getAttachRate(1)==2.f);
  assert(ad0.getAttachRate(2)==0.f);


  ad1 = ad0;

  ESO2mtmDemanBomAppData ad2(ad0);
  v = ad2.getAttachRate();
  assert(v[0]==0.f);
  assert(v[1]==2.f);
  assert(v[2]==0.f);
  }

  v = ad1.getAttachRate();
  assert(v[0]==0.f);
  assert(v[1]==2.f);
  assert(v[2]==0.f);


  
}
