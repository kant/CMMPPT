
#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif

// RW2STL -- inserted:
#include <map>
#include <scenario/src/RWToSTLHelpers.h>
#include <string>
// End of RW2STL -- inserted includes.
#include <iostream>
#include <float.h>

#include <scenario/src/schedLF.h>
#include <scenario/src/demdSchd.h>
#include <scenario/src/demdNmFS.h>
#include <scenario/src/machdep.h>
#include <scenario/src/hashUtil.h>


// Create and initialize emptySchedule
const LgFrScheduleFloat  LgFrScheduleFloat::emptySched;
const LgFrFilterStrategy LgFrScheduleFloat::defaultFilterStrategy;


// Format a schedule for display.
// Returns a string representing the schedule.
std::string 
LgFrScheduleFloat::format(
   const LgFrScheduleFloat & highLight,
   const bool truncate, 
   const char highLightFlag,
   const int width,
   const int precision )//,
   //const long iosFlags )
const
{
   std::string retVal;
   LgFrTimeVecFloat hlVec;

   // Loop once for each itemTimeVec pair in schedule
   size_t i = 0; // Pulled out of the for below by RW2STL
   for ( i=0; i<schedule_.size(); i++ ) {

      // Check to see if highlighting information exits for i'th entry
      if ( highLight.contains( (*this)[i].item() ) ) {
         // Highlighting information does exist, so get it
         hlVec =  highLight[ (*this)[i].item() ].timeVecFloat();
      }
      else {
         // Highlighting information does not exist
         hlVec =  LgFrTimeVecFloat();
      }
 
      // Format i'th itemTimeVecPairFloat
      retVal = retVal + 
               (*this)[i].format( 
                  hlVec,
                  truncate,
                  highLightFlag,
                  width,
                  precision 
                  //iosFlags
               );  
 
      // Add new line character at end of itemTimeVec pair
      retVal = retVal + "\n";
   }

   return retVal;
}

  
// Is item in the schedule?
bool
LgFrScheduleFloat::contains (const LgFrItem& item)
	const
{
    // For some reason find insists on being a non-const method, so here we go...
    ItemItemTimeVecDictionary::const_iterator it = ((LgFrScheduleFloat*)this)->itemToEntry_.find( ( LgFrItem *const) &item);
    return ( it == itemToEntry_.end() ? false : true );
//  return itemToEntry_.contains(&item);
}

// If item is in the table, set tv to be its timeVec and return true,
// else return false.
bool
LgFrScheduleFloat::findValue (const LgFrItem& item, 
			      LgFrTimeVecFloat& retVal)
     const
{
  ItemItemTimeVecDictionary::const_iterator it = ((LgFrScheduleFloat*)this)->itemToEntry_.find( ( LgFrItem *const) &item);
  if( it == itemToEntry_.end() )
      return false;

  retVal = it->second->timeVecFloat();
  return true;

/*  LgFrItemTimeVecPairFloat *p = (LgFrItemTimeVecPairFloat*) (itemToEntry_.findValue(&item));
  if (p != 0) 
  {
    retVal = p->timeVecFloat();
    return true;
  }
  return false;*/
}

// Add new item to the end of the collection
void
LgFrScheduleFloat::insert (const LgFrItemTimeVecPairFloat pair)
{
  if ( schedule_.size() + 1 >= capacity() ) reserve( 2*capacity() + 1);
  schedule_.push_back( pair );
  // Ensure schedule only has one type of item
  assert( schedule_[0].item().isAPart() && pair.item().isAPart() ||
          schedule_[0].item().isADemand() && pair.item().isADemand());

  // Update itemToEntry_.  Note that the only reason the whole dictionary
  // doesn't have to be cleared and refilled is that 
  // schedule_.push_back(...) could not have caused a resize of the schedule
  // because the beginning of this routine makes sure that this class'
  // resize is called if schedule_ is full.
  size_t loc  =  schedule_.size() - 1;
  LgFrItemTimeVecPairFloat* itv = & (schedule_[loc]);
  // insertKeyAndValue requires const cast-away for argument 1
  bool didIt = itemToEntry_.insert( ItemItemTimeVecPair( (LgFrItem*)& (itv->item() ), itv ) ).second;
  if (!didIt) 
  {
    // The key just inserted was already in the dictionary, so it
    // didn't get inserted this time.  Searches for key key will
    // find the itemTimeVec associated with the key previously inserted
    std::cerr << "WARNING: Searches will not find value just inserted" << std::endl;
  }
}



// Add new item to the collection at position index.  Everything afterwards
// is shifted to a higher index.
void
LgFrScheduleFloat::insertAt (const size_t index, 
			  const LgFrItemTimeVecPairFloat pair)
{
  // Test for valid index
  assert (index < this->size());

  // Everthing that itemToEntry_ points to after and including element index
  // of schedule_ will be moved, so delete all of the entries.  (This isn't
  // the most efficient thing to do, but it probably won't happen often.)
  itemToEntry_.clear();

  if (  this->contains(pair.item())  ) {
    std::cerr << "WARNING: Searches will not find one of the itemTimeVec\'s for "
      << pair.item().printingName()  
	 << "; multiple entries have been inserted" << std::endl;
  }
  schedule_.insert(schedule_.begin()+index, pair);
  // Ensure schedule only has one type of item
  assert( schedule_[0].item().isAPart() && pair.item().isAPart() ||
          schedule_[0].item().isADemand() && pair.item().isADemand());

  // Update itemToEntry_
  size_t loc  =  schedule_.size() - 1;
  LgFrItemTimeVecPairFloat* itv = & (schedule_[loc]);

  /* no point doing this since we are going to refresh the whole thing anyway
  bool didIt = itemToEntry_.insert( ItemItemTimeVecPair( (LgFrItem*)& (itv->item() ), itv ) ).second;
  if (!didIt) 
  {
    // The key just inserted was already in the dictionary, so it
    // didn't get inserted this time.  Searches for key key will
    // find the itemTimeVec associated with the key previously inserted
    std::cerr << "WARNING: Searches will not find item just passed to insertAt" << std::endl;
  }*/

  // Since the whole dictionary was cleared, refill it.
  this->fillDictionary();
}

// Insert Item if it isn't already in the schedule.  In any case,
// associate timeVec with item.
void
LgFrScheduleFloat::insertKeyAndValue(const LgFrItem& item, 
				     const LgFrTimeVecFloat& value)
{
  ((*this)[item]) . timeVecFloat (value);
}


// Return the i'th ItemTimeVecPairFloat
LgFrItemTimeVecPairFloat
LgFrScheduleFloat::operator[](const size_t& i )
const
{  
   return ( schedule_[i] );
}

LgFrItemTimeVecPairFloat &
LgFrScheduleFloat::operator[](const size_t& i )
{  
   return ( schedule_[i] );
}


// Return the LgFrItemTimeVecPairFloat whose item matches item.
// If there is no such LgFrItemTimeVecPairFloat, a new one is created
// (using the default LgFrTimeVecFloat constructor) and inserted at
// the end.
LgFrItemTimeVecPairFloat &
LgFrScheduleFloat::operator[](const LgFrItem& item)
{
  LgFrItemTimeVecPairFloat *p = 0;
  ItemItemTimeVecDictionary::const_iterator it = ((LgFrScheduleFloat*)this)->itemToEntry_.find( ( LgFrItem *const) &item);
  if( it != itemToEntry_.end() )
  {
      return *(it->second);
  }
  insert( LgFrItemTimeVecPairFloat( item, LgFrTimeVecFloat() ) );

  it = ((LgFrScheduleFloat*)this)->itemToEntry_.find( ( LgFrItem *const) &item);

  return *(it->second);

/*
  LgFrItemTimeVecPairFloat *p 
    = (LgFrItemTimeVecPairFloat *) (itemToEntry_.findValue(&item));
  if (  p != 0  )
    return *p;
  else {
    this->insert (LgFrItemTimeVecPairFloat(item,
					   LgFrTimeVecFloat())  // default t-v
		  );
    return schedule_[this->size() - 1];	// insert is done at end
  }*/
}

// Return the LgFrItemTimeVecPairFloat whose item matches item.
// If there is no such LgFrItemTimeVecPairFloat, throw an assert exception.
const LgFrItemTimeVecPairFloat &
LgFrScheduleFloat::operator[](const LgFrItem& item)
const
{
  assert ( this->contains(item) );
  // OK to cast away const, because item is contained in schedule_.
  return ((LgFrScheduleFloat*)this)->operator[](item);
}

// Remove all pairs from schedule.
// This is a protected member function.
// Pairs are PairsOfItemTimeVecFloat
void
LgFrScheduleFloat::clear()
{
  itemToEntry_.clear();
  schedule_.clear();
}

// Return number of pairs in schedule
size_t
LgFrScheduleFloat::size()
const
{  
   return ( schedule_.size() );
}

// return schedule capacity, the number of
// itemTimeVec entries the schedule can contain before
// resizing is needed.
size_t 
LgFrScheduleFloat::capacity()
const
{
  return capacity_;
}

// Change the size of the schedule
void
LgFrScheduleFloat::reserve (const size_t newCapacity)
{
  itemToEntry_.clear();	// Old pointers will become invalid
  schedule_.reserve(newCapacity);
  capacity_ = newCapacity;

  // resize and insert correct pointers into itemToEntry_
  this->fillDictionary();
}

void
LgFrScheduleFloat::fillDictionary()
{
//  itemToEntry_.resize(goodSize(capacity_)); can't resize a std::map<>
  size_t n = schedule_.size();
  size_t loc = 0; // Pulled out of the for below by RW2STL
  for (loc = 0;  loc < n;  ++loc) 
  {
    LgFrItemTimeVecPairFloat* itv = & (schedule_[loc]);

    itemToEntry_.insert( ItemItemTimeVecPair( (LgFrItem*)& (itv->item() ), itv ) );
  }
}

// Do real work of diadic schedule operation.
void
LgFrScheduleFloat::kernalOfOp(
   LgFrTimeVecFloatOpFunction vectorOp,
   const LgFrScheduleFloat& rhs,
   LgFrScheduleFloat & retVal,
   float defaultValue )
const
{
   // Determine length of TimeVecs in schedule
   size_t vecLen(0);
   if ( this->size() > 0 )
      vecLen = (*this)[0].timeVecFloat().length();
   else if ( rhs.size() > 0 )
      vecLen = rhs[0].timeVecFloat().length();
   
   // Create vector of zeros to be used when an item is abscent from
   // rhs or (this)
   LgFrTimeVecFloat zeroVec( vecLen, defaultValue );
   LgFrTimeVecFloat resultVec;

   // Loop once for each item in (this)
   size_t i = 0; // Pulled out of the for below by RW2STL
   for (i=0; i<this->size(); i++ ) {
	  const LgFrItemTimeVecPairFloat & ithItemTimeVecFloat = (*this)[i];
      const LgFrTimeVecFloat & ithTimeVecFloat = ithItemTimeVecFloat.timeVecFloat();
      
      int contains = rhs.contains( (*this)[i].item() );	// Is item in rhs?
      if (contains)
         // Item is in rhs, apply vectorOp to lhs and rhs.
         resultVec = ( ithTimeVecFloat .*vectorOp )
                       ( rhs[ (*this)[i].item() ].timeVecFloat() );
      else {
         // Item is not in rhs, use zeroVec for rhs
         resultVec = ( ithTimeVecFloat .*vectorOp ) (zeroVec);
	  }

      // Add computed itemTimeVecPair to schedule
      retVal.insertKeyAndValue ( (*this)[i].item(), resultVec );
   }

   // Loop once for each item in rhs
   for ( i=0; i<rhs.size(); i++ ) {
      // Is the i'th item in rhs, but not (this)
      if ( ! (*this).contains( rhs[i].item() ) ) {
         // Use zeroVec for (this)
         resultVec = ( zeroVec.*vectorOp )
                        ( rhs[i].timeVecFloat() );
         retVal.insertKeyAndValue ( rhs[i].item(), resultVec );
      }
   }
}

// Do real work of diadic schedule operation.
// schedule operation float
void
LgFrScheduleFloat::kernalOfOp(
   LgFrTimeVecFloatRhsScalarOpFunction scalarOp,
   const float rhs,
   LgFrScheduleFloat & retVal)
const
{
   LgFrTimeVecFloat resultVec;

   // Loop once for each item in (this)
   size_t i = 0; // Pulled out of the for below by RW2STL
   for (i=0; i<this->size(); i++ ) {

	  const LgFrItemTimeVecPairFloat & ithItemTimeVecFloat = (*this)[i];
      const LgFrTimeVecFloat & ithTimeVecFloat = ithItemTimeVecFloat.timeVecFloat();
      
      resultVec = ( scalarOp ) ( ithTimeVecFloat, rhs );

      // Add computed itemTimeVecPair to schedule
      retVal.insertKeyAndValue ( (*this)[i].item(), resultVec );
   }

}

void
LgFrScheduleFloat::kernalOfOp(
   const float lhs,
   LgFrTimeVecFloatLhsScalarOpFunction scalarOp,
   LgFrScheduleFloat & retVal)
const
{
   LgFrTimeVecFloat resultVec;

   // Loop once for each item in (this)
   size_t i = 0; // Pulled out of the for below by RW2STL
   for (i=0; i<this->size(); i++ ) {

	  const LgFrItemTimeVecPairFloat & ithItemTimeVecFloat = (*this)[i];
      const LgFrTimeVecFloat & ithTimeVecFloat = ithItemTimeVecFloat.timeVecFloat();
      
      resultVec = ( scalarOp ) ( lhs, ithTimeVecFloat );

      // Add computed itemTimeVecPair to schedule
      retVal.insertKeyAndValue ( (*this)[i].item(), resultVec );
   }

}
// Perform diadic schedule operation.
LgFrScheduleFloat
LgFrScheduleFloat::op(
   LgFrTimeVecFloatOpFunction vectorOp,
   const LgFrScheduleFloat& rhs,
   float defaultValue )
const
{
   LgFrScheduleFloat retVal;

   kernalOfOp(  vectorOp, rhs, retVal, defaultValue );

   return retVal;
}


// Do real work of columnOp method.
void
LgFrScheduleFloat::kernalOfColumnOp(
   const size_t period,
   float (*scalarOperation) (const float, const float),
   const float rhs,
   LgFrScheduleFloat & retVal)
const
{

 LgFrTimeVecFloat resultVec; 
  
 // Loop once for each item in (this)
 size_t i = 0; // Pulled out of the for below by RW2STL
 for (i=0; i<this->size(); i++ ) {

   const LgFrItemTimeVecPairFloat & ithItemTimeVecFloat = (*this)[i];
   const LgFrTimeVecFloat & ithTimeVecFloat = ithItemTimeVecFloat.timeVecFloat();
   
   resultVec = ithTimeVecFloat;
   resultVec[period] = ( scalarOperation ) ( ithTimeVecFloat[period], rhs );

      // Add computed itemTimeVecPair to schedule
   retVal.insertKeyAndValue ( (*this)[i].item(), resultVec );
   }

}

// perform scalar operation on column (period) in schedule
LgFrScheduleFloat
LgFrScheduleFloat::columnOp(
  const size_t period,
  float (*scalarOperation) (const float, const float),
  const float rhs )
const
{
   LgFrScheduleFloat retVal;

   kernalOfColumnOp(  period, scalarOperation, rhs, retVal );

   return retVal;
}

// Perform reduceItems schedule operation.
LgFrTimeVecFloat
LgFrScheduleFloat::reduceItems(
   float (*scalarOp) ( const float, const float),
   const LgFrFilterStrategy & filter )
const
{
   LgFrTimeVecFloat retVal;

   // Determine length of TimeVecs in schedule &
   // Get first TimeVec from schedule which is to be included in the
   // reduction.
   size_t length(0);
   size_t i = 0; // Pulled out of the for below by RW2STL
   for ( i=0; i<this->size(); i++ ) {
      if ( !filter.exclude( (*this)[i] ) ) {
         retVal = (*this)[i].timeVecFloat();
         length = retVal.length();
         break;
      }
   }
   
   // Loop on itemTimeVecs in schedule starting with first itemTimeVec after
   // after the one assigned to retVal
   for ( i++; i<this->size(); i++ )

      // Check to see if the itemTimeVec is to be included in the reduction
      if (  !filter.exclude( (*this)[i] )  )
      {
         // Loop once for each element of the timeVec
         size_t p = 0; // Pulled out of the for below by RW2STL
         for ( p=0; p<length; p++ ) 
            // Perform reduction operation
            retVal[p] = scalarOp( retVal[p], (*this)[i].timeVecFloat()[p] );
      }

   return retVal;
}

// Determine if two schedules are equal.
// Return true if both schedules contain the same itemTimeVec pairs.
bool
LgFrScheduleFloat::isEqual(
   const LgFrScheduleFloat& rhs)
const
{

   // To be equal must have same number of entries
   if ( this->size() != rhs.size() ) return false;

   // Test that itemTimeVecs are equal and in the same order
   size_t i = 0; // Pulled out of the for below by RW2STL
   for ( i=0; i<this->size(); i++ ) 
      if ( ! ( (*this)[i] == rhs[i] )  ) return false;

   return true;
}

bool
LgFrScheduleFloat::operator==(
   const LgFrScheduleFloat& rhs)
const
{
   return this->isEqual(rhs);
}


bool
LgFrScheduleFloat::operator!=(
   const LgFrScheduleFloat& rhs)
const
{
   return !(this->isEqual(rhs));
}



// Assignment operator
LgFrScheduleFloat &
LgFrScheduleFloat::operator=(
			  const LgFrScheduleFloat& rhs)
{
  if (this != &rhs) {		// Check for assignment to self
#if 0
    schedule_ = rhs.schedule_;
#else
    // Try doing assignment w/o operator= because OrderPartVector does not support
    // operator=.
    {
      // Clear the vector contents and change its size
      schedule_.clear();
      schedule_.reserve( rhs.size() );
      int j = 0; // Pulled out of the for below by RW2STL
      for (j = 0;  j < rhs.size();  ++j) {
        schedule_.push_back( rhs[j] );
      }        
      capacity_ = rhs.size();
      // Clear the dictionary contents and refill it
      itemToEntry_.clear();
      this -> fillDictionary();
    }
#endif    
    assert (schedule_.size() == rhs.schedule_.size());
  }
  return *this;
}


// Copy constructor
LgFrScheduleFloat::LgFrScheduleFloat (const LgFrScheduleFloat & source)
:   schedule_(source.schedule_),
    capacity_(source.capacity_)//,
    //itemToEntry_(goodSize (source.capacity_))
{
  this->fillDictionary();
}


// Constructor for a set expected to have up to n parts
LgFrScheduleFloat::LgFrScheduleFloat (const size_t n)
: //schedule_( n ), //size_tMax(n,1) ),
  capacity_( n ) //size_tMax(n,1) )//,
  //itemToEntry_(  goodSize( size_tMax(n,1) )  )
{
    schedule_.reserve( n );
}

// Destructor
LgFrScheduleFloat::~LgFrScheduleFloat ()
{
  // Nothing to do in here
}


// Print member data in class
void
LgFrScheduleFloat::print()
const
{
   std::cout <<"************************************************" <<std::endl;
   std::cout <<"LgFrSchedule" << std::endl;
   int i = 0; // Pulled out of the for below by RW2STL
   for ( i=0; i<size(); i++ )  {
   std::cout <<" *********************************" <<std::endl;
      std::cout <<" entry number " <<i <<":";
      schedule_[i].print();
   }
   std::cout <<"************************************************" <<std::endl;
}

// Addition operator
LgFrScheduleFloat
operator+ (const LgFrScheduleFloat& lhs, const LgFrScheduleFloat& rhs)
{
  return lhs.op(&LgFrTimeVecFloat::add,rhs);
}

// Subtraction operator
LgFrScheduleFloat
operator- (const LgFrScheduleFloat& lhs, const LgFrScheduleFloat& rhs)
{
  return lhs.op(&LgFrTimeVecFloat::subtract,rhs);
}

// maximum 
LgFrScheduleFloat
LgFrScheduleFloat::maximum (
  const LgFrScheduleFloat& rhs)
const
{
  return op(&LgFrTimeVecFloat::maximum,rhs,-FLT_MAX);
}


#ifdef NDEBUG
#undef NDEBUG
#endif
// test LgFrScheduleFloat methods.
void
LgFrScheduleFloat::test()
{
  // Make some parts for the test
  LgFrPart p1( "part1" );
  LgFrPart p2( "part2" );
  LgFrPart p3( "part3" );

  // Make some timeVecFloat for the test
  LgFrTimeVecFloat tvf1(10,1.11f);
  LgFrTimeVecFloat tvf2(10,2.22f);

  // Make some part timeVecFloat pairs for the test
  LgFrItemTimeVecPairFloat ptvfp1( p1, tvf1 );
  LgFrItemTimeVecPairFloat ptvfp2( p2, tvf1 );
  LgFrItemTimeVecPairFloat ptvfp3( p3, tvf2 );
  LgFrItemTimeVecPairFloat ptvfp4( p1, tvf2 );

  // Test the insertion methods
  LgFrScheduleFloat ss;// (1);	// Make it a little small
  				// to test automatic resizing
  ss.insert(ptvfp2);
  ss.insert(ptvfp3);
  ss.insertAt(0, ptvfp1);
  assert (3 == ss.size());
  assert (ss[0].item().name() == "part1");
  assert (ss[2].item().name() == "part3");
  assert (ss[1].item().name() == "part2");

  // Test the assignment operator and the copy constructor
  LgFrScheduleFloat ss1 (1);	// Make it a little small
  				// to test resizing at assignment time
  assert( !ss1.isEqual(ss) );
  ss1.insert(ptvfp4);		// Give it something that should go away
  ss1 = ss;
  LgFrScheduleFloat ss2 (ss);

  size_t i = 0; // Pulled out of the for below by RW2STL
  for ( i = 0; i < ss.size(); i++) {
    // Test result of assignment
    assert (ss1[i] == ss[i]);
    // Test result of copy
    assert (ss2[i] == ss[i]);
    // Print the contents of the OrderedVector part of sop
    // ss[i].print();
    // std::cout << std::endl;
  }

  // Test isEqual method
  assert( ss1.isEqual(ss) );
  assert( ss2.isEqual(ss) );
  assert( ss1 == ss );
  assert( ss2 == ss );

  // Test indexing by item
  assert( ss[p3] == ptvfp3 );
  const LgFrScheduleFloat constSs(ss);
  assert( constSs[p3] == ptvfp3 );


  // Test resize and capacity
/*  LgFrScheduleFloat ss3(0);
  assert( ss3.capacity() == 1 );
  ss3.reserve(2);
  assert( ss3.capacity() == 2 );
  assert (ss3.size() == 0);

  ss3.insertKeyAndValue( p1, LgFrTimeVecFloat( 3, "1 2 3" ) );
  assert( ss3.capacity() == 2 );
  assert (ss3.size() == 1);

  ss3.insertKeyAndValue( p2, LgFrTimeVecFloat( 3, "1 2 3" ) );
  assert( ss3.capacity() == 2 );
  assert (ss3.size() == 2);

  ss3.insertKeyAndValue( p3, LgFrTimeVecFloat( 3, "1 2 3" ) );
  assert( ss3.capacity() == 4 );
  assert (ss3.size() == 3);*/

  // Test format
  std::cout <<"LgFrSchedule.format()" <<std::endl;
  std::cout <<ss.format() <<std::endl;

  LgFrScheduleFloat hl(ss);
  size_t vecLength = hl[0].timeVecFloat().length();
  LgFrTimeVecFloat   hlVec(vecLength,0.0);
  for ( i=0; i<hl.size(); i++ ) {
     hlVec[ i%vecLength ] = 1.0;
     hl[i].timeVecFloat( hlVec );
  }
  std::cout <<"LgFrSchedule.format(hl,true,'X')" <<std::endl;
  std::cout <<ss.format(hl,true,'X') <<std::endl;

  LgFrScheduleFloat hl2(1);
  for ( i=0; i<hl.size(); i=i+2 ) {
     hl2.insert( hl[i] );
  }
  std::cout <<"LgFrSchedule.format(hl2,false,'+')" <<std::endl;
  std::cout <<ss.format(hl2,false,'+') <<std::endl;

  // More testing of isEqual
  assert( !hl2.isEqual(hl) );
  assert( !(hl2 == hl) );
  assert( hl2 == hl2 );

  // Test op method
  {
  LgFrScheduleFloat lhs,rhs,diff;
  float v1[]={ 1., 2., 3., 4. };
  float v2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vec1(4,v1), vec2(4,v2);
  lhs.insertKeyAndValue( p1, vec1 );
  lhs.insertKeyAndValue( p2, vec1 );
  rhs.insertKeyAndValue( p2, vec2 );
  rhs.insertKeyAndValue( p3, vec2 );
  diff = lhs.op( &LgFrTimeVecFloat::subtract, rhs );
  // std::cout <<"diff" <<std::endl;
  // std::cout <<diff.format() <<std::endl;
  assert( diff.size() == 3 );
  assert( diff[0].item() == p1 );
  assert( diff[1].item() == p2 );
  assert( diff[2].item() == p3 );
  assert( diff[0].timeVecFloat() == vec1 );
  assert( diff[1].timeVecFloat() == vec1-vec2 );
  assert( diff[2].timeVecFloat() == 0.0 -vec2 );
  }
  {  // empty rhs
  LgFrScheduleFloat lhs,rhs,diff;
  float v1[]={ 1., 2., 3., 4. };
  float v2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vec1(4,v1), vec2(4,v2);
  lhs.insertKeyAndValue( p1, vec1 );
  lhs.insertKeyAndValue( p2, vec2 );
  diff = lhs.op( &LgFrTimeVecFloat::subtract, rhs );
  // std::cout <<"diff" <<std::endl;
  // std::cout <<diff.format() <<std::endl;
  assert( diff.size() == 2 );
  assert( diff[0].item() == p1 );
  assert( diff[1].item() == p2 );
  assert( diff[0].timeVecFloat() == vec1 );
  assert( diff[1].timeVecFloat() == vec2 );
  }
  {   // empty lhs
  LgFrScheduleFloat lhs,rhs,diff;
  float v1[]={ 1., 2., 3., 4. };
  float v2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vec1(4,v1), vec2(4,v2);
  rhs.insertKeyAndValue( p2, vec1 );
  rhs.insertKeyAndValue( p3, vec2 );
  diff = lhs.op( &LgFrTimeVecFloat::subtract, rhs );
  // std::cout <<"diff" <<std::endl;
  // std::cout <<diff.format() <<std::endl;
  assert( diff.size() == 2 );
  assert( diff[0].item() == p2 );
  assert( diff[1].item() == p3 );
  assert( diff[0].timeVecFloat() == 0.0-vec1 );
  assert( diff[1].timeVecFloat() == 0.0 -vec2 );
  }
  {  // empty rhs and lhs
  LgFrScheduleFloat lhs,rhs,diff;
  diff = lhs.op( &LgFrTimeVecFloat::subtract, rhs );
  // std::cout <<"diff" <<std::endl;
  // std::cout <<diff.format() <<std::endl;
  assert( diff.size() == 0 );
  }
  
  
  // Test operation- method
  {
  LgFrScheduleFloat lhs,rhs,diff;
  float v1[]={ 1., 2., 3., 4. };
  float v2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vec1(4,v1), vec2(4,v2);
  lhs.insertKeyAndValue( p1, vec1 );
  lhs.insertKeyAndValue( p2, vec1 );
  rhs.insertKeyAndValue( p2, vec2 );
  rhs.insertKeyAndValue( p3, vec2 );
  diff = lhs - rhs;
  // std::cout <<"diff" <<std::endl;
  // std::cout <<diff.format() <<std::endl;
  assert( diff.size() == 3 );
  assert( diff[0].item() == p1 );
  assert( diff[1].item() == p2 );
  assert( diff[2].item() == p3 );
  assert( diff[0].timeVecFloat() == vec1 );
  assert( diff[1].timeVecFloat() == vec1-vec2 );
  assert( diff[2].timeVecFloat() == 0.0 -vec2 );
  }
  
  // Test operation+ method
  {
  LgFrScheduleFloat lhs,rhs,result;
  float v1[]={ 1., 2., 3., 4. };
  float v2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vec1(4,v1), vec2(4,v2);
  lhs.insertKeyAndValue( p1, vec1 );
  lhs.insertKeyAndValue( p2, vec1 );
  rhs.insertKeyAndValue( p2, vec2 );
  rhs.insertKeyAndValue( p3, vec2 );
  result = lhs + rhs;
  // std::cout <<"Add" <<std::endl;
  // std::cout <<result.format() <<std::endl;
  assert( result.size() == 3 );
  assert( result[0].item() == p1 );
  assert( result[1].item() == p2 );
  assert( result[2].item() == p3 );
  assert( result[0].timeVecFloat() == vec1 );
  assert( result[1].timeVecFloat() == vec1+vec2 );
  assert( result[2].timeVecFloat() == 0.0 +vec2 );
  }


  // maximum method
  {
  LgFrScheduleFloat rhs,lhs,result;
  float v1a[]={ -1., 0.,  5. };
  float v2a[]={  1., 1., -5. };
  float v1b[]={ -2., 1.,  6. };
  float v3b[]={  3.,-1.,  6. };
  LgFrTimeVecFloat vec1a(3,v1a), vec2a(3,v2a);
  LgFrTimeVecFloat vec1b(3,v1b), vec3b(3,v3b);
  lhs.insertKeyAndValue( p1, vec1a );
  lhs.insertKeyAndValue( p2, vec2a );
  rhs.insertKeyAndValue( p1, vec1b );
  rhs.insertKeyAndValue( p3, vec3b );
  result=lhs.maximum(rhs);
  // std::cout <<"Add" <<std::endl;
  // std::cout <<sched.format() <<std::endl;
  assert( result.size() == 3 );
  assert( result[0].item() == p1 );
  assert( result[1].item() == p2 );
  assert( result[2].item() == p3 );
  assert( result[0].timeVecFloat() == LgFrTimeVecFloat(3,"-1  1  6") );
  assert( result[1].timeVecFloat() == LgFrTimeVecFloat(3," 1  1 -5") );
  assert( result[2].timeVecFloat() == LgFrTimeVecFloat(3," 3 -1  6") );
  }

 
  // Test reduceItems method
  {
  // Make some demands for the test
  LgFrDemand d1( "demand1", &p1);
  LgFrDemand d2( "demand2", &p2);
  LgFrDemand d3( "demand3", &p3);

  LgFrSortingDemandScheduleFloat sched;
  float v1[]={ 1., 2., 3., 4. };
  float v2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vec1(4,v1), vec2(4,v2);
  sched.insertKeyAndValue( d1, vec1 );
  sched.insertKeyAndValue( d2, vec1 );
  sched.insertKeyAndValue( d3, vec2 );
  LgFrTimeVecFloat redSched(sched.reduceItems(LgFrAdd,LgFrFilterStrategy())); 
  assert( redSched.length() == 4 );
  assert( redSched[0]== v1[0]+v1[0]+v2[0] );
  assert( redSched[1]== v1[1]+v1[1]+v2[1] );
  assert( redSched[2]== v1[2]+v1[2]+v2[2] );
  assert( redSched[3]== v1[3]+v1[3]+v2[3] );

  LgFrDemandNameFilterStrategy fs(".*","demand3");
  redSched = sched.reduceItems( LgFrAdd, fs );
  assert( redSched.length() == 4 );
  assert( redSched[0]== v2[0] );
  assert( redSched[1]== v2[1] );
  assert( redSched[2]== v2[2] );
  assert( redSched[3]== v2[3] );

  }
  
  // Test columnOp
  {
  LgFrScheduleFloat schedA;
  float vA1[]={ 1., 2., 3., 4. };
  float vA2[]={ 4., 3., 2., 1. };
  LgFrTimeVecFloat vecA1(4,vA1), vecA2(4,vA2);
  schedA.insertKeyAndValue( p1, vecA1 );
  schedA.insertKeyAndValue( p2, vecA2 );
  LgFrScheduleFloat schedB = schedA.columnOp( 2, ::LgFrAssign, 3.14f );
  LgFrTimeVecFloat vecB1 = vecA1;
  vecB1[2] = 3.14f;
  LgFrTimeVecFloat vecB2 = vecA2;
  vecB2[2] = 3.14f;
  assert( schedB.size() == 2 );
  assert( schedB[0].item() == p1 );
  assert( schedB[1].item() == p2 );
  assert( schedB[0].timeVecFloat() == vecB1 );
  assert( schedB[1].timeVecFloat() == vecB2 );

  LgFrScheduleFloat schedC = schedB.columnOp( 2, ::LgFrMultiply, 2.0 );
  LgFrTimeVecFloat vecC1 = vecB1;
  vecC1[2] = 2 * vecB1[2];
  LgFrTimeVecFloat vecC2 = vecB2;
  vecC2[2] = 2 * vecB2[2];
  assert( schedC.size() == 2 );
  assert( schedC[0].item() == p1 );
  assert( schedC[1].item() == p2 );
  assert( schedC[0].timeVecFloat() == vecC1 );
  assert( schedC[1].timeVecFloat() == vecC2 );
  }
}
