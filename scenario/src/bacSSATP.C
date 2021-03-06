#include <iostream.h>

#include <scenario/src/bacSSATP.h>
#include <scenario/src/itemTimV.h>
#include <scenario/src/scenATP.h>
#include <scenario/src/zeroTvFS.h>
#include <scenario/src/machdep.h>
#include <scenario/src/demdSchP.h>

// Override base class populate method.  Explicitly invoke base 
// class populate method if current state contains data for this class.
void
LgFrBacklogScheduleStrategyForATP::populate(
   LgFrSortingScheduleFloat& sched )
const
{
  // only populate schedule if the state machine is in the state
  //  that this ASS really corresponds to
  if (scenario()->currentState() == desiredState())  
     LgFrAttributeScheduleStrategy::populate( sched );
}


// Make clone copy of object
LgFrAttributeScheduleStrategy *
LgFrBacklogScheduleStrategyForATP::clone()
const
{
  LgFrBacklogScheduleStrategyForATP* newInst = 
      new LgFrBacklogScheduleStrategyForATP(*this);
  return (newInst);
}

// Make clone copy of object with newScenario
LgFrAttributeScheduleStrategy *
LgFrBacklogScheduleStrategyForATP::clone(LgFrScenarioSmartPointer newScenario)
const
{ 
  LgFrBacklogScheduleStrategyForATP* newInst = 
      new LgFrBacklogScheduleStrategyForATP(*this, newScenario);
  return (newInst);
}

// clone method you should not use!
LgFrAttributeScheduleStrategy *
LgFrBacklogScheduleStrategyForATP::clone(LgFrScenario&)
const
{
    cerr << "should not call this clone method" << endl;
    return (LgFrAttributeScheduleStrategy *)NULL;
}

// Return schedule strategy identifier
isAReturnType
LgFrBacklogScheduleStrategyForATP::isA()
const
{
   return id_;
}

// Assignment operator
LgFrBacklogScheduleStrategyForATP &
LgFrBacklogScheduleStrategyForATP::operator=(
	  const LgFrBacklogScheduleStrategyForATP& rhs)
{
  assert( id_ == rhs.id_ );
  if (this != &rhs) {		// Check for assignment to self
    (this->LgFrBacklogScheduleStrategy::operator= ) (rhs);
    scenario(rhs.scenario());
  }
  return *this;
}


// Copy constructor
LgFrBacklogScheduleStrategyForATP::
   LgFrBacklogScheduleStrategyForATP (
      const LgFrBacklogScheduleStrategyForATP & source)
:  LgFrBacklogScheduleStrategy( source ),
   LgFrScheduleMixInForATP( source.scenario(), source.desiredState() ),
   id_(__LGFRBACKLOGSCHEDULESTRATEGYFORATP)
{
  // Nothing to do here
}

// Copy constructor with newScenario
LgFrBacklogScheduleStrategyForATP::
   LgFrBacklogScheduleStrategyForATP (
      const LgFrBacklogScheduleStrategyForATP & source,
      LgFrScenarioSmartPointer newScenario)
:  LgFrBacklogScheduleStrategy(source, *newScenario ),
   LgFrScheduleMixInForATP( newScenario, source.desiredState() ),
   id_(__LGFRBACKLOGSCHEDULESTRATEGYFORATP)
{
  assert( id_ == source.id_);
}

// Constructor
LgFrBacklogScheduleStrategyForATP::
   LgFrBacklogScheduleStrategyForATP (
     LgFrScenarioSmartPointer scenario, int desiredState )
:  LgFrBacklogScheduleStrategy(*scenario),
   LgFrScheduleMixInForATP(scenario, desiredState),
   id_(__LGFRBACKLOGSCHEDULESTRATEGYFORATP)
{
  // Nothing to do here
}

// Default Constructor
// This is a private member function.
LgFrBacklogScheduleStrategyForATP::
   LgFrBacklogScheduleStrategyForATP ()
:  LgFrBacklogScheduleStrategy(),
   LgFrScheduleMixInForATP(),
   id_(__LGFRBACKLOGSCHEDULESTRATEGYFORATP)
{
  // Nothing to do here
}

// Destructor
LgFrBacklogScheduleStrategyForATP::
   ~LgFrBacklogScheduleStrategyForATP ()
{
  // Nothing to do here
}



#ifdef NDEBUG
#undef NDEBUG
#endif
// test LgFrBacklogScheduleStrategyForATP methods.
void
LgFrBacklogScheduleStrategyForATP::test()
{
}

// test LgFrBacklogScheduleStrategyForATP methods.
// Expects the data in testScenario to come from
// "/u/implode/wit/data/examples/diner12/wit.data"
//
// calls difThin2.C and uses contextal information for testing (10/14/94)
void
LgFrBacklogScheduleStrategyForATP::contextTest
  (
   LgFrScenarioForATPSmartPointer testScenario,
   LgFrLastProblemSolved	& lps,			// testScenario's 
   LgFrSetOfDemands		& sod)			// testScenario's 
{
  int i;

  // Test constructor
  LgFrBacklogScheduleStrategyForATP cs1(testScenario);
  assert( cs1.id_ == __LGFRBACKLOGSCHEDULESTRATEGYFORATP );

  // Test isA method
  assert( cs1.isA() == __LGFRBACKLOGSCHEDULESTRATEGYFORATP );

  // Test copy constructor
  LgFrBacklogScheduleStrategyForATP cs2(cs1);
  assert( cs2.isA() == __LGFRBACKLOGSCHEDULESTRATEGYFORATP );

  // Test assignment operator
  LgFrBacklogScheduleStrategyForATP cs3;
  cs3 = cs2;
  assert( cs3.isA() == cs2.isA() );

  {
    // Test clone method
    LgFrAttributeScheduleStrategy *cs4Ptr = cs1.clone();
    assert( cs4Ptr->isA() == cs1.isA() );

    // Test destructor
    delete cs4Ptr;
  }

  LgFrSortingDemandScheduleFloat scheduleCommitted;
  const LgFrSortingDemandScheduleFloat & constScheduleCommitted = 
      scheduleCommitted;

  LgFrSortingDemandScheduleFloatSmartPointer schedule;
  const LgFrSortingDemandScheduleFloat & constSchedule = *schedule;

  // Test populate.  Should be populated with zero timeVec because
  // a backlog schedule does not exist.
  // play with state machine to get into State 0
  cs1.scenario()->changeState(0);
  cs1.setOfDemands(cs1.scenario()->
                         setOfDemands(LGFR_COMMITTED_DEMANDS));
  cs1.desiredState(0);  
  scheduleCommitted.attributeScheduleStrategy( cs1 );

  scheduleCommitted.populate();
  cout << "Committed backlog schedule contains : " <<endl
       << scheduleCommitted.format() << endl;
  assert( constScheduleCommitted[0].item().name() == "Albert_Einstein" );
  

  cs1.setOfDemands(cs1.scenario()->
                         setOfDemands(LGFR_NEWOPPOR_DEMANDS));
  cs1.populate( *schedule );
  cout << "New opportunities schedule contains : " <<endl
       << schedule->format() << endl;  
  assert( constSchedule.entries() == 0 );

  cs1.setOfDemands(cs1.scenario()->
                         setOfDemands(LGFR_DEFAULT_SET));
  cs1.populate( *schedule );
  cout << "Default schedule contains : " <<endl
       << schedule->format() << endl;  
  assert( constSchedule[13].item().name() == "FirmForecast" );

  for ( i=0; i<sod.entries(); i++ ) {
     assert(  constSchedule.contains( sod[i] )  );
     if ( constSchedule[i].item().name() == "Albert_Einstein" ||
          constSchedule[i].item().name() == "Robert_Oppenheimer" ||
          constSchedule[i].item().name() == "Issac_Newton" ||
          constSchedule[i].item().name() == "Mr_Pythagoras" ||
          constSchedule[i].item().name() == "Joseph_Orlicky" ||
          constSchedule[i].item().name() == "Michael_Faraday" ||
          constSchedule[i].item().name() == "Leonhard_Euler" ||
          constSchedule[i].item().name() == "Turings_Machine_Shop" ||
          constSchedule[i].item().name() == "Thomas_Edisons_Repair")        
       { assert( !(constSchedule[sod[i]].timeVecFloat() == 0.0) ); 
       }
     else 
       { assert(  (constSchedule[sod[i]].timeVecFloat() == 0.0) ); 
       }
   }

  // Test populate, due to new implosion solution.
  {
  LgFrFilterStrategy defaultFilter;
  schedule->filterStrategy( defaultFilter );
  }
  schedule->attributeScheduleStrategy( cs1 );
  testScenario->newImplosionSolution();
  cs1.populate( *schedule );
  cout <<endl <<"LgFrBacklogScheduleStrategyForATP" <<endl
       <<schedule->format() <<endl;

  for ( i=0; i<sod.entries(); i++ ) {
     assert(  constSchedule.contains( sod[i] )  );
     assert( lps.backlogVolume(&sod[i]) == 
                constSchedule[sod[i]].timeVecFloat() );
  }

}

