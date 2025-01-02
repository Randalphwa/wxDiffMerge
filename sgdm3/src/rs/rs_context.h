// rs_context.h -- declaration for a single RuleSet "context".
// a "context" defines a span of text (which may be contained
// within one line, match a line, or match a series of lines)
// where we want special diff-engine treatment for the content
// within the span.
//
// this can be used to describe source comment syntax, string
// or character literals, etc.
//////////////////////////////////////////////////////////////////

#ifndef H_RS_CONTEXT_H
#define H_RS_CONTEXT_H

//////////////////////////////////////////////////////////////////

class rs_context
{
public:
	rs_context(rs_context_attrs attrs,
			   const wxString & strStartPattern,
			   const wxString & strEndPattern,
			   wxChar   chEscape=0,
			   bool bEndsAtEOL=true);
	rs_context(const rs_context & ctx);		// copy constructor
	~rs_context(void);

	bool						isEqual(const rs_context * pCTX) const;

//	inline bool					isValid(void)				const;

	inline bool					haveValidStartPattern(void)	const { return m_pReStartPattern && m_pReStartPattern->IsValid(); };
	inline bool					haveValidEndPattern(void)	const { return m_pReEndPattern   && m_pReEndPattern->IsValid();   };

	bool						isStartMatch(const wxChar * szLine, size_t * pOffset, size_t * pLen) const;
	bool						isEndMatch  (const wxChar * szLine, size_t * pOffset, size_t * pLen) const;

	inline rs_context_attrs		getContextAttrs(void)		const { return m_attrs; };
	inline const wxString *		getStartPatternString(void)	const { return &m_strStartPattern; };
	inline const wxString *		getEndPatternString(void)	const { return &m_strEndPattern; };
	inline wxChar				getEscapeChar(void)			const { return m_chEscape; };
	inline bool					getEndsAtEOL(void)			const { return m_bEndsAtEOL; };

	wxString					getSummaryDescription(void) const;

private:
	bool						_ends_in_escape(const wxChar * szBegin, const wxChar * szEnd) const;

private:
	rs_context_attrs			m_attrs;
	wxString					m_strStartPattern;
	wxString					m_strEndPattern;
	wxChar						m_chEscape;
	bool						m_bEndsAtEOL;

	wxRegEx	*					m_pReStartPattern;
	wxRegEx	*					m_pReEndPattern;

	// warning: if you add any fields, update isEqual() in addition to ctors

	//////////////////////////////////////////////////////////////////

#ifdef DEBUG
public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_CONTEXT_H
