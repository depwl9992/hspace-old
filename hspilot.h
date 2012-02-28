#ifdef _WIN32
#pragma once
#endif

#if !defined(__HSPILOT_H__)
#define __HSPILOT_H__

// Library Includes
#include "hsai.h"

// Local Includes

// Forward Declarations

// Types

// Constants

// Prototypes

void InitPilots(void);

#define HSPACE_PILOT_ALLOC(PILOT) \
	CHSPilot *PILOT = new CHSPilot(#PILOT)
#define HSPACE_AI_ALLOC(PILOT, AI, TYPE) \
	CHSAI *AI = new CHSAI(#AI, TYPE, hsai_ ## AI); \
							PILOT->SetAI(AI)
#define HSPACE_ATTR_ALLOC(AI, NAME, TYPE, DEFAULT) \
	AI->AddAtr(NAME, TYPE, DEFAULT)

#define HSPACE_AI_HDR(AI) \
	void hsai_ ## AI(CHSSysAutoPilot *, CHSAI *)
#define HSPACE_AI(AI) \
	void hsai_ ## AI(CHSSysAutoPilot *cAutoPilot, CHSAI *cAI)

#endif // __HSPILOT_H__
