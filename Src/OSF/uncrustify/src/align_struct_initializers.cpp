/**
 * @file align_struct_initializers.cpp
 *
 * @author  Guy Maurel
 * split from align.cpp
 * @author  Ben Gardner
 * @license GPL v2+
 */
#include <uncrustify-internal.h>
#pragma hdrstop
//#include "align_struct_initializers.h"
//#include "align_init_brace.h"
//#include "chunk.h"

void align_struct_initializers()
{
	LOG_FUNC_ENTRY();
	Chunk * pc = Chunk::GetHead();
	while(pc->IsNotNullChunk()) {
		Chunk * prev = pc->GetPrevNcNnl();
		if(prev->Is(CT_ASSIGN) && (pc->Is(CT_BRACE_OPEN) || (  language_is_set(LANG_D) && pc->Is(CT_SQUARE_OPEN)))) {
			align_init_brace(pc);
		}
		pc = pc->GetNextType(CT_BRACE_OPEN);
	}
} // align_struct_initializers
