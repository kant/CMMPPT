//  LAST EDIT: Tue Nov 27 16:30:04 CET 2001 by Laszlo Ladanyi
//-----------------------------------------------------------------------------
// name:     OSI Interface for CPLEX
// author:   Tobias Pfender
//           Konrad-Zuse-Zentrum Berlin (Germany)
//           email: pfender@zib.de
// date:     09/25/2000
// comments: please scan this file for '???' and read the comments
//-----------------------------------------------------------------------------
// Copyright (C) 2000, Tobias Pfender, International Business Machines
// Corporation and others.  All Rights Reserved.

#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif

#include <iostream>
#include <cassert>
#include <string>
#include <numeric>

#include "CoinError.hpp"

#include "OsiCpxSolverInterface.hpp"
#include "OsiRowCut.hpp"
#include "OsiColCut.hpp"
#include "CoinPackedMatrix.hpp"
#include "CoinWarmStartBasis.hpp"


// #define DEBUG 1

#ifdef DEBUG
#define debugMessage printf
#else
#define debugMessage if( false ) printf
#endif


//#############################################################################
// A couple of helper functions
//#############################################################################

inline void freeCacheDouble( double*& ptr )
{
  if( ptr != NULL )
    {
      delete [] ptr;
      ptr = NULL;
    }
}

inline void freeCacheChar( char*& ptr )
{
  if( ptr != NULL )
    {
      delete [] ptr;
      ptr = NULL;
    }
}

inline void freeCacheMatrix( CoinPackedMatrix*& ptr )
{
  if( ptr != NULL )
    {
      delete ptr;
      ptr = NULL;
    }
}

static inline void
checkCPXerror( int err, std::string cpxfuncname, std::string osimethod )
{
  if( err != 0 )
    {
      char s[100];
      sprintf( s, "%s returned error %d", cpxfuncname.c_str(), err );
      std::cout << "ERROR: " << s << " (" << osimethod << 
	" in OsiCpxSolverInterface)" << std::endl;
      throw CoinError( s, osimethod.c_str(), "OsiCpxSolverInterface" );
    }
}

void
OsiCpxSolverInterface::switchToLP( void )
{
  debugMessage("OsiCpxSolverInterface::switchToLP()\n");

  if( probtypemip_ )
  {
     CPXLPptr lp = getMutableLpPtr();

#if CPX_VERSION >= 800
     assert(CPXgetprobtype(env_,lp) == CPXPROB_MILP);
#else
     assert(CPXgetprobtype(env_,lp) == CPXPROB_MIP);
#endif

     int err = CPXchgprobtype( env_, lp, CPXPROB_LP );
     checkCPXerror( err, "CPXchgprobtype", "switchToLP" );
     probtypemip_ = false;
  }
}

void
OsiCpxSolverInterface::switchToMIP( void )
{
  debugMessage("OsiCpxSolverInterface::switchToMIP()\n");

  if( !probtypemip_ )
  {
     CPXLPptr lp = getMutableLpPtr();
     int nc = getNumCols();
     int *cindarray = new int[nc];

     assert(CPXgetprobtype(env_,lp) == CPXPROB_LP);
     assert(coltype_ != NULL);

     for( int i = 0; i < nc; ++i )
        cindarray[i] = i;

#if CPX_VERSION >= 800
     int err = CPXchgprobtype( env_, lp, CPXPROB_MILP );
#else
     int err = CPXchgprobtype( env_, lp, CPXPROB_MIP );
#endif
     checkCPXerror( err, "CPXchgprobtype", "switchToMIP" );

     err = CPXchgctype( env_, lp, nc, cindarray, coltype_ );
     checkCPXerror( err, "CPXchgctype", "switchToMIP" );

     delete[] cindarray;
     probtypemip_ = true;
  }
}

void
OsiCpxSolverInterface::resizeColType( int minsize )
{
  debugMessage("OsiCpxSolverInterface::resizeColType()\n");

  if( minsize > coltypesize_ )
  {
     int newcoltypesize = 2*coltypesize_;
     if( minsize > newcoltypesize )
        newcoltypesize = minsize;
     char *newcoltype = new char[newcoltypesize];

     if( coltype_ != NULL )
     {
        CoinDisjointCopyN( coltype_, coltypesize_, newcoltype );
        delete[] coltype_;
     }
     coltype_ = newcoltype;
     coltypesize_ = newcoltypesize;
  }
  assert(minsize == 0 || coltype_ != NULL);
  assert(coltypesize_ >= minsize);
}

void
OsiCpxSolverInterface::freeColType()
{
  debugMessage("OsiCpxSolverInterface::freeColType()\n");

   if( coltypesize_ > 0 )
   {
      delete[] coltype_;
      coltype_ = NULL;
      coltypesize_ = 0;
   }
   assert(coltype_ == NULL);
}


//#############################################################################
// Solve methods
//#############################################################################

void OsiCpxSolverInterface::initialSolve()
{
  debugMessage("OsiCpxSolverInterface::initialSolve()\n");

  switchToLP();

  CPXLPptr lp = getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS );

  int term = CPXprimopt( env_, lp );

  /* If the problem is found infeasible during presolve, resolve it to get a 
     proper term code */
  if (term == CPXERR_PRESLV_INForUNBD){
    CPXsetintparam( env_, CPX_PARAM_PREIND, CPX_OFF );
    CPXprimopt( env_, lp );
    CPXsetintparam( env_, CPX_PARAM_PREIND, CPX_ON );
  }
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::resolve()
{
  debugMessage("OsiCpxSolverInterface::resolve()\n");

  switchToLP();

  CPXLPptr lp = getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS );

  int term = CPXdualopt( env_, lp );   

  /* If the problem is found infeasible during presolve, resolve it to get a 
     proper term code */
  if (term == CPXERR_PRESLV_INForUNBD){
    CPXsetintparam( env_, CPX_PARAM_PREIND, CPX_OFF );
    CPXdualopt( env_, lp );
    CPXsetintparam( env_, CPX_PARAM_PREIND, CPX_ON );
  }
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::branchAndBound()
{
  debugMessage("OsiCpxSolverInterface::branchAndBound()\n");

  switchToMIP();

  CPXLPptr lp = getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS );

  CPXmipopt( env_, lp );
}

//#############################################################################
// Parameter related methods
//#############################################################################

bool
OsiCpxSolverInterface::setIntParam(OsiIntParam key, int value)
{
  debugMessage("OsiCpxSolverInterface::setIntParam(%d, %d)\n", key, value);

  bool retval = false;
  switch (key)
    {
    case OsiMaxNumIteration:
      retval = ( CPXsetintparam( env_, CPX_PARAM_ITLIM, value ) == 0 );  // ??? OsiMaxNumIteration == #Simplex-iterations ???
      break;
    case OsiMaxNumIterationHotStart:
      if( value >= 0 )
	{
	  hotStartMaxIteration_ = value;
	  retval = true;
	}
      else
	retval = false;
      break;
    case OsiLastIntParam:
      retval = false;
      break;
    }
  return retval;
}

//-----------------------------------------------------------------------------

bool
OsiCpxSolverInterface::setDblParam(OsiDblParam key, double value)
{
  debugMessage("OsiCpxSolverInterface::setDblParam(%d, %g)\n", key, value);

  bool retval = false;
  switch (key)
    {
    case OsiDualObjectiveLimit:
      if( getObjSense() == +1 )
	retval = ( CPXsetdblparam( env_, CPX_PARAM_OBJULIM, value ) == 0 ); // min
      else
	retval = ( CPXsetdblparam( env_, CPX_PARAM_OBJLLIM, value ) == 0 ); // max
      break;
    case OsiPrimalObjectiveLimit:
      if( getObjSense() == +1 )
	retval = ( CPXsetdblparam( env_, CPX_PARAM_OBJLLIM, value ) == 0 ); // min
      else
	retval = ( CPXsetdblparam( env_, CPX_PARAM_OBJULIM, value ) == 0 ); // max
      break;
    case OsiDualTolerance:
      retval = ( CPXsetdblparam( env_, CPX_PARAM_EPOPT, value ) == 0 ); // ??? OsiDualTolerance == CPLEX Optimality tolerance ???
      break;
    case OsiPrimalTolerance:
      retval = ( CPXsetdblparam( env_, CPX_PARAM_EPRHS, value ) == 0 ); // ??? OsiPrimalTolerance == CPLEX Feasibility tolerance ???
      break;
    case OsiObjOffset:
      retval = OsiSolverInterface::setDblParam(key,value);
      break;
    case OsiLastDblParam:
      retval = false;
      break;
    }
  return retval;
}


//-----------------------------------------------------------------------------

bool
OsiCpxSolverInterface::setStrParam(OsiStrParam key, const std::string & value)
{
  debugMessage("OsiCpxSolverInterface::setStrParam(%d, %s)\n", key, value.c_str());

  bool retval=false;
  switch (key) {
  case OsiProbName:
    OsiSolverInterface::setStrParam(key,value);
    return retval = true;
  case OsiSolverName:
    return false;
  case OsiLastStrParam:
    return false;
  }
  return false;
}

//-----------------------------------------------------------------------------

bool
OsiCpxSolverInterface::getIntParam(OsiIntParam key, int& value) const
{
  debugMessage("OsiCpxSolverInterface::getIntParam(%d)\n", key);

  bool retval = false;
  switch (key)
    {
    case OsiMaxNumIteration:
      retval = ( CPXgetintparam( env_, CPX_PARAM_ITLIM, &value ) == 0 );  // ??? OsiMaxNumIteration == #Simplex-iterations ???
      break;
    case OsiMaxNumIterationHotStart:
      value = hotStartMaxIteration_;
      retval = true;
      break;
    case OsiLastIntParam:
      retval = false;
      break;
    }
  return retval;
}

//-----------------------------------------------------------------------------

bool
OsiCpxSolverInterface::getDblParam(OsiDblParam key, double& value) const
{
  debugMessage("OsiCpxSolverInterface::getDblParam(%d)\n", key);

  bool retval = false;
  switch (key) 
    {
    case OsiDualObjectiveLimit:
      if( getObjSense() == +1 )
	retval = ( CPXgetdblparam( env_, CPX_PARAM_OBJULIM, &value ) == 0 ); // min
      else
	retval = ( CPXgetdblparam( env_, CPX_PARAM_OBJLLIM, &value ) == 0 ); // max
      break;
    case OsiPrimalObjectiveLimit:
      if( getObjSense() == +1 )
	retval = ( CPXgetdblparam( env_, CPX_PARAM_OBJLLIM, &value ) == 0 ); // min
      else
	retval = ( CPXgetdblparam( env_, CPX_PARAM_OBJULIM, &value ) == 0 ); // max
      break;
    case OsiDualTolerance:
      retval = ( CPXgetdblparam( env_, CPX_PARAM_EPOPT, &value ) == 0 ); // ??? OsiDualTolerance == CPLEX Optimality tolerance ???
      break;
    case OsiPrimalTolerance:
      retval = ( CPXgetdblparam( env_, CPX_PARAM_EPRHS, &value ) == 0 ); // ??? OsiPrimalTolerance == CPLEX Feasibility tolerance ???
      break;
    case OsiObjOffset:
      retval = OsiSolverInterface::getDblParam(key, value);
      break;
    case OsiLastDblParam:
      retval = false;
      break;
    }
  return retval;
}


//-----------------------------------------------------------------------------

bool
OsiCpxSolverInterface::getStrParam(OsiStrParam key, std::string & value) const
{
  debugMessage("OsiCpxSolverInterface::getStrParam(%d)\n", key);

  switch (key) {
  case OsiProbName:
    OsiSolverInterface::getStrParam(key, value);
    break;
  case OsiSolverName:
    value = "cplex";
    break;
  case OsiLastStrParam:
    return false;
  }

  return true;
}

//#############################################################################
// Methods returning info on how the solution process terminated
//#############################################################################

bool OsiCpxSolverInterface::isAbandoned() const
{
  debugMessage("OsiCpxSolverInterface::isAbandoned()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  return (stat == 0 || 
	  stat == CPX_STAT_NUM_BEST || 
	  stat == CPX_STAT_NUM_BEST || 
	  stat == CPX_STAT_ABORT_USER || 
	  stat == CPX_STAT_ABORT_USER || 
	  stat == CPX_STAT_ABORT_USER);
#else
  return (stat == 0 || 
	  stat == CPX_NUM_BEST_FEAS || 
	  stat == CPX_NUM_BEST_INFEAS || 
	  stat == CPX_ABORT_FEAS || 
	  stat == CPX_ABORT_INFEAS || 
	  stat == CPX_ABORT_CROSSOVER);
#endif
}

bool OsiCpxSolverInterface::isProvenOptimal() const
{
  debugMessage("OsiCpxSolverInterface::isProvenOptimal()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  return ((probtypemip_ == false && 
	  (stat == CPX_STAT_OPTIMAL || stat == CPX_STAT_OPTIMAL_INFEAS)) ||
	  probtypemip_ == true && stat == CPXMIP_OPTIMAL);
#else
  return ((probtypemip_ == false && 
	  (stat == CPX_OPTIMAL || stat == CPX_OPTIMAL_INFEAS)) ||
	  probtypemip_ == true && stat == CPXMIP_OPTIMAL);
#endif
}

bool OsiCpxSolverInterface::isProvenPrimalInfeasible() const
{
  debugMessage("OsiCpxSolverInterface::isProvenPrimalInfeasible()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  // In CPLEX 8, the return code is with respect
  // to the original problem, regardless of the algorithm used to solve it
  // --tkr 7/31/03
  return (stat == CPX_STAT_INFEASIBLE);
  //  return (method == CPX_ALG_PRIMAL && stat == CPX_STAT_INFEASIBLE || 
  //  method == CPX_ALG_DUAL && stat == CPX_STAT_UNBOUNDED);
#else

  int method = CPXgetmethod( env_, getMutableLpPtr() );

  return (method == CPX_ALG_PRIMAL && stat == CPX_INFEASIBLE || 
	  method == CPX_ALG_DUAL && stat == CPX_UNBOUNDED || 
	  stat == CPX_ABORT_PRIM_INFEAS ||
	  stat == CPX_ABORT_PRIM_DUAL_INFEAS);
#endif
}

bool OsiCpxSolverInterface::isProvenDualInfeasible() const
{
  debugMessage("OsiCpxSolverInterface::isProvenDualInfeasible()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  // In CPLEX 8, the return code is with respect
  // to the original problem, regardless of the algorithm used to solve it
  // --tkr 7/31/03
  return (stat == CPX_STAT_UNBOUNDED);
  //return (method == CPX_ALG_PRIMAL && stat == CPX_STAT_UNBOUNDED || 
  //	  method == CPX_ALG_DUAL && stat == CPX_STAT_INFEASIBLE);
#else

  int method = CPXgetmethod( env_, getMutableLpPtr() );

  return (method == CPX_ALG_PRIMAL && stat == CPX_UNBOUNDED || 
	  method == CPX_ALG_DUAL && stat == CPX_INFEASIBLE || 
	  stat == CPX_ABORT_DUAL_INFEAS || 
	  stat == CPX_ABORT_PRIM_DUAL_INFEAS);
#endif
}

bool OsiCpxSolverInterface::isPrimalObjectiveLimitReached() const
{
  debugMessage("OsiCpxSolverInterface::isPrimalObjectiveLimitReached()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );
  int method = CPXgetmethod( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  return method == CPX_ALG_PRIMAL && stat == CPX_STAT_ABORT_OBJ_LIM;
#else
  return method == CPX_ALG_PRIMAL && stat == CPX_OBJ_LIM;
#endif
}

bool OsiCpxSolverInterface::isDualObjectiveLimitReached() const
{
  debugMessage("OsiCpxSolverInterface::isDualObjectiveLimitReached()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );
  int method = CPXgetmethod( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  return method == CPX_ALG_DUAL && stat == CPX_STAT_ABORT_OBJ_LIM;
#else
  return method == CPX_ALG_DUAL && stat == CPX_OBJ_LIM;
#endif
}

bool OsiCpxSolverInterface::isIterationLimitReached() const
{
  debugMessage("OsiCpxSolverInterface::isIterationLimitReached()\n");

  int stat = CPXgetstat( env_, getMutableLpPtr() );

#if CPX_VERSION >= 800
  return stat == CPX_STAT_ABORT_IT_LIM;
#else
  return stat == CPX_IT_LIM_FEAS || stat == CPX_IT_LIM_INFEAS;
#endif
}

//#############################################################################
// WarmStart related methods
//#############################################################################

CoinWarmStart* OsiCpxSolverInterface::getWarmStart() const
{
  debugMessage("OsiCpxSolverInterface::getWarmStart()\n");

  CoinWarmStartBasis* ws = NULL;
  int numcols = getNumCols();
  int numrows = getNumRows();
  int *cstat = new int[numcols];
  int *rstat = new int[numrows];
  int restat, i;

  assert(!probtypemip_);

  restat = CPXgetbase( env_, getMutableLpPtr(), cstat, rstat );
  if( restat == 0 )
    {
      ws = new CoinWarmStartBasis;
      ws->setSize( numcols, numrows );
      
      for( i = 0; i < numrows; ++i )
	{
	  switch( rstat[i] )
	    {
	    case CPX_BASIC:
	      ws->setArtifStatus( i, CoinWarmStartBasis::basic );
	      break;
	    case CPX_AT_LOWER:
	      ws->setArtifStatus( i, CoinWarmStartBasis::atLowerBound );
	      break;
	    case CPX_AT_UPPER:
	      ws->setArtifStatus( i, CoinWarmStartBasis::atUpperBound );
	      break;
	    default:  // unknown row status
	      delete ws;
	      ws = NULL;
	      goto TERMINATE;
	    }
	}
      for( i = 0; i < numcols; ++i )
	{
	  switch( cstat[i] )
	    {
	    case CPX_BASIC:
	      ws->setStructStatus( i, CoinWarmStartBasis::basic );
	      break;
	    case CPX_AT_LOWER:
	      ws->setStructStatus( i, CoinWarmStartBasis::atLowerBound );
	      break;
	    case CPX_AT_UPPER:
	      ws->setStructStatus( i, CoinWarmStartBasis::atUpperBound );
	      break;
	    case CPX_FREE_SUPER:
	      ws->setStructStatus( i, CoinWarmStartBasis::isFree );
	      break;
	    default:  // unknown column status
	      delete ws;
	      ws = NULL;
	      goto TERMINATE;
	    }
	}
    }
 TERMINATE:
  delete[] cstat;
  delete[] rstat;

  return ws;
}

//-----------------------------------------------------------------------------

bool OsiCpxSolverInterface::setWarmStart(const CoinWarmStart* warmstart)
{
  debugMessage("OsiCpxSolverInterface::setWarmStart(%p)\n", warmstart);

  const CoinWarmStartBasis* ws = dynamic_cast<const CoinWarmStartBasis*>(warmstart);
  int numcols, numrows, i, restat;
  int *cstat, *rstat;
  bool retval = false;

  if( !ws )
    return false;

  numcols = ws->getNumStructural();
  numrows = ws->getNumArtificial();
  
  if( numcols != getNumCols() || numrows != getNumRows() )
    return false;

  switchToLP();

  cstat = new int[numcols];
  rstat = new int[numrows];
  for( i = 0; i < numrows; ++i )
    {
      switch( ws->getArtifStatus( i ) )
	{
	case CoinWarmStartBasis::basic:
	  rstat[i] = CPX_BASIC;
	  break;
	case CoinWarmStartBasis::atLowerBound:
	  rstat[i] = CPX_AT_LOWER;
	  break;
	case CoinWarmStartBasis::atUpperBound:
	  rstat[i] = CPX_AT_UPPER;
	  break;
	default:  // unknown row status
	  retval = false;
	  goto TERMINATE;
	}
    }
  for( i = 0; i < numcols; ++i )
    {
      switch( ws->getStructStatus( i ) )
	{
	case CoinWarmStartBasis::basic:
	  cstat[i] = CPX_BASIC;
	  break;
	case CoinWarmStartBasis::atLowerBound:
	  cstat[i] = CPX_AT_LOWER;
	  break;
	case CoinWarmStartBasis::atUpperBound:
	  cstat[i] = CPX_AT_UPPER;
	  break;
	case CoinWarmStartBasis::isFree:
	  cstat[i] = CPX_FREE_SUPER;
	  break;
	default:  // unknown row status
	  retval = false;
	  goto TERMINATE;
	}
    }

  // *FIXME* : can this be getMutableLpPtr() ? Does any cached data change by
  // *FIXME* : setting warmstart? Or at least wouldn't it be sufficient to
  // *FIXME* : clear the cached results but not the problem data?
  // -> is fixed by using FREECACHED_RESULTS; only cached solution will be discarded
  restat = CPXcopybase( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS ), cstat, rstat );
  retval = (restat == 0);
 TERMINATE:
  delete[] cstat;
  delete[] rstat;
  return retval;
}

//#############################################################################
// Hotstart related methods (primarily used in strong branching)
//#############################################################################

void OsiCpxSolverInterface::markHotStart()
{
  debugMessage("OsiCpxSolverInterface::markHotStart()\n");

  int err;
  int numcols, numrows;

  assert(!probtypemip_);

  numcols = getNumCols();
  numrows = getNumRows();
  if( numcols > hotStartCStatSize_ )
    {
      delete[] hotStartCStat_;
      hotStartCStatSize_ = static_cast<int>( 1.2 * static_cast<double>( numcols ) ); // get some extra space for future hot starts
      hotStartCStat_ = new int[hotStartCStatSize_];
    }
  if( numrows > hotStartRStatSize_ )
    {
      delete[] hotStartRStat_;
      hotStartRStatSize_ = static_cast<int>( 1.2 * static_cast<double>( numrows ) ); // get some extra space for future hot starts
      hotStartRStat_ = new int[hotStartRStatSize_];
    }
  err = CPXgetbase( env_, getMutableLpPtr(), hotStartCStat_, hotStartRStat_ );
  checkCPXerror( err, "CPXgetbase", "markHotStart" );
}

void OsiCpxSolverInterface::solveFromHotStart()
{
  debugMessage("OsiCpxSolverInterface::solveFromHotStart()\n");

  int err;
  int maxiter;

  switchToLP();

  assert( getNumCols() <= hotStartCStatSize_ );
  assert( getNumRows() <= hotStartRStatSize_ );
  err = CPXcopybase( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS ), hotStartCStat_, hotStartRStat_ );
  checkCPXerror( err, "CPXcopybase", "solveFromHotStart" );

  err = CPXgetintparam( env_, CPX_PARAM_ITLIM, &maxiter );
  checkCPXerror( err, "CPXgetintparam", "solveFromHotStart" );
  err = CPXsetintparam( env_, CPX_PARAM_ITLIM, hotStartMaxIteration_ );
  checkCPXerror( err, "CPXsetintparam", "solveFromHotStart" );
  
  resolve();

  err = CPXsetintparam( env_, CPX_PARAM_ITLIM, maxiter );
  checkCPXerror( err, "CPXsetintparam", "solveFromHotStart" );
}

void OsiCpxSolverInterface::unmarkHotStart()
{
  debugMessage("OsiCpxSolverInterface::unmarkHotStart()\n");

  // ??? be lazy with deallocating memory and do nothing here, deallocate memory in the destructor
}

//#############################################################################
// Problem information methods (original data)
//#############################################################################

//------------------------------------------------------------------
// Get number of rows, columns, elements, ...
//------------------------------------------------------------------
int OsiCpxSolverInterface::getNumCols() const
{
  debugMessage("OsiCpxSolverInterface::getNumCols()\n");

  return CPXgetnumcols( env_, getMutableLpPtr() );
}
int OsiCpxSolverInterface::getNumRows() const
{
  debugMessage("OsiCpxSolverInterface::getNumRows()\n");

  return CPXgetnumrows( env_, getMutableLpPtr() );
}
int OsiCpxSolverInterface::getNumElements() const
{
  debugMessage("OsiCpxSolverInterface::getNumElements()\n");

  return CPXgetnumnz( env_, getMutableLpPtr() );
}

//------------------------------------------------------------------
// Get pointer to rim vectors
//------------------------------------------------------------------  

const double * OsiCpxSolverInterface::getColLower() const
{
  debugMessage("OsiCpxSolverInterface::getColLower()\n");

  if( collower_ == NULL )
    {
      int ncols = CPXgetnumcols( env_, getMutableLpPtr() );
      if( ncols > 0 )
	{
	  collower_ = new double[ncols];
	  CPXgetlb( env_, getMutableLpPtr(), collower_, 0, ncols-1 );
	}
    }
  return collower_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getColUpper() const
{
  debugMessage("OsiCpxSolverInterface::getColUpper()\n");

  if( colupper_ == NULL )
    {
      int ncols = CPXgetnumcols( env_, getMutableLpPtr() );
      if( ncols > 0 )
	{
	  colupper_ = new double[ncols];
	  CPXgetub( env_, getMutableLpPtr(), colupper_, 0, ncols-1 );
	}
    }
  return colupper_;
}
//------------------------------------------------------------------
const char * OsiCpxSolverInterface::getRowSense() const
{
  debugMessage("OsiCpxSolverInterface::getRowSense()\n");

  if ( rowsense_==NULL )
    {      
      // rowsense is determined with rhs, so invoke rhs
      getRightHandSide();
      assert( rowsense_!=NULL || getNumRows() == 0 );
    }
  return rowsense_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getRightHandSide() const
{
  debugMessage("OsiCpxSolverInterface::getRightHandSide()\n");

  if ( rhs_==NULL )
    {
      CPXLPptr lp = getMutableLpPtr();
      int nrows = getNumRows();
      if( nrows > 0 ) 
	{
	  rhs_ = new double[nrows];
	  CPXgetrhs( env_, lp, rhs_, 0, nrows-1 );
	  
	  assert( rowrange_ == NULL );
	  rowrange_ = new double[nrows];
	  CPXgetrngval( env_, lp, rowrange_, 0, nrows-1 );
	  
	  assert( rowsense_ == NULL );
	  rowsense_ = new char[nrows];
	  CPXgetsense( env_, lp, rowsense_, 0, nrows-1 );
	  
	  double inf = getInfinity();
	  int i;
	  for ( i = 0; i < nrows; ++i ) 
	    {  
	      if ( rowsense_[i] != 'R' ) 
		rowrange_[i]=0.0;
	      else
		{
		  if ( rhs_[i] <= -inf ) 
		    {
		      rowsense_[i] = 'N';
		      rowrange_[i] = 0.0;
		      rhs_[i] = 0.0;
		    } 
		  else 
		    {
		      if( rowrange_[i] >= 0.0 )
			rhs_[i] = rhs_[i] + rowrange_[i];
		      else
			rowrange_[i] = -rowrange_[i];
		    }
		}
	    }
	}
    }
  return rhs_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getRowRange() const
{
  debugMessage("OsiCpxSolverInterface::getRowRange()\n");

  if ( rowrange_==NULL ) 
    {
      // rowrange is determined with rhs, so invoke rhs
      getRightHandSide();
      assert( rowrange_!=NULL || getNumRows() == 0 );
    }
  return rowrange_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getRowLower() const
{
  debugMessage("OsiCpxSolverInterface::getRowLower()\n");

  if ( rowlower_ == NULL )
    {
      int     nrows = getNumRows();
      const   char    *rowsense = getRowSense();
      const   double  *rhs      = getRightHandSide();
      const   double  *rowrange = getRowRange();
    
      if ( nrows > 0 )
	{
	  rowlower_ = new double[nrows];
	  
	  double dum1;
	  for ( int i = 0;  i < nrows;  i++ )
	    convertSenseToBound( rowsense[i], rhs[i], rowrange[i],
				 rowlower_[i], dum1 );
	}
    }
  return rowlower_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getRowUpper() const
{  
  debugMessage("OsiCpxSolverInterface::getRowUpper()\n");

  if ( rowupper_ == NULL )
    {
      int     nrows = getNumRows();
      const   char    *rowsense = getRowSense();
      const   double  *rhs      = getRightHandSide();
      const   double  *rowrange = getRowRange();
      
      if ( nrows > 0 ) 
	{
	  rowupper_ = new double[nrows];
	  
	  double dum1;
	  for ( int i = 0;  i < nrows;  i++ )
	    convertSenseToBound( rowsense[i], rhs[i], rowrange[i],
				 dum1, rowupper_[i] );
	}
    }
  
  return rowupper_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getObjCoefficients() const
{
  debugMessage("OsiCpxSolverInterface::getObjCoefficients()\n");

  if ( obj_==NULL )
    {
      int ncols = CPXgetnumcols( env_, getMutableLpPtr() );
      if( ncols > 0 )
	{
	  obj_ = new double[ncols];
	  int err = CPXgetobj( env_, getMutableLpPtr(), obj_, 0, ncols-1 );
	  checkCPXerror( err, "CPXgetobj", "getObjCoefficients" );
	}
    }
  return obj_;
}
//------------------------------------------------------------------
double OsiCpxSolverInterface::getObjSense() const
{
  debugMessage("OsiCpxSolverInterface::getObjSense()\n");

  if( CPXgetobjsen( env_, getMutableLpPtr() ) == CPX_MIN )
    return +1.0;
  else
    return -1.0;
}

//------------------------------------------------------------------
// Return information on integrality
//------------------------------------------------------------------

bool OsiCpxSolverInterface::isContinuous( int colNumber ) const
{
  debugMessage("OsiCpxSolverInterface::isContinuous(%d)\n", colNumber);

  return getCtype()[colNumber] == CPX_CONTINUOUS;
}

//------------------------------------------------------------------
// Row and column copies of the matrix ...
//------------------------------------------------------------------

const CoinPackedMatrix * OsiCpxSolverInterface::getMatrixByRow() const
{
  debugMessage("OsiCpxSolverInterface::getMatrixByRow()\n");

  if ( matrixByRow_ == NULL ) 
    {
      int nrows = getNumRows();
      int ncols = getNumCols();
      int nelems;
      int *starts   = new int   [nrows + 1];
      int *len      = new int   [nrows];
      
      int requiredSpace;
      int rc = CPXgetrows( env_, getMutableLpPtr(), 
			   &nelems, starts, NULL, NULL, 0, &requiredSpace,
			   0, nrows-1 );
      
      assert( -requiredSpace == getNumElements() );
      int     *indices  = new int   [-requiredSpace];
      double  *elements = new double[-requiredSpace]; 
      
      rc = CPXgetrows( env_, getMutableLpPtr(), 
		       &nelems, starts, indices, elements, -requiredSpace,
		       &requiredSpace, 0, nrows-1 );
      assert( requiredSpace == 0 );
            
      matrixByRow_ = new CoinPackedMatrix();
      
      // Should be able to pass null for length of packed matrix,
      // assignMatrix does not seem to allow (even though documentation
      // say it is possible to do this). 
      // For now compute the length.
      starts[nrows] = nelems;
      for ( int i = 0; i < nrows; ++i )
	len[i]=starts[i+1] - starts[i];
      
      matrixByRow_->assignMatrix( false /* not column ordered */,
				  ncols, nrows, nelems,
				  elements, indices, starts, len /*NULL*/);
      
    }
  return matrixByRow_;
} 

//------------------------------------------------------------------

const CoinPackedMatrix * OsiCpxSolverInterface::getMatrixByCol() const
{
  debugMessage("OsiCpxSolverInterface::getMatrixByCol()\n");

  if ( matrixByCol_ == NULL )
    {
      int nrows = getNumRows();
      int ncols = getNumCols();
      int nelems;
      int *starts = new int   [ncols + 1];
      int *len    = new int   [ncols];
      
      int requiredSpace;
      int rc = CPXgetcols( env_, getMutableLpPtr(), 
			   &nelems, starts, NULL, NULL, 0, &requiredSpace,
			   0, ncols-1 );
      assert( -requiredSpace == getNumElements() );
      
      int     *indices  = new int   [-requiredSpace];
      double  *elements = new double[-requiredSpace]; 
      
      rc = CPXgetcols( env_, getMutableLpPtr(), 
		       &nelems, starts, indices, elements, -requiredSpace,
		       &requiredSpace, 0, ncols-1 );
      assert( requiredSpace == 0);
      
      matrixByCol_ = new CoinPackedMatrix();
      
      // Should be able to pass null for length of packed matrix,
      // assignMatrix does not seem to allow (even though documentation
      // say it is possible to do this). 
      // For now compute the length.
      starts[ncols] = nelems;
      for ( int i = 0; i < ncols; i++ )
	len[i]=starts[i+1] - starts[i];
      
      matrixByCol_->assignMatrix( true /* column ordered */,
				  nrows, ncols, nelems,
				  elements, indices, starts, len /*NULL*/);
      assert( matrixByCol_->getNumCols()==ncols );
      assert( matrixByCol_->getNumRows()==nrows );
    }
  return matrixByCol_;
} 

//------------------------------------------------------------------
// Get solver's value for infinity
//------------------------------------------------------------------
double OsiCpxSolverInterface::getInfinity() const
{
  debugMessage("OsiCpxSolverInterface::getInfinity()\n");

  return CPX_INFBOUND;
}

//#############################################################################
// Problem information methods (results)
//#############################################################################

// *FIXME*: what should be done if a certain vector doesn't exist???

const double * OsiCpxSolverInterface::getColSolution() const
{
  debugMessage("OsiCpxSolverInterface::getColSolution()\n");

  if( colsol_==NULL )
    {
      CPXLPptr lp = getMutableLpPtr();
      int ncols = CPXgetnumcols( env_, lp );
      if( ncols > 0 )
	{
	  colsol_ = new double[ncols]; 

          if( probtypemip_ ) {
	    int err = CPXgetmipx( env_, lp, colsol_, 0, ncols-1 );
	    if ( err == CPXERR_NO_INT_SOLN ) 
	      CoinFillN( colsol_, ncols, 0.0 );
	    else
	      checkCPXerror( err, "CPXgetmipx", "getColSolution" );
	  } else {
	    int err = CPXgetx( env_, lp, colsol_, 0, ncols-1 );
	    if ( err == CPXERR_NO_SOLN ) 
	      CoinFillN( colsol_, ncols, 0.0 );
	    else
	      checkCPXerror( err, "CPXgetx", "getColSolution" );
	  }
	}
    }
  return colsol_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getRowPrice() const
{
  debugMessage("OsiCpxSolverInterface::getRowPrice()\n");

  if( rowsol_==NULL )
    {
      int nrows = getNumRows();
      if( nrows > 0 )
	{
	  rowsol_ = new double[nrows];
	  int err = CPXgetpi( env_, getMutableLpPtr(), rowsol_, 0, nrows-1 );
	  if ( err == CPXERR_NO_SOLN ) 
	    CoinFillN( rowsol_, nrows, 0.0 );
	  else
	    checkCPXerror( err, "CPXgetpi", "getRowPrice" );
	}
    }
  return rowsol_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getReducedCost() const
{
  debugMessage("OsiCpxSolverInterface::getReducedCost()\n");

  if( redcost_==NULL )
    {
      int ncols = CPXgetnumcols( env_, getMutableLpPtr() );
      if( ncols > 0 )
	{
	  redcost_ = new double[ncols]; 
	  int err = CPXgetdj( env_, getMutableLpPtr(), redcost_, 0, ncols-1 );
	  if ( err == CPXERR_NO_SOLN ) 
	    CoinFillN( redcost_, ncols, 0.0 );
	  else
	    checkCPXerror( err, "CPXgetdj", "getReducedCost" );
	}
    }
  return redcost_;
}
//------------------------------------------------------------------
const double * OsiCpxSolverInterface::getRowActivity() const
{
  debugMessage("OsiCpxSolverInterface::getRowActivity()\n");

  if( rowact_==NULL )
    {
      int nrows = getNumRows();
      if( nrows > 0 )
	{
	  rowact_ = new double[nrows];
          if( probtypemip_ )
          {
             double *rowslack = new double[nrows];
             int err = CPXgetmipslack( env_, getMutableLpPtr(), rowslack, 0, nrows-1 );
             if ( err == CPXERR_NO_SOLN )
                CoinFillN( rowact_, nrows, 0.0 );
             else
             {
                // *FIXME* : this has to be tested for ranged rows
                checkCPXerror( err, "CPXgetmipslack", "getRowActivity" );
                for( int r = 0; r < nrows; ++r )
                   rowact_[r] = getRightHandSide()[r] - rowslack[r];
             }
             delete [] rowslack;
          }
          else
          {
             int err = CPXgetax( env_, getMutableLpPtr(), rowact_, 0, nrows-1 );
             if ( err == CPXERR_NO_SOLN )
                CoinFillN( rowact_, nrows, 0.0 );
             else
                checkCPXerror( err, "CPXgetax", "getRowActivity" );
          }
	}
    }
  return rowact_;
}
//------------------------------------------------------------------
double OsiCpxSolverInterface::getObjValue() const
{
  debugMessage("OsiCpxSolverInterface::getObjValue()\n");

  double objval = 0.0;
  int err;

  CPXLPptr lp = getMutableLpPtr();

  if( probtypemip_ ) {
    err = CPXgetmipobjval( env_, lp, &objval);
    if ( err == CPXERR_NO_INT_SOLN ) 
      // => return 0.0 as objective value (?? is this the correct behaviour ??)
      objval = 0.0;
    else
      checkCPXerror( err, "CPXgetmipobjval", "getObjValue" );
  } else {
    err = CPXgetobjval( env_, lp, &objval );
    if ( err == CPXERR_NO_SOLN ) 
      objval = 0.0;
    else
      checkCPXerror( err, "CPXgetobjval", "getObjValue" );
  }

  // Adjust objective function value by constant term in objective function
  double objOffset;
  getDblParam(OsiObjOffset,objOffset);
  objval = objval - objOffset;

  return objval;
}
//------------------------------------------------------------------
int OsiCpxSolverInterface::getIterationCount() const
{
  debugMessage("OsiCpxSolverInterface::getIterationCount()\n");

  return CPXgetitcnt( env_, getMutableLpPtr() );
}
//------------------------------------------------------------------
std::vector<double*> OsiCpxSolverInterface::getDualRays(int maxNumRays) const
{
  debugMessage("OsiCpxSolverInterface::getDualRays(%d)\n", maxNumRays);

   OsiCpxSolverInterface solver(*this);

   const int numcols = getNumCols();
   const int numrows = getNumRows();
   int* index = new int[CoinMax(numcols,numrows)];
   int i;
   for ( i = CoinMax(numcols,numrows)-1; i >= 0; --i) {
      index[i] = i;
   }
   double* obj = new double[CoinMax(numcols,2*numrows)];
   CoinFillN(obj, numcols, 0.0);
   solver.setObjCoeffSet(index, index+numcols, obj);

   double* clb = new double[2*numrows];
   double* cub = new double[2*numrows];

   const double plusone = 1.0;
   const double minusone = -1.0;
   const char* sense = getRowSense();

   const CoinPackedVectorBase** cols =
      new const CoinPackedVectorBase*[2*numrows];
   int newcols = 0;
   for (i = 0; i < numrows; ++i) {
      switch (sense[i]) {
      case 'L':
	 cols[newcols++] =
	    new CoinShallowPackedVector(1, &index[i], &minusone, false);
	 break;
      case 'G':
	 cols[newcols++] =
	    new CoinShallowPackedVector(1, &index[i], &plusone, false);
	 break;
      case 'R':
	 cols[newcols++] =
	    new CoinShallowPackedVector(1, &index[i], &minusone, false);
	 cols[newcols++] =
	    new CoinShallowPackedVector(1, &index[i], &plusone, false);
	 break;
      case 'N':
	 break;
      }
   }

   CoinFillN(obj, newcols, 1.0);
   CoinFillN(clb, newcols, 0.0);
   CoinFillN(cub, newcols, getInfinity());

   solver.addCols(newcols, cols, clb, cub, obj+numcols);
   delete[] index;
   delete[] cols;
   delete[] clb;
   delete[] cub;
   delete[] obj;

   solver.setObjSense(1.0); // minimize
   solver.initialSolve();

   const double* solverpi = getRowPrice();
   double* pi = new double[numrows];
   for ( i = numrows - 1; i >= 0; --i) {
      pi[i] = -solverpi[i];
   }
   return std::vector<double*>(1, pi);
}

//------------------------------------------------------------------
std::vector<double*> OsiCpxSolverInterface::getPrimalRays(int maxNumRays) const
{
  debugMessage("OsiCpxSolverInterface::getPrimalRays(%d)\n", maxNumRays);

  // *FIXME* : must write the method -LL
  throw CoinError("method is not yet written", "getPrimalRays",
		 "OsiCpxSolverInterface");
  return std::vector<double*>();
}

//#############################################################################
// Problem modifying methods (rim vectors)
//#############################################################################

void OsiCpxSolverInterface::setObjCoeff( int elementIndex, double elementValue )
{
  debugMessage("OsiCpxSolverInterface::setObjCoeff(%d, %g)\n", elementIndex, elementValue);

  int err = CPXchgobj(env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_COLUMN ), 1, &elementIndex, &elementValue);
  checkCPXerror(err, "CPXchgobj", "setObjCoeff");
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::setObjCoeffSet(const int* indexFirst,
					   const int* indexLast,
					   const double* coeffList)
{
  debugMessage("OsiCpxSolverInterface::setObjCoeffSet(%p, %p, %p)\n", indexFirst, indexLast, coeffList);

   const int cnt = indexLast - indexFirst;
   int err = CPXchgobj(env_,
		       getLpPtr(OsiCpxSolverInterface::FREECACHED_COLUMN), cnt,
		       const_cast<int*>(indexFirst),
		       const_cast<double*>(coeffList));
   checkCPXerror(err, "CPXchgobj", "setObjCoeffSet");
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::setColLower(int elementIndex, double elementValue)
{
  debugMessage("OsiCpxSolverInterface::setColLower(%d, %g)\n", elementIndex, elementValue);

  char c = 'L';
  int err = CPXchgbds( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_COLUMN ), 1, &elementIndex, &c, &elementValue );
  checkCPXerror( err, "CPXchgbds", "setColLower" );
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::setColUpper(int elementIndex, double elementValue)
{  
  debugMessage("OsiCpxSolverInterface::setColUpper(%d, %g)\n", elementIndex, elementValue);

  char c = 'U';
  int err = CPXchgbds( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_COLUMN ), 1, &elementIndex, &c, &elementValue );
  checkCPXerror( err, "CPXchgbds", "setColUpper" );
} 
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::setColBounds( int elementIndex, double lower, double upper )
{
  debugMessage("OsiCpxSolverInterface::setColBounds(%d, %g, %g)\n", elementIndex, lower, upper);

  char c[2] = { 'L', 'U' };
  int ind[2];
  double bd[2];
  int err;

  ind[0] = elementIndex;
  ind[1] = elementIndex;
  bd[0] = lower;
  bd[1] = upper;
  err = CPXchgbds( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_COLUMN ), 2, ind, c, bd );
  checkCPXerror( err, "CPXchgbds", "setColBounds" );
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::setColSetBounds(const int* indexFirst,
					    const int* indexLast,
					    const double* boundList)
{
  debugMessage("OsiCpxSolverInterface::setColSetBounds(%p, %p, %p)\n", indexFirst, indexLast, boundList);

   const int cnt = indexLast - indexFirst;
   if (cnt <= 0)
      return;

   char* c = new char[2*cnt];
   int* ind = new int[2*cnt];
   for (int i = 0; i < cnt; ++i) {
      register const int j = 2 * i;
      c[j] = 'L';
      c[j+1] = 'U';
      ind[j] = indexFirst[i];
      ind[j+1] = indexFirst[i];
   }
   int err = CPXchgbds( env_,
			getLpPtr(OsiCpxSolverInterface::FREECACHED_COLUMN),
			2*cnt, ind, c, const_cast<double*>(boundList) );
   checkCPXerror( err, "CPXchgbds", "setColSetBounds" );
   delete[] ind;
   delete[] c;
   // OsiSolverInterface::setColSetBounds( indexFirst, indexLast, boundList );
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setRowLower( int i, double elementValue )
{
  debugMessage("OsiCpxSolverInterface::setRowLower(%d, %g)\n", i, elementValue);

  double rhs   = getRightHandSide()[i];
  double range = getRowRange()[i];
  char   sense = getRowSense()[i];
  double lower, upper;

  convertSenseToBound( sense, rhs, range, lower, upper );
  if( lower != elementValue ) {
      convertBoundToSense( elementValue, upper, sense, rhs, range );
      setRowType( i, sense, rhs, range );
    }
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setRowUpper( int i, double elementValue )
{
  debugMessage("OsiCpxSolverInterface::setRowUpper(%d, %g)\n", i, elementValue);

  double rhs   = getRightHandSide()[i];
  double range = getRowRange()[i];
  char   sense = getRowSense()[i];
  double lower, upper;

  convertSenseToBound( sense, rhs, range, lower, upper );
  if( upper != elementValue ) {
      convertBoundToSense( lower, elementValue, sense, rhs, range );
      setRowType( i, sense, rhs, range );
  }
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setRowBounds( int elementIndex, double lower, double upper )
{
  debugMessage("OsiCpxSolverInterface::setRowBounds(%d, %g, %g)\n", elementIndex, lower, upper);

  double rhs, range;
  char sense;
  
  convertBoundToSense( lower, upper, sense, rhs, range );
  setRowType( elementIndex, sense, rhs, range );
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setRowType(int i, char sense, double rightHandSide,
				  double range)
{
  debugMessage("OsiCpxSolverInterface::setRowType(%d, %c, %g, %g)\n", i, sense, rightHandSide, range);

  int err;

  if (sense == 'R') {
     assert( range >= 0.0 );
     rightHandSide -= range;
  }
  if (sense == 'N') {
     sense = 'R';
     rightHandSide = -getInfinity();
     range = 2*getInfinity();
  }
  err = CPXchgsense( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_ROW ),
		     1, &i, &sense );
  checkCPXerror( err, "CPXchgsense", "setRowType" );
  err = CPXchgrhs( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_ROW ),
		   1, &i, &rightHandSide );
  checkCPXerror( err, "CPXchgrhs", "setRowType" );
  err = CPXchgrngval( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_ROW ),
		      1, &i, &range );
  checkCPXerror( err, "CPXchgrngval", "setRowType" );
}
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::setRowSetBounds(const int* indexFirst,
					    const int* indexLast,
					    const double* boundList)
{
  debugMessage("OsiCpxSolverInterface::setRowSetBounds(%p, %p, %p)\n", indexFirst, indexLast, boundList);

   const int cnt = indexLast - indexFirst;
   if (cnt <= 0)
      return;

   char* sense = new char[cnt];
   double* rhs = new double[cnt];
   double* range = new double[cnt];
   for (int i = 0; i < cnt; ++i) {
      convertBoundToSense(boundList[2*i], boundList[2*i+1],
			  sense[i], rhs[i], range[i]);
   }
   setRowSetTypes(indexFirst, indexLast, sense, rhs, range);
   delete[] range;
   delete[] rhs;
   delete[] sense;
   
   //  OsiSolverInterface::setRowSetBounds( indexFirst, indexLast, boundList );
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setRowSetTypes(const int* indexFirst,
				      const int* indexLast,
				      const char* senseList,
				      const double* rhsList,
				      const double* rangeList)
{
  debugMessage("OsiCpxSolverInterface::setRowSetTypes(%p, %p, %p, %p, %p)\n", 
     indexFirst, indexLast, senseList, rhsList, rangeList);

   const int cnt = indexLast - indexFirst;
   if (cnt <= 0)
      return;

   char* sense = new char[cnt];
   double* rhs = new double[cnt];
   double* range = new double[cnt];
   int* rangeind = new int[cnt];
   int rangecnt = 0;
   for (int i = 0; i < cnt; ++i) {
      sense[i] = senseList[i];
      rhs[i] = rhsList[i];
      if (sense[i] == 'R') {
	 assert(rangeList[i] >= 0.0);
	 rhs[i] -= rangeList[i];
	 rangeind[rangecnt] = indexFirst[i];
	 range[rangecnt] = rangeList[i];
	 ++rangecnt;
      }
      if (sense[i] == 'N') {
	 sense[i] = 'R';
	 rhs[i] = -getInfinity();
	 rangeind[rangecnt] = indexFirst[i];
	 range[rangecnt] = 2*getInfinity();
	 ++rangecnt;
      }
   }
   int err;
   err = CPXchgsense(env_, getLpPtr(OsiCpxSolverInterface::FREECACHED_ROW),
		     cnt, const_cast<int*>(indexFirst), sense);
   checkCPXerror( err, "CPXchgsense", "setRowSetTypes" );
   err = CPXchgrhs(env_, getLpPtr(OsiCpxSolverInterface::FREECACHED_ROW),
		   cnt, const_cast<int*>(indexFirst), rhs);
   checkCPXerror( err, "CPXchgrhs", "setRowSetTypes" );
   err = CPXchgrngval(env_, getLpPtr(OsiCpxSolverInterface::FREECACHED_ROW),
		      rangecnt, rangeind, range);
   checkCPXerror( err, "CPXchgrngval", "setRowSetTypes" );

   delete[] rangeind;
   delete[] range;
   delete[] rhs;
   delete[] sense;
//    OsiSolverInterface::setRowSetTypes( indexFirst, indexLast, senseList,
//  				      rhsList, rangeList );
}
//#############################################################################
void
OsiCpxSolverInterface::setContinuous(int index)
{
  debugMessage("OsiCpxSolverInterface::setContinuous(%d)\n", index);

  assert(coltype_ != NULL);
  assert(coltypesize_ >= getNumCols());

  coltype_[index] = 'C';

  if ( probtypemip_ )
    {
      CPXLPptr lp = getMutableLpPtr();
      int err;
      err = CPXchgctype( env_, lp, 1, &index, &coltype_[index] );
      checkCPXerror( err, "CPXchgctype", "setContinuous" );
    }
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setInteger(int index)
{
  debugMessage("OsiCpxSolverInterface::setInteger(%d)\n", index);

  assert(coltype_ != NULL);
  assert(coltypesize_ >= getNumCols());

  if( getColLower()[index] == 0.0 && getColUpper()[index] == 1.0 )
     coltype_[index] = 'B';
  else
     coltype_[index] = 'I';

  if ( probtypemip_ )
    {
      CPXLPptr lp = getMutableLpPtr();
      int err;
      err = CPXchgctype( env_, lp, 1, &index, &coltype_[index] );
      checkCPXerror( err, "CPXchgctype", "setInteger" );
    }
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setContinuous(const int* indices, int len)
{
  debugMessage("OsiCpxSolverInterface::setContinuous(%p, %d)\n", indices, len);

  for( int i = 0; i < len; ++i )
     setContinuous(indices[i]);
}
//-----------------------------------------------------------------------------
void
OsiCpxSolverInterface::setInteger(const int* indices, int len)
{
  debugMessage("OsiCpxSolverInterface::setInteger(%p, %d)\n", indices, len);

  for( int i = 0; i < len; ++i )
     setInteger(indices[i]);
}
//#############################################################################

void OsiCpxSolverInterface::setObjSense(double s) 
{
  debugMessage("OsiCpxSolverInterface::setObjSense(%g)\n", s);

  if( s == +1.0 )
    CPXchgobjsen( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS ), CPX_MIN );
  else
    CPXchgobjsen( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_RESULTS ), CPX_MAX );
}
 
//-----------------------------------------------------------------------------

void OsiCpxSolverInterface::setColSolution(const double * cs) 
{
  debugMessage("OsiCpxSolverInterface::setColSolution(%p)\n", cs);

  int nc = getNumCols();

  if( cs == NULL )
    freeCachedResults();
  else if( nc > 0 )
    {
      // If colsol isn't allocated, then allocate it
      if ( colsol_ == NULL )
	colsol_ = new double[nc];
	
      // Copy in new col solution.
      CoinDisjointCopyN( cs, nc, colsol_ );
      
      // CPLEX < 7.0 doesn't support setting a col solution without a row solution
      // -> if a row solution exists or CPLEX version >= 7, then pass into CPLEX
      if ( rowsol_ != NULL || cpxVersionMajor_ >= 7 )
	{
	  int err = CPXcopystart( env_, getMutableLpPtr(), NULL, NULL, 
				  const_cast<double*>( colsol_ ), 
				  const_cast<double*>( rowsol_ ), 
				  NULL, NULL );
	  checkCPXerror( err, "CPXcopystart", "setColSolution" );
	}
    }
}

//-----------------------------------------------------------------------------

void OsiCpxSolverInterface::setRowPrice(const double * rs) 
{
  debugMessage("OsiCpxSolverInterface::setRowPrice(%p)\n", rs);

  int nr = getNumRows();

  if( rs == NULL )
    freeCachedResults();
  else if( nr > 0 )
    {
      // If rowsol isn't allocated, then allocate it
      if ( rowsol_ == NULL )
	rowsol_ = new double[nr];

      // Copy in new row solution.
      CoinDisjointCopyN( rs, nr, rowsol_ );
      
      // if a col solution exists, then pass into CPLEX
      if ( colsol_ != NULL )
	{
	  int err = CPXcopystart( env_, getMutableLpPtr(), NULL, NULL, 
				  const_cast<double*>( colsol_ ), 
				  const_cast<double*>( rowsol_ ), 
				  NULL, NULL );
	  checkCPXerror( err, "CPXcopystart", "setRowPrice" );
	}
    }
}

//#############################################################################
// Problem modifying methods (matrix)
//#############################################################################
void 
OsiCpxSolverInterface::addCol(const CoinPackedVectorBase& vec,
			      const double collb, const double colub,   
			      const double obj)
{
  debugMessage("OsiCpxSolverInterface::addCol(%p, %g, %g, %g)\n", &vec, collb, colub, obj);

  int nc = getNumCols();
  assert(coltypesize_ >= nc);

  resizeColType(nc + 1);
  coltype_[nc] = 'C';

  int err;
  int cmatbeg[2] = {0, vec.getNumElements()};

  err = CPXaddcols( env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_ROW ),
		    1, vec.getNumElements(), const_cast<double*>(&obj),
		    cmatbeg,
		    const_cast<int*>(vec.getIndices()),
		    const_cast<double*>(vec.getElements()),
		    const_cast<double*>(&collb),
		    const_cast<double*>(&colub), NULL );
  checkCPXerror( err, "CPXaddcols", "addCol" );
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::addCols(const int numcols,
			       const CoinPackedVectorBase * const * cols,
			       const double* collb, const double* colub,   
			       const double* obj)
{
  debugMessage("OsiCpxSolverInterface::addCols(%d, %p, %p, %p, %p)\n", numcols, cols, collb, colub, obj);

  int nc = getNumCols();
  assert(coltypesize_ >= nc);

  resizeColType(nc + numcols);
  CoinFillN(&coltype_[nc], numcols, 'C');

  int i;
  int nz = 0;
  for (i = 0; i < numcols; ++i)
    nz += cols[i]->getNumElements();

  int* index = new int[nz];
  double* elem = new double[nz];
  int* start = new int[numcols+1];

  nz = 0;
  start[0] = 0;
  for (i = 0; i < numcols; ++i) {
    const CoinPackedVectorBase* col = cols[i];
    const int len = col->getNumElements();
    CoinDisjointCopyN(col->getIndices(), len, index+nz);
    CoinDisjointCopyN(col->getElements(), len, elem+nz);
    nz += len;
    start[i+1] = nz;
  }
  int err = CPXaddcols(env_, getLpPtr(OsiCpxSolverInterface::KEEPCACHED_ROW),
		       numcols, nz, const_cast<double*>(obj),
		       start, index, elem, 
		       const_cast<double*>(collb),
		       const_cast<double*>(colub), NULL );
  checkCPXerror( err, "CPXaddcols", "addCols" );

  delete[] start;
  delete[] elem;
  delete[] index;

//    int i;
//    for( i = 0; i < numcols; ++i )
//      addCol( *(cols[i]), collb[i], colub[i], obj[i] );
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::deleteCols(const int num, const int * columnIndices)
{
  debugMessage("OsiCpxSolverInterface::deleteCols(%d, %p)\n", num, columnIndices);

  int ncols = getNumCols();
  int *delstat = new int[ncols];
  int i, err;

  CoinFillN(delstat, ncols, 0);
  for( i = 0; i < num; ++i )
    delstat[columnIndices[i]] = 1;
  err = CPXdelsetcols( env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_ROW ), delstat );
  checkCPXerror( err, "CPXdelsetcols", "deleteCols" );

  for( i = 0; i < ncols; ++i )
  {
     assert(delstat[i] <= i);
     if( delstat[i] != -1 )
        coltype_[delstat[i]] = coltype_[i];
  }

  delete[] delstat;
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::addRow(const CoinPackedVectorBase& vec,
			      const double rowlb, const double rowub)
{
  debugMessage("OsiCpxSolverInterface::addRow(%p, %g, %g)\n", &vec, rowlb, rowub);

  char sense;
  double rhs, range;

  convertBoundToSense( rowlb, rowub, sense, rhs, range );
  addRow( vec, sense, rhs, range );
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::addRow(const CoinPackedVectorBase& vec,
			      const char rowsen, const double rowrhs,   
			      const double rowrng)
{
  debugMessage("OsiCpxSolverInterface::addRow(%p, %c, %g, %g)\n", &vec, rowsen, rowrhs, rowrng);

  int err;
  int rmatbeg = 0;
  double rhs;
  double range;
  char sense = rowsen;

  switch( rowsen )
    {
    case 'R':
      assert( rowrng >= 0.0 );
      rhs = rowrhs - rowrng;
      range = rowrng;
      break;
    case 'N':
      sense = 'R';
      rhs   = -getInfinity();
      range = 2*getInfinity();
      break;
    default:
      rhs = rowrhs;
      range = 0.0;
    }

  err = CPXaddrows( env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_COLUMN ), 0, 1, vec.getNumElements(), 
		    &rhs,
		    &sense,
		    &rmatbeg,
		    const_cast<int*>(vec.getIndices()),
		    const_cast<double*>(vec.getElements()),
		    NULL, NULL );
  checkCPXerror( err, "CPXaddrows", "addRow" );
  if( rowsen == 'R' )
    {
      int row = getNumRows() - 1;
      err = CPXchgrngval( env_, getLpPtr( OsiCpxSolverInterface::FREECACHED_ROW ), 1, &row, &range );
      checkCPXerror( err, "CPXchgrngval", "addRow" );
    }
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::addRows(const int numrows,
			       const CoinPackedVectorBase * const * rows,
			       const double* rowlb, const double* rowub)
{
  debugMessage("OsiCpxSolverInterface::addRows(%d, %p, %p, %p)\n", numrows, rows, rowlb, rowub);

  int i;

  for( i = 0; i < numrows; ++i )
    addRow( *(rows[i]), rowlb[i], rowub[i] );
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::addRows(const int numrows,
			       const CoinPackedVectorBase * const * rows,
			       const char* rowsen, const double* rowrhs,   
			       const double* rowrng)
{
  debugMessage("OsiCpxSolverInterface::addRows(%d, %p, %p, %p, %p)\n", numrows, rows, rowsen, rowrhs, rowrng);

  int i;

  for( i = 0; i < numrows; ++i )
    addRow( *(rows[i]), rowsen[i], rowrhs[i], rowrng[i] );
}
//-----------------------------------------------------------------------------
void 
OsiCpxSolverInterface::deleteRows(const int num, const int * rowIndices)
{
  debugMessage("OsiCpxSolverInterface::deleteRows(%d, %p)\n", num, rowIndices);

  int nrows = getNumRows();
  int *delstat = new int[nrows];
  int i, err;

  CoinFillN( delstat, nrows, 0 );
  for( i = 0; i < num; ++i )
    delstat[rowIndices[i]] = 1;
  err = CPXdelsetrows( env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_COLUMN ), delstat );
  checkCPXerror( err, "CPXdelsetrows", "deleteRows" );
  delete[] delstat;
}

//#############################################################################
// Methods to input a problem
//#############################################################################

void
OsiCpxSolverInterface::loadProblem( const CoinPackedMatrix& matrix,
				    const double* collb, const double* colub,
				    const double* obj,
				    const double* rowlb, const double* rowub )
{
  debugMessage("OsiCpxSolverInterface::loadProblem(%p, %p, %p, %p, %p, %p)\n", &matrix, collb, colub, obj, rowlb, rowub);

  const double inf = getInfinity();
  
  int nrows = matrix.getNumRows();
  char   * rowSense = new char  [nrows];
  double * rowRhs   = new double[nrows];
  double * rowRange = new double[nrows];
  
  int i;
  for ( i = nrows - 1; i >= 0; --i )
    {
      const double lower = rowlb ? rowlb[i] : -inf;
      const double upper = rowub ? rowub[i] : inf;
      convertBoundToSense( lower, upper, rowSense[i], rowRhs[i], rowRange[i] );
    }

  loadProblem( matrix, collb, colub, obj, rowSense, rowRhs, rowRange ); 
  delete [] rowSense;
  delete [] rowRhs;
  delete [] rowRange;
}
			    
//-----------------------------------------------------------------------------

void
OsiCpxSolverInterface::assignProblem( CoinPackedMatrix*& matrix,
				      double*& collb, double*& colub,
				      double*& obj,
				      double*& rowlb, double*& rowub )
{
  debugMessage("OsiCpxSolverInterface::assignProblem()\n");

  loadProblem( *matrix, collb, colub, obj, rowlb, rowub );
  delete matrix;   matrix = 0;
  delete[] collb;  collb = 0;
  delete[] colub;  colub = 0;
  delete[] obj;    obj = 0;
  delete[] rowlb;  rowlb = 0;
  delete[] rowub;  rowub = 0;
}

//-----------------------------------------------------------------------------

void
OsiCpxSolverInterface::loadProblem( const CoinPackedMatrix& matrix,
				    const double* collb, const double* colub,
				    const double* obj,
				    const char* rowsen, const double* rowrhs,
				    const double* rowrng )
{
  debugMessage("OsiCpxSolverInterface::loadProblem(%p, %p, %p, %p, %p, %p, %p)\n",
     &matrix, collb, colub, obj, rowsen, rowrhs, rowrng);

  int nc=matrix.getNumCols();
  int nr=matrix.getNumRows();

  if( nr == 0 || nc == 0 )   // empty LP?
    gutsOfDestructor();      // -> kill old LP
  else
    {
      assert( rowsen != NULL );
      assert( rowrhs != NULL );

      int i;
      
      // Set column values to defaults if NULL pointer passed
      double * clb;  
      double * cub;
      double * ob;
      double * rr = NULL;
      double * rhs;
      if ( collb!=NULL )
	clb=const_cast<double*>(collb);
      else
	{
	  clb = new double[nc];
	  CoinFillN(clb, nc, 0.0);
	}
      if ( colub!=NULL )
	cub=const_cast<double*>(colub);
      else
	{
	  cub = new double[nc];
	  CoinFillN(cub, nc, getInfinity());
	}
      if ( obj!=NULL )
	ob=const_cast<double*>(obj);
      else
	{
	  ob = new double[nc];
	  CoinFillN(ob, nc, 0.0);
	}
      if ( rowrng != NULL )
	{
	  rhs = new double[nr];
	  rr = new double[nr];
	  for ( i=0; i<nr; i++ )
	    {
	      if (rowsen[i] == 'R')
		{
		  if( rowrng[i] >= 0 )
		    {
		      rhs[i] = rowrhs[i] - rowrng[i];
		      rr[i] = rowrng[i];
		    }
		  else
		    {
		      rhs[i] = rowrhs[i];
		      rr[i] = -rowrng[i];
		    }
		} 
	      else
		{
		  rhs[i] = rowrhs[i];
		  rr[i] = 0.0;
		}
	    }
	} 
      else
	rhs = const_cast<double*>(rowrhs);
      
      bool freeMatrixRequired = false;
      CoinPackedMatrix * m = NULL;
      if ( !matrix.isColOrdered() ) 
	{
	  m = new CoinPackedMatrix();
	  m->reverseOrderedCopyOf(matrix);
	  freeMatrixRequired = true;
	} 
      else 
	m = const_cast<CoinPackedMatrix *>(&matrix);
      
      assert( nc == m->getNumCols() );
      assert( nr == m->getNumRows() );
      assert( m->isColOrdered() ); 
      
      int objDirection = CPXgetobjsen( env_, getMutableLpPtr() );
      
      int err = CPXcopylp( env_, getLpPtr(), 
			   nc, nr,
			   // Leave ObjSense alone(set to current value).
			   objDirection,
			   ob, 
			   rhs,
			   const_cast<char *>(rowsen),
			   const_cast<int *>(m->getVectorStarts()),
			   const_cast<int *>(m->getVectorLengths()),
			   const_cast<int *>(m->getIndices()),
			   const_cast<double *>(m->getElements()),
			   const_cast<double *>(clb), 
			   const_cast<double *>(cub), 
			   rr );
      checkCPXerror( err, "CPXcopylp", "loadProblem" );
            
      if ( collb == NULL )
	delete[] clb;
      if ( colub == NULL ) 
	delete[] cub;
      if ( obj   == NULL )
	delete[] ob;
      if ( rowrng != NULL ) {
	delete[] rr;
	delete[] rhs;
      }
      
      if ( freeMatrixRequired ) 
	delete m;

      resizeColType(nc);
      CoinFillN(coltype_, nc, 'C');
    }
}
   
//-----------------------------------------------------------------------------

void
OsiCpxSolverInterface::assignProblem( CoinPackedMatrix*& matrix,
				      double*& collb, double*& colub,
				      double*& obj,
				      char*& rowsen, double*& rowrhs,
				      double*& rowrng )
{
  debugMessage("OsiCpxSolverInterface::assignProblem()\n");

   loadProblem( *matrix, collb, colub, obj, rowsen, rowrhs, rowrng );
   delete matrix;   matrix = 0;
   delete[] collb;  collb = 0;
   delete[] colub;  colub = 0;
   delete[] obj;    obj = 0;
   delete[] rowsen; rowsen = 0;
   delete[] rowrhs; rowrhs = 0;
   delete[] rowrng; rowrng = 0;
}

//-----------------------------------------------------------------------------

void
OsiCpxSolverInterface::loadProblem(const int numcols, const int numrows,
				   const int* start, const int* index,
				   const double* value,
				   const double* collb, const double* colub,   
				   const double* obj,
				   const double* rowlb, const double* rowub )
{
  debugMessage("OsiCpxSolverInterface::loadProblem()\n");

  const double inf = getInfinity();
  
  char   * rowSense = new char  [numrows];
  double * rowRhs   = new double[numrows];
  double * rowRange = new double[numrows];
  
  for ( int i = numrows - 1; i >= 0; --i ) {
    const double lower = rowlb ? rowlb[i] : -inf;
    const double upper = rowub ? rowub[i] : inf;
    convertBoundToSense( lower, upper, rowSense[i], rowRhs[i], rowRange[i] );
  }

  loadProblem(numcols, numrows, start, index, value, collb, colub, obj,
	      rowSense, rowRhs, rowRange);
  delete [] rowSense;
  delete [] rowRhs;
  delete [] rowRange;

}

//-----------------------------------------------------------------------------

void
OsiCpxSolverInterface::loadProblem(const int numcols, const int numrows,
				   const int* start, const int* index,
				   const double* value,
				   const double* collb, const double* colub,   
				   const double* obj,
				   const char* rowsen, const double* rowrhs,
				   const double* rowrng )
{
  debugMessage("OsiCpxSolverInterface::loadProblem(%d, %d, %p, %p, %p, %p, %p, %p, %p, %p, %p)\n",
     numcols, numrows, start, index, value, collb, colub, obj, rowsen, rowrhs, rowrng);

  const int nc = numcols;
  const int nr = numrows;

  if( nr == 0 || nc == 0 ) {
    // empty LP? -> kill old LP
    gutsOfDestructor();
    return;
  }

  assert( rowsen != NULL );
  assert( rowrhs != NULL );
      
  int i;
      
  // Set column values to defaults if NULL pointer passed
  int * len = new int[nc];
  double * clb = new double[nc];  
  double * cub = new double[nc];  
  double * ob = new double[nc];  
  double * rr = new double[nr];
  double * rhs = new double[nr];
  char * sen = new char[nr];
  
  for (i = 0; i < nc; ++i) {
    len[i] = start[i+1] - start[i];
  }

  if ( collb != NULL )
    CoinDisjointCopyN(collb, nc, clb);
  else
    CoinFillN(clb, nc, 0.0);

  if ( colub!=NULL )
    CoinDisjointCopyN(colub, nc, cub);
  else
    CoinFillN(cub, nc, getInfinity());
  
  if ( obj!=NULL )
    CoinDisjointCopyN(obj, nc, ob);
  else
    CoinFillN(ob, nc, 0.0);
  
  if ( rowrng != NULL ) {
    for ( i=0; i<nr; i++ ) {
      if (rowsen[i] == 'R') {
	if ( rowrng[i] >= 0 ) {
	  rhs[i] = rowrhs[i] - rowrng[i];
	  rr[i] = rowrng[i];
	} else {
	  rhs[i] = rowrhs[i];
	  rr[i] = -rowrng[i];
	}
      } else {
	rhs[i] = rowrhs[i];
	rr[i] = 0.0;
      }
    }
  } else {
    CoinDisjointCopyN(rowrhs, nr, rhs);
  }

  CoinDisjointCopyN(rowsen, nr, sen);
  
  int objDirection = CPXgetobjsen( env_, getMutableLpPtr() );
      
  int err = CPXcopylp( env_, getLpPtr(), 
		       nc, nr,
		       // Leave ObjSense alone(set to current value).
		       objDirection, ob, rhs, sen,
		       const_cast<int *>(start), 
		       len, const_cast<int *>(index), 
		       const_cast<double *>(value),
		       clb, cub, rr);

  checkCPXerror( err, "CPXcopylp", "loadProblem" );
  
  delete[] len;
  delete[] clb;
  delete[] cub;
  delete[] ob;
  delete[] rr;
  delete[] rhs;
  delete[] sen;

  resizeColType(nc);
  CoinFillN(coltype_, nc, 'C');
}
 
//-----------------------------------------------------------------------------
// Read mps files
//-----------------------------------------------------------------------------
int OsiCpxSolverInterface::readMps( const char * filename,
				     const char * extension )
{
  debugMessage("OsiCpxSolverInterface::readMps(%s, %s)\n", filename, extension);

#if 0
  std::string f(filename);
  std::string e(extension);
  std::string fullname = f + "." + e;
  int err = CPXreadcopyprob( env_, getLpPtr(), const_cast<char*>( fullname.c_str() ), NULL );
  checkCPXerror( err, "CPXreadcopyprob", "readMps" );
#endif
  // just call base class method
  return OsiSolverInterface::readMps(filename,extension);
}


//-----------------------------------------------------------------------------
// Write mps files
//-----------------------------------------------------------------------------
void OsiCpxSolverInterface::writeMps( const char * filename,
				      const char * extension,
				      double objSense ) const
{
  debugMessage("OsiCpxSolverInterface::writeMps(%s, %s, %g)\n", filename, extension, objSense);

  // *FIXME* : this will not output ctype information to the MPS file
  char filetype[4] = "MPS";
  std::string f(filename);
  std::string e(extension);
  std::string fullname = f + "." + e;
  int err = CPXwriteprob( env_, getMutableLpPtr(), const_cast<char*>( fullname.c_str() ), filetype );
  checkCPXerror( err, "CPXwriteprob", "writeMps" );
}

//#############################################################################
// CPX specific public interfaces
//#############################################################################

CPXENVptr OsiCpxSolverInterface::getEnvironmentPtr()
{
  assert( env_ != NULL );
  return env_;
}

CPXLPptr OsiCpxSolverInterface::getLpPtr( int keepCached )
{
  freeCachedData( keepCached );
  return getMutableLpPtr();
}

//-----------------------------------------------------------------------------

const char * OsiCpxSolverInterface::getCtype() const
{
  debugMessage("OsiCpxSolverInterface::getCtype()\n");

  return coltype_;
}

//#############################################################################
// Static instance counter methods
//#############################################################################

void OsiCpxSolverInterface::incrementInstanceCounter()
{
  if ( numInstances_ == 0 )
    {
      int err;

#if CPX_VERSION >= 800
      env_ = CPXopenCPLEX( &err );
#else
      env_ = CPXopenCPLEXdevelop( &err );
#endif

      checkCPXerror( err, "CPXopenCPLEXdevelop", "incrementInstanceCounter" );
      assert( env_ != NULL );
#ifndef NDEBUG
      CPXsetintparam( env_, CPX_PARAM_SCRIND, CPX_ON ); // for testing purposes
#endif
      //char logfileName[]="cplex.log";
      //char filemode[]="a+";
      //CPXFILEptr fp = CPXfopen( logfileName, filemode );
      //CPXsetlogfile( env_, fp );
      err = sscanf( CPXversion( env_ ), "%d.%d.%d", &cpxVersionMajor_, &cpxVersionMinor_, &cpxVersionMinorMinor_ );
      assert( err == 3 );
    }
  numInstances_++;
}

//-----------------------------------------------------------------------------

void OsiCpxSolverInterface::decrementInstanceCounter()
{
  assert( numInstances_ != 0 );
  numInstances_--;
  if ( numInstances_ == 0 )
    {
      int err = CPXcloseCPLEX( &env_ );
      checkCPXerror( err, "CPXcloseCPLEX", "decrementInstanceCounter" );
      env_ = NULL;
    }
}

//-----------------------------------------------------------------------------

unsigned int OsiCpxSolverInterface::getNumInstances()
{
  return numInstances_;
}

//#############################################################################
// Constructors, destructors clone and assignment
//#############################################################################

//-------------------------------------------------------------------
// Default Constructor 
//-------------------------------------------------------------------
OsiCpxSolverInterface::OsiCpxSolverInterface()
  : OsiSolverInterface(),
    lp_(NULL),
    hotStartCStat_(NULL),
    hotStartCStatSize_(0),
    hotStartRStat_(NULL),
    hotStartRStatSize_(0),
    hotStartMaxIteration_(1000000), // ??? default iteration limit for strong branching is large
    obj_(NULL),
    collower_(NULL),
    colupper_(NULL),
    rowsense_(NULL),
    rhs_(NULL),
    rowrange_(NULL),
    rowlower_(NULL),
    rowupper_(NULL),
    colsol_(NULL),
    rowsol_(NULL),
    redcost_(NULL),
    rowact_(NULL),
    matrixByRow_(NULL),
    matrixByCol_(NULL),
    coltype_(NULL),
    coltypesize_(0),
    probtypemip_(false)
{
  debugMessage("OsiCpxSolverInterface::OsiCpxSolverInterface()\n");

  incrementInstanceCounter();
  gutsOfConstructor();
}


//----------------------------------------------------------------
// Clone
//----------------------------------------------------------------
OsiSolverInterface * OsiCpxSolverInterface::clone(bool copyData) const
{
  debugMessage("OsiCpxSolverInterface::clone(%d)\n", copyData);

  return( new OsiCpxSolverInterface( *this ) );
}

//-------------------------------------------------------------------
// Copy constructor 
//-------------------------------------------------------------------
OsiCpxSolverInterface::OsiCpxSolverInterface( const OsiCpxSolverInterface & source )
  : OsiSolverInterface(source),
    lp_(NULL),
    hotStartCStat_(NULL),
    hotStartCStatSize_(0),
    hotStartRStat_(NULL),
    hotStartRStatSize_(0),
    hotStartMaxIteration_(source.hotStartMaxIteration_),
    obj_(NULL),
    collower_(NULL),
    colupper_(NULL),
    rowsense_(NULL),
    rhs_(NULL),
    rowrange_(NULL),
    rowlower_(NULL),
    rowupper_(NULL),
    colsol_(NULL),
    rowsol_(NULL),
    redcost_(NULL),
    rowact_(NULL),
    matrixByRow_(NULL),
    matrixByCol_(NULL),
    coltype_(NULL),
    coltypesize_(0),
    probtypemip_(false)
{
  debugMessage("OsiCpxSolverInterface::OsiCpxSolverInterface(%p)\n", &source);

  incrementInstanceCounter();  
  gutsOfConstructor();
  gutsOfCopy( source );
}


//-------------------------------------------------------------------
// Destructor 
//-------------------------------------------------------------------
OsiCpxSolverInterface::~OsiCpxSolverInterface()
{
  debugMessage("OsiCpxSolverInterface::~OsiCpxSolverInterface()\n");

  gutsOfDestructor();
  decrementInstanceCounter();
}

//----------------------------------------------------------------
// Assignment operator 
//-------------------------------------------------------------------
OsiCpxSolverInterface& OsiCpxSolverInterface::operator=( const OsiCpxSolverInterface& rhs )
{
  debugMessage("OsiCpxSolverInterface::operator=(%p)\n", &rhs);

  if (this != &rhs)
    {    
      OsiSolverInterface::operator=( rhs );
      gutsOfDestructor();
      gutsOfConstructor();
      if ( rhs.lp_ !=NULL )
	gutsOfCopy( rhs );
    }
  return *this;
}

//#############################################################################
// Applying cuts
//#############################################################################

void OsiCpxSolverInterface::applyColCut( const OsiColCut & cc )
{
  debugMessage("OsiCpxSolverInterface::applyColCut(%p)\n", &cc);

  const double * cplexColLB = getColLower();
  const double * cplexColUB = getColUpper();
  const CoinPackedVector & lbs = cc.lbs();
  const CoinPackedVector & ubs = cc.ubs();
  int i;

  for( i = 0; i < lbs.getNumElements(); ++i ) 
    if ( lbs.getElements()[i] > cplexColLB[lbs.getIndices()[i]] )
      setColLower( lbs.getIndices()[i], lbs.getElements()[i] );
  for( i = 0; i < ubs.getNumElements(); ++i )
    if ( ubs.getElements()[i] < cplexColUB[ubs.getIndices()[i]] )
      setColUpper( ubs.getIndices()[i], ubs.getElements()[i] );
}

//-----------------------------------------------------------------------------

void OsiCpxSolverInterface::applyRowCut( const OsiRowCut & rowCut )
{
  debugMessage("OsiCpxSolverInterface::applyRowCut(%p)\n", &rowCut);

  int err = 0;
  double rhs = 0.0;
  double rng = 0.0;
  char sns;
  double lb = rowCut.lb();
  double ub = rowCut.ub();
  if( lb <= -getInfinity() && ub >= getInfinity() )   // free constraint
    {
      rhs = -getInfinity();
      rng = 2*getInfinity();  // CPLEX doesn't support free constraints
      sns = 'R';           // -> implement them as ranged rows with infinite bounds
    }
  else if( lb <= -getInfinity() )  // <= constraint
    {
      rhs = ub;
      sns = 'L';
    }
  else if( ub >= getInfinity() )  // >= constraint
    {
      rhs = lb;
      sns = 'G';
    }
  else if( ub == lb )  // = constraint
    {
      rhs = ub;
      sns = 'E';
    }
  else  // range constraint
    {
      rhs = lb;
      rng = ub - lb;
      sns = 'R';
    }
  int rmatbeg = 0;
  err = CPXaddrows( env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_COLUMN ), 0, 1, rowCut.row().getNumElements(),
		    &rhs, &sns, &rmatbeg, 
		    const_cast<int*>( rowCut.row().getIndices() ), 
		    const_cast<double*>( rowCut.row().getElements() ),
		    NULL, NULL );
  checkCPXerror( err, "CPXaddrows", "applyRowCut" );
  if( sns == 'R' )
    {
      err = CPXchgcoef( env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_COLUMN ), 
			CPXgetnumrows(env_, getLpPtr( OsiCpxSolverInterface::KEEPCACHED_COLUMN ))-1,
			-2, rng );
      checkCPXerror( err, "CPXchgcoef", "applyRowCut" );
    }
}

//#############################################################################
// Private methods (non-static and static) and static data
//#############################################################################

//------------------------------------------------------------------
// Static data
//------------------------------------------------------------------      
CPXENVptr OsiCpxSolverInterface::env_ = NULL;

int OsiCpxSolverInterface::cpxVersionMajor_ = 0;
int OsiCpxSolverInterface::cpxVersionMinor_ = 0;
int OsiCpxSolverInterface::cpxVersionMinorMinor_ = 0;
unsigned int OsiCpxSolverInterface::numInstances_ = 0;
 
//-------------------------------------------------------------------
// Get pointer to CPXLPptr.
// const methods should use getMutableLpPtr().
// non-const methods should use getLpPtr().
//------------------------------------------------------------------- 
CPXLPptr OsiCpxSolverInterface::getMutableLpPtr() const
{
  if ( lp_ == NULL )
    {
      int err;
      assert(env_ != NULL);
#if 0
      //char pn[] = "OSI_CPLEX";
      lp_ = CPXcreateprob( env_, &err, pn );
#else
      std::string pn;
      getStrParam(OsiProbName,pn);
      lp_ = CPXcreateprob( env_, &err, const_cast<char*>(pn.c_str()) );
#endif
      checkCPXerror( err, "CPXcreateprob", "getMutableLpPtr" );
//      err = CPXchgprobtype(env_,lp_,CPXPROB_LP);
//      checkCPXerror( err, "CPXchgprobtype", "getMutableLpPtr" );
      assert( lp_ != NULL ); 
    }
  return lp_;
}

//-------------------------------------------------------------------

void OsiCpxSolverInterface::gutsOfCopy( const OsiCpxSolverInterface & source )
{
  // Set Objective Sense
  setObjSense(source.getObjSense());

  // Set Rim and constraints
  const double* obj = source.getObjCoefficients();
  const double* rhs = source.getRightHandSide();
  const char* sense = source.getRowSense();
  const CoinPackedMatrix * cols = source.getMatrixByCol();
  const double* lb = source.getColLower();
  const double* ub = source.getColUpper();
  loadProblem(*cols,lb,ub,obj,sense,rhs,source.getRowRange());

  // Set MIP information
  resizeColType(source.coltypesize_);
  CoinDisjointCopyN( source.coltype_, source.coltypesize_, coltype_ );
  
  // Set Solution
  setColSolution(source.getColSolution());
  setRowPrice(source.getRowPrice());

  // Should also copy row and col names.
#if 0
  char** cname = new char*[numcols];
  char* cnamestore = NULL;
  int surplus;
  err = CPXgetcolname( env_, source.lp_, cname, NULL, 0, &surplus, 0, numcols-1 );
  if( err != CPXERR_NO_NAMES )
    {
      cnamestore = new char[-surplus];
      err = CPXgetcolname( env_, source.lp_, cname, cnamestore, -surplus, &surplus, 0, numcols-1 );
      checkCPXerror( err, "CPXgetcolname", "gutsOfCopy" );
      assert( surplus == 0 );
    }
  else
    {
      delete [] cname;
      cname = NULL;
    }
  
  char** rname = new char*[numrows];
  char* rnamestore = NULL;
  err = CPXgetrowname( env_, source.lp_, rname, NULL, 0, &surplus, 0, numrows-1 );
  if( err != CPXERR_NO_NAMES )
    {
      rnamestore = new char[-surplus];
      err = CPXgetrowname( env_, source.lp_, rname, rnamestore, -surplus, &surplus, 0, numrows-1 );
      checkCPXerror( err, "CPXgetrowname", "gutsOfCopy" );
      assert( surplus == 0 );
    }
  else
    {
      delete [] rname;
      rname = NULL;
    }

  err = CPXcopylpwnames( env_, getLpPtr(), 
			 numcols, numrows, objsen, 
			 const_cast<double *>(obj), 
			 const_cast<double *>(rhs), 
			 const_cast<char *>(sense),
			 const_cast<int *>(cols->vectorStarts()),
			 const_cast<int *>(cols->vectorLengths()),
			 const_cast<int *>(cols->indices()),
			 const_cast<double *>(cols->elements()),
			 const_cast<double *>(lb), 
			 const_cast<double *>(ub), 
			 rng, 
			 cname, rname);
  checkCPXerror( err, "CPXcopylpwnames", "gutsOfCopy" );
  
  if( rname != NULL )
    {
      delete [] rnamestore;
      delete [] rname;
    }
  if( cname != NULL )
    {
      delete [] cnamestore;
      delete [] cname;
    }
  delete [] rng;
#endif
 
}

//-------------------------------------------------------------------
void OsiCpxSolverInterface::gutsOfConstructor()
{  
#if 0
  // CPXcreateprob was moved to getLpPtr() method.
  int err;
  lp_ = CPXcreateprob( env_, &err, "OSI_CPLEX" );
  checkCPXerror( err, "CPXcreateprob", "gutsOfConstructor" );
//  err = CPXchgprobtype(env_,lp_,CPXPROB_LP);
//  checkCPXerror( err, "CPXchgprobtype", "getMutableLpPtr" );
  assert( lp_ != NULL );
#endif
}

//-------------------------------------------------------------------
void OsiCpxSolverInterface::gutsOfDestructor()
{  
  if ( lp_ != NULL )
    {
      int err = CPXfreeprob( env_, &lp_ );
      checkCPXerror( err, "CPXfreeprob", "gutsOfDestructor" );
      lp_=NULL;
      freeAllMemory();
    }
  assert( lp_==NULL );
  assert( obj_==NULL );
  assert( collower_==NULL );
  assert( colupper_==NULL );
  assert( rowsense_==NULL );
  assert( rhs_==NULL );
  assert( rowrange_==NULL );
  assert( rowlower_==NULL );
  assert( rowupper_==NULL );
  assert( colsol_==NULL );
  assert( rowsol_==NULL );
  assert( redcost_==NULL );
  assert( rowact_==NULL );
  assert( matrixByRow_==NULL );
  assert( matrixByCol_==NULL );
  assert( coltype_==NULL );
  assert( coltypesize_==0 );
}

//-------------------------------------------------------------------
/// free cached vectors

void OsiCpxSolverInterface::freeCachedColRim()
{
  freeCacheDouble( obj_ );  
  freeCacheDouble( collower_ ); 
  freeCacheDouble( colupper_ ); 
  assert( obj_==NULL );
  assert( collower_==NULL );
  assert( colupper_==NULL );
}

void OsiCpxSolverInterface::freeCachedRowRim()
{
  freeCacheChar( rowsense_ );
  freeCacheDouble( rhs_ );
  freeCacheDouble( rowrange_ );
  freeCacheDouble( rowlower_ );
  freeCacheDouble( rowupper_ );
  assert( rowsense_==NULL ); 
  assert( rhs_==NULL ); 
  assert( rowrange_==NULL ); 
  assert( rowlower_==NULL ); 
  assert( rowupper_==NULL );
 }

void OsiCpxSolverInterface::freeCachedMatrix()
{
  freeCacheMatrix( matrixByRow_ );
  freeCacheMatrix( matrixByCol_ );
  assert( matrixByRow_==NULL ); 
  assert( matrixByCol_==NULL ); 
}

void OsiCpxSolverInterface::freeCachedResults()
{
  freeCacheDouble( colsol_ ); 
  freeCacheDouble( rowsol_ );
  freeCacheDouble( redcost_ );
  freeCacheDouble( rowact_ );
  assert( colsol_==NULL );
  assert( rowsol_==NULL );
  assert( redcost_==NULL );
  assert( rowact_==NULL );
}


void OsiCpxSolverInterface::freeCachedData( int keepCached )
{
  if( !(keepCached & OsiCpxSolverInterface::KEEPCACHED_COLUMN) )
    freeCachedColRim();
  if( !(keepCached & OsiCpxSolverInterface::KEEPCACHED_ROW) )
    freeCachedRowRim();
  if( !(keepCached & OsiCpxSolverInterface::KEEPCACHED_MATRIX) )
    freeCachedMatrix();
  if( !(keepCached & OsiCpxSolverInterface::KEEPCACHED_RESULTS) )
    freeCachedResults();
}

void OsiCpxSolverInterface::freeAllMemory()
{
  freeCachedData();
  if( hotStartCStat_ != NULL )
    delete[] hotStartCStat_;
  if( hotStartRStat_ != NULL )
    delete[] hotStartRStat_;
  hotStartCStat_     = NULL;
  hotStartCStatSize_ = 0;
  hotStartRStat_     = NULL;
  hotStartRStatSize_ = 0;
  freeColType();
}

//#############################################################################
// Resets as if default constructor
void 
OsiCpxSolverInterface::reset()
{
  setInitialData(); // clear base class
  gutsOfDestructor();
}
