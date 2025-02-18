/** \file
	\brief Contains the AutoAnnotate class
*/
#ifndef __AUTOANNOTATE__
#define __AUTOANNOTATE__

#include "main.h"

class MyChild ;
/**	\brief The AutoAnnotate class scans database(s) for known features in the current sequence

	It uses the TAutoAnnotateDialog class to display a settings dialog, then runs through one or
	two databases (depending on the settings), extracting DNA, and matching it to the current sequence.
	Finally, it will add all not-identified open reading frames as "unknown" features.
*/
class AutoAnnotate
	{
	public :
	AutoAnnotate ( MyChild *_p = NULL ) ; ///< Constructor
	~AutoAnnotate () {} ; ///< Destructor (empty)

	bool SettingsDialog () ; ///< Runs the dialog; returns wether to Run or not
	void Run () ; ///< Runs the search
	
	private :
	bool ScanDatabase ( wxString database ) ; ///< Scans a database for matching features
	bool addORFs ( TVector *v ) ; ///< Adds open reading frames as features (that could not be identified)
 	bool MatchItem ( TVector *tv , TVectorItem &item , TVector *v , wxString &oseq ) ; ///< Compares two items to prevent double entries
 	bool RawMatch ( TVectorItem &item , TVector *v , wxString &oseq , wxString &s ) ; ///< Tries to match the full item sequence against the current sequence
 	void machete ( TVector *v ) ; ///< Reduces the number of identified items, so that one can actually read something in the map again :-)
 	bool within ( TVectorItem &i1 , TVectorItem &i2 , TVector *v ) ; ///< Checks if TVectorItem i2 lies within i1 (no need to add it then)

 	friend class TAutoAnnotateDialog ;
 	
 	MyChild *p ; ///< Pointer to the calling MyChild
 	wxArrayString alreadyin ; ///< Cache of items already added
 	wxString commonVectorsDatabase , additionalDatabase ;
 	bool useCommonDatabase , useAdditionalDatabase , useMachete , doAddORFs ;
    wxArrayString dbname , dbfile ;
	} ;

#endif
