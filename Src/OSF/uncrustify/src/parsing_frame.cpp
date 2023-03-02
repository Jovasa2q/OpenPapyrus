/**
 * @file parsing_frame.cpp
 *
 * Holds data needed for indenting and brace parsing
 *
 * @author  Daniel Chumak
 * @license GPL v2+
 */
#include <uncrustify-internal.h>
#pragma hdrstop
//#include "parsing_frame.h"
//#include "chunk.h"
//#include "uncrustify.h"

using std::string;
using std::to_string;
using std::invalid_argument;

//! amount of elements for which memory is going to be pre-initialized
static constexpr const int CONTAINER_INIT_SIZE = 16;

static ParenStackEntry genDummy()
{
	ParenStackEntry tmp_dummy{};
	tmp_dummy.indent     = 1;
	tmp_dummy.indent_tmp = 1;
	tmp_dummy.indent_tab = 1;
	tmp_dummy.type       = CT_EOF;
	tmp_dummy.pc         = Chunk::NullChunkPtr;
	tmp_dummy.pop_pc     = Chunk::NullChunkPtr;

	return (tmp_dummy);
}

void ParsingFrame::clear()
{
	m_lastPopped = genDummy();

	m_parenStack.clear();
	m_parenStack.push_back(genDummy());

	m_refNumber  = 0;
	m_parenLevel = 0;
	m_braceLevel = 0;
	pp_level     = 0;
	sparen_count = 0;
	paren_count  = 0;
	in_ifdef     = E_Token::CT_NONE;
	stmt_count   = 0;
	expr_count   = 0;
}

ParsingFrame::ParsingFrame()
{
	m_parenStack = std::vector<ParenStackEntry>();
	m_parenStack.reserve(CONTAINER_INIT_SIZE);
	clear();
}

ParenStackEntry &ParsingFrame::at(size_t idx)
{
	return (m_parenStack.at(idx));
}

const ParenStackEntry &ParsingFrame::at(size_t idx) const
{
	return (m_parenStack.at(idx));
}

ParenStackEntry &ParsingFrame::prev(size_t idx)
{
	LOG_FUNC_ENTRY();

	if(idx == 0) {
		throw invalid_argument(string(__FILE__) + ":" + to_string(__LINE__)
			  + " idx can't be zero");
	}

	if(idx >= m_parenStack.size()) {
		LOG_FMT(LINDPSE, "%s(%d): idx is %zu, size is %zu\n",
		    __func__, __LINE__, idx, m_parenStack.size());
		throw invalid_argument(string(__FILE__) + ":" + to_string(__LINE__)
			  + " idx can't be >= size()");
	}
	return (*std::prev(std::end(m_parenStack), idx + 1));
}

const ParenStackEntry &ParsingFrame::prev(size_t idx) const
{
	LOG_FUNC_ENTRY();

	if(idx == 0
	    || idx >= m_parenStack.size()) {
		throw invalid_argument(string(__FILE__) + ":" + to_string(__LINE__)
			  + " idx can't be zero or >= size()");
	}
	return (*std::prev(std::end(m_parenStack), idx + 1));
}

ParenStackEntry &ParsingFrame::top()
{
	// always at least one (dummy) element inside m_parenStack guaranteed
//   if (m_parenStack.empty())
//   {
//      throw logic_error(string(__FILE__) + ":" + to_string(__LINE__)
//                        + " called top on an empty stack");
//   }
	return (*std::prev(std::end(m_parenStack)));
}

const ParenStackEntry &ParsingFrame::top() const
{
	// always at least one (dummy) element inside m_parenStack guaranteed
//   if (m_parenStack.empty())
//   {
//      throw logic_error(string(__FILE__) + ":" + to_string(__LINE__)
//                        + " called top on an empty stack");
//   }
	return (*std::prev(std::end(m_parenStack)));
}

void ParsingFrame::push(std::nullptr_t, E_BraceStage stage)
{
	static Chunk dummy;

	push(&dummy, __func__, __LINE__, stage);
	top().pc = Chunk::NullChunkPtr;
}

void ParsingFrame::push(Chunk * pc, const char * func, int line, E_BraceStage stage)
{
	LOG_FUNC_ENTRY();

	ParenStackEntry new_entry = {};

	new_entry.type      = pc->GetType();
	new_entry.level     = pc->GetLevel();
	new_entry.open_line = pc->GetOrigLine();
	new_entry.open_colu = pc->GetOrigCol();
	new_entry.pc        = pc;

	new_entry.indent_tab  = top().indent_tab;
	new_entry.indent_cont = top().indent_cont;
	new_entry.stage       = stage;

	new_entry.in_preproc = pc->TestFlags(PCF_IN_PREPROC);
	new_entry.non_vardef = false;
	new_entry.ip         = top().ip;
	new_entry.pop_pc     = Chunk::NullChunkPtr;

	m_parenStack.push_back(new_entry);

// uncomment the line below to get the address of the m_parenStack
// #define DEBUG_PUSH_POP
#ifdef DEBUG_PUSH_POP
	LOG_FMT(LINDPSE, "ParsingFrame::push(%s:%d) Add is %4zu: orig line is %4zu, orig col is %4zu, type is %12s, "
	    "brace level is %2zu, level is %2zu, pse_tos: %2zu -> %2zu\n",
	    func, line, (size_t)this, pc->GetOrigLine(), pc->GetOrigCol(),
	    get_token_name(pc->GetType()), pc->GetBraceLevel(), pc->GetLevel(),
	    (m_parenStack.size() - 2), (m_parenStack.size() - 1));
#else /* DEBUG_PUSH_POP */
	LOG_FMT(LINDPSE, "ParsingFrame::push(%s:%d): orig line is %4zu, orig col is %4zu, type is %12s, "
	    "brace level is %2zu, level is %2zu, pse_tos: %2zu -> %2zu\n",
	    func, line, pc->GetOrigLine(), pc->GetOrigCol(),
	    get_token_name(pc->GetType()), pc->GetBraceLevel(), pc->GetLevel(),
	    (m_parenStack.size() - 2), (m_parenStack.size() - 1));
#endif /* DEBUG_PUSH_POP */
}

void ParsingFrame::pop(const char * func, int line, Chunk * pc)
{
	LOG_FUNC_ENTRY();

	// always at least one (dummy) element inside m_parenStack guaranteed
//   if (m_parenStack.empty())
//   {
//      throw logic_error(string(__FILE__) + ":" + to_string(__LINE__)
//                        + "the stack index is already zero");
//   }

	if(pc->GetType() == CT_PAREN_CLOSE
	    || pc->GetType() == CT_BRACE_CLOSE
	    || pc->GetType() == CT_VBRACE_CLOSE
	    || pc->GetType() == CT_FPAREN_CLOSE
	    || pc->GetType() == CT_LPAREN_CLOSE
	    || pc->GetType() == CT_SPAREN_CLOSE
	    || pc->GetType() == CT_TPAREN_CLOSE
	    || pc->GetType() == CT_CLASS_COLON
	    || pc->GetType() == CT_ANGLE_CLOSE
	    || pc->GetType() == CT_SEMICOLON
	    || pc->GetType() == CT_SQUARE_CLOSE) {
		LOG_FMT(LINDPSE, "ParsingFrame::pop (%s:%d): orig line is %4zu, orig col is %4zu, type is %12s, pushed with\n",
		    func, line, pc->GetOrigLine(), pc->GetOrigCol(), get_token_name(pc->GetType()));
	}
	else if(pc->GetType() == CT_ACCESS
	    || pc->GetType() == CT_ASSIGN
	    || pc->GetType() == CT_BRACE_OPEN
	    || pc->GetType() == CT_BOOL
	    || pc->GetType() == CT_CASE
	    || pc->GetType() == CT_COMMA
	    || pc->GetType() == CT_COMMENT
	    || pc->GetType() == CT_COMMENT_CPP
	    || pc->GetType() == CT_COMMENT_MULTI
	    || pc->GetType() == CT_COMPARE                 // Issue #3915
	    || pc->GetType() == CT_COND_COLON
	    || pc->GetType() == CT_FPAREN_OPEN
	    || pc->GetType() == CT_PAREN_OPEN
	    || pc->GetType() == CT_TPAREN_OPEN
	    || pc->GetType() == CT_MACRO_CLOSE
	    || pc->GetType() == CT_MACRO_OPEN
	    || pc->GetType() == CT_NEWLINE
	    || pc->GetType() == CT_NONE
	    || pc->GetType() == CT_OC_END
	    || pc->GetType() == CT_OC_MSG_NAME
	    || pc->GetType() == CT_OC_SCOPE
	    || pc->GetType() == CT_OC_PROPERTY
	    || pc->GetType() == CT_PREPROC
	    || pc->GetType() == CT_SQUARE_OPEN
	    || pc->GetType() == CT_SQL_END
	    || pc->GetType() == CT_TYPEDEF
	    || pc->GetType() == CT_VSEMICOLON
	    || pc->GetType() == CT_WORD) {
		LOG_FMT(LINDPSE, "ParsingFrame::pop (%s:%d): orig line is %4zu, orig col is %4zu, type is %12s\n",
		    func, line, pc->GetOrigLine(), pc->GetOrigCol(), get_token_name(pc->GetType()));
	}
	else {
		LOG_FMT(LINDPSE, "ParsingFrame::pop (%s:%d): orig line is %4zu, orig col is %4zu, type is %12s,\n",
		    func, line, pc->GetOrigLine(), pc->GetOrigCol(), get_token_name(pc->GetType()));
		LOG_FMT(LINDPSE, "ParsingFrame::pop (%s:%d): the type is %s, is not coded. Please make a call.\n",
		    func, line, get_token_name(pc->GetType()));
		log_flush(true);
		exit(EX_SOFTWARE);
	}
#ifdef DEBUG_PUSH_POP
	LOG_FMT(LINDPSE, "ParsingFrame::pop (%s:%d) Add is %4zu: open_line is %4zu, clos_col is %4zu, type is %12s, "
	    "cpd.level   is %2d, level is %2zu, pse_tos: %2zu -> %2zu\n",
	    func, line, (size_t)this, m_parenStack.back().open_line, m_parenStack.back().open_colu,
	    get_token_name(m_parenStack.back().type), cpd.pp_level, m_parenStack.back().level,
	    (m_parenStack.size() - 1), (m_parenStack.size() - 2));
#else /* DEBUG_PUSH_POP */
	LOG_FMT(LINDPSE, "ParsingFrame::pop (%s:%d): open_line is %4zu, clos_col is %4zu, type is %12s, "
	    "cpd.level   is %2d, level is %2zu, pse_tos: %2zu -> %2zu\n",
	    func, line, m_parenStack.back().open_line, m_parenStack.back().open_colu,
	    get_token_name(m_parenStack.back().type), cpd.pp_level, m_parenStack.back().level,
	    (m_parenStack.size() - 1), (m_parenStack.size() - 2));
#endif /* DEBUG_PUSH_POP */

	m_lastPopped = *std::prev(std::end(m_parenStack));

	if(m_parenStack.size() == 1) {
		*std::begin(m_parenStack) = genDummy();
	}
	else {
		m_parenStack.pop_back();
	}
} // ParsingFrame::pop
