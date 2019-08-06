//==============================================================================
// IBM Confidential
//
// OCO Source Materials
//
// Watson Implosion Technology
//
// (C) Copyright IBM Corp. 1993, 2012  All Rights Reserved
//
// The Source code for this program is not published or otherwise divested of
// its trade secrets, irrespective of what has been deposited with the U. S.
// Copyright office.
//==============================================================================

#ifndef ObjStageMgrH
#define ObjStageMgrH

//------------------------------------------------------------------------------
// Header file: "ObjStageMgr.h"
//
// Contains the declaration of class ObjStageMgr.
//------------------------------------------------------------------------------

#include <StageMgr.h>
#include <PtrMap.h>

//------------------------------------------------------------------------------
// Class ObjStageMgr
//
// "Object Stage Manager"
// Manages the stage data for stochastic implosion in stage-by-object mode.
//
// Class hierarchy:
//
//    ProbAssoc
//       StageMgr
//          ObjStageMgr
//
// Implemented in Stage.C.
//------------------------------------------------------------------------------

class WitObjStageMgr: public WitStageMgr
   {
   public:

      //------------------------------------------------------------------------
      // Constructor functions.
      //------------------------------------------------------------------------

      WitObjStageMgr (WitStochModeMgr *);

      //------------------------------------------------------------------------
      // Destructor function.
      //------------------------------------------------------------------------

      virtual ~WitObjStageMgr ();

      //------------------------------------------------------------------------
      // Other public member functions.
      //------------------------------------------------------------------------

      virtual void       setObjectStageIdx (WitNode *, int);
      virtual void       setPeriodStageIdx (WitPeriod, int);
      virtual WitStage * objectStage       (WitNode *);
      virtual WitStage * periodStage       (WitPeriod);
      virtual WitStage * myStageOf         (WitNode *,  WitPeriod);
      virtual bool       includesNode      (WitStage *, WitNode *);
      virtual bool       includesPeriod    (WitStage *, WitPeriod);
      virtual void       writeData         ();
      virtual void       displayData       ();
      virtual void       copyAttrsFrom     (WitStageMgr *);
      virtual void       validateData      ();
      virtual void       issueAttMisMatchHeadMsg ();
         //
         // Overrides from class StageMgr.

   private:

      //------------------------------------------------------------------------
      // Private member functions.
      //------------------------------------------------------------------------

      void writeObjectStage (WitNode * theNode, const char * nodeTypeName);
         //
         // Writes objectStage for theNode to the input data file.
         // objTypeName indicates the type the theNode.

      void validateBomEnts ();
         //
         // Verifies that there is no BOM entry connecting a stage 1 Operation
         // to a stage 0 part.

      void validateSubs ();
         //
         // Verifies that there is no substitute connecting a stage 1 Operation
         // to a stage 0 part.

      void validateBopEnts ();
         //
         // Verifies that there is no BOP entry connecting a stage 1 Operation
         // to a stage 0 part.

      noCopyCtorAssign (WitObjStageMgr);

      //------------------------------------------------------------------------
      // Private member data.
      //------------------------------------------------------------------------

      WitPtrMap <WitNode, WitStage> objectStage_;
         //
         // objectStage_ (theNode) is the Stage to which theNode belongs.
   };

#endif