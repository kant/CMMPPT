#include <iostream.h>

#include <scenario/src/timeVecs.h>
#include <scenario/src/sortSchd.h>
#include <scenario/src/sortSchP.h>
#include <scenario/src/timVComp.h>
#include <scenario/src/schdSnPC.h>
#include <scenario/src/scenario.h>
#include <scenario/src/machdep.h>
#include <scenario/src/partSchP.h>

#if defined (WIN32) || defined (__OS2__) || defined (__WINDOWS__)
#undef max
#undef min
#endif


// Do comparison
// The logic for this method is as follows:
// switch:
// case (only one of the items is in the schedule)
//	it precedes the other tiem
// case (neither item is in the schedule)
//	do comparison based on the itemTimeVec's themselves
// case (both items are in the schedule)
//	do the comparison based on the timeVec's in the schedule
// endswitch
// The most straightforward implementation of this is not
// efficient because the current implementation of schedules
// makes it much faster to find an item's timevec if you just
// checked to see if it is in the schedule than if you just
// checked to see if some other item is in the schedule.
int
LgFrScheduleSensitivePairCompareFloat::itemTimeVecPairCompareFloat(
   const LgFrItemTimeVecPairFloat & itvpf1,
   const LgFrItemTimeVecPairFloat & itvpf2)
const
{
  assert( !schedulePtr_.null() );
  assert( timeVecCompareFloatPtr_ != NULL );

  RWBoolean item1InSchedule;	// true iff item 1 is in *schedulePtr_
  RWBoolean item2InSchedule;	// true iff item 2 is in *schedulePtr_

  // Set values of item*InSchedule and return the correct answer if
  // both items are in the schedule
  item1InSchedule  =   schedulePtr_->contains( itvpf1.item() );
  if (item1InSchedule) {
    const LgFrSortingScheduleFloat & constSchedule = *schedulePtr_;
    LgFrTimeVecFloat itvpf1SchedTV (constSchedule[ itvpf1.item() ]
				    .timeVecFloat()  		);
    item2InSchedule  =  schedulePtr_->contains( itvpf2.item() );
    if (item2InSchedule) {
      // Both items are in schedule.
      // Compare the items' timeVecs from the schedule
      int majorKey = 
	 timeVecCompareFloatPtr_->compare(
	    itvpf1SchedTV,			  
	    constSchedule[ itvpf2.item() ].timeVecFloat()  );
      if ( majorKey != 0 ) return majorKey;
      return itvpf1.compareTo( &itvpf2 );
    }
  }
  else
    item2InSchedule  =  schedulePtr_->contains( itvpf2.item() );    

  // If both items are not in schedule then just compare itemTimeVec's
  if ( !item1InSchedule && !item2InSchedule )
     return itvpf1.compareTo( &itvpf2 );

  // Only one item is in schedule, so it preceeds the other
  assert ( ( item1InSchedule && !item2InSchedule ) || 
	   (!item1InSchedule &&  item2InSchedule )   );
  if ( item1InSchedule ) return -1;
  else                   return  1;
}

// Set Schedule containing values to be sorted on
void
LgFrScheduleSensitivePairCompareFloat::schedule(
   LgFrSortingScheduleFloatSmartPointer schedule )
{
   if ( !schedulePtr_.null() ) 
       schedulePtr_->removeDependentFromHeterogeneousList( this );
   schedulePtr_ = schedule;
   schedulePtr_->addDependent( this );
}
   

// Get Schedule containing values to be sorted on
LgFrSortingScheduleFloatSmartPointerToConst 
LgFrScheduleSensitivePairCompareFloat::schedule()
const
{
   return schedulePtr_;
}

// Set timeVec pair compare object.
void
LgFrScheduleSensitivePairCompareFloat::timeVecCompareFloat(
   const LgFrTimeVecCompareFloat & timeVecCompareFloat )
{
   delete timeVecCompareFloatPtr_;
   timeVecCompareFloatPtr_ = timeVecCompareFloat.clone();
}
 

// Get timeVec pair compare object.
const LgFrTimeVecCompareFloat *
LgFrScheduleSensitivePairCompareFloat::timeVecCompareFloat()
const
{
   return timeVecCompareFloatPtr_;
}

// Make clone copy of object - deep copy
LgFrPairCompareFloat *
LgFrScheduleSensitivePairCompareFloat::clone()
const
{
  return new LgFrScheduleSensitivePairCompareFloat(this);
}

// Make clone copy of object with new Scenario
LgFrPairCompareFloat *
LgFrScheduleSensitivePairCompareFloat::clone(LgFrScenario & newScenario)
const
{
  return new LgFrScheduleSensitivePairCompareFloat(*this, newScenario);
}

// Return comparison strategy identifier
isAReturnType
LgFrScheduleSensitivePairCompareFloat::isA()
const
{
   return id_;
}

// Assignment operator
LgFrScheduleSensitivePairCompareFloat &
LgFrScheduleSensitivePairCompareFloat::operator=(
   const LgFrScheduleSensitivePairCompareFloat& rhs)
{
  assert( id_ == rhs.id_ );
  if (this != &rhs) {		// Check for assignment to self
     (this->LgFrPairCompareFloat::operator= ) (rhs);

     if ( !schedulePtr_.null() ) 
        schedulePtr_->removeDependentFromHeterogeneousList( this );

     schedulePtr_ = rhs.schedulePtr_;

     if ( !schedulePtr_.null() ) 
	 schedulePtr_->addDependent( this );

     delete timeVecCompareFloatPtr_;
     timeVecCompareFloatPtr_ = rhs.timeVecCompareFloatPtr_->clone();

  }
  return *this;
}


// Copy constructor
LgFrScheduleSensitivePairCompareFloat::
   LgFrScheduleSensitivePairCompareFloat (
      const LgFrScheduleSensitivePairCompareFloat & source)
:  LgFrPairCompareFloat(source),
   schedulePtr_( source.schedulePtr_ ),
   timeVecCompareFloatPtr_( NULL ),
   id_(__LGFRSCHEDULESENSITIVEPAIRCOMPARE)
{
  assert( id_ == source.id_ );
  if ( !schedulePtr_.null() ) 
      schedulePtr_->addDependent( this );
  timeVecCompareFloatPtr_ = source.timeVecCompareFloatPtr_->clone();
}

// Copy constructor used by clone
LgFrScheduleSensitivePairCompareFloat::
   LgFrScheduleSensitivePairCompareFloat (
      const LgFrScheduleSensitivePairCompareFloat* source)
:  LgFrPairCompareFloat(*source),
   schedulePtr_( source->schedulePtr_.deepCopy() ),
   timeVecCompareFloatPtr_( NULL ),
   id_(__LGFRSCHEDULESENSITIVEPAIRCOMPARE)
{
  assert( id_ == source->id_ );
  if ( !schedulePtr_.null() ) 
      schedulePtr_->addDependent( this );
  timeVecCompareFloatPtr_ = source->timeVecCompareFloatPtr_->clone();
}

// Copy constructor with new Scenario
LgFrScheduleSensitivePairCompareFloat::
   LgFrScheduleSensitivePairCompareFloat (
      const LgFrScheduleSensitivePairCompareFloat & source,
      LgFrScenario & newScenario)
:  LgFrPairCompareFloat(source, newScenario),
   schedulePtr_(),
   timeVecCompareFloatPtr_( NULL ),
   id_(__LGFRSCHEDULESENSITIVEPAIRCOMPARE)
{
  assert( id_ == source.id_ );
  if ( !(source.schedulePtr_.null()) )  {
     schedulePtr_ = LgFrSortingScheduleFloatSmartPointer(source.schedulePtr_->clone(newScenario));
     schedulePtr_->addDependent( this );
   }
  timeVecCompareFloatPtr_ = source.timeVecCompareFloatPtr_->clone();
}

// Default Constructor
LgFrScheduleSensitivePairCompareFloat::
   LgFrScheduleSensitivePairCompareFloat ()
: LgFrPairCompareFloat(),
   schedulePtr_(NULL),
   timeVecCompareFloatPtr_( NULL ),
   id_(__LGFRSCHEDULESENSITIVEPAIRCOMPARE)
{
   timeVecCompareFloatPtr_ = new  LgFrTimeVecCompareFloat;
}

// Destructor
LgFrScheduleSensitivePairCompareFloat::
   ~LgFrScheduleSensitivePairCompareFloat ()
{
  if ( !schedulePtr_.null() ) 
      schedulePtr_->removeDependentFromHeterogeneousList( this );
  delete timeVecCompareFloatPtr_;
}



#ifdef NDEBUG
#undef NDEBUG
#endif
// test LgFrScheduleSensitivePairCompareFloat methods.
void
LgFrScheduleSensitivePairCompareFloat::test()
{
}

// test LgFrScheduleSensitivePairCompareFloat methods.
// Expects testScenario and testScenario2 to contain data from
// /u/implode/wit/data/brenda/denise/wit.data
void
LgFrScheduleSensitivePairCompareFloat::contextTest
    (
     LgFrScenario & testScenario,
     LgFrLastProblemSolved & lps,		// testScenario's
     LgFrSortingPartScheduleFloatSmartPointer criticalSchedule, // testScenario's,
                                                      // these values will be
                                                      // used for sorting
                                                      // a supply and production
                                                      // schedule
     LgFrSortingPartScheduleFloatSmartPointer supplySchedule,   // testScenario's,
                                                      // this schedule will be
                                                      // sorted by
                                                      // critical value
     LgFrSortingPartScheduleFloatSmartPointer productionSchedule, // testScenario's,
                                                      // this schedule will be
                                                      // sorted by max
                                                      // critical value
     LgFrScenario & testScenario2,		// used to test clone(scenario)
     						// method
     LgFrLastProblemSolved & lps2)		// testScenario2's
{

  int i;

  const LgFrSortingPartScheduleFloat & constCriticalSchedule = *criticalSchedule;
  const LgFrSortingPartScheduleFloat & constSupplySchedule = *supplySchedule; 
  const LgFrSortingPartScheduleFloat & constProductionSchedule = 
                      *productionSchedule;
   
  LgFrTimeVecCompareFloat tvcf;
  LgFrMinimumElementTimeVecCompareFloat minElemTC;

  // Test constructor
  LgFrScheduleSensitivePairCompareFloat cs1;
  assert( cs1.id_ == __LGFRSCHEDULESENSITIVEPAIRCOMPARE );
  assert( cs1.schedulePtr_.null() );
  assert( cs1.timeVecCompareFloatPtr_->isA() == __LGFRTIMEVECCOMPAREFLOAT );
  assert( cs1.timeVecCompareFloat()->isA() == __LGFRTIMEVECCOMPAREFLOAT );

  // Test isA method
  assert( cs1.isA() == __LGFRSCHEDULESENSITIVEPAIRCOMPARE );

  // Test setting getting schedule
  assert( cs1.schedule().null() );
  cs1.schedule( criticalSchedule );
  assert( cs1.schedule() == criticalSchedule );

  // Test setting/getting timeVecCompare
  assert( cs1.timeVecCompareFloat()->isA() == __LGFRTIMEVECCOMPAREFLOAT );
  cs1.timeVecCompareFloat( minElemTC );
  assert( cs1.timeVecCompareFloat()->isA() == 
          __LGFRMAXIMUMELEMENTTIMEVECCOMPAREFLOAT );
  cs1.timeVecCompareFloat( tvcf );
  assert( cs1.timeVecCompareFloat()->isA() == __LGFRTIMEVECCOMPAREFLOAT );

  // Test copy constructor - shallow copy of schedule
  LgFrScheduleSensitivePairCompareFloat cs2(cs1);
  assert( cs2.isA() == __LGFRSCHEDULESENSITIVEPAIRCOMPARE );
  assert( cs1.schedule() == cs2.schedule() );

  // Test assignment operator - shallow copy of schedule
  cs2.timeVecCompareFloat( minElemTC );
  LgFrScheduleSensitivePairCompareFloat cs3;
  cs3 = cs2;
  assert( cs3.isA() == cs2.isA() );
  assert( cs3.schedule() == cs2.schedule() );
  assert( cs3.timeVecCompareFloat()->isA() == 
          __LGFRMAXIMUMELEMENTTIMEVECCOMPAREFLOAT );

  // Test clone method - deep copy of schedule
  LgFrPairCompareFloat *cs4Ptr = cs3.clone();
  assert( cs4Ptr->isA() == cs3.isA() );
  assert( ((LgFrScheduleSensitivePairCompareFloat *)cs4Ptr)->
          schedule() != cs3.schedule() );
  assert( *(((LgFrScheduleSensitivePairCompareFloat *)cs4Ptr)->
          schedule()) == *(cs3.schedule()) );
  assert( ((LgFrScheduleSensitivePairCompareFloat *)cs4Ptr)->
          timeVecCompareFloat()->isA() == 
          __LGFRMAXIMUMELEMENTTIMEVECCOMPAREFLOAT );

  // Test destructor
  delete cs4Ptr;

  // Test clone(scenario) method - deep copy of schedule
  LgFrPairCompareFloat * clonedPCwithScen = cs3.clone(testScenario2);
  assert( clonedPCwithScen->isA() == cs1.isA() );
  assert( ((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->
          schedule() != cs1.schedule() );
  assert( *(((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->
          schedule()) == (*(cs1.schedule())) );
  assert( ((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->
          timeVecCompareFloat()->isA() == 
          __LGFRMAXIMUMELEMENTTIMEVECCOMPAREFLOAT );

  // Do implosion and populate criticalSchedule
  lps.computeCriticalList(TRUE);
  testScenario.newImplosionSolution();

  // Make sure contents in the critical schedule in cloned(scen) 
  // is now different from critical
  // schedule in PC used to do the cloning
  assert( ! ( (*((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->
                schedule()) == (*(cs1.schedule())) ) );

  // Test to make sure supply not sorted by critical'ness
  RWBoolean sorted(TRUE);
  for ( i=1; i<supplySchedule->entries(); i++ ){
     if( criticalSchedule->contains( (constSupplySchedule)[i-1].item() ) )
        if( criticalSchedule->contains( (constSupplySchedule)[i].item() ) )
           if ( (constCriticalSchedule)[ (constSupplySchedule)[i-1].part() ].
                   timeVecFloat().compareTo(
                   (constCriticalSchedule)[ (constSupplySchedule)[i].part() ].
                   timeVecFloat() ) > 0 ) {
              sorted=FALSE;
              break;
           }
  }
  assert( !sorted );

  // Sort and test results
  supplySchedule->sort(&cs1);
  sorted = TRUE;
  for ( i=1; i<supplySchedule->entries(); i++ ){
     if( criticalSchedule->contains( (constSupplySchedule)[i-1].item() ) )
        if( criticalSchedule->contains( (constSupplySchedule)[i].item() ) )
           if ( (constCriticalSchedule)[ (constSupplySchedule)[i-1].part() ].
                   timeVecFloat().compareTo(
                   (constCriticalSchedule)[ (constSupplySchedule)[i].part() ].
                   timeVecFloat() ) > 0 ) {
              sorted=FALSE;
              break;
           }
  }
  assert( sorted );

  // Test to make sure production not sorted by min critical'ness
  sorted = TRUE;
  for ( i=1; i<productionSchedule->entries(); i++ ){
     if( criticalSchedule->contains( (constProductionSchedule)[i-1].item() ) )
        if( criticalSchedule->contains( (constProductionSchedule)[i].item() ) )
           if ( (constCriticalSchedule)[ (constProductionSchedule)[i-1].part() ].
                   timeVecFloat().min() >
                   (constCriticalSchedule)[ (constProductionSchedule)[i].part() ].
                   timeVecFloat().min()  ) {
              sorted=FALSE;
              break;
           }
  }
  assert( !sorted );

  // Sort and test results
  productionSchedule->sort(&cs3);
  sorted = TRUE;
  for ( i=1; i<productionSchedule->entries(); i++ ){
     if( criticalSchedule->contains( (constProductionSchedule)[i-1].item() ) )
        if( criticalSchedule->contains( (constProductionSchedule)[i].item() ) )
           if ( (constCriticalSchedule)[ (constProductionSchedule)[i-1].part() ].
                   timeVecFloat().min() >
                   (constCriticalSchedule)[ (constProductionSchedule)[i].part() ].
                   timeVecFloat().min()  ) {
              sorted=FALSE;
              break;
           }
  }
  assert( sorted );

  
  // Change order of sorted supply and production schedule, 
  // do an implosion, and
  // then check to make sure schedules are returned to sorted order.
  assert( supplySchedule->entries()>5 );
  assert( productionSchedule->entries()>5 );
  LgFrItemTimeVecPairFloat itv4supply( constSupplySchedule[4] );
  LgFrItemTimeVecPairFloat itv3supply( constSupplySchedule[3] );
  LgFrItemTimeVecPairFloat itv0supply( constSupplySchedule[0] );
  supplySchedule->localInsertAt (4, itv3supply);
  supplySchedule->localInsertAt (3, itv0supply);
  supplySchedule->localInsertAt (0, itv4supply);

  LgFrItemTimeVecPairFloat itv4production( constProductionSchedule[4] );
  LgFrItemTimeVecPairFloat itv3production( constProductionSchedule[3] );
  LgFrItemTimeVecPairFloat itv0production( constProductionSchedule[0] );
  productionSchedule->localInsertAt (4, itv3production);
  productionSchedule->localInsertAt (3, itv0production);
  productionSchedule->localInsertAt (0, itv4production);
  
  testScenario.newImplosionSolution();

  assert( constSupplySchedule[4] == itv4supply );
  assert( constSupplySchedule[3] == itv3supply );
  assert( constSupplySchedule[0] == itv0supply );
  assert( constProductionSchedule[4] == itv4production );
  assert( constProductionSchedule[3] == itv3production );
  assert( constProductionSchedule[0] == itv0production );

  // Test copy constructor of PC created by clone(scen)
  LgFrScheduleSensitivePairCompareFloat copyOfClonedPCwithScen(
    * (LgFrScheduleSensitivePairCompareFloat*) clonedPCwithScen);
  assert( copyOfClonedPCwithScen.isA() == __LGFRSCHEDULESENSITIVEPAIRCOMPARE );
  // Schedule pointers should be the same as it's a shallow copy
  assert( copyOfClonedPCwithScen.schedule() == 
	((LgFrScheduleSensitivePairCompareFloat*)clonedPCwithScen)->schedule() );

  // Test assignment operator of PC created by clone(scen) - shallow copy
  LgFrScheduleSensitivePairCompareFloat assignmentOfClonedPCwithScen;
  assignmentOfClonedPCwithScen = 
     *((LgFrScheduleSensitivePairCompareFloat*)clonedPCwithScen);
  assert( assignmentOfClonedPCwithScen.isA() == 
          ((LgFrScheduleSensitivePairCompareFloat*)clonedPCwithScen)->isA() );
  // Schedule pointers should be the same
  assert( assignmentOfClonedPCwithScen.schedule() == 
          ((LgFrScheduleSensitivePairCompareFloat*)
          clonedPCwithScen)->schedule() );

  // Do implosion for 2nd scenario
  lps2.computeCriticalList(TRUE);
  testScenario2.newImplosionSolution();

  // Make sure critical schedule in cloned(scen) is now equal to critical
  // schedule in PC used to do the cloning
  assert(  (*((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->
                schedule()) == (*(cs1.schedule())) );

  // Make sure critical schedule stored in PC's associated with the 2nd scenario
  // are equal
  assert(  
     *(((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->schedule())
     == 
     *(copyOfClonedPCwithScen.schedule())   ); 
  assert(  
     *(((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->schedule())
     == 
     *(assignmentOfClonedPCwithScen.schedule())   ); 

  // Test setting a schedule in the PC when the PC owns the current schedule
  LgFrSortingPartScheduleFloatSmartPointer aPartSchedulePtr(new LgFrSortingPartScheduleFloat);
  ((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->
     schedule( aPartSchedulePtr );
  // Now test to make sure these 2 PC's have the same schedule
  assert(  
((LgFrScheduleSensitivePairCompareFloat *)clonedPCwithScen)->schedule() ==
aPartSchedulePtr );

  // Test clone(scenario) destructor
  delete clonedPCwithScen;
}
