// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _MKC_INIT_H
#define _MKC_INIT_H

#include "BCP_USER.hpp"

class MKC_initialize : public USER_initialize {
   // Declare this function if not the default single process communication is
   // wanted
//    BCP_message_environment * msgenv_init();
   BCP_tm_user * tm_init(BCP_tm_prob& p,
			 const int argnum, const char * const * arglist);
   BCP_lp_user * lp_init(BCP_lp_prob& p);
};

#endif
