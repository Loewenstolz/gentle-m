/** \file
	\brief Contains the TVector class, as well as helper classes TAAProp, TORF, Tdna2aa, and TVectorItem
*/
#ifndef _TVECTOR_H_
#define _TVECTOR_H_

#include "main.h"
#include "TUndo.h"

class MyChild ;
class TRestrictionEnzyme ;
class TRestrictionCut ;
class TGenBank ;
class TVectorTree ;
class PlasmidCanvas ;
class TVectorEditor ;
class ChildBase ;
class SeqAA ;
class TEnzymeRules ;

//number of possible types
#define VIT_TYPES     10

#define VIT_GENE       1
#define VIT_CDS        2
#define VIT_REP_ORI    3
#define VIT_PROMOTER   4
#define VIT_TERMINATOR 5
#define VIT_MISC       6
#define VIT_PROT_BIND  7
#define VIT_ORI_T      8
#define VIT_SEQUENCING 9

/// This class manages amino acid properties; so 20 total. Used by the static part of TVector
class TAAProp
    {
    public :
    TAAProp () ; ///< Constructor
    virtual void set_cf ( int pa , int pb , int pt , float f0 , float f1 , float f2 , float f3 ) ; ///< Some data
    virtual void set_data ( float _mw , float _pi , wxString _tla ) ; ///< Molecular weight, isoelectric point, three-letter acronym
    virtual void set_hp ( float _hp_kd , float _hp_hw ) ; ///< More data
    virtual void set_atoms ( int C , int H , int N , int O , int S ) ; ///< Atoms in this amino acid
    virtual void set_halflife ( int mammal , int yeast , int ecoli ) ; ///< Half-life (not the game!)
    virtual wxString get_halflife_text ( int hl ) ; ///< Returns half-life estimation
    wxString tla ; ///< Three-letter acronym
    float mw , pi ;
    float cf_f[4] ; ///< Some data
    int cf_pa , cf_pb , cf_pt ;
    int carbon , hydrogen , nitrogen , oxygen , sulfur ;
    int hl_mammal , hl_yeast , hl_ecoli ;
    float hp_kd , hp_hw ;
    vector <float> data ; ///< For temporary data only
    } ;

/// This class manages open reading frames (ORFs)
class TORF
    {
    public :
    TORF () ; ///< Default constructor, empty
    TORF ( int _f , int _t , int _r ) ; ///< Constructor
    
    virtual wxString getText () ;
    
    /// \brief Start of ORF
    int from , to ; ///< End of ORF
    int rf ; ///< Reading frame
    
    float dist1 , dist2 ;
    float deg1 , deg2 ;
    } ;

/// This class stores a codon and the corresponding amino acid. Used by TVector
class Tdna2aa
    {
    public :
    Tdna2aa () {} ; ///< Default constructor, empty
    Tdna2aa ( char _aa , int i1 , int i2 , int i3 ) ; ///< Constructor
    char aa ; ///< Single-letter AA code
    int dna[3] ; ///< The three DNA positions that make up the codon
    } ;

/// This class stores a vector item.
class TVectorItem
    {
    public :
    TVectorItem () ; ///< Default constructor
    TVectorItem ( wxString sn , wxString n , int f , int t , char ty ) ; ///< Constructor
    
    virtual wxBrush *getBrush () ; ///< Returns a pointer to the brush used to draw the item
    virtual wxColour getFontColor () ; ///< Returns the font color
    virtual void setColor ( wxColour col ) ; ///< Sets the font color
    virtual bool isVisible () ; ///< Is this item visible?
    virtual void setVisible ( bool b ) ; ///< Set visibility
    virtual int getRF () ; ///< Returns the reading frame
    virtual void setRF ( int x ) ; ///< Sets the reading frame
    virtual int getOffset () ; ///< returns the offset; -1 = no offset
    virtual void setOffset ( int o = -1 ) ; ///< Sets the offset; -1 = no offset
    virtual void setType ( wxString s ) ; ///< Sets the item type
    virtual int getOffsetAt ( int i ) ; ///< Returns the offset of the item at a specific position
    
    virtual void doRemove ( int f , int t , int l ) ; ///< Remove a part of the item
    virtual wxString implodeParams () ; ///< Join item parameters for storage
    virtual void explodeParams ( wxString _s ) ; ///< Extract item parameters from stored form
    virtual wxTreeItemId getTreeID () { return treeid ; } ; ///< The current ID of the item in TVectorTree
    virtual void setTreeID ( wxTreeItemId newid ) { treeid = newid ; } ///< Set new item ID in TVectorTree
    virtual int getDirection () { return direction ; } ///< Return the direction
    virtual void setDirection ( int newdir ) { direction = newdir ; } ///< Set the direction
    virtual char getType () { return type ; } ///< Returns the item type
    virtual void setType ( char newtype ) { type = newtype ; } ///< Sets the item type
    virtual int getMem () ; ///< Estimates the memory usage of this item (debugging use only)

    // Parameters
    virtual wxString getParam ( wxString p , wxString def = _T("") ) ; ///< Returns the value of a parameter; optional default value
    virtual wxArrayString getParamKeys () ; ///< Get a list of all parameter keys
    virtual void setParam ( wxString p , wxString v ) ; ///< Set a parameter key/value pair
    virtual void setParam ( wxString p , int v ) ; ///< Set a parameter key/value pair

    // dna2aa stuff
    virtual void translate ( TVector *v , SeqAA *aa , vector <Tdna2aa> &dna2aa ) ; ///< Translate DNA to amino acids
    virtual void getArrangedAA ( TVector *v , wxString &s , int disp , SeqAA *aa = NULL ) ; ///< Generate the amino acid sequence in place
    virtual wxString getAminoAcidSequence () ; ///< Return the amino acid sequence
    virtual void setLastVector ( TVector *v ) ; ///< Set the last TVector to own this item
    
    // Variables
    /// \brief Item description
    wxString desc , name ; ///< Item name
    
    int from , ///< Item start
    	to ; ///< Item end
    
    private:
    virtual void initParams () ; ///< Reset parameters
    
    signed char direction ; ///< The item direction; 1=clockwise, -1=counter-clockwise
    char type ; ///< The type of the item in VIT_XXX form
    wxTreeItemId treeid ; ///< The item ID in the TVectorTree
    
    /// \brief Parameter keys
    wxArrayString pname , pvalue ; ///< Parameter values
    vector <Tdna2aa> dna2aa_item ; ///< The cache of the translated amino acids
    TVector *lastVector ; ///< The last TVector to own this item

    // Visual information
    friend class TVectorEditor ;
    friend class MyChild ;
    friend class PlasmidCanvas ;
    int r1 , r2 ;
    int r3 , r4 ; // Only used in linar display mode
    float a1 , a2 ;
    } ;

/** \brief This class stores all sequence information, both DNA and amino acids

	This class is of extreme importance. It carries sequence data, annotated items,
	methods for sequence manipulation, and static data, e.e., about amino acid
	properties and translation tables. So don't let the name fool you, this class
	does much more than storing vectors.
*/
class TVector
    {
    public :
    TVector ( ChildBase *win = NULL ) ; ///< Constructor
    virtual ~TVector () ; ///< Destructor
    virtual void init () ; ///< Set up basic values
    virtual void clear () ; ///< Reset internal state
    
    // Restriction enzymes
    virtual void recalculateCuts () ; ///< Recalculate restriction enzyme cuts
    virtual void getCuts ( TRestrictionEnzyme *e , vector <TRestrictionCut> &ret , 
    						bool clear_vector = true , int max = 10000000 ) ; ///< Gets the cuts of restriction enzyme in this sequence
    virtual bool reduceToFragment ( TRestrictionCut left , TRestrictionCut right ) ; ///< Cuts off everything except what is betreen these two cuts
    virtual void doRestriction () ; ///< Performs restriction. See TRestrictionEditor
    virtual void sortRestrictionSites () ; ///< Sorts the restriction sites by point of cut
    virtual int countCuts ( wxString enzyme ) ; ///< Counts the number of cuts for an enzyme in this sequence
    virtual void methylationSites ( wxArrayInt &vi , int what = ALL_METHYLATION_ENZYMES ) ; ///< Finds methylation sites
    
    // Nucleotide access/conversion
    virtual bool basematch ( char b1 , char b2 ) ; ///< matches DNA bases; b1 in IUPAC, b2 in SIUPAC. This is superior to the "=" operator :-)
    virtual void setIUPAC ( char b , char *s , char *pac = NULL ) ;
    virtual char getNucleotide ( int pos , bool complement = false ) ; ///< Returns the base at a certain position, or its complement
    virtual void setNucleotide ( int pos , char t ) ; ///< Sets a base at a certain position
    virtual char getComplement ( char c ) ; ///< Returns the complement of a given base
    
    // Vector/sequence access
    virtual void ligate_right ( TVector &v , bool inverted = false ) ; ///< Adds another TVector to the right of this one
    virtual void closeCircle () ; ///< Closes a linear sequence into a circular (intramolecular ligation)
    virtual void turn ( int off ) ; ///< Turns a circular sequence "off" bases
    
    virtual TVector *getAAvector ( int from , int to , int dir = 1 ) ; ///< Returns part of the sequence translated into amino acids
    virtual void doAction () ; ///< Perform an action (restriction, usually). Internal use
    virtual void doRemove ( int from , int to , bool update = true , bool enableUndo = true ) ; ///< Removes part of the sequence
    virtual void insert_char ( char x , int pos , bool overwrite = false ) ; ///< Inserts a character into the sequence
    
    virtual float getAAmw ( char aa ) ; ///< Returns the molecular weight of an amino acid
    virtual float getAApi ( char aa ) ; ///< Returns the isoelectric point of an amino acid
    virtual wxString dna2aa ( wxString codon , int translation_table = -1 ) ; ///< Translates a codon into an amino acid
    virtual void setAction ( wxString _action , int _action_value = 0 ) ; ///< Sets the action for doAction()
    virtual void setDatabase ( wxString s ) { database = s ; } ///< Sets the database in which the sequence is stored
    virtual wxString getDatabase () { return database ; } ///< Returns the database name in which the sequence is stored
    virtual TVector *newFromMark ( int from , int to ) ; ///< Generates a new sequence based on a part of this one
    
    virtual void setChanged ( bool c = true ) ; ///< The sequence has been changed.
    virtual bool isChanged () { return changed ; } ///< Was the sequence changed?
    
    virtual void ClearORFs () ; ///< Clear the found open reading frames
    virtual void addORFs ( int off ) ; ///< Add open reading frames for a specific reading frame

    virtual void removeBlanksFromSequence () ; ///< Removes blank chars (spaces etc.) from the sequence
    virtual void removeBlanksFromVector () ; ///< Removes blank chars (spaces etc.) from the sequence, altering items if necessary
    
    virtual wxString getParams () ; ///< Return sequence parameters
    virtual void setParams ( wxString t ) ; ///< Set sequence parameters
    virtual wxString getParam ( wxString key ) ; ///< Return the value of a parameter
    virtual void setParam ( wxString key , wxString value ) ; ///< Set a parameter key/value pair
    virtual void setWindow ( ChildBase *c ) ; ///< Set the window which owns this sequence
    virtual void setCircular ( bool c = true ) ; ///< This is a circular sequence (or linear, if c == false)
    virtual bool isCircular () ; ///< Is this a circular sequence?
    virtual bool isLinear () ; ///< Is this a linear sequence?
    virtual wxString one2three ( int a ) ; ///< Converts one letter amino acid code to three-letter acronym
    virtual char three2one ( wxString s ) ; ///< Converts three-letter acronym to one letter amino acid code
    virtual void setStickyEnd ( bool left , bool upper , wxString s ) ; ///< Sets one of the four possible sticky ends
    virtual wxString getStickyEnd ( bool left , bool upper ) ; ///< Returns one of the possible sticky ends
    virtual bool hasStickyEnds () ; ///< Does this sequence have sticky ends?
    virtual void callUpdateUndoMenu () ; ///< Refreshes the Undo menu
    virtual void setFromVector ( TVector &v ) ; ///< Makes this sequence a copy of another one (v)
    virtual void doRemoveNucleotide ( int x ) ; ///<Removes single base at position x
    virtual int getItemLength ( int a ) ; ///< Return the length of item a
    virtual TVector *backtranslate ( wxString mode = _T("") ) ; ///< Generate a new DNA sequence from this amino acid sequence
    virtual wxString getStrand53 () ; ///< Returns the 5'->3' strand of the sequence
    virtual wxString getStrand35 () ; ///< Returns the 3'->5' strand of the sequence, as 5'->3'

    virtual TAAProp getAAprop ( char a ) ; ///< Returns the properties of the one-letter code amino acid
    virtual int find_item ( wxString s ) ; ///< Finds an item with that name
    virtual bool isEnzymeHidden ( wxString s ) ; ///< Is enzyme "s" hidden?
    virtual void hideEnzyme ( wxString s , bool hideit = true ) ; ///< Set enzyme hidden state

    virtual wxString getSubstring ( int mf , int mt ) ; ///< Returns a sequence substring
    virtual wxString transformSequence ( bool inverse , bool reverse ) ; ///< Transforms the sequence
    virtual wxString getSequence () ; ///< Returns the sequence
    virtual wxString *getSequencePointer () ; ///< Internal use; only used by SequenceCanvas::OnCharHook
    virtual char getSequenceChar ( int x ) ; ///< Gets base at position x
    virtual void setSequence ( wxString ns ) ; ///< Sets the sequence
    virtual void addToSequence ( wxString x ) ; ///< Appends to the sequence
    virtual void alterSequence ( int pos , char c ) ; ///< Changes the base at position pos into c
    virtual int getSequenceLength() ; ///< Returns the sequence length
    virtual void eraseSequence ( int from , int len ) ; ///< Removes part of the sequence
    virtual void removeAlignmentArtifacts ( char what = '-' ) ; ///< Removes alignment artifacts from the sequence
    
    virtual wxString getDescription () ; ///< Returns the sequence description
    virtual void setDescription ( wxString s ) ; ///< Sets the sequence description
    virtual void addDescription ( wxString s ) ; ///< Appends to the sequence description
    virtual wxString getName () ; ///< Returns the sequence name
    virtual void setName ( wxString s ) ; ///< Sets the sequence name
    virtual void addName ( wxString s ) ; ///< Appends to the sequence name
    virtual void setGenomeMode ( bool gm = true ) ; ///< Turns genome mode on/off
    virtual bool getGenomeMode () ; ///< Returns the genome mode
    virtual int getMem () ; ///< Returns memory usage estimate for this sequence. Debugging use only
    virtual bool getVectorCuts ( TVector *v ) ; ///< Returns wether or not isoenzymes should be joined
    virtual TEnzymeRules *getEnzymeRule () ; ///< Gets the enzyme rules to follow
    virtual int showGC () ; ///< Returns 0 for "no", otherwise the number of blocks
    virtual TORF *getORF ( int a ) ; ///< Returns an open reading frame
    virtual int countORFs () ; ///< Returns the number of found open reading frames
    virtual void updateDisplay ( bool update = true ) ; ///< Recalc visual information on next draw
    virtual bool displayUpdate () ; ///< Update the display?
    virtual void setType ( int newtype ) ; ///< Set the sequence type
    virtual int getType () ; ///< Returns the sequence type
    virtual int getMethylationSiteIndex ( int pos ) ; ///< Returns the index of a potential methylation site
    virtual int getMethylationSite ( int index ) ; ///< Returns the position of a methylation site
    virtual int countMethylationSites () ; ///< Returns the number of found methylation sites
    virtual void prepareFeatureEdit ( int pos , bool overwrite ) ; ///< Changes feature names or cuts features that are about to be edited
    virtual TEnzymeRules *getEnzymeRules () ; ///< Returns the restriction enzyme display rules for this vector
    virtual void setEnzymeRules ( TEnzymeRules *er ) ; ///< Sets the restriction enzyme display rules for this vector
    
    // Variables
    vector <TVectorItem> items ; ///< Items/features/annotations
    vector <TRestrictionCut> rc ; ///< Restriction enzyme cuts
    
    wxArrayTRestrictionEnzyme re , ///< Manually specified restriction enzymes
    						  re2 ; ///< Automatically added restriction enzymes

    wxArrayString proteases ,  ///< Proteases used
     	cocktail ; ///< Enzymes from the last restriction
    TUndo undo ; ///< Undo information
    
    private :
    virtual wxString invert ( wxString s ) ; ///< Inverts a string
    virtual wxString vary_base ( char b ) ; ///< Turns a SIUPAC into a string of A, C, G, T
    virtual void makeAA2DNA ( wxString mode = _T("" ) ; ///< "Translate" amino acid sequence into DNA; can sp specified fo an organism
    virtual wxString mergeCodons ( wxString c1 , wxString c2 ) ; ///< Used by makeAA2DNA for generating "abstract" (SIUPAC) DNA
    virtual void setCodonTable ( int table , wxString sequence ) ; ///< Sets up the codon_tables variable
    virtual void evaluate_key_value ( wxString key , wxString value ) ; ///< Used in setParam() and setParams()

    int type ; ///< The sequence type
    TEnzymeRules *enzyme_rules ; ///< Pointer to the restriction enzyme display rules
    bool recalcvisual ; ///< Recalculate the layout of the sequence?
    vector <TORF> worf ; ///< Open Reading Frames
    wxArrayInt methyl ; ///< Methylation sites
    wxArrayString hiddenEnzymes ; ///< Enzymes that are not shown
    wxString sequence ; ///< The sequence that all this fuss is about
    wxString _lu ,  ///< Left upper sticky end
    		 _ll , ///< Left lower sticky end
       		 _ru , ///< Right upper sticky end
          	 _rl ; ///< Right lower sticky end
    wxString name , ///< The name of the sequence
    		 desc ; ///< The sequence description
    bool circular , ///< Is this sequence circular?
    	 changed , ///< Was this sequence changed?
         genomeMode ; ///< Is this a genome (using different display routines)?
    ChildBase *window ; ///< The window this TVector belongs to (might be NULL for most methods)
    int turned , ///< A circular sequence can be turned; this is the number of bases (negative for "left")
    	action_value ; ///< Used by doAction()
    wxString database , ///< The database this vector was loaded from
    		 action , ///< Used by doAction()
 			 aa ; ///< Hell if I remember what this is for
    wxString AA2DNA[256] ; ///< AA-to-DNA "translation" table; used by makeAA2DNA()
    wxArrayString paramk , ///< Parameter keys
    			  paramv ; ///< Parameter values
    
    static char IUPAC[256] , ///< The ACTG chars in binary encoding (IUPAC_A, IUPAC_C, IUPAC_T, IUPAC_G)
    			SIUPAC[256] , ///< The extended IUPAC codes in binary encoding
       			COMPLEMENT[256] ; ///< The complement to each SIUPAC base
    static char ACGT[256] ; ///< Different values for A, C, G, T; used in dna2aa()
    static vector <TAAProp> aaprop ; ///< The 20 amino acids and their properties
    static wxArrayString codon_tables ; ///< The codon tables for different organisms
    } ;
    
#endif

