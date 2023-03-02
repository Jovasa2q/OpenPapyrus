/**
 * @file align_func_proto.cpp
 *
 * @author  Guy Maurel
 * split from align.cpp
 * @author  Ben Gardner
 * @license GPL v2+
 */
#include <uncrustify-internal.h>
#pragma hdrstop
//#include "align_func_proto.h"
//#include "align_stack.h"
//#include "align_tools.h"
//#include "log_rules.h"

constexpr static auto LCURRENT = LALPROTO;
using namespace uncrustify;

void align_func_proto(size_t span)
{
	LOG_FUNC_ENTRY();
	size_t myspan   = span;
	size_t mythresh = 0;
	log_rule_B("align_func_proto_gap");
	size_t mygap = options::align_func_proto_gap();

	log_rule_B("align_func_proto_thresh");
	mythresh = options::align_func_proto_thresh();

	// Issue #2771
	// we align token-1 and token-2 if:
	//   token-1->GetLevel() == token-2->GetLevel()
	//   and
	//   token-1->GetBraceLevel() == token-2->GetBraceLevel()
	// we don't check if token-1 and token-2 are in the same block

	log_rule_B("align_func_proto_star_style");
	size_t mystar_style = options::align_func_proto_star_style();

	log_rule_B("align_func_proto_amp_style");
	size_t myamp_style = options::align_func_proto_amp_style();

	size_t num_of_column     = 1;
	size_t num_of_row        = 1;
	AlignStack * stack_init_value = nullptr;

	// Issue #2984
	vector<vector<AlignStack *> > many_as;
	// Issue #2771
	vector<vector<AlignStack *> > many_as_brace;

	// init the vector ...
	many_as.resize(num_of_column, vector<AlignStack *>(num_of_row, stack_init_value));
	many_as_brace.resize(num_of_column, vector<AlignStack *>(num_of_row, stack_init_value));

	log_rule_B("align_single_line_brace_gap");
	size_t mybr_gap = options::align_single_line_brace_gap();

	bool look_bro = false;
	Chunk * toadd;

	for(Chunk * pc = Chunk::GetHead(); pc->IsNotNullChunk(); pc = pc->GetNext()) {
		char copy[1000];
		LOG_FMT(LAS, "%s(%d): orig line is %zu, orig col is %zu, Text() is '%s', type is %s, level is %zu, brace level is %zu\n",
		    __func__, __LINE__, pc->GetOrigLine(), pc->GetOrigCol(), pc->ElidedText(copy),
		    get_token_name(pc->GetType()), pc->GetLevel(), pc->GetBraceLevel());

		// make the vector larger if necessary
		if(pc->GetLevel() >= num_of_column                      // Issue #2960
		    || pc->GetBraceLevel() >= num_of_row) {
			num_of_column = pc->GetLevel() + 1;
			num_of_row    = pc->GetBraceLevel() + 1;

			many_as.resize(num_of_column);
			many_as_brace.resize(num_of_column);

			for(size_t i = 0; i < num_of_column; ++i) {
				many_as[i].resize(num_of_row);
				many_as_brace[i].resize(num_of_row);
			}
		}

		if(pc->IsNewline() && !pc->TestFlags(PCF_IN_FCN_CALL)) {    // Issue #2831
			look_bro = false;
			AlignStack * stack_at_l_bl = many_as.at(pc->GetLevel()).at(pc->GetBraceLevel());
			if(stack_at_l_bl == nullptr) {
				// get a Stack
				stack_at_l_bl = new AlignStack();
				// start it
				stack_at_l_bl->Start(myspan, mythresh);
				stack_at_l_bl->m_gap        = mygap;
				stack_at_l_bl->m_star_style = static_cast<AlignStack::StarStyle>(mystar_style);
				stack_at_l_bl->m_amp_style  = static_cast<AlignStack::StarStyle>(myamp_style);
				// store
				many_as.at(pc->GetLevel()).at(pc->GetBraceLevel()) = stack_at_l_bl;
			}
			stack_at_l_bl->Debug();
			for(size_t idx = 0; idx < num_of_column; idx++) {
				for(size_t idx_brace = 0; idx_brace < num_of_row; idx_brace++) {
					stack_at_l_bl = many_as.at(idx).at(idx_brace);
					if(stack_at_l_bl != nullptr) {
						stack_at_l_bl->NewLines(pc->GetNlCount());
					}
				}
			}
			AlignStack * stack_at_l_bl_brace = many_as_brace.at(pc->GetLevel()).at(pc->GetBraceLevel());
			if(stack_at_l_bl_brace == nullptr) {
				// get a Stack
				stack_at_l_bl_brace = new AlignStack();
				// start it
				stack_at_l_bl_brace->Start(myspan, mythresh);
				stack_at_l_bl_brace->m_gap = mybr_gap;
				// store
				many_as_brace.at(pc->GetLevel()).at(pc->GetBraceLevel()) = stack_at_l_bl_brace;
			}
			stack_at_l_bl_brace->Debug();
			stack_at_l_bl_brace->NewLines(pc->GetNlCount());
		}
		else if(pc->Is(CT_FUNC_PROTO) || (pc->Is(CT_FUNC_DEF) && options::align_single_line_func())) {
			log_rule_B("align_single_line_func");
			log_rule_B("align_on_operator");
			if(pc->GetParentType() == CT_OPERATOR && options::align_on_operator()) {
				toadd = pc->GetPrevNcNnl();
			}
			else {
				toadd = pc;
			}
			Chunk * tmp = step_back_over_member(toadd);
			LOG_FMT(LAS, "%s(%d): tmp->Text() is '%s', orig line is %zu, orig col is %zu, level is %zu, brace level is %zu\n",
			    __func__, __LINE__, tmp->Text(), tmp->GetOrigLine(), tmp->GetOrigCol(),
			    tmp->GetLevel(), tmp->GetBraceLevel());
			// test the Stack
			AlignStack * stack_at_l_bl = many_as.at(pc->GetLevel()).at(pc->GetBraceLevel());

			if(stack_at_l_bl == nullptr) {
				// get a Stack
				stack_at_l_bl = new AlignStack();
				// start it
				stack_at_l_bl->Start(myspan, mythresh);
				stack_at_l_bl->m_gap        = mygap;
				stack_at_l_bl->m_star_style = static_cast<AlignStack::StarStyle>(mystar_style);
				stack_at_l_bl->m_amp_style  = static_cast<AlignStack::StarStyle>(myamp_style);
				// store
				many_as.at(pc->GetLevel()).at(pc->GetBraceLevel()) = stack_at_l_bl;
			}
			stack_at_l_bl->Add(tmp);
			log_rule_B("align_single_line_brace");
			look_bro = (pc->Is(CT_FUNC_DEF))
			    && options::align_single_line_brace();
		}
		else if(look_bro && pc->Is(CT_BRACE_OPEN) && pc->TestFlags(PCF_ONE_LINER)) {
			AlignStack * stack_at_l_bl_brace = many_as_brace.at(pc->GetLevel()).at(pc->GetBraceLevel());
			if(stack_at_l_bl_brace == nullptr) {
				stack_at_l_bl_brace = new AlignStack();
				stack_at_l_bl_brace->Start(myspan, mythresh);
				stack_at_l_bl_brace->m_gap = mybr_gap;
				many_as_brace.at(pc->GetLevel()).at(pc->GetBraceLevel()) = stack_at_l_bl_brace;
			}
			stack_at_l_bl_brace->Debug();
			stack_at_l_bl_brace->Add(pc);
			look_bro = false;
		}
	}
	LOG_FMT(LAS, "%s(%d):  as\n", __func__, __LINE__);
	// purge
	for(size_t idx = 0; idx < num_of_column; idx++) {
		for(size_t idx_brace = 0; idx_brace < num_of_row; idx_brace++) {
			AlignStack * stack_at_l_bl = many_as.at(idx).at(idx_brace);
			if(stack_at_l_bl != nullptr) {
				stack_at_l_bl->End();
				delete stack_at_l_bl;
				stack_at_l_bl = nullptr;
			}
			AlignStack * stack_at_l_bl_brace = many_as_brace.at(idx).at(idx_brace);

			if(stack_at_l_bl_brace != nullptr) {
				stack_at_l_bl_brace->End();
				delete stack_at_l_bl_brace;
				stack_at_l_bl_brace = nullptr;
			}
		}
	}
} // align_func_proto
