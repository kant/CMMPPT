// Copyright (C) 2002, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef ClpDualRowSteepest_H
#define ClpDualRowSteepest_H

#include "ClpDualRowPivot.hpp"

//#############################################################################

/** Dual Row Pivot Steepest Edge Algorithm Class

See Forrest-Goldfarb paper for algorithm

*/

class CoinIndexedVector;

class ClpDualRowSteepest : public ClpDualRowPivot {
  
public:
  
  ///@name Algorithmic methods 
  //@{
  
  /// Returns pivot row, -1 if none
  virtual int pivotRow();
  
  /// Updates weights (may be empty)
  virtual void updateWeights(CoinIndexedVector * input,
			     CoinIndexedVector * spare,
			     CoinIndexedVector * updatedColumn);
  
  /** Updates primal solution (and maybe list of candidates)
      Uses input vector which it deletes
      Computes change in objective function
  */
  virtual void updatePrimalSolution(CoinIndexedVector * input,
				    double theta,
				    double & changeInObjective);

  /** Saves any weights round factorization as pivot rows may change
      Save model 
      May also recompute infeasibility stuff
      1) before factorization
      2) after good factorization (if weights empty may initialize)
      3) after something happened but no factorization 
         (e.g. check for infeasible)
      4) as 2 but restore weights from previous snapshot
      5) for strong branching - initialize (uninitialized) , infeasibilities
  */
  virtual void saveWeights(ClpSimplex * model, int mode);
  /// Gets rid of last update
  virtual void unrollWeights();
  /// Gets rid of all arrays
  virtual void clearArrays();
  //@}
  
  
  ///@name Constructors and destructors
  //@{
  /** Default Constructor 
      0 is unintialized, 1 full
  */
  ClpDualRowSteepest(int mode=0); 
  
  /// Copy constructor 
  ClpDualRowSteepest(const ClpDualRowSteepest &);
  
  /// Assignment operator 
  ClpDualRowSteepest & operator=(const ClpDualRowSteepest& rhs);
  
  /// Destructor 
  virtual ~ClpDualRowSteepest ();

  /// Clone
  virtual ClpDualRowPivot * clone(bool copyData = true) const;
 
   //@}
  /**@name gets and sets */
  //@{ 
  /// Mode
  inline int mode() const
    { return mode_;};
 //@}

  //---------------------------------------------------------------------------
  
private:
  ///@name Private member data 
  /** Status
      0) Normal 
      -1) Needs initialization
      1) Weights are stored by sequence number
  */
  int state_;
  /// If 0 then we are using uninitialized weights, 1 then full
  int mode_;
  /// weight array 
  double * weights_;
  /// square of infeasibility array (just for infeasible rows)
  CoinIndexedVector * infeasible_;
  /// alternate weight array (so we can unroll)
  CoinIndexedVector * alternateWeights_;
  /// save weight array (so we can use checkpoint)
  CoinIndexedVector * savedWeights_;
  //@}
};

#endif
