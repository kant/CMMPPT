// Copyright (C) 2002, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef SbbCompareUser_H
#define SbbCompareUser_H


//#############################################################################
/*  These are alternative strategies for node traversal.  
    They can take data etc for fine tuning 

    At present the node list is stored as a heap and the "test"
    comparison function returns true if node y is better than node x.

*/
#include "SbbNode.hpp"
#include "SbbCompareBase.hpp"

/* Before first solution do depth first,
   then it is computed to hit first solution less 2%
*/
class SbbCompareUser  : public SbbCompareBase {
public:
  // Weight for each infeasibility
  double weight_;
  // Number of solutions
  int numberSolutions_;
  // Default Constructor 
  SbbCompareUser () : weight_(-1.0), numberSolutions_(0) {test_=this;};

  ~SbbCompareUser() {};

  /* 
     Return true if y better than x
     Node y is better than node x if y has fewer unsatisfied (greater depth on tie) or
     after solution weighted value of y is less than weighted value of x
  */
  virtual bool test (SbbNode * x, SbbNode * y) {
    if (weight_<0.0) {
      // before solution
      /* printf("x %d %d %g, y %d %d %g\n",
	     x->numberUnsatisfied(),x->depth(),x->objectiveValue(),
	     y->numberUnsatisfied(),y->depth(),y->objectiveValue()); */
      if (x->numberUnsatisfied() > y->numberUnsatisfied())
	return true;
      else if (x->numberUnsatisfied() < y->numberUnsatisfied())
	return false;
      else
	return x->depth() < y->depth();
    } else {
      // after solution
      return x->objectiveValue()+ weight_*x->numberUnsatisfied() > 
	y->objectiveValue() + weight_*y->numberUnsatisfied();
    }
  }
  // This allows method to change behavior as it is called
  // after each solution
  virtual void newSolution(SbbModel * model,
			   double objectiveAtContinuous,
			   int numberInfeasibilitiesAtContinuous) 
  {
    if (model->getSolutionCount()==model->getNumberHeuristicSolutions())
      return; // solution was got by rounding
    // set to get close to this solution
    double costPerInteger = 
      (model->getObjValue()-objectiveAtContinuous)/
      ((double) numberInfeasibilitiesAtContinuous);
    weight_ = 0.98*costPerInteger;
    numberSolutions_++;
    if (numberSolutions_>5)
      weight_ =0.0; // this searches on objective
  }
  // This allows method to change behavior 
  virtual void every1000Nodes(SbbModel * model, int numberNodes)
  {
    if (numberNodes>10000)
      weight_ =0.0; // this searches on objective
  }
};

#endif
