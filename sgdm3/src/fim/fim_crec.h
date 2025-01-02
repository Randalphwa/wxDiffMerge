// fim_crec.h -- a change record -- represents one individual change
// or a transaction end-point against a piecetable.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_CREC_H
#define H_FIM_CREC_H

//////////////////////////////////////////////////////////////////

class fim_crec
{
public:
	typedef enum _verb
	{	verb_invalid			= 0,
		verb_ta_begin			= 1,	verb_ta_end				= -1,
		verb_text_insert		= 2,	verb_text_delete		= -2,
		verb_prop_set			= 3,	verb_prop_set2			= -3,	// both mean the same
	} Verb;
	
	typedef unsigned long	TAID;				// transaction id -- for matching up begin- & end- TA crec's
	
public:
	// WARNING: we do not allocate these -- because we treat this
	// as a struct and calloc/realloc an array of them in fim_crecvec.

	void					set_ta(Verb v, TAID t);
	void					set_text(Verb v, fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop);
	void					set_prop(fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop, fr_prop propNew);

	inline Verb				getVerb(void)      const { return m_verb; };
	inline TAID				getTAID(void)      const { return m_taid; };

	inline fb_offset		getBufOffset(void) const { return m_offsetBuf; };
	inline fim_length		getBufLength(void) const { return m_lenData; };
	inline fim_offset		getDocOffset(void) const { return m_offsetDoc; };
	inline fr_prop			getProp(void)      const { return m_prop; };
	inline fr_prop			getNewProp(void)   const { return m_propNew; };

	inline bool				isText(void)       const { return ((m_verb==verb_text_insert) || (m_verb==verb_text_delete)); };
	inline bool				isProp(void)       const { return ((m_verb==verb_prop_set)    || (m_verb==verb_prop_set2)  ); };

	inline void				expandRightEdge(fim_length delta)
		{
			m_lenData += delta;
		};
	inline void				expandLeftEdge(fim_length delta)	
		{
			m_offsetBuf -= delta;
			m_lenData += delta;
			m_offsetDoc -= delta;
		};

public:
	fim_crec(void);
	fim_crec(const fim_crec * pCRec);
	void					reverse(void);

private:
	Verb					m_verb;
	TAID					m_taid;				// valid when m_verb is one of verb_ta_*

	fb_offset				m_offsetBuf;		// offset in growbuf where raw data is -- valid when m_verb is one of verb_text_*
	fim_length				m_lenData;			// length of raw data inserted/deleted -- valid when m_verb is one of verb_text_*
	fim_offset				m_offsetDoc;		// absolute position in document of text -- valid when m_verb is one of verb_text_*
	fr_prop					m_prop;				// props for frag (when appropriate) -- valid when m_verb is one of verb_text_*
	fr_prop					m_propNew;			// new props for frag (when appropriate) -- valid when m_verb is one of verb_prop_*

#ifdef DEBUG
public:
	void					dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_CREC_H
