// fim_patchset.h
// a list of patches to all be applied at once -- used by auto-merge
// feature.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_PATCHSET_H
#define H_FIM_PATCHSET_H

//////////////////////////////////////////////////////////////////

class fim_patch
{
public:
	fim_patch(void)				{};
	virtual ~fim_patch(void)	{};

	virtual void				setPatchOpCurrent(fim_patch_op op) = 0;

	virtual fim_patch_op		getPatchOpCurrent(void)			const = 0;
	virtual fim_patch_op		getPatchOpOriginal(void)		const = 0;
	virtual fim_offset			getDocPosWhere(void)			const = 0;

	virtual fim_length			getLenDelete(void)				const = 0;
	virtual const wxChar *		getNewText(void)				const = 0;
	virtual fim_length			getNewTextLen(void)				const = 0;
	virtual fr_prop				getProp(void)					const = 0;

#ifdef DEBUG
public:
	virtual void		dump(int indent)						const = 0;
#endif
};

//////////////////////////////////////////////////////////////////

typedef std::vector<fim_patch *>		TVec_Patches;
typedef TVec_Patches::iterator			TVec_PatchesIterator;
typedef TVec_Patches::reverse_iterator	TVec_PatchesReverseIterator;
typedef TVec_Patches::const_iterator	TVec_PatchesConstIterator;
typedef TVec_Patches::value_type		TVec_PatchesValue;

//////////////////////////////////////////////////////////////////

class fim_patchset
{
public:
	fim_patchset(void);
	~fim_patchset(void);

	void				appendPatch(fim_patch * pPatch);
	inline long			getNrPatches(void)		const { return (long)m_vecPatches.size(); };
	inline fim_patch *	getNthPatch(long ndx)	const { return m_vecPatches[ndx]; };

private:
	TVec_Patches		m_vecPatches;

#ifdef DEBUG
public:
	void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_PATCHSET_H
