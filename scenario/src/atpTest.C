#include <stdlib.h>
#include <wit/src/wit.h>
#include <rw/rstream.h>
#include <scenario/src/calendar.h>
#include <scenario/src/difImpl.h>
#include <scenario/src/difPWatp.h>
#include <scenario/src/difCmrpF.h>
#include <scenario/src/scBdATP.h>
#include <scenario/src/scenATP.h>
#include <scenario/src/scenDrct.h>
#include <scenario/src/ipATP.h>
#include <scenario/src/schFcATP.h>
#include <scenario/src/outInter.h>
#include <scenario/src/schdDeSt.h>
#include <scenario/src/multPSch.h>
#include <scenario/src/multDSch.h>
#include <scenario/src/timeVecs.h>
// #include <scenario/src/sOfPaFS.h>
// #include <scenario/src/tvElemPC.h>
// #include <scenario/src/zeroTvFS.h>
// #include <scenario/src/boolFS.h>
#include <scenario/src/scATPP.h>


  // -----------------------------------------------------------------
  // The 5 multi schedules:
  //    Committed Order Window 
  //    New-op Order Window 
  //    Forecast Order Window 
  //    Material Window 
  //    Capacity Window 
  // -----------------------------------------------------------------

  // -----------------------------------------------------------------
  //
  // Present on the window are:
  //
  // Three task buttons.  These are radio buttons and always selectable.
  // They are:   Committed Orders
  //             New Op Orders
  //             Forecast orders
  //
  // There is a RecoveryPlan toggle button.  This is only selectable
  // when the Committed Order Task Button is pressed.
  // If the RecoveryPlan button is selected when NewOpOrders or 
  // ForecastOrders is selected, then RecoveryPlan becomes unselected
  // (ie toggle button pops out) and becomes unselectable (not sensitive)
  //
  // There is an implode push button.  This button becomes selectable
  // (sensitive or lit) when solution for current task is no longer correct.
  //
  // -----------------------------------------------------------------

// -----------------------------------------------------------------
// Define tasks
// -----------------------------------------------------------------
typedef enum { COMMITTED, FORECAST, NEWOP } LgFrTasks;


// -----------------------------------------------------------------
// Take required action when implode button is pressed
// -----------------------------------------------------------------
void
implodeButtonPressed(
   LgFrScenarioForATPSmartPointer s,
   LgFrTasks taskButtonCurrentlyPressed,   
   RWBoolean viewingPartOrCapacityMultischedule )
{
   if ( viewingPartOrCapacityMultischedule ) {

      assert( taskButtonCurrentlyPressed == COMMITTED );
      s->changeState(LGFR_ATP_STATE_COMMITTED_IMPLODED_RECOVERY);

   }
   else {

      if ( taskButtonCurrentlyPressed == COMMITTED ) 
         s->changeState(LGFR_ATP_STATE_COMMITTED_IMPLODED);

      else if ( taskButtonCurrentlyPressed == FORECAST ) 
         s->changeState(LGFR_ATP_STATE_ATP_IMPLODED);

      else if ( taskButtonCurrentlyPressed == NEWOP ) 
         s->changeState(LGFR_ATP_STATE_NEWOPP_IMPLODED);

      else
         assert( 1==2 && "Unrecognized task button pressed" );

      s->newImplosionSolution();
   }
}


//-----------------------------------------------------------------------
// Create committed demand multi schedule
//-----------------------------------------------------------------------
LgFrMultiDemandScheduleFloat * 
committedMultiSched( 
  LgFrScenarioSmartPointer scenarioPtr )
{
  LgFrMultiDemandScheduleFloat * retVal =  
         new LgFrMultiDemandScheduleFloat(scenarioPtr);

  retVal->newSchedule(      LGFRCommittedDemandVolume               );
  retVal->newSchedule(      LGFRCommittedCumDemandVolume            );
  retVal->newSchedule(      LGFRCommittedShipmentVolume             );
  retVal->newSchedule(      LGFRCommittedCumShipmentVolume          );
  retVal->newSchedule(      LGFRCommittedBacklog                    );

#if 0
  // wibni's TO BE ADDED
  DONT YET HAVE SCHEDULE STRATEGIES
  retVal->newSchedule(      LGFRCommittedPriority                   );
  retVal->newSchedule(      LGFRCommittedRevenueTimesDemandVol      );
  retVal->newSchedule(      LGFRCommittedCumRevenueTimesDemandVol   );
  retVal->newSchedule(      LGFRCommittedRevenueTimesShipmentVol    );
  retVal->newSchedule(      LGFRCommittedCumRevenueTimesShipmentVol );
#endif

  return retVal;
}


//-----------------------------------------------------------------------
// Create forecast demand multi schedule
//-----------------------------------------------------------------------
LgFrMultiDemandScheduleFloat * 
firmForecastMultiSched( 
  LgFrScenarioSmartPointer scenarioPtr )
{
  LgFrMultiDemandScheduleFloat * retVal =  
         new LgFrMultiDemandScheduleFloat(scenarioPtr);

  retVal->newSchedule(      LGFRFirmForecastDemandVolume               );
  retVal->newSchedule(      LGFRFirmForecastCumDemandVolume            );
  retVal->newSchedule(      LGFRFirmForecastShipmentVolume             );
  retVal->newSchedule(      LGFRFirmForecastCumShipmentVolume          );

  return retVal;
}


//-----------------------------------------------------------------------
// Create forecast demand multi schedule
//-----------------------------------------------------------------------
LgFrMultiDemandScheduleFloat * 
newOpportunityMultiSched( 
  LgFrScenarioSmartPointer scenarioPtr )
{
  LgFrMultiDemandScheduleFloat * retVal =  
         new LgFrMultiDemandScheduleFloat(scenarioPtr);

  retVal->newSchedule(      LGFRNewOpportunityDemandVolume               );
  retVal->newSchedule(      LGFRNewOpportunityCumDemandVolume            );
  retVal->newSchedule(      LGFRNewOpportunityShipmentVolume             );
  retVal->newSchedule(      LGFRNewOpportunityCumShipmentVolume          );
  retVal->newSchedule(      LGFRNewOpportunityBacklog                    );
#if 0
  WIBNI
  retVal->newSchedule(      LGFRNewOpportunityPriority                   );
#endif
  return retVal;
}


//-----------------------------------------------------------------------
// Create material recovery multi schedule
//-----------------------------------------------------------------------
LgFrMultiPartScheduleFloat * 
materialRecoveryMultiSched( 
  LgFrScenarioSmartPointer scenarioPtr )
{
  LgFrMultiPartScheduleFloat * retVal =  
         new LgFrMultiPartScheduleFloat(scenarioPtr);

  retVal->newSchedule(      LGFRMaterialSupplyVolume                       );
  retVal->newSchedule(      LGFRMaterialCumSupplyVolume                    );
  retVal->newSchedule(      LGFRMaterialFocussedShortageSchedule           );
  retVal->newSchedule(      LGFRMaterialCumFocussedShortageSchedule        );

  return retVal;
}

    
//-----------------------------------------------------------------------
// Create capacity recovery multi schedule
//-----------------------------------------------------------------------
LgFrMultiPartScheduleFloat * 
capacityRecoveryMultiSched( 
  LgFrScenarioSmartPointer scenarioPtr )
{
  LgFrMultiPartScheduleFloat * retVal =  
         new LgFrMultiPartScheduleFloat(scenarioPtr);

  retVal->newSchedule(      LGFRCapacitySupplyVolume                       );
  retVal->newSchedule(      LGFRCapacityFocussedShortageSchedule           );
  retVal->newSchedule(      LGFRCapacityCumFocussedShortageSchedule        );

  return retVal;
}

//-----------------------------------------------------------------------
// Return true if implode button should be lit (sensitive)
//-----------------------------------------------------------------------
RWBoolean
implodeButtonLit(
   LgFrScenarioForATPSmartPointer s,
   LgFrTasks taskButtonCurrentlyPressed, 
   RWBoolean viewingPartOrCapacityMultischedule )
{
   RWBoolean retval=FALSE;

   if ( viewingPartOrCapacityMultischedule ) {

      assert( taskButtonCurrentlyPressed == COMMITTED );
      retval=!s->solutionIsUpToDate(LGFR_ATP_STATE_COMMITTED_IMPLODED_RECOVERY);

   }
   else {

      if ( taskButtonCurrentlyPressed == COMMITTED ) 
         retval=!s->solutionIsUpToDate(LGFR_ATP_STATE_COMMITTED_IMPLODED);

      else if ( taskButtonCurrentlyPressed == FORECAST ) 
         retval=!s->solutionIsUpToDate(LGFR_ATP_STATE_ATP_IMPLODED);

      else if ( taskButtonCurrentlyPressed == NEWOP ) 
         retval=!s->solutionIsUpToDate(LGFR_ATP_STATE_NEWOPP_IMPLODED);

      else
         assert( 1==2 && "Unrecognized task button pressed" );

   }

   return retval;
}


//-----------------------------------------------------------------------
// Change the value of a timeVecFloat entry contained in a multi part
// schedule.
//-----------------------------------------------------------------------
void
changeMultiScheduleTimeVecEntry(
  LgFrScenarioForATPSmartPointer s,
  LgFrMultiScheduleFloat & multiSched,
  LgFrScheduleDescriptor schedDesc,
  RWCString partName,
  size_t period,
  float newValue )
{

  // Get set of all parts
  const LgFrSetOfParts & setOfParts = s->constSetOfParts();

  // Get part with changed timeVec entry
  const LgFrPart * alteredPartPtr = setOfParts.findValue(partName);
  assert( alteredPartPtr != NULL );
  LgFrPart alteredPart = *alteredPartPtr;

  // Get original timeVec
  const LgFrTimeVecFloat * timeVecPtr =
     multiSched.getTimeVecFloat(alteredPart, schedDesc);
  assert( timeVecPtr != NULL );
  LgFrTimeVecFloat timeVec( *timeVecPtr );

  // Change time vec entry
  timeVec[period] = newValue;

  // Set the updated timeVec in the schedule
  RWBoolean result;
  result = multiSched.changeTimeVecInSchedule(
                         alteredPart, schedDesc, timeVec );
  assert( result );

}


//-----------------------------------------------------------------------
//
// ATP Test Main program
//
// Command Line Format: cmrpTest [filename-prefix] [v]
//
// Both command line parameters are optional, but filename-prefix must be
// used if you want to use the second parameter.
//
// If the filename parameter is given, it is used as the beginning part
// of a set of input file in the "CMRP" format documented in difCmrpF.h.
//
// If the second parameter is given, the output will be much more verbose
// than if it is not
//
//-----------------------------------------------------------------------
int main(int argc, char * * argv)
{
  size_t i;
  RWBoolean useFileData;
  RWBoolean verbosePrinting;

  useFileData = FALSE;
  verbosePrinting = FALSE;
  if (argc > 1) {
    // interactive = TRUE;	// Old version
    useFileData = TRUE;
  }
  if (argc > 2) {
    verbosePrinting = TRUE;
  }

  const LgFrSchedulePrintStyle printStyle(useFileData  ?  CSV  :  TEXT);

  LgFrCalendar calendar;
  LgFrScenarioForATPSmartPointer scenarioPtr;

  // Initialize calendar and create a scenario
  if (useFileData) {
    RWCString baseFileName = *(argv+1);
    RWCString msg
      = "INFO: Using the set of CMRP input files named\n"
      + baseFileName + ".{raw,dem,bom,...}";
    cout << msg << endl;
    cerr << msg << endl;
    calendar =
      LgFrDataInterfaceImplementationFromCmrpFiles::calendar( baseFileName );
    LgFrDataInterfaceImplementationFromCmrpFiles difImp( baseFileName );
    registerCmrpFilesDifImpFunctions(&difImp);
    LgFrDataInterface dif(&difImp);

    // Create a scenario using the builder
    LgFrScenarioBuilderForATP builder;
    LgFrScenarioDirector director(&builder);
    director.construct(dif, calendar);

    scenarioPtr = builder.getScenario();
  }
  else {	// Use Perfect World Data
    calendar = LgFrCalendar(8, RWTime(RWDate("8/01/95")));

    // -----------------------------------------------------------------
    // Create the dataInterfaceObject.
    // When obtaining data from the AS400 the code will be something like:
    //    RWCString as400Library="thinair2"  
    //    LgFrDataInterfaceFromMapics 
    //        dif("mapics","fasano","htrag7",as400Library)
    // It would be good to be able to specify/change the AS400 library 
    // on the command line or in a dialog box.
    //
    // -----------------------------------------------------------------
    // For now use the hard coded dif.
    // -----------------------------------------------------------------
    LgFrDataInterfaceImplementation * difPWImpPtr
	= new LgFrDataInterfaceImplementation("internal data set pw2");
    // Register all functions
    registerPWatpDifImpFunctions(difPWImpPtr);
    LgFrDataInterface dif(difPWImpPtr);

    delete difPWImpPtr;
    difPWImpPtr = NULL;

    // Create a scenario using the builder
    LgFrScenarioBuilderForATP builder;
    LgFrScenarioDirector director(&builder);  
    director.construct(dif, calendar);
    scenarioPtr = builder.getScenario();
  }

  // -----------------------------------------------------------------
  // Instantiate the Committed Order Window 
  // -----------------------------------------------------------------
  LgFrMultiDemandScheduleFloat * committedMSDptr =  
         committedMultiSched(scenarioPtr);
  LgFrMultiDemandScheduleFloat & committedMSD = *committedMSDptr;

  // -----------------------------------------------------------------
  // Instantiate the Forecast Order Window 
  // -----------------------------------------------------------------
  LgFrMultiDemandScheduleFloat * forecastMSDptr =  
         firmForecastMultiSched( scenarioPtr );
  LgFrMultiDemandScheduleFloat & forecastMSD = *forecastMSDptr;

  // -----------------------------------------------------------------
  // Instantiate the NewOp Order Window 
  // -----------------------------------------------------------------
  LgFrMultiDemandScheduleFloat * newOpMSDptr =  
         newOpportunityMultiSched( scenarioPtr );
  LgFrMultiDemandScheduleFloat & newOpMSD = *newOpMSDptr;


  // -----------------------------------------------------------------
  // Obtain abbreviations of committed multiSchedule window attributes
  // -----------------------------------------------------------------
  LgFrVectorRWCString abbr = committedMSD.schedAbbrevVec();
  for ( i=0; i<abbr.length(); i++ )
     cout <<"Abbreviation of attribute " <<i <<" is " <<abbr[i] <<endl;

  // -----------------------------------------------------------------
  // Obtain schedule descriptions from multiSchedule
  // -----------------------------------------------------------------
  LgFrVectorScheduleDescriptor sd = committedMSD.schedDescVec();
  assert( sd.length() == 5 );
  assert( sd[0] == LGFRCommittedDemandVolume               );
  assert( sd[1] == LGFRCommittedCumDemandVolume            );
  assert( sd[2] == LGFRCommittedShipmentVolume             );
  assert( sd[3] == LGFRCommittedCumShipmentVolume          );
  assert( sd[4] == LGFRCommittedBacklog                    );

  // ----------------------------------------------------------------- 
  cout << "Step 0:  Display Committed Multischedule\n";
  cerr << "Step 0:  Display Committed Multischedule\n";
  // ----------------------------------------------------------------- 

  // display the commitedMSD
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // NOTE: the committed task button should be down,
  //       the Implosion button should be lit,
  //       the recovery plan button should be lit and up.
  assert( implodeButtonLit( scenarioPtr, COMMITTED, FALSE ) );

  // ----------------------------------------------------------------- 
  cout << "Step 1:  Implode and show implosion result attributes\n";
  cerr << "Step 1:  Implode and show implosion result attributes\n";
  // ----------------------------------------------------------------- 

  // User hits the implode button
  implodeButtonPressed( scenarioPtr, COMMITTED, FALSE );

  // NOTE: the committed task button should be down,
  //       the Implosion button should be off,
  //       the recovery plan button should be lit and up(?).
  assert( !implodeButtonLit( scenarioPtr, COMMITTED, FALSE ) );


  // display the commitedMSD
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout << "Step 1.1:  Filter out demands not beginning with M\n";
  cerr << "Step 1.1:  Filter out demands not beginning with M\n";
  // ----------------------------------------------------------------- 

  committedMSD.filterItemSelectionSchedule( 
     LGFRCommittedDemandVolume, 
     LGFRFilterOnDemandAndPartName, ".*", "^M.*" );
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout << "Step 1.2:  Filter out demands not beginning with M\n"
       << "           and parts not beginning with C\n";
  cerr << "Step 1.2:  Filter out demands not beginning with M\n"
       << "           and parts not beginning with C\n";
  // ----------------------------------------------------------------- 

  committedMSD.filterItemSelectionSchedule( 
     LGFRCommittedDemandVolume, 
     LGFRFilterOnDemandAndPartName, "^C.*", "^M.*" );
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 1.3:  Remove filter and show all demands\n";
  cerr <<"Step 1.3:  Remove filter and show all demands\n";
  // ----------------------------------------------------------------- 

  committedMSD.filterItemSelectionSchedule( 
     LGFRCommittedDemandVolume, LGFRFilterNothing ); 
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);


  // ----------------------------------------------------------------- 
  cout << "Step 2:  Look at recovery windows \n";
  cerr << "Step 2:  Look at recovery windows \n";
  // ----------------------------------------------------------------- 

  // User presses recovery plan button putting it in the down position

  // Bring state machine to recovery plan state
  scenarioPtr->changeState(LGFR_ATP_STATE_COMMITTED_IMPLODED_RECOVERY);
#if 0
  //doing an explode to get the reqVol's for cooking purposes
  witReturnCode rc;
  witSetMesgTimesPrint( scenarioPtr->lps().witRun(), 
     WitFALSE, WitINFORMATIONAL_MESSAGES, UCHAR_MAX );
  witSetMesgFileAccessMode( scenarioPtr->lps().witRun(), WitFALSE,  "w" ); 
  rc = witMrp ( scenarioPtr->lps().witRun() );
  witWriteReqSched( scenarioPtr->lps().witRun(), NULL, WitBSV );
  witSetMesgTimesPrint( scenarioPtr->lps().witRun(), 
     WitFALSE, WitINFORMATIONAL_MESSAGES, 0 );
#endif  
  // ----------------------------------------------------------------
  // When changing to state LGFR_ATP_STATE_COMMITTED_IMPLODED_RECOVERY,
  // the state machine sets the focus to contain every demand
  // (possibly only those with nonzero backlog) with the horizon being
  // the last period.
  // ----------------------------------------------------------------

  // ----------------------------------------------------------------
  // A wibini is to let user select the demands and period to
  // add to the focus horizon, rather than just adding all demands
  // with the last period.
  // Code to do this might look like:
  // ----------------------------------------------------------------
#if 0
  {
    // Get fss ctrl object from scenario
    LgFrFocussedShortageScheduleControl & fssCtrl =
         scenarioPtr->focussedShortageScheduleControl();

    // Clear the focus horizon set by the state machine. 
    fssCtrl.clearFocusHorizons();

    // Somehow get the demand(s) to be added to the focus.
    // This code selects the 2nd demand in the set of committed demands.
    const LgFrDemand focusDemand = 
       scenarioPtr->constSetOfDemands(LGFR_COMMITTED_DEMANDS)[1];

    // Set the selected demand's focus horizon 
    int horizon = 6; 

    // Set focus horizon for the selected demand(s)
    fssCtrl.focusHorizon( focusDemand, horizon );
  }
#endif

  // NOTE: the committed task button should be down,
  //       the Implosion button should be unlit,
  //       the recovery plan button should be lit and down.
  assert( !implodeButtonLit( scenarioPtr, COMMITTED, TRUE ) );

  // Create Material & Capacity Recovery Window
  LgFrMultiPartScheduleFloat * matRecovMSPptr =  
         materialRecoveryMultiSched( scenarioPtr );
  LgFrMultiPartScheduleFloat & matRecovMSP = *matRecovMSPptr;
  LgFrMultiPartScheduleFloat * capRecovMSPptr =  
         capacityRecoveryMultiSched( scenarioPtr );
  LgFrMultiPartScheduleFloat & capRecovMSP = *capRecovMSPptr;

  // display the material and capacity windows
  if ( ! useFileData )
    matRecovMSP.printByItemAndAttribute(printStyle);
  if ( ! useFileData )
    capRecovMSP.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 2.1:  Filter out parts that don't match name Screen_*\n";
  cerr <<"Step 2.1:  Filter out parts that don't match name Screen_*\n";
  // ----------------------------------------------------------------- 

  matRecovMSP.filterItemSelectionSchedule( 
     LGFRMaterialSupplyVolume, LGFRFilterOnPartName, "^Screen_.*" );
  if ( ! useFileData )
    matRecovMSP.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 2.2:  Filter out parts which don't have shortages\n";
  cerr <<"Step 2.2:  Filter out parts which don't have shortages\n";
  // ----------------------------------------------------------------- 

  matRecovMSP.filterItemSelectionSchedule( 
     LGFRMaterialSupplyVolume, LGFRFilterOnShortPart );
  if ( ! useFileData )
    matRecovMSP.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 2.3:  Filter out capacities which don't have shortages\n";
  cerr <<"Step 2.3:  Filter out capacities which don't have shortages\n";
  // ----------------------------------------------------------------- 

  capRecovMSP.filterItemSelectionSchedule( 
     LGFRCapacitySupplyVolume, LGFRFilterOnShortCapacity );
  if ( ! useFileData )
    capRecovMSP.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 2.4:  Remove filter and show all capacities\n";
  cerr <<"Step 2.4:  Remove filter and show all capacities\n";
  // ----------------------------------------------------------------- 

  capRecovMSP.filterItemSelectionSchedule( 
     LGFRCapacitySupplyVolume, LGFRFilterNothing );
  if ( ! useFileData )
    capRecovMSP.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 2.5:  Remove filter and show all material\n";
  cerr <<"Step 2.5:  Remove filter and show all material\n";
  // ----------------------------------------------------------------- 

  matRecovMSP.filterItemSelectionSchedule( 
     LGFRMaterialSupplyVolume, LGFRFilterNothing );
  if ( ! useFileData )
    matRecovMSP.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  if ( useFileData ) {
    cout <<"Step 3:  Do nothing\n";
    cerr <<"Step 3:  Do nothing\n";
    cout <<"Step 4:  Do nothing\n";
    cerr <<"Step 4:  Do nothing\n";
  }
  else {
    cout <<"Step 3:  Change material/capacity supply\n";
    cerr <<"Step 3:  Change material/capacity supply\n";

    // Change supply of several parts in varying periods
    changeMultiScheduleTimeVecEntry( 
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume, 
	"RAW_Board_F", 2, 500. );

    changeMultiScheduleTimeVecEntry( 
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume, 
	"Screen_S", 3, 385. );

    changeMultiScheduleTimeVecEntry( 
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume, 
	"Screen_S", 4, 385. );


// #define BigATPNumbers
#ifdef BigATPNumbers
    // For Testing, Set Supply very high to ensure that
    // there will be shipments to forecast demands.
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"MEM_Module_S", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"CPU_Type_F", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"CPU_Type_S", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"RAW_Board_F", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"RAW_Board_S", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"Disk_Drive_L", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"Disk_Drive_S", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"Disk_Drive_S", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"Disk_Drive_S", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"Screen_L", 0, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, matRecovMSP, LGFRMaterialSupplyVolume,
	"Screen_S", 0, 50000. );
    // Change supply of several capacities in varying periods
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, capRecovMSP, LGFRCapacitySupplyVolume,
	"System_Test", 5, 50000. );
    changeMultiScheduleTimeVecEntry(
	scenarioPtr, capRecovMSP, LGFRCapacitySupplyVolume,
	"System_Test", 6, 50000. );
#endif

    // Display updated material and capacity supply 
    if ( ! useFileData )
      matRecovMSP.printByItemAndAttribute(printStyle);
    if ( ! useFileData )
      capRecovMSP.printByItemAndAttribute(printStyle);

    // NOTE: the committed task button should be down,
    //       the Implosion button should be lit,
    //       the recovery plan button should be lit and down.
    assert( implodeButtonLit( scenarioPtr, COMMITTED, TRUE ) );


    // ----------------------------------------------------------------- 
    cout <<"Step 4:  Implode using new supply data\n";
    cerr <<"Step 4:  Implode using new supply data\n";
    // ----------------------------------------------------------------- 

    // User presses implode button to get new committed shipment schedule and
    // recovery plan.
    implodeButtonPressed( scenarioPtr, COMMITTED, TRUE );

    // NOTE: the committed task button should be down,
    //       the Implosion button should be unlit,
    //       the recovery plan button should be lit and down.
    assert( ! implodeButtonLit( scenarioPtr, COMMITTED, TRUE ) );
  }


#if 0
  DEBUGGING CODE.
  witSetMesgTimesPrint( scenarioPtr->lps().witRun(), 
     WitFALSE, WitINFORMATIONAL_MESSAGES, UCHAR_MAX );
  witSetMesgFileAccessMode( scenarioPtr->lps().witRun(), WitFALSE,  "w" );
  witWriteData( scenarioPtr->lps().witRun(), "wit.data" );
  witWriteShipSched( scenarioPtr->lps().witRun(), WitSTDOUT, WitBSV );
  witDisplayData( scenarioPtr->lps().witRun(), WitSTDOUT );
  witSetMesgTimesPrint( scenarioPtr->lps().witRun(), 
     WitFALSE, WitINFORMATIONAL_MESSAGES, 0 );
#endif


  // display committed demand window
  // display the material and capacity windows with computed fss
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);
  if ( ! useFileData )
    matRecovMSP.printByItemAndAttribute(printStyle);
  if ( ! useFileData )
    capRecovMSP.printByItemAndAttribute(printStyle);


  // ----------------------------------------------------------------- 
  cout <<"Step 4.1:  Sort commited shipVol in period 5\n";
  cerr <<"Step 4.1:  Sort commited shipVol in period 5\n";
  // ----------------------------------------------------------------- 

  committedMSD.itemOrderSchedule(
                LGFRCommittedShipmentVolume,LGFRSortByDemandQtyInPeriod,
                4,FALSE);
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 4.2:  Sort material supplyVol by criticalness\n";
  cerr <<"Step 4.2:  Sort material supplyVol by criticalness\n";
  // ----------------------------------------------------------------- 

  matRecovMSP.itemOrderSchedule(
                LGFRMaterialSupplyVolume,LGFRSortByCriticality );
  if ( ! useFileData )
    matRecovMSP.printByItemAndAttribute(printStyle);

#if 0
  FOR TESTING SORT_BY_CRITICALITY
  witSetMesgTimesPrint( scenarioPtr->lps().witRun(), 
     WitFALSE, WitINFORMATIONAL_MESSAGES, UCHAR_MAX );
  witSetMesgFileAccessMode( scenarioPtr->lps().witRun(), WitFALSE,  "w" );
  witWriteCriticalList( scenarioPtr->lps().witRun(), WitSTDOUT, WitBSV,99999);
  witSetMesgTimesPrint( scenarioPtr->lps().witRun(), 
     WitFALSE, WitINFORMATIONAL_MESSAGES, 0 );
#endif

  // ----------------------------------------------------------------- 
  cout <<"Step 5:  Change from Committed to Forecast task\n";
  cerr <<"Step 5:  Change from Committed to Forecast tasK\n";
  // ----------------------------------------------------------------- 

  // User presses forecast task button
  //   This causes the recovery plan button to pop up and not be
  //   selectable (lit).
  scenarioPtr->changeState(LGFR_ATP_STATE_ATP_IMPLODED);

  // NOTE: the forecast task button should be down,
  //       the Implosion button should be lit,
  //       the recovery plan button should be unlit and up.
  assert( implodeButtonLit( scenarioPtr, FORECAST, FALSE ) );

  // Since the recovery plan button is no longer selected, the
  // material and capacity recovery plan windows are removed from
  // the display and can be deleted.
  delete matRecovMSPptr;
  delete capRecovMSPptr;

  // Display the forecast demands.
  if ( ! useFileData )
    forecastMSD.printByItemAndAttribute(printStyle);

  
  // ----------------------------------------------------------------- 
  cout <<"Step 5.1:  Switch back to display of committed orders\n";
  cerr <<"Step 5.1:  Switch back to display of committed orders\n";
  // ----------------------------------------------------------------- 
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 6:  Implode to see how much of forecast can be achieved\n";
  cerr <<"Step 6:  Implode to see how much of forecast can be achieved\n";
  // ----------------------------------------------------------------- 

  // User presses implode button
  implodeButtonPressed( scenarioPtr, FORECAST, FALSE );

  // NOTE: the forecast task button should be down,
  //       the Implosion button should be unlit,
  //       the recovery plan button should be unlit and up.
  assert( ! implodeButtonLit( scenarioPtr, FORECAST, FALSE ) );

  // Display ATP shipments.
  forecastMSD.printByItemAndAttribute(printStyle);
  
  // ----------------------------------------------------------------- 
  cout <<"Step 6.1:  Switch back to display of committed orders\n";
  cerr <<"Step 6.1:  Switch back to display of committed orders\n";
  // ----------------------------------------------------------------- 
  if ( ! useFileData )
    committedMSD.printByItemAndAttribute(printStyle);

  // ----------------------------------------------------------------- 
  cout <<"Step 7:  Change from Forecast to New Opp task\n";
  cerr <<"Step 7:  Change from Forecast to New Opp tasK\n";
  // ----------------------------------------------------------------- 

  // User presses NewOp task button
  scenarioPtr->changeState(LGFR_ATP_STATE_NEWOPP_IMPLODED);

  // NOTE: the NewOp task button should be down,
  //       the Implosion button should be lit,
  //       the recovery plan button should be unlit and up.
  assert( implodeButtonLit( scenarioPtr, NEWOP, FALSE ) );

  // Make sure new-opp multiSchedule is empty
  assert( newOpMSD.numberOfItems() == 0 );


  // ----------------------------------------------------------------- 
  cout <<"Step 8:  This just in ... A new NewOpp -- can we meet it\n";
  cerr <<"Step 8:  This just in ... A new NewOpp -- can we meet it\n";
  // ----------------------------------------------------------------- 

  // User presses Add a new NewOpp demand

  // Press/select  Add a new NewOpp demand
  //    This should induce a dialog box to appear asking for ...

  // Get sets of newOp demands
  const LgFrSetOfDemands & setOfNewOpDemands =
     scenarioPtr->constSetOfDemands( LGFR_NEWOPPOR_DEMANDS );

  // a demand name ...
# define NEW_DEMANDS_PART_NAME (useFileData ? "MC0250" : "Computer_4")
# define NEW_DEMAND_NAME "PotentialNewOp"
  RWCString newDemandName(NEW_DEMAND_NAME);

  // and, via a selection list,  a part.
  const LgFrPart * demandedPartPtr = scenarioPtr->
                   constSetOfParts(LGFR_MATERIAL_SET)
		     . findValue(NEW_DEMANDS_PART_NAME);
  assert (demandedPartPtr != NULL);

  // Dialog box closes and we issue the command to add the new demand
  RWBoolean didItWork = 
     scenarioPtr->newNewOppDemand(newDemandName, *demandedPartPtr);
  assert (didItWork == TRUE);

  // The implode button should now light up

  // NOTE: the newOp task button should be down,
  //       the Implosion button should be lit,
  //       the recovery plan button should be unlit and up.
  assert ( implodeButtonLit(scenarioPtr, NEWOP, FALSE) == TRUE);
  
  // The user should now edit the demandVol through the multiSched, as
  // done below.

  // Get demand associated with NEW_DEMAND_NAME
  const LgFrDemand * newDemandPtr = 
     setOfNewOpDemands.findValue(NEW_DEMAND_NAME, NEW_DEMANDS_PART_NAME);
  assert( newDemandPtr != NULL );
  LgFrDemand newDemand = *newDemandPtr;

  // Get the  demand vol (which should all be zero) and change to new demand
  // vol.  A newop for one of the out-going models comes in, exceeding the 
  // projected forcast -oh my.
  LgFrTimeVecFloat  newDemandVol =
     *(newOpMSD.getTimeVecFloat(newDemand, LGFRNewOpportunityDemandVolume));
  newDemandVol[3] = 150.;

  // Set the new new Demand vol in the schedule
  RWBoolean result;
  result = newOpMSD.changeTimeVecInSchedule(
                    newDemand, LGFRNewOpportunityDemandVolume, newDemandVol );
  assert( result );

  // User Now hits the implode button to see when the demand can be met
  implodeButtonPressed( scenarioPtr, NEWOP, FALSE );

  // The implode button should go out
  assert ( implodeButtonLit(scenarioPtr, NEWOP, FALSE) == FALSE);

  // Display new shipment solution
  newOpMSD.printByItemAndAttribute(CSV /*printStyle*/);
  

  // ATP Application terminating, so free new'ed storage.
  delete newOpMSDptr;
  delete forecastMSDptr;
  delete committedMSDptr;

  // Release smart pointers to schedules in the schedule factory so that the
  // scenario can be deleted when the current block exists.
  ( (LgFrScheduleFactory *) (scenarioPtr->scheduleFactory()) ) // Cast away const
    -> clearAndDestroy();

  cerr << "All tests completed successfully\n";
  cout << "All tests completed successfully\n";
  return 0;
}
